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
'* owner : helge.delfs@oracle.com
'*
'* short description : Tool library for setting controls and verifying the functionality
'*
'**************************************************************************************************
'*
' #0 fSetListBoxByItem              'Function to select item in list box
' #0 fSetListBoxByString            'Function to select item by string in list box
' #0 fSetSpinFieldByString          'Function to set a string in a spin field control
' #0 fSetSpinFieldByButton          'Function to set a spin field control by using 'more' or 'less' button
' #0 fSetSpinFieldByLimit           'Function to set a spin field control to minimum or maximum value
' #0 fSetComboBoxByItem             'Function to select item in combo box
' #0 fSetComboBoxByString           'Function to select item in combo box
' #0 fSetComboBoxByText             'Function to Edit Field part of a combo box
' #0 fSetCheckBox                   'Function to (un)check a check box
' #0 fSetRadioButton                'Function to check a radio button
' #0 fSetTextBox                    'Function to set a textbox
'*
'\************************************************************************************************

'///<u><b>Library for abtracted handling of several control types</b></u>
'///+Note: This functions are designed to be called by wrapping functions
'///+All functions are designed to return error codes depending on the behaviour of the action applied.
'///<b>Return codes:</b>
'///+<ul><li>Error  0: Success</li>
'///+<li>Error  1: The basic action beeing applied caused a serious problem, e.g. a crash</li>
'///+<li>Error  2 TO  9: A functional problem occurred.</li>
'///+<li>Error 11 TO 19: Wrong marginal conditions end up in Failure, e.g. control not visible</li></ul>
'/// NOTE: This errors can also be used for 'negative' testing.
'///+<ul><li>Error 42: Wrong input. Probably only of interest during test development</li>
'///+<li>Error 99: Unexpected behaviour - Shouldn't normally occur</li></ul>
'///<b>ATTENTION:</b>
'///+<ul><li>Only Errors 42 and 99 throw 'warnlogs'</li>
'///+<li>All other errors are silent!</li>
'///+<li>They only throw QAErrorlogs the give a hint what probably went wrong.</li>
'///+<li>Expected Errors MUST exclusivly be handled by the calling routine!</li></ul>
'///+<p><font size=-1><i>The idea and the first implementation of this library has been made by Peter Junge (pj at openoffice.org)</i></font></p> 
'
'
function fSetTextBox ( oThisTextBox as OBJECT , sThisText as STRING ) as INTEGER
         fSetTextBox = 99
'Function to set a text box
'Input: 
'+OBJECT oThisTextBox (text box name in declaration)
'+STRING sThisText (text to be set in text box control)
' Return (Error-codes):
'+0  = Sucess
'+1  = Serious problem trying to set value
'+2  = Value was not set
'+11 = Text box is not visible
'+12 = Text box is not enabled
'+99 = Unexpected error

    dim iIndex as INTEGER
    dim iListLength as INTEGER
    dim sResultInUI as STRING
    
    sResultInUI = ""

    printlog "Setting text in Text Box Edit field"
    ' Check if desired Text box is visible
    if NOT oThisTextBox.IsVisible then
        QAErrorLog "Error 11: Text box is not visible"
        fSetTextBox = 11
        exit function
    endif
    ' Check if desired Text box is enabled
    if NOT oThisTextBox.IsEnabled then
        QAErrorLog "Error 12: Text box is not enabled"
        fSetTextBox = 12
        exit function
    endif
    ' Try to set text on Text Box
    try 
        oThisTextBox.setText sThisText
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Set text on Text box seems to cause a serious problem."
        fSetTextBox = 1
        exit function
    endcatch
    'Verify (against input) if text was set correctly    
    sResultInUI = oThisTextBox.GetText
    if sResultInUI = sThisText then
        fSetTextBox = 0
        printlog "Set '" & sThisText & "' in 'text box' control PASSED."
    else
        QAErrorLog "Error 2: Setting '" & sThisText & "' in 'text box' control failed!"
        fSetTextBox = 2
    endif

    if fSetTextBox = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function
'
'--------------------------------------------------------------------
'
function fSetListBoxByItem ( oThisListBox as OBJECT , iThisValue as INTEGER ) as INTEGER
         fSetListBoxByItem = 99
'Function to select item in list box
'Input: 
'+OBJECT oThisListBox (list box name in declaration)
'+INTEGER iThisValue (item number to be selected in list box control)
'Return (Error-codes):
'+0  = Sucess
'+ 1 = Serious problem trying to set value
'+ 2 = Value was not set
'+11 = List box is not visible
'+12 = List box is not enabled
'+13 = Item number to be selected out of list range
'+99 = Unexpected error

    dim iListLength as INTEGER
    dim iResultFromUI as INTEGER
    
    iResultFromUI = ""
    
    printlog "Setting Item in list box"
    'Check if desired list box is visible
    if NOT oThisListBox.IsVisible then
        QAErrorLog "Error 11: List box is not visible"
        fSetListBoxByItem = 11
        exit function
    endif
    'Check if desired list box is enabled
    if NOT oThisListBox.IsEnabled then
        QAErrorLog "Error 12: List box is not enabled"
        fSetListBoxByItem = 12
        exit function
    endif
    'Check if input value is within list length
    iListLength = oThisListBox.GetItemCount
    if iThisValue < 1 OR iThisValue > iListLength then
        QAErrorLog "Error 13: Item number out of list range"        
        fSetListBoxByItem = 13
        exit function
    endif
    'Try to set value in List Box
    try 
        oThisListBox.select iThisValue
    catch
        'Throw error 1 and quit on serious problem
        QAErrorLog "Error 1: Set value on list box seems to cause a serious problem."
        fSetListBoxByItem = 1
        exit function
    endcatch
    'Verify (against input) if item was set correctly
    iResultFromUI = oThisListBox.GetSelIndex
    if iResultFromUI = iThisValue then
        fSetListBoxByItem = 0
        printlog ">> Set value in list box seems to work"
    else
        QAErrorLog "Error 2: Set value in list box failed."
        fSetListBoxByItem = 2
    endif

    if fSetListBoxByItem = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function
'
'--------------------------------------------------------------------
'
function fSetListBoxByString ( oThisListBox as OBJECT , sThisString as STRING ) as INTEGER
         fSetListBoxByString = 99
'Function to select item by string in list box
'Input: 
'+OBJECT oThisListBox (list box name in declaration)
'+STRING sThisString (string trying to match in list box items)
'Return(E rror-codes):
'+ 0 = Sucess
'+ 1 = Serious problem trying to set value
'+ 2 = Value was not set
'+11 = List box is not visible
'+12 = List box is not enabled
'+13 = String to be selected doesn't exist in list box
'+99 = Unexpected error

    dim iIndex as INTEGER
    dim iListLength as INTEGER
    dim bStringIsInList as BOOLEAN
    dim sUIStringNow as STRING

    bStringIsInList = FALSE
    sUIStringNow = ""

    printlog "** Setting Item in list box"
    'Check if desired list box is visible
    if NOT oThisListBox.IsVisible then
        qaErrorLog "Error 11: List box is not visible"
        fSetListBoxByString = 11
        exit function
    endif
    'Check if desired list box is enabled
    if NOT oThisListBox.IsEnabled then
        qaErrorLog "Error 12: List box is not enabled"
        fSetListBoxByString = 12
        exit function
    endif
    'Check if input value is existent in list entries
    iListLength = oThisListBox.GetItemCount
    for iIndex = 1 to iListLength
        if oThisListBox.GetItemText ( iIndex ) = sThisString then
            bStringIsInList = TRUE
        endif
    next iIndex
    if bStringIsInList then
        printlog "OK, String exists in list box"
    else
        qaErrorLog "Error 13: Input String not found in list entries"
        fSetListBoxByString = 13
        exit function
    endif
    'Try to set value in List Box
    try 
        oThisListBox.select sThisString
    catch
        'Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Set value on list box seems to cause a serious problem."
        fSetListBoxByString = 1
        exit function
    endcatch
    'Verify (against input) if item was set correctly
    sUIStringNow = oThisListBox.GetSelText
    if sUIStringNow = sThisString then
        fSetListBoxByString = 0
        printlog "Setting value in list box is OK"
    else
        qaErrorLog "Error 2: Set value in list box failed."
        fSetListBoxByString = 2
    endif

    if fSetListBoxByString = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function
'
'--------------------------------------------------------------------
'
function fSetSpinFieldByString ( oThisSpinField as OBJECT , sThisString as STRING ) as INTEGER
         fSetSpinFieldByString = 99
'Function to set a string in a spin field control
'Input: 
'+OBJECT oThisSpinField (spin field name in declaration)
'+STRING sThisString (string to set in spin field control)
'Return (Error codes):
'+ 0 = Sucess
'+ 1 = Serious problem trying to set value
'+11 = Spin field is not visible
'+12 = Spin field is not enabled
'+99 = Unexpected error

    printlog "Setting value in spin field"
    'Check if desired spin field is visible
    if NOT oThisSpinField.IsVisible then
        qaErrorLog "Error 11: Spin field is not visible"
        fSetSpinFieldByString = 11
        exit function
    endif
    'Check if desired spin field is enabled
    if NOT oThisSpinField.IsEnabled then
        qaErrorLog "Error 12: Spin field is not enabled"
        fSetSpinFieldByString = 12
        exit function
    endif
    'Try to set value in spin field
    try 
        oThisSpinField.setText sThisString
        fSetSpinFieldByString = 0
    catch
        'Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Setting value on spin field seems to cause a serious problem."
        fSetSpinFieldByString = 1
        exit function
    endcatch
    if fSetSpinFieldByString = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function
'
'--------------------------------------------------------------------
'
function fSetSpinFieldByButton ( oThisSpinField as OBJECT , sMoreOrLess as STRING , OPTIONAL iTimes as INTEGER ) as INTEGER
         fSetSpinFieldByButton = 99
'Function to set a spin field control by using More or Less button
'Input:
'+OBJECT oThisSpinField (spin field control name in declaration
'+STRING sMoreOrLess has to be <i>more</i> or <i>less</i> (to click on 'More' or 'Less' button in spin field)
'+<i>optional</i> INTEGER iTimes (How often to click if more than once)
'Return (Error codes):
'+ 0 = Sucess
'+ 1 = Serious problem trying to set value
'+11 = Spin field is not visible
'+12 = Spin field is not enabled
'+42 = User error, input doesn't match
'+99 = Unexpected error

    'toggle spin field as least once
    if isMissing ( iTimes ) then
        iTimes = 1
    endif
    printlog "Toggle value in spin field"
    'Check if desired spin field is visible
    if NOT oThisSpinField.IsVisible then
        qaErrorLog "Error 11: Spin field is not visible"
        fSetSpinFieldByButton = 11
        exit function
    endif
    'Check if desired spin field is enabled
    if NOT oThisSpinField.IsEnabled then
        qaErrorLog "Error 12: Spin field is not enabled"
        fSetSpinFieldByButton = 12
        exit function
    endif
    'Try to toggle value in spin field
    try 
        select case lcase ( sMoreOrLess ) 
               case "more" : oThisSpinField.more ( iTimes )
               case "less" : oThisSpinField.less ( iTimes )
               case else
                    fSetSpinFieldByButton = 42
                    warnlog "USER ERROR: Input doesn't match!"
                    exit function
        end select
        fSetSpinFieldByButton = 0
    catch
        'Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Try to toggle spin field seems to cause a serious problem."
        fSetSpinFieldByButton = 1
        exit function
    endcatch
    if fSetSpinFieldByButton = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function
'
'--------------------------------------------------------------------
'
function fSetSpinFieldByLimit ( oThisSpinField as OBJECT , sMinOrMax as STRING ) as INTEGER
         fSetSpinFieldByLimit = 99
'Function to set a spin field control to minimum or maximum value
'Input:
'+OBJECT oThisSpinField (spin field name in declaration
'+STRING sMinOrMax has to be <i>min</i> or <i>max</i> (to set spin field to minimum or maximum value)
'Return (Error codes):
'+ 0 = Sucess
'+ 1 = Serious problem trying to set value
'+11 = Spin field is not visible
'+12 = Spin field is not enabled
'+42 = User error, input doesn't match
'+99 = Unexpected error

    printlog "Set spin field control to minimum or maximum value"
    'Check if desired spin field is visible
    if NOT oThisSpinField.IsVisible then
        qaErrorLog "Error 11: Spin field is not visible"
        fSetSpinFieldByLimit = 11
        exit function
    endif
    'Check if desired spin field is enabled
    if NOT oThisSpinField.IsEnabled then
        qaErrorLog "Error 12: spin field is not enabled"
        fSetSpinFieldByLimit = 12
        exit function
    endif
    'Try to set spin field to minimum or maximum value
    try 
        select case lcase ( sMinOrMax ) 
               case "max" : oThisSpinField.toMax
               case "min" : oThisSpinField.toMin
               case else
                    fSetSpinFieldByLimit = 42
                    warnlog "USER ERROR: Input doesn't match!"
                    exit function
        end select
        fSetSpinFieldByLimit = 0
    catch
        'Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Trying to set spin field to minimum or maximum value seems to cause a serious problem."
        fSetSpinFieldByLimit = 1
        exit function
    endcatch
    if fSetSpinFieldByLimit = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif 
end function
'
'--------------------------------------------------------------------
'
function fSetComboBoxByItem ( oThisComboBox as OBJECT , iThisValue as INTEGER ) as INTEGER
'This is an alias for 'fSetListBoxByItem'

'Function to select item in combo box
'Input:
'+OBJECT oThisComboBox (combo box name in declaration)
'+INTEGER iThisValue (item number to be selected in Combo box)
'Return (Error codes):
'+ 0 = Sucess
'+ 1 = Serious problem trying to set value
'+ 2 = Value was not set
'+11 = Combo box is not visible
'+12 = Combo box is not enabled
'+13 = Item number to be selected out of list range
'+99 = Unexpected error
    fSetComboBoxByItem = fSetListBoxByItem ( oThisComboBox , iThisValue )
end function
'
'--------------------------------------------------------------------
'
function fSetComboBoxByString ( oThisComboBox as OBJECT , iThisString as STRING ) as INTEGER
'This is an alias for 'fSetListBoxByString'
'Function to select item in combo box
'Input:
'+OBJECT oThisComboBox (combo box name in declaration)
'+STRING iThisString (string to be selected in combo box control)
'Return (Error codes):
'+ 0 = Sucess
'+ 1 = Serious problem trying to set value
'+ 2 = Value was not set
'+11 = Combo box is not visible
'+12 = Combo box is not enabled
'+13 = String to be selected doesn't exist in list box
'+99 = Unexpected error
    fSetComboBoxByString = fSetListBoxByString ( oThisComboBox , iThisString )
end function
'
'--------------------------------------------------------------------
'
function fSetComboBoxByText ( oThisComboBox as OBJECT , sThisText as STRING , OPTIONAL bVerifyAgainstEntries as BOOLEAN ) as INTEGER
         fSetComboBoxByText = 99
'Function to Edit Field part of a combo box
'Input:
'+OBJECT oThisComboBox (combo box name in declaration)
'+STRING sThisText (Text to be set in combo box edit field
'+<i>optional</i> BOOLEAN bVerifyAgainstEntries (Check if setting the text succeeded)
'Return (Error codes):
'+ 0 = Sucess
'+ 1 = Serious problem trying to set value
'+ 2 = Value was not set
'+11 = Combo box is not visible
'+12 = Combo box is not enabled
'+13 = String to be selected doesn't exist in list box (if <i>optional</i> input parameter has been used)
'+99 = Unexpected error

    dim iIndex as INTEGER
    dim iListLength as INTEGER
    dim bStringIsInList as BOOLEAN
    dim sStringinUI as STRING
    
        bStringIsInList = FALSE
        sStringinUI = ""
        
    'Only check Edit Field against list box if 'bVerifyAgainstEntries' is explizitly TRUE	
    if isMissing ( bVerifyAgainstEntries ) then
        bVerifyAgainstEntries = FALSE
    endif
    
    printlog "Setting text in Combo Box Edit field"
    'Check if desired combo box is visible
    if NOT oThisComboBox.IsVisible then
        qaErrorLog "Error 11: Combo box is not visible"
        fSetComboBoxByText = 11
        exit function
    endif
    'Check if desired combo box is enabled
    if NOT oThisComboBox.IsEnabled then
        qaErrorLog "Error 12: Combo box is not enabled"
        fSetComboBoxByText = 12
        exit function
    endif
    'Check if input value is existent in list entries (optinal if desired)
    if bVerifyAgainstEntries then
        iListLength = oThisComboBox.GetItemCount
        for iIndex = 1 to iListLength
            if oThisComboBox.GetItemText ( iIndex ) = sThisText then
                bStringIsInList = TRUE
            endif
        next iIndex
        if bStringIsInList then
            printlog "OK, string exists in list box control"
        else
            qaErrorLog "Error 13: Input string not found in list entries"
            fSetComboBoxByText = 13
            exit function
        endif
    endif
    'Trying to set text on combo box
    try 
        oThisComboBox.setText sThisText
    catch
        'Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Set text in combo box seems to cause a serious problem."
        fSetComboBoxByText = 1
        exit function
    endcatch
    'Verify (against input) if text was set correctly
    sStringinUI = oThisComboBox.GetSelText
    if sStringinUI = sThisText then
        fSetComboBoxByText = 0
        printlog "Setting text in Combo box works"
    else
        qaErrorLog "Error 2: Set text in Combo box failed."
        fSetComboBoxByText = 2
    endif

    if fSetComboBoxByText = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif  
end function
'
'--------------------------------------------------------------------
'
function fSetCheckBox ( oThisCheckBox as OBJECT , bCheck as BOOLEAN ) as INTEGER
         fSetCheckBox = 99
'Function to (un)check a check box
'Input:
'+OBJECT oThisCheckBox (check box name in declaration)
'+BOOLEAN bCheck (check or uncheck the check box control)
'Return (Error codes):
'+ 0 = Sucess
'+ 1 = Serious problem trying to check the box
'+ 2 = Box was not checked
'+11 = Check box is not visible
'+12 = Check box is not enabled
'+99 = Unexpected error

    printlog "Checking check box"
    'Check if desired check box is visible
    if NOT oThisCheckBox.IsVisible then
        qaErrorLog "Error 11: Check box is not visible"
        fSetCheckBox = 11
        exit function
    endif
    'Check if desired check box is enabled
    if NOT oThisCheckBox.IsEnabled then
        qaErrorLog "Error 12: Check box is not enabled"
        fSetCheckBox = 12
        exit function
    endif
    'Try to check check box control
    try 
        if bCheck then
            oThisCheckBox.Check
        else
            oThisCheckBox.Uncheck
        endif
    catch
        'Throw error 1 and quit on serious problem
        if bCheck then
            qaErrorLog "Error 1: Checking check box control cause into a serious problem."
        else
            qaErrorLog "Error 1: Unchecking check box control cause into a serious problem."
        endif
        fSetCheckBox = 1
        exit function
    endcatch
    'Verify (against input) if check box is checked
    if oThisCheckBox.IsChecked = bCheck then
        fSetCheckBox = 0
        printlog "Check check box seems to work"
    else
        qaErrorLog "Error 2: Check check box failed."
        fSetCheckBox = 2
    endif
    if fSetCheckBox = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif  
end function
'
'--------------------------------------------------------------------
'
function fSetRadioButton ( oThisRadioButton as OBJECT  ) as INTEGER
         fSetRadioButton = 99
'Function to check a radio button
'Input:
'+OBJECT oThisRadioButton (radio button name in declaration)
'Return (Error codes):
'+ 0 = Sucess
'+ 1 = Serious problem trying to check the radio button
'+ 2 = Radio button was not checked
'+11 = Radio button is not visible
'+12 := Radio button is not enabled
'+99 := Unexpected error
    printlog "Checking radio button"
    'Check if desired radio button is visible
    if NOT oThisRadioButton.IsVisible then
        qaErrorLog "Error 11: Radio button is not visible"
        fSetRadioButton = 11
        exit function
    endif
    'Check if desired radio button is enabled
    if NOT oThisRadioButton.IsEnabled then
        qaErrorLog "Error 12: Radio button is not enabled"
        fSetRadioButton = 12
        exit function
    endif
    'Try to check Radio Button
    try 
        oThisRadioButton.Check
    catch
        'Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Check radio button seems to cause a serious problem."
        fSetRadioButton = 1
        exit function
    endcatch
    'Verify if radio button is checked
    if oThisRadioButton.IsChecked = TRUE then
        fSetRadioButton = 0
        printlog "Check radio button seems to work"
    else
        qaErrorLog "Error 2: Check radio button failed."
        fSetRadioButton = 2
    endif
    if fSetRadioButton = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif  
end function

