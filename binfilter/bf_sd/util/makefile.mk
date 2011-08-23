#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE
PRJ=..$/..
BFPRJ=..

PRJNAME=binfilter
TARGET=sdraw3

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_sd

.IF "$(GUI)"=="WIN"
LINKFLAGS+=/PACKCODE:65500 /SEG:16000 /NOE /NOD /MAP
MAPSYM=tmapsym
.ENDIF

.IF "$(COM)"=="ICC"
LINKFLAGS+=/SEGMENTS:512 /PACKD:32768
.ENDIF

# --- Resourcen ----------------------------------------------------

RESLIB1NAME=bf_sd
RESLIB1SRSFILES=\
         $(SRS)$/sd_app.srs $(SRS)$/sd_core.srs  

# --- StarDraw DLL

SHL1TARGET= bf_sd$(DLLPOSTFIX)
SHL1VERSIONMAP= bf_sd.map
SHL1IMPLIB= bf_sdi

# static libraries
SHL1STDLIBS= $(BFSCHLIB) $(BFSCLIB) $(BFSMLIB)

# dynamic libraries
SHL1STDLIBS+= \
    $(BFSVXLIB) \
    $(LEGACYSMGRLIB)	\
    $(BFSO3LIB) \
    $(BFSVTOOLLIB) \
    $(TKLIB) \
    $(VCLLIB) \
    $(SOTLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(I18NISOLANGLIB) \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)


SHL1LIBS=   $(LIB3TARGET)


SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
.IF "$(GUI)" == "WNT" || "$(GUI)" == "WIN"
SHL1RES=    $(RCTARGET)
.ENDIF

# --- Linken der Applikation ---------------------------------------

LIB2TARGET=$(SLB)$/sd_sdmod.lib
LIB2OBJFILES=   \
            $(SLO)$/sd_sdmod.obj

LIB3TARGET=$(SLB)$/sd_sdraw3.lib
LIB3FILES=      \
            $(SLB)$/sd_view.lib			\
            $(SLB)$/sd_app.lib			\
            $(SLB)$/sd_docshell.lib    \
            $(SLB)$/sd_core.lib		\
            $(SLB)$/sd_xml.lib			\
            $(SLB)$/sd_bin.lib			\
            $(SLB)$/sd_filter.lib		\
            $(SLB)$/sd_unoidl.lib

LIB4TARGET=$(LB)$/bf_sdlib.lib
LIB4ARCHIV=$(LB)$/libbf_sdlib.a
LIB4OBJFILES=$(SLO)$/sd_sdlib.obj \
          $(SLO)$/sd_sdresid.obj

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(depend)" == ""

.IF "$(GUI)" == "WNT"

$(MISC)$/$(SHL1TARGET).def:
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET)                                  >$@
.IF "$(COM)"!="GCC"
    @echo DESCRIPTION 'SDRAW3 DLL'                                 >>$@
    @echo DATA        READ WRITE NONSHARED                          >>$@
.ENDIF
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

.IF "$(GUI)" == "OS2"

$(MISC)$/$(SHL1TARGET).def:
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET8)  INITINSTANCE TERMINSTANCE    >$@
    @echo DESCRIPTION 'SDRAW3 DLL'                                 >>$@
    @echo DATA        MULTIPLE                                     >>$@
    @echo EXPORTS                                                   >>$@
    @echo   _CreateSdDrawDocShellDll                                >>$@
    @echo   _CreateSdGraphicDocShellDll                             >>$@
    @echo   _CreateObjSdDrawDocShellDll                             >>$@
    @echo   _CreateObjSdGraphicDocShellDll                          >>$@
    @echo   _InitSdDll                                              >>$@
    @echo   _DeInitSdDll                                            >>$@
    @echo _component_getImplementationEnvironment 		    >>$@
    @echo _component_writeInfo					    >>$@
    @echo _component_getFactory					    >>$@
.ENDIF

.ENDIF

$(MISCX)$/$(SHL1TARGET).flt:
    @echo ------------------------------
    @echo Making: $@
    @echo WEP>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
