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
' Error 2 TO 9: A functional problem occurred.
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

