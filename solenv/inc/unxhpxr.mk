
# mak file fuer unxhpxr
ASM=
AFLAGS=

CDEFS+=-D_PTHREADS -D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE -DNO_AUDIO -DSYSV -D_POSIX_PTHREAD_SEMANTICS  -D_HPUX_SOURCE -DRWSTD_MULTI_THREAD -D_REENTRANT -D__HPACC_NOEH -D__STL_NO_EXCEPTIONS

# kann c++ was c braucht??

# obernervige Warnungen abschalten
# 67  = invalid pragma name
# 251 = An object cannot be deleted using a pointer of type 'void *' since the 
#		type of the object allocated is unknown.
# 370 = The friend declaration is not in the correct form for either a function or a class.
#       (future error)	
# 600 = Type specifier is omitted; "int" is no longer assumed.
DISWARN=+W67,251,370,600

#CC=/opt/aCC/bin/aCC
#cc=/opt/ansic/bin/cc
CC=aCC
cc=cc
CFLAGS= -c $(INCLUDE)
CFLAGSCC= -Aa +e +Z
CFLAGSCXX=$(DISWARN) +Z
CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=
CFLAGSSLOCUIMT=
CFLAGSPROF=
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
CFLAGSOPT=-O
CFLAGSNOOPT=
CFLAGSOUTOBJ=-o


THREADLIB=-L/opt/dce/lib -ldce
# use native aCC to build or link with shared libs, for the sake of
# global class-instances
LINK=/opt/aCC/bin/aCC
LINKFLAGS=
LINKFLAGSAPPGUI=-Wl,+s
LINKFLAGSSHLGUI=-b -L/usr/solar/lib -L/usr/contrib/X11R6/lib 
LINKFLAGSAPPCUI=-Wl,+s
LINKFLAGSSHLCUI=-b -L/usr/contrib/X11R6/lib
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=
LINKFLAGSOPT=

# reihenfolge der libs NICHT egal!
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
STDLIBGUIST= -lX11 -lXt -lXmu -lXext -lsec -lcxx -ldld -lm
STDLIBCUIST= -lsec -lcxx -ldld -lm
#STDLIBGUIMT= -lX11 -lXt -lXmu -lXext -lcma -lsec -lcxx -ldld -lc_r -lm
STDLIBGUIMT= -lX11 -lXt -lXmu -lXext -lpthread
STDLIBCUIMT= -lcma
#STDSHLGUIMT= -lX11 -lXt -lXmu -lXext -lcma -lsec -lcxx -ldld -lc_r
STDSHLCUIMT=
# -L/opt/dce/lib -ldce -lsec -lcxx

LIBMGR=ar
LIBFLAGS=-r
# LIBEXT=.so

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=
RCFLAGS=
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=hr
DLLPRE=lib
DLLPOST=.sl

LDUMP=c++filt

.IF "$(WORK_STAMP)"!="MIX364"
DLLPOSTFIX=ss
.ELSE
DLLPOSTFIX=
.ENDIF


.IF "$(PRJNAME)"=="uno" || "$(PRJNAME)"=="vos"
#CFLAGS += -D_PTHREADS  -U_REENTRANT -U_POSIX_PTHREAD_SEMANTICS
#CFLAGSSLO += -instances=extern -template=wholeclass -ptr$(SLO)
#CFLAGSOBJ += -instances=global -template=wholeclass -ptr$(SLO)
#LINKFLAGS+= -temp=/tmp -PIC -instances=extern -ptr$(SLO)
.ENDIF

#SHLLINKARCONLY=yes

