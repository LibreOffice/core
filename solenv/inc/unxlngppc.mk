
# Makefile for linux-ppc 
# Christer Gustavsson <cg@nocrew.org> 
ASM= 
AFLAGS= 

SOLAR_JAVA=TRUE 
JAVAFLAGSDEBUG=-g 

# filter for supressing verbose messages from linker
#not needed at the moment
#LINKOUTPUT_FILTER=" |& $(SOLARENV)$/bin$/msg_filter"

# _PTHREADS is needed for the stl 
CDEFS+=-DGLIBC=2 -D_PTHREADS -D_REENTRANT -DNEW_SOLAR -D_USE_NAMESPACE=1 -DSTLPORT_VERSION=400 -DPOWERPC -DPPC 

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

# source code is still not signed versus unsigned char clean 
CFLAGS=-fsigned-char -w -nostdinc -c $(INCLUDE) -I$(SOLARENV)/unxlngppc/usr/include 
CFLAGSCC=-fsigned-char

# Flags for enabling exception handling
CFLAGSEXCEPTIONS=-fexceptions 
# Flags for disabling exception handling
CFLAGS_NO_EXCEPTIONS=-fno-exceptions 

# -fpermissive should be removed as soon as possible 
CFLAGSCXX= -pipe -fno-for-scope -fpermissive 


#Note: the build is not consistent in that it links static librtaries
# libraries into dynamic libraries in places, so use -fPIC throughout
# until fixed.

# Compiler flags for compiling static object in single threaded environment with graphical user interface
CFLAGSOBJGUIST=-fPIC 
# Compiler flags for compiling static object in single threaded environment with character user interface
CFLAGSOBJCUIST=-fPIC
# Compiler flags for compiling static object in multi threaded environment with graphical user interface
CFLAGSOBJGUIMT=-fPIC
# Compiler flags for compiling static object in multi threaded environment with character user interface
CFLAGSOBJCUIMT=-fPIC
# Compiler flags for compiling shared object in multi threaded environment with graphical user interface
CFLAGSSLOGUIMT=-fPIC
# Compiler flags for compiling shared object in multi threaded environment with character user interface
CFLAGSSLOCUIMT=-fPIC 

# Compiler flags for profiling
CFLAGSPROF= 

# Compiler flags for debugging
CFLAGSDEBUG=-g 
CFLAGSDBGUTIL= 

# Compiler flags for enabling optimazations
CFLAGSOPT=-O1 
# Compiler flags for disabling optimazations
CFLAGSNOOPT=-O0

# Compiler flags for discibing the output path
CFLAGSOUTOBJ=-o 

# switches for dynamic and static linking
STATIC = -Wl,-Bstatic 
DYNAMIC = -Wl,-Bdynamic 

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
.ENDIF # "$(NO_BSYMBOLIC)"=="" 

LINKVERSIONMAPFLAG=-Wl,--version-script 

# Sequence of libs does matter !

STDLIBCPP=-lstdc++ 

# default objectfilenames to link
STDOBJGUI= 
STDSLOGUI= 
STDOBJCUI= 
STDSLOCUI= 

# libraries for linking applications
STDLIBCUIST=-ldl -lm
STDLIBGUIMT=-lXaw -lXt -lX11 -ldl -lpthread -lm
STDLIBCUIMT=-ldl -lpthread -lm
STDLIBGUIST=-lXaw -lXt -lX11 -ldl -lm
# libraries for linking shared libraries 
STDSHLGUIMT=-lXaw -lXt -lX11 -lXext -ldl -lpthread -lm
STDSHLCUIMT=-ldl -lpthread -lm

LIBSTLPORT=$(DYNAMIC) -lstlport_gcc
LIBSTLPORTST=$(STATIC) -lstlport_gcc $(DYNAMIC)

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
DLLPOSTFIX=lp
DLLPRE=lib 
DLLPOST=.so 

