# This is an OS specific configuration file
#	It assumes that OBJDIR, TARGET and DEBUG are previously defined.
#	It defines	CFLAGS, LDARGS, CPPFLAGS, STARTUPFILE, LDOBJS
#	It augments	SRC, OBJDIR, TARGET, CFLAGS, LDLIBS
#
OSRELEASE *= ibm
- := $(SWITCHAR)

# Memory model to compile for
# set to s - small, m - medium, c - compact, l - large
# Use only large model now.
MODEL = l

STARTUPFILE	= $(OS)/startup.mk

CPPFLAGS 	= $(CFLAGS)
LDOBJS		= $(CSTARTUP) $(OBJDIR)/{$(<:f)}
LDARGS		= $(LDHEAD) @$(LDTMPOBJ),$(TARGET),NUL.MAP,,$(LDTAIL)
LDTAIL		= $(_libs)$(LDFLAGS:s/ //)
_libs           = $(!null,$(LDLIBS) ,@$(LDTMPLIB))
LDTMPOBJ	= $(mktmp,,$(DIVFILE) $(LDOBJS:s,/,\\,:t"+\n")\n)
LDTMPLIB	= $(mktmp,,$(DIVFILE) $(LDLIBS)\n)

# Debug flags
DB_CFLAGS	= -DDBUG
DB_LDFLAGS	=
DB_LDLIBS	=

# NO Debug flags
NDB_CFLAGS	= 
NDB_LDFLAGS	=
NDB_LDLIBS      =

# Local configuration modifications for CFLAGS.
CFLAGS         += $-I$(OS)

# OS2 does not have a swap version. The operating system will
# handle all swapping.
# To save copying unchanged files in from elsewhere, I shall use them in situ.
OS_SRC   += ruletab.c dchdir.c switchar.c
DOS_SRC  += dirbrk.c arlib.c dstrlwr.c runargv.c rmprq.c

SRC += $(OS_SRC) $(DOS_SRC)
.SETDIR=$(OS) : $(ASRC) $(OS_SRC)
.SETDIR=msdos : $(DOS_SRC)

# Set source dirs so that we can find files named in this
# config file.
.SOURCE.h : $(OS)

# See if we modify anything in the lower levels.
.IF $(OSRELEASE) != $(NULL)
   .INCLUDE .IGNORE : $(OS)$(DIRSEPSTR)$(OSRELEASE)$(DIRSEPSTR)config.mk
.END
