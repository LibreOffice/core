# This config file adds vfprintf.c and memcpy.c for those systems that
# do not have it.
#

osredir := $(OS)$(DIRSEPSTR)$(OSRELEASE)$(DIRSEPSTR)$(OSENVIRONMENT)
CFLAGS += -I$(osredir)

# The following sources are required for coherent version 4.0
OSDESRC := vfprintf.c getcwd.c
SRC    += $(OSDESRC)
.SETDIR=$(osredir) : $(OSDESRC)
