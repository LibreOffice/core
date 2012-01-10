'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
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
'*************************************************************************

'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : Data Series dialog test cases
'*
'************************************************************************
'*
' #1 tPlotMissingValuesBar
' #1 tPlotMissingValuesLine
'*
'\************************************************************************

testcase tPlotMissingValuesBar
    Dim iCountTabPg as integer    
    iCountTabPg = 0   
    Dim iTestRadio as integer
    iTestRadio = 99
    printlog "Load simple chart document"
    
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    printlog "Go one row down"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<DOWN>"
    printlog "Insert / Rows to insert one empty row in the data series"
    InsertRowsCalc    
    printlog "Select chart using navigator"
    if fSelectFirstOLE = 0 then        
        printlog "Edit / Object / Edit"
        EditObjectEdit        
        printlog "Select DataSeries A by toolbar selectorbox"
	call fChartSelectElement (8)
	printlog "Open the properties dialog by using the FormatSelection button in the toolbar"
	FormatSelection.Click
        printlog "Select tab page Options"
        Kontext
        iCountTabPg = Active.GetPageCount
        if iCountTabPg <> 4 then
            warnlog "Edit / Properties on a data series should have 4 tab pages but has " & iCountTabPg & "."
        endif
        Active.SetPage TabDataSeriesOptions        
        Kontext "TabDataSeriesOptions"
        call DialogTest ( TabDataSeriesOptions )        
        printlog "Verify that 'Leave gap' is checked (default)"
        if NOT LeaveGap.IsChecked then
            warnlog "'Leave gap' should be the default setting in bar charts!"
        endif
        printlog "Check that 'ContinueLine' is disabled"
        if ContinueLine.IsEnabled then
            warnlog "Continue Line radio button is enabled but should not in bar charts!"
        endif                                       
        printlog "Select 'AssumeZero' and close the dialog with OK"
        iTestRadio = fSetRadioButton(AssumeZero)
        if iTestRadio = 2 then
            warnlog "Assume Zero radio button was not checked."
            elseif iTestRadio = 11 then
            warnlog "Assume Zero radio button is not visible."
            elseif iTestRadio = 12 then
            warnlog "Assume Zero radio button is not enabled."
        endif                       
        Kontext
        printlog "Clicking OK"         
        TabDataSeriesOptions.OK
        printlog "Invoke Format / Object Properties"
        FormatSelection
        Kontext
        Active.SetPage TabDataSeriesOptions        
        Kontext "TabDataSeriesOptions"        
        printlog "Check if 'AssumeZero' is selected"
        if NOT AssumeZero.IsChecked then
            warnlog "'AssumeZero' should be selected."
        endif
        TabDataSeriesOptions.Cancel                
    else
        warnlog "It was not possible to select the chart object!"
    endif
    printlog "Close document"
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tPlotMissingValuesLine
    Dim iCountTabPg as integer    
    iCountTabPg = 0
    Dim iTestRadio as integer
    iTestRadio = 99
    
    printlog "Load simple chart document"
    
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    printlog "Go one row down"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<DOWN>"
    printlog "Insert / Rows to insert one empty row in the data series"
    InsertRowsCalc    
    printlog "Select chart using navigator"
    if fSelectFirstOLE = 0 then        
        printlog "Edit / Object / Edit"
        EditObjectEdit
        if fInvokeChartTypeDialog <> 0 then
            warnlog "Impossible to invike the chart type dialog!"
            goto endsub
        endif
        if fSetChartType("line") <> 0 then
            warnlog "Selecting chart type 'line' failed!"
            goto endsub            
        end if
        if fSetChartTypeVariant(2) <> 0 then
            warnlog "Selecting chart type 'line' variant 2 failed!"
            goto endsub            
        end if        
        if fCloseChartTypeDialogOK <> 0 then
            warnlog "Closing Chart Type dialog with OK failed!"
            goto endsub            
        end if
        printlog "Select DataSeries A by toolbar selectorbox"
        call fChartSelectElement (8)
        printlog "Invoke Format / Object Properties"
        FormatSelection.Click
        printlog "Select tab page Options"
        Kontext
        iCountTabPg = Active.GetPageCount
        if iCountTabPg <> 2 then
            warnlog "Edit / Properties on a data series of a line chart should have 2 tab pages but has " & iCountTabPg & "."
        endif
        Active.SetPage TabDataSeriesOptions        
        Kontext "TabDataSeriesOptions"
        call DialogTest ( TabDataSeriesOptions )        
        printlog "Verify that 'Leave gap' is checked (default)"
        if NOT LeaveGap.IsChecked then
            warnlog "'Leave gap' should be the default setting in line charts!"
        endif
        printlog "Select 'AssumeZero' and close the dialog with OK"
        iTestRadio = fSetRadioButton(AssumeZero)
        if iTestRadio = 2 then
            warnlog "Assume Zero radio button was not checked."
            elseif iTestRadio = 11 then
            warnlog "Assume Zero radio button is not visible."
            elseif iTestRadio = 12 then
            warnlog "Assume Zero radio button is not enabled."
        endif                               
        Kontext
        printlog "Clicking OK"         
        TabDataSeriesOptions.OK
        printlog "Invoke Format / Object Properties"
        FormatSelection
        Kontext
        Active.SetPage TabDataSeriesOptions        
        Kontext "TabDataSeriesOptions"        
        printlog "Check if 'AssumeZero' is selected"
        if NOT AssumeZero.IsChecked then
            warnlog "'AssumeZero' should be selected."
        endif
        printlog "Select 'ContinueLine' and close the dialog with OK"
        iTestRadio = 99
        iTestRadio = fSetRadioButton(ContinueLine)
        if iTestRadio = 2 then
            warnlog "Continue Line radio button was not checked."
            elseif iTestRadio = 11 then
            warnlog "Continue Line radio button is not visible."
            elseif iTestRadio = 12 then
            warnlog "Continue Line radio button is not enabled."
        endif                                       
        Kontext
        TabDataSeriesOptions.OK
        printlog "Invoke Format / Object Properties"
        FormatSelection
        Kontext
        Active.SetPage TabDataSeriesOptions        
        Kontext "TabDataSeriesOptions"                
        printlog "Check if 'ContinueLine' is selected"
        if NOT ContinueLine.IsChecked then
            warnlog "'ContinueLine' should be selected."
        endif                
        printlog "Close dialog with Cancel-button"
        TabDataSeriesOptions.Cancel
    else
        warnlog "It was not possible to select the chart object!"
    endif
    printlog "Close document"
    Call hCloseDocument        
endcase
