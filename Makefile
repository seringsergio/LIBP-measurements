CONTIKI = contiki
CONTIKI_PROJECT = example-libp
APPS += powertrace
PROJECT_SOURCEFILES += libp.c libp-neighbour.c libp-link-metric.c tree.c queue.c

all: example-libp
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
include $(CONTIKI)/Makefile.include
