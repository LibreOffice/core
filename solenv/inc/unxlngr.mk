#*************************************************************************
#
# unxlngr.mk for armv4l
# Tak-Shing Chan <chan@aleph1.co.uk>
#
# Derived from unxlngi3.mk by obo
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#*************************************************************************

# mk file for unxlngr
ASM=
AFLAGS=

SOLAR_JAVA=
JAVAFLAGSDEBUG=-g

# filter for supressing verbose messages from linker
LINKOUTPUT_FILTER=" |& $(SOLARENV)$/bin$/msg_filter"

# _PTHREADS is needed for the stl
CDEFS+=-DGLIBC=2 -DARM32 -D_PTHREADS -D_REENTRANT -DNEW_SOLAR -D_USE_NAMESPACE=1 -DSTLPORT_VERSION=400

# this is a platform with JAVA support
.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
.IF "$(debug)"==""
JAVA_RUNTIME=-ljava
.ELSE
JAVA_RUNTIME=-ljava_g
.ENDIF
.ENDIF 

# name of C++ Compiler
CC=g++
# name of C Compiler
cc=gcc
# flags for C and C++ Compiler
# do not use standard header search paths
# here the Compiler is installed in the solenv hierarchy, to be changed
# if installed elsewhere
CFLAGS=-w -nostdinc -c $(INCLUDE) -I$(SOLARENV)/unxlngr/lib/gcc-lib/arm-linux/2.95.2/include -I$(SOLARENV)/unxlngr/usr/include
# flags for the C++ Compiler
CFLAGSCC=

# Flags for enabling exception handling
CFLAGSEXCEPTIONS=-fexceptions
# Flags for disabling exception handling
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

# -fpermissive should be removed as soon as possible
CFLAGSCXX= -fno-for-scope -fpermissive

# Compiler flags for compiling static object in single threaded environment with graphical user interface
CFLAGSOBJGUIST=
# Compiler flags for compiling static object in single threaded environment with character user interface
CFLAGSOBJCUIST=
# Compiler flags for compiling static object in multi threaded environment with graphical user interface
CFLAGSOBJGUIMT=
# Compiler flags for compiling static object in multi threaded environment with character user interface
CFLAGSOBJCUIMT=
# Compiler flags for compiling shared object in multi threaded environment with graphical user interface
CFLAGSSLOGUIMT=-fpic
# Compiler flags for compiling shared object in multi threaded environment with character user interface
CFLAGSSLOCUIMT=-fpic
# Compiler flags for profiling
CFLAGSPROF=
# Compiler flags for debugging
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
# Compiler flags for enabling optimazations
CFLAGSOPT=-O2
# Compiler flags for disabling optimazations
CFLAGSNOOPT=-O
# Compiler flags for discibing the output path
CFLAGSOUTOBJ=-o

# switches for dynamic and static linking
STATIC		= -Wl,-Bstatic
DYNAMIC		= -Wl,-Bdynamic

# name of linker
LINK=gcc
# default linker flags
LINKFLAGS=

# linker flags for linking applications
LINKFLAGSAPPGUI= -Wl,-export-dynamic 
LINKFLAGSAPPCUI= -Wl,-export-dynamic
# linker flags for linking shared libraries
LINKFLAGSSHLGUI= -shared
LINKFLAGSSHLCUI= -shared

LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

.IF "$(NO_BSYMBOLIC)"==""
.IF "$(PRJNAME)" != "envtest"
LINKFLAGSSHLGUI+=-Wl,-Bsymbolic
LINKFLAGSSHLCUI+=-Wl,-Bsymbolic
.ENDIF
.ENDIF				# "$(NO_BSYMBOLIC)"==""

LINKVERSIONMAPFLAG=-Wl,--version-script

SONAME_SWITCH=-Wl,-h

# Sequence of libs does matter !

STDLIBCPP=-lstdc++

# default objectfilenames to link
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

# libraries for linking applications
STDLIBCUIST=-ldl -lm -lstlport_gcc
STDLIBGUIMT=-ldl -lpthread -lm -lstlport_gcc
STDLIBCUIMT=-ldl -lpthread -lm -lstlport_gcc
STDLIBGUIST=-lXaw -lXt -lX11 -ldl -lm
# libraries for linking shared libraries
STDSHLGUIMT=-lXaw -lXt -lX11 -lXext -ldl -lpthread -lm -lstlport_gcc
STDSHLCUIMT=-ldl -lpthread -lm -lstlport_gcc

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
DLLPOSTFIX=lr
DLLPRE=lib
DLLPOST=.so

