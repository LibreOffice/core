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
'* short description : New chart types
'*
'************************************************************************
'*
' #1 tCreateNewFilledNetChart      ' Create a new filled netchart and check the preserved values
'*
'\************************************************************************

testcase tCreateNewFilledNetChart
    Dim sInputFile as STRING
    Dim sOutputFile as STRING
    Dim sControlString as STRING
    Dim iChartVariant as integer
    Dim iChartVariantText as string
    
   
    sInputFile = convertpath ( gTesttoolPath & "chart2/optional/input/randomData.ods" )
    sOutputFile = convertpath ( gOfficePath & "user/work/tCreateNewFilledChart.ods" )
	
	    
    printlog "Load test document TesttoolPath/chart2/optional/input/randomData.ods"
    call hFileOpen(sInputFile)
    sleep(2)    
    printlog "Save document as OfficePath/user/work/tCreateNewFilledNetChart.ods"
    if NOT hFileSaveAsWithFilterKill ( sOutputFile , "calc8" ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    endif    
    printlog "Insert / Chart"
    InsertChart
    sleep(5)        
    Kontext "ChartType"
	if NOT ChartType.Exists(5) then
		warnlog "Chart Wizard was not visible. EXITING!"
        goto endsub
    else
        printlog "Chart Wizard visible."
    end if
    printlog "Choose the Chart type net (should be the eights in the top-down list)"
    if fSetChartType( "net" ) > 0 then
        warnlog "Something went wrong setting chart type to BAR."
    endif    
    printlog "Select 4th chart variant from left (filled)"
    if fSetChartTypeVariant( 4 ) > 0 then
        warnlog "Something went wrong setting chart type VARIANT."
    endif
	printlog "Create Chart by clicking Finish-button"
    Kontext "ChartWizard"
    ChartWizard.OK    
    sleep(2)    
    printlog "File / Save"
    FileSave
    Kontext "DocumentCalc"
    printlog "Press twice <ESCAPE>-key"
    DocumentCalc.TypeKeys "<Escape>" , 2                
    printlog "File / Close document"
    Call hCloseDocument
    sleep(2)    
    printlog "Open saved file again"
    call hFileOpen(sOutputFile)
    sleep(2)    
    printlog "Open the Navigator (F5)"    
    printlog "Select the first OLE object in the Navigator"
    call fSelectFirstOLE    
    printlog "Edit / Object / Edit to enter Inplace Mode"
    EditObjectEdit    
    printlog "Format / Chart Type"    
    FormatChartType
    Kontext "ChartType"
    if ChartType.Exists(2) then        
        printlog "Check if chart type is NET CHART"
        if ChooseType.GetSelIndex <> 8 then
            warnlog "Selected chart type wasn't preserved after save and reload."
        endif        
        printlog "Check if chart variant is Filled Net CHART (4th variant in non-CTL versions from the left)"
        if Variant.GetSelIndex <> 4 then
            warnlog "Selected chart type variant wasn't preserved after save and reload."
        end if            
        printlog "Leave Chart Type dialog with Cancel"
        Kontext "ChartType"
        if ChartType.Exists(2) then
	        ChartType.Cancel
	    else
	        warnlog "ChartType dialog was not visible!?"
	    end if        
    else
        warnlog "Chart Type dialog was not up!"
    end if
	printlog "Press ESCAPE button (deselecting the OLE object)"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"    
    printlog "Close document"
    Call hCloseDocument
endcase
