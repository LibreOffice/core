# Definition of macros for library, and C startup code.
osedir    = $(osrdir)$(DIRSEPSTR)$(OSENVIRONMENT)

LDLIBS	  = e:/cc/borland/bcc40/lib/c$(MODEL)
CSTARTUP  = e:/cc/borland/bcc40/lib/c0$(MODEL).obj

CFLAGS   += -I$(osedir) -w-pro
