
# mak file fuer unxlngp
ASM=
AFLAGS=

# _PTHREADS is needed for the stl
CDEFS+=-D_PTHREADS -D_REENTRANT -DGLIBC=2
CDEFS+=-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
CDEFS+=-DNO_INET_ON_DEMAND -DX86 -DNEW_SOLAR

# kann c++ was c braucht??

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
CFLAGS=-w -nostdinc -c $(INCLUDE) 
CFLAGSCC= -pipe
CFLAGSCXX= -pipe -fguiding-decls -fno-rtti -fno-exceptions

# Exceptions increase the size of shared libraries by 50% !!
.IF "$(PRJNAME)"=="usr" || "$(PRJNAME)"=="uno" || "$(PRJNAME)"=="starone" || "$(PRJNAME)"=="schedule" || "$(PRJNAME)"=="one" || "$(SET_EXEPTIONS)"!=""
CFLAGSCXX += -fexceptions
.ENDIF

#STDOBJVCL=$(L)$/salmain.o
CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=-fPIC
CFLAGSSLOCUIMT=-fPIC
CFLAGSPROF=
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
CFLAGSOPT=-O2
CFLAGSNOOPT=
CFLAGSOUTOBJ=-o

STATIC		= -Bstatic
DYNAMIC		= -Bdynamic

LINK=ld
LINKFLAGS=-melf_i386 -z nodefs -dynamic-linker /lib/ld-linux.so.2 /nw386/dev/s/solenv/unxlngp/usr/lib/crti.o /nw386/dev/s/solenv/unxlngp/lib/gcc-lib/i586-pc-linux-gnu/egcs-2.90.29/crtbegin.o
LINKFLAGSAPPGUI=/usr/lib/crt1.o
LINKFLAGSSHLGUI=-noinhibit-exec -warn-once -G
LINKFLAGSAPPCUI=/usr/lib/crt1.o
LINKFLAGSSHLCUI=-noinhibit-exec -warn-once -G
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

.IF "$(NO_BSYMBOLIC)"==""
.IF "$(PRJNAME)" != "envtest"
LINKFLAGSSHLGUI+=-Bsymbolic
LINKFLAGSSHLCUI+=-Bsymbolic
.ENDIF
.ENDIF				# "$(NO_BSYMBOLIC)"==""

# reihenfolge der libs NICHT egal!

# standard C++ Library
#
# das statische dazulinken der libstdc++ macht jede shared library um 50k 
# (ungestrippt) oder so groesser, auch wenn sie ueberhaupt nicht gebraucht 
# wird. Da muessen wir uns was besseres ueberlegen.
#
# Da mit der neuen libc.so.6 (libc-2.0.5.so) sowieso eine System-Library
# ausgeliefert werden muss, linken wir auch die libstdc++.so dynamisch.

STDLIBCPP=-lstdc++

STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

STDLIBGUIST=-Bdynamic -lcrypt -ldl -lm -lgcc -lc /nw386/dev/s/solenv/unxlngp/lib/gcc-lib/i586-pc-linux-gnu/egcs-2.90.29/crtend.o /nw386/dev/s/solenv/unxlngp/usr/lib/crtn.o 
STDLIBCUIST=-Bdynamic -lcrypt -ldl -lm -lgcc -lc /nw386/dev/s/solenv/unxlngp/lib/gcc-lib/i586-pc-linux-gnu/egcs-2.90.29/crtend.o /nw386/dev/s/solenv/unxlngp/usr/lib/crtn.o
STDLIBGUIMT=-Bdynamic -lcrypt -ldl -lpthread -lm -lgcc -lc /nw386/dev/s/solenv/unxlngp/lib/gcc-lib/i586-pc-linux-gnu/egcs-2.90.29/crtend.o /nw386/dev/s/solenv/unxlngp/usr/lib/crtn.o
STDLIBCUIMT=-Bdynamic -lcrypt -ldl -lpthread -lm -lgcc -lc /nw386/dev/s/solenv/unxlngp/lib/gcc-lib/i586-pc-linux-gnu/egcs-2.90.29/crtend.o /nw386/dev/s/solenv/unxlngp/usr/lib/crtn.o
STDSHLGUIMT=-Bdynamic -lX11 -lXext -lcrypt -ldl -lpthread -lm -lgcc -lc /nw386/dev/s/solenv/unxlngp/lib/gcc-lib/i586-pc-linux-gnu/egcs-2.90.29/crtend.o /nw386/dev/s/solenv/unxlngp/usr/lib/crtn.o
STDSHLCUIMT=-Bdynamic -lcrypt -ldl -lpthread -lm -lgcc -lc /nw386/dev/s/solenv/unxlngp/lib/gcc-lib/i586-pc-linux-gnu/egcs-2.90.29/crtend.o /nw386/dev/s/solenv/unxlngp/usr/lib/crtn.o

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

.IF "$(WORK_STAMP)"!="LVM364"
.IF "$(WORK_STAMP)"!="MIX364"
DLLPOSTFIX=li
.ENDIF
.ENDIF
DLLPRE=lib
DLLPOST=.so

LDUMP=cppfilt /b /n /o /p

