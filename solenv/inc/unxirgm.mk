
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
CDEFS+=	-DVCL 
CDEFS+=	-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
# CFLAGS+=	-D__STL_NATIVE_INCLUDE_PATH=/usr/include -D__STL_NATIVE_C_INCLUDE_PATH=/usr/include
# CFLAGS+=	-D_STL_NATIVE_INCLUDE_PATH=/usr/include -D_STL_NATIVE_C_INCLUDE_PATH=/usr/include
CFLAGSCC=	 -c
CFLAGSCXX= 	-fno-for-scope -fpermissive
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
LINKFLAGS=	-L/usr/lib32	
LINKFLAGSAPPGUI= -Wl,-multigot
LINKFLAGSAPPCUI= -Wl,-multigot

LINKFLAGSSHLGUI= -shared
LINKFLAGSSHLCUI= -shared

LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG= -g
LINKFLAGSOPT=

LINKFLAGSSHLGUI += -Wl,-Bsymbolic
LINKFLAGSSHLCUI += -Wl,-Bsymbolic

APPLINKSTATIC=-Bstatic
APPLINKSHARED=-Bsymbolic

# Sequence of libraries DOES matter!
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
# STDLIBGUIST=	$(DYNAMIC) -lXt -lXmu -lX11 -lc -lm
STDLIBGUIST=	$(DYNAMIC) -lX11 -lc -lm
STDLIBCUIST=	$(DYNAMIC) -lc -lm
# STDLIBGUIMT=	$(THREADLIB) $(DYNAMIC) -lXt -lXmu -lX11 -lc -lm
STDLIBGUIMT=	$(THREADLIB) $(DYNAMIC) -lX11 -lc -lm
STDLIBCUIMT=	$(THREADLIB) $(DYNAMIC) -lc -lm
# STDSHLGUIMT=	-L/usr/lib32 $(THREADLIB) $(DYNAMIC) -lXt -lXmu -lX11 -lc -lm
STDSHLGUIMT=	-L/usr/lib32 $(THREADLIB) $(DYNAMIC) -lX11 -lc -lm
STDSHLCUIMT=	-L/usr/lib32 $(THREADLIB) $(DYNAMIC) -lc -lm
THREADLIB=		-lpthread

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


LDUMP=

STDLIBCPP = -lstdc++

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

