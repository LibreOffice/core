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
'* short description : Tool library for Data Labels dialog
'*
'**************************************************************************************************
'*
' #1 fInvokeDataLabelsDialog
' #1 fCloseDataLabelsDialogOK
' #1 fSetShowValue
' #1 fSetValueType
' #1 fSetShowLabelText
' #1 fSetShowLegendIconWithLabel
'*
'\************************************************************************************************

'ch_tools_data_labels.inc - Library for automation of the 'Insert::Data Labels...' dialog
'This Include contains a set of functions to modify the controls of the Data Labels dialog.
'All functions are designed to return error-codes depending on the behaviour of the action applied.
'Return codes:
'Error 0: Success.
'Error 1: The basic action beeing applied caused a serious problem, e.g. a crash.
'Error 2 TO 9: A functional problem occurred.
'Error 11 TO 19: Wrong marginal conditions end up in Failure, e.g. control not visible.
'NOTE: This errors can also be used for 'negative' testing.
'Error 42: Wrong input. Probably only of interest during test development .
'Error 99: Unexpected behaviour - Shouldn't normally occur.
'ATTENTION:
'Only Errors 42 and 99 throw 'Warnlogs'.
'All other errors are silent!
'They only throw QAErrorlogs the give a hint what probably went wrong.
'Expected Errors MUST exclusivly be handled by the calling routine!
'
'--------------------------------------------------------------------
'
function fInvokeDataLabelsDialog() as INTEGER
         fInvokeDataLabelsDialog = 99
'///<u><b>Function to invoke the 'Insert::Data Labels...' dialog</b></u>///
'///No Input
'///Returns error-code:
'///+ 0 := Sucess
'///+ 1 := Serious problem trying to invoke the dialog
'///+ 2 := Failure (Dialog not present after invocation)
'///+99 := Unexpected error
    
'///<u><b>Step-by-Step</b></u>///
'///<ul>
    printlog ""
    printlog "** Invoking 'Insert::Data Labels...' in menu"
    '///+<li>Execute menu item 'Insert::Data Labels...'</li>///
    try 
        InsertDataLabels
    catch
        qaErrorLog "Error 1: Invoking menu item 'Insert::Data Labels...' failed"
        fInvokeDataLabelsDialog = 1
        exit function
    endcatch
    
    '///+<li>Lookup if call was successful</li>///
    Kontext "InsertLabelsDialog"
    if InsertLabelsDialog.exists(2) then
        printlog ">> Data Labels dialog is visible now."
        fInvokeDataLabelsDialog = 0
    else
        qaErrorLog "Error 2: OOPS, calling the slot 'InsertDataLabels' didn't cause any problem ..."
        qaErrorLog "..., BUT the dialog doesn't seem to be visible"
        fInvokeDataLabelsDialog = 2
    endif

    if fInvokeDataLabelsDialog = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
'///</ul>    
end function
'
'--------------------------------------------------------------------
'
function fCloseDataLabelsDialogOK() as INTEGER
         fCloseDataLabelsDialogOK = 99
'///<u><b>Function to close the Data Labels dialog using OK button</b></u>///
'///No Input
'///Returns error-code:
'///+ 0 := Sucess
'///+ 1 := Serious problem trying to Close the dialog
'///+ 2 := Failure (Dialog present after applying OK button)
'///+15 := Dialog not present before closing
'///+99 := Unexpected error
    
'///<u><b>Step-by-Step</b></u>///
'///<ul>
    printlog " "
    printlog "** Closing Data Labels dialog"
    Kontext "InsertLabelsDialog"
    '///+<li>Check existence of Data Labels dialog</li>///
    if InsertLabelsDialog.exists(2) then
        printlog ">> Data Labels dialog is visible as expected."
    else
        '///+<li>-> Return Error 15 and quit if dialog not found</li>///
        qaErrorLog "Error 15: OOPS, Data Labels dialog should be visible ..."
        qaErrorLog "... this is a BUG or a scripting error -> Check this out!"
        fCloseDataLabelsDialogOK = 15
        exit function
    endif
    '///+<li>Click OK button in Data Labels dialog</li>///
    try 
        InsertLabelsDialog.OK
    catch
        qaErrorLog "Error 1: Closing the Data Labels dialog seems to have a serious problem -> Check this out!"
        fCloseDataLabelsDialogOK = 1
        exit function
    endcatch
    '///+<li>Check existence of Data Labels dialog</li>///
    Kontext "InsertLabelsDialog"
    if InsertLabelsDialog.exists(2) then
        '///+<li>-> Return Error 2 if still present</li>///
        qaErrorLog "Error 2: Data Labels dialog should be invisible now ..."
        qaErrorLog "... closing the dialog doesn't seem to work -> Check this out!"
        fCloseDataLabelsDialogOK = 2
    else
        printlog ">> Closing the Data Labels dialog seems to work as expected"
        fCloseDataLabelsDialogOK = 0
    endif
    
    if fCloseDataLabelsDialogOK = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
'///</ul>
end function
'
'--------------------------------------------------------------------
'
function fSetShowValue ( bShowValueCheck as BOOLEAN ) as INTEGER
         fSetShowValue = 99
'///<u><b>Function to check 'Show Value'</b></u>///
'///<b>BOOLEAN bShowValueCheck</b>
'///+TRUE := Check 'Show Value' - FALSE := Uncheck 'Show Value'
'///<b>Returns error-code:</b>
'///+ 0 := Sucess
'///+ 1 := Serious problem trying to check 'Show Value'
'///+ 2 := 'Show Value' was not set
'///+12 := Check-box for 'Show Value' is not enabled
'///+99 := Unexpected error
    
'///<u><b>Step-by-Step</b></u>///
'///<ul>
    printlog ""
    printlog "** Setting 'Show Value'"
    '///+<li>Check if 'Show Value' is enabled</li>///
    if NOT ShowValue.IsEnabled then
        qaErrorLog "Error 12: Check-box for 'Show Value' is not enabled"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetShowValue = 12
        exit function
    endif
    '///+<li>Try to (Un)Check 'Show Value'</li>///
    Kontext "InsertLabelsDialog"
    try 
        if bShowValueCheck then
            ShowValue.Check
        else
            ShowValue.UnCheck
        endif
    catch
        '///+<li>Throw error 1 and quit on serious problem</li>///
        qaErrorLog "Error 1: Check 'Show Value' seems to cause a serious problem -> Check this out!"
        fSetShowValue = 1
        exit function
    endcatch
    '///+<li>Verify (against input) if 'Show Value' was checked</li>///
    if ShowValue.IsChecked = bShowValueCheck then
        fSetShowValue = 0
        printlog ">> Check 'Show Value' seems to work"
    else
        qaErrorLog "Error 2: 'Show Value' was not checked -> Check this out!"
        fSetShowValue = 2
    endif

    if fSetShowValue = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
'///</ul>   
end function
'
'--------------------------------------------------------------------
'
function fSetValueType ( oValueType as OBJECT ) as INTEGER
         fSetValueType = 99
'///<u><b>Function to select Value Type</b></u>///
'///Input:
'///<b>OBJECT oValueType</b>
'///+Declaration name of Radio-Button: <i>AsNumber</i> and <i>AsPercentage</i> are valid values
'///<b>Returns error-code:</b>
'///+ 0 := Sucess
'///+ 1 := Serious problem trying to check Legend position radio-button
'///+ 2 := Value type radio-button was not set
'///+12 := Radio-Button for desired Value type is not enabled
'///+99 := Unexpected error
    
'///<u><b>Step-by-Step</b></u>///
'///<ul>
    printlog ""
    printlog "** Setting Value type"
    '///+<li>Check if Value type radio-button is enabled</li>///
    Kontext "InsertLabelsDialog"
    if NOT oValueType.IsEnabled then
        qaErrorLog "Error 12: Radio-Button for desired Value type is not enabled"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetValueType = 12
        exit function
    endif
    '///+<li>Try to (Un)Check desired radio-button</li>///
    Kontext "InsertLabelsDialog"
    try 
        oValueType.Check
    catch
        '///+<li>Throw error 1 and quit on serious problem</li>///
        qaErrorLog "Error 1: Check Value type radio-button seems to cause a serious problem -> Check this out!"
        fSetValueType = 1
        exit function
    endcatch
    '///+<li>Verify (against input) if Value type radio-button was checked</li>///
    if oValueType.IsChecked then
        fSetValueType = 0
        printlog ">> Setting Value type radio-button seems to work"
    else
        qaErrorLog "Error 2: Desired Value type radio-button was not checked -> Check this out!"
        fSetValueType = 2
    endif

    if fSetValueType = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
'///</ul>   
end function
'
'--------------------------------------------------------------------
'
function fSetShowLabelText ( bShowLabelTextCheck as BOOLEAN ) as INTEGER
         fSetShowLabelText = 99
'///<u><b>Function to check 'Show Label Text'</b></u>///
'///<b>BOOLEAN bShowLabelTextCheck</b>
'///+TRUE := Check 'Show Label Text' - FALSE := Uncheck 'Show Label Text'
'///<b>Returns error-code:</b>
'///+ 0 := Sucess
'///+ 1 := Serious problem trying to check 'Show Label Text'
'///+ 2 := 'Show Label Text' was not set
'///+12 := Check-box for 'Show Label Text' is not enabled
'///+99 := Unexpected error
    
'///<u><b>Step-by-Step</b></u>///
'///<ul>
    printlog ""
    printlog "** Setting 'Show Label Text'"
    '///+<li>Check if 'Show Label Text' is enabled</li>///
    if NOT ShowLabelText.IsEnabled then
        qaErrorLog "Error 12: Check-box for 'Show Label Text' is not enabled"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetShowLabelText = 12
        exit function
    endif
    '///+<li>Try to (Un)Check 'Show Label Text'</li>///
    Kontext "InsertLabelsDialog"
    try 
        if bShowLabelTextCheck then
            ShowLabelText.Check
        else
            ShowLabelText.UnCheck
        endif
    catch
        '///+<li>Throw error 1 and quit on serious problem</li>///
        qaErrorLog "Error 1: Check 'Show Label Text' seems to cause a serious problem -> Check this out!"
        fSetShowLabelText = 1
        exit function
    endcatch
    '///+<li>Verify (against input) if 'Show Label Text' was checked</li>///
    if ShowLabelText.IsChecked = bShowlabelTextCheck then
        fSetShowLabelText = 0
        printlog ">> Check 'Show Label Text' seems to work"
    else
        qaErrorLog "Error 2: 'Show Label Text' was not checked -> Check this out!"
        fSetShowLabelText = 2
    endif

    if fSetShowLabelText = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
'///</ul>   
end function
'
'--------------------------------------------------------------------
'
function fSetShowLegendIconWithLabel ( bShowLegendIconCheck as BOOLEAN ) as INTEGER
         fSetShowLegendIconWithLabel = 99
'///<u><b>Function to check 'Show Legend Icon With Label'</b></u>///
'///<b>BOOLEAN bShowLegendIconCheck</b>
'///+TRUE := Check 'Show Legend Icon With Label' - FALSE := Uncheck 'Show Legend Icon With Label'
'///<b>Returns error-code:</b>
'///+ 0 := Sucess
'///+ 1 := Serious problem trying to check 'Show Legend Icon With Label'
'///+ 2 := 'Show Legend Icon With Label' was not set
'///+12 := Check-box for 'Show Legend Icon With Label' is not enabled
'///+99 := Unexpected error
    
'///<u><b>Step-by-Step</b></u>///
'///<ul>
    printlog ""
    printlog "** Setting 'Show Legend Icon With Label'"
    '///+<li>Check if 'Show Legend Icon With Label' is enabled</li>///
    if NOT ShowLegendIconWithLabel.IsEnabled then
        qaErrorLog "Error 12: Check-box for 'Show Legend Icon With Label' is not enabled"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetShowLegendIconWithLabel = 12
        exit function
    endif
    '///+<li>Try to (Un)Check 'Show Legend Icon With Label'</li>///
    Kontext "InsertLabelsDialog"
    try 
        if bShowLegendIconCheck then
            ShowLegendIconWithLabel.Check
        else
            ShowLegendIconWithLabel.UnCheck
        endif
    catch
        '///+<li>Throw error 1 and quit on serious problem</li>///
        qaErrorLog "Error 1: Check 'Show Legend Icon With Label' seems to cause a serious problem -> Check this out!"
        fSetShowLegendIconWithLabel = 1
        exit function
    endcatch
    '///+<li>Verify (against input) if 'Show Legend Icon With Label' was checked</li>///
    if ShowLegendIconWithLabel.IsChecked = bShowLegendIconCheck then
        fSetShowLegendIconWithLabel = 0
        printlog ">> Check 'Show Legend Icon With Label' seems to work"
    else
        qaErrorLog "Error 2: 'Show Legend Icon With Label' was not checked -> Check this out!"
        fSetShowLegendIconWithLabel = 2
    endif

    if fSetShowLegendIconWithLabel = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
'///</ul>   
end function

