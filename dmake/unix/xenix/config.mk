# This is the SysV R3 UNIX configuration file for DMAKE
#	It simply modifies the values of SRC, and checks to see if
#	OSENVIRONMENT is defined.  If so it includes the appropriate
#	config.mk file.
#
# It also sets the values of .SOURCE.c and .SOURCE.h to include the local
# directory.
#
osrdir := $(OS)$(DIRSEPSTR)$(OSRELEASE)

# The following are required sources
OSDSRC := 
.IF $(OSDSRC)
   SRC    += $(OSDSRC)
   .SETDIR=$(osrdir) : $(OSDSRC)
.END

.SOURCE.h : $(osrdir)

# Local configuration modifications for CFLAGS, there's local SysV includes
# too.
CFLAGS += -I$(osrdir) -DM_XENIX

# See if we modify anything in the lower levels.
.IF $(OSENVIRONMENT) != $(NULL)
   .INCLUDE .IGNORE : $(osrdir)$(DIRSEPSTR)$(OSENVIRONMENT)$(DIRSEPSTR)config.mk
.END
