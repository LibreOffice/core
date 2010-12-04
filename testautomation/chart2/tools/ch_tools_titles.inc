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
'* short description : Tool library for Titles dialog
'*
'**************************************************************************************************
'*
' #1 fInvokeTitlesDialog
' #1 fCloseTitlesDialogOK
' #1 fSetTitle
'*
'\************************************************************************************************

' ch_tools_titles.inc -  A library for automation of the 'Insert::Titles...' dialog
' This Include contains a set of functions to modify the controls of the Titles dialog.
' All functions are designed to return error-codes depending on the behaviour of the action applied.
' Return codes:
' Error 0: Success.
' Error 1: The basic action beeing applied caused a serious problem, e.g. a crash.
' Error 2 TO 9: A functional problem occurred.
' Error 11 TO 19: Wrong marginal conditions end up in Failure, e.g. control not visible.
' NOTE: This errors can also be used for 'negative' testing.
' Error 42: Wrong input. Probably only of interest during test development .
' Error 99: Unexpected behaviour - Shouldn't normally occur.
' ATTENTION:
' Only Errors 42 and 99 throw 'Warnlogs'.
' All other errors are silent!
' They only throw QAErrorlogs the give a hint what probably went wrong.
' Expected Errors MUST exclusivly be handled by the calling routine!
'
'--------------------------------------------------------------------
'
function fInvokeTitlesDialog() as INTEGER
         fInvokeTitlesDialog = 99
' Function to invoke the 'Insert::Titles...' dialog
' Return (Error codes):
' 0 = Sucess
' 1 = Serious problem trying to invoke the dialog
' 2 = Failure (Dialog not present after invocation)
'99 = Unexpected error
    printlog "Invoking 'Insert::Titles...' in menu"
    '/// Execute menu item 'Insert::Titles...'
    try 
        InsertTitle
    catch
        qaErrorLog "Error 1: Invoking menu item 'Insert::Titles...' failed"
        fInvokeTitlesDialog = 1
        exit function
    endcatch
    
    '/// Lookup if call was successfull
    Kontext "InsertTitleDialog"
    if InsertTitleDialog.exists(2) then
        printlog "Titles dialog is visible now."
        fInvokeTitlesDialog = 0
    else
        qaErrorLog "Error 2: 'InsertTitles' has been executed but the dialog is not visible"
        fInvokeTitlesDialog = 2
    endif

    if fInvokeTitlesDialog = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif    
end function
'
'--------------------------------------------------------------------
'
function fCloseTitlesDialogOK() as INTEGER
         fCloseTitlesDialogOK = 99
' Function to close the Titles dialog using OK button
' No Input
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to Close the dialog
' 2 := Failure (Dialog present after applying OK button)
'15 := Dialog not present before closing
'99 := Unexpected error        
    
printlog "** Closing Titles dialog"
    Kontext "InsertTitleDialog"
    '/// Check existence of Titles dialog
    if InsertTitleDialog.exists(2) then
        printlog ">> Titles dialog is visible as expected."
    else
        ' Return Error 15 and quit if dialog not found
        qaErrorLog "Error 15: OOPS, Titles dialog should be visible ..."
        qaErrorLog "... this is a BUG or a scripting error -> Check this out!"
        fCloseTitlesDialogOK = 15
        exit function
    endif
    '/// Click OK button in Titles dialog
    try 
        InsertTitleDialog.OK
    catch
        qaErrorLog "Error 1: Closing the Titles dialog seems to have a serious problem -> Check this out!"
        fCloseTitlesDialogOK = 1
        exit function
    endcatch
    '/// Check that Titles dialog not existing anymore
    Kontext "InsertTitleDialog"
    if InsertTitleDialog.exists(2) then
        ' Return Error 2 if still present
        qaErrorLog "Error 2: Titles dialog should be invisible now ..."
        qaErrorLog "... closing the dialog doesn't seem to work -> Check this out!"
        fCloseTitlesDialogOK = 2
    else
        printlog ">> Closing the Titles dialog seems to work as expected"
        fCloseTitlesDialogOK = 0
    endif
    
    if fCloseTitlesDialogOK = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
end function
'
'--------------------------------------------------------------------
'
function fSetTitle ( oTitle as OBJECT , sTitleString as STRING ) as INTEGER
         fSetTitle = 99
' Function to set strings in Titles dialog
' Input: OBJECT oTitle
' Title Indicator (Name of declaration):
' MainTitle, SubTitle, TitleXaxis, TitleYaxis, TitleZaxis are valid names
' STRING sTitleString
' Text to set on 'oTitle' 
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to set title
' 2 := Title was not set
'12 := Text-box for desired title is not enabled
'99 := Unexpected error
    printlog "** Setting title"
    '/// Check if desired title text-box is enabled
    if NOT oTitle.IsEnabled then
        qaErrorLog "Error 12: Text-box for desired title is not enabled"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetTitle = 12
        exit function
    endif
    '/// Put the text in desired title field 
    Kontext "InsertTitleDialog"
    try 
        oTitle.setText ( sTitleString )
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Set text on title seems to cause a serious problem -> Check this out!"
        fSetTitle = 1
        exit function
    endcatch
    '/// Verify (against input) if title was set correctly
    if oTitle.getText = sTitleString then
        fSetTitle = 0
        printlog ">> Setting title seems to work"
    else
        qaErrorLog "Error 2: Title was not set -> Check this out!"
        fSetTitle = 2
    endif

    if fSetTitle = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function

