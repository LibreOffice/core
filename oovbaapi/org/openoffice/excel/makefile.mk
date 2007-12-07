#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: vg $ $Date: 2007-12-07 12:14:13 $
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

