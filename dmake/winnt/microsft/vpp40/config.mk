# Definition of macros for library, and C startup code.
osedir        = $(osrdir)$(DIRSEPSTR)$(OSENVIRONMENT)

.IMPORT .IGNORE : MSC_VER
MSC_VER      *= 8.0

CFLAGS       += -I$(osedir)

NDB_CFLAGS   += -Od -GF -Ge
NDB_LDFLAGS  +=
NDB_LDLIBS   +=

# Redefine rule for making our objects, we don't need mv
%$O : %.c ;% $(CC) -c $(CFLAGS) -Fo$@ $<
