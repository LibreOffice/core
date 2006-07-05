#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: wntmsci10.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: kz $ $Date: 2006-07-05 22:06:01 $
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

CFLAGS+=-Zm500 -Zc:forScope -GR
CFLAGS+=-c -nologo -Gs $(NOLOGO)

CDEFS+= -D_X86_=1

.IF "$(product)" != ""
CFLAGS+= -Gy
.ENDIF # "$(product)" != ""

.IF "$(bndchk)" == ""
.IF "$(VC_STANDARD)"==""
CFLAGS+= -Ob1
.ENDIF	# "$(VC_STANDARD)"==""
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

# For C and C++, certain warnings are disabled globally, as they result in
# spurious warnings and are hard or impossible to workaround:
# - "warning C4061: enumerate in switch of enum is not explicitly handled by a
#   case label",
# - "warning C4127: conditional expression is constant",
# - "warning C4191: unsafe conversion from function type to function type",
# - "warning C4217: member template functions cannot be used for copy-assignment
#   or copy-construction",
# - "warning C4355: 'this' used in base member initializer list",
# - "warning C4511: copy constructor could not be generated",
# - "warning C4512: assignment operator could not be generated",
# - "warning C4514: unreferenced inline function has been removed",
# - "warning C4611: interaction between '_setjmp' and C++ object destruction is
#   non-portable",
# - "warning C4625: copy constructor could not be generated because a base class
#   copy constructor is inaccessible",
# - "warning C4626: assignment operator could not be generated because a base
#   class assignment operator is inaccessible",
# - "warning C4675: resolved overload was found by argument-dependent lookup",
# - "warning C4710: function not inlined",
# - "warning C4711: function selected for automatic inline expansion",
# - "warning C4820: padding added after member".
# - "warning C4503: 'identifier' : decorated name length exceeded, name was truncated"
#   (http://msdn2.microsoft.com/en-us/library/074af4b6.aspx)
# For C, certain warnings from system headers (stdlib.h etc.) have to be
# disabled globally (for C++, this is not necessary, as the system headers are
# wrapped by STLport):
# - "warning C4255: no function prototype given: converting
#   '()' to '(void)'".
CFLAGSWARNCXX=-Wall -wd4061 -wd4127 -wd4191 -wd4217 -wd4251 -wd4275 -wd4290 \
    -wd4294 -wd4355 -wd4511 -wd4512 -wd4514 -wd4611 -wd4625 -wd4626 -wd4640 \
    -wd4675 -wd4710 -wd4711 -wd4786 -wd4800 -wd4820 -wd4503
CFLAGSWARNCC=$(CFLAGSWARNCXX) -wd4255
CFLAGSWALLCC=$(CFLAGSWARNCC)
CFLAGSWALLCXX=$(CFLAGSWARNCXX)
CFLAGSWERRCC=-WX

# Once all modules on this platform compile without warnings, set
# COMPILER_WARN_ERRORS=TRUE here instead of setting MODULES_WITH_WARNINGS (see
# settings.mk):
MODULES_WITH_WARNINGS := \
    automation \
    avmedia \
    b_server \
    basctl \
    basic \
    binfilter \
    canvas \
    chart2 \
    connectivity \
    cppcanvas \
    customres \
    databaseext \
    dbaccess \
    desktop \
    devtools \
    dxcanvas \
    embeddedobj \
    extensions \
    extras_full \
    filter \
    finalize \
    forms \
    fpicker \
    framework \
    glcanvas \
    goodies \
    helpcontent2 \
    instset_native \
    instsetoo_native \
    lingu \
    lingucomponent \
    linguistic \
    macromigration \
    migrationanalysis \
    officecfg \
    postprocess \
    r_tools \
    sc \
    sch \
    scripting \
    sd \
    sfx2 \
    sj2 \
    slideshow \
    smoketest_native \
    smoketestoo_native \
    so3 \
    starmath \
    svtools \
    svx \
    sw \
    tab \
    toolkit \
    top \
    uui \
    wizards \
    writerperfect \
    xmlhelp \
    xmloff \
    xmlsecurity

CDEFS+=-DSTLPORT_VERSION=400 -DWINVER=0x400 -D_WIN32_IE=0x400
CDEFS+=-D_MT

COMMENTFLAG=/COMMENT:"$(PRJNAME)_$(UPD)_$(VCSID)_"

LINK=$(WRAPCMD) link $(COMMENTFLAG) $(NOLOGO) /MACHINE:IX86
LINKOUTPUTFILTER= $(PIPEERROR) $(GREP) -v "LNK4197:"
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

UWINAPILIB*=uwinapi.lib
.IF "$(DYNAMIC_CRT)"!=""
LIBCST=msvcrt.lib
.IF "$(USE_STLP_DEBUG)" != ""
LIBCMT=msvcrtd.lib
.ELSE  # "$(USE_STLP_DEBUG)" != ""
LIBCMT=msvcrt.lib
.ENDIF # "$(USE_STLP_DEBUG)" != ""
# See iz57443 for details about LIBCIMT
#LIBCIMT=
.ELSE
LIBCST=libc.lib
LIBCMT=libcmt.lib
LIBCIMT=libcimt.lib
.ENDIF

STDOBJVCL=$(L)$/salmain.obj
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

STDLIBGUIST=$(LIBCST) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDLIBCUIST=$(LIBCST) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDLIBGUIMT=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDLIBCUIMT=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDSHLGUIST=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDSHLCUIST=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDSHLGUIMT=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDSHLCUIMT=$(LIBCMT) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib

.IF "$(USE_STLP_DEBUG)" != ""
CFLAGS+=-MTd
LIBSTLPORT=stlport_vc71_stldebug.lib
LIBSTLPORTST=stlport_vc71_stldebug_static.lib
.ELSE
LIBSTLPORT=stlport_vc71.lib
LIBSTLPORTST=stlport_vc71_static.lib
.ENDIF

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
