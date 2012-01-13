'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************

************************
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
' Error 2 TO 9: A functional problem occured.
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

