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
TARGET=bf_svx

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

INC+= -I$(PRJ)$/inc$/bf_svx

# --- Svx - DLL ----------

SHL1TARGET= bf_svx$(DLLPOSTFIX)
SHL1IMPLIB= bf_svx
SHL1BASE  = 0x1d800000

SHL1STDLIBS= \
            $(BFSVTOOLLIB) \
            $(BFXMLOFFLIB) \
            $(BFGOODIESLIB) \
            $(BFBASICLIB) \
            $(BFSO3LIB) \
            $(TKLIB) \
            $(VCLLIB) \
            $(SOTLIB) \
            $(UNOTOOLSLIB) \
            $(TOOLSLIB) \
            $(I18NISOLANGLIB) \
            $(COMPHELPERLIB) \
            $(UCBHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(VOSLIB) \
            $(SALLIB) \
            $(ICUUCLIB)\
            $(LEGACYSMGRLIB) \
                        $(SVTOOLLIB) \
            $(XMLSCRIPTLIB)

.IF "$(GUI)"=="WNT"

SHL1STDLIBS+=\
        $(OLEAUT32LIB) \
        $(UWINAPILIB) \
        $(ADVAPI32LIB) \
        $(SHELL32LIB) \
        $(GDI32LIB) \
        $(OLE32LIB) \
        $(UUIDLIB)

SHL1STDLIBS+=\
            $(SHELLLIB)

SHL1DEPN=       $(SLB)$/svx_svx.lib
.ENDIF # WNT

SHL1LIBS=       $(SLB)$/svx_svx.lib

SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
DEF1NAME        =$(SHL1TARGET)
DEF1DEPN        =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME=svx_svx
DEF1DES		= Rtf, Edt, Outliner, SvDraw, Form, Fmcomp, Engine3D, MSFilter
# THB: exports list svx checked for 6.0 Final 6.12.2001
DEF1EXPORTFILE	= svx.dxp

LIB1TARGET      =$(SLB)$/svx_svx.lib
LIB1FILES       = \
        $(SLB)$/svx_dialogs.lib \
        $(SLB)$/svx_editeng.lib \
        $(SLB)$/svx_engine3d.lib \
        $(SLB)$/svx_form.lib \
        $(SLB)$/svx_items.lib \
        $(SLB)$/svx_options.lib \
        $(SLB)$/svx_outliner.lib \
        $(SLB)$/svx_svdraw.lib \
        $(SLB)$/svx_svxlink.lib \
        $(SLB)$/svx_unodraw.lib \
        $(SLB)$/svx_unoedit.lib \
        $(SLB)$/svx_xml.lib \
        $(SLB)$/svx_xout.lib \
        $(SLB)$/bf_sfx.lib

.IF "(GUIBASE)" == "WIN"
LIB1FILES  += \
            $(SLB)$/ibrwimp.lib
.ENDIF


SRS1FILELIST=\
                $(SRS)$/svx_svdstr.srs      \
                $(SRS)$/svx_editeng.srs     \
                $(SRS)$/svx_outliner.srs \
                $(SRS)$/svx_dialogs.srs     \
                $(SRS)$/svx_drawdlgs.srs \
                $(SRS)$/svx_svxitems.srs \
                $(SRS)$/svx_engine3d.srs \
                $(SRS)$/svx_svxlink.srs

RESLIB1NAME=bf_svx
RESLIB1SRSFILES= $(SRS1FILELIST)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    $(TYPE) bf_svx.flt >$@
    $(TYPE) bf_sfxwin.flt > $@

