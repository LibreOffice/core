#*************************************************************************
#
#   $RCSfile: wnt.mk,v $
#
#   $Revision: 1.56 $
#
#   last change: $Author: vg $ $Date: 2003-10-06 18:30:57 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

SOLAR_JAVA=TRUE
FULL_DESK=TRUE
JAVAFLAGSDEBUG=-g

# --- Windows-NT-Environment ---------------------------------------

.IF "$(GUI)" == "WNT"

# --- Ole2 Behandlung -------------

OLE2=
OLE2DEF=

# --- C/C++ defines fuer nt ----------------------------------------

.IF "$(GUI)"=="WNT"
.IF "$(profile)" != ""
.IF "$(WST)"!=""
CAPLIB=$(LIBPRE) wst.lib
.ELSE
CAPLIB=$(LIBPRE) cap.lib
.ENDIF
.ENDIF
.ENDIF

# --- Set Wrapper command ---
.IF "$(GUI)"=="WNT"
.IF "$(USE_SHELL)"!= "4nt"
.IF "$(COM)" == "MSC" || "$(COM)" == "GCC"
#Fallback, normaly set by winenv.*
WRAPCMD*=guw.pl
.ENDIF
.ENDIF
.ENDIF

# --- IBM Visual Age 3.5 Compiler ---
.IF "$(COM)" == "ICC"
.IF "$(CPU)" == "I"
.IF "$(COMEX)"==""
!INCLUDE <wnticci.mak>
.ENDIF
.ENDIF
.ENDIF

# --- IBM Visual Age 3.5 Compiler ---
.IF "$(COM)" == "ICC"
.IF "$(CPU)" == "I"
.IF "$(COMEX)"=="a"
!INCLUDE <wnticcia.mak>
.ENDIF
.ENDIF
.ENDIF

# --- Microsoft Intel ---
.IF "$(COM)"=="MSC"
.IF "$(CPU)"=="I"

# SOLAR JAva Unterstuetzung nur fuer wntmsci

ASM=ml
AFLAGS=/c /Cp /coff
OLE2ANSI=TRUE

.IF "$(bndchk)" != ""
CXX*=nmcl
.ELSE
.IF "$(truetime)" != ""
CXX*=nmcl /NMttOn
.ELSE
.IF "$(syntax)"!=""
CXX*=$(SOLARROOT)\gcc\h-i386-cygwin32\bin\i386-cygwin32-gcc
.ELSE
.IF "$(USE_SHELL)"=="4nt"
CXX*=cl
.ELSE			# "$(USE_SHELL)"=="4nt"
CXX*=$(WRAPCMD) cl
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF
.ENDIF
.ENDIF

.IF "$(stoponerror)" != ""
CXX+= /NMstoponerror
.ENDIF

.IF "$(nmpass)" != ""
CXX+= /NMpass
.ENDIF

.IF "$(ttinlines)" != ""
CXX+= /NMttInlines
.ENDIF

.IF "$(ttnolines)" != ""
CXX+= /NMttNoLines
.ENDIF

#.IF"$(bndchk)"==""
.IF "$(COMEX)"=="3" || "$(COMEX)"=="7" || "$(COMEX)"=="9"
CFLAGS+=-Zm200
.ENDIF
#.ENDIF

#.IF"$(bndchk)"==""
.IF "$(COMEX)"=="8" || "$(COMEX)"=="10"
CFLAGS+=-Zm500 -wd4290 -wd4786 -wd4800
.ENDIF
#.ENDIF

.IF "$(product)" != ""
CDEFS+= -D_X86_=1 $(OLE2DEF)
CFLAGS+=-c -nologo -Gs -Gy $(NOLOGO) $(MINUS_I)$(INCLUDE)
.IF "$(bndchk)" == ""
CFLAGS+= -Ob1
.ENDIF
.ELSE	# ist keine product...
CDEFS+= -D_X86_=1 $(OLE2DEF)
CFLAGS+=-c -nologo -Gs $(NOLOGO) $(MINUS_I)$(INCLUDE)
.IF "$(bndchk)" == ""
CFLAGS+= -Ob1
.ENDIF
.ENDIF

# flags to enable build with symbols; required for crashdump feature
CFLAGSENABLESYMBOLS=-Zi -Fd$(MISC)$/_ooo_st_$(TARGET).PDB

.IF "$(bndchk)" != ""
.IF "$(debug)" == ""
CFLAGS+= -Z7
.ENDIF
.ENDIF

.IF "$(truetime)" != ""
.IF "$(debug)" == ""
CFLAGS+= -Z7
.ENDIF
.ENDIF

.IF "$(FULL_DESK)"!=""
CDEFS+=-DFULL_DESK
RSCDEFS+=-DFULL_DESK
.ENDIF

CFLAGSEXCEPTIONS=-GX
CFLAGS_NO_EXCEPTIONS=

.IF "$(CALL_CDECL)"=="TRUE"
CFLAGSCALL=-Gd
.ELSE			# "$(CALL_CDECL)"=="TRUE"
CFLAGSCALL=-Gz
.ENDIF			# "$(CALL_CDECL)"=="TRUE"

CFLAGSCC=
.IF "$(DYNAMIC_CRT)"!=""
CDEFSSLOMT+=-DWIN32 -D_MT -D_DLL
CDEFSSLOMT+=-DWIN32 -D_MT -D_DLL
.IF "$(NO_DYNAMIC_OBJ)"==""
CDEFSOBJST+=-DWIN32 -D_DLL
CDEFSOBJST+=-DWIN32 -D_DLL
CDEFSOBJMT+=-DWIN32 -D_MT -D_DLL
CDEFSOBJMT+=-DWIN32 -D_MT -D_DLL
.ELSE
CDEFSOBJST+=-DWIN32
CDEFSOBJST+=-DWIN32
CDEFSOBJMT+=-DWIN32 -D_MT
CDEFSOBJMT+=-DWIN32 -D_MT
.ENDIF
.ELSE
CDEFSSLOMT+=-DWIN32 -D_MT
CDEFSSLOMT+=-DWIN32 -D_MT
CDEFSOBJST+=-DWIN32
CDEFSOBJST+=-DWIN32
CDEFSOBJMT+=-DWIN32 -D_MT
CDEFSOBJMT+=-DWIN32 -D_MT
.ENDIF
CFLAGSPROF=-Gh -Zd -Fd$(MISC)\_ooo_st_$(TARGET).PDB
.IF "$(PDBTARGET)"!=""
CFLAGSDEBUG=-Zi -Fd$(MISC)\$(PDBTARGET).PDB
.ELSE
CFLAGSDEBUG=-Zi -Fd$(MISC)\_ooo_st_$(TARGET).PDB
.ENDIF
CFLAGSDBGUTIL=
CFLAGSOPT=-Ox -Oy-
CFLAGSNOOPT=-Od
CFLAGSOUTOBJ=-Fo
.IF "$(COMEX)"=="8" || "$(COMEX)"=="10"
CFLAGSWALL=-Wall -wd4294 -wd4640
.ELSE			# "$(COMEX)"=="8"
CFLAGSWALL=-W4
.ENDIF			# "$(COMEX)"=="8"
CFLAGSDFLTWARN=-W3

.IF "$(syntax)"!=""
CFLAGS=-v -fsyntax-only -Wall $(INCLUDE)
# -I$(SOLARROOT)$/H-i386-cygwin32$/i386-cygwin32/include
#plattform hart setzen - macht sonst ms cl.exe
CDEFS+=-D_M_IX86
# -D_MSC_VER=1100

CDEFS+=-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE -D_cdecl=
CFLAGSOUTOBJ=-o
CFLAGSCALL=
CFLAGSCXX=
CFLAGSOPT=
CFLAGSNOOPT=
CFLAGSPROF=
CFLAGSDEBUG=
.ENDIF			# "$(syntax)"!=""
.IF "$(COMEX)"=="3"
CDEFS+=-D__STL_NO_NEW_IOSTREAMS -DSTLPORT_VERSION=400 -D__STL_USE_ABBREVS
CDEFS+=-D_MT
.ENDIF
.IF "$(COMEX)"=="7" || "$(COMEX)"=="9"
#CDEFS+=-D__STL_NO_NEW_IOSTREAMS -DSTLPORT_VERSION=400 -D__STL_USE_ABBREVS
CDEFS+=-DSTLPORT_VERSION=400 -DWINVER=0x400 -D_WIN32_IE=0x400
CDEFS+=-D_MT
.ENDIF
.IF "$(COMEX)"=="8" || "$(COMEX)"=="10"
#CDEFS+=-D__STL_NO_NEW_IOSTREAMS -DSTLPORT_VERSION=450 -D__STL_USE_ABBREVS
CDEFS+=-DSTLPORT_VERSION=400 -DWINVER=0x400 -D_WIN32_IE=0x400
CDEFS+=-D_MT
.ENDIF

COMMENTFLAG=/COMMENT:"$(PRJNAME)_$(UPD)_$(DESTINATION_MINOR)_$(FUNCORD)_$(__DATE)_$(__TIME)_$(VCSID)_"


.IF "$(USE_SHELL)"=="4nt"
LINK=link $(COMMENTFLAG) $(NOLOGO) /MACHINE:IX86
.ELSE			# "$(USE_SHELL)"=="4nt"
LINK=$(WRAPCMD) link $(NOLOGO) /MACHINE:IX86
.ENDIF			# "$(USE_SHELL)"=="4nt"

.IF "$(PRODUCT)"!="full"
.ELSE
LINKFLAGS=/MAP /OPT:NOREF
.ENDIF

.IF "$(linkinc)" != ""
LINKFLAGS=/NODEFAULTLIB /DEBUG:full /DEBUGTYPE:cv /INCREMENTAL:YES
MAPFILE=
.ELSE
.IF "$(PRODUCT)"!="full"
LINKFLAGS+= /NODEFAULTLIB /DEBUG:full /DEBUGTYPE:cv
.ELSE
LINKFLAGS+= /NODEFAULTLIB /RELEASE /DEBUG:notmapped,full
.ENDIF
MAPFILE=-out:$$@
.ENDIF

.IF "$(bndchk)" != ""
LINK=nmlink $(COMMENTFLAG) $(NOLOGO) /MACHINE:IX86
LINKFLAGS=/NODEFAULTLIB /DEBUG:full /DEBUGTYPE:cv
.ENDIF

.IF "$(truetime)" != ""
LINK=nmlink /NMttOn $(COMMENTFLAG) $(NOLOGO) /MACHINE:IX86
LINKFLAGS=/NODEFAULTLIB /DEBUG:full /DEBUGTYPE:cv
.ENDIF

LINKFLAGSAPPGUI=/SUBSYSTEM:WINDOWS,4.0
LINKFLAGSSHLGUI=/SUBSYSTEM:WINDOWS,4.0 /DLL
LINKFLAGSAPPCUI=/SUBSYSTEM:CONSOLE /BASE:0x1b000000
LINKFLAGSSHLCUI=/SUBSYSTEM:CONSOLE /DLL
LINKFLAGSTACK=/STACK:
LINKFLAGSPROF=/DEBUG:mapped,partial /DEBUGTYPE:coff cap.lib
LINKFLAGSWST=/DEBUG:mapped,partial /DEBUGTYPE:coff wst.lib /NODEFAULTLIB
LINKFLAGSDEBUG=/DEBUG:full /DEBUGTYPE:cv
LINKFLAGSOPT=

.IF "$(DYNAMIC_CRT)"!=""
LIBCMT=msvcrt.lib
LIBCIMT=msvcirt.lib
OLDNAMES=oldnames.lib
UWINAPILIB*=uwinapi.lib
.ELSE
LIBCMT=libcmt.lib
LIBCIMT=libcimt.lib
OLDNAMES=oldnames.lib
UWINAPILIB*=uwinapi.lib
.ENDIF

STDOBJVCL=$(L)$/salmain.obj
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
.IF "$(DYNAMIC_CRT)"!=""
STDLIBGUIST=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDLIBCUIST=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDLIBGUIMT=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib $(OLDNAMES)
STDLIBCUIMT=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib $(OLDNAMES)
STDSHLGUIST=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib $(OLDNAMES)
STDSHLCUIST=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib $(OLDNAMES)
STDSHLGUIMT=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib $(OLDNAMES)
STDSHLCUIMT=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib $(OLDNAMES)
.ELSE
STDLIBGUIST=libc.lib $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDLIBCUIST=libc.lib $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDLIBGUIMT=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib $(OLDNAMES)
STDLIBCUIMT=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib $(OLDNAMES)
STDSHLGUIST=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib $(OLDNAMES)
STDSHLCUIST=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib $(OLDNAMES)
STDSHLGUIMT=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib $(OLDNAMES)
STDSHLCUIMT=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib $(OLDNAMES)
.ENDIF

.IF "$(COMEX)" =="8" || "$(COMEX)" > "9"
.IF "$(COMEX)"=="10"
LIBSTLPORT=stlport_vc71.lib
LIBSTLPORTST=stlport_vc71_static.lib
.ELSE
LIBSTLPORT=stlport_vc7.lib
LIBSTLPORTST=stlport_vc7_static.lib
.ENDIF
ATL_INCLUDE=$(COMPATH)$/atlmfc$/include
.ELSE
LIBSTLPORT=stlport_vc6.lib
LIBSTLPORTST=stlport_vc6_static.lib
ATL_INCLUDE=$(COMPATH)$/atl$/include
.ENDIF

.IF "$(USE_SHELL)"=="4nt"
LIBMGR=lib $(NOLOGO)
IMPLIB=lib
.ELSE			# "$(USE_SHELL)"=="4nt"
LIBMGR=$(WRAPCMD) lib $(NOLOGO)
IMPLIB=$(WRAPCMD) lib
.ENDIF			# "$(USE_SHELL)"=="4nt"
LIBFLAGS=

IMPLIBFLAGS=-machine:IX86

MAPSYM=
MAPSYMFLAGS=

.IF "$(USE_SHELL)"=="4nt"
RC=rc
.ELSE			# "$(USE_SHELL)"=="4nt"
RC=$(WRAPCMD) rc
.ENDIF			# "$(USE_SHELL)"=="4nt"
RCFLAGS=-r -DWIN32 -fo$@ $(RCFILES)
RCLINK=rc
RCLINKFLAGS=
RCSETVERSION=


DLLPOSTFIX=mi

.IF "$(USE_SHELL)"!="4nt"
IDLC=$(WRAPCMD) idlc
REGMERGE=$(WRAPCMD) regmerge
REGCOMPARE=$(WRAPCMD) regcompare
REGCOMP=$(WRAPCMD) regcomp
CPPUMAKER=$(WRAPCMD) cppumaker
JAVAMAKER=$(WRAPCMD) javamaker
RDBMAKER=$(WRAPCMD) rdbmaker
STARDEP=$(WRAPCMD) javadep
JAVAC=$(WRAPCMD) javac
JAVA=$(WRAPCMD) java
SCPCOMP=$(WRAPCMD) scpcomp
SCPLINK=$(WRAPCMD) scplink
LZIP=$(WRAPCMD) -env lzip
CPPLCC=$(WRAPCMD) cpplcc
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF
.ENDIF              # "$(COM)"=="MSC"

# --- Cygnus Gnu Compiler ---
.IF "$(COM)" == "GCC"
.IF "$(CPU)" == "I"

ASM=
AFLAGS=

CXX*=gcc
### Der gcc vertraegt kein Semikolon im Include-Pfad         RT
# old:
#CFLAGS=-c -Wall -I$(INCLUDE) $(OLE2DEF)
# new:
CYGINC=$(INCLUDE:s/-I /-I/:+"  ":s/;/ -I/:s/-I  //:s/   / /)
CFLAGS=-c -Wall -I$(CYGINC) $(OLE2DEF)
###
CFLAGS+=-nostdinc -fPIC
CFLAGSCC=-pipe -mpentium
CFLAGSCXX=-pipe -mpentium -fguiding-decls
CFLAGSEXCEPTIONS=-fexceptions
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

CFLAGSOBJGUIST=-DWIN32
CFLAGSOBJCUIST=-DWIN32
CFLAGSOBJGUIMT=-DWIN32
CFLAGSOBJCUIMT=-DWIN32
CFLAGSSLOGUIMT=-DWIN32 -fPIC
CFLAGSSLOCUIMT=-DWIN32 -fPIC
CFLAGSPROF=
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
CFLAGSOPT=-O3
CFLAGSNOOPT=-O
CFLAGSOUTOBJ=-o
#plattform hart setzen
CDEFS+=-D_M_IX86

STATIC= -static
DYNAMIC= -dynamic

LINK*=ld
CYGLIB=$(LIB:s/;/ -L/)
LINKFLAGS=-nodefaultlibs -L$(CYGLIB)
LINKFLAGSAPPGUI=$(COMPATH)$/i386-mingw32$/lib$/crt1.o
LINKFLAGSSHLGUI=--warn-once
LINKFLAGSAPPCUI=$(COMPATH)$/i386-mingw32$/lib$/crt1.o
LINKFLAGSSHLCUI=--warn-once
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

STDLIBCPP=-lstdc++

STDOBJVCL=$(L)$/salmain.obj
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
STDLIBGUIST=-lmingw32 -lgcc -lmoldname -dynamic -ldl -lm -lkernel32 -luser32
STDLIBCUIST=-lmingw32 -lgcc -lmoldname -dynamic -ldl -lm -lkernel32 -luser32
STDLIBGUIMT=-dynamic -ldl -lm -lmingw32 -lkernel32 -luser32 -lgcc -lmoldname
STDLIBGUIMT+= -lmsvcrt -lcrtdll -luser32
STDLIBCUIMT=-lmingw32 -lgcc -lmoldname -dynamic -ldl -lpthread -lm -lkernel32 -luser32
STDSHLGUIMT=-dynamic -ldl -lm -lkernel32 -luser32 -lmingw32 -lgcc
STDSHLGUIMT+= -lmoldname -lmsvcrt -lcrtdll -luser32
STDSHLCUIMT=-dynamic -ldl -lm -lkernel32 -luser32

LIBMGR=ar
LIBFLAGS=-rsu

IMPLIB=ld
IMPLIBFLAGS=

MAPSYM=tmapsym
MAPSYMFLAGS=

RC=rc
RCFLAGS=-DWIN32 -fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=gi

.ENDIF
.ENDIF
# --- Windows-NT Allgemein ---
HC=hc
HCFLAGS=
PATH_SEPERATOR*=;
DLLPRE=
DLLPOST=.dll
EXECPOST=.exe
SCPPOST=.inf
DLLDEST=$(BIN)

.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
.IF "$(debug)"==""
JAVA_RUNTIME=javai.lib
.ELSE
JAVA_RUNTIME=javai_g.lib
.ENDIF
.ENDIF

.ENDIF

