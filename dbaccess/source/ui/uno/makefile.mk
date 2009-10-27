#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.16 $
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
#************************************************************************

PRJ=..$/..$/..
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=uiuno

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files -------------------------------------

# ... resource files ............................

SRS1NAME=$(TARGET)
SRC1FILES =	\
        dbinteraction.src \
        copytablewizard.src

# ... object files ............................
# workaround for issue http://qa.openoffice.org/issues/show_bug.cgi?id=102305 Linux specific
.IF "$(COM)" == "GCC"
NOOPTFILES=\
        $(SLO)$/copytablewizard.obj
.ENDIF

SLOFILES=	\
        $(SLO)$/copytablewizard.obj \
        $(SLO)$/composerdialogs.obj	\
        $(SLO)$/unosqlmessage.obj	\
        $(SLO)$/unoadmin.obj	\
        $(SLO)$/admindlg.obj	\
        $(SLO)$/TableFilterDlg.obj	\
        $(SLO)$/AdvancedSettingsDlg.obj	\
        $(SLO)$/unoDirectSql.obj	\
        $(SLO)$/DBTypeWizDlg.obj	\
        $(SLO)$/DBTypeWizDlgSetup.obj	\
        $(SLO)$/UserSettingsDlg.obj \
        $(SLO)$/ColumnModel.obj	\
        $(SLO)$/ColumnControl.obj	\
        $(SLO)$/ColumnPeer.obj	\
        $(SLO)$/dbinteraction.obj \
        $(SLO)$/textconnectionsettings_uno.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk
