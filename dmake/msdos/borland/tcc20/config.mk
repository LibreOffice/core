# Definition of macros for library, and C startup code.
osedir    = $(osrdir)$(DIRSEPSTR)$(OSENVIRONMENT)

LDLIBS	  = e:/cc/borland/tcc20/lib/c$(MODEL)
CSTARTUP  = e:/cc/borland/tcc20/lib/c0$(MODEL).obj

CFLAGS   += -I$(osedir) -f-

# Case of identifiers is significant
NDB_LDFLAGS += -c
