
ASM=/usr/ccs/bin/as
AFLAGS=-P -q


CDEFS+=-mt -D_PTHREADS -DSYSV -DSUN -DSUN4 -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -DSTLPORT_VERSION=400

.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
JAVAFLAGSDEBUG=-g
.IF "$(debug)"==""
JAVA_RUNTIME=-ljava
.ELSE
JAVA_RUNTIME=-ljava_g
.ENDIF
.ENDIF

CXX=CC
CC=cc

CFLAGS=$(PREENVCFLAGS) -c -temp=/tmp $(INCLUDE) -g0
CFLAGSCC=-xCC
CFLAGSCXX=-features=no%altspell,no%localfor -library=no%Cstd

CFLAGSEXCEPTIONS=
CFLAGS_NO_EXCEPTIONS=-noex

CFLAGSOBJGUIST=-KPIC
CFLAGSOBJCUIST=-KPIC
CFLAGSOBJGUIMT=-KPIC
CFLAGSOBJCUIMT=-KPIC
CFLAGSSLOGUIMT=-KPIC
CFLAGSSLOCUIMT=-KPIC
CFLAGSPROF=-xpg
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
CFLAGSOPT=-xarch=v8 -xO3 -xspace
CFLAGSNOOPT=
CFLAGSOUTOBJ=-o

# Enable all warnings
CFLAGSWALL=+w2
# Set default warn level
CFLAGSDFLTWARN=-w

STDOBJVCL=$(L)$/salmain.o

THREADLIB=
.IF "$(PURIFY)"!=""
LINK=/usr/local/purify-4.2-solaris2/purify CC
.ELSE
LINK=CC
.ENDIF

# -z combreloc combines multiple relocation sections. Reduces overhead on startup
# -norunpath prevents the compiler from recording his own libs in the runpath
LINKFLAGSRUNPATH*=-R\''$$ORIGIN'\'
LINKFLAGS=-w -mt -z combreloc -PIC -temp=/tmp $(LINKFLAGSRUNPATH) -norunpath -library=no%Cstd

# -z text force fatal error if non PIC code is linked into shared library. Such code 
#    would be expensive on startup
CHECKFORPIC =-z text
LINKFLAGSSHLGUI=$(CHECKFORPIC) -G
LINKFLAGSSHLCUI=$(CHECKFORPIC) -G

# switches for dynamic and static linking
LINKFLAGSDEFS*= -z defs
STATIC		= -Bstatic
DIRECT		= -Bdirect $(LINKFLAGSDEFS)
DYNAMIC		= -Bdynamic

LINKFLAGSAPPGUI+=$(DIRECT)
LINKFLAGSAPPCUI+=$(DIRECT)
LINKFLAGSSHLGUI+=$(DIRECT)
LINKFLAGSSHLCUI+=$(DIRECT)

LINKFLAGSTACK=
LINKFLAGSPROF=-L$(COMPATH)/WS6U1/lib/libp -xpg -z allextract
LINKFLAGSDEBUG=
LINKFLAGSOPT=
LINKVERSIONMAPFLAG=-M

APPLINKSTATIC=$(STATIC)
APPLINKSHARED=$(DIRECT)
APP_LINKTYPE=

# reihenfolge der libs NICHT egal!
STDOBJGUI=
.IF "DBG_UTIL" != ""
STDSLOGUI=#-lpthread
.ELSE
STDSLOGUI=
.ENDIF
STDOBJCUI=
STDSLOCUI=

STDLIBGUIST=$(DYNAMIC) -lm
STDLIBCUIST=$(DYNAMIC) -lm
STDLIBGUIMT=$(DYNAMIC) -lpthread -lm
STDLIBCUIMT=$(DYNAMIC) -lpthread -lm
STDSHLGUIST=$(DYNAMIC) -lCrun -lm -lc
STDSHLCUIST=$(DYNAMIC) -lCrun -lm -lc
STDSHLGUIMT=$(DYNAMIC) -lpthread -lCrun -lm -lc
STDSHLCUIMT=$(DYNAMIC) -lpthread -lCrun -lm -lc

STDLIBGUIST+=-lX11
STDLIBGUIMT+=-lX11
STDSHLGUIST+=-lX11
STDSHLGUIMT+=-lX11

# @@@ interposer needed for -Bdirect @@@
# LIBSALCPPRT*=-z allextract -lsalcpprt -z defaultextract
LIBSALCPPRT=

LIBSTLPORT=$(DYNAMIC) -lstlport_sunpro
LIBSTLPORTST=$(STATIC) -lstlport_sunpro $(DYNAMIC)


LIBMGR=CC
LIBFLAGS=-xar -o 

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=ss

DLLPRE=lib
DLLPOST=.so

LDUMP=cppfilt /b /n /o /p

CFLAGSCXXSLO += -instances=static
CFLAGSCXXOBJ += -instances=static

LINKFLAGSAPPGUI+= -instances=static
LINKFLAGSSHLGUI+= -instances=static
LINKFLAGSAPPCUI+= -instances=static
LINKFLAGSSHLCUI+= -instances=static
