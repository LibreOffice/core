
# mak file fuer wnticci
ASM=
AFLAGS=

CDEFS+=-D_PTHREADS -D_REENTRANT 
CDEFS+=-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE -DX86 -DNEW_SOLAR
# kann c++ was c braucht??

CC=/nw386/dev/s/solenv/unxscoi/bin/g++
cc=/nw386/dev/s/solenv/unxscoi/bin/gcc
CFLAGS=-w -c $(INCLUDE)
CFLAGSCC=-mpentium
CFLAGSCXX= -mpentium
CFLAGSEXCEPTIONS=-fexceptions
CFLAGS_NO_EXCEPTIONS=-fno-exceptions
CFLAGSCXX=-mpentium -fguiding-decls -frtti

CFLAGSOBJGUIST=-fPIC
CFLAGSOBJCUIST=-fPIC
CFLAGSOBJGUIMT=-fPIC
CFLAGSOBJCUIMT=-fPIC
CFLAGSSLOGUIMT=-fPIC
CFLAGSSLOCUIMT=-fPIC
CFLAGSPROF=
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
CFLAGSOPT=-O2
CFLAGSNOOPT=-O
CFLAGSOUTOBJ=-o

STATIC		= -Wl,-Bstatic
DYNAMIC		= -Wl,-Bdynamic

THREADLIB=
LINK=/nw386/dev/s/solenv/unxscoi/bin/gcc
LINKFLAGS=
# SCO hat grosse Probleme mit fork/exec und einigen shared libraries
# rsc2 muss daher statisch gelinkt werden
.IF "$(PRJNAME)"=="rsc"
LINKFLAGSAPPGUI=-L/nw386/dev/s/solenv/unxscoi/lib $(STATIC) -lpthread_init $(DYNAMIC)
LINKFLAGSAPPCUI=-L/nw386/dev/s/solenv/unxscoi/lib $(STATIC) -lpthread_init $(DYNAMIC)
.ELSE
LINKFLAGSAPPGUI=-L/nw386/dev/s/solenv/unxscoi/lib -lpthread_init
LINKFLAGSAPPCUI=-L/nw386/dev/s/solenv/unxscoi/lib -lpthread_init
.ENDIF
LINKFLAGSSHLGUI=-G -W,l,-Bsymbolic
LINKFLAGSSHLCUI=-G -W,l,-Bsymbolic
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

# standard C++ Library
#
# das statische dazulinken der libstdc++ macht jede shared library um 50k
# (ungestrippt) oder so groesser, auch wenn sie ueberhaupt nicht gebraucht
# wird. Da muessen wir uns was besseres ueberlegen.
STDLIBCPP=-Wl,-Bstatic -lstdc++ -Wl,-Bdynamic

# reihenfolge der libs NICHT egal!
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
STDLIBGUIST=-lXt -lXext -lXmu -lX11 -ldl -lsocket -lm
STDLIBCUIST=-ldl -lsocket -lm
.IF "$(PRJNAME)"=="rsc"
STDLIBGUIMT=-lXt -lXext -lXmu -lX11 $(STATIC) -lpthread $(DYNAMIC) -ldl -lsocket -lm
STDLIBCUIMT=$(STATIC) -lpthread $(DYNAMIC) -ldl -lsocket -lm 
.ELSE
STDLIBGUIMT=-lXt -lXext -lXmu -lX11 -lpthread -ldl -lsocket -lm
STDLIBCUIMT=-lpthread -ldl -lsocket -lm 
.ENDIF
#STDSHLGUIMT=-lXt -lXext -lXmu -lX11 -lpthread -ldl -lsocket -lm
#STDSHLCUIMT=-lpthread -ldl -lsocket -lm 

STDLIBCPP= -lstdc++
SHLLINKARCONLY=yes

LIBMGR=ar
LIBFLAGS=-r
# LIBEXT=.so

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=ci
DLLPRE=lib
DLLPOST=.so

LDUMP=cppfilt /b /n /o /p

.IF "$(WORK_STAMP)"!="MIX364"
DLLPOSTFIX=ci
.ELSE
DLLPOSTFIX=
.ENDIF
