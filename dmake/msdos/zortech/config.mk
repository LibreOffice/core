# This is the ZTC DOS configuration file for DMAKE
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
# Swapping for DOS versions is enabled by default.  ZTC will automatically
# perform swapping to XMS, EMS or disk by including _swapl.obj at link time.
# To be most effective, _swapl.obj should be the first file linked so we
# assign it to CSTARTUP if needed.
.IF $(SWAP) == y
   CSTARTUP = _swapl.obj
.END

# The following sources are required for ZTC
# The tempnam supplied with ZTC doesn't handle a NULL dir.
OSR_SRC = tempnam.c environ.c
.SETDIR=$(osrdir) : $(OSR_SRC)

SRC += $(OSR_SRC)
.SOURCE.h : $(osrdir)

# Local configuration modifications for CFLAGS 
# If you have a 286, you can use -2 or appropriate to get better code, 
# in that case uncomment the line below.  (You can also simply set
# it in the CL environment variable.)
#CFLAGS += -2
ASFLAGS += -t -mx $(S_$(MODEL))

# Redefine this, it isn't needed!
LDTAIL = ;

# Debugging libraries
DB_LDFLAGS += -g
DB_LDLIBS  +=

# NO Debug ZTC flags:
#

CFLAGS      += -I$(osrdir) $(C_$(MODEL))
CFLAGS      += -DM_I86=1 -DMSDOS
CFLAGS      += -b             # use large compiler
#CFLAGS      += -w            # no warnings
CFLAGS      += -mi            # integer only
CFLAGS      += -p             # no auto-prototyping
NDB_CFLAGS  += -o
DB_CFLAGS   += -g

# Redefine rule for making our objects, we don't need mv
%$O : %.c ;% $(CC) -c $(CFLAGS) -o$@ $<

# See if we modify anything in the lower levels.
.IF $(OSENVIRONMENT) != $(NULL)
   .INCLUDE .IGNORE : $(osrdir)$(DIRSEPSTR)$(OSENVIRONMENT)$(DIRSEPSTR)config.mk
.END

C_s =
C_m = -mM
C_c = -mC
C_l = -mL

S_s = -Dmsmall
S_m = -Dmmedium
S_c = -Dmcompact
S_l = -Dmlarge
