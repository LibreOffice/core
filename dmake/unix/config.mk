# This is an OS specific configuration file
#	It assumes that OBJDIR, TARGET and DEBUG are previously defined.
#	It defines	CFLAGS, LDARGS, CPPFLAGS, STARTUPFILE, LDOBJS
#			PRINTER, PRINTFLAGS
#	It augments	SRC, OBJDIR, TARGET, CFLAGS, LDLIBS
#
PRINTER		= hw
PRINTFLAGS	= -P$(PRINTER)
STARTUPFILE	= $(OS)/startup.mk
CPPFLAGS 	= $(CFLAGS)
LDOBJS		= $(CSTARTUP) $(OBJDIR)/{$(<:f)}
LDARGS		= $(LDFLAGS) -o $@ $(LDOBJS) $(LDLIBS)

# Debug flags
DB_CFLAGS	= -g -DDBUG
DB_LDFLAGS	= -g
DB_LDLIBS	=

# NO Debug flags
NDB_CFLAGS	= -O
NDB_LDFLAGS	= -O
NDB_LDLIBS	=

# Local configuration modifications for CFLAGS.
CFLAGS         += -I$(OS)

# Sources that must be defined for each different version
OSSRC := arlib.c dirbrk.c rmprq.c ruletab.c runargv.c dcache.c
SRC  += $(OSSRC)
.SETDIR=$(OS) : $(OSSRC)

# Set source dirs so that we can find files named in this
# config file.
.SOURCE.h : $(OS)

# See if we modify anything in the lower levels.
.IF $(OSRELEASE) != $(NULL)
   .INCLUDE .IGNORE : $(OS)$(DIRSEPSTR)$(OSRELEASE)$(DIRSEPSTR)config.mk
.END
