# Definition of macros for library, and C startup code.
osedir     = $(osrdir)$(DIRSEPSTR)$(OSENVIRONMENT)

.IMPORT .IGNORE : MSC_VER
MSC_VER      *= 6.0

CFLAGS       += $-I$(osedir) $-Sp1 $-Q $-Fi-

NDB_CFLAGS   += $-O
NDB_LDFLAGS  += $-de $-pmtype:vio $-align:16 $-nologo $-m $-stack:32768
NDB_LDLIBS   +=
