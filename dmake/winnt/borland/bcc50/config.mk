# Definition of macros for library, and C startup code.
osedir    = $(osrdir)$(DIRSEPSTR)$(OSENVIRONMENT)

# Change the CCROOT variable to reflect the installation directory of your
# C++ compiler.
CCVER   *:= c:/cc/borland/$(OSENVIRONMENT)

# Definitions for compiles and links
CSTARTUP  = $(CCVER)/lib/c0x32.obj
LDLIBS	  = $(CCVER)/lib/cw32 $(CCVER)/lib/import32

CFLAGS    += -A- -w-pro -I$(osedir)
