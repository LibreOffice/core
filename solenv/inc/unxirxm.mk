
# mak-file fuer IRIX/mips
ASM=
AFLAGS=

#
# Zu den Compilerflags:
#
# -ptused :		instantisiert Templates, wenn sie benutzt werden.
#				Der Schalter wurde neu eingefuegt, weil sich eine 
#				statische Memberfunktion nicht instantisieren liess.
#				(ts&sdo/10.06.98)
#
cc=			cc -KPIC -c -n32
CC=			CC -KPIC -c -n32 -ptused -OPT:Olimit=20523 
CFLAGS=		$(INCLUDE)
CFLAGS+=	-DVCL -D_PTHREADS
CFLAGS+=	-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
# 1009    ...nested comment is not allowed...
# 1021    ...type qualifiers are meaningless in this declaration...
# 1107    ...signed bit field of length 1...
# 1110    ...statement not reached...
# 1155    ...unrecognized #pragma...
# 1171    ...expression has no effect...
# 1183    ...comparison of unsigned integer with zero... 
# 1185    ...enumerated type mixed with another type... 
# 1188    ...type qualifier is meaningless on cast type... 
# 1233    ...explicit type is missing ("int" assumed)...
# 1234    ...access control not specified...
# 1257    ...NULL reference is not allowed...
# 1440    ...initial value of reference to non-const must be an lvalue...
# 1516    ...pointless comparison of unsigned integer with a negative constant
# 1681    ...virtual function override intended...
# 1682    ...is only partially overridden in class...
CFLAGSCC=	-Xcpluscomm -woff 1009,1021,1107,1110,1155,1171,1183,1185,1188,1233,1234,1257,1440,1516,1681,1682
CFLAGSCXX=	-woff 1009,1021,1107,1110,1155,1171,1183,1185,1188,1233,1234,1257,1440,1516,1681,1682
CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=
CFLAGSSLOCUIMT=
CFLAGSPROF=
CFLAGSDEBUG=	-g
CFLAGSDBGUTIL=
CFLAGSOPT=		-O
CFLAGSNOOPT=
CFLAGSOUTOBJ=	-o

STATIC=			-B static
DYNAMIC=		-B dynamic

#
# Zu den Linkflags:
#
# -update_registry <file> :	fuer das Linken mehrerer shared Libraries 
#				erforderlich. 
#				**FUER LOKALE STAENDE FEHLT Z.ZT. EIN GULTIGER 
#				MECHANISMUS**					
#
LINK=			CC
LINKFLAGS=		
LINKFLAGSAPPGUI=-n32 -multigot
LINKFLAGSSHLGUI=-B symbolic -soname $(DLLPRE)$(SHL$(TNR)TARGET)$(DLLPOSTFIX)$(DLLPOST) $(DYNAMIC) -shared -n32 -multigot -update_registry $(SOLARLIBDIR)/so_locations
LINKFLAGSAPPCUI=-n32 -multigot
LINKFLAGSSHLCUI=-B symbolic -soname $(DLLPRE)$(SHL$(TNR)TARGET)$(DLLPOSTFIX)$(DLLPOST) $(DYNAMIC) -shared -n32 -multigot -update_registry $(SOLARLIBDIR)/so_locations
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=
LINKFLAGSOPT=
APPLINKSTATIC=-Bstatic
APPLINKSHARED=-Bsymbolic

# Reihenfolge der Libraries NICHT egal!
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


# --------------------------
# AUS DEM ALTEN ENVIRONMENT:
# --------------------------
#
# Linken einer statischen Library:
#	ar -r ...
#
# Linken einer shared Library:
#	CC -B symbolic -soname <...> -B dynamic -shared -n32 -multigot
#		-update_registry <...> ...
#
# Linken einer Applikation mit statischen Librarys:
#	CC -B static ...
#
# Linken einer Applikation mit stared Librarys:
#	CC -n32 -multigot ...
#

