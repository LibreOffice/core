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
PRJ=..$/..
PRJNAME=cui
TARGET=dialogs
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_LAYOUT)" == "TRUE"
CFLAGS+= -DENABLE_LAYOUT=1 -I../$(PRJ)/layout/inc -I../$(PRJ)/layout/$(INPATH)/inc
.ENDIF # ENABLE_LAYOUT == TRUE

# --- Files --------------------------------------------------------

SRS1NAME=dialogs
SRC1FILES =  \
        about.src \
        charmap.src \
        commonlingui.src \
        cuiimapdlg.src \
        dlgname.src \
        fmsearch.src \
        gallery.src \
        grfflt.src \
        hangulhanjadlg.src \
        hyphen.src \
        hlmarkwn.src \
        hyperdlg.src \
        iconcdlg.src \
        insrc.src \
        multipat.src \
        newtabledlg.src \
        passwdomdlg.src \
        postdlg.src \
        scriptdlg.src \
        sdrcelldlg.src \
        showcols.src \
        SpellDialog.src \
        splitcelldlg.src \
        srchxtra.src \
        svuidlg.src \
        tbxform.src \
        thesdlg.src \
        zoom.src \


SLOFILES+=\
        $(SLO)$/about.obj \
        $(SLO)$/commonlingui.obj \
        $(SLO)$/cuicharmap.obj \
        $(SLO)$/cuifmsearch.obj \
        $(SLO)$/cuigaldlg.obj \
        $(SLO)$/cuigrfflt.obj \
        $(SLO)$/cuihyperdlg.obj \
        $(SLO)$/cuiimapwnd.obj \
        $(SLO)$/cuitbxform.obj \
        $(SLO)$/dlgname.obj \
        $(SLO)$/hangulhanjadlg.obj \
        $(SLO)$/hldocntp.obj \
        $(SLO)$/hldoctp.obj \
        $(SLO)$/hlinettp.obj \
        $(SLO)$/hlmailtp.obj \
        $(SLO)$/hlmarkwn.obj \
        $(SLO)$/hltpbase.obj \
        $(SLO)$/hyphen.obj \
        $(SLO)$/iconcdlg.obj \
        $(SLO)$/insdlg.obj \
        $(SLO)$/insrc.obj \
        $(SLO)$/linkdlg.obj \
        $(SLO)$/multifil.obj \
        $(SLO)$/multipat.obj \
        $(SLO)$/newtabledlg.obj \
        $(SLO)$/passwdomdlg.obj \
        $(SLO)$/pastedlg.obj \
        $(SLO)$/plfilter.obj \
        $(SLO)$/postdlg.obj \
        $(SLO)$/scriptdlg.obj \
        $(SLO)$/sdrcelldlg.obj \
        $(SLO)$/showcols.obj \
        $(SLO)$/SpellAttrib.obj \
        $(SLO)$/SpellDialog.obj \
        $(SLO)$/splitcelldlg.obj \
        $(SLO)$/srchxtra.obj \
        $(SLO)$/thesdlg.obj \
        $(SLO)$/zoom.obj \


.IF "$(GUI)"=="WNT"
SLOFILES+=$(SLO)$/winpluginlib.obj
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
