
# mak file fuer unxlnxi
ASM=
AFLAGS=

# _PTHREADS is needed for the stl
CDEFS+=-D_PTHREADS
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
.IF "$(GLIBC)"=="2"
CFLAGS=-w -c -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(SOLARINC)
.ELSE
CFLAGS=-w -c -I. -I/usr/solar/inc/pthread_provenzano -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(SOLARINC)
.ENDIF
CFLAGSCC=-pipe -mpentium -fguiding-decls
CFLAGSCXX=-pipe -mpentium -fguiding-decls
#STDOBJVCL=$(L)$/salmain.o
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
CFLAGSNOOPT=-O2
CFLAGSOUTOBJ=-o

STATIC		= -Bstatic
DYNAMIC		= -Bdynamic

LINK=ld
.IF "$(GLIBC)"=="2"
LINKFLAGS=-melf_i386 -z nodefs -dynamic-linker /lib/ld-linux.so.2 /usr/lib/crti.o /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtbegin.o
.ELSE
.IF "$(TARGETTHREAD)"=="MT"
LINKFLAGS=-melf_i386 -z nodefs -dynamic-linker /lib/ld-linux.so.1 /usr/lib/crti.o /usr/lib/crtbegin.o -lpthread_init
.ELSE
LINKFLAGS=-melf_i386 -z nodefs -dynamic-linker /lib/ld-linux.so.1 /usr/lib/crti.o /usr/lib/crtbegin.o
.ENDIF
.ENDIF
LINKFLAGSAPPGUI=/usr/lib/crt1.o 
LINKFLAGSSHLGUI=-noinhibit-exec -warn-once -Bsymbolic -G
LINKFLAGSAPPCUI=/usr/lib/crt1.o 
LINKFLAGSSHLCUI=/usr/lib/crt1.o
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

# reihenfolge der libs NICHT egal!

# standard C++ Library
#
# das statische dazulinken der libstdc++ macht jede shared library um 50k 
# (ungestrippt) oder so groesser, auch wenn sie ueberhaupt nicht gebraucht 
# wird. Da muessen wir uns was besseres ueberlegen.
#
# Da mit der neuen libc.so.6 (libc-2.0.5.so) sowieso eine System-Library
# ausgeliefert werden muss, linken wir auch die libstdc++.so dynamisch.

.IF "$(GLIBC)"=="2"
STDLIBCPP=-lstdc++
.ELSE
STDLIBCPP=-Bstatic -lstdc++ -Bdynamic
.ENDIF

STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

.IF "$(WORK_STAMP)=="MIX364"
.IF "$(GLIBC)"=="2"
STDLIBGUIST=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDLIBCUIST=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDLIBGUIMT=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDLIBCUIMT=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDSHLGUIMT=-Bdynamic -lX11 -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDSHLCUIMT=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
.ELSE
STDLIBGUIST=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDLIBCUIST=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDLIBGUIMT=-Bdynamic -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDLIBCUIMT=-Bdynamic -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDSHLGUIMT=-Bdynamic -lX11 -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDSHLCUIMT=-Bdynamic -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
.ENDIF
.ELSE
.IF "$(GLIBC)"=="2"
STDLIBGUIST=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDLIBCUIST=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDLIBGUIMT=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDLIBCUIMT=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDSHLGUIMT=-Bdynamic -lX11 -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDSHLCUIMT=-Bdynamic -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
.ELSE
STDLIBGUIST=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDLIBCUIST=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDLIBGUIMT=-Bdynamic -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDLIBCUIMT=-Bdynamic -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDSHLGUIMT=-Bdynamic -lX11 -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDSHLCUIMT=-Bdynamic -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
.ENDIF
.ENDIF

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

