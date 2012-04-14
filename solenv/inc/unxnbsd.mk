#
# mk file for NetBSD
#

# arch specific defines
.IF "$(CPUNAME)" == "INTEL"
CDEFS+=-DX86
.ENDIF

.IF "$(CPUNAME)" == "X86_64"
CDEFS+=-DX86_64
BUILD64=1
.ENDIF

.INCLUDE : unxgcc.mk

# default linker flags
# NetBSD has no support for using relative paths with $ORIGIN
LINKFLAGSRUNPATH_UREBIN=
LINKFLAGSRUNPATH_OOO=
LINKFLAGSRUNPATH_SDKBIN=

LINKFLAGSDEFS=-Wl,--ignore-unresolved-symbol,environ
LINKFLAGS=-Wl,-z,combreloc $(LINKFLAGSDEFS)
