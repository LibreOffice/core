#
# mk file for NetBSD/amd64 with gcc 3.4.x and higher.
#

ASM=
AFLAGS=

SOLAR_JAVA*=
JAVAFLAGSDEBUG=-g

# filter for supressing verbose messages from linker
#not needed at the moment
#LINKOUTPUT_FILTER=" |& $(SOLARENV)$/bin$/msg_filter"

# _PTHREADS is needed for the stl
CDEFS+= -DX86_64 $(PTHREAD_CFLAGS) -D_PTHREADS -D_REENTRANT -DNEW_SOLAR -D_USE_NAMESPACE=1

# enable visibility define in "sal/types.h"
.IF "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"
CDEFS += -DHAVE_GCC_VISIBILITY_FEATURE
.ENDIF # "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"

# this is a platform with JAVA support
.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
.IF "$(debug)"==""
JAVA_RUNTIME=-ljava
.ELSE
JAVA_RUNTIME=-ljava_g
.ENDIF
.ENDIF 

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
ARCH_FLAGS*=

# name of C++ Compiler
CXX*=g++
# name of C Compiler
CC*=gcc
# flags for C and C++ Compiler
CFLAGS+=-fmessage-length=0 -c

# Compiler flags for enabling optimizations
.IF "$(PRODUCT)"!=""
CFLAGSOPT=-Os -fno-strict-aliasing             # optimizing for products
.ELSE  # "$(PRODUCT)"!=""
CFLAGSOPT=                                     # no optimizing for non products
.ENDIF # "$(PRODUCT)"!=""

# flags to enable build with symbols; required for crashdump feature
.IF "$(ENABLE_SYMBOLS)"=="SMALL"
CFLAGSENABLESYMBOLS=-g1
.ELSE
CFLAGSENABLESYMBOLS=-g
.ENDIF

# flags for the C++ Compiler
CFLAGSCC= -pipe $(ARCH_FLAGS)
# Flags for enabling exception handling
CFLAGSEXCEPTIONS=-fexceptions -fno-enforce-eh-specs
# Flags for disabling exception handling
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

CFLAGSCXX= -pipe $(ARCH_FLAGS)
PICSWITCH:=-fpic
.IF "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"
CFLAGSCXX += -fvisibility-inlines-hidden
.ENDIF # "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"

# Compiler flags for compiling static object in multi threaded environment with graphical user interface
CFLAGSOBJGUIMT=
# Compiler flags for compiling static object in multi threaded environment with character user interface
CFLAGSOBJCUIMT=
# Compiler flags for compiling shared object in multi threaded environment with graphical user interface
CFLAGSSLOGUIMT=$(PICSWITCH)
# Compiler flags for compiling shared object in multi threaded environment with character user interface
CFLAGSSLOCUIMT=$(PICSWITCH)
# Compiler flags for profiling
CFLAGSPROF=
# Compiler flags for debugging
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
# Compiler flags for enabling optimizations
# CFLAGSOPT=-O2
# reduce to -O1 to avoid optimization problems
CFLAGSOPT=-O1
# Compiler flags for disabling optimizations
CFLAGSNOOPT=-O0
# Compiler flags for describing the output path
CFLAGSOUTOBJ=-o

CFLAGSWARNCC=
CFLAGSWARNCXX=$(CFLAGSWARNCC) -Wno-ctor-dtor-privacy
# -Wshadow does not work for C with nested uses of pthread_cleanup_push:
CFLAGSWALLCC=-Wall -Wextra -Wendif-labels
CFLAGSWALLCXX=$(CFLAGSWALLCC) -Wshadow -Wno-ctor-dtor-privacy
CFLAGSWERRCC=-Werror

# switches for dynamic and static linking
STATIC		= -Wl,-Bstatic
DYNAMIC		= -Wl,-Bdynamic

# name of linker
LINK*=$(CXX)
LINKC*=$(CC)

# default linker flags
LINKFLAGSDEFS*=-Wl,-z,defs
#LINKFLAGSRUNPATH*=-Wl,-rpath,\''$$ORIGIN'\'
LINKFLAGS=-Wl,-z,combreloc $(LINKFLAGSDEFS) $(LINKFLAGSRUNPATH)

# linker flags for linking applications
LINKFLAGSAPPGUI= -Wl,-export-dynamic 
LINKFLAGSAPPCUI= -Wl,-export-dynamic 

# linker flags for linking shared libraries
LINKFLAGSSHLGUI= -shared
LINKFLAGSSHLCUI= -shared

LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=-L$(SOLARVERSION)$/$(INPATH)$/lib

# linker flags for optimization (symbol hashtable)
# for now, applied to symbol scoped libraries, only
LINKFLAGSOPTIMIZE*=-Wl,-O1
LINKVERSIONMAPFLAG=$(LINKFLAGSOPTIMIZE) -Wl,--version-script

SONAME_SWITCH=-Wl,-h

# Sequence of libs does matter !

STDLIBCPP=-lgcc_s -lstdc++

# default objectfilenames to link
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
STDOBJVCL=$(L)$/salmain.o

# libraries for linking applications
STDLIBGUIMT=-lX11 -lm -lc -lgcc_s -lstdc++ -lpthread
STDLIBCUIMT=-lm -lc -lgcc_s -lstdc++ -lpthread
# libraries for linking shared libraries
STDSHLGUIMT=-lX11 -lXext -lm -lc -lgcc_s -lstdc++ -lpthread
STDSHLCUIMT=-lm -lc -lgcc_s -lstdc++ -lpthread

LIBSALCPPRT*=-Wl,--whole-archive -lsalcpprt -Wl,--no-whole-archive

# name of library manager
LIBMGR=ar
LIBFLAGS=-r

# tool for generating import libraries
IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

# platform specific identifier for shared libs
DLLPOSTFIX=bx
DLLPRE=lib
DLLPOST=.so
