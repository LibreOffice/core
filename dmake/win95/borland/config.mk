# This is the Turbo C++ 2.0 DOS configuration file for DMAKE
#	It simply modifies the values of SRC, and checks to see if
#	OSENVIRONMENT is defined.  If so it includes the appropriate
#	config.mk file.
#
# It also sets the values of .SOURCE.c and .SOURCE.h to include the local
# directory.
#
osrdir := $(OS)$(DIRSEPSTR)$(OSRELEASE)

# OS specific sources
OSR_SRC  += tempnam.c ruletab.c
DOS_SRC += runargv.c rmprq.c

SRC += $(OSR_SRC) $(DOS_SRC)
.SOURCE.h : $(osrdir)

# Local configuration modifications for CFLAGS.  Make sure your turboc.cfg
# file contains a -D__STDC__=1 and -DM_I86=1, if not then uncomment the line
# below!
#CFLAGS += -DM_I86=1 -D__STDC__=1

# You can get a smaller executable still, buy adding a -1 to the list of
# flags below, but then you can't run this on an 8086/88 cpu.
#CFLAGS += -1
CFLAGS  += -I$(osrdir) -d -O -N- -w-nod
ASFLAGS += -t -mx $(S_$(MODEL))

LDOBJS		= $(CSTARTUP) $(OBJDIR)/{$(<:f)}
LDARGS		= $(LDHEAD) @$(LDTMPOBJ),$(TARGET),NUL.MAP$(LDTAIL)
LDTAIL		= $(_libs)$(LDFLAGS:s/ //)
_libs           = $(!null,$(LDLIBS) ,@$(LDTMPLIB))
LDTMPOBJ	= $(mktmp,,$(DIVFILE) $(LDOBJS:s,/,\\,:t"+\n")\n)
LDTMPLIB	= $(mktmp,,$(DIVFILE) $(LDLIBS:s,/,\\,:t"+\n")\n)

# Debugging information for Turbo-C
DB_CFLAGS  += -v
DB_LDFLAGS += /v

# See if we modify anything in the lower levels.
.IF $(OSENVIRONMENT) != $(NULL)
   .INCLUDE .IGNORE : $(osrdir)$(DIRSEPSTR)$(OSENVIRONMENT)$(DIRSEPSTR)config.mk
.END

.SETDIR=$(osrdir) : $(OSR_SRC)
.SETDIR=msdos      : $(DOS_SRC)

S_s = -dmsmall
S_m = -dmmedium
S_c = -dmcompact
S_l = -dmlarge
