
# Makefile for IRIX/mips using GNU compilers 2.95.2 for n32 ABI
ASM=
AFLAGS=

#
# Compiler flags:
#
# -ptused :	Instantiate templates while they're in
#		use. This switch was added because a static member function
#		couldn't be instantiated.
#		(ts&sdo/10.06.98)
#
#cc=			cc -KPIC -c -n32 -ansi
cc=			gcc
#CC=			CC -KPIC -c -n32 -ptused -OPT:Olimit=20523 
CC=			g++ -c
CFLAGS=		$(INCLUDE)
CDEFS+=	-DVCL -D_PTHREADS
CDEFS+=	-D_STD_NO_NAMESPACE -D_UNO_NO_NAMESPACE -DHAVE_UNISTD_H -DSTLPORT_VERSION=0x450
#CDEFS+=	-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE -DHAVE_UNISTD_H -DSTLPORT_VERSION=0x450
# CFLAGS+=	-D__STL_NATIVE_INCLUDE_PATH=/usr/include -D__STL_NATIVE_C_INCLUDE_PATH=/usr/include
# CFLAGS+=	-D_STL_NATIVE_INCLUDE_PATH=/usr/include -D_STL_NATIVE_C_INCLUDE_PATH=/usr/include
CFLAGSCC=	 -w -c -nostdinc
CFLAGSCXX= 	-w -fno-for-scope -fpermissive -nostdinc -nostdinc++
CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT= -fpic
CFLAGSSLOCUIMT= -fpic
CFLAGSPROF=
CFLAGSDEBUG=	-g
CFLAGSDBGUTIL=
CFLAGSOPT=		-O2
CFLAGSNOOPT=		-O
CFLAGSOUTOBJ=	-o

STATIC=			-Wl,-Bstatic
DYNAMIC=		-Wl,-Bdynamic

#
# Link flags:
#
# -update_registry <file> :	Is needed to link several shared libraries
#				**VALID MECHANISM SOMETIMES MISSING FOR LOCAL STANDS**
#
LINK=			g++
LINKFLAGS=	-L/usr/lib32 -Wl,-no_unresolved

.IF "$(TARGETTHREAD)"=="MT"
LINKFLAGSAPPGUI= $(THREADLIB) -Wl,-multigot -nodefaultlibs
LINKFLAGSAPPCUI= $(THREADLIB) -Wl,-multigot -nodefaultlibs
LINKFLAGSSHLGUI= $(THREADLIB) -shared -nodefaultlibs
LINKFLAGSSHLCUI= $(THREADLIB) -shared -nodefaultlibs
.ELSE
LINKFLAGSAPPGUI= -Wl,-multigot -nodefaultlibs
LINKFLAGSAPPCUI= -Wl,-multigot -nodefaultlibs
LINKFLAGSSHLGUI= -shared -nodefaultlibs
LINKFLAGSSHLCUI= -shared -nodefaultlibs
.ENDIF

LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG= -g
LINKFLAGSOPT=

#LINKFLAGSSHLGUI += -Wl,-Bsymbolic -Wl,-soname -Wl,$(DLLPRE)$(SHL$(TNR)TARGET)$(DLLPOST) $(DYNAMIC)
#LINKFLAGSSHLCUI += -Wl,-Bsymbolic -Wl,-soname -Wl,$(DLLPRE)$(SHL$(TNR)TARGET)$(DLLPOST) $(DYNAMIC)

APPLINKSTATIC=-Bstatic
APPLINKSHARED=-Bsymbolic

# Sequence of libraries DOES matter!
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
STDLIBGUIST=	$(DYNAMIC) -lX11 -lc -lm
STDLIBCUIST=	$(DYNAMIC) -lgcc -lc
STDLIBGUIMT=	$(THREADLIB) $(DYNAMIC) -lX11 -lm -lgcc -lc
STDLIBCUIMT=	$(THREADLIB) $(DYNAMIC) -lgcc -lc -lm
STDSHLGUIMT=	-L/usr/lib32 $(THREADLIB) $(DYNAMIC) -lX11 -lm -lgcc -lc
STDSHLCUIMT=	-L/usr/lib32 $(THREADLIB) $(DYNAMIC) -lm -lgcc -lc
THREADLIB=		-lpthread

LIBSTLPORT=$(DYNAMIC) -lstlport_gcc
LIBSTLPORTST= -lstlport_gcc

LIBMGR=			ar
LIBFLAGS=		-r
# LIBEXT=		.so

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=				irc
RCFLAGS=		-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=		im
.IF "$(WORK_STAMP)"=="MIX364"
DLLPOSTFIX=
.ENDIF
DLLPRE=			lib
DLLPOST=		.so


LDUMP=c++filt

#STDLIBCPP = -lstdc++

# --------------------------
# FROM THE OLE ENVIRONMENT:
# --------------------------
#
# Linking of a static library:
#	ar -r ...
#
# Linking of a shared library:
#	CC -B symbolic -soname <...> -B dynamic -shared -n32 -multigot
#		-update_registry <...> ...
#
# Linking of an application with static libraries:
#	CC -B static ...
#
# Linking of an application with shared libraries:
#	CC -n32 -multigot ...
#

