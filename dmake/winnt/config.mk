# This is an OS specific configuration file
#	It assumes that OBJDIR, TARGET and DEBUG are previously defined.
#	It defines	CFLAGS, LDARGS, CPPFLAGS, STARTUPFILE, LDOBJS
#	It augments	SRC, OBJDIR, TARGET, CFLAGS, LDLIBS
#

# Memory model to compile for
# set to s - small, m - medium, c - compact, l - large
# Need large model now, dmake has grown up :-)
MODEL = l

STARTUPFILE	= startup/startup.mk

CPPFLAGS 	= $(CFLAGS)

# Debug flags
DB_CFLAGS	= -DDBUG
DB_LDFLAGS	=
DB_LDLIBS	=

# NO Debug flags
NDB_CFLAGS	=
NDB_LDFLAGS	=
NDB_LDLIBS	=

# Local configuration modifications for CFLAGS.
CFLAGS += -I$(OS)

# Common Win32 source files.
OS_SRC  += dchdir.c

# Imported Win95 files.
WIN95SRC += switchar.c

# Imported MSDOS Files.
DOSSRC  += dstrlwr.c arlib.c dirbrk.c

SRC += $(OS_SRC) $(UNIXSRC) $(DOSSRC)

# Provide our own %$O : %$S rule.
%$O : %$S
    +$(AS) $(ASFLAGS) \
    $(<:s,\,${__.DIVSEP-sh-${USESHELL}},:s,/,${__.DIVSEP-sh-${USESHELL}},);
    mv $(@:f) $(OBJDIR)

# Set source dirs so that we can find files named in this
# config file.
.SOURCE.h : $(OS)

# See if we modify anything in the lower levels.
.IF $(OSRELEASE) != $(NULL)
   .INCLUDE : $(OS)$(DIRSEPSTR)$(OSRELEASE)$(DIRSEPSTR)config.mk
.END

.SETDIR=msdos : $(DOSSRC)
.SETDIR=win95 : $(WIN95SRC)
.SETDIR=$(OS) : $(ASRC) $(OS_SRC)
