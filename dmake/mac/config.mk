# This is an OS Mac specific configuration file
#   It assumes that OBJDIR, TARGET and DEBUG are previously defined.
#   It defines  CFLAGS, LDARGS, CPPFLAGS, STARTUPFILE, LDOBJS
#   It augments SRC, OBJDIR, TARGET, CFLAGS, LDLIBS
#

STARTUPFILE = :$(OS):startup.mk

CPPFLAGS    = $(CFLAGS)
LDOBJS      = $(CSTARTUP) :$(OBJDIR):{$(<:f)}
LDARGS      = $(LDFLAGS) -o $@ $(LDOBJS) $(LDLIBS)

# Debug flags
DB_CFLAGS   = -sym on 
DB_LDFLAGS  = -sym on
DB_LDLIBS   =

# NO Debug flags
NDB_CFLAGS  = -sym off
NDB_LDFLAGS = -sym off
NDB_LDLIBS  =

# Local configuration modifications for CFLAGS.
CFLAGS     += -I :$(OS) -d _MPW -s $(<:b)
LDFLAGS    += -w -c 'MPS ' -t MPST

# Since we writing out what files we want to execute, we can't use .SETDIR
# to specify the files to compile in the Mac directory.
# Instead, we copy the files to the (top-level) current directory and compile
# them there.
%.c : ":$(OS):%.c"
    duplicate -y $< $@

# Common Mac source files.
OS_SRC = arlib.c bogus.c dirbrk.c directry.c environ.c main.c rmprq.c \
         ruletab.c tempnam.c tomacfil.c
.IF $(SHELL) != mwp
   .SETDIR=$(OS) : $(OS_SRC)
.ENDIF
SRC += $(OS_SRC)

# Set source dirs so that we can find files named in this
# config file.
.SOURCE.h : $(OS)
