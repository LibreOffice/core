# This is an OS specific configuration file
#	It assumes that OBJDIR, TARGET and DEBUG are previously defined.
#	It defines	CFLAGS, LDARGS, CPPFLAGS, STARTUPFILE, LDOBJS
#	It augments	SRC, OBJDIR, TARGET, CFLAGS, LDLIBS
#

# Memory model to compile for
# set to s - small, m - medium, c - compact, l - large
# Need large model now, dmake has grown up :-)
MODEL = l

STARTUPFILE	= $(OS)/startup.mk

CPPFLAGS 	= $(CFLAGS)
LDOBJS		= $(CSTARTUP) $(OBJDIR)/{$(<:f)}
LDARGS		= $(LDHEAD) $(LDFLAGS:s/ //) @$(LDTMPOBJ),$(TARGET),NUL.MAP$(LDTAIL)
LDTAIL		= $(_libs)
_libs           = $(!null,$(LDLIBS) ,@$(LDTMPLIB))
LDTMPOBJ	= $(mktmp,,$(DIVFILE) $(LDOBJS:s,/,\\,:t"+\n")\n)
LDTMPLIB	= $(mktmp,,$(DIVFILE) $(LDLIBS:s,/,\\,:t"+\n")\n)

# Debug flags
DB_CFLAGS	= -DDBUG
DB_LDFLAGS	=
DB_LDLIBS	=

# NO Debug flags
NDB_CFLAGS	=
NDB_LDFLAGS	=
NDB_LDLIBS	=

# Local configuration modifications for CFLAGS.
CFLAGS         += -I$(OS)

# Common MSDOS source files.
# Define SWAP to anything but 'y' for the swap code to be excluded on making.
# Swapping for DOS versions is enabled by default.
# Note: swapping is handled specially for ZTC in msdos/zortech/config.mk.
SWAP *= y

.IF $(OSRELEASE) != zortech
   .IF $(SWAP) == y
      SWP_SRC += spawn.c
      ASRC += exec.asm
   .ELSE
      SWP_SRC += tee.c
   .END
.ELSE
   SWP_SRC += tee.c
.END

OS_SRC += ruletab.c dirbrk.c runargv.c arlib.c dchdir.c switchar.c rmprq.c\
        $(SWP_SRC) find.c dirlib.c dstrlwr.c
UNIXSRC := dcache.c
SRC += $(OS_SRC) $(UNIXSRC)
.SETDIR=$(OS) : $(ASRC) $(OS_SRC)
.SETDIR=unix  : $(UNIXSRC)

# Provide our own %$O : %$S rule.
%$O : %$S
    +$(AS) $(ASFLAGS) $(<:s,\,${__.DIVSEP-sh-${USESHELL}},:s,/,${__.DIVSEP-sh-${USESHELL}},),,,;
    mv $(@:f) $(OBJDIR)

# Set source dirs so that we can find files named in this
# config file.
.SOURCE.h : $(OS)

# See if we modify anything in the lower levels.
.IF $(OSRELEASE) != $(NULL)
   .INCLUDE : $(OS)$(DIRSEPSTR)$(OSRELEASE)$(DIRSEPSTR)config.mk
.END
