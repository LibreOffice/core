# mak file fuer unxbsds

ASM=
AFLAGS=

cc=gcc -c 
CC=g++ -c 
CDEFS+=-D_PTHREADS -D_REENTRANT -DOSL_USE_SYS_V_SEMAPHORE
CDEFS+=-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE -DSTLPORT_VERSION=321
CDEFS+=-DNO_INET_ON_DEMAND -DNEW_SOLAR -DNCIfeature
CFLAGS+=-w -c $(INCLUDE) 
CFLAGSCC=-pipe 
CFLAGSEXCEPTIONS=-fexceptions
CFLAGS_NO_EXCEPTIONS=-fno-exceptions
CFLAGSCXX=-pipe -fguiding-decls -frtti

CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
CFLAGSOBJGUIMT= 
CFLAGSOBJCUIMT= 
CFLAGSSLOGUIMT=	-fPIC
CFLAGSSLOCUIMT=	-fPIC
CFLAGSPROF=     -pg
CFLAGSDEBUG=	-g
CFLAGSDBGUTIL=
# die zusaetzlichen Optimierungsschalter schalten alle Optimierungen ein, die zwischen -O und -O2 liegen und
# per Schalter einschaltbar sind. Dennoch gibt es einen Unterschied: einige Files im Writer werden
# misoptimiert wenn -O2 eingeschaltet ist und waehrend die untenstehenden Schalter funktionieren.
CFLAGSOPT=-O -fcse-follow-jumps -fcse-skip-blocks -fexpensive-optimizations -fstrength-reduce -fforce-mem -fcaller-saves -fgcse -frerun-cse-after-loop -frerun-loop-opt -fschedule-insns2 -fregmove -foptimize-register-move 
#CFLAGSOPT=-O2
CFLAGSNOOPT=-O
CFLAGSOUTOBJ=-o

STATIC=			-Bstatic
DYNAMIC=		-Bdynamic

THREADLIB=
LINK= gcc
LINKFLAGS=
.IF "$(PRJNAME)"=="osl" ||  "$(PRJNAME)"=="rtl" 
LINKFLAGSSHLGUI= -shared -nostdlib -Wl,-shared
LINKFLAGSSHLCUI= -shared -nostdlib -Wl,-shared
.ELSE
LINKFLAGSSHLGUI= -shared -nostdlib -Wl,-shared
LINKFLAGSSHLCUI= -shared -nostdlib -Wl,-shared
.ENDIF
LINKFLAGSAPPGUI= -lpthread
LINKFLAGSAPPCUI= -lpthread
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=
LINKFLAGSOPT= 

_SYSLIBS= -lc -lm 
_X11LIBS= -L/usr/X11R6/lib -lXext -lXt -lX11

STDLIBCPP= -lstdc++

STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

STDLIBGUIST=  ${_X11LIBS} ${_SYSLIBS}
STDLIBCUIST=              ${_SYSLIBS}
STDLIBGUIMT=  ${_X11LIBS} ${_SYSLIBS}
STDLIBCUIMT=              ${_SYSLIBS}
STDSHLGUIMT=
STDSHLCUIMT=

LIBMGR=			ar
LIBFLAGS=		-r
LIBEXT=			.a

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=		-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=		bs
DLLPRE=			lib
DLLPOST=		.so.1.0

LDUMP=


