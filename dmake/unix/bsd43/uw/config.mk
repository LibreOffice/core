# This is the BSD 4.3 University of Waterloo (uw) UNIX configuration file
# for DMAKE
#	It simply modifies the values of LDLIBS to include libuw.a
#	so that vfprintf can be found.
#

LDLIBS += -luw
osredir := $(OS)$(DIRSEPSTR)$(OSRELEASE)$(DIRSEPSTR)$(OSENVIRONMENT)
CFLAGS += -I$(osredir)

# install script for UW's /usr/software hierarchy...
install:
    mkdir ../bin; strip ./dmake; mv ./dmake ../bin
    chmod a+rx ../bin/dmake ../bin
    mkdir ../lib; chmod a+rx ../lib
    cp $(STARTUPFILE) ../lib
    chmod a+r ../lib/startup.mk
