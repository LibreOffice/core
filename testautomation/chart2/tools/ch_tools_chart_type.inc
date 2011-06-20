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
'* short description : Tool library for chart type dialog
'*
'**************************************************************************************************
'*
' #1 fInvokeChartTypeDialog
' #1 fCloseChartTypeDialogOK
' #1 fSetChartType
' #1 fSetChartTypeVariant
' #1 fSetChartType3D
' #1 fSetChartLook3D
' #1 fSetChartShape3D
' #1 fSetStackSeries
' #1 fSetSmoothLines
' #1 fInvokeSmoothLineProperties
' #1 fCloseSmoothLinePropertiesOK
'*
'\************************************************************************************************

' ch_tools_chart_type.inc - A library for automation of the Chart Type dialog
' This Include contains a set of functions to modify the controls of the chart type dialog.
' All functions are designed to return error-codes depending on the behaviour of the action applied.
' Return codes:
' Error 0: Success.
' Error 1: The basic action beeing applied caused a serious problem, e.g. a crash.
' Error 2 TO 9: A functional problem occurred.
' Error 11 TO 19: Wrong marginal conditions end up in failure, e.g. control not visible.
'
' NOTE: Following errors can also be used for 'negative' testing.
' Error 42: Wrong input. Probably only of interest during test development.
' Error 99: Unexpected behaviour - Shouldn't normally occur.
' ATTENTION:
' Only Errors 42 and 99 throw 'warnlogs'
' All other errors are silent.
' They only throw QAErrorlogs the give a hint what probably went wrong.
' Expected Errors MUST exclusivly be handled by the calling routine!
'
'-------------------------------------------------------------------------
'
function fInvokeChartTypeDialog() as INTEGER
         fInvokeChartTypeDialog = 99
' Function to invoke the 'Format::Chart Type...' dialog
' No Input
' Returns error-code:
'  0 := Sucess
'  1 := Serious problem trying to invoke the dialog
'  2 := Failure (Dialog not present after invocation)
' 99 := Unexpected error
    
    printlog "** Invoking 'Format::Chart Type' in menu"
    '/// Execute menu item 'Format::Chart type'
    try 
        FormatChartType
    catch
        qaErrorLog "Error 1: Invoking menu item 'Format::Chart Type' failed"
        fInvokeChartTypeDialog = 1
        exit function
    endcatch
    
    '/// Lookup if call was successful
    Kontext "ChartType"
    if ChartType.exists(2) then
        printlog ">> 'Chart Type' dialog is visible now."
        fInvokeChartTypeDialog = 0
    else
        qaErrorLog "Error 2: OOPS, calling the slot 'FormatChartType' didn't cause any problem ..."
        qaErrorLog "..., BUT the dialog doesn't seem to be visible"
        fInvokeChartTypeDialog = 2
    endif

    if fInvokeChartTypeDialog = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif    
end function
'
'--------------------------------------------------------------------
'
function fCloseChartTypeDialogOK() as INTEGER
         fCloseChartTypeDialogOK = 99
' Function to close the 'Chart Type' dialog using OK button
' No Input
' Returns error-code:
'  0 := Sucess
'  1 := Serious problem trying to Close the dialog
'  2 := Failure (Dialog present after applying OK button)
' 15 := Dialog not present before closing
' 99 := Unexpected error
    
    printlog "** Closing 'Chart Type' dialog"
    Kontext "ChartType"
    '/// Check existence of Chart Type(Wizard) dialog
    if ChartType.exists(2) then
        printlog ">> 'Chart Type' dialog is visible as expected."
    else
        '-> Return Error 15 and quit if dialog not found
        qaErrorLog "Error 15: OOPS, 'Chart Type' dialog should be visible ..."
        qaErrorLog "... this is a BUG or a scripting error -> Check this out!"
        fCloseChartTypeDialogOK = 15
        exit function
    endif
    try 
        '/// Click on OK
        ChartType.OK
        printlog "Chart Type dialog has been successfull closed with OK button."
    catch
        qaErrorLog "Error 1: Closing the 'Chart Type' dialog seems to have a serious problem -> Check this out!"
        fCloseChartTypeDialogOK = 1
        exit function
    endcatch
    '/// Check if the Chart Type(Wizard) dialog gone
    if ChartType.Exists(2) then
        '-> Return Error 2 if still present
        qaErrorLog "Error 2: 'Chart Type' dialog should be invisible now ..."
        qaErrorLog "... closing the dialog doesn't seem to work -> Check this out!"
        fCloseChartTypeDialogOK = 2
    else
        printlog ">> Closing the 'Chart Type' dialog seems to work as expected"
        fCloseChartTypeDialogOK = 0
    endif
    
    if fCloseChartTypeDialogOK = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
end function
'
'--------------------------------------------------------------------
'
function fSetChartType ( sChartType as STRING ) as INTEGER
	'Function to choose a chart type in 'Chart Type' dialog
    fSetChartType = 99

	' Input: STRING sChartType (English UI name of chart type to select) 
	' column
	' bar
	' pie
	' area
	' line
	' xy
	' bubble
	' net
	' stock
	' column_line
	' Output (Error-codes)
	' 0 = Sucess
	' 1 = Serious problem trying to select chart type
	' 2 = Failure (wrong chart type selected)
	' 42 = User error, unknown chart type
	' 99 = Unexpected error
 
    dim iEntryInList as INTEGER
    
    printlog "** Selecting the chart type"
    ' Translate UI string to positon in control 'ChooseType'
    select case lCase ( sChartType )
           case "column"      : iEntryInList = 1
           case "bar"         : iEntryInList = 2
           case "pie"         : iEntryInList = 3
           case "area"        : iEntryInList = 4
           case "line"        : iEntryInList = 5
           case "xy"          : iEntryInList = 6
		   case "bubble"      : iEntryInList = 7
           case "net"         : iEntryInList = 8
           case "stock"       : iEntryInList = 9
           case "column_line" : iEntryInList = 10
           case else
                fSetChartType = 42
                warnlog "USER ERROR: Input doesn't match!"
                exit function
    end select
    Kontext "ChartType"
    '///  Select chart type
    try 
        ChooseType.Select ( iEntryInList )
    catch
        qaErrorLog "Error 1: Selecting the chart type seems to cause a serious problem -> Check this out!"
        fSetChartType = 1
        exit function
    endcatch
    '/// Check if entry was selected
    if ChooseType.GetSelText = ChooseType.GetItemText ( iEntryInList ) then
        printlog ">> The desired chart type was selected."
        fSetChartType = 0 
    else
        ' Throw Error 2 if selection fails
        qaErrorLog "Error 2: Selecting the desired chart type seems to fail -> Check this out!"
        fSetChartType = 2
    endif
    
    if fSetChartType = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif  
end function
'
'--------------------------------------------------------------------
'
function fSetChartTypeVariant ( iChartTypeVariant as INTEGER ) as INTEGER
	' Function to select the chart type variant 
	' Input: INTEGER iChartTypeVariant (Position in variant control)
	'  Posible Values are 1,2,3,4 (depending on the chart type)
	'  Output (Error-codes)
	' 0 = Sucess
	' 1 = Serious problem while selecting the variant control
	' 42 = User error, input out of range
	' 99 = Unexpected error   
    
    fSetChartTypeVariant = 99
    
    dim iMaximumVariant as INTEGER
    dim iChartTpye as INTEGER
    
    Kontext "ChartType"
    ' Lookup valid input range for variant depending on chart type
    iChartTpye = ChooseType.getSelIndex
    
    select case iChartTpye
        case 1, 2       :
                            if Look3D.isChecked then
                                iMaximumVariant = 4
                            else
                                iMaximumVariant = 3
                            endif
		case 7          :   iMaximumVariant = 1							
        case 4       :   iMaximumVariant = 3
        case 3, 5, 6, 8 ,9 :   iMaximumVariant = 4
        case 10         :   iMaximumVariant = 2
    end select

    ' Check if input matches this range
    if (iChartTypeVariant < 1) OR (iChartTypeVariant > iMaximumVariant) then
        fSetChartTypeVariant = 42
        warnlog "USER ERROR: Input doesn't match!"
        exit function
    endif    

    printlog " Selecting the chart type variant"
    '/// Select the chart variant
    Kontext "ChartType"
    try 
        variant.typeKeys "<HOME>"
        variant.typeKeys "<RIGHT>", (iChartTypeVariant - 1)
        fSetChartTypeVariant = 0
        printlog "Selecting the chart variant probably worked"
    catch
        ' Throw error 1 and quit on serious problem
        QAErrorLog "Error 1: Selecting variant seems to cause a serious problem -> Check this out!"
        fSetChartTypeVariant = 1
        exit function
    endcatch
    
    if fSetChartTypeVariant = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function
'
'--------------------------------------------------------------------
'
function fSetChartType3D ( bEnable3D as BOOLEAN ) as INTEGER
         fSetChartType3D = 99
' Function to select the 3D option
' Input:
' BOOLEAN bEnable3D:
' Enable (TRUE) or Disable (FALSE) 3D chart.
' Returns error-code:
' 0  = Sucess
' 1  = Serious problem while (de)selecting 3D check-box
' 11 = 3D control not visible
' 12 = 3D control not enabled
' 99 = Unexpected error.   
    printlog "Selecting the chart 3D feature"
    'Check visibility of 3D check-box
    Kontext "ChartType"
    if NOT Look3D.isVisible then
        QAErrorLog "Error 11: 3D check box has to be visible to check it."
        fSetChartType3D = 11
        exit function
    endif
    '/// Check if 3D check-box is enabled
    if NOT Look3D.isEnabled then
        qaErrorLog "Error 12: 3D check box has to be enabled to check it."
        fSetChartType3D = 12
        exit function
    endif
    '/// (Un)Check 3D check-box
    try 
        if bEnable3D then
            Look3D.Check
        else
            Look3D.unCheck
        endif
        fSetChartType3D = 0
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: (Un)Checking the 3D check box seems to cause a serious issue -> Check this out!"
        fSetChartType3D = 1
        exit function
    endcatch
    
    if fSetChartType3D = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function
'
'--------------------------------------------------------------------
'
function fSetChartLook3D ( sSetLook as STRING ) as INTEGER
    fSetChartLook3D = 99
' Function to select the 3D look///
' Input: STRING sSetLook (English UI name of 3D look) 
' simple
' realistic
'  Returns error-code:
' 0  = Sucess
' 1  = Serious problem while selecting 3D look
' 12 = 3D look not enabled
' 42 = User error, input doesn't match
' 99 = Unexpected error
    dim iEntryInList as INTEGER
    ' Translate SIMPLE or REALISTIC to item no. in list-box
    select case lcase(sSetLook)
           case "simple"    : iEntryInList = 1
               				  printlog "Selecting the chart 3D look 'simple' (default)"
           case "realistic" : iEntryInList = 2
           					  printlog "Selecting the chart 3D look 'realistic'"
           case else
                fSetChartLook3D = 42
                warnlog "USER ERROR: Input doesn't match!"
                exit function
    end select    
    '/// Check if 3D look list-box is enabled
    if NOT Scheme3D.isEnabled then
        QAErrorLog "Error 12: 3D check box has to be checked to set 3D look ..."
        QAErrorLog "... BUG or Script problem -> Check this out!"
        fSetChartLook3D = 12
        exit function
    endif
    '/// Select desired item in list-box
    try 
        Scheme3D.Select ( iEntryInList )
        fSetChartLook3D = 0
        printlog "Selecting the chart 3D scheme seems to work."
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Selecting the 3D look seems to cause a serious issue -> Check this out!"
        fSetChartLook3D = 1
        exit function
    endcatch

    if fSetChartLook3D = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif  
end function
'
'--------------------------------------------------------------------
'
function fSetChartShape3D ( sSetShape as STRING ) as INTEGER
    fSetChartShape3D = 99
' Function to select the 3D shape///
' Input: STRING sSetShape (English UI name of shape)
' box
' cylinder
' cone
' pyramid
'  Returns (Error-codes):
' 0  = Sucess
' 1  = Serious problem while selecting 3D shape
' 11 = 3D look not visible
' 12 = 3D look not enabled
' 42 = User error, input doesn't match
' 99 = Unexpected error

    dim iEntryInList as INTEGER
    ' Translate shape UI name to item no. in list-box
    select case lcase ( sSetShape )
        case "box"      : iEntryInList = 1
        case "cylinder" : iEntryInList = 2
        case "cone"     : iEntryInList = 3
        case "pyramid"  : iEntryInList = 4
        case else           
                            fSetChartShape3D = 42
                            warnlog "USER ERROR: Input doesn't match!"
                            exit function
    end select
    printlog "Selecting the chart 3D shape"
    '/// Check if shape list-box is visible
    if NOT BarColumnShape.isVisible then
        QAErrorLog "Error 11: List box has to be visible to select 3D shape ..."
        fSetChartShape3D = 11
        exit function
    endif
    '/// Check if shape list-box is enabled
    if NOT BarColumnShape.isEnabled then
        QAErrorLog "Error 12: List box has to be enabled to select 3D shape ..."
        fSetChartShape3D = 12
        exit function
    endif
    '/// Select desired entry in list-box
    try 
        BarColumnShape.Select ( iEntryInList )
        fSetChartShape3D = 0
        printlog "Selecting the chart 3D shape seems to work."
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Selecting the 3D shape seems to cause a serious issue -> Check this out!"
        fSetChartShape3D = 1
        exit function
    endcatch

    if fSetChartShape3D = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function
'
'--------------------------------------------------------------------
'
function fSetStackSeries ( sStackMode as STRING ) as INTEGER
    fSetStackSeries = 99
	' Function to adjust data stacking feature///
	' Input:
	' STRING sStackMode:
	' Select stack mode using three controls
	' <i>disable - Uncheck 'Stack Series' check-box
	' <i>ontop - Check 'Stack Series' check-box and check 'On Top' radio button
	' <i>percent - Check 'Stack Series' check-box and check 'Percent' radio button
	' Returns error-code:
	'  0 := Sucess.
	'  1 := Serious problem while selecting stack mode.
	' 11 := 'Stack Series' not visible
	' 42 := User error, input doesn't match.
	' 99 := Unexpected error.
    
    printlog ""
    printlog "** Selecting the Stack Series Option"
    '///  Check visibility of 'Stack Series'
    if NOT StackSeries.isVisible then
        qaErrorLog "Error 11: 'Stack Series' check-box has to be visible to set this feature"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetStackSeries = 11
        exit function
    endif
    '/// Select desired control combination
    try 
        select case lcase ( sStackMode )
               case "disable"
                    StackSeries.Uncheck
               case "ontop"
                    StackSeries.Check
                    OnTop.Check
               case "percent"
                    StackSeries.Check
                    Percent.Check
               case else
                    fSetStackSeries = 42
                    warnlog "USER ERROR: Input doesn't match!"
                    exit function
        end select
        fSetStackSeries = 0
        printlog ">> Selecting the 'Stack Series' mode seems to work."
    catch
        ' Throw error 1 and quit on serious problem///
        qaErrorLog "Error 1: Selecting the 'Stack Series' mode seems to cause a serious issue -> Check this out!"
        fSetStackSeries = 1
        exit function
    endcatch

    if fSetStackSeries = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
' 
end function
'
'--------------------------------------------------------------------
'
function fSetSmoothLines ( bEnableSmoothLines as BOOLEAN ) as INTEGER
    fSetSmoothLines = 99
	' Function to adjust smooth lines feature
	' Input:
	' BOOLEAN bEnableSmoothLines:
	' Enable (TRUE), Disable (FALSE)'Smooth Lines'
	' Returns error-code:
	'  0 := Sucess.
	'  1 := Serious problem while Enabling/Disable 'Smooth Lines.
	' 11 := 'Smooth lines' not visible
	' 42 := User error, input doesn't match.
	' 99 := Unexpected error.
     
    printlog ""
    printlog "** Selecting the Smooth Lines Option"
    '/// Check if 'Smooth Lines' check-box is visible
    if NOT SmoothLines.isVisible then
        qaErrorLog "Error 11: 'Smooth Lines' check-box has to be visible to set this feature"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetSmoothLines = 11
        exit function
    endif
    '/// (Un)Check 'Smooth Lines' check-box
    try 
        if bEnableSmoothLines then
            SmoothLines.Check
        else
            SmoothLines.UnCheck
        endif
        fSetSmoothLines = 0
        printlog ">> (Un)Selecting the 'Smooth Lines' seems to work."
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: (Un)Selecting 'Smooth Lines' seems to cause a serious issue -> Check this out!"
        fSetSmoothLines = 1
        exit function
    endcatch

    if fSetSmoothLines = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif    
end function
'
'--------------------------------------------------------------------
'
function fInvokeSmoothLineProperties() as INTEGER
    fInvokeSmoothLineProperties = 99
' Click 'Smooth Lines' button to invoke 'Smooth Lines Properties' dialog///
' No Input
' Returns error-code:
'  0 := Sucess
'  1 := Serious problem trying to invoke the dialog
'  2 := Failure (Dialog not present after invocation)
' 11 := 'Smooth Lines' button isn't visible
' 99 := Unexpected error
    
    printlog ""
    printlog "** Set the Smooth Line Properties"
    '/// Check if 'Smooth Lines' button is visible
    if NOT SmoothLineProperties.isVisible then
        qaErrorLog "Error 11: 'Smooth Lines Properties' button has to be visible to open the dialog ..."
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fInvokeSmoothLineProperties = 11
        exit function
    endif
    '/// Click 'Smooth Lines' button
    try 
        SmoothLineProperties.Click
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Opening 'Smooth Lines Properties' seems to cause a serious issue -> Check this out!"
        fInvokeSmoothLineProperties = 1
        exit function
    endcatch
    '/// Check if 'Smooth Lines Properties' dialog exists
    Kontext "SmoothLineProperties"
    if SmoothLineProperties.exists(1) then
        printlog ">> OK, found 'Smooth Lines Properties' dialog"
        fInvokeSmoothLineProperties = 0
    else
        ' Throw error 2 and quit if opening dialog failed
        qaErrorLog "Error 2: Click 'Smooth Lines Properties' button seems to have no effect -> Check this out!"
        fInvokeSmoothLineProperties = 2
        exit function
    endif

    if fInvokeSmoothLineProperties = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif    
end function
'
'--------------------------------------------------------------------
'
function fCloseSmoothLinePropertiesOK() as INTEGER
    fCloseSmoothLinePropertiesOK = 99
' Function to close the 'Smooth Lines Properties' dialog using OK button///
' No Input
' Returns error-code:
'  0 := Sucess
'  1 := Serious problem trying to Close the dialog
'  2 := Failure (Dialog present after applying OK button)
'  3 := 'Smooth Lines' check-box isn't checked after closing the dialog
' 99 := Unexpected error

    printlog "** Closing the 'Smooth Line Properties' dialog"
    '/// Closing the 'Smooth Line Properties' dialog using OK button
    Kontext "SmoothLineProperties"
    try 
        SmoothLineProperties.OK
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Closing the 'Smooth Line Properties' dialog seems to cause a serious issue -> Check this out!"
        fCloseSmoothLinePropertiesOK = 1
        exit function
    endcatch
    '/// Check if 'Smooth Lines Properties' dialog does not exist
    Kontext "SmoothLineProperties"
    if SmoothLineProperties.exists(1) then
        ' Throw error 2 and quit if closing dialog failed
        qaErrorLog "Error 2: Click 'Smooth Lines Properties' OK button seems to have no effect -> Check this out!"
        fCloseSmoothLinePropertiesOK = 2
        exit function
    else
        printlog ">> OK, closing the 'Smooth Line Properties' dialog seems to work"
    endif
    '/// Check if 'Smooth Lines' check-box is checked
    Kontext "ChartType"
    if SmoothLines.IsChecked then
        printlog ">> OK, after adjusting 'Smooth Line Properties' dialog 'Smooth Lines' check-box should always be visible!"
        fCloseSmoothLinePropertiesOK = 0
    else
        ' Throw error 3 'Smooth Lines' check-box is NOT checked
        qaErrorLog "Error 3: After adjusting 'Smooth Line Properties' dialog the 'Smooth Lines' check-box should have been visible! -> Check this out!"
        fCloseSmoothLinePropertiesOK = 3
    endif

    if fCloseSmoothLinePropertiesOK = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif    
end function

