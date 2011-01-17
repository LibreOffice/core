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
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : Updatetest for Flexible Source Range Selection in Writer
'*
'************************************************************************
'*
' #1 tDataRangeWizardWriter                 ' Updatetest for tabpage DataRange via chart wizard
' #1 tDataSeriesWizardWriter                ' Updatetest for tabpage DataSeries via chart wizard
' #1 tDataRangeDialogWriter                 ' Updatetest for tabpage DataRange via data ranges dialog
' #1 tDataSeriesDialogWriter                ' Updatetest for tabpage DataSeries via data ranges dialog
' #1 fOpenWriterTestDocumentAndInsertChart  ' Sub routine to load test documents
'*
'\***********************************************************************

testcase tDataRangeWizardWriter
    dim sDataRange as string
    dim sInputFile as string
    sInputFile = convertpath(gTestToolPath & "chart2/required/input/ch_flexrange05.odt")
    '///Open test document <i>gTestToolPath</i>/chart2/required/input/ch_flexrange05.odt
    printlog "Open test document gTestToolPath/chart2/required/input/ch_flexrange05.odt"    
    if fOpenWriterTestDocumentAndInsertChart(sInputFile) then
        '///Choose the Chart type <i>line</i> (should be the fifth in the top-down list)
        printlog "Choose the Chart type line (should be the fifth in the top-down list)"
        if fSetChartType( "line" ) > 0 then
            warnlog "Something went wrong setting chart type to Line."            
            Kontext "ChartWizard"
            ChartWizard.Cancel
            call hCloseDocument
            goto endsub
        endif                        
        Kontext "ChartWizard"
        '/// Click Next button
        GoNext.Click            
        Kontext "TabChartTypeDataRange"
        call DialogTest ( TabChartTypeDataRange )        
        '///Check data range in edit field value (ChartFlexRange.A1:E5)
        printlog "Check data range in edit field value (ChartFlexRange.A1:E5)"
        Kontext "TabChartTypeDataRange"
        sDataRange = DataRange.GetText
        if sDataRange <> "ChartFlexRange.A1:E5" then
            warnlog "This is not the expected datarange, it is " & sDataRange & " but should be ChartFlexRange.A1:E5" 
        else
            printlog "    Datarange is correct"
        endif        
        '///Check if -Data series in rows- is enabled
        printlog "Check if -Data series in rows- is enabled"
        if Rows.IsEnabled then
            printlog "    Data series in rows is enabled" 
        else
            warnlog "Data series in rows is disabled"
        endif           
        '///Check if -Data series in columns- is enabled and chosen
        printlog "Check if -Data series in columns- is enabled and chosen"
        if Columns.IsEnabled then
            printlog "    Data series in columns is enabled"
            if Columns.IsChecked then
                printlog "        and checked"
            else
                warnlog "Data series in columns is not checked"
            endif
        else
            warnlog "Data series in columns is disabled"
        endif        
        '///Check if -First row as label- is enabled and checked
        printlog "Check if -First row as label- is enabled and checked"
        if FirstRowAsLabel.IsEnabled then
            printlog "    First row as label is enabled"
            if FirstRowAsLabel.IsChecked then
                printlog "        and checked"        
            else
                warnlog "First row as label is not checked"
            endif
        else
            warnlog "First row as label is disabled"
        endif        
        '///Check if -First column as label- is enabled checked
        printlog "Check if -First column as label- is enabled checked"
        if FirstColumnAsLabel.IsEnabled then
            printlog "    First column as label is enabled"
            if FirstColumnAsLabel.IsChecked then
                printlog "        and checked"        
            else
                warnlog "First column as label is not checked"
            endif
        else
            warnlog "First column as label is disabled"
        endif        
        '///Enter invalid range ChartFlexRange.A1:E
        printlog "Enter invalid range ChartFlexRange.A1:E"
        DataRange.SetText "ChartFlexRange.A1:E"        
        '///Check if -Data series in rows- is disabled
        printlog "Check if -Data series in rows- is disabled"
        Kontext "TabChartTypeDataRange"
        if NOT Rows.IsEnabled then
            printlog "    Data series in rows is disabled" 
        else
            warnlog "Data series in rows is enabled, but should be disabled"
        endif        
        '///Check if -Data series in columns- is disabled
        printlog "Check if -Data series in columns- is disabled"
        if NOT Columns.IsEnabled then
            printlog "    Data series in columns is disabled" 
        else
            warnlog "Data series in columns is enabled, but should be disabled"
        endif        
        '///Check if -First row as label- is disabled
        printlog "Check if -First row as label- is disabled"
        if NOT FirstRowAsLabel.IsEnabled then
            printlog "    First row as label is disabled" 
        else
            warnlog "First row as label is enabled, but should be disabled"
        endif        
        '///Check if -First column as label- is disabled
        printlog "Check if -First column as label- is disabled"
        if NOT FirstColumnAsLabel.IsEnabled then
            printlog "    First Column as label is disabled" 
        else
            warnlog "First Column as label is enabled, but should be disabled"
        endif        
        '///Check that Back, Next and Finish does not work
        printlog "Check that Back, Next and Finish does not work"
        kontext "ChartWizard"
        GoBack.Click
        kontext "TabChartTypeDataRange"
        if FirstColumnAsLabel.IsVisible then
            printlog "    Success: Back button does not work"
        else
            warnlog "Back button is not functionless"
            Kontext "ChartWizard"
            GoNext.Click
        endif        
        '///Enter valid range ChartFlexRange.A1:E5
        printlog "Enter valid range ChartFlexRange.A1:E5"
        kontext "TabChartTypeDataRange"
        DataRange.SetText "ChartFlexRange.A1:E5"        
        '///Check if -Data series in rows- is enabled
        printlog "Check if -Data series in rows- is enabled"
        if Rows.IsEnabled then
            printlog "    Data series in rows is enabled" 
        else
            warnlog "Data series in rows is disabled"
        endif        
        '///Check if -Data series in columns- has been enabled and chosen
        printlog "Check if -Data series in columns- has been enabled and chosen"
        if Columns.IsEnabled then
            printlog "    Data series in columns is enabled"
            if Columns.IsChecked then
                printlog "        and checked"
            else
                warnlog "Data series in columns is not checked"
            endif
        else
            warnlog "Data series in rows is disabled"
        endif        
        '///Check if -First row as label- is enabled and checked
        printlog "Check if -First row as label- is enabled and checked"
        if FirstRowAsLabel.IsEnabled then
            printlog "    First row as label is enabled"
            if FirstRowAsLabel.IsChecked then
                printlog "        and checked"        
            else
                warnlog "First row as label is not checked"
            endif
        else
            warnlog "First row as label is disabled"
        endif        
        '///Check if -First column as label- is enabled checked
        printlog "Check if -First column as label- is enabled and checked"
        if FirstColumnAsLabel.IsEnabled then
            printlog "    First column as label is enabled"
            if FirstColumnAsLabel.IsChecked then
                printlog "        and checked"        
            else
                warnlog "First column as label is not checked"
            endif
        else
            warnlog "First column as label is disabled"
        endif        
        '///Press Cancel button
        printlog "Press Cancel button"
        Kontext "ChartWizard"
        ChartWizard.Cancel        
        '///Close document
        printlog "Close document"
        Call hCloseDocument
    else
        warnlog "It was not possible to open the test document." 
    endif
endcase
'
'-------------------------------------------------------------------------
'
testcase tDataSeriesWizardWriter
    dim sInputFile as string
    dim iDataSeries as integer
    dim sDataRanges as string
    dim sRangeFor as string
    dim sCategories as string
    sInputFile = convertpath(gTestToolPath & "chart2/required/input/ch_flexrange06.odt" )    
    '///Open test document gTestToolPath/chart2/required/input/ch_flexrange06.odt
    printlog "Open test document gTestToolPath/chart2/required/input/ch_flexrange06.odt"    
    if fOpenWriterTestDocumentAndInsertChart(sInputFile) then            
        '///Choose the Chart type <i>area</i> (should be the fourth in the top-down list)
        printlog "Choose the Chart type <i>area</i> (should be the fourth in the top-down list)"
        Kontext "ChartType"
        if fSetChartType( "area" ) > 0 then
            warnlog "Something went wrong setting chart type to area."            
            Kontext "ChartWizard"
            ChartWizard.Cancel
            call hCloseDocument
            goto endsub
        endif        
        Kontext "ChartWizard"
        '/// Click Next button
        GoNext.Click
        '/// Click Next button
        GoNext.Click
        Kontext "TabChartTypeDataSeries"
        if RangeFor.IsVisible then
            printlog "    Switching to tabpage was successfull"
        else
            warnlog "Switching to tabpage Data Series was not successfull"
            goto endsub
        endif        
        call DialogTest ( TabChartTypeDataSeries )            
        '///Check if the data series listbox is enabled and contains four entries
        printlog "Check if the data series listbox is enabled and contains four entries"
        iDataSeries = DataSeries.GetItemCount
        if iDataSeries = 4 then
            printlog "    All 4 data series are available"
        else
            warnlog "There are " & iDataSeries & " data series instead of 4"
        endif        
        '///Check if the ADD button is enabled and click the button
        printlog "Check if the ADD button is enabled and click the button"
        if AddButton.IsEnabled then
            printlog "    ADD button is enabled"
            AddButton.Click
        else
            warnlog "ADD button is not enabled"
        endif            
        '///Check if there are now five entries in the data series listbox
        printlog "Check if there are now five entries in the data series listbox"
        iDataSeries = DataSeries.GetItemCount
        if iDataSeries = 5 then
            printlog "    Adding a data series was successfull"
        else
            warnlog "There are " & iDataSeries & " data series instead of 5"
        endif        
        '///Check if the new data series was added below the highlighted one
        printlog "Check if the new data series was added below the highlighted one"
        if DataSeries.GetSelIndex = 2 then
            printlog "    The new data series was inserted below the highlighted (first) one"
        else
            warnlog "The new dataseries was not inserted below the highlighted one"
        endif        
        '///Check if the REMOVE button is enabled and click the button
        printlog "Check if the REMOVE button is enabled and click the button"
        if RemoveButton.IsEnabled then
            printlog "    REMOVE button is enabled"
            RemoveButton.Click
        else
            warnlog "REMOVE button is not enabled"
        endif        
        '///Check if there are now four entries in the data series listbox
        printlog "Check if there are now four entries in the data series listbox"
        iDataSeries = DataSeries.GetItemCount
        if iDataSeries = 4 then
            printlog "    Removing a data series was successfull"
        else
            warnlog "There are " & iDataSeries & " data series instead of 4"
        endif
        '/// Select third entry
        printlog "Select third entry"         
        DataSeries.Select(3)
        if MoveUp.IsEnabled then
            printlog "    The MoveUp button is enabled"
            '/// The Move Up button should be enabled
            printlog "The Move Up button should be enabled"
            '/// Click the Move Up button twice
            printlog "Click the Move Up button twice"
            MoveUp.Click
            MoveUp.Click
        else
            warnlog "The MoveUp button is disabled"
        endif
        if DataSeries.GetSelIndex = 1 then
            printlog "    Moving data series was successfull"
            if MoveUp.IsEnabled then
                warnlog "MoveUp button is not disabled on first entry"
            else
                printlog "        and MoveUp button is disabled on first enty"
            endif
        else
            warnlog "Moving data series was not successfull"
        endif
        '/// Select second entry
        printlog "Select second entry"         
        DataSeries.Select(2)
        if MoveDown.IsEnabled then
            printlog "    The MoveDown button is enabled"
            '/// If Move Down button is enabled
            printlog "If Move Down button is enabled"
            '/// Click Move Down button twice
            printlog "Click Move Down button twice"
            MoveDown.Click
            MoveDown.Click
        else
            warnlog "The MoveDown button is disabled"
        endif
        if DataSeries.GetSelIndex = 4 then
            printlog "    Moving data series was successfull"
            if MoveDown.IsEnabled then
                warnlog "MoveDown button is not disabled on first entry"
            else
                printlog "        and MoveDown button is disabled on first enty"
            endif
        else
            warnlog "Moving data series was not successfull"
        endif        
        '///Select first entry in Data Ranges
        printlog "Select first entry in Data Ranges"
        DataRanges.Select(1)        
        '///Verify that the range matches the one in edit field
        printlog "Verify that the range matches the one in edit field"
        'GetSelText(line,column) is a new parameter since CWS:SRC680gh13
        'Instead of 1,2 there must be 1,4 used becqause there are two invisible
        'label-columns in that treelist-box        
        sDataRanges = DataRanges.GetSelText (1,4)
        sRangeFor = RangeFor.GetText
        if sDataRanges = sRangeFor then
            printlog "    Both ranges are matching"
        else
            warnlog "The ranges does not match: " & sDataRanges & sRangeFor
        endif        
        '///Select second entry in Data Ranges
        printlog "Select second entry in Data Ranges"
        DataRanges.Select(2)        
        '///Check that the range in the edit field has changed, too
        printlog "Check that the range in the edit field has changed, too"
        'GetSelText(line,column) is a new parameter since CWS:SRC680gh13
        'Instead of 1,2 there must be 1,4 used becqause there are two invisible
        'label-columns in that treelist-box        
        sDataRanges = DataRanges.GetSelText (1,4)
        sRangeFor = RangeFor.GetText
        if sDataRanges = sRangeFor then
            printlog "    Both ranges are matching"
        else
            warnlog "The ranges does not match" & sDataRanges & sRangeFor
        endif        
        '///Change the range in edit field 
        printlog "Change the range in edit field"
        RangeFor.SetText "ChartFlexRange.B2:B4"            
        '///Check that the range in Data Ranges has changed, too
        printlog "Check that the range in Data Ranges has changed, too"
        'GetSelText(line,column) is a new parameter since CWS:SRC680gh13
        'Instead of 1,2 there must be 1,4 used becqause there are two invisible
        'label-columns in that treelist-box        
        sDataRanges = DataRanges.GetSelText (1,4)
        if sDataRanges = "ChartFlexRange.B2:B4" then
           printlog "    The data range has also changed"
        else
            warnlog "The data range has not changed"
        endif        
        '/// Set ChartFlexRange.A2:A4 as range for categories
        printlog "Set ChartFlexRange.A2:A4 as range for categories"        
        Categories.SetText "ChartFlexRange.A2:A4"
        sCategories = Categories.GetText
        if sCategories = "ChartFlexRange.A2:A4" then
            printlog "    The range has been modified"
        else
            warnlog "Modifiing the range was not successfull"
        endif        
        '///Enter invalid range ChartFlexRange.A2:A
        printlog "Enter invalid range ChartFlexRange.A2:A"
        Categories.SetText "ChartFlexRange.A2:A"        
        '///Check that the BACK button is disabled
        printlog "Check that the BACK button does not work"
        kontext "ChartWizard"        
        GoBack.Click
        kontext "TabChartTypeDataSeries"
        if Categories.IsVisible then
            printlog "    Success: Back button does not work"
        else
            warnlog "Back button is not functionless"
            Kontext "ChartWizard"
            GoNext.Click
        endif        
        '///Enter valid range ChartFlexRange.A2:A5
        printlog "Enter valid range ChartFlexRange.A2:A5"
        Categories.SetText "ChartFlexRange.A2:A5"        
        '///Press Cancel button
        printlog "Press Cancel button"
        Kontext "ChartWizard"
        ChartWizard.Cancel        
        '///Close document
        printlog "Close document"
        Call hCloseDocument
    else
        warnlog "It was not possible to open the test document." 
    endif
endcase
'
'-------------------------------------------------------------------------
'
testcase tDataRangeDialogWriter
    dim sDataRange as string
    dim sInputFile as string
    sInputFile = convertpath(gTestToolPath & "chart2/required/input/ch_flexrange07.odt")
    '///Open test document <i>gTestToolPath</i>/chart2/required/input/ch_flexrange07.odt
    printlog "Open test document gTestToolPath/chart2/required/input/ch_flexrange07.odt"    
    if fOpenTestDocumentAndSelectDataRange(sInputFile) then                        
        '///Check data range in edit field (ChartFlexRange.A2:A3;ChartFlexRange.B1:E3)
        printlog "Check data range in edit field"
        Kontext "TabChartTypeDataRange"
        sDataRange = DataRange.GetText
        if sDataRange <> "ChartFlexRange.A2:A3;ChartFlexRange.B1:E3" then
            warnlog "This is not the expected datarange, it is " & sDataRange & " but should be ChartFlexRange.A2:A3;ChartFlexRange.B1:E3" 
        else
            printlog "    Datarange is correct"
        endif        
        '///Check if -Data series in columns- is enabled and chosen
        printlog "Check if -Data series in columns- is enabled and chosen"
        if Columns.IsEnabled then
            printlog "    Data series in columns is enabled"
            if Columns.IsChecked then
                printlog "        and checked"
            else
                warnlog "Data series in columns is not checked"
            endif
        else
            warnlog "Data series in columns is disabled"
        endif        
        '///Check if -Data series in rows- is enabled
        printlog "Check if -Data series in rows- is enabled"
        if Rows.IsEnabled then
            printlog "    Data series in rows is enabled" 
        else
            warnlog "Data series in rows is disabled"
        endif        
        '///Check if -First row as label- is enabled and checked
        printlog "Check if -First row as label- is enabled and checked"
        if FirstRowAsLabel.IsEnabled then
            printlog "    First row as label is enabled"
            if FirstRowAsLabel.IsChecked then
                printlog "        and checked"        
            else
                warnlog "First row as label is not checked"
            endif
        else
            warnlog "First row as label is disabled"
        endif        
        '///Check if -First column as label- is enabled and checked
        printlog "Check if -First column as label- is enabled and checked"
        if FirstColumnAsLabel.IsEnabled then
            printlog "    First column as label is enabled"
            if FirstColumnAsLabel.IsChecked then
                printlog "        and checked"
            else
                warnlog "First column as label is not checked"
            endif
        else
            warnlog "First column as label is disabled"
        endif        
        '///Enter invalid range ChartFlexRange.A1:E
        printlog "Enter invalid range ChartFlexRange.A1:E"
        DataRange.SetText "ChartFlexRange.A1:E"        
        '///Check that -Data series in rows- is disabled
        printlog "Check that -Data series in rows- is disabled"
        Kontext "TabChartTypeDataRange"
        if NOT Rows.IsEnabled then
            printlog "    Data series in rows is disabled" 
        else
            warnlog "Data series in rows is enabled, but should be disabled"
        endif        
        '///Check that -Data series in columns- is disabled
        printlog "Check that -Data series in columns- is disabled"
        if NOT Columns.IsEnabled then
            printlog "    Data series in columns is disabled" 
        else
            warnlog "Data series in columns is enabled, but should be disabled"
        endif        
        '///Check if -First row as label- is disabled
        printlog "Check if -First row as label- is disabled"
        if NOT FirstRowAsLabel.IsEnabled then
            printlog "    First row as label is disabled" 
        else
            warnlog "First row as label is enabled, but should be disabled"
        endif        
        '///Check if -First column as label- is disabled
        printlog "Check if -First column as label- is disabled"
        if NOT FirstColumnAsLabel.IsEnabled then
            printlog "    First Column as label is disabled" 
        else
            warnlog "First Column as label is enabled, but should be disabled"
        endif        
        '///Check that OK button is disabled
        printlog "Check that OK button is disabled"
        try
            Kontext "TabChartTypeDataRange"
            TabChartTypeDataRange.OK
            warnlog "The OK button is not disabled"
            goto endsub
        catch
            printlog "    The OK button is disabled"
        endcatch        
        '///Enter valid range ChartFlexRange.A1:E3
        printlog "Enter valid range ChartFlexRange.A1:E3"
        kontext "TabChartTypeDataRange"
        DataRange.SetText "ChartFlexRange.A1:E3"        
        '///Check if -Data series in columns- is enabled and chosen
        printlog "Check if -Data series in columns- is enabled and chosen"
        if Columns.IsEnabled then
            printlog "    Data series in columns is enabled"
            if Columns.IsChecked then
                printlog "        and checked"
            else
                warnlog "Data series in columns is not checked"
            endif
        else
            warnlog "Data series in columns is disabled"
        endif        
        '///Check if -Data series in rows- is enabled
        printlog "Check if -Data series in rows- is enabled"
        if Rows.IsEnabled then
            printlog "    Data series in rows is enabled" 
        else
            warnlog "Data series in rows is disabled"
        endif        
        '///Check if -First row as label- is enabled and checked
        printlog "Check if -First row as label- is enabled and checked"
        if FirstRowAsLabel.IsEnabled then
            printlog "    First row as label is enabled"
            if FirstRowAsLabel.IsChecked then
                printlog "        and checked"        
            else
                warnlog "First row as label is not checked"
            endif
        else
            warnlog "First row as label is disabled"
        endif        
        '///Check if -First column as label- is enabled and checked
        printlog "Check if -First column as label- is enabled and checked"
        if FirstColumnAsLabel.IsEnabled then
            printlog "    First column as label is enabled"
            if FirstColumnAsLabel.IsChecked then
                printlog "        and checked"
            else
                warnlog "First column as label is not checked"
            endif
        else
            warnlog "First column as label is disabled"
        endif        
        '///Press Cancel button
        printlog "Press Cancel button"
        Kontext "TabChartTypeDataRange"
        TabChartTypeDataRange.Cancel        
        '///Close document
        printlog "Close document"
        Call hCloseDocument
    else
        warnlog "It was not possible to open the test document."
    endif
endcase
'
'-------------------------------------------------------------------------
'
testcase tDataSeriesDialogWriter
    dim sInputFile as string
    dim iDataSeries as integer
    dim sDataRanges as string
    dim sRangeFor as string
    dim sCategories as string
    sInputFile = convertpath(gTestToolPath & "chart2/required/input/ch_flexrange08.odt")

    '/// Open test document <i>gTestToolPath</i>/chart2/required/input/ch_flexrange08.odt
    printlog "Open test document gTestToolPath/chart2/required/input/ch_flexrange08.odt"
    if fOpenTestDocumentAndSelectDataRange(sInputFile) then    
        Kontext
        '/// Switching to tab page Data Series
        printlog "Switching to tab page Data Series"
        active.SetPage TabChartTypeDataSeries
        Kontext "TabChartTypeDataSeries"
        call DialogTest ( TabChartTypeDataSeries )    
        '///Check if the data series listbox is enabled and contains four entries
        printlog "Check if the data series listbox is enabled and contains four entries"
        iDataSeries = DataSeries.GetItemCount
        if iDataSeries = 4 then
            printlog "    One data serie is available"
        else
            warnlog "There are " & iDataSeries & " data series instead of four"
        endif        
        '///Check if the ADD button is enabled and click the button
        printlog "Check if the ADD button is enabled and click the button"
        if AddButton.IsEnabled then
            printlog "    ADD button is enabled"
            AddButton.Click
        else
            warnlog "ADD button is not enabled"
        endif        
        '///Check if there are now five entries in the data series listbox
        printlog "Check if there are now five entries in the data series listbox"
        iDataSeries = DataSeries.GetItemCount
        if iDataSeries = 5 then
            printlog "    Adding a data series was successfull"
        else
            warnlog "There are " & iDataSeries & " data series instead of five"
        endif        
        '///Check if the new data series was added below the highlighted one
        printlog "Check if the new data series was added below the highlighted one"
        if DataSeries.GetSelIndex = 2 then
            printlog "    The new data series was inserted below the highlighted (first) one"
        else
            warnlog "The new dataseries was not inserted below the highlighted one"
        endif        
        '///Check if the REMOVE button is enabled and click the button
        printlog "Check if the REMOVE button is enabled and click the button"
        if RemoveButton.IsEnabled then
            printlog "    REMOVE button is enabled"
            RemoveButton.Click
        else
            warnlog "REMOVE button is not enabled"
        endif        
        '///Check if there are now four entries in the data series listbox
        printlog "Check if there are now four entries in the data series listbox"
        iDataSeries = DataSeries.GetItemCount
        if iDataSeries = 4 then
            printlog "    Removing a data series was successfull"
        else
            warnlog "There are " & iDataSeries & " data series instead of 4"
        endif        
        '///Use UP button and verify that it it disabled on first entry
        printlog "Use UP button and verify that it it disabled on first entry"
        DataSeries.Select(2)
        if MoveUp.IsEnabled then
            printlog "    The MoveUp button is enabled"
            MoveUp.Click
        else
            warnlog "The MoveUp button is disabled"
        endif
        if DataSeries.GetSelIndex = 1 then
            printlog "    Moving data series was successfull"
            if MoveUp.IsEnabled then
                warnlog "MoveUp button is not disabled on first entry"
            else
                printlog "        and MoveUp button is disabled on first enty"
            endif
        else
            warnlog "Moving data series was not successfull"
        endif        
        '///Use DOWN button thrice and verify that it is disabled on last entry
        printlog "Use DOWN button thrice and verify that it is disabled on last entry"
        DataSeries.Select(1)
        if MoveDown.IsEnabled then
            printlog "    The MoveDown button is enabled"
            MoveDown.Click  
            MoveDown.Click
            MoveDown.Click
        else
            warnlog "The MoveDown button is disabled"
        endif
        if DataSeries.GetSelIndex = 4 then
            printlog "    Moving data series was successfull"
            if MoveDown.IsEnabled then
                warnlog "MoveDown button is not disabled on first entry"
            else
                printlog "        and MoveDown button is disabled on first enty"
            endif
        else
            warnlog "Moving data series was not successfull"
        endif        
        '///Select first entry in Data Ranges
        printlog "Select first entry in Data Ranges"
        DataRanges.Select(1)            
        '///Verify that the range matches the one in edit field
        printlog "Verify that the range matches the one in edit field"
        'GetSelText(line,column) is a new parameter since CWS:SRC680gh13
        'Instead of 1,2 there must be 1,4 used becqause there are two invisible
        'label-columns in that treelist-box        
        sDataRanges = DataRanges.GetSelText (1,4)
        sRangeFor = RangeFor.GetText
        if sDataRanges = sRangeFor then
            printlog "    Both ranges are matching"
        else
            warnlog "The ranges does not match: " & sDataRanges & sRangeFor
        endif            
        '///Select second entry in Data Ranges
        printlog "Select second entry in Data Ranges"
        DataRanges.Select(2)        
        '///Check that the range in the edit field has changed,too
        printlog "Check that the range in the edit field has changed,too"
        'GetSelText(line,column) is a new parameter since CWS:SRC680gh13
        'Instead of 1,2 there must be 1,4 used becqause there are two invisible
        'label-columns in that treelist-box        
        sDataRanges = DataRanges.GetSelText (1,4)
        sRangeFor = RangeFor.GetText
        if sDataRanges = sRangeFor then
            printlog "    Both ranges are matching"
        else
            warnlog "The ranges does not match" & sDataRanges & sRangeFor
        endif            
        '///Change the range in edit field to ChartFlexRange.B2:B3
        printlog "Change the range in edit field to ChartFlexRange.B2:B3" 
        RangeFor.SetText "ChartFlexRange.B2:B3"        
        '///Check that the range in Data Ranges has changed, too
        printlog "Check that the range in Data Ranges has changed, too"
        'GetSelText(line,column) is a new parameter since CWS:SRC680gh13
        'Instead of 1,2 there must be 1,4 used becqause there are two invisible
        'label-columns in that treelist-box        
        sDataRanges = DataRanges.GetSelText (1,4)
        if sDataRanges = "ChartFlexRange.B2:B3" then
           printlog "    The data range has also changed"
        else
            warnlog "The data range has not changed"
        endif    
        '///Edit range for Categories (ChartFlexRange.A2:A3)
        printlog "Edit range for Categories (ChartFlexRange.A2:A3)"
        Categories.SetText "ChartFlexRange.A2:A3"
        sCategories = Categories.GetText
        if sCategories = "ChartFlexRange.A2:A3" then
            printlog "     The range has been modified"
        else
            warnlog "Modifiing the range was not successfull"
        endif        
        '///Enter invalid range ChartFlexRange.A2:A
        printlog "Enter invalid range ChartFlexRange.A2:A"
        Categories.SetText "ChartFlexRange.A2:A"        
        '///Check that the OK button is disabled
        printlog "Check that the OK button is disabled"
        try
            Kontext "TabChartTypeDataSeries"
            TabChartTypeDataSeries.OK
            warnlog "The OK button is not disabled"
            goto endsub
        catch
            printlog "    The OK button is disabled"
        endcatch        
        '///Enter valid range ChartFlexRange.A2:A4
        printlog "Enter valid range ChartFlexRange.A2:A4"
        kontext "TabChartTypeDataSeries"
        Categories.SetText "ChartFlexRange.A2:A4"        
        '///Press Cancel button
        printlog "Press Cancel button"
        Kontext "TabChartTypeDataSeries"
        TabChartTypeDataSeries.Cancel        
        '///Close document
        printlog "Close document"
        Call hCloseDocument
    else
        warnlog "It was not possible to open the test document."
    endif        
endcase
'
'-------------------------------------------------------------------------
'
function fOpenWriterTestDocumentAndInsertChart(sInputFile as STRING) as BOOLEAN
    fOpenWriterTestDocumentAndInsertChart = FALSE
    hFileOpenLocally(sInputFile)
    '///Menu Insert Object Chart to open chart wizard
    printlog "Menu Insert Object Chart to open chart wizard"
    InsertObjectChart
    WaitSlot()
    fOpenWriterTestDocumentAndInsertChart = TRUE
end function

