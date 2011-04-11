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
'* short description : Tool library for Grids dialog
'*
'**************************************************************************************************
'*
' #1 fInvokeGridsDialog
' #1 fCloseGridsDialogOK
' #1 fSetGrid
'*
'\************************************************************************************************

' ch_tools_grids.inc - Library for automation of the 'Insert::Grids...' dialog
' This Include contains a set of functions to modify the controls of the Grids dialog.
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
function fInvokeGridsDialog() as INTEGER
         fInvokeGridsDialog = 99
' Function to invoke the 'Insert / Grids...' dialog
' Returns (Error codes):
' 0 = Sucess
' 1 = Serious problem trying to invoke the dialog
' 2 = Failure (Dialog not present after invocation)
'99 = Unexpected error
    printlog "Invoking 'Insert::Grids...' in menu"
    '/// Execute menu item 'Insert::Grids...'
    try 
        InsertGrids
    catch
        QAErrorLog "Error 1: Invoking menu item 'Insert::Grids...' failed"
        fInvokeGridsDialog = 1
        exit function
    endcatch
    
    '/// Lookup if call was successful
    Kontext "InsertGridsDialog"
    if InsertGridsDialog.exists(2) then
        printlog ">> Grids dialog is visible now."
        fInvokeGridsDialog = 0
    else
        qaErrorLog "Error 2: The slot 'InsertGrids' has been executed but the dialog is not visible"
        fInvokeGridsDialog = 2
    endif

    if fInvokeGridsDialog = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif    
end function
'
'--------------------------------------------------------------------
'
function fCloseGridsDialogOK() as INTEGER
         fCloseGridsDialogOK = 99
' Function to close the Grids dialog using OK button
' No Input
' Returns error-codes:
' 0 := Sucess
' 1 := Serious problem trying to Close the dialog
' 2 := Failure (Dialog present after applying OK button)
'15 := Dialog not present before closing
'99 := Unexpected error    
    printlog "** Closing Grids dialog"
    Kontext "InsertGridsDialog"
    '/// Check existence of Grids dialog
    if InsertGridsDialog.exists(2) then
        printlog ">> Grids dialog is visible as expected."
    else
        'Return Error 15 and quit if dialog not found
        qaErrorLog "Error 15: OOPS, Grids dialog should be visible ..."
        qaErrorLog "... this is a BUG or a scripting error -> Check this out!"
        fCloseGridsDialogOK = 15
        exit function
    endif
    '/// Click OK button in Grids dialog
    try 
        InsertGridsDialog.OK
    catch
        qaErrorLog "Error 1: Closing the Grids dialog seems to have a serious problem -> Check this out!"
        fCloseGridsDialogOK = 1
        exit function
    endcatch
    '/// Check that Grids dialog is not available anymore
    Kontext "InsertGridsDialog"
    if InsertGridsDialog.exists(2) then
        'Return Error 2 if still present
        qaErrorLog "Error 2: Grids dialog should be invisible now ..."
        qaErrorLog "... closing the dialog doesn't seem to work -> Check this out!"
        fCloseGridsDialogOK = 2
    else
        printlog ">> Closing the Grids dialog seems to work as expected"
        fCloseGridsDialogOK = 0
    endif
    
    if fCloseGridsDialogOK = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif
end function
'
'--------------------------------------------------------------------
'
function fSetGrid ( oGrid as OBJECT , bGridCheck as BOOLEAN ) as INTEGER
         fSetGrid = 99
' Function to check-boxes in Grids dialog
' Input:
' OBJECT oGrid
' Grid Indicator (Name of declaration): MainGrid[XYZ]Axis, MinorGrid[XYZ]Axis< are valid name
' BOOLEAN bGridCheck
' TRUE := Check Grid
' FALSE := Uncheck Grid
' Returns error-code:
' 0 := Sucess
' 1 := Serious problem trying to check grid
' 2 := Grid was not set
'11 := Check-box for desired grid is not visible
'12 := Check-box for desired grid is not enabled
'99 := Unexpected error 
    printlog "** Setting Grid"
    Kontext "InsertGridsDialog"
    '/// Check if desired grid check-box is visible
    if NOT oGrid.IsVisible then
        qaErrorLog "Error 11: Check-box for desired grid is not visible"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetGrid = 11
        exit function
    endif
    '/// Check if desired grid check-box is enabled
    if NOT oGrid.IsEnabled then
        qaErrorLog "Error 12: Check-box for desired grid is not enabled"
        qaErrorLog "... BUG or Script problem -> Check this out!"
        fSetGrid = 12
        exit function
    endif
    '/// Try to (Un)Check desired Grid
    Kontext "InsertGridsDialog"
    try 
        if bGridCheck then
            oGrid.Check
        else
            oGrid.UnCheck
        endif
    catch
        ' Throw error 1 and quit on serious problem
        qaErrorLog "Error 1: Check grid seems to cause a serious problem -> Check this out!"
        fSetGrid = 1
        exit function
    endcatch
    '/// Verify (against input) if grid was checked
    if oGrid.IsChecked = bGridCheck then
        fSetGrid = 0
        printlog ">> Setting grid seems to work"
    else
        qaErrorLog "Error 2: Grid was not checked -> Check this out!"
        fSetGrid = 2
    endif

    if fSetGrid = 99 then
        warnlog "Error 99: Something unexpected happened!!"
    endif   
end function

