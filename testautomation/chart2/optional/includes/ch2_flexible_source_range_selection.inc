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
'* short description : Updatetest for Flexible Source Range Selection in spreadsheets
'*
'************************************************************************
'*
' #1 tDataRangeWizardCalc                   ' Updatetest for tabpage DataRange via chart wizard
' #1 tDataSeriesWizardCalc                  ' Updatetest for tabpage DataSeries via chart wizard
' #1 tDataRangeDialogCalc                   ' Updatetest for tabpage DataRange via data ranges dialog
' #1 tDataSeriesDialogCalc                  ' Updatetest for tabpage DataSeries via data ranges dialog
' #1 fOpenTestDocumentAndInsertChart        ' Sub routine which is being used by two test cases
'*
'\***********************************************************************

testcase tDataRangeWizardCalc
    dim sDataRange as string
    dim sInputFile as string
    sInputFile = convertpath(gTestToolPath & "chart2/required/input/ch_flexrange01.ods")    
    printlog "Open test document gtesttoolpath/chart2/required/input/ch_flexrange01.ods"
    if fOpenTestDocumentAndInsertChart(sInputFile) then
        Kontext "ChartType"    
        printlog "Choose the Chart type <i>bar</i> (should be the second in the top-down list)"
        if fSetChartType( "bar" ) > 0 then
            warnlog "Something went wrong setting chart type to BAR."
            warnlog "Testcase aborted"
            Kontext "ChartWizard"
            ChartWizard.Cancel
            call hCloseDocument
            goto endsub
        endif    
        printlog "Switch to tabpage Data Range"
        Kontext "ChartWizard"
        GoNext.Click            
        Kontext "TabChartTypeDataRange"
        call DialogTest ( TabChartTypeDataRange )    
        printlog "Check that the data range in edit field has the value $ChartFlexRange.$B$3:$E$6"        
        Kontext "TabChartTypeDataRange"
        sDataRange = DataRange.GetText
        if sDataRange <> "$ChartFlexRange.$B$3:$E$6" then
            warnlog "This is not the expected datarange, it is " & sDataRange & " but should be $ChartFlexRange.$B$3:$E$6" 
        else
            printlog "    Datarange is correct"
        endif    
        printlog "Check if -Data series in rows- is enabled"
        if Rows.IsEnabled then
            printlog "    Data series in rows is enabled" 
        else
            warnlog "Data series in rows is disabled"
        endif                    
        printlog "Check if Data series in columns- is enabled and chosen"
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
        printlog "Check if -First column as label- is enabled and not checked"
        if FirstColumnAsLabel.IsEnabled then
            printlog "    First column as label is enabled"
            if FirstColumnAsLabel.IsChecked then
                warnlog "First column as label is checked"
            else
                printlog "        and not checked"
            endif
        else
            warnlog "First column as label is disabled"
        endif            
        printlog "Press button Range Chooser"
        SelectDataRange.Click            
        printlog "Enter invalid range $ChartFlexRange.$B$3:$E$0 and close Range Chooser"    
        Kontext "AcceptOrRejectChangesArea"
        AreaEditField.SetText "$ChartFlexRange.$B$3:$E$0"
        ShrinkButton.Click            
        printlog "Check if -Data series in rows- is disabled"
        Kontext "TabChartTypeDataRange"
        if NOT Rows.IsEnabled then
            printlog "    Data series in rows is disabled" 
        else
            warnlog "Data series in rows is enabled, but should be disabled"
        endif            
        printlog "Check if -Data series in columns- is disabled"
        if NOT Columns.IsEnabled then
            printlog "    Data series in columns is disabled" 
        else
            warnlog "Data series in columns is enabled, but should be disabled"
        endif           
        printlog "Check if -First row as label- is disabled"
        if NOT FirstRowAsLabel.IsEnabled then
            printlog "    First row as label is disabled" 
        else
            warnlog "First row as label is enabled, but should be disabled"
        endif            
        printlog "Check if -First column as label- is disabled"
        if NOT FirstColumnAsLabel.IsEnabled then
            printlog "    First Column as label is disabled" 
        else
            warnlog "First Column as label is enabled, but should be disabled"
        endif            
        printlog "Check if Back, Next and Finish does not work"
        Kontext "ChartWizard"
        GoBack.Click
        Kontext "TabChartTypeDataRange"
        if FirstColumnAsLabel.IsVisible then
            printlog "    Success: Back button does not work"
        else
            warnlog "Back button is not functionless"
            Kontext "ChartWizard"
            GoNext.Click
        endif                
        printlog "Enter valid range $ChartFlexRange.$B$3:$E$6"
        Kontext "TabChartTypeDataRange"
        DataRange.SetText "$ChartFlexRange.$B$3:$E$6"            
        printlog "Check if -Data series in rows- is enabled"
        if Rows.IsEnabled then
            printlog "    Data series in rows is enabled" 
        else
            warnlog "Data series in rows is disabled"
        endif                    
        printlog "Check if -Data series in columns- is enabled and chosen"
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
        printlog "Check if -First column as label- is enabled and not checked"
        if FirstColumnAsLabel.IsEnabled then
            printlog "    First column as label is enabled"
            if FirstColumnAsLabel.IsChecked then
                warnlog "First column as label is checked"
            else
                printlog "        and not checked"
            endif
        else
            warnlog "First column as label is disabled"
        endif            
        printlog "Press Cancel button"
        Kontext "ChartWizard"
        ChartWizard.Cancel            
        printlog "Close document"
        Call hCloseDocument
    else
        warnlog "It was not possible to open the test document."
    endif
endcase
'
'-------------------------------------------------------------------------
'
testcase tDataSeriesWizardCalc
    dim sInputFile as string
    dim iDataSeries as integer
    dim sDataRanges as string
    dim sRangeFor as string
    dim sCategories as string
    sInputFile = convertpath(gTestToolPath & "chart2/required/input/ch_flexrange02.ods")    
    printlog "Open test document gTestToolPath/chart2/required/input/ch_flexrange02.ods"    
    if fOpenTestDocumentAndInsertChart(sInputFile) then        
        Kontext "ChartType"
        printlog "Choose the Chart type XY (SCATTER) (should be the second in the top-down list)"
        if fSetChartType( "xy" ) > 0 then
            warnlog "Something went wrong setting chart type to XY (SCATTER)."
            Kontext "ChartWizard"
            ChartWizard.Cancel
            call hCloseDocument
            goto endsub
        endif        
        Kontext "ChartWizard"        
        printlog "Click on Next button"
        GoNext.Click        
        printlog "Click on Next button"        
        GoNext.Click
        Kontext "TabChartTypeDataSeries"
        if RangeFor.IsVisible then
            printlog "    Switching to tabpage was successfull"
        else
            warnlog "Switching to tabpage Data Series failed"
            Kontext "ChartWizard"
            ChartWizard.Cancel
            call hCloseDocument            
            goto endsub
        endif        
        call DialogTest ( TabChartTypeDataSeries )                    
        printlog "Check if the data series listbox is enabled and contains three entries"
        iDataSeries = DataSeries.GetItemCount
        if iDataSeries = 4 then
            printlog "    All 4 data series are available"
        else
            warnlog "There are " & iDataSeries & " data series instead of 4"
        endif                
        printlog "Check if the ADD button is enabled and click the button"
        if AddButton.IsEnabled then
            printlog "    ADD button is enabled"
            AddButton.Click
        else
            warnlog "ADD button is not enabled"
        endif                    
        printlog "Check if there are now four entries in the data series listbox"
        iDataSeries = DataSeries.GetItemCount
        if iDataSeries = 5 then
            printlog "    Adding a data series was successfull"
        else
            warnlog "There are " & iDataSeries & " data series instead of 5"
        endif                
        printlog "Check if the second data series was added below the highlighted one"
        if DataSeries.GetSelIndex = 2 then
            printlog "    The new data series was inserted below the highlighted (first) one"
        else
            warnlog "The new dataseries was not inserted below the highlighted one"
        endif                
        printlog "Check if the REMOVE button is enabled and click the button"
        if RemoveButton.IsEnabled then
            printlog "    REMOVE button is enabled"
            RemoveButton.Click
        else
            warnlog "REMOVE button is not enabled"
        endif                
        printlog "Check if there are now 4 entries in the data series listbox"
        iDataSeries = DataSeries.GetItemCount
        if iDataSeries = 4 then
            printlog "    Removing a data series was successfull"
        else
            warnlog "There are " & iDataSeries & " data series instead of 3"
        endif                
        printlog "Use UP button on last data series and verify that it it disabled on first entry"
        DataSeries.Select(3)
        if MoveUp.IsEnabled then
            printlog "    The MoveUp button is enabled"            
            printlog "Click on Move Up button twice."
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
        printlog "Use DOWN button and verify that it is disabled on last entry"
        DataSeries.Select(1)
        if MoveDown.IsEnabled then
            printlog "    The MoveDown button is enabled"            
            printlog "Click Move Down button three times"
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
        printlog "Select first entry in Data Ranges"
        DataRanges.Select(1)                
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
        printlog "Select second entry in Data Ranges"
        DataRanges.Select(2)                
        printlog "Check that the range in the edit field has changed, too."
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
        printlog "Change the range in edit field"
        RangeFor.SetText "$ChartFlexRange.$D$5:$D$6;$ChartFlexRange.$D$8"                    
        printlog "Check that the range in Data Ranges has changed, too"
        'GetSelText(line,column) is a new parameter since CWS:SRC680gh13
        'Instead of 1,2 there must be 1,4 used becqause there are two invisible
        'label-columns in that treelist-box        
        sDataRanges = DataRanges.GetSelText (1,4)
        if sDataRanges = "$ChartFlexRange.$D$5:$D$6;$ChartFlexRange.$D$8" then
           printlog "    The data range has also changed"
        else
            warnlog "The data range has not changed"
        endif                
        printlog "Click on Range Chooser and change the range"
        SelectRange.Click
        Kontext "AcceptOrRejectChangesArea"        
        printlog "Enter $ChartFlexRange.$D$5:$D$6 in edit field"
        AreaEditField.SetText "$ChartFlexRange.$D$5:$D$6"        
        printlog "Click on Shirnk button"
        ShrinkButton.Click                    
        printlog "Check if the range in Data ranges has changed"
        Kontext "TabChartTypeDataSeries"
        'GetSelText(line,column) is a new parameter since CWS:SRC680gh13
        'Instead of 1,2 there must be 1,4 used becqause there are two invisible
        'label-columns in that treelist-box        
        sDataRanges = DataRanges.GetSelText (1,4)
        if sDataRanges = "$ChartFlexRange.$D$5:$D$6" then
           printlog "    The data range has also changed"
        else
            warnlog "The data range has not changed"
        endif                
        printlog "Edit range for Data Labels"        
        printlog "Set text $ChartFlexRange.$C$15:$C$16 in categories"
        Categories.SetText "$ChartFlexRange.$C$15:$C$16"
        sCategories = Categories.GetText        
        printlog "Validate the changes have not been modified"
        if sCategories = "$ChartFlexRange.$C$15:$C$16" then
            printlog "    The range has been modified"
        else
            warnlog "Modifiing the range was not successfull"
        endif                
        printlog "Click on Range Chooser and change the range"
        SelectCategories.Click
        Kontext "AcceptOrRejectChangesArea"        
        printlog "Set text $ChartFlexRange.$C$5:$C$6 in the edit field"
        AreaEditField.SetText "$ChartFlexRange.$C$5:$C$6"        
        printlog "Click on Shrink button"
        ShrinkButton.Click                
        printlog "Verify that the range is taken"
        Kontext "TabChartTypeDataSeries"
        sCategories = Categories.GetText
        if sCategories = "$ChartFlexRange.$C$5:$C$6" then
            printlog "    The range has been modified"
        else
            warnlog "Modifiing the range was not successfull"
        endif                
        printlog "Enter invalid range $ChartFlexRange.$C$5:$C$"        
        Categories.SetText "$ChartFlexRange.$C$5:$C$"                        
        Kontext "ChartWizard"       
        printlog "Click on the Back button"
        GoBack.Click
        Kontext "TabChartTypeDataSeries"
        if Categories.IsVisible then
            printlog "    Success: Back button is disabled"
        else
            warnlog "Back button is not functionless"
            Kontext "ChartWizard"
            GoNext.Click
        endif                
        printlog "Enter $ChartFlexRange.$C$5:$C$6 - a valid range"
        Categories.SetText "$ChartFlexRange.$C$5:$C$6"                
        printlog "Press Range Chooser"
        SelectCategories.Click                
        printlog "Press <END>-key"        
        Kontext "AcceptOrRejectChangesArea"
        AreaEditField.TypeKeys "<END>"                
        printlog "Enter a semicolon and the second range $ChartFlexRange.$C$8"
        AreaEditField.TypeKeys ";$ChartFlexRange.$C$8"                
        printlog "Close Range Chooser"
        ShrinkButton.Click                
        printlog "Verify that there is a multiple range"
        Kontext "TabChartTypeDataSeries"
        sCategories = Categories.GetText
        if sCategories = "$ChartFlexRange.$C$5:$C$6;$ChartFlexRange.$C$8" then
            printlog "    The range has been modified to a multiple range"
        else
            warnlog "Modifiing the range was not successfull"
        endif                
        printlog "Press Cancel button"
        Kontext "ChartWizard"
        ChartWizard.Cancel                
        printlog "Close document"
        Call hCloseDocument
    else
        warnlog "It was not possible to open the test document."
    endif
endcase

'-------------------------------------------------------------------------

testcase tDataRangeDialogCalc
    dim sDataRange as string
    dim sInputFile as string
    sInputFile = convertpath(gTestToolPath & "chart2/required/input/ch_flexrange03.ods" )    
    printlog "Open test document gTestToolPath/chart2/required/input/ch_flexrange03.ods"        
    if fOpenTestDocumentAndSelectDataRange(sInputFile) then            
        printlog "Check if the data range in edit field is $ChartFlexRange.$D$4:$G$6;$ChartFlexRange.$C$5:$G$6;$ChartFlexRange.$C$4"
        Kontext "TabChartTypeDataRange"
        sDataRange = DataRange.GetText
        if sDataRange <> "$ChartFlexRange.$C$4:$G$6" then
            warnlog "This is not the expected datarange, it is " & sDataRange & " but should be $ChartFlexRange.$C$4:$G$6" 
        else
            printlog "    Datarange is correct"
        endif            
        printlog "Check if -Data series in columns- is enabled"
        if Columns.IsEnabled then
            printlog "    Data series in columns is enabled" 
        else
            warnlog "Data series in columns is disabled"
        endif                    
        printlog "Check if -Data series in columns- is enabled and chosen"
        if Rows.IsEnabled then
            printlog "    Data series in rows is enabled"
            if rows.IsChecked then
                printlog "        and checked"
            else
                warnlog "Data series in rows is not checked"
            endif
        else
            warnlog "Data series in rows is disabled"
        endif            
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
        printlog "Press button Range Chooser"
        SelectDataRange.Click            
        printlog "Enter $ChartFlexRange.$B$3:$E$0 in Range Chooser and close Range Chooser"    
        Kontext "AcceptOrRejectChangesArea"
        AreaEditField.SetText "$ChartFlexRange.$B$3:$E$0"
        ShrinkButton.Click            
        printlog "Check if -Data series in rows- is disabled"
        Kontext "TabChartTypeDataRange"
        if NOT Rows.IsEnabled then
            printlog "    Data series in rows is disabled" 
        else
            warnlog "Data series in rows is enabled, but should be disabled"
        endif            
        printlog "Check if -Data series in columns- is disabled"
        if NOT Columns.IsEnabled then
            printlog "    Data series in columns is disabled" 
        else
            warnlog "Data series in columns is enabled, but should be disabled"
        endif            
        printlog "Check if -First row as label- is disabled"
        if NOT FirstRowAsLabel.IsEnabled then
            printlog "    First row as label is disabled" 
        else
            warnlog "First row as label is enabled, but should be disabled"
        endif            
        printlog "Check if -First column as label- is disabled"
        if NOT FirstColumnAsLabel.IsEnabled then
            printlog "    First Column as label is disabled" 
        else
            warnlog "First Column as label is enabled, but should be disabled"
        endif            
        printlog "Check if OK button is disabled"
        try
            Kontext "TabChartTypeDataRange"
            TabChartTypeDataRange.OK
            warnlog "The OK button is not disabled"
            goto endsub
        catch
            printlog "    The OK button is disabled"
        endcatch            
        printlog "Enter a valid range"
        Kontext "TabChartTypeDataRange"
        DataRange.SetText "$ChartFlexRange.$C$4:$G$6;$ChartFlexRange.$C$8:$G$8"            
        printlog "Check if -Data series in columns- is enabled"
        if Columns.IsEnabled then
            printlog "    Data series in columns is enabled" 
        else
            warnlog "Data series in columns is disabled"
        endif                    
        printlog "Check if -Data series in columns- is enabled and chosen"
        if Rows.IsEnabled then
            printlog "    Data series in rows is enabled"
            if rows.IsChecked then
                printlog "        and checked"
            else
                warnlog "Data series in rows is not checked"
            endif
        else
            warnlog "Data series in rows is disabled"
        endif            
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
        printlog "Press Cancel button"
        Kontext "TabChartTypeDataRange"
        TabChartTypeDataRange.Cancel            
        printlog "Close document"
        Call hCloseDocument
    else
        warnlog "It was not possible to open the test document."
    endif
endcase
'
'-------------------------------------------------------------------------
'
testcase tDataSeriesDialogCalc
    dim sInputFile as string
    dim iDataSeries as integer
    dim sDataRanges as string
    dim sRangeFor as string
    dim sCategories as string
    sInputFile = convertpath(gTestToolPath & "chart2/required/input/ch_flexrange04.ods")    
    printlog "Open test document gTestToolPath/chart2/required/input/ch_flexrange04.ods"    
    if fOpenTestDocumentAndSelectDataRange(sInputFile) then
        Kontext        
        Active.SetPage TabChartTypeDataSeries
        Kontext "TabChartTypeDataSeries"        
        printlog "Check if the data series listbox is enabled and contains one entry"
        iDataSeries = DataSeries.GetItemCount
        if iDataSeries = 1 then
            printlog "    One data serie is available"
        else
            warnlog "There are " & iDataSeries & " data series instead of 1"
        endif                
        printlog "Check if the ADD button is enabled and click the button"
        if AddButton.IsEnabled then
            printlog "    ADD button is enabled"
            AddButton.Click
        else
            warnlog "ADD button is not enabled"
        endif                
        printlog "Check if there are now two entries in the data series listbox"
        iDataSeries = DataSeries.GetItemCount
        if iDataSeries = 2 then
            printlog "    Adding a data series was successfull"
        else
            warnlog "There are " & iDataSeries & " data series instead of 2"
        endif                
        printlog "Check if the new data series was added below the highlighted one"
        if DataSeries.GetSelIndex = 2 then
            printlog "    The new data series was inserted below the highlighted (first) one"
        else
            warnlog "The new dataseries was not inserted below the highlighted one"
        endif                
        printlog "Use UP button on last data series and verify that it it disabled on first entry"
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
        printlog "Use DOWN button and verify that it is disabled on last entry"
        DataSeries.Select(1)
        if MoveDown.IsEnabled then
            printlog "    The MoveDown button is enabled"
            MoveDown.Click        
        else
            warnlog "The MoveDown button is disabled"
        endif
        if DataSeries.GetSelIndex = 2 then
            printlog "    Moving data series was successfull"
            if MoveDown.IsEnabled then
                warnlog "MoveDown button is not disabled on first entry"
            else
                printlog "        and MoveDown button is disabled on first enty"
            endif
        else
            warnlog "Moving data series was not successfull"
        endif                
        printlog "Check if the REMOVE button is enabled and click the button"
        if RemoveButton.IsEnabled then
            printlog "    REMOVE button is enabled"
            RemoveButton.Click
        else
            warnlog "REMOVE button is not enabled"
        endif                
        printlog "Check if there are now one entry in the data series listbox"
        iDataSeries = DataSeries.GetItemCount
        if iDataSeries = 1 then
            printlog "    Removing a data series was successfull"
        else
            warnlog "There are " & iDataSeries & " data series instead of 1"
        endif                
        printlog "Select first entry in Data Ranges"
        DataRanges.Select(1)                    
        printlog "Verify that the range for NAME matches the one in edit field"
        'GetSelText(line,column) is a new parameter since CWS:SRC680gh13
        'Instead of 1,2 there must be 1,4 used becqause there are two invisible
        'label-columns in that treelist-box        
        sDataRanges = DataRanges.GetSelText(1,4)
        sRangeFor = RangeFor.GetText
        if sDataRanges = sRangeFor then
            printlog "    Both ranges are matching"
        else
            warnlog "The ranges does not match: " & sDataRanges & sRangeFor
        endif                
        printlog "Select second entry in Data Ranges"
        DataRanges.Select(2)                
        printlog "Check that the range in the edit field has changed too"
        'GetSelText(line,column) is a new parameter since CWS:SRC680gh13
        'Instead of 1,2 there must be 1,4 used becqause there are two invisible
        'label-columns in that treelist-box        
        sDataRanges = DataRanges.GetSelText(1,4)
        sRangeFor = RangeFor.GetText
        if sDataRanges = sRangeFor then
            printlog "    Both ranges are matching"
        else
            warnlog "The ranges does not match" & sDataRanges & sRangeFor
        endif                
        printlog "Change the range in edit field"
        RangeFor.SetText "$ChartFlexRange.$D$5:$D$10"                    
        printlog "Check that the range in Data Ranges has changed too"
        'GetSelText(line,column) is a new parameter since CWS:SRC680gh13
        'Instead of 1,2 there must be 1,4 used becqause there are two invisible
        'label-columns in that treelist-box        
        sDataRanges = DataRanges.GetSelText (1,4)
        if sDataRanges = "$ChartFlexRange.$D$5:$D$10" then
           printlog "    The data range has also changed"
        else
            warnlog "The data range has not changed"
        endif            
        printlog "Click on Range Chooser and change the range"
        SelectRange.Click
        Kontext "AcceptOrRejectChangesArea"
        AreaEditField.SetText "$ChartFlexRange.$D$5:$D$12"
        ShrinkButton.Click                    
        printlog "Check if the range in Data ranges has changed"
        Kontext "TabChartTypeDataSeries"
        'GetSelText(line,column) is a new parameter since CWS:SRC680gh13
        'Instead of 1,2 there must be 1,4 used becqause there are two invisible
        'label-columns in that treelist-box        
        sDataRanges = DataRanges.GetSelText (1,4)
        if sDataRanges = "$ChartFlexRange.$D$5:$D$12" then
           printlog "    The data range has also changed"
        else
            warnlog "The data range has not changed"
        endif            
        printlog "Edit range for Categories"
        Categories.SetText "$ChartFlexRange.$B$5:$B$10"
        sCategories = Categories.GetText
        if sCategories = "$ChartFlexRange.$B$5:$B$10" then
            printlog "    The range has been modified"
        else
            warnlog "Modifiing the range was not successfull"
        endif                
        printlog "Click on Range Chooser and change the range to $ChartFlexRange.$B$5:$B$12"
        SelectCategories.Click
        Kontext "AcceptOrRejectChangesArea"
        AreaEditField.SetText "$ChartFlexRange.$B$5:$B$12"
        ShrinkButton.Click                
        printlog "Verify that the range is taken"
        Kontext "TabChartTypeDataSeries"
        sCategories = Categories.GetText
        if sCategories = "$ChartFlexRange.$B$5:$B$12" then
            printlog "    The range has been modified"
        else
            warnlog "Modifiing the range was not successfull"
       endif                
        printlog "Enter $ChartFlexRange.$B$5:$B$ as an invalid range"
        Categories.SetText "$ChartFlexRange.$B$5:$B$"                
        printlog "Check that the OK button does not work"
        try
            Kontext "TabChartTypeDataSeries"
            TabChartTypeDataSeries.OK
            warnlog "The OK button is not disabled"
            goto endsub
        catch
            printlog "    The OK button is disabled"
        endcatch                
        printlog "Enter as valid range $ChartFlexRange.$B$5:$B$12"
        Kontext "TabChartTypeDataSeries"
        Categories.SetText "$ChartFlexRange.$B$5:$B$12"                
        printlog "Press Range Chooser"
        SelectCategories.Click                
        printlog "Move cursor to the end"
        Kontext "AcceptOrRejectChangesArea"
        AreaEditField.TypeKeys "<END>"                
        printlog "Enter a semicolon and the second range $ChartFlexRange.$B$14"
        AreaEditField.TypeKeys ";$ChartFlexRange.$B$14"                
         printlog "Close Range Chooser"
        ShrinkButton.Click                 
        printlog "Verify that there is a multiple range"
        Kontext "TabChartTypeDataSeries"
        sCategories = Categories.GetText
        if sCategories = "$ChartFlexRange.$B$5:$B$12;$ChartFlexRange.$B$14" then
            printlog "    The range has been modified to a multiple range"
        else
            warnlog "Modifiing the range was not successfull"
        endif                
        printlog "Press Cancel button"
        Kontext "TabChartTypeDataSeries"
        TabChartTypeDataSeries.Cancel                
        printlog "Close document"
        Call hCloseDocument
    else
        warnlog "It was not possible to open the test document."
    endif
endcase

