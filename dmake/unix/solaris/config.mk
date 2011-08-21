# This is the BSD 4.3 UNIX configuration file for DMAKE
#	It simply modifies the values of SRC, and checks to see if
#	OSENVIRONMENT is defined.  If so it includes the appropriate
#	config.mk file.
#
# It also sets the values of .SOURCE.c and .SOURCE.h to include the local
# directory.
#
osrdir := $(OS)$(DIRSEPSTR)$(OSRELEASE)

# The following sources are required for Solaris 2.1 or greater
OSDSRC := tempnam.c getcwd.c
.IF $(OSDSRC)
   SRC    += $(OSDSRC)
   .SETDIR=$(osrdir) : $(OSDSRC)
.END

.SOURCE.h : $(osrdir)

# Local configuration modifications for CFLAGS, there's local BSD includes
# too.
CFLAGS += -I$(osrdir) -DSolaris

# See if we modify anything in the lower levels.
.IF $(OSENVIRONMENT) != $(NULL)
   .INCLUDE .IGNORE : $(osrdir)$(DIRSEPSTR)$(OSENVIRONMENT)$(DIRSEPSTR)config.mk
.END
