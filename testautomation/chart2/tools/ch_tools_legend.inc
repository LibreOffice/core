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
'* short description : Tool library for Legend dialog
'*
'**************************************************************************************************
'*
' #1 fInvokeLegendDialog
' #1 fCloseLegendDialogOK
' #1 fSetDisplayLegend
' #1 fSetLegendPosition
'*
'\************************************************************************************************

' ch_tools_legend.inc - A library for automation of the 'Insert/Legend...' dialog
' This Include contains a set of functions to modify the controls of the Legend dialog.
' All functions are designed to return error-codes depending on the behaviour of the action applied.
' Return codes:
' Error 0: Success.
' Error 1: The basic action beeing applied caused a serious problem, e.g. a crash.
' Error 2 TO 9: A functional problem occurred.
' Error 11 TO 19: Wrong marginal conditions end up in failture, e.g. control not visible.
' NOTE: This errors can also be used for 'negative' testing.
' Error 42: Wrong input. Probably only of interest during test development .
' Error 99: Unexpected behaviour - Shouldn't normally occur.
' ATTENTION:
' Only Errors 42 and 99 throw 'warnlogs'.
' All other errors are silent.
' They only throw QAErrorlogs the give a hint what probably went wrong.
' Expected Errors MUST exclusivly be handled by the calling routine.
'
'--------------------------------------------------------------------
'
function fInvokeLegendDialog() as INTEGER
         fInvokeLegendDialog = 99
' Function to invoke the 'Insert/Legend...' dialog
' Return (Error codes):
' 0 = Sucess
' 1 = Serious problem trying to invoke the dialog
' 2 = Failure (Dialog not present after invocation)
' 99 = Unexpected error
    printlog "Invoking 'Insert/Legend...' in menu"
    '/// Execute menu item 'Insert/Legend...'
    try 
        InsertLegend
    catch
        qaErrorLog "Error 1: Invoking menu item 'Insert/Legend...' failed"
        fInvokeLegendDialog = 1
        exit function
    endcatch
    
    '/// Lookup if call was successful
    Kontext "InsertLegendDialog"
    if InsertLegendDialog.exists(2) then
        printlog ">> Legend dialog is visible now."
        fInvokeLegendDialog = 0
    else
        QAErrorLog "Error 2: The slot 'InsertLegend' has been executed but the dialog is not visible"
        fInvokeLegendDialog = 2
    endif

    if fInvokeLegendDialog = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif    
end function
'
'--------------------------------------------------------------------
'
function fCloseLegendDialogOK() as INTEGER
         fCloseLegendDialogOK = 99
' Function to close the Legend dialog using OK button
' No Input
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to Close the dialog
' 2 := Failure (Dialog present after applying OK button)
'15 := Dialog not present before closing
'99 := Unexpected error    
    printlog "** Closing Legend dialog"
    Kontext "InsertLegendDialog"
    '/// Check existence of Legend dialog
    if InsertLegendDialog.exists(2) then
        printlog ">> Legend dialog is visible as expected."
    else
        ' Return Error 15 and quit if dialog not found
        qaErrorLog "Error 15: OOPS, Legend dialog should be visible ..."
        qaErrorLog "... this is a BUG or a scripting error -> Check this out!"
        fCloseLegendDialogOK = 15
        exit function
    endif
    '/// Click OK button in Legend dialog
    try 
        InsertLegendDialog.OK
    catch
        qaErrorLog "Error 1: Closing the Legend dialog seems to have a serious problem -> Check this out!"
        fCloseLegendDialogOK = 1
        exit function
    endcatch
    '///+ Check that the Legend dialog is not available any longer
    Kontext "InsertLegendDialog"
    if InsertLegendDialog.exists(2) then
        ' Return Error 2 if still present
        qaErrorLog "Error 2: Legend dialog should be invisible now ..."
        qaErrorLog "... closing the dialog doesn't seem to work -> Check this out!"
        fCloseLegendDialogOK = 2
    else
        printlog ">> Closing the Legend dialog seems to work as expected"
        fCloseLegendDialogOK = 0
    endif
    
    if fCloseLegendDialogOK = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
end function
'
'--------------------------------------------------------------------
'
function fSetDisplayLegend ( bDisplayLegend as BOOLEAN ) as INTEGER
         fSetDisplayLegend = 99
' Function to [en|dis]able visibility of chart legend
' Input:
' BOOLEAN bDisplayLegend
' TRUE := Check 'Dislay (legend)' 
' FALSE := Uncheck 'Dislay (legend)'
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to check grid
' 2 := 'Dislay (legend)' was not set
'11 := 'Dislay (legend)' Check-box is not visible
'99 := Unexpected error    
    printlog "** Setting Display Legend"
    '/// Check if 'Dislay (legend)' check-box is visible
    Kontext "InsertLegendDialog"
    if NOT Display.IsVisible then
        qaErrorLog "Error 11: Check-box for 'Dislay (legend)' is not visible"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetDisplayLegend = 11
        exit function
    endif
    '/// Try to (Un)Check 'Dislay (legend)'
    Kontext "InsertLegendDialog"
    try 
        if bDisplayLegend then
            Display.Check
        else
            Display.UnCheck
        endif
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Check 'Dislay (legend)' seems to cause a serious problem -> Check this out!"
        fSetDisplayLegend = 1
        exit function
    endcatch
    '/// Verify (against input) if check-box was checked
    if Display.IsChecked = bDisplayLegend then
        fSetDisplayLegend = 0
        printlog ">> Setting 'Dislay (legend)' seems to work"
    else
        qaErrorLog "Error 2: 'Dislay (legend)' was not checked -> Check this out!"
        fSetDisplayLegend = 2
    endif

    if fSetDisplayLegend = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function
'
'--------------------------------------------------------------------
'
function fSetLegendPosition ( oPosition as OBJECT ) as INTEGER
         fSetLegendPosition = 99
' Function to select Legend position
' Input:
' OBJECT oPosition
' Declaration name of Radio-Button: Position[Left|Right|Top|Bottom] are valid values
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to check Legend position radio-button
' 2 := Legend position radio-button was not set
'12 := Radio-Button for desired position is not enabled
'42 := User error, input doesn't match.
'99 := Unexpected error
    printlog "** Setting Legend position"
    '/// Check if Legend position radio-button is enabled
    Kontext "InsertLegendDialog"
    if NOT oPosition.IsEnabled then
        qaErrorLog "Error 12: Radio-Button for desired position is not enabled"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetLegendPosition = 12
        exit function
    endif
    '/// Try to (Un)Check desired radio-button
    Kontext "InsertLegendDialog"
    try 
        oPosition.Check
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Check Legend position radio-button seems to cause a serious problem -> Check this out!"
        fSetLegendPosition = 1
        exit function
    endcatch
    '/// Verify (against input) if Legend position radio-button was checked
    if oPosition.IsChecked then
        fSetLegendPosition = 0
        printlog ">> Setting Legend position radio-button seems to work"
    else
        qaErrorLog "Error 2: Desires Legend position radio-button was not checked -> Check this out!"
        fSetLegendPosition = 2
    endif

    if fSetLegendPosition = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function

