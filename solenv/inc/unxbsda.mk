
# mak file fuer unxbsda

ASM=
AFLAGS=

cc=				gcc -c
CC=				g++ -c
CDEFS+=			-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
CDEFS+=			-D_REENTRANT 		# -D_PTHREADS -DPOSIX_PTHREAD_SEMANTICS
CDEFS+= 		-DNO_AUDIO -DPRINTER_DUMMY
CFLAGS+=		$(INCLUDE)
CFLAGSCC=		-fsigned-char
CFLAGSCXX=		-fsigned-char
CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=	-fPIC
CFLAGSSLOCUIMT=	-fPIC
CFLAGSPROF=
CFLAGSDEBUG=	-g
CFLAGSDBGUTIL=
CFLAGSOPT=		-O2
CFLAGSNOOPT=
CFLAGSOUTOBJ=	-o

STATIC=			-Bstatic
DYNAMIC=		-Bdynamic

THREADLIB=		pthread
LINK=			ld -e start -dc -dp
LINKFLAGS=
LINKFLAGSAPPGUI=
LINKFLAGSSHLGUI=
LINKFLAGSAPPCUI=
LINKFLAGSSHLCUI=
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=
LINKFLAGSOPT=

STDOBJGUI=		/usr/lib/crt0.o
STDSLOGUI=		/usr/lib/crt0.o
STDOBJCUI=		/usr/lib/crt0.o
STDSLOCUI=		/usr/lib/crt0.o
STDLIBGUIST=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc
STDLIBCUIST=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc
STDLIBGUIMT=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc
STDLIBCUIMT=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc
STDSHLGUIMT=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc
STDSHLCUIMT=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc

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

DLLPOSTFIX=		ba
DLLPRE=			lib
DLLPOST=		.so

LDUMP=

