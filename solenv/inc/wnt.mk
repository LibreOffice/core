#*************************************************************************
#
#   $RCSfile: wnt.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: kz $ $Date: 2001-03-28 16:06:38 $
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

.IF "$(DONT_USE_OLE2)"!=""
OLE2=
OLE2DEF=
.ELSE
OLE2=true
OLE2DEF=-DOLE2
.ENDIF

# --- C/C++ defines fuer nt ----------------------------------------

.IF "$(HBTOOLKIT)"!=""
CDEFS+=-DHB_DEBUG
.ENDIF

# --- Base fuer WNT setzen -----------------------------------------

.IF "$(GUI)"=="WNT"
BASE=0x1100000
RESBASE=0x1100000
.IF "$(PRJNAME)"=="OSL" || "$(PRJNAME)"=="osl"
BASE=0x1c000000
.ENDIF
.IF "$(PRJNAME)"=="RTL" || "$(PRJNAME)"=="rtl"
BASE=0x1c100000
.ENDIF
.IF "$(PRJNAME)"=="VOS" || "$(PRJNAME)"=="vos"
BASE=0x1c200000
.ENDIF
.IF "$(PRJNAME)"=="UCR" || "$(PRJNAME)"=="ucr"
BASE=0x1c300000
.ENDIF
.IF "$(PRJNAME)"=="USR" || "$(PRJNAME)"=="usr"
BASE=0x1c400000
.ENDIF
.IF "$(PRJNAME)"=="UNO" || "$(PRJNAME)"=="uno"
BASE=0x1c500000
.ENDIF
.IF "$(PRJNAME)"=="SOT" || "$(PRJNAME)"=="sot"
BASE=0x1c900000
.ENDIF
.IF "$(PRJNAME)"=="TOOLS" || "$(PRJNAME)"=="tools"
BASE=0x1c800000
.ENDIF
.IF "$(PRJNAME)"=="VCL" || "$(PRJNAME)"=="vcl"
BASE=0x1c600000
.ENDIF
.IF "$(PRJNAME)"=="TOOLKIT" || "$(PRJNAME)"=="toolkit"
BASE=0x1ca00000
.ENDIF
.IF "$(PRJNAME)"=="TKT" || "$(PRJNAME)"=="tkt"
BASE=0x1cd00000
.ENDIF
.IF "$(PRJNAME)"=="SJ" || "$(PRJNAME)"=="sj"
BASE=0x1d000000
.ENDIF
.IF "$(PRJNAME)"=="STARONE" || "$(PRJNAME)"=="starone" || "$(PRJNAME)"=="ONE" || "$(PRJNAME)"=="one" 
BASE=0x1ce00000
.ENDIF
.IF "$(PRJNAME)"=="CHAOS" || "$(PRJNAME)"=="chaos" 
BASE=0x1d100000
.ENDIF
.IF "$(PRJNAME)"=="SVTOOLS" || "$(PRJNAME)"=="svtools"
BASE=0x1cb00000
.ENDIF
.IF "$(PRJNAME)"=="SO3" || "$(PRJNAME)"=="so3"
BASE=0x1d400000
.ENDIF
.IF "$(PRJNAME)"=="GOODIES" || "$(PRJNAME)"=="goodies"
BASE=0x1d290000
.ENDIF
.IF "$(PRJNAME)"=="BASIC" || "$(PRJNAME)"=="basic"
BASE=0x1d380000
.ENDIF
.IF "$(PRJNAME)"=="HM2" || "$(PRJNAME)"=="hm2"
BASE=0x1c700000
.ENDIF
.IF "$(PRJNAME)"=="SFX2" || "$(PRJNAME)"=="sfx2"
BASE=0x1d500000
RESBASE=0x1a00000
.ENDIF
.IF "$(PRJNAME)"=="SVX" || "$(PRJNAME)"=="svx"
RESBASE=0x1d70000
.ENDIF
.IF "$(PRJNAME)"=="OFFMGR" || "$(PRJNAME)"=="offmgr"
BASE=0x1dc00000
RESBASE=0x1a20000
.ENDIF
.IF "$(PRJNAME)"=="SIM" || "$(PRJNAME)"=="sim" || "$(PRJNAME)"=="simage3"
BASE=0x1cd00000
RESBASE=0x1a30000
.ENDIF
.IF "$(PRJNAME)"=="SCH" || "$(PRJNAME)"=="sch" || "$(PRJNAME)"=="schart3"
BASE=0x1ce00000
RESBASE=0x1a40000
.ENDIF
.IF "$(PRJNAME)"=="SM" || "$(PRJNAME)"=="sm" || "$(PRJNAME)"=="smath3"
BASE=0x1cf00000
RESBASE=0x1a50000
.ENDIF
.IF "$(PRJNAME)"=="SDB" || "$(PRJNAME)"=="sdb"
BASE=0x1d000000
.ENDIF
.IF "$(PRJNAME)"=="INET" || "$(PRJNAME)"=="inet"
BASE=0x1d800000
.ENDIF
.IF "$(PRJNAME)"=="SW" || "$(PRJNAME)"=="sw" || "$(PRJNAME)"=="swriter3"
BASE=0x1e000000
RESBASE=0x1a60000
.ENDIF
.IF "$(PRJNAME)"=="SC" || "$(PRJNAME)"=="sc" || "$(PRJNAME)"=="scalc3"
BASE=0x1e800000
RESBASE=0x1a70000
.ENDIF
.IF "$(PRJNAME)"=="SD" || "$(PRJNAME)"=="sd" || "$(PRJNAME)"=="sdraw3"
BASE=0x1eb00000
RESBASE=0x1a80000
.ENDIF
.IF "$(PRJNAME)"=="OFF" || "$(PRJNAME)"=="off" || "$(PRJNAME)"=="offmgr"
BASE=0x1b900000
RESBASE=0x1a90000
.ENDIF
.ENDIF

.IF "$(GUI)"=="WNT"
.IF "$(profile)" != ""
.IF "$(WST)"!=""
CAPLIB=$(LIBPRE) wst.lib
.ELSE
CAPLIB=$(LIBPRE) cap.lib
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

.IF "$(UPD)">="616"
ASM=ml
AFLAGS=/c /Cp /coff
.ELSE			# "$(UPD)">="616"
ASM=masm386
AFLAGS=/ml
.ENDIF			# "$(UPD)">="616"
OLE2ANSI=TRUE

.IF "$(bndchk)" != ""
CC=$(DEVROOT)\bcheck6\nmcl
.ELSE
.IF "$(truetime)" != ""
CC=$(DEVROOT)\truetime\nmcl /NMttOn
.ELSE
.IF "$(syntax)"!=""
CC=$(SOLARROOT)\gcc\h-i386-cygwin32\bin\i386-cygwin32-gcc
.ELSE
CC=cl
.ENDIF
.ENDIF
.ENDIF

.IF "$(stoponerror)" != ""
CC+= /NMstoponerror
.ENDIF

.IF "$(nmpass)" != ""
CC+= /NMpass
.ENDIF

.IF "$(ttinlines)" != ""
CC+= /NMttInlines	
.ENDIF

.IF "$(ttnolines)" != ""
CC+= /NMttNoLines
.ENDIF

#.IF"$(bndchk)"==""
.IF "$(COMEX)"=="3" || "$(COMEX)"=="7" 
CFLAGS+=-Zm200
.ENDIF
#.ENDIF

#.IF defined ( product ) && !defined ( seg ) && !defined ( demo )
.IF "$(product)" != ""
.IF "$(seg)" == ""
.IF "$(demo)" == ""
CDEFS+= -D_X86_=1 $(OLE2DEF)
CFLAGS+=-c -nologo -W3 -Gs -Gy $(NOLOGO) $(MINUS_I)$(INCLUDE)
.IF "$(bndchk)" == ""
CFLAGS+= -Ob1
.ENDIF
.ENDIF
.ENDIF
.ELSE	# ist keine product...
CDEFS+= -D_X86_=1 $(OLE2DEF)
CFLAGS+=-c -nologo -W3 -Gs $(NOLOGO) $(MINUS_I)$(INCLUDE)
.IF "$(bndchk)" == ""
CFLAGS+= -Ob1
.ENDIF
.ENDIF

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

#.IF DEFINED compinc
#CFLAGS+= -Gi -Zi
#.ENDIF

#.IF DEFINED chkhxx
#CDEFSS+= -DHEADER_WARNING
#.ENDIF
#.IF "$(UPD)"=="368"
#.IF !DEFINED nocheck
#CDEFS+= -DHEADER_WARNING
#.ENDIF
#.ENDIF

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
CFLAGSPROF=-Gh -Zd -Fd$(MISC)\$(TARGET).PDB
.IF "$(PDBTARGET)"!=""
CFLAGSDEBUG=-Zi -Fd$(MISC)\$(PDBTARGET).PDB
.ELSE
CFLAGSDEBUG=-Zi -Fd$(MISC)\$(TARGET).PDB
.ENDIF
CFLAGSDBGUTIL=
CFLAGSOPT=-Ox
CFLAGSNOOPT=-Od
CFLAGSOUTOBJ=-Fo

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
.IF "$(COMEX)"=="7"
CDEFS+=-D__STL_NO_NEW_IOSTREAMS -DSTLPORT_VERSION=400 -D__STL_USE_ABBREVS
CDEFS+=-D_MT
.ENDIF

COMMENTFLAG=/COMMENT:"$(PRJNAME)_$(UPD)_$(DESTINATION_MINOR)_$(FUNCORD)_$(__DATE)_$(__TIME)_$(VCSID)_"


LINK=link $(COMMENTFLAG) $(NOLOGO) /MACHINE:IX86

.IF "$(PRODUCT)"!="full"
.IF "$(PRODUCT)"!="demo"
.IF "$(PRODUCT)"!="compact"
#.IF !defined(DEBUG)
.IF "$(debug)" == ""
LINKFLAGS=/PDB:NONE
.ENDIF
.ENDIF
.ENDIF
.ELSE
LINKFLAGS=/MAP /NODEFAULTLIB /ORDER:@$(FUNCORD) /OPT:NOREF
#LINKFLAGS=/MAP /NODEFAULTLIB
.ENDIF

.IF "$(linkinc)" != ""
LINKFLAGS=/NODEFAULTLIB /DEBUG:notmapped,full /DEBUGTYPE:cv /INCREMENTAL:YES
MAPFILE=
.ELSE
.IF "$(PRODUCT)"!="full"
.IF "$(PRODUCT)"!="demo"
.IF "$(PRODUCT)"!="compact"
LINKFLAGS+= /NODEFAULTLIB /DEBUG:notmapped,full /DEBUGTYPE:cv
.ENDIF
.ENDIF
.ELSE # ist keine product...
.IF "$(COMEX)"!="3"
LINKFLAGS+= /RELEASE
.ENDIF
.ENDIF
MAPFILE=-out:$$@
.ENDIF

.IF "$(bndchk)" != ""
LINK=$(DEVROOT)\bcheck6\nmlink $(COMMENTFLAG) $(NOLOGO) /MACHINE:IX86
#LINKFLAGS=/NODEFAULTLIB /DEBUG:notmapped,full /DEBUGTYPE:cv /PDB:NONE
LINKFLAGS=/NODEFAULTLIB /DEBUG:notmapped,full /DEBUGTYPE:cv
.ENDIF

.IF "$(truetime)" != ""
LINK=$(DEVROOT)\truetime\nmlink /NMttOn $(COMMENTFLAG) $(NOLOGO) /MACHINE:IX86
#LINKFLAGS=/NODEFAULTLIB /DEBUG:notmapped,full /DEBUGTYPE:cv 
LINKFLAGS=/NODEFAULTLIB /DEBUG:notmapped,full /DEBUGTYPE:cv /PDB:NONE
.ENDIF

LINKFLAGSAPPGUI=/SUBSYSTEM:WINDOWS,4.0
LINKFLAGSSHLGUI=/SUBSYSTEM:WINDOWS,4.0 /DLL
LINKFLAGSAPPCUI=/SUBSYSTEM:CONSOLE /BASE:0x1b000000
LINKFLAGSSHLCUI=/SUBSYSTEM:CONSOLE /DLL
LINKFLAGSTACK=/STACK:
LINKFLAGSPROF=/DEBUG:mapped,partial /DEBUGTYPE:coff cap.lib
LINKFLAGSWST=/DEBUG:mapped,partial /DEBUGTYPE:coff wst.lib /NODEFAULTLIB
LINKFLAGSDEBUG=/DEBUG:notmapped,full /DEBUGTYPE:cv
LINKFLAGSOPT=

.IF "$(DYNAMIC_CRT)"!=""
LIBCMT=msvcrt.lib
LIBCIMT=msvcirt.lib
OLDNAMES=oldnames.lib
.ELSE
.IF "$(PRODUCT)"==""
LIBCMT=libcmt.lib
LIBCIMT=libcimt.lib
OLDNAMES=oldnames.lib
.ELSE
LIBCMT=libcmt.lib
LIBCIMT=libcimt.lib
OLDNAMES=oldnames.lib
.ENDIF
.ENDIF

STDOBJVCL=$(L)$/salmain.obj
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
.IF "$(DYNAMIC_CRT)"!=""
STDLIBGUIST=$(LIBCMT) kernel32.lib user32.lib oldnames.lib
STDLIBCUIST=$(LIBCMT) kernel32.lib oldnames.lib
STDLIBGUIMT=$(LIBCMT) kernel32.lib user32.lib $(OLDNAMES)
STDLIBCUIMT=$(LIBCMT) kernel32.lib $(OLDNAMES)
STDSHLGUIMT=$(LIBCMT) kernel32.lib user32.lib $(OLDNAMES)
STDSHLCUIMT=$(LIBCMT) kernel32.lib $(OLDNAMES)
.ELSE
STDLIBGUIST=libc.lib kernel32.lib user32.lib oldnames.lib
STDLIBCUIST=libc.lib kernel32.lib oldnames.lib
STDLIBGUIMT=$(LIBCMT) kernel32.lib user32.lib $(OLDNAMES)
STDLIBCUIMT=$(LIBCMT) kernel32.lib $(OLDNAMES)
STDSHLGUIMT=$(LIBCMT) kernel32.lib user32.lib $(OLDNAMES)
STDSHLCUIMT=$(LIBCMT) kernel32.lib $(OLDNAMES)
.ENDIF

LIBMGR=lib $(NOLOGO)
LIBFLAGS=

IMPLIB=lib
IMPLIBFLAGS=-machine:IX86

MAPSYM=
MAPSYMFLAGS=

RC=rc
RCFLAGS=-r -DWIN32 -fo$@ $(RCFILES)
RCLINK=rc
RCLINKFLAGS=
RCSETVERSION=


DLLPOSTFIX=mi

.ENDIF
.ENDIF              # "$(COM)"=="MSC"

# --- Cygnus Gnu Compiler ---
.IF "$(COM)" == "GCC"
.IF "$(CPU)" == "I"

ASM=
AFLAGS=

CC=gcc
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

LINK=ld
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
STDLIBCUIST=-lmingw32 -lgcc -lmoldname -dynamic -ldl -lm -lkernel32
STDLIBGUIMT=-dynamic -ldl -lm -lmingw32 -lkernel32 -luser32 -lgcc -lmoldname
STDLIBGUIMT+= -lmsvcrt -lcrtdll
STDLIBCUIMT=-lmingw32 -lgcc -lmoldname -dynamic -ldl -lpthread -lm -lkernel32
STDSHLGUIMT=-dynamic -ldl -lm -lkernel32 -luser32 -lmingw32 -lgcc
STDSHLGUIMT+= -lmoldname -lmsvcrt -lcrtdll
STDSHLCUIMT=-dynamic -ldl -lm -lkernel32

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

.IF "$(TFDEF)"!=""
CFLAGS+=-D$(TFDEF)
.ENDIF
