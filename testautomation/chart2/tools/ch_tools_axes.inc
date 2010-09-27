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
'* short description : Library for automation of the 'Insert::Axes...' dialog
'*
'**************************************************************************************************
'*
' #1 fInvokeAxesDialog
' #1 fCloseAxesDialogOK
' #1 fSetAxis
'*
'\************************************************************************************************

' Return codes (for all functions):
' Error 0: Success.
' Error 1: The basic action beeing applied caused a serious problem, e.g. a crash.
' Error 2 to 9: A functional problem occured.
' Error 11 to 19: Wrong marginal conditions end up in Failure, e.g. control not visible.
' Note: Following errors can be used for negative testing.
' Error 42: Wrong input. Probably only of interest during test development.
' Error 99: Unexpected behaviour - Shouldn't normally occur.
' Attention:
' Only error 42 and error 99 throw 'warnlogs'.
' All other errors are silent.
' They only throw QAErrorlogs the give a hint what probably went wrong.
' Expected Errors <b>must</b> exclusivly be handled by the calling routine.
'
'--------------------------------------------------------------------
'
function fInvokeAxesDialog() as INTEGER
         fInvokeAxesDialog = 99
' Function to invoke the 'Insert::Axes...' dialog
' No Input
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to invoke the dialog
' 2 := Failure (Dialog not present after invocation)
'99 := Unexpected error
    
    printlog "** Invoking 'Insert::Axes...' in menu"
    '/// Execute menu item 'Insert::Axes...'
    try 
        InsertAxes
    catch
        qaErrorLog "Error 1: Invoking menu item 'Insert::Axes...' failed"
        fInvokeAxesDialog = 1
        exit function
    endcatch
    
    '/// Lookup if call was successful
    Kontext "InsertAxesDialog"
    if InsertAxesDialog.exists(2) then
        printlog ">> Axes dialog is visible now."
        fInvokeAxesDialog = 0
    else
        qaErrorLog "Error 2: OOPS, calling the slot 'InsertAxes' didn't cause any problem ..."
        qaErrorLog "..., BUT the dialog doesn't seem to be visible"
        fInvokeAxesDialog = 2
    endif

    if fInvokeAxesDialog = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif    
end function
'
'--------------------------------------------------------------------
'
function fCloseAxesDialogOK() as INTEGER
         fCloseAxesDialogOK = 99
' Function to close the Axes dialog using OK button
' No Input
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to Close the dialog
' 2 := Failure (Dialog present after applying OK button)
'15 := Dialog not present before closing
'99 := Unexpected error    
    printlog "** Closing Axes dialog"
    Kontext "InsertAxesDialog"
    '/// Check existence of Axes dialog
    if InsertAxesDialog.exists(2) then
        printlog ">> Axes dialog is visible as expected."
    else
        ' Return Error 15 and quit if dialog not found
        qaErrorLog "Error 15: OOPS, Axes dialog should be visible ..."
        qaErrorLog "... this is a BUG or a scripting error -> Check this out!"
        fCloseAxesDialogOK = 15
        exit function
    endif
    '/// Click OK button in Axes dialog
    try 
        InsertAxesDialog.OK
    catch
        qaErrorLog "Error 1: Closing the Axes dialog seems to have a serious problem -> Check this out!"
        fCloseAxesDialogOK = 1
        exit function
    endcatch
    '/// Check that Axes dialog is not visible anymore
    Kontext "InsertAxesDialog"
    if InsertAxesDialog.exists(2) then
        ' Return Error 2 if still present
        qaErrorLog "Error 2: Axes dialog should be invisible now ..."
        qaErrorLog "... closing the dialog doesn't seem to work -> Check this out!"
        fCloseAxesDialogOK = 2
    else
        printlog ">> Closing the Axes dialog seems to work as expected"
        fCloseAxesDialogOK = 0
    endif
    
    if fCloseAxesDialogOK = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
end function
'
'--------------------------------------------------------------------
'
function fSetAxis ( oAxis as OBJECT , bAxisCheck as BOOLEAN ) as INTEGER
         fSetAxis = 99
' Function to check-boxes in Axes dialog
' Input:
' OBJECT oAxis
' Axis Indicator (Name of declaration): Major[XYZ]Axis, Secondary[XY]Axis are valid names
' BOOLEAN bAxisCheck
' TRUE  := Check Axis
' FALSE := Uncheck Axis
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to check axis
' 2 := Axis was not set
'12 := Check-box for desired axis is not enabled
'99 := Unexpected error    
    printlog "** Setting axis"
    '/// Check if desired axis check-box is enabled
    if NOT oAxis.IsEnabled then
        qaErrorLog "Error 12: Check-box for desired axis is not enabled"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetAxis = 12
        exit function
    endif
    '/// Try to (Un)Check desired axis
    Kontext "InsertAxesDialog"
    try 
        if bAxisCheck then
            oAxis.Check
        else
            oAxis.UnCheck
        endif
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Check axis seems to cause a serious problem -> Check this out!"
        fSetAxis = 1
        exit function
    endcatch
    '/// Verify (against input) if axis was checked
    if oAxis.IsChecked = bAxisCheck then
        fSetAxis = 0
        printlog ">> Chech axis seems to work"
    else
        qaErrorLog "Error 2: Axis was not checked -> Check this out!"
        fSetAxis = 2
    endif

    if fSetAxis = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function

