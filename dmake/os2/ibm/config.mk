# This is the MSC 4.0 and higher DOS configuration file for DMAKE
#	It simply modifies the values of SRC, and checks to see if
#	OSENVIRONMENT is defined.  If so it includes the appropriate
#	config.mk file.
#
# It also sets the values of .SOURCE.c and .SOURCE.h to include the local
# directory.
#
osrdir := $(OS)$(DIRSEPSTR)$(OSRELEASE)

TMPDIR :=
.EXPORT : TMPDIR

# Definition of macros for library, and C startup code.

# The following sources are required for MSC
OSR_SRC = tempnam.c
.SETDIR=$(osrdir) : $(OSR_SRC)

SRC += $(OSR_SRC)
.SOURCE.h : $(osrdir)

SET_STACK    = $-stack:32768
NDB_LDFLAGS +=

ASFLAGS += -t -mx $(S_$(MODEL))

# Microsoft C doesn't need tail but needs head
LDTAIL = ,;
LDHEAD = $(LDFLAGS)

# Debugging libraries
DB_LDFLAGS += $-co $-li $-map $(SET_STACK)
DB_LDLIBS  +=

# NO Debug MSC flags:
# Set the environment variable MSC_VER to be one of 5.1, 6.0, 8.0 (for VC++4.0)
# to get these by default when you make dmake using 'dmake'.
#
# Setting MSC_VER to one of the above sets the variable _MSC_VER appropriately
# and sets the flags appropriately.

CFLAGS    += $-I$(osrdir)
DB_CFLAGS += $-Ti+

# See if we modify anything in the lower levels.
.IF $(OSENVIRONMENT) != $(NULL)
   .INCLUDE .IGNORE : $(osrdir)$(DIRSEPSTR)$(OSENVIRONMENT)$(DIRSEPSTR)config.mk
.END

S_s = -Dmsmall
S_m = -Dmmedium
S_c = -Dmcompact
S_l = -Dmlarge
