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
'* short description : Chart wizard general functionality
'*
'************************************************************************
'*
' #1 tCreateNew3DChart      ' Create a new 3D chart and check the preserved values
' #1 tCreateNewLineChart    ' Create a new line chart and check the preserved values
'*
'\************************************************************************

testcase tCreateNew3DChart
    Dim sInputFile as STRING
    Dim sOutputFile as STRING
    Dim sControlString as STRING
    Dim iChartVariant as integer
    Dim iChartVariantText as string
    
   
    sInputFile = convertpath ( gTesttoolPath & "chart2/optional/input/randomData.ods" )
    sOutputFile = convertpath ( gOfficePath & "user/work/tCreateNew3DChart.ods" )
    
    printlog "Load test document TesttoolPath/chart2/optional/input/randomData.ods"
    call hFileOpen(sInputFile)
    sleep(2)    
    printlog "Save document as OfficePath/user/work/tCreateNew3DChart.ods"
    if NOT hFileSaveAsWithFilterKill ( sOutputFile , "calc8" ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    endif    
    printlog "Insert / Chart"
    InsertChart
    sleep(5)        
    Kontext "ChartType"    
    printlog "Choose the Chart type bar (should be the second in the top-down list)"
    if fSetChartType( "bar" ) > 0 then
        warnlog "Something went wrong setting chart type to BAR."
    endif    
    printlog "Select 2nd chart variant from left"
    if fSetChartTypeVariant( 2 ) > 0 then
        warnlog "Something went wrong setting chart type VARIANT."
    endif    
    printlog "Check 3D Look checkbox"
    if fSetChartType3D( TRUE ) > 0 then
        warnlog "Something went wrong setting 3D chart."
    endif    
    printlog "Change Chart type look to simple"
    if fSetChartLook3D( "simple" ) > 0 then
       warnlog "Something went wrong setting 3D look."
    endif    
    printlog "Change Chart shape to cone"
    if fSetChartShape3D( "cone" ) > 0 then
        warnlog "Something went wrong setting 3D shape."
    endif    
    printlog "Change to Data Range tab page by clicking the Next-button"
    Kontext "ChartWizard"
    GoNext.Click
    Kontext "TabChartTypeDataRange"    
    printlog "Set data range text box to $Sheet1.$A$2:$E$13"
    if fSetTextBox ( DataRange , "$Sheet1.$A$2:$E$13" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif   
    printlog "Select data series in Rows radiobox"
    if fSetRadioButton ( Rows ) > 0 then
        warnlog "Something went wrong selecting 'Data Series in: Rows'."
    endif    
    printlog "Uncheck First row as label checkbox"
    if fSetCheckBox ( FirstRowAsLabel , FALSE ) > 0 then
        warnlog "Something went wrong unselecting 'First row as label'."
    endif   
    printlog "Change to Data Series tab page by clicking the Next-button"
    Kontext "ChartWizard"
    GoNext.Click
    Kontext "TabChartTypeDataSeries"    
    printlog "Select last (12th) entry in Data Series list on the left"
    if fSetListBoxByItem ( DataSeries , 12 ) > 0 then
        warnlog "Something went wrong selecting 'Data Series' item no. 12."
    endif    
    printlog "Click on the add-button"
    AddButton.Click    
    printlog "Set Range for Name text field to A17"
    if fSetTextBox ( RangeFor , "A17" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif    
    printlog "Select 2nd entry in Data Ranges (Y-Values) list on the right"
    if fSetListBoxByItem ( DataRanges , 2 ) > 0 then
        warnlog "Something went wrong selecting 'Data Ranges' item No. 2."
    endif    
    printlog "Set Range for Y-Values text field to B17:E17"
    if fSetTextBox ( RangeFor , "B17:E17" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif    
    printlog "Change to Chart Elements tab-page by clicking the Next-button"
    Kontext "ChartWizard"
    GoNext.Click
    Kontext "TabChartTypeChartElements"    
    printlog "Set 'Title' text box to MyTitle"
    if fSetTextBox ( Title , "MyTitle" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif    
    printlog "Set 'Subtitle' text box to 1 2 3"
    if fSetTextBox ( Subtitle , "1 2 3" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif    
    printlog "Set 'X Title' text box to Grått"
    if fSetTextBox ( TitleXAxis , "Grått" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif    
    printlog "Set 'Y Title' text box to Gräulich"
    if fSetTextBox ( TitleYAxis , "Gräulich" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif    
    printlog "Set 'Z Title' text box to 灰色"
    if fSetTextBox ( TitleZAxis , "灰色" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif    
    printlog "Check X Axis-checkbox"
    if fSetCheckBox ( GridLinesXAxis , TRUE ) > 0 then
        warnlog "Something went wrong checking 'X Axis'."
    endif    
    printlog "Uncheck Y Axis-checkbox"
    if fSetCheckBox ( GridLinesYAxis , FALSE ) > 0 then
        warnlog "Something went wrong unchecking 'Y Axis'."
    endif    
    printlog "heck Z Axis-checkbox"
    if fSetCheckBox ( GridLinesZAxis , TRUE ) > 0 then
        warnlog "Something went wrong checking 'Z Axis'."
    endif    
    printlog "Select under Legend (on the right) the Left radio-button"
    if fSetRadioButton ( PositionLeft ) > 0 then
        warnlog "Something went wrong selecting 'Legend Display Left'."
    endif    
    printlog "Click the Finish-button"
    Kontext "ChartWizard"
    ChartWizard.OK
    sleep(2)    
    printlog "File / Save"
    FileSave
    Kontext "DocumentCalc"   
    printlog "Press ESCAPE button (deselecting the OLE object)"
    DocumentCalc.TypeKeys "<Escape>"   
    printlog "Close the document"
    Call hCloseDocument
    sleep(2)   
    printlog "Open saved file again"
    call hFileOpen(sOutputFile)
    sleep(2)   
    printlog "Select chart OLE using navigator"
    call fSelectFirstOLE   
    printlog "Edit / Object / Edit to enter inplace mode of the chart"
    printlog "(enabling the chart menus and editing possibilities)"
    EditObjectEdit    
    printlog "Format / Chart Type"
    FormatChartType
    sleep(1)   
    Kontext "ChartType"       
    printlog "Check if chart type is 'bar'"    
    if ChooseType.GetSelIndex <> 2 Then
        warnlog "Selected chart type (bar) wasn't preserved after save and reload."
    endif    
    printlog "Check that variant is 'stacked'"    
    iChartVariant = variant.GetSelIndex
    iChartVariantText = variant.GetSelText(iChartVariant)    
    printlog "Check that the chart variant is 'Stacked' (2nd entry)"
    if iChartVariant <> 2 then
        warnlog "Instead of chart variant 'Stacked' (2) the variant " & iChartVariant & "(" & iChartVariantText & ") has been used - FAILED" 
    end if    
    printlog "Check the the '3D Look' box is checked"
    if NOT Look3D.IsChecked then
        warnlog "Check box 3D (enabled) wasn't preserved after save and reload."
    endif    
    printlog "Check that 3D look is 'simple' (=default)"
    if Scheme3D.GetSelIndex <> 1 then
        warnlog "#i112522# - Default 3D Look ('simple') has not been preserved after save and reload."
    endif    
    printlog "Check that 3D shape 'cone' is selected"
    if BarColumnShape.GetSelIndex <> 3 Then
        warnlog "3D Shape CONE wasn't preserved after save and reload."
    endif   
	Kontext "ChartType"    
    printlog "Leave dialog with CANCEL button."
    ChartType.CANCEL    
    printlog "Invoke Format / Data Ranges"
    FormatDataRanges    
    printlog "Goto tab-page Data Range"    
    printlog "Verify now the made settings in the 'Data Ranges' dialog:"
    Kontext
    Active.SetPage TabFormatDataRangesDataRange
    Kontext "TabFormatDataRangesDataRange"
    if TabFormatDataRangesDataRange.Exists(2) then        
        printlog "Check substrings $A$2:$E$13 and $A$17:$E$17 in 'Data Range'"        
        sControlString = DataRange.GetText        
        if instr ( sControlString , "$A$2:$E$13" ) = 0 Then
            warnlog "Data range wasn't correctly preserved after save and reload and does not caontain $A$2:$E$13."
        endif
        if instr ( sControlString , "$A$17:$E$17" ) = 0 Then
            warnlog "#i72241# - Data range wasn't correctly preserved after save and reload and does not contain $A$17:$E$17."        
        endif        
        printlog "Check that radio button 'rows' is checked"
        if NOT Rows.IsChecked then
            warnlog "Radio Button ROWS (enabled) wasn't preserved after save and reload."
        endif        
        printlog "Check that radio button 'columns is unchecked."
        if Columns.IsChecked then
            warnlog "Radio Button COLUMNS (disabled) wasn't preserved after save and reload."
        endif        
        printlog "Check if 'First row as label' checkbox is unchecked"
        if FirstRowAsLabel.IsChecked then
            warnlog "Check box 'First row as label' (disabled) wasn't preserved after save and reload."
        endif        
        printlog "Check that 'First column as label' checkbox is checked."
        if NOT FirstColumnAsLabel.IsChecked then
            warnlog "#i72241# - Check box 'First column as label' (enabled) wasn't preserved after save and reload."
        endif
    else
        warnlog "Tab page 'Data Range' not visible!"
    end if    
    printlog "Go to tab page 'Data Series'"
    Kontext
    Active.SetPage TabFormatDataRangesDataSeries
    Kontext "TabFormatDataRangesDataSeries"
    if TabFormatDataRangesDataSeries.Exists(2) then        
        printlog "Check if added range (13 in list on the left) was preserved."
        if fSetListBoxByItem ( DataSeries , 13 ) > 0 then
            warnlog "Selecting 13th item in Data Series seems to fail. Added range wasn't preserved."
        else            
            printlog "Select 'Name'-entry in Data Ranges list on the right."
            if fSetListBoxByItem ( DataRanges , 1 ) > 0 then
                warnlog "Something went wrong selecting 'Data Ranges' item No. 1."
            else                
                printlog "Check that the string $A$17 exists in range for 'Name'"                
                sControlString = RangeFor.GetText
                printlog "'Range for name' is: " & sControlString
                if instr ( sControlString , "$A$17" ) = 0 Then
                    warnlog "#i72241# - 'Range for name' wasn't correctly preserved after save and reload."
                else                    
                    printlog "Select 'Y-Values' in Data Ranges list on the right."
                    if fSetListBoxByItem ( DataRanges , 2 ) > 0 then
                        warnlog "Something went wrong selecting 'Data Ranges' item No. 2."
                    else                        
                        printlog "Check that the string $B$17:$E$17 is in range for 'Y-Values'"                        
                        sControlString = RangeFor.GetText                        
                        if instr ( sControlString , "$B$17:$E$17" ) = 0 Then
                            warnlog "'Range for Y-Values' wasn't correctly preserved after save and reload."
                        endif                    
                    endif            
                endif            
            endif
        endif        
        printlog "Close the dialog with 'cancel'-button"
        TabFormatDataRangesDataSeries.Cancel        
    else
        warnlog "Tab page 'Data series' not visible!"
    end if
    if fInvokeTitlesDialog() = 0 then        
        printlog "Insert / Title"         
        printlog "Verify now the made settings in the 'Titles' dialog:"
        Kontext "InsertTitleDialog"        
        printlog "Check that 'Title' text field contains 'MyTitle'"
        sControlString = MainTitle.GetText        
        if sControlString <> "MyTitle" then
            warnlog "Title string 'MyTitle' wasn't preserved after save and reload."
        endif        
        printlog "Check that 'Subtitle'text field contains 1 2 3"
        sControlString = SubTitle.GetText        
        if sControlString <> "1 2 3" then
            warnlog "Subtitle string '1 2 3' wasn't preserved after save and reload."
        endif        
        printlog "Check that 'X Title' text field contains Grått"
        sControlString = TitleXAxis.GetText
        printlog "'X Title' is: " & sControlString
        if sControlString <> "Grått" then
            warnlog "X Title string 'Grått' wasn't preserved after save and reload."
        endif        
        printlog "Check that 'Y Title' text field contains 'Gräulich'"
        sControlString = TitleYAxis.GetText        
        if sControlString <> "Gräulich" then
            warnlog "Y Title string 'Gräulich' wasn't preserved after save and reload."
        endif        
        printlog "Check that 'Z Title' text field contains 灰色"
        sControlString = TitleZAxis.GetText
        printlog "'Z Title' is: " & sControlString
        if sControlString <> "灰色" then
            warnlog "Z Title string '灰色' wasn't preserved after save and reload."
        endif        
        printlog "Close the Title-dialog by pressing 'Cancel'-button"
        InsertTitleDialog.Cancel        
    endif    
    if fInvokeGridsDialog() = 0 then        
        printlog "Insert / Grid" 
        Kontext "InsertGridsDialog"        
        printlog "Check that 'Main Grid X Axis' checkbox is checked"        
        if NOT MainGridXaxis.IsChecked then                        
            warnlog "Check box 'Gridlines X Axis' (checked) wasn't preserved after save and reload."
        endif        
        printlog "Check that 'Main Grid Y Axis' checkbox is checked"
        if MainGridYaxis.IsChecked then
            warnlog "Check box 'Gridlines Y Axis' (unchecked) wasn't preserved after save and reload."
        endif            
        printlog "Check that 'Main Grid Z Axis' checkbox is checked"
        if NOT  MainGridZaxis.IsChecked then
            warnlog "Check box 'Gridlines Z Axis' (checked) wasn't preserved after save and reload."
        endif        
        printlog "Check that 'Minor Grid X Axis' checkbox is unchecked"        
        if MinorGridXaxis.IsChecked then            
            warnlog "Check box 'Minor Grid X Axis' (unchecked) wasn't preserved after save and reload."
        endif        
        printlog "Check that 'Minor Grid Y Axis' checkbox is unchecked"
        if MinorGridYaxis.IsChecked then            
            warnlog "Check box 'Minor Grid Y Axis' (unchecked) wasn't preserved after save and reload."
        endif        
        printlog "Check that 'Minor Grid Z Axis' checkbox is unchecked"
        if MinorGridZaxis.IsChecked then            
            warnlog "Check box 'Minor Grid Z Axis' (unchecked) wasn't preserved after save and reload."
        endif        
        printlog "Leave dialog with Cancel-button."
        InsertGridsDialog.Cancel
    endif    
    if fInvokeLegendDialog() = 0 then        
        printlog "Insert / Legend" 
        Kontext "InsertLegendDialog"                
        printlog "Check that 'Legend Display' checkbox is checked"
        if NOT Display.IsChecked then
            warnlog "Check box 'Legend Display' (enabled) wasn't preserved after save and reload."
        endif        
        printlog "Check that 'Left' radio button is selected"
        if NOT PositionLeft.IsChecked then
            warnlog "Radio button 'Left' (enabled) wasn't preserved after save and reload."
        endif        
        printlog "Cancel InsertLegend-dialog"
        Kontext "InsertLegendDialog"
        InsertLegendDialog.Cancel
    end if
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"    
    printlog "Close document"    
    Call hCloseDocument
endcase
'
'--------------------------------------------------------------------
'
testcase tCreateNewLineChart    
    dim sInputFile as STRING
    dim sOutputFile as STRING
    dim sControlString as STRING
    sInputFile = convertpath ( gTesttoolPath & "chart2/optional/input/randomData.ods" )
    sOutputFile = convertpath ( gOfficepath & "user/work/tCreateNewLineChart.ods" )    
    printlog "File / Load TesttoolPath/chart2/optional/input/randomData.ods"
    call hFileOpen(sInputFile)
    sleep(2)    
    printlog "Save document as Officepath/user/work/tCreateNewLineChart.ods"
    if NOT hFileSaveAsWithFilterKill ( sOutputFile , "calc8" ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    endif    
    printlog "Insert / Chart"
    InsertChart       
    printlog "Change Chart type to LINE (5th entry from the top)"
    Kontext "ChartType"
    if NOT ChartType.Exists(5) then
        warnlog "Chart Wizard was not visible. EXITING!"
        goto endsub
    else
        printlog "Chart Wizard visible."
    end if
    if fSetChartType ( "line" ) > 0 then
        warnlog "Something went wrong setting chart type to LINE."
    endif    
    printlog "Select 3rd Change chart type variant (lines only)"
    if fSetChartTypeVariant ( 3 ) > 0 then
        warnlog "Something went wrong setting chart type to 3rd variant (lines only)."
    endif    
    printlog "Enable 'Stack Series' and select 'Percent'"
    if fSetStackSeries ( "percent" ) > 0 then
        warnlog "Something went wrong setting Stack Series to Percent."
    endif
    '/// Enable 'Smooth Lines'
    printlog "Enable 'Smooth Lines'"
    if fSetSmoothLines ( TRUE ) > 0 then
        warnlog "Something went wrong enabling Smooth Lines."
    endif    
    printlog "Invoke 'Smooth Lines' dialog"
    if fInvokeSmoothLineProperties() > 0 then
        warnlog "Something went wrong invoking Smooth Lines Dialog."
    endif    
    printlog "Check B-Spline"
    BSpline.check        
    printlog "Set Resolution to 33"
    Resolution.SetText "33"        
    printlog "et Data Points Order to 5"    
    DataPointsOrder.SetText "5"    
    printlog "Close 'Smooth Lines' dialog"
    if fCloseSmoothLinePropertiesOK() > 0 then
        warnlog "Something went wrong closing Smooth Lines Dialog."
    endif    
    printlog "Change to Data Range tab-page"
    Kontext "ChartWizard"
    GoNext.Click
    Kontext "TabChartTypeDataRange"    
    printlog "Set Data Range text box to $Sheet1.$A$1:$F$13;$Sheet1.$A$17:$F$17"
    if fSetTextBox ( DataRange , "$Sheet1.$A$1:$F$13;$Sheet1.$A$17:$F$17" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif    
    printlog "Change to Data Series tab-page"
    Kontext "ChartWizard"
    GoNext.Click
    Kontext "TabChartTypeDataSeries"    
    printlog "Select 3rd entry in Data Series"
    if fSetListBoxByItem ( DataSeries , 3 ) > 0 then
        warnlog "Something went wrong selecting 'Data Series' item No. 3."
    endif    
    printlog "Remove series labeled 'John'"
    RemoveButton.Click        
    printlog "Select 4th entry in Data Series"
    if fSetListBoxByItem ( DataSeries , 4 ) > 0 then
        warnlog "Something went wrong selecting 'Data Series' item No. 4."
    endif    
    printlog "Select 2nd entry in Data Ranges"
    if fSetListBoxByItem ( DataRanges , 2 ) > 0 then
        warnlog "Something went wrong selecting 'Data Ranges' item No. 2."
    endif    
    printlog "Set 'Range for Y-Values' text box to $Sheet1.$F$2:$F$13;$Sheet1.$D$17"
    if fSetTextBox ( RangeFor , "$Sheet1.$F$2:$F$13;$Sheet1.$D$17" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif    
    printlog "Set 'Categories' text box to $Sheet1.$A$2:$A$13;$Sheet1.$A$1"
    if fSetTextBox ( Categories , "$Sheet1.$A$2:$A$13;$Sheet1.$A$19" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif    
    printlog "Change to Chart Elements tab-page"
    Kontext "ChartWizard"
    GoNext.Click
    Kontext "TabChartTypeChartElements"    
    printlog "Uncheck 'Y Axis'"
    if fSetCheckBox ( GridLinesYAxis , FALSE ) > 0 then
        warnlog "Something went wrong unselecting 'Y Axis'."
    endif    
    printlog "Uncheck 'Legend Display'"
    if fSetCheckBox ( Display , FALSE ) > 0 then
        warnlog "Something went wrong unselecting 'Legend Display'."
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
        printlog "Check if chart type is LINE"
        if ChooseType.GetSelIndex <> 5 then
            warnlog "Selected chart type wasn't preserved after save and reload."
        endif        
        printlog "Check if chart variant is LINES ONLY (third variant in non-CTL versions from the left)"
        if Variant.GetSelIndex <> 3 then
            warnlog "Selected chart type variant wasn't preserved after save and reload."
        end if            
        printlog "Check if 'Stack Series' box is checked"
        if NOT StackSeries.IsChecked then
            warnlog "Check box 'Stack Series' (enabled) wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Percent' radio button is checked"
        if NOT Percent.IsChecked then
            warnlog "Radio button box 'Percent' (enabled) wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Smooth Lines' box is checked"
        if NOT SmoothLines.IsChecked then
            warnlog "Check box 'Smooth Lines' (enabled) wasn't preserved after save and reload."
        endif        
        printlog "Invoke 'Smooth Lines' dialog"
        if fInvokeSmoothLineProperties() > 0 then
            warnlog "Something went wrong invoking Smooth Lines dialog."
        endif        
        printlog "Check if 'B-Spline' radio button is checked"
        if NOT BSpline.IsChecked then
            warnlog "Radio button box 'B-Spline' (enabled) wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Resolution' has value '33'"
        if Resolution.GetText <> "33" then
            warnlog "Value for Resolution '33' wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Data Points Order' has value '5'"
        if DataPointsOrder.GetText <> "5" then
            warnlog "Value for Data Points Order '5' wasn't preserved after save and reload."
        endif        
        printlog "Close 'Smooth Lines' dialog"
        if fCloseSmoothLinePropertiesOK() > 0 then
            warnlog "Something went wrong closing Smooth Lines Dialog."
        endif        
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
    printlog "Format / Data Ranges"   
    FormatDataRanges
    printlog "Switch to tab page Data Series"
    Kontext
    Active.SetPage TabFormatDataRangesDataRange 'Due to bug #i77237# the tabpage has to be switched once before 'OK' or 'Cancel' can be used
    Active.SetPage TabFormatDataRangesDataSeries 
    Kontext "TabChartTypeDataSeries"
    if TabChartTypeDataSeries.Exists(2) then
        printlog "Verify that there are 4 data series"
        if DataSeries.GetItemCount <> 4 then
            warnlog "There should be 4 dataseries, but there are " & DataSeries.GetItemCount
        end if
        printlog "Select 4th entry in Data Series"
        if fSetListBoxByItem ( DataSeries , 4 ) > 0 then
            warnlog "Selecting 4th item in Data Series failed. EXITING!"
            TabChartTypeDataSeries.Cancel
            call hCloseDocument
            goto endsub
        endif        
        printlog "Select Name in Data Ranges"
        if fSetListBoxByItem ( DataRanges , 1 ) > 0 then
            warnlog "Something went wrong selecting 'Data Ranges' item No. 1."
        endif        
        printlog "Check substring '$F$1' in Range for Name"        
        sControlString = RangeFor.GetText
        printlog "'Range for name' is: " & sControlString
        if instr ( sControlString , "$F$1" ) = 0 Then
            warnlog "'Range for name' wasn't correctly preserved after save and reload."
        endif        
        printlog "Select Y-Values in Data Ranges"
        if fSetListBoxByItem ( DataRanges , 2 ) > 0 then
            warnlog "Something went wrong selecting 'Data Ranges' item No. 2. EXITING!"
            TabChartTypeDataSeries.Cancel
            call hCloseDocument
            goto endsub
        endif        
        printlog "Check substring '$F$2:$F$13' and '$D$17'in Range for Name"       
        sControlString = RangeFor.GetText        
        if instr ( sControlString , "$F$2:$F$13" ) = 0 Then
            warnlog "'Range for Y-Values' wasn't correctly preserved after save and reload."
        endif
        if instr ( sControlString , "$D$17" ) = 0 Then
            warnlog "'Range for Y-Values' wasn't correctly preserved after save and reload."
        endif        
        printlog "Check substring '$A$2:$A$13' and '$A$19'in Categories"        
        sControlString = Categories.GetText        
        if instr ( sControlString , "$A$2:$A$13" ) = 0 Then
            warnlog "'Categories' wasn't correctly preserved after save and reload."
        endif
        if instr ( sControlString , "$A$19" ) = 0 Then
            warnlog "'Categories' wasn't correctly preserved after save and reload."
        endif
        printlog "Leave tab page data series with CANCEL" 
        TabChartTypeDataSeries.Cancel
    else
        warnlog "Tab page Data Series was not up!"
    end if
    printlog "Insert / Title"    
    InsertTitle        
    Kontext "InsertTitleDialog"
    if InsertTitleDialog.Exists(2) then        
        printlog "Check if 'Title' text box is empty"
        sControlString = MainTitle.GetText    
        if sControlString <> "" then
            warnlog "Empty Title string wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Subtitle' text box is empty"
        sControlString = SubTitle.GetText    
        if sControlString <> "" then
            warnlog "Empty Subtitle string wasn't preserved after save and reload."
        endif        
        printlog "Check if 'X Title' text box is empty"
        sControlString = TitleXaxis.GetText    
        if sControlString <> "" then
            warnlog "Empty X Title string wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Y Title' text box is empty"
        sControlString = TitleYaxis.GetText   
        if sControlString <> "" then
            warnlog "Empty Y Title string wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Z Title' text box is empty"
        sControlString = TitleZaxis.GetText    
        if sControlString <> "" then
            warnlog "Empty Z Title string wasn't preserved after save and reload."
        endif        
        printlog "Leaving Title dialog with Cancel"
        InsertTitleDialog.Cancel
    else
        warnlog "Title dialog was not up!"    
    end if    
    printlog "Insert / Grids"    
    If fInvokeGridsDialog = 0 then
        Kontext "InsertGridsDialog"            
        printlog "Check if 'Gridlines X Axis' box is checked"
        if MainGridXaxis.IsChecked then
            warnlog "Check box 'Gridlines X Axis' (disabled) wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Gridlines Y Axis' box is unchecked"
        if MainGridYaxis.IsChecked then
            warnlog "Check box 'Gridlines Y Axis' (disabled) wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Gridlines Z Axis' box is checked"
        if MainGridZaxis.IsEnabled then
            warnlog "Check box 'Gridlines Z Axis' shouldn't be enabled after save and reload."
        endif
        printlog "Leave dialog with CANCEL"
        InsertGridsDialog.Cancel
    else
        warnlog "Insert Grids dialog was not up!"
    end if    
    printlog "Insert / Legend"
    if fInvokeLegendDialog() = 0 then                         
        printlog "Check if 'Legend Display' box is checked"
        Kontext "InsertLegendDialog"
        if Display.IsChecked then
            warnlog "Check box 'Legend Display' (disabled) wasn't preserved after save and reload."
        endif
        printlog "Leave dialog with CANCEL."
        InsertLegendDialog.Cancel
    else
        warnlog "Insert Legend dialog was not up!"
    end if    
    printlog "Press ESCAPE button (deselecting the OLE object)"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"    
    printlog "Close document"
    Call hCloseDocument
endcase
'
'--------------------------------------------------------------------
'
testcase tCreateNewBubbleChart    
    dim sInputFile as STRING
    dim sOutputFile as STRING
    dim sControlString as STRING
    sInputFile = convertpath ( gTesttoolPath & "chart2/optional/input/BubbleChartData.ods" )
    sOutputFile = convertpath ( gOfficepath & "user/work/tCreateNewBubbleChart.ods" )    
    printlog "File / Load TesttoolPath/chart2/optional/input/BubbleChartData.ods"
    call hFileOpen(sInputFile)
    sleep(2)    
    printlog "Save document as Officepath/user/work/tCreateNewBubbleChart.ods"
    if NOT hFileSaveAsWithFilterKill ( sOutputFile , "calc8" ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    endif    
    printlog "Insert / Chart"
    InsertChart       
    printlog "Change Chart type to BUBBLE (7th entry from the top)"
    Kontext "ChartType"
    if NOT ChartType.Exists(5) then
        warnlog "Chart Wizard was not visible. EXITING!"
        goto endsub
    else
        printlog "Chart Wizard visible."
    end if	
    if fSetChartType ( "bubble" ) > 0 then
        warnlog "Something went wrong setting chart type to Bubble Chart."
    endif    
    printlog "Select 1st chart type variant (Bubble Chart)"
    if fSetChartTypeVariant ( 1 ) > 0 then
        warnlog "Something went wrong setting chart type to 1st variant (Bubble Chart)."
    endif    
    printlog "Change to Data Range tab-page"
    Kontext "ChartWizard"
    GoNext.Click
    Kontext "TabChartTypeDataRange"    
    printlog "Set Data Range text box to $Sheet1.$A$2:$C$5;$Sheet1.$A$13:$C$16"
    if fSetTextBox ( DataRange , "$Sheet1.$A$2:$C$5;$Sheet1.$A$13:$C$16" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif    
    printlog "Change to Data Series tab-page"
    Kontext "ChartWizard"
    GoNext.Click
    Kontext "TabChartTypeDataSeries"    
	'### Name ###
    printlog "Select 1st entry in Data Series"
    if fSetListBoxByItem ( DataSeries , 1 ) > 0 then
        warnlog "Something went wrong selecting 'Data Series' item No. 1."
    endif
	printlog "Select 1st entry in Data Ranges"
    if fSetListBoxByItem ( DataRanges , 1 ) > 0 then
        warnlog "Something went wrong selecting 'Data Ranges' item No. 1."
    endif        
	printlog "Set 'Range for Name' text box to $Sheet1.$C$1"
    if fSetTextBox ( RangeFor , "$Sheet1.$C$1" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif
	'### Bubble Sizes ###	
	printlog "Select 2nd entry in Data Ranges"
    if fSetListBoxByItem ( DataRanges , 2 ) > 0 then
        warnlog "Something went wrong selecting 'Data Ranges' item No. 2."
    endif        
	printlog "Set 'Range for Bubble Sizes' text box to $Sheet1.$C$2:$C$5"
    if fSetTextBox ( RangeFor , "$Sheet1.$C$2:$C$5" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif
	'### X-Values ###	
	printlog "Select 3rd entry in Data Ranges"
    if fSetListBoxByItem ( DataRanges , 3 ) > 0 then
        warnlog "Something went wrong selecting 'Data Ranges' item No. 3."
    endif        
	printlog "Set 'X-Values' text box to $Sheet1.$A$2:$A$5"
    if fSetTextBox ( RangeFor , "$Sheet1.$A$2:$A$5" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif
	'### Y-Values ###	
	printlog "Select 4th entry in Data Ranges"
    if fSetListBoxByItem ( DataRanges , 4 ) > 0 then
        warnlog "Something went wrong selecting 'Data Ranges' item No. 4."
    endif        
	printlog "Set 'Y-Values' text box to $Sheet1.$B$2:$B$5"
    if fSetTextBox ( RangeFor , "$Sheet1.$B$2:$B$5" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif
	printlog "Add data series"
	AddButton.Click
	'### Name ###
	printlog "Select 1st entry in Data Ranges"
    if fSetListBoxByItem ( DataRanges , 1 ) > 0 then
        warnlog "Something went wrong selecting 'Data Ranges' item No. 2."
    endif        
	printlog "Set 'Range for Name' text box to $Sheet1.$C$12"
    if fSetTextBox ( RangeFor , "$Sheet1.$C$12" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif
	'### Bubble Sizes ###	
	printlog "Select 2nd entry in Data Ranges"
    if fSetListBoxByItem ( DataRanges , 2 ) > 0 then
        warnlog "Something went wrong selecting 'Data Ranges' item No. 2."
    endif        
	printlog "Set 'Range for Bubble Sizes' text box to $Sheet1.$C$13:$C$16"
    if fSetTextBox ( RangeFor , "$Sheet1.$C$13:$C$16" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif
	'### X-Values ###	
	printlog "Select 3rd entry in Data Ranges"
    if fSetListBoxByItem ( DataRanges , 3 ) > 0 then
        warnlog "Something went wrong selecting 'Data Ranges' item No. 3."
    endif        
	printlog "Set 'X-Values' text box to $Sheet1.$A$13:$A$16"
    if fSetTextBox ( RangeFor , "$Sheet1.$A$13:$A$16" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif
	'### Y-Values ###	
	printlog "Select 4th entry in Data Ranges"
    if fSetListBoxByItem ( DataRanges , 4 ) > 0 then
        warnlog "Something went wrong selecting 'Data Ranges' item No. 4."
    endif        
	printlog "Set 'Y-Values' text box to $Sheet1.$B$13:$B$16"
    if fSetTextBox ( RangeFor , "$Sheet1.$B$13:$B$16" ) > 0 then
        warnlog "Something went wrong setting Text."
    endif

    printlog "Change to Chart Elements tab-page"
    Kontext "ChartWizard"
    GoNext.Click
    Kontext "TabChartTypeChartElements"    
    printlog "Uncheck 'Y Axis'"
    if fSetCheckBox ( GridLinesYAxis , FALSE ) > 0 then
        warnlog "Something went wrong unselecting 'Y Axis'."
    endif    
    printlog "Uncheck 'Legend Display'"
    if fSetCheckBox ( Display , FALSE ) > 0 then
        warnlog "Something went wrong unselecting 'Legend Display'."
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
        printlog "Check if chart type is BUBBLE CHART"
        if ChooseType.GetSelIndex <> 7 then
            warnlog "Selected chart type wasn't preserved after save and reload."
        endif        
        printlog "Check if chart variant is BUBBLE CHART (1st variant in non-CTL versions from the left)"
        if Variant.GetSelIndex <> 1 then
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
    printlog "Format / Data Ranges"   
    FormatDataRanges
    Kontext
    Active.SetPage TabChartTypeDataRange        
    Kontext "TabChartTypeDataRange"
    if TabChartTypeDataRange.Exists(2) then        
        printlog "Check substrings '$A$2:$C$5', '$C$1', '$B$13:$C$16', '$C$12' in Data Range"        
        sControlString = DataRange.GetText
        printlog "Data Range is: " & sControlString
        if instr ( sControlString , "$A$2:$C$5" ) = 0 Then
            warnlog "Data range wasn't correctly preserved after save and reload: $A$2:$C$13"
        endif
        if instr ( sControlString , "$C$1" ) = 0 Then
            warnlog "Data range wasn't correctly preserved after save and reload: $A$19"
        endif
        if instr ( sControlString , "$B$13:$C$16" ) = 0 Then
           QAErrorlog "#i100780#Data range wasn't correctly preserved after save and reload: $B$13:$C$16"
        endif
        if instr ( sControlString , "$C$12" ) = 0 Then
            warnlog "Data range wasn't correctly preserved after save and reload: $E$1:$F$13"
        endif
        printlog "Check if Radio Button ROWS is checked"
        if Rows.IsChecked then
            warnlog "Radio Button ROWS (disabled) wasn't preserved after save and reload."
        endif        
        printlog "Check if Radio Button COLUMNS is unchecked"
        if NOT Columns.IsChecked then
            warnlog "Radio Button COLUMNS (enabled) wasn't preserved after save and reload."
        endif        
        printlog "Check if 'First row as label' box is checked"
        if NOT FirstRowAsLabel.IsChecked then
            warnlog "Check box 'First row as label' (checked) wasn't preserved after save and reload."
        endif        
        printlog "Check if 'First column as label' box is not checked"
        if FirstColumnAsLabel.IsChecked then
            warnlog "Check box 'First column as label' (unchecked) wasn't preserved after save and reload."
        endif
    else
        warnlog "Tab page Data Range was not up!"
        FormatDataRanges        
    end if    
    printlog "Switch to tab page Data Series"
    Kontext
    Active.SetPage TabFormatDataRangesDataSeries 
    Kontext "TabChartTypeDataSeries"
    if TabChartTypeDataSeries.Exists(2) then        
        printlog "Select 1st entry in Data Series"
        if fSetListBoxByItem ( DataSeries , 1 ) > 0 then
            warnlog "Selecting 1st item in Data Series failed. EXITING!"
            TabChartTypeDataSeries.Cancel
            call hCloseDocument
            goto endsub
        endif        
        printlog "Select Name in Data Ranges"
        if fSetListBoxByItem ( DataRanges , 1 ) > 0 then
            warnlog "Something went wrong selecting 'Data Ranges' item No. 1."
        endif        
        printlog "Check substring '$C$1' in Range for Name"        
        sControlString = RangeFor.GetText
        printlog "'Range for name' is: " & sControlString
        if instr ( sControlString , "$C$1" ) = 0 Then
            warnlog "'Range for name' wasn't correctly preserved after save and reload."
        endif        
		
        printlog "Select Bubble Sizes in Data Ranges"
        if fSetListBoxByItem ( DataRanges , 2 ) > 0 then
            warnlog "Something went wrong selecting 'Data Ranges' item No. 2. EXITING!"
            TabChartTypeDataSeries.Cancel
            call hCloseDocument
            goto endsub
        endif        
        printlog "Check substring '$C$2:$C$5'in Range for Name"       
        sControlString = RangeFor.GetText        
        if instr ( sControlString , "$C$2:$C$5" ) = 0 Then
            warnlog "'Range for Bubble Sizes' wasn't correctly preserved after save and reload."
        endif				

        printlog "Select X-Values in Data Ranges"
        if fSetListBoxByItem ( DataRanges , 3 ) > 0 then
            warnlog "Something went wrong selecting 'Data Ranges' item No. 3. EXITING!"
            TabChartTypeDataSeries.Cancel
            call hCloseDocument
            goto endsub
        endif        
        printlog "Check substring '$A$2:$A$5' in Range for Name"       
        sControlString = RangeFor.GetText        
        if instr ( sControlString , "$A$2:$A$5" ) = 0 Then
            warnlog "'Range for X-Values' wasn't correctly preserved after save and reload."
        endif
		
        printlog "Select Y-Values in Data Ranges"
        if fSetListBoxByItem ( DataRanges , 4 ) > 0 then
            warnlog "Something went wrong selecting 'Data Ranges' item No. 4. EXITING!"
            TabChartTypeDataSeries.Cancel
            call hCloseDocument
            goto endsub
        endif        
        printlog "Check substring '$B$2:$B$5' in Range for Name"       
        sControlString = RangeFor.GetText        
        if instr ( sControlString , "$B$2:$B$5" ) = 0 Then
            warnlog "'Range for Y-Values' wasn't correctly preserved after save and reload."
        endif

        printlog "Select 2nd entry in Data Series"
        if fSetListBoxByItem ( DataSeries , 2 ) > 0 then
            warnlog "Selecting 1st item in Data Series failed. EXITING!"
            TabChartTypeDataSeries.Cancel
            call hCloseDocument
            goto endsub
        endif        
        printlog "Select Name in Data Ranges"
        if fSetListBoxByItem ( DataRanges , 1 ) > 0 then
            warnlog "Something went wrong selecting 'Data Ranges' item No. 1."
        endif        
        printlog "Check substring '$C$12' in Range for Name"        
        sControlString = RangeFor.GetText
        printlog "'Range for name' is: " & sControlString
        if instr ( sControlString , "$C$12" ) = 0 Then
            warnlog "'Range for name' wasn't correctly preserved after save and reload."
        endif        
		
        printlog "Select Bubble Sizes in Data Ranges"
        if fSetListBoxByItem ( DataRanges , 2 ) > 0 then
            warnlog "Something went wrong selecting 'Data Ranges' item No. 2. EXITING!"
            TabChartTypeDataSeries.Cancel
            call hCloseDocument
            goto endsub
        endif        
        printlog "Check substring '$C$13:$C$16'in Range for Name"       
        sControlString = RangeFor.GetText        
        if instr ( sControlString , "$C$13:$C$16" ) = 0 Then
            warnlog "'Range for Bubble Sizes' wasn't correctly preserved after save and reload."
        endif				

        printlog "Select X-Values in Data Ranges"
        if fSetListBoxByItem ( DataRanges , 3 ) > 0 then
            warnlog "Something went wrong selecting 'Data Ranges' item No. 3. EXITING!"
            TabChartTypeDataSeries.Cancel
            call hCloseDocument
            goto endsub
        endif        
        printlog "Check substring '$A$13:$A$16' in Range for Name"       
        sControlString = RangeFor.GetText        
        if instr ( sControlString , "$A$13:$A$16" ) = 0 Then
            warnlog "'Range for X-Values' wasn't correctly preserved after save and reload."
        endif
		
        printlog "Select Y-Values in Data Ranges"
        if fSetListBoxByItem ( DataRanges , 4 ) > 0 then
            warnlog "Something went wrong selecting 'Data Ranges' item No. 4. EXITING!"
            TabChartTypeDataSeries.Cancel
            call hCloseDocument
            goto endsub
        endif        
        printlog "Check substring '$B$13:$B$16' in Range for Name"       
        sControlString = RangeFor.GetText        
        if instr ( sControlString , "$B$13:$B$16" ) = 0 Then
            warnlog "'Range for Y-Values' wasn't correctly preserved after save and reload."
        endif
		
        printlog "Leave tab page data series with CANCEL" 
        TabChartTypeDataSeries.Cancel
    else
        warnlog "Tab page Data Series was not up!"
    end if
    printlog "Insert / Title"    
    InsertTitle        
    Kontext "InsertTitleDialog"
    if InsertTitleDialog.Exists(2) then        
        printlog "Check if 'Title' text box is empty"
        sControlString = MainTitle.GetText    
        if sControlString <> "" then
            warnlog "Empty Title string wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Subtitle' text box is empty"
        sControlString = SubTitle.GetText    
        if sControlString <> "" then
            warnlog "Empty Subtitle string wasn't preserved after save and reload."
        endif        
        printlog "Check if 'X Title' text box is empty"
        sControlString = TitleXaxis.GetText    
        if sControlString <> "" then
            warnlog "Empty X Title string wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Y Title' text box is empty"
        sControlString = TitleYaxis.GetText   
        if sControlString <> "" then
            warnlog "Empty Y Title string wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Z Title' text box is empty"
        sControlString = TitleZaxis.GetText    
        if sControlString <> "" then
            warnlog "Empty Z Title string wasn't preserved after save and reload."
        endif        
        printlog "Leaving Title dialog with Cancel"
        InsertTitleDialog.Cancel
    else
        warnlog "Title dialog was not up!"    
    end if    
    printlog "Insert / Grids"    
    If fInvokeGridsDialog = 0 then
        Kontext "InsertGridsDialog"            
        printlog "Check if 'Gridlines X Axis' box is checked"
        if MainGridXaxis.IsChecked then
            warnlog "Check box 'Gridlines X Axis' (disabled) wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Gridlines Y Axis' box is unchecked"
        if MainGridYaxis.IsChecked then
            warnlog "Check box 'Gridlines Y Axis' (disabled) wasn't preserved after save and reload."
        endif        
        printlog "Check if 'Gridlines Z Axis' box is checked"
        if MainGridZaxis.IsEnabled then
            warnlog "Check box 'Gridlines Z Axis' shouldn't be enabled after save and reload."
        endif
        printlog "Leave dialog with CANCEL"
        InsertGridsDialog.Cancel
    else
        warnlog "Insert Grids dialog was not up!"
    end if    
    printlog "Insert / Legend"
    if fInvokeLegendDialog() = 0 then                         
        printlog "Check if 'Legend Display' box is checked"
        Kontext "InsertLegendDialog"
        if Display.IsChecked then
            warnlog "Check box 'Legend Display' (disabled) wasn't preserved after save and reload."
        endif
        printlog "Leave dialog with CANCEL."
        InsertLegendDialog.Cancel
    else
        warnlog "Insert Legend dialog was not up!"
    end if    
    printlog "Press ESCAPE button (deselecting the OLE object)"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"    
    printlog "Close document"
    Call hCloseDocument
endcase


