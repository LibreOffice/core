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
' #1 fInvokeTabLine
' #1 fCloseTabLineOK
'*
'\************************************************************************************************

' ch_tools_tab_borders_lines.inc - A library for automation of tab-pages Borders and Lines
' NOTE: Internal both tab-pages are identical
' This Include contains a set of functions to modify the controls of these tab-pages.
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
function fInvokeTabLine() as INTEGER
         fInvokeTabLine = 99
' Function to invoke the Borders or Line tab page in several dialogs
' No Input
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to invoke the page
' 2 := Failure (Page not present after invocation)
'99 := Unexpected error
    
    printlog "** Invoking 'Line' or 'Borders' tab page"
    '/// Try to change to 'Line' or 'Borders' tab page
    Kontext
    try 
        active.setPage TabFormatChartLine
    catch
        qaErrorLog "Error 1: Invoking tab page 'Line' or 'Borders' failed"
        fInvokeTabLine = 1
        exit function
    endcatch
    
    '/// Lookup if call was successful
    Kontext "TabFormatChartLine"
    if TabFormatChartLine.exists(2) then
        printlog ">> Tab page 'Line' or 'Borders' is visible now."
        fInvokeTabLine = 0
    else
        qaErrorLog "Error 2: OOPS, calling Tab page 'Line' or 'Borders' didn't cause any problem ..."
        qaErrorLog "..., BUT the page doesn't seem to be visible"
        fInvokeTabLine = 2
    endif

    if fInvokeTabLine = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif    
end function
'
'--------------------------------------------------------------------
'
function fCloseTabLineOK() as INTEGER
         fCloseTabLineOK = 99
' Function to close a dialog using OK button while tab page 'Line' or 'Borders' is visible
' No Input
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to Close the page
' 2 := Failure (Page present after applying OK button)
'15 := Page not present before closing
'99 := Unexpected error    
    printlog "** Closing 'Line' or 'Borders' tab page"
    Kontext "TabFormatChartLine"
    '/// Check existence of 'Line' or 'Borders' tab page
    if TabFormatChartLine.exists(2) then
        printlog ">> 'Line' or 'Borders' tab page is visible as expected."
    else
        ' Return Error 15 and quit if page not found
        qaErrorLog "Error 15: OOPS,'Line' or 'Borders' tab page should be visible ..."
        qaErrorLog "... this is a BUG or a scripting error -> Check this out!"
        fCloseTabLineOK = 15
        exit function
    endif
    '/// Click OK button in 'Line' or 'Borders' tab page
    try 
        TabFormatChartLine.OK
    catch
        qaErrorLog "Error 1: Closing the 'Line' or 'Borders' tab page seems to have a serious problem -> Check this out!"
        fCloseTabLineOK = 1
        exit function
    endcatch
    '/// Check that the  'Line' or 'Borders' tab page is not available anymore.
    Kontext "TabFormatChartLine"
    if TabFormatChartLine.exists(2) then
        ' Return Error 2 if still present
        qaErrorLog "Error 2: 'Line' or 'Borders' tab page should be invisible now ..."
        qaErrorLog "... closing the dialog doesn't seem to work -> Check this out!"
        fCloseTabLineOK = 2
    else
        printlog ">> Closing 'Line' or 'Borders' tab page seems to work as expected"
        fCloseTabLineOK = 0
    endif
    
    if fCloseTabLineOK = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
end function

