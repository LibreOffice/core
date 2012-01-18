'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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

