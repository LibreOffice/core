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
'* short description : selecting objects in calc
'*
'**************************************************************************************************
'*
' #1 fCalcSelectRange
' #1 fCalcNameRange
' #1 fCalcSelectCell
' #1 fCalcGetSelectionString
' #1 fCalcSelectSheet
' #1 fNavigatorSelectObject
'*
'\************************************************************************************************

function fCalcSelectRange ( sSelectThis ) as boolean
    '///<b>Select a range in calc and check success
    '///+Valid cell reference and name should work here
    '///+RETURNS: BOOLEAN</b>
    fCalcSelectRange = FALSE
    '///<ul>
    '///<li>Check if 'formula bar' is visible</li>///
    Kontext "RechenleisteCalc"
    if not RechenleisteCalc.isvisible then
        '///<li>-> Throw warning and Invoke 'formula bar' if invisible</li>///
        '///<li>-> NOTE: The testcase developer has to take care herself that function is entered correctly</li>///
        warnlog "Formula bar was expected to be visible"
        ViewToolbarsFormulaBar
    end if
    sleep(2)
    '///<li>Select the desired range</li>///
    Kontext "RechenleisteCalc"
    Bereich.TypeKeys "<MOD1 A>"
    Bereich.TypeKeys ( sSelectThis )
    sleep(1)
    Bereich.TypeKeys "<RETURN>"
    sleep(2)
    '///<li>Check if input leads to success</li>///
    kontext
    '///<li>If a MsgBox is in the way the selection must have failed</li>///
    if active.exists then
       if active.getRT = 304 then
           '///<li>-> NOTE: Only throw a PrintLog if selection fails because failture may be the desired result!</li>///
           '///<li>-> FURTHER NOTE: We retrun FALSE to check this in the calling routine</li>///
           printlog "Selecting the range failed due to known reason, e.g. wrong selection or invalid name"
           Active.OK
       else
           '///<li>Only throw a waring if a dialog of unexpected resource type appears</li>///
           warnlog "OOPS, don't know what's disturbing now -> CHECK IT OUT!"
       end if
    else
       '///<li>Everything OK? -> TRUE</li>///
       fCalcSelectRange = TRUE
    end if
end function
'///</ul>

'-------------------------------------------------------------------------

function fCalcNameRange ( sNameThis ) as boolean
    '///<b>This is just an alias function to avoid confusion -> see 'fCalcSelectRange'<b>
    '///With the integration of CWS 'rangename' the function 'fCalcSelectRange'
    '///+could also be used to name a previous selected range
    fCalcNameRange = fCalcSelectRange ( sNameThis )
end function

'
'--------------------------------------------------------------------
'

function fCalcSelectCell ( sSelectThisCell ) as boolean
    '///<b>This is just an alias function to avoid confusion -> see 'fCalcSelectRange'<b>
    fCalcSelectCell = fCalcSelectRange ( sSelectThisCell )
end function

'
'--------------------------------------------------------------------
'

function fCalcGetSelectionString() as String
   '///<b>Obtain range (e.g. 'A1:E10') or range name in 'formula bar'
   '///+RETURNS: STRING</b>///
    '///<ul>
    '///<li>Check if 'formula bar' is visible</li>///
    Kontext "RechenleisteCalc"
    if not RechenleisteCalc.isvisible then
        '///<li>-> Throw warning and Invoke 'formula bar' if invisible</li>///
        '///<li>-> NOTE: The testcase developer has to take care herself that function is entered correctly</li>///
        warnlog "Formula bar was expected to be visible"
        ViewToolbarsFormulaBar
    end if
    sleep(2)
    '///<li>Get and return string in range selection list box</li>///
    Kontext "RechenleisteCalc"
    fCalcGetSelectionString = Bereich.GetSelText
    '///</ul>
end function

'
'--------------------------------------------------------------------
'

function fCalcSelectSheet ( sSelectThisSheet ) as boolean

   '///<b>Select a sheet by name (STRING) or number (INTEGER)///
   '///+RETURNS: boolean (success)</u>///
   fCalcSelectSheet = FALSE
   '///<ul>
   try
        Kontext "DocumentCalc"
        '///<li>Invoke Edit::Sheet::Select</li>///
        EditSheetSelect
        Kontext "SelectSheets"
        '///<li>Select sheet no. or sheet name in listbox</li>///
        SheetSelectionBox.Select ( sSelectThisSheet )
        '///<li>OK</li>///
        SelectSheets.OK
        '///<li>Return true</li>///
        fCalcSelectSheet = TRUE
   catch
        warnlog "Something went wrong while selecting a sheet by name"
        '///<li>In case of failture lookup 'Select sheets' dialog andd close it if exists</li>///
        Kontext "SelectSheets"
        if SelectSheets.exists(2) then
            SelectSheets.Cancel
        end if
    endcatch
    '///</ul>
end function
'
'-------------------------------------------------------------------------
'
function fNavigatorSelectObject ( sWhatObject as STRING , iObjectPosition as INTEGER )
    '///<b>Select any Object in Navigator</b>
    '///<b>STRING: sWhatObject</b> - A keyword to choose desired obeject category
    '///+Posible values: sheets, range names, database ranges, linked areas, graphics, ole objects, notes, drawing objects
    '///<b>INTEGER: iObjectPosition</b> - Position of object (>=1) within 'sWhatObject'
    '///+<b>NOTE:</b>You're moving within a treelistbox which is quite messy to handle.
    '///+In case 'iObjectPosition' is larger than the real number of objects in category you're end up in another categories.
    '///+<b>WITHOUT ANY WARNING!</b>
    '///<b>RETURNS:</b> BOOLEAN (success)</b>
    '///-----
    '///<ul>
    dim bNavigatorWasVisible as boolean
        bNavigatorWasVisible = FALSE
    dim iIndex
    dim iCategoryPosition as INTEGER
    
    fNavigatorSelectObject = FALSE
    '///<li>Translate 'sWhatObject' to postion number of desired object category</li>///
    select case lcase ( sWhatObject )
           case "sheets"           : iCategoryPosition = 1
           case "range names"      : iCategoryPosition = 2
           case "database ranges"  : iCategoryPosition = 3
           case "linked areas"     : iCategoryPosition = 4
           case "graphics"         : iCategoryPosition = 5
           case "ole objects"      : iCategoryPosition = 6
           case "notes"            : iCategoryPosition = 7
           case "drawing objects"  : iCategoryPosition = 8
           case else
                warnlog "Invalid keyword -> Aborting fNavigatorSelectObject()"
                exit function
    end select
    '///<li>Check if Navigator is visible</li>///
    Kontext "NavigatorCalc"
    if NavigatorCalc.exists (3) then
    '///<li>Remember state of visibiliy TRUE/FALSE</li>///
        bNavigatorWasVisible = TRUE
    else
        '///<li>Invoke navigator if not visible</li>///
        ViewNavigator
    end if
    
    try 
        Kontext "NavigatorCalc"
        '///<li>Go to Top in Navigator</li>///
        liste.TypeKeys "<HOME>"
        '///<li>Travel top to bottom through all categories</li>///
        for iIndex = 1 to 8
        '///<li>Make sure all elements in category are hidden, apply '-' key</li>///
             liste.TypeKeys "-<DOWN>"
        next iIndex
        '///<li>Select desired category</li>///
        liste.select ( iCategoryPosition )
        '///<li>Unfold elements of selected category</li>///
        liste.TypeKeys "+"
        '///<li>Select desired position within category</li>///
        liste.select ( iCategoryPosition + iObjectPosition )
        '///<li>Hit 'RETURN' key to select element in document</li>///
        liste.TypeKeys "<RETURN>"
        '///<li>Return 'TRUE' for 'fNavigatorSelectObject'</li>///
        fNavigatorSelectObject = TRUE
    catch
        warnlog "Selecting the desired object failed. Perhaps your input wasn't valid"
    endcatch
    '///<li>Close navigator if it wasn't visible before entering this function</li>///
    if bNavigatorWasVisible = TRUE then
        printlog "Leaving navigator open as initially found"
    else
        ViewNavigator
        printlog "Closing navigator as initially found"
    end if
    '///</ul>
end function

