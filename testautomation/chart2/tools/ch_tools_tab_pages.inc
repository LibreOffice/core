'encoding UTF-8  Do not remove or change this line!
'**************************************************************
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
'**************************************************************
'*
'* short description : Tool library for Borders and Lines tab-page
'*
'**************************************************************************************************
'*
' #1 fInvokeTabPage
' #1 fCloseTabLineOK
'*
'\************************************************************************************************

' ch_tools_tab_pages.inc - Library for automation of tab pages
' This Include contains a functions to handle tab-pages.
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
' Only Errors 42 and 99 throw 'warnlogs'.
' All other errors are silent.
' They only throw QAErrorlogs the give a hint what probably went wrong.
' Expected Errors MUST exclusivly be handled by the calling routine.
'
'--------------------------------------------------------------------
'
function fInvokeTabPage( oThisPage as OBJECT ) as INTEGER
         fInvokeTabPage = 99
' Function to invoke a tab page
' Input:
' OBJECT oThisPage
' Tab page name in declaration
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to invoke the page
' 2 := Failure (Page not present after invocation)
'99 := Unexpected error
    
    printlog "** Invoking tab page"
    '/// Try to invoke tab page
    Kontext
    try 
        active.setPage oThisPage
    catch
        qaErrorLog "Error 1: Invoking tab page failed"
        fInvokeTabPage = 1
        exit function
    endcatch
    
    '/// Lookup if call was successful
    Kontext oThisPage
    if oThisPage.exists(2) then
        printlog ">> Tab page is visible now."
        fInvokeTabPage = 0
    else
        qaErrorLog "Error 2: OOPS, calling Tab page cause any problem ..."
        qaErrorLog "..., BUT the page doesn't seem to be visible"
        fInvokeTabPage = 2
    endif

    if fInvokeTabPage = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif    
end function
'
'--------------------------------------------------------------------
'
function fCloseTabPage( oThisPage as OBJECT ) as INTEGER
         fCloseTabPage = 99
' Function to close a tab page dialog using OK button
' Input:
' OBJECT oThisPage
' Tab page name in declaration
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to Close the page
' 2 := Failure (Page present after applying OK button)
'15 := Page not present before closing
'99 := Unexpected error
    printlog "** Closing tab page"
    Kontext oThisPage
    '/// Check existence of tab page
    if oThisPage.exists(2) then
        printlog ">> Tab page is visible as expected."
    else
        ' Return Error 15 and quit if page not found
        qaErrorLog "Error 15: OOPS, tab page should be visible ..."
        qaErrorLog "... this is a BUG or a scripting error -> Check this out!"
        fCloseTabPage = 15
        exit function
    endif
    '/// Click OK button in tab page
    try 
        oThisPage.OK
    catch
        qaErrorLog "Error 1: Closing tab page seems to have a serious problem -> Check this out!"
        fCloseTabPage = 1
        exit function
    endcatch
    '/// Check that of tab page has been gone
    Kontext oThisPage
    if oThisPage.exists(2) then
        ' Return Error 2 if still present
        qaErrorLog "Error 2: Tab page should be invisible now ..."
        qaErrorLog "... closing the dialog doesn't seem to work -> Check this out!"
        fCloseTabPage = 2
    else
        printlog ">> Tab page seems to work as expected"
        fCloseTabPage = 0
    endif
    
    if fCloseTabPage = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
end function

