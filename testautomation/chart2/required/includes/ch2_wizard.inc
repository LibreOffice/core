'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : Create new Chart using the Wizard
'*
'************************************************************************
'*
' #1 tCreateNewChart
'*
'\************************************************************************

testcase tCreateNewChart
'///<u><b>Test creation of new chart using the Chart Wizard</b></u>
   dim sInputFile as string
   dim sOutputFile as string
   sInputFile = convertpath ( gtesttoolpath & "chart2/required/input/spreadsheetFile.ods" )
   sOutputFile = convertpath ( gOfficepath & "user/work/spreadsheetFile.ods" )

   '/// Load test document <i>gTesttoolPath</i>/chart2/required/input/spreadsheetFile.ods
   call hFileOpen(sInputFile)
   sleep(2)
   '/// Save the document local <i>gOfficepath</i>user/work/spreadsheetFile[.ods]
   if NOT hFileSaveAsWithFilterKill ( sOutputFile , "calc8" ) then
       warnlog "Saving test document localy failed -> Aborting"
       call hCloseDocument
       goto endsub
   endif
   '/// Insert / Chart
   InsertChart
   sleep(5)   
   Kontext "ChartWizard"
   Call DialogTest ( ChartWizard )
   '/// In the Chart wizard switch to the <i>Chart Type</i> page
   Kontext "ChartType"
   Call DialogTest ( ChartType )
   '/// Switch to the <i>Data Range</i> page using <DOWN>-key
   Kontext "ChartWizard"
   GoNext.Click   
   Kontext "TabChartTypeDataRange"
   Call DialogTest ( TabChartTypeDataRange )
   '/// Switch to the <i>Data Series</i> page using <i>Next >></i> button
   Kontext "ChartWizard"
   GoNext.Click
   Kontext "TabChartTypeDataSeries"
   Call DialogTest ( TabChartTypeDataSeries )
   '/// Switch to the <i>Chart Elements</i> page using <i>Next >></i> button
   Kontext "ChartWizard"
   GoNext.Click   
   Kontext "TabChartTypeChartElements"
   Call DialogTest ( TabChartTypeChartElements )
   '/// Create Chart using <i>Finish</i> button
   Kontext "ChartWizard"
   ChartWizard.OK
   sleep(2)
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   '/// Close document
   Call hCloseDocument
endcase

