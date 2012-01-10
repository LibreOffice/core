#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************


PRJ=..$/..$/..

PRJNAME=oovapi

TARGET=excel
PACKAGE=ooo$/vba$/excel

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------


IDLFILES= XGlobals.idl\
    Globals.idl\
    SheetObject.idl\
    SheetObjects.idl\
    XApplication.idl\
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
    Window.idl \
     XHyperlink.idl \
     Hyperlink.idl \
     XHyperlinks.idl \
     XPageSetup.idl \
     XPageBreak.idl \
     XHPageBreak.idl \
     XHPageBreaks.idl \
     XVPageBreak.idl \
     XVPageBreaks.idl \
     TextFrame.idl \


# ------------------------------------------------------------------

.INCLUDE :  target.mk

