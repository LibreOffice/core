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
