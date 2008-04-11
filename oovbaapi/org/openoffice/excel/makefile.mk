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
# $Revision: 1.4 $
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
PRJ=..$/..$/..

PRJNAME=oovapi

TARGET=excel
PACKAGE=org$/openoffice$/Excel

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_VBA)"!="YES"
dummy:
        @echo "not building vba..."
.ENDIF

# ------------------------------------------------------------------------


IDLFILES= XApplication.idl\
    XComment.idl\
    XComments.idl\
    XRange.idl\
    XWorkbook.idl\
    XWorkbooks.idl\
    XWorksheet.idl\
    XWorksheets.idl\
    XFont.idl\
    XOutline.idl\
    XInterior.idl\
    XWorksheetFunction.idl\
    XWindow.idl\
    XChartObject.idl\
    XChartObjects.idl\
    XChart.idl\
    XCharts.idl\
    XSeriesCollection.idl\
    XSeries.idl\
    XDialog.idl \
    XPivotTables.idl \
    XPivotTable.idl \
    XPivotCache.idl \
    XDialogs.idl \
    XWindows.idl \
    XBorder.idl \
    XBorders.idl \
    XCharacters.idl \
    XValidation.idl\
    XOLEObject.idl\
    XOLEObjects.idl\
    XPane.idl\
    XTextFrame.idl\
    XTitle.idl\
    XChartTitle.idl\
    XDataLabel.idl\
    XDataLabels.idl\
    XAxes.idl\
    XAxis.idl\
    XAxisTitle.idl\
    XStyle.idl \
    XStyles.idl \
    XFormat.idl \
    XFormatCondition.idl \
    XFormatConditions.idl \
    Range.idl\
    XName.idl\
    XNames.idl \
    Workbook.idl \
    Worksheet.idl \


# ------------------------------------------------------------------

.INCLUDE :  target.mk

