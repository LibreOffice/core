#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: vg $ $Date: 2003-04-24 12:53:39 $
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

PRJ=..

PRJNAME=sd
TARGET=sdraw3
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

#IENV!:=$(IENV);..$/res

.IF "$(GUI)"=="WIN"
LINKFLAGS+=/PACKCODE:65500 /SEG:16000 /NOE /NOD /MAP
MAPSYM=tmapsym
.ENDIF

.IF "$(COM)"=="ICC"
LINKFLAGS+=/SEGMENTS:512 /PACKD:32768
.ENDIF

# --- Resourcen ----------------------------------------------------

.IF "$(GUI)"=="WIN"
RESLIBSPLIT1NAME=sd
RESLIBSPLIT1SRSFILES= \
     $(SRS)$/app.srs $(SRS)$/dlg.srs $(SRS)$/core.srs $(SRS)$/html.srs $(SRS)$/sdslots.srs \
     $(SRS)$/accessibility.srs $(SOLARRESDIR)$/sfx.srs
.ELSE
RESLIB1NAME=sd
RESLIB1SRSFILES=\
     $(SRS)$/app.srs $(SRS)$/dlg.srs $(SRS)$/core.srs $(SRS)$/html.srs $(SRS)$/sdslots.srs \
     $(SRS)$/accessibility.srs $(SOLARRESDIR)$/sfx.srs
.ENDIF

#.IF "$(solarlang)" == "deut"
#SRC1FILES=      ..$/source$/ui$/app$/appmain.src
#SRS1NAME =      appmain
#SRS1FILES=$(SRS)$/appmain.srs \
#	 $(SOLARRESDIR)$/sfx.srs
#RES1TARGET= sdappi
#.ENDIF

# --- StarDraw DLL

SHL1TARGET= sd$(UPD)$(DLLPOSTFIX)
SHL1VERSIONMAP= sd.map
SHL1IMPLIB= sdi

# static libraries
SHL1STDLIBS= $(SCHLIB) $(SCLIB) $(SMLIB)

# dynamic libraries
SHL1STDLIBS+= \
    $(OFALIB) \
    $(SVXLIB) \
    $(SFXLIB) \
    $(BASICLIB) \
    $(GOODIESLIB) \
    $(SO2LIB) \
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(VCLLIB) \
    $(SVLLIB) \
    $(SOTLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(VOSLIB) \
    $(SALLIB)

.IF "$(GUI)" == "MAC"
SHL1STDLIBS += \
            $(SOLARBINDIR)$/SDB$(UPD)$(DLLPOSTFIX).DLL \
            $(SOLARLIBDIR)$/plugctor.lib
.ENDIF

SHL1DEPN=   $(L)$/itools.lib
SHL1LIBS=   $(LIB3TARGET)


SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
.IF "$(GUI)" == "WNT" || "$(GUI)" == "WIN"
SHL1RES=    $(RCTARGET)
.ENDIF

# --- Linken der Applikation ---------------------------------------

LIB2TARGET=$(SLB)$/sdmod.lib
LIB2OBJFILES=   \
            $(SLO)$/sdmod1.obj      \
            $(SLO)$/sdmod2.obj      \
            $(SLO)$/sdmod.obj

LIB3TARGET=$(SLB)$/sdraw3.lib
LIB3FILES=      \
            $(SLB)$/view.lib        \
            $(SLB)$/app.lib			\
            $(SLB)$/func.lib        \
            $(SLB)$/docshell.lib    \
            $(SLB)$/dlg.lib			\
            $(SLB)$/core.lib		\
            $(SLB)$/xml.lib			\
            $(SLB)$/cgm.lib			\
            $(SLB)$/grf.lib			\
            $(SLB)$/bin.lib			\
            $(SLB)$/html.lib		\
            $(SLB)$/filter.lib		\
            $(SLB)$/unoidl.lib		\
            $(SLB)$/accessibility.lib		


LIB4TARGET=$(LB)$/sdlib.lib
LIB4ARCHIV=$(LB)$/libsdlib.a
.IF "$(GUI)"=="UNX"
LIB4OBJFILES=$(OBJ)$/sdlib.obj \
          $(OBJ)$/sdresid.obj
.ELSE
.IF "$(GUI)"=="MAC"
LIB4OBJFILES=$(OBJ)$/sdlib.obj \
          $(OBJ)$/sdresid.obj
.ELSE
LIB4OBJFILES=$(OBJ)$/sdlib.obj \
          $(OBJ)$/sdresid.obj
.ENDIF
.ENDIF


.IF "$(depend)" == ""
ALL:    \
    ALLTAR
.ENDIF


.IF "$(SVXLIGHT)" != "" 
LIB5TARGET= $(LB)$/sdl.lib
LIB5ARCHIV= $(LB)$/libsdl.a
LIB5FILES=  \
            $(LB)$/sxl_core.lib\
            $(LB)$/sxl_unoidl.lib
.ENDIF

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(depend)" == ""

# -------------------------------------------------------------------------
# MAC
# -------------------------------------------------------------------------

.IF "$(GUI)" == "MAC"

$(MISCX)$/$(APP1TARGET).def : makefile.mk


$(MISC)$/$(SHL1TARGET).def:  makefile.mk
        delete -i $@.exp
        $(LINK)  $(LINKFLAGS) $(LINKFLAGSSHL) $(SHL1OBJS) $(SHL1LIBS) -f $@.exp · dev:null
        duplicate -y $@.exp $@
.ENDIF
# -------------------------------------------------------------------------
# Windows 3.1
# -------------------------------------------------------------------------

.IF "$(GUI)" == "WIN"

$(MISC)$/$(SHL1TARGET).def:  makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET)                                  >$@
    @echo DESCRIPTION 'SDRAW3 DLL'                                 >>$@
    @echo EXETYPE     WINDOWS                                       >>$@
    @echo PROTMODE                                                  >>$@
    @echo CODE        LOADONCALL MOVEABLE DISCARDABLE               >>$@
    @echo DATA        PRELOAD MOVEABLE SINGLE                       >>$@
    @echo HEAPSIZE    0                                             >>$@
    @echo EXPORTS                                                   >>$@
    @echo _CreateSdDrawDocShellDll @2                              >>$@
    @echo _CreateSdGraphicDocShellDll @3                           >>$@
    @echo _CreateObjSdDrawDocShellDll @4                           >>$@
    @echo _CreateObjSdGraphicDocShellDll @5                        >>$@
    @echo _InitSdDll @6                                            >>$@
    @echo _DeInitSdDll @7                                          >>$@
    @echo component_getImplementationEnvironment 				   >>$@
    @echo component_writeInfo									   >>$@
    @echo component_getFactory									   >>$@
.ENDIF

.IF "$(GUI)" == "WNT"

$(MISC)$/$(SHL1TARGET).def:
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET)                                  >$@
    @echo DESCRIPTION 'SDRAW3 DLL'                                 >>$@
    @echo DATA        READ WRITE NONSHARED                          >>$@
    @echo EXPORTS                                                   >>$@
    @echo   CreateSdDrawDocShellDll @20                            >>$@
    @echo   CreateSdGraphicDocShellDll @21                         >>$@
    @echo   CreateObjSdDrawDocShellDll @22                         >>$@
    @echo   CreateObjSdGraphicDocShellDll @23                      >>$@
    @echo   InitSdDll @24                                          >>$@
    @echo   DeInitSdDll @25                                        >>$@
    @echo component_getImplementationEnvironment 				   >>$@
    @echo component_writeInfo									   >>$@
    @echo component_getFactory									   >>$@
.ENDIF

# -------------------------------------------------------------------------
# Presentation Manager 2.0
# -------------------------------------------------------------------------

.IF "$(GUI)" == "OS2"

$(MISC)$/$(SHL1TARGET).def:  makefile.mk
    @echo ================================================================
    @echo building $@
    @echo ----------------------------------------------------------------
.IF "$(COM)"!="WTC"
    echo  LIBRARY           INITINSTANCE TERMINSTANCE                       >$@
    echo  DESCRIPTION   'SdDLL'                            >>$@
    echo  PROTMODE                                                                             >>$@
    @echo CODE        LOADONCALL                                  >>$@
    @echo DATA                PRELOAD MULTIPLE NONSHARED                                      >>$@
    @echo EXPORTS                                              >>$@
.IF "$(COM)"!="ICC"
    @echo _CreateSdDrawDocShellDll @2                              >>$@
    @echo _CreateSdGraphicDocShellDll @3                           >>$@
    @echo _CreateObjSdDrawDocShellDll @4                           >>$@
    @echo _CreateObjSdGraphicDocShellDll @5                        >>$@
    @echo _InitSdDll @6                                            >>$@
    @echo _DeInitSdDll @7                                          >>$@
.ELSE
    @echo CreateSdDrawDocShellDll @2                              >>$@
    @echo CreateSdGraphicDocShellDll @3                           >>$@
    @echo CreateObjSdDrawDocShellDll @4                           >>$@
    @echo CreateObjSdGraphicDocShellDll @5                        >>$@
    @echo InitSdDll @6                                            >>$@
    @echo DeInitSdDll @7                                          >>$@
.ENDIF
.ELSE
    @echo option DESCRIPTION 'SdDLL'                            >$@
    @echo name $(BIN)$/$(SHL1TARGET).dll                         >>$@
    @echo CreateSdDrawDocShellDll_ @2      >>temp.def
    @echo CreateSdGraphicDocShellDll_ @3   >>temp.def
    @echo CreateObjSdDrawDocShellDll_ @4   >>temp.def
    @echo CreateObjSdGraphicDocShellDll_ @5   >>temp.def
    @echo InitSdDll_ @6                    >>temp.def
    @echo DeInitSdDll_ @7                  >>temp.def
    @gawk -f s:\util\exp.awk temp.def                               >>$@
    del temp.def
.ENDIF
.ENDIF

.ENDIF

$(MISCX)$/$(SHL1TARGET).flt:
    @echo ------------------------------
    @echo Making: $@
    @echo WEP>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
