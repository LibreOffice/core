# This config file adds vfprintf.c and memcpy.c for those systems that
# do not have it.
#

osredir := $(OS)$(DIRSEPSTR)$(OSRELEASE)$(DIRSEPSTR)$(OSENVIRONMENT)
CFLAGS += -I$(osredir)

# The following sources are required for Coherent version 4.2
#OSDESRC :=
#SRC    += $(OSDESRC)
#.SETDIR=$(osredir) : $(OSDESRC)
