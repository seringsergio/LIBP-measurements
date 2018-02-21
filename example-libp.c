/**
 * \file
 *         Example of how the LIBP paradigm works.
 * \author
 *         Lutando Ngqakaza <lutando.ngqakaza@gmail.com>
 */

#include "contiki.h"
#include "lib/random.h"
#include "net/rime.h"
#include "libp.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "tree.h"
#include "node.h"
#include "queue.h"
#include "net/netstack.h"
 #include "powertrace.h"

#include <stdio.h>


#define BEACONING_PERIOD 30
#define CHANNEL 130

static struct libp_conn lc;
static int is_sink = 0;

/*---------------------------------------------------------------------------*/
PROCESS(example_libp_process, "Test LIBP process");
PROCESS(gateway_monitoring_process, "Gateway Monitoring Process");
PROCESS(energy_powertrace, "energy measurement");

AUTOSTART_PROCESSES(&example_libp_process, &energy_powertrace);
/*---------------------------------------------------------------------------*/
static void
recv(const rimeaddr_t *originator, uint8_t seqno, uint8_t hops)
{
    printf("Sink got message from %d.%d, seqno %d, hops %d: len %d '%s'\n",
           originator->u8[0], originator->u8[1],
           seqno, hops,
           packetbuf_datalen(),
           (char *)packetbuf_dataptr());
}
/*---------------------------------------------------------------------------*/
static const struct collect_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(energy_powertrace, ev, data)
{
  PROCESS_BEGIN();

  powertrace_start(CLOCK_SECOND * 10);

  PROCESS_END();
}

PROCESS_THREAD(example_libp_process, ev, data)
{
    static struct etimer periodic;
    static struct etimer et;

    PROCESS_BEGIN();


    libp_open(&lc, CHANNEL, LIBP_ROUTER, &callbacks);

    if(rimeaddr_node_addr.u8[0] == 1 &&
            rimeaddr_node_addr.u8[1] == 0)
    {
        clock_time_t period;
        period = CLOCK_SECOND * BEACONING_PERIOD;
        printf("I am sink\n");
        libp_set_sink(&lc, 1);
        is_sink = 1;
        libp_set_beacon_period(&lc, period);
        tree_init(); //only gateway needs to use the tree methods
        process_start(&gateway_monitoring_process, NULL);

    }

    /* Allow some time for the network to settle. */
    etimer_set(&et, 120 * CLOCK_SECOND);
    PROCESS_WAIT_UNTIL(etimer_expired(&et));

    while(1)
    {

        /* Send a packet every 30 seconds. */
        if(etimer_expired(&periodic))
        {
            etimer_set(&periodic, CLOCK_SECOND * 30);
            etimer_set(&et, random_rand() % (CLOCK_SECOND * 30));
        }

        PROCESS_WAIT_EVENT();


        if(etimer_expired(&et))
        {

            static rimeaddr_t oldparent;
            const rimeaddr_t *parent;

            printf("Sending\n");
            //printf("stats %d %d %d",  stats.beaconsent, stats.acksent ,stats.datasent );
            packetbuf_clear();
            parent = libp_parent(&lc);
            packetbuf_set_datalen(sprintf(packetbuf_dataptr(),
                                          "%s %d", "Hello", (int)parent->u8[0]) + 1);
            libp_send(&lc, 15);

            parent = libp_parent(&lc);
            if(!rimeaddr_cmp(parent, &oldparent))
            {
                if(!rimeaddr_cmp(&oldparent, &rimeaddr_null))
                {
                    printf("#L %d 0\n", oldparent.u8[0]);
                }
                if(!rimeaddr_cmp(parent, &rimeaddr_null))
                {
                    printf("#L %d 1\n", parent->u8[0]);
                }
                rimeaddr_copy(&oldparent, parent);
            }
        }

    }

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(gateway_monitoring_process, ev, data)
{
    static struct etimer monitor_timer;
    static struct etimer wait_timer;
    //xint monitoring
    PROCESS_BEGIN();

    //Allow some time for system to detect weather it is a gateway or not.
    etimer_set(&wait_timer, 60 * 2 * CLOCK_SECOND);
    PROCESS_WAIT_UNTIL(etimer_expired(&wait_timer));

    //If the node is not a sink, turn off this process.
    printf("Hi from sink thread\n\n");

    while(1)
    {
        //monitoring loop

        // Monitor every 20 seconds
        etimer_set(&monitor_timer, CLOCK_SECOND * 20 + random_rand() % (CLOCK_SECOND * 20));

        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&monitor_timer));

        //monitor
    }

    PROCESS_END();
}
