# This is the Sys VR3 PWD configuration file.  It configures SysvR3 unix
# versions of dmake to use a provided version of getcwd rather than the
# standard library version that uses popen to capture the output of pwd.
#

osredir := $(OS)$(DIRSEPSTR)$(OSRELEASE)$(DIRSEPSTR)$(OSENVIRONMENT)

# The following are required sources
OSRESRC := getcwd.c

.IF $(OSRESRC)
   SRC    += $(OSRESRC)
   .SETDIR=$(osredir) : $(OSRESRC)
.END

.SOURCE.h : $(osredir)

# Local configuration modifications for CFLAGS, there's local SysV includes
# too.
CFLAGS += -I$(osredir)
