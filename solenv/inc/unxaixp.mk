
# mak file fuer unxaixp
ASM=
AFLAGS=

# kann c++ was c braucht??

CC=				xlC_r
cc=				xlc_r
CFLAGS=			-c -qlanglvl=extended -qchars=signed 
CFLAGS+=		$(INCLUDE)
CDEFS+=			-DVCL -D_PTHREADS
CDEFS+=			-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
CFLAGSCC=
CFLAGSCXX=
CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=
CFLAGSSLOCUIMT=
CFLAGSPROF=
CFLAGSDEBUG=
CFLAGSDBGUTIL=
CFLAGSOPT=		-O
CFLAGSNOOPT=
CFLAGSOUTOBJ=	-o

STATIC= 		# -Bstatic
DYNAMIC= 		# -Bdynamic

#
# Zu den Linkflags von shared Libraries:
#
# -G			Erzeugen eines Shared-Objects
# -brtl			aktiviert Runtime-Linking, wird von -G mitgesetzt
# -bsymbolc		loest Symbole innerhalb einer Library zur Linkzeit auf
# -p0			??? (ENOMANPAGESINSTALLABLEFROMCD)
# -bnoquiet		verbose
#
LINK=
LINKFLAGS=
LINKFLAGSAPPCUI=	xlC_r -qlanglvl=extended -qchars=signed -brtl -bnolibpath
LINKFLAGSAPPGUI=	xlC_r -qlanglvl=extended -qchars=signed -brtl -bnolibpath
LINKFLAGSSHLCUI=	makeC++SharedLib_r -G -bsymbolic -bdynamic -bnolibpath -p0
LINKFLAGSSHLGUI=	makeC++SharedLib_r -G -bsymbolic -bdynamic -bnolibpath -p0
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=
LINKFLAGSOPT=

# Reihenfolge der libs NICHT egal!
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
STDLIBGUIST=
STDLIBCUIST=
# application
STDLIBGUIMT=	# -lX11 -ldl
STDLIBCUIMT=	# -lX11 -ldl
# shared library
STDSHLGUIMT=	-lX11 -ldl
STDSHLCUIMT=	-lX11 -ldl
THREADLIB=

LIBMGR=			ar
LIBFLAGS=		-r
# LIBEXT=		.so

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=		-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=		ap
.IF "$(WORK_STAMP)"=="MIX364"
DLLPOSTFIX=
.ENDIF
DLLPRE=			lib
DLLPOST=		.so


LDUMP=			cppfilt /b /n /o /p

