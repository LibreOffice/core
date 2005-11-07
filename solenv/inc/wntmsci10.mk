#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: wntmsci10.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2005-11-07 12:44:03 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

# mk file for $(OS)$(COM)$(CPU)$(COMEX) == WNTMSCI10 

SOLAR_JAVA*=TRUE
FULL_DESK=TRUE
JAVAFLAGSDEBUG=-g

# SOLAR JAva Unterstuetzung nur fuer wntmsci

ASM=ml
AFLAGS=/c /Cp /coff
OLE2ANSI=TRUE

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
ARCH_FLAGS*=

.IF "$(bndchk)" != ""
CXX*=nmcl
.ELSE
.IF "$(truetime)" != ""
CXX*=nmcl /NMttOn
.ELSE
CXX*:=$(WRAPCMD) cl
.ENDIF
.ENDIF # "$(bndchk)" != ""

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

# Flags for COMEX == 10
CFLAGS+=-Zm500 -wd4251 -wd4275 -wd4290 -wd4786 -wd4800 -Zc:forScope -GR

.IF "$(product)" != ""
CDEFS+= -D_X86_=1
CFLAGS+=-c -nologo -Gs -Gy $(NOLOGO) $(MINUS_I)$(INCLUDE)
.IF "$(bndchk)" == ""
.IF "$(VC_STANDARD)"==""
CFLAGS+= -Ob1
.ENDIF	# "$(VC_STANDARD)"==""
.ENDIF
.ELSE	# "$(product)" != ""
CDEFS+= -D_X86_=1
CFLAGS+=-c -nologo -Gs $(NOLOGO) $(MINUS_I)$(INCLUDE)
.IF "$(bndchk)" == ""
.IF "$(VC_STANDARD)"==""
CFLAGS+= -Ob1
.ENDIF	# "$(VC_STANDARD)"==""
.ENDIF
.ENDIF # "$(product)" != ""

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

CFLAGSCC=$(ARCH_FLAGS)
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
.IF "$(VC_STANDARD)"==""
CFLAGSOPT=-Oxs -Oy-
CFLAGSNOOPT=-Od
.ELSE			#  "$(VC_STANDARD)"==""
CFLAGSOPT=
CFLAGSNOOPT=
.ENDIF			#  "$(VC_STANDARD)"==""
CFLAGSOUTOBJ=-Fo

# Flags for COMEX == 10
CFLAGSWALL=-Wall -wd4294 -wd4640
CFLAGSDFLTWARN=-W3
CDEFS+=-DSTLPORT_VERSION=400 -DWINVER=0x400 -D_WIN32_IE=0x400
CDEFS+=-D_MT

COMMENTFLAG=/COMMENT:"$(PRJNAME)_$(UPD)_$(VCSID)_"

LINK=$(WRAPCMD) link $(COMMENTFLAG) $(NOLOGO) /MACHINE:IX86
LINKOUTPUTFILTER= |& $(GREP) -v "LNK4197:"
.IF "$(PRODUCT)"!="full"
.ELSE
LINKFLAGS=/MAP /OPT:NOREF
.ENDIF

.IF "$(linkinc)" != ""
LINKFLAGS=/NODEFAULTLIB /DEBUG:full /DEBUGTYPE:cv /INCREMENTAL:YES
MAPFILE=
.ELSE # "$(linkinc)" != ""
.IF "$(PRODUCT)"!="full"
LINKFLAGS+= /NODEFAULTLIB /DEBUG:full /DEBUGTYPE:cv
.ELSE # "$(PRODUCT)"!="full"
LINKFLAGS+= /NODEFAULTLIB /RELEASE /DEBUG:full
.ENDIF # "$(PRODUCT)"!="full"
MAPFILE=-out:$$@
.ENDIF # "$(linkinc)" != ""

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

LIBSTLPORT=stlport_vc71.lib
LIBSTLPORTST=stlport_vc71_static.lib

ATL_INCLUDE*=$(COMPATH)$/atlmfc$/include 
ATL_LIB*=$(COMPATH)$/atlmfc$/lib 

LIBMGR=$(WRAPCMD) lib $(NOLOGO)
IMPLIB=$(WRAPCMD) lib
LIBFLAGS=

IMPLIBFLAGS=-machine:IX86

MAPSYM=
MAPSYMFLAGS=

RC=$(WRAPCMD) rc
RCFLAGS=-r -DWIN32 -fo$@ $(RCFILES)
RCLINK=rc
RCLINKFLAGS=
RCSETVERSION=


DLLPOSTFIX=mi

CSC*=$(WRAPCMD) csc
VBC*=$(WRAPCMD) vbc
