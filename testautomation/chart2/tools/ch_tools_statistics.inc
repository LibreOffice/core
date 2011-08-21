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
'* short description : Tool library for Statistics dialog
'*
'**************************************************************************************************
'*
' #1 fInvokeTrendlinesDialog
' #1 fInvokeInsertYErrorBarsDialog
' #1 fInsertMeanValueLines
' #1 fSetErrorCategory
' #1 fSetErrorValue
' #1 fSetErrorIndicatorItem
' #1 fSetRegressionCurvesItem
'*
'\************************************************************************************************

' ch_tools_statistics.inc - Library for automation of the 'Insert::Statistics...' dialog
' This Include contains a set of functions to modify the controls of the Statistics dialog.
' All functions are designed to return error-codes depending on the behaviour of the action applied.
' Return codes:
' Error 0: Success.
' Error 1: The basic action beeing applied caused a serious problem, e.g. a crash.
' Error 2 TO 9: A functional problem occurred.
' Error 11 TO 19: Wrong marginal conditions end up in failture, e.g. control not visible.
' NOTE: This errors can also be used for 'negative' testing.
' Error 42: Wrong input. Probably only of interest during test development.
' Error 99: Unexpected behaviour - Shouldn't normally occur.
' ATTENTION:
' Only Errors 42 and 99 throw 'warnlogs'.
' All other errors are silent.
' They only throw QAErrorlogs the give a hint what probably went wrong.
' Expected Errors MUST exclusivly be handled by the calling routine.
'
'--------------------------------------------------------------------
'
function fInvokeTrendlinesDialog() as INTEGER
         fInvokeTrendlinesDialog = 99
' Function to invoke the 'Insert::Statistics...' dialog
' No Input
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to invoke the dialog
' 2 := Failure (Dialog not present after invocation)
'99 := Unexpected error
    
    printlog "** Invoking 'Insert::Trendlines...' in menu"
    '/// Execute menu item 'Insert::Trendlines...'
    try 
        InsertTrendlines
    catch
        qaErrorLog "Error 1: Invoking menu item 'Insert::Trendlines...' failed"
        fInvokeTrendlinesDialog = 1
        exit function
    endcatch
    
    '/// Lookup if call was successful
	Kontext
	active.SetPage (TabTrendLineType)
    Kontext "TabTrendLineType"
    if TabTrendLineType.exists(2) then
        printlog ">> Trend lines dialog is visible now."
        fInvokeTrendlinesDialog = 0
    else
        qaErrorLog "Error 2: OOPS, calling the slot '.uno:InsertTrendlines' didn't cause any problem ..."
        qaErrorLog "..., BUT the dialog doesn't seem to be visible"
        fInvokeTrendlinesDialog = 2
    endif

    if fInvokeTrendlinesDialog = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif    
end function
'
'--------------------------------------------------------------------
'
function fInvokeInsertYErrorBarsDialog() as INTEGER
         fInvokeInsertYErrorBarsDialog = 99
' Function to invoke the 'Insert::InsertYErrorBars...' dialog
' No Input
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to invoke the dialog
' 2 := Failure (Dialog not present after invocation)
'99 := Unexpected error    
    printlog "** Invoking 'Insert::InsertYErrorBars...' in menu"
    '/// Execute menu item 'Insert::InsertYErrorBars...'
    try 
        InsertYErrorBars
    catch
        qaErrorLog "Error 1: Invoking menu item 'Insert::YErrorBars...' failed"
        fInvokeInsertYErrorBarsDialog = 1
        exit function
    endcatch
    
    '/// Lookup if call was successful
    Kontext "InsertYErrorBarsDLG"
    if InsertYErrorBarsDLG.exists(2) then
        printlog ">> InsertYErrorBars dialog is visible now."
        fInvokeInsertYErrorBarsDialog = 0
    else
        qaErrorLog "Error 2: OOPS, calling the slot '.uno:InsertYErrorBars' didn't cause any problem ..."
        qaErrorLog "..., BUT the dialog doesn't seem to be visible"
        fInvokeInsertYErrorBarsDialog = 2
    endif
    
    if fInvokeInsertYErrorBarsDialog = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
end function
'
'-------------------------------------------------------------------------
'
function fInsertMeanValueLines() as INTEGER
         fInsertMeanValueLines = 99
' Function to insert mean value lines in an activated chart object
' Input:
    QAErrorLog "#i86055#-Insert/ Mean Value Lines needs a check box in sub-menu to remove all lines with once click"
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to execute the command
' 2 := Serious problem trying to check if the line has been inserted
' 3 := Uncorrect selected object
' 4 := Format Line-dialog did not occour
'99 := Unexpected error
    
    printlog "** Executing 'Insert::MeanValueLines...' in menu"
    '/// Execute menu item 'Insert::MeanValueLines...'
    try 
        InsertMeanValueLines
    catch
        qaErrorLog "Error 1: Executing menu item 'Insert::MeanValueLines...' failed"
        fInsertMeanValueLines = 1
        exit function
    endcatch
    
    Kontext "DocumentChart"
    '/// Type three times TAB-key to select the first data series
    DocumentChart.TypeKeys "<TAB>" , 3
    '/// Type F3-function key to select the mean value line of that data series
    DocumentChart.TypeKeys "<F3>"
    '/// Open the context menu
    DocumentChart.OpenContextMenu TRUE     
    if hMenuItemGetCount = 6 then       
        '/// Select the first entry (properties)
        hMenuSelectNr (1)
    else
        qaErrorLog "Error 2: Opening context menu on inserted mean value line failed!"        
        fInsertMeanValueLines = 2
        exit function        
    endif
    Kontext
    '/// There should be one tab page (chart line)
    Active.SetPage TabFormatChartLine
    if Active.GetPageCount <> 1 then
        qaErrorLog "Error 3: Incorrect selected object!"        
        fInsertMeanValueLines = 3
        Active.Cancel        
        exit function
    endif
    Kontext "TabFormatChartLine"
    if TabFormatChartLine.Exists(1) then        
        fInsertMeanValueLines = 0
        '/// Leave the dialog with CANCEL
        TabFormatChartLine.CANCEL
        printlog ">> Inserting 'Mean Value Lines' seems to work"
    else
        qaErrorLog "Error 4: Format Line-dialog did not occour!"        
        fInsertMeanValueLines = 4        
        exit function                            
    end if          
    if fInsertMeanValueLines = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function
'
'--------------------------------------------------------------------
'
function fSetErrorCategory ( oErrorCategory as OBJECT ) as INTEGER
         fSetErrorCategory = 99
' Function to select 'Error Category'
' Input:
' OBJECT oErrorCategory
' Declaration name of Radio-Button (valid values):
' - NoFunction
' - Variance<
' - StandardDeviation
' - Percentage 
' - ErrorMargin
' - ConstantValue
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to check 'Error Category' radio-button
' 2 := 'Error Category' radio-button was not set
'12 := Radio-Button for desired 'Error Category' is not enabled
'42 := User error, input doesn't match.
'99 := Unexpected error
    printlog "** Setting 'Error Category'"
    '/// Check if desired 'Error Category' radio-button is enabled
    Kontext "InsertStatisticsDialog"
    if NOT oErrorCategory.IsEnabled then
        qaErrorLog "Error 12: Radio-Button for desired 'Error Category' is not enabled"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetErrorCategory = 12
        exit function
    endif
    '/// Try to check desired 'Error Category'
    Kontext "InsertStatisticsDialog"
    try 
        oErrorCategory.Check
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Check 'Error Category' radio-button seems to cause a serious problem -> Check this out!"
        fSetErrorCategory = 1
        exit function
    endcatch
    '/// Verify (against input) if 'Error Category' radio-button was checked
    if oErrorCategory.IsChecked then
        fSetErrorCategory = 0
        printlog ">> Setting 'Error Category' radio-button seems to work"
    else
        qaErrorLog "Error 2: Desires 'Error Category' radio-button was not checked -> Check this out!"
        fSetErrorCategory = 2
    endif

    if fSetErrorCategory = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif  
end function
'
'--------------------------------------------------------------------
'
function fSetErrorValue ( oErrorValue as OBJECT , _
                          sErrorValue as STRING , _
                          OPTIONAL bCheckResult as BOOLEAN , _
                          OPTIONAL sExpectedValue as STRING ) as INTEGER
         fSetErrorValue = 99
' Function to set 'Error Value'
' Input:
' OBJECT oErrorValue
' Declaration name of Spin-Field (valid values):
' - PercentageValue
' - ErrorMarginValue
' - ConstantValuePlus
' - ConstantValueMinus
' - STRING (numeric) sErrorValue
' - Value to assign to Spin-Field
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to check 'Error Value' Spin-Field
' 2 := 'Error Value' Spin-Field was not set
'12 := Spin-Field for desired 'Error Value' is not enabled
'42 := User error, input doesn't match.
'99 := Unexpected error
    if isMissing ( bCheckResult ) then
        bCheckResult = FALSE
    endif
    if isMissing ( sExpectedValue ) then
        sExpectedValue = sErrorValue
    endif
    printlog ""
    printlog "** Setting 'Error Value'"
    '/// Check if desired 'Error Value' Spin-Field is enabled
    Kontext "InsertStatisticsDialog"
    if NOT oErrorValue.IsEnabled then
        qaErrorLog "Error 12: Spin-Field for desired 'Error Value' is not enabled"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetErrorValue = 12
        exit function
    endif
    '/// Try to set desired 'Error Value'
    Kontext "InsertStatisticsDialog"
    try 
        oErrorValue.setText ( sErrorValue )
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Setting 'Error Value' Spin-Field seems to cause a serious problem -> Check this out!"
        fSetErrorValue = 1
        exit function
    endcatch
    '/// Verify (against input) if 'Error Value' Spin-Field was set correctly
    if bCheckResult then
        if oErrorValue.getText = sExpectedValue then
            fSetErrorValue = 0
            printlog ">> Setting 'Error Value' Spin-Field seems to work"
        else
            qaErrorLog "Error 2: Desires 'Error Value' Spin-Field was not set correctly (value out of range?) -> Check this out!"
            qaErrorLog "Expected result: " & sExpectedValue
            qaErrorLog "Obtained result: " & oErrorValue.getText
            fSetErrorValue = 2
        endif
    else
        fSetErrorValue = 0
        qaErrorLog "!! Skipped verification of result"
    endif

    if fSetErrorValue = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function
'
'--------------------------------------------------------------------
'
function fSetErrorIndicatorItem ( iErrorIndicatorNumber as INTEGER ) as INTEGER
         fSetErrorIndicatorItem = 99
' Function to select the 'Error Indicator' item
' Input:
' INTEGER iErrorIndicatorNumber:
' Position in 'Error Indicator' control.
' Posible Values are 1 to 4
' NOTE: Currently the 'Error Indicator' control can only be addressed using 'typekeys'.
' -> No 100% proof verification of success possible!
' Returns error-code:
' 0 := Sucess.
' 1 := Serious problem while selecting the variant control.
'11 := 'Error Indicator' is not visible
'42 := User error, input out of range.
'99 := Unexpected error. 
    '/// Check if 'Error Indicator' is visible
    Kontext "InsertStatisticsDialog"
    if NOT ErrorIndicator.IsVisible then
        qaErrorLog "Error 11: 'Error Indicator' is not visible"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetErrorIndicatorItem = 11
        exit function
    endif
    '/// Lookup valid input value for 'Error Indicator Item'
    if ( iErrorIndicatorNumber < 1 ) OR ( iErrorIndicatorNumber > 4 ) then
         fSetErrorIndicatorItem = 42
         warnlog "USER ERROR: Input doesn't match!"
         exit function
    endif   
    printlog "** Selecting the 'Error Indicator'"
    '/// Select the desired entry number
    Kontext "InsertStatisticsDialog"
    try 
        ErrorIndicator.typeKeys "<HOME>"
        ErrorIndicator.typeKeys "<RIGHT>", ( iErrorIndicatorNumber - 1 )
        fSetErrorIndicatorItem = 0
        printlog ">> Selecting the 'Error Indicator' probably worked"
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Selecting 'Error Indicator' seems to cause a serious problem -> Check this out!"
        fSetErrorIndicatorItem = 1
        exit function
    endcatch
    
    if fSetErrorIndicatorItem = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function
'
'--------------------------------------------------------------------
'
function fSetRegressionCurvesItem ( iRegressionCurveNumber as INTEGER ) as INTEGER
         fSetRegressionCurvesItem = 99
' Function to select the 'Regression Curves' item
' Input:
' INTEGER iRegressionCurveNumber:
' Position in 'Regression Curves' control.
' Posible Values are 1 to 5
' NOTE: Currently the 'Regression Curves' control can only be addressed using 'typekeys'.
' -> No 100% proof verification of success possible!
' Returns error-code:
' 0 := Sucess.
' 1 := Serious problem while selecting the variant control.
'11 := 'Regression Curves' is not visible
'42 := User error, input out of range.
'99 := Unexpected error.
    ' Check if 'Regression Curves' is visible
    Kontext "InsertStatisticsDialog"
    if NOT RegressionCurves.IsVisible then
        qaErrorLog "Error 11: 'Regression Curves' is not visible"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetRegressionCurvesItem = 11
        exit function
    endif
    '/// Lookup valid input value for 'Regression Curves Item'
    if ( iRegressionCurveNumber < 1 ) OR ( iRegressionCurveNumber > 5 ) then
         fSetRegressionCurvesItem = 42
         warnlog "USER ERROR: Input doesn't match!"
         exit function
    endif        
    printlog "** Selecting the 'Regression Curves'"
    '/// Select the desired entry number
    Kontext "InsertStatisticsDialog"
    try 
        RegressionCurves.typeKeys "<HOME>"
        RegressionCurves.typeKeys "<RIGHT>", ( iRegressionCurveNumber - 1 )
        fSetRegressionCurvesItem = 0
        printlog ">> Selecting the 'Regression Curves' probably worked"
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Selecting 'Regression Curves' seems to cause a serious problem -> Check this out!"
        fSetRegressionCurvesItem = 1
        exit function
    endcatch
    
    if fSetRegressionCurvesItem = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function

