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
