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
'* short description : selecting charts
'*
'**************************************************************************************************
'*
' #1 fSelectChartByNumber
' #1 fCalcSelectSheet
' #1 fChartSelectElement
'*
'\************************************************************************************************

function fSelectChartByNumber ( iObjectPosition as INTEGER )
    ' Select any Chart in document
    ' INTEGER: iObjectPosition - Position of object (>=1) within OLE objects
    ' NOTE: You're moving within a treelistbox which is quite messy to handle.
    ' In case 'iObjectPosition' is larger than the real number of objects in category you're end up in another categories.
    ' WITHOUT ANY WARNING!
    ' RETURNS: BOOLEAN (success)    
    dim bNavigatorWasVisible as boolean
        bNavigatorWasVisible = FALSE
    dim iIndex
    dim iCategoryPosition as INTEGER
        iCategoryPosition = 6
    
    fSelectChartByNumber = FALSE

    '/// Check if Navigator is visible
    Kontext "NavigatorCalc"
    if NavigatorCalc.exists (3) then    
        bNavigatorWasVisible = TRUE
    else
        '/// Invoke navigator if not visible
        ViewNavigator
    end if
    
    try 
        Kontext "NavigatorCalc"
        '/// Go to Top in Navigator
        liste.TypeKeys "<HOME>"
        '/// Travel top to bottom through all categories
        for iIndex = 1 to 6
        '/// Make sure all elements in category are hidden, apply '-' key
             liste.TypeKeys "-<DOWN>"
        next iIndex
        '/// Select desired category OLE
        liste.select ( iCategoryPosition )
        '/// Unfold elements of selected category
        liste.TypeKeys "+"
        '/// Select desired position within category
        liste.select ( iCategoryPosition + iObjectPosition )
        '/// Hit 'RETURN' key to select element in document
        liste.TypeKeys "<RETURN>"
        '/// Return 'TRUE' for 'fNavigatorSelectObject'
        fSelectChartByNumber = TRUE
    catch
        warnlog "Selecting the desired object failed. Perhaps your input wasn't valid"
    endcatch
    '/// Close navigator if it wasn't visible before entering this function
    if bNavigatorWasVisible = TRUE then
        printlog "Leaving navigator open as initially found"
    else
        ViewNavigator
        printlog "Closing navigator as initially found"
    end if
end function
'
'--------------------------------------------------------------------
'
function fCalcSelectSheet ( sSelectThisSheet ) as boolean
   ' Select a sheet by name (STRING) or number (INTEGER)
   ' RETURNS: boolean (success)
   fCalcSelectSheet = FALSE   
   try
       Kontext "DocumentCalc"
       '/// Invoke Edit::Sheet::Select
       EditSheetSelect
       Kontext "SelectSheets"
       '/// Select sheet no. or sheet name in listbox
       SheetSelectionBox.Select ( sSelectThisSheet )
       '/// OK
       SelectSheets.OK
       '/// Return true
       fCalcSelectSheet = TRUE
   catch
       warnlog "Something went wrong while selecting a sheet by name"
       '/// In case of Failure lookup 'Select sheets' dialog andd close it if exists
       Kontext "SelectSheets"
       if SelectSheets.exists(2) then
           SelectSheets.Cancel
       endif
   endcatch   
end function
'
'--------------------------------------------------------------------
'
function fChartSelectElement ( iObjectPosition as INTEGER ) as boolean
	' Select a chartelement by using the ChartElementSelectorListBox in the Toolbar
	' RETURNS: boolean (success)
	fChartSelectElement = FALSE
	try
		Kontext "Toolbar"
		sleep (2)
		ChartElementSelector.Select (iObjectPosition)
		fChartSelectElement = TRUE
	catch
		warnlog "Something went wrong while selecting a chartelement"
	endcatch
end function
