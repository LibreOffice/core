# mak file fuer unxbsdi

ASM=
AFLAGS=

cc=gcc -c
CC=g++ -c
CDEFS+=-D_PTHREADS -D_REENTRANT
CDEFS+=-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
CDEFS+=-DNO_INET_ON_DEMAND -DX86 -DNEW_SOLAR -DNCIfeature
CFLAGS+=-w -c $(INCLUDE)
CFLAGSCC=-pipe -mpentium
CFLAGSEXCEPTIONS=-fexceptions
CFLAGS_NO_EXCEPTIONS=-fno-exceptions
CFLAGSCXX=-pipe -mpentium -fguiding-decls -frtti

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
LINKFLAGSSHLGUI= -shared -nostdlib
LINKFLAGSSHLCUI= -shared -nostdlib
.ELSE
LINKFLAGSSHLGUI= -shared -nostdlib /usr/lib/c++rt0.o
LINKFLAGSSHLCUI= -shared -nostdlib /usr/lib/c++rt0.o
.ENDIF
LINKFLAGSAPPGUI= -L/nw386/dev/s/solenv/unxbsdi/lib -lpthread_init -lpthread
LINKFLAGSAPPCUI= -L/nw386/dev/s/solenv/unxbsdi/lib -lpthread_init -lpthread
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=
LINKFLAGSOPT=

_SYSLIBS= -lpthread -lgcc -lc -lm 
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

DLLPOSTFIX=		bi
DLLPRE=			lib
DLLPOST=		.so.1.0

LDUMP=

