# This config file adds vfprintf.c and memcpy.c for those systems that
# do not have it.
#

osredir := $(OS)$(DIRSEPSTR)$(OSRELEASE)$(DIRSEPSTR)$(OSENVIRONMENT)
CFLAGS += -I$(osredir)

# The following sources are required for BSD4.3
OSDESRC := memcpy.c vfprintf.c
SRC    += $(OSDESRC)
.SETDIR=$(osredir) : $(OSDESRC)
