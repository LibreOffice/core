
# Makefile for linux-ppc 
# Christer Gustavsson <cg@nocrew.org> 
ASM= 
AFLAGS= 

SOLAR_JAVA=TRUE 
JAVAFLAGSDEBUG=-g 

LINKOUTPUT_FILTER=" |& $(SOLARENV)$/bin$/msg_filter" 

# _PTHREADS is needed for the stl 
CDEFS+=-DGLIBC=2 -D_PTHREADS -D_REENTRANT -DNEW_SOLAR -D_USE_NAMESPACE=1
-DSTLPORT_VERSION=321 -DPOWERPC -DPPC 

.IF "$(SOLAR_JAVA)"!="" 
JAVADEF=-DSOLAR_JAVA 
.IF "$(debug)"=="" 
JAVA_RUNTIME=-ljava 
.ELSE 
JAVA_RUNTIME=-ljava_g 
.ENDIF 
.ENDIF 

CC=g++ 
cc=gcc 
CFLAGS=-w -nostdinc -c $(INCLUDE) -I$(SOLARENV)/unxlngppc/usr/include 
CFLAGSCC= 

CFLAGSEXCEPTIONS=-fexceptions 
CFLAGS_NO_EXCEPTIONS=-fno-exceptions 

# -fpermissive should be removed as soon as possible 
CFLAGSCXX= -pipe -fno-for-scope -fpermissive 


CFLAGSOBJGUIST= 
CFLAGSOBJCUIST= 
CFLAGSOBJGUIMT= 
CFLAGSOBJCUIMT= 
CFLAGSSLOGUIMT=-fpic 
CFLAGSSLOCUIMT=-fpic 
CFLAGSPROF= 
CFLAGSDEBUG=-g 
CFLAGSDBGUTIL= 
CFLAGSOPT=-O2 
CFLAGSNOOPT=-O 
CFLAGSOUTOBJ=-o 

STATIC = -Wl,-Bstatic 
DYNAMIC = -Wl,-Bdynamic 

LINK=gcc 
LINKFLAGS= 

LINKFLAGSAPPGUI= -Wl,-export-dynamic 
LINKFLAGSAPPCUI= -Wl,-export-dynamic 
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

# Reihenfolge der libs NICHT egal! 

STDLIBCPP=-lstdc++ 

STDOBJGUI= 
STDSLOGUI= 
STDOBJCUI= 
STDSLOCUI= 

STDLIBCUIST=-ldl -lm 
STDLIBGUIMT=-ldl -lpthread -lm 
STDLIBCUIMT=-ldl -lpthread -lm 
STDLIBGUIST=-lXaw -lXt -lX11 -ldl -lm 
STDSHLGUIMT=-lXaw -lXt -lX11 -lXext -ldl -lpthread -lm 
STDSHLCUIMT=-ldl -lpthread -lm 

LIBMGR=ar 
LIBFLAGS=-r 

IMPLIB= 
IMPLIBFLAGS= 

MAPSYM= 
MAPSYMFLAGS= 

RC=irc 
RCFLAGS=-fo$@ $(RCFILES) 
RCLINK= 
RCLINKFLAGS= 
RCSETVERSION= 

DLLPOSTFIX=lp
DLLPRE=lib 
DLLPOST=.so 

