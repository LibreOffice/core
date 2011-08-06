'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
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
'/******************************************************************************
'*
'* Owner : thorsten.bosbach@oracle.com
'*
'* short description : wrappers for accessing menues (context-, bar-, button- menues)
'*
'\******************************************************************************

function hMenuItemGetCount as Integer

    '/// OUTPUT: number of entries in the activated menu (without seperators) ///'
    '///+ menu left open ///'
    dim iCurrentMenuItem as integer
    dim iValidMenuItems as integer

    sleep( 2 )

    iValidMenuItems = 0
    for iCurrentMenuItem = 1 to MenuGetItemCount
        if ( NOT MenuIsSeperator ( iCurrentMenuItem ) ) then
            iValidMenuItems = iValidMenuItems + 1
        endif
    next iCurrentMenuItem
    hMenuItemGetCount = iValidMenuItems

end function

'*******************************************************************************

function hMenuSelectNr ( EintragsNr as Integer ) as String

    '/// Selects the given nr. of <b>enabled</b> menu entry.
    '/// <u>Note</u>: If there are disabled menu items are included the routine ignores them!
    '///INPUT : number of entry from the top (>=1) OR
    '///INPUT : number of entry from the bottom (<=-1)
    '///INPUT : 0 to close menu
    '///+ OUTPUT: text of the entry

    Dim i, j, RealEnd, Ende as Integer
    Dim nID as long

    sleep( 2 )

    Ende = EintragsNr
    RealEnd = MenuGetItemCount
    If (Ende > 0) then
        if Ende > RealEnd then
            Warnlog "hMenuSelectNr canceled: Entry number "+ Ende +" does not exists, because there are only " + RealEnd + "  entries! "
            hMenuClose
            exit function
        endif
        j=0
        for i=1 to Ende
            if MenuIsSeperator ( i+j ) then
                j = j+1
            endif
        next i
    else
        if ((RealEnd + Ende) < 0) then
            Warnlog "hMenuSelectNr canceled: Entry number "+ Ende +" does not exists, because there are only " + RealEnd + "  entries! "
            hMenuClose
            exit function
        endif
        j=1
        for i=-1 to Ende step -1
            if MenuIsSeperator (RealEnd +i+j) then
                j = j -1
            endif
        next i
        j = j + RealEnd
    endif
    if EintragsNr = 0 then
        hMenuClose
        exit function
    endif
    nID = MenuGetItemID ( EintragsNr+j )
    hMenuSelectNr = MenuGetItemText ( nID )
    MenuSelect ( nID )

end function

'*******************************************************************************

function hMenuItemCheck ( EintragsNr as Integer ) as String

    '/// check the menu item (if it is checked, it stays checked) ///'
    '///+ INPUT : number of entry from the top (>=1)///'
    '///+ OUTPUT: text of the entry ///'
    Dim i as Integer, j as Integer, RealEnd as Integer, Ende as Integer, nID as Integer

    Ende = EintragsNr
    RealEnd = hMenuItemGetCount

    if Ende > RealEnd then
        Warnlog "hMenuItemCheck canceled: Entry number "+ Ende +" does not exists, because there are only " + RealEnd + "  entries! "
        hMenuClose
        exit function
    endif
    j=0
    for i=1 to Ende
        if MenuIsSeperator ( i+j ) then
            j = j+1
        endif
    next i
    nID = MenuGetItemID ( EintragsNr+j )
    hMenuItemCheck = MenuGetItemText ( nID )
    if NOT MenuIsItemChecked ( nID ) then
        MenuSelect ( nID )
    else
        hMenuClose()
    endif

end function

'*******************************************************************************

function hMenuItemUnCheck ( EintragsNr as Integer ) as String

    '/// UNcheck the menu item (if it is UNchecked, it stays UNchecked) ///'
    '///+ INPUT : number of entry from the top (>=1)///'
    '///+ OUTPUT: text of the entry ///'
    Dim i, j, RealEnd, Ende, nID as Integer

    Ende = EintragsNr
    RealEnd = hMenuItemGetCount
    if Ende > RealEnd then
        Warnlog "hMenuItemUnCheck canceled: Entry number "+ Ende +" does not exists, because there are only " + RealEnd + "  entries! "
        hMenuClose
        exit function
    endif
    j=0
    for i=1 to Ende
        if MenuIsSeperator ( i+j ) then
            j = j+1
        endif
    next i
    nID = MenuGetItemID ( EintragsNr+j )
    hMenuItemUnCheck = MenuGetItemText ( nID )
    if MenuIsItemChecked ( nID ) then
        MenuSelect ( nID )
    else
        hMenuClose()
    endif

end function

'*******************************************************************************

function hMenuFindSelect(MenuID as variant, SelectSlot as Boolean, ExpectedNr as integer, optional VerboseWrongPosition as Boolean) as Boolean

    '/// Find specified MenuID in Menu, check if its found on the expected place, ///'
    '/// if not - report the correct slot, and if specified to do so - Select the correct slot. ///'
    '///+ INPUT: MenuID: the string-name of the menu-entry we are lookin for. ///'
    '///+ INPUT: TRUE, if the slot should be called, ///'
    '///+            FALSE, if the slot shouldn't be called. ///'
    '///+ INPUT: Optional: TRUE, if errors should be reported. ///'
    '///+                           FALSE, silent mode for errors. ///'
    '///+ INPUT: Optional: The Expected Number of entry from the top (>=1) ///'
    '///+ OUTPUT: If not found at the expected place - the correct Entry-number is reported ///'

    Dim i as Integer
    Dim RealId as Integer
    Dim RealPosition as Integer
    Dim MenuIDAsNumber as integer
    Dim MenuIDAsString as string
    Dim RealEnd as Integer
    Dim nID as string
    Dim found as string
    Dim StringType as Boolean

    '/// find out if we are going to search for an ID, or a string (.uno:) ///'
    if IsNumeric(MenuID) then
        MenuIDAsNumber = MenuID
        StringType = FALSE
    else
        MenuIDAsString = MenuID
        StringType = TRUE
    endif

    RealEnd = MenuGetItemCount
    if ExpectedNr > RealEnd then
        Warnlog "Entry number "+ ExpectedNr +" is higher than the number of entries (" + RealEnd + ") in this menu! "
    endif

    if StringType = FALSE then  'Search for the number.

        nID = MenuGetItemID ( ExpectedNr )
        if nID <> MenuID then
            if VerboseWrongPosition = TRUE then
                warnlog " the questioned ID (" + MenuID + ") were not found on position " + ExpectedNr
            endif
            Found = FALSE
            for i=1 to RealEnd
                nID = MenuGetItemID (i)
                printlog "For the position '" + i + "', the ItemCommand was: " + MenuGetItemCommand (MenuGetItemID (i)) + " and the ID was: " + MenuGetItemID (i) + "."
                if nID = MenuID then
                    Found = TRUE
                    RealID = MenuGetItemID (i)
                    RealPosition = i
                    if VerboseWrongPosition = TRUE then
                        warnlog "The real Position (Separators included) was: " + i
                    endif
                    i = RealEnd
                endif
            next i
        else
            Found = TRUE
            RealPosition = ExpectedNr
            RealId = MenuGetItemID (ExpectedNr)
        endif

        if Found = TRUE then
            if SelectSlot then
                MenuSelect (RealID)
                hMenuFindSelect = TRUE
            else
                printlog "the slot was found, but the tester had choosen NOT to open the slot"
                hMenuFindSelect = TRUE
                hMenuClose
            endif
        else
            if VerboseWrongPosition = TRUE then
                warnlog "The slot was NOT found. Has either been removed, or it's a bug."
            endif
            hMenuFindSelect = FALSE
            hMenuClose
        endif

    else 'StringType = TRUE   :   Seach for the .uno; -name.
        nID = lCase(MenuGetItemCommand (MenuGetItemID (ExpectedNr)))

        if nID <> lCase(MenuID) then
            if VerboseWrongPosition = TRUE then
                warnlog " the questioned ID (" + MenuID + ") were not found on position " + ExpectedNr
            endif
            Found = FALSE
            for i=1 to RealEnd
                nID = MenuGetItemCommand (MenuGetItemID (i))
                printlog "For the position '" + i + "', the ItemCommand was: " + nID + " and the ID was: " + MenuGetItemID (i) + "."
                if nID = MenuID then
                    Found = TRUE
                    RealID = MenuGetItemID (i)
                    RealPosition = i
                    if VerboseWrongPosition = TRUE then
                        warnlog "The real Position (Separators included) was: " + i
                    endif
                    i = RealEnd
                else
                    if ( i = ExpectedNr ) then
                        if nID = "" then
                           warnlog "#i115132#Missing .uno Slots in context menus"
                        endif
                    endif
                endif
            next i
        else
            Found = TRUE
            RealPosition = ExpectedNr
            RealId = MenuGetItemID (ExpectedNr)
        endif
        if Found = TRUE then
            if SelectSlot then
                MenuSelect (RealID)
                hMenuFindSelect = TRUE
            else
                printlog "the slot was found, but the tester had choosen NOT to open the slot"
                hMenuFindSelect = TRUE
                hMenuClose()
            endif
        else
            if VerboseWrongPosition = TRUE then
                warnlog "The slot was NOT found. Has either been removed, or it's a bug."
            endif
            hMenuFindSelect = FALSE
            hMenuClose()
        endif
    endif 'StringType = TRUE
    wait 500   'Wait 0,5 seconds in case we just have called a Submenu.

end function

'*******************************************************************************

function hMenuItemIsChecked ( EintragsNr as Integer ) as Boolean

    '/// is the the menu item checked ? ///'
    '///+ INPUT : number of entry from the top (>=1)///'
    '///+ OUTPUT: True = yes!, False = no! ///'

    Dim i, j, Ende, RealEnd, nID as Integer

    Ende = EintragsNr
    RealEnd = hMenuItemGetCount
    if Ende > RealEnd then
        Warnlog "hMenuItemIsChecked canceled: Entry number "+ Ende +" does not exists, because there are only " + RealEnd + "  entries! "
        hMenuClose
        exit function
    endif
    j=0
    for i=1 to Ende
        if MenuIsSeperator ( i+j ) then
            j = j+1
        endif
    next i
    nID = MenuGetItemID ( EintragsNr + j )
    hMenuItemIsChecked = MenuIsItemChecked ( nID )

end function

'*******************************************************************************

function hMenuItemIsEnabled ( EintragsNr as Integer ) as Boolean

    '/// is the the menu item enabled (not greyed out) ? ///'
    '///+ INPUT : number of entry from the top (>=1)///'
    '///+ OUTPUT: True = yes!, False = no! ///'
    '///+ menu left open ///'

    Dim i, j, Ende, RealEnd, nID as Integer

    Ende = EintragsNr
    RealEnd = MenuGetItemCount
    if Ende > RealEnd then
        Warnlog "hMenuItemIsEnabled canceled: Entry number "+ Ende +" does not exists, because there are only " + RealEnd + "  entries! "
        hMenuClose
        exit function
    endif
    j=0
    for i=1 to Ende
        if MenuIsSeperator ( i+j ) then
            j = j+1
        endif
    next i
    nID = MenuGetItemID ( EintragsNr + j )
    hMenuItemIsEnabled = MenuIsItemEnabled ( nID )

end function

'*******************************************************************************

function hMenuItemGetText ( EintragsNr as Integer ) as String

    '///+ INPUT : number of entry from the top (>=1)///'
    '///+ OUTPUT: text of item without tilde ///'
    '///+ menu left open ///'

    Dim i, j, RealEnd, Ende, nID as Integer

    Ende = EintragsNr
    RealEnd = MenuGetItemCount
    if Ende > RealEnd then
        Warnlog "hMenuItemGetText canceled: Entry number "+ Ende +" does not exists, because there are only " + RealEnd + "  entries! "
        hMenuClose
        exit function
    endif
    j=0
    for i=1 to Ende
        if MenuIsSeperator ( i+j ) then
            j = j+1
        endif
    next i
    nID = MenuGetItemID ( EintragsNr + j )
    hMenuItemGetText = hhEntferneTilde ( MenuGetItemText ( nID ) )

end function

'*******************************************************************************

sub hMenuClose

    '/// close menue ///'
    '///+ after a hMenuClose you have to execute Kontext.UseMenu again
    '///+ if you want to open any other menu.
    MenuSelect ( 0 )

end sub

'*******************************************************************************

function hhEntferneTilde ( sEintrag as String ) as String

    '///+ INPUT : string with tilde ///'
    '///+ OUTPUT: string without tilde ///'

    Dim i
    i = Instr ( 1, sEintrag, "~", 1 )
    if i <> 0 then
        hhEntferneTilde = Left ( sEintrag, i-1 ) + Right ( sEintrag, ( Len(sEintrag) - i ) )
    else
        hhEntferneTilde = sEintrag
    endif

end function

'*******************************************************************************

function hMenuGetItemCommand ( EintragsNr as Integer ) as String

    '/// <u>hMenuGetItemCommand(Nr)</u> ///
    '///+ To read the slot/UNO slot which is behind a menu item. ///
    '///+ Seperators will be ignored for the input INTEGER!

    Dim i as integer
    Dim j as integer
    Dim RealEnd as integer
    Dim DasEnde as integer
    Dim nID as Integer
    DasEnde = EintragsNr
    RealEnd = MenuGetItemCount
    if DasEnde > RealEnd then
        warnlog "Parameter (" & DasEnde & ") bigger than items in menu (" & RealEnd & ") , 'hMenuGetItemCommand' canceled."
    else
        j=0
        for i=1 to DasEnde
            if MenuIsSeperator ( i+j ) then
                j = j+1
            endif
        next i
        nID = MenuGetItemID ( EintragsNr+j )
        hMenuGetItemCommand = MenuGetItemCommand ( nID )
    endif

end function

'*******************************************************************************

function hMenuGetItemId ( EintragsNr as Integer ) as integer

    '///+ To read the Slot ID which is behind a menu item. ///
    '///+ Seperators will be ignored for the input INTEGER!

    Dim i as integer
    Dim j as integer
    Dim RealEnd as integer
    Dim DasEnde as integer
    DasEnde = EintragsNr
    RealEnd = MenuGetItemCount
    if DasEnde > RealEnd then
        warnlog "Parameter (" & DasEnde & ") bigger than items in menu (" & RealEnd & ") , 'hMenuGetItemId' canceled."
    else
        j=0
        for i=1 to DasEnde
            if MenuIsSeperator ( i+j ) then
                j = j+1
            endif
        next i
        hMenuGetItemId = MenuGetItemID ( EintragsNr+j )
    endif

end function

'*******************************************************************************

'/// Functions for the lower part of the 'Window' Menu ///'
'///+ philosophie: you can't do any actions on the first document -> it get's the number 0 and it doesn't count ///'
'///+ so the first window you can work with gets the number 1 and counting starts with 1 ... ///'
'///<b> function hUseMenu () </b>///'
'///+ very global function: depends on 'gApplication' just activates the menu-bar ///'
'///<b> function hWindowGetIndex () as integer </b>///'
'///+ returns the number of the active window ///'
'///<b> function hWindowGetText (optional iWindow as integer) as string </b>///'
'///+ returns the text of the menu entry without the tilde: ///'
'///+ w/ calling parameter: the active window ///'
'///+ w/o calling parameter: the window i ///'
'///<b> function hWindowSelect (iWindow as integer) </b>///'
'///+ switches to the window i ///'
'///+ calling parameter: id of the window to activate ///'

'*******************************************************************************

function hWindowGetIndex () as integer

    '///<b> function hWindowGetIndex () as integer </b>///'
    '///+ returns the number of the active window ///'

    dim iMenues as integer      ' takes the number of entries in the windows menue
    dim iNotWindows as integer  ' number of open windows
    dim iKandidate as integer
    dim i as integer

    iNotWindows = GetDocumentCount()
    hUseMenu()

    iMenues = hMenuItemGetCount
    hMenuSelectNr(iMenues-1)    ' 'windows' ist usually the one before the last
    waitslot() 'wait 200
    iKandidate = 0
    iMenues = hMenuItemGetCount
    for i = ((iMenues - iNotWindows) + 1) to iMenues
        if (hMenuItemIsChecked (i)) then
            if (iKandidate = 0) then
                iKandidate = i
            else
                Warnlog "unexpected error 4711 - more than one window active??!! hWindowGetIndex () "
            endif
        endif
    next i
    iMenues = (iKandidate - (iMenues - iNotWindows) - 1)
    hMenuClose()

    hWindowGetIndex = iMenues

end function

'*******************************************************************************

function hWindowGetText (optional iWindow as integer) as string

    '///<b> function hWindowGetText (optional iWindow as integer) as string </b>///'
    '///+ returns the text of the menu entry without the tilde: ///'
    '///+ w/ calling parameter: the active window ///'
    '///+ w/o calling parameter: the window i ///'

    dim iMenues as integer      ' takes the number of entries in the windows menue
    dim iNotWindows as integer  ' number of open windows
    dim iUseWindow

    if (isMissing(iWindow) = FALSE) then
        iUseWindow = iWindow
    else
        iUseWindow = hWindowGetIndex()
    endif

    iNotWindows = GetDocumentCount()
    hUseMenu()

    iMenues = hMenuItemGetCount
    hMenuSelectNr(iMenues-1)    ' 'windows' ist usually the one before the last

    waitslot() 'wait 200
    iMenues = hMenuItemGetCount
    hWindowGetText = hMenuItemGetTextMitTilde (iUseWindow + (iMenues - iNotWindows) + 1)
    hWindowGetText = hhEntferneTilde(hWindowGetText)
    hMenuClose()

end function

'*******************************************************************************

function hWindowSelect (iWindow as integer)

    '///<b> function hWindowSelect (iWindow as integer) </b>///'
    '///+ switches to the window i ///'
    '///+ calling parameter: id of the window to activate ///'

    dim iMenues as integer      ' takes the number of entries in the windows menue
    dim iNotWindows as integer  ' number of open windows

    iNotWindows = GetDocumentCount ()
    hUseMenu()

    'ActivateDocument iNumber as integer ' is a testtool command, but there is no system that stays behind iNumber :-[
    iMenues = hMenuItemGetCount
    hMenuSelectNr(iMenues-1)    ' 'windows' ist usually the one before the last
    WaitSlot() 'wait 200

    iMenues = hMenuItemGetCount
    hMenuSelectNr (iWindow + 1 + (iMenues - iNotWindows))

end function

'*******************************************************************************

function hUseMenu ()

    '///<b> function hUseMenu () </b>///'
    '///+ very global function: depends on 'gApplication' just activates the menu-bar ///'

    Select Case ( gApplication )
    Case "WRITER"

        Kontext "DocumentWriter"
        DocumentWriter.UseMenu

    Case "MASTERDOCUMENT"

        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.UseMenu

    Case "HTML"

        Kontext "DocumentWriterWeb"
        DocumentWriterWeb.UseMenu

    case "CALC" ' there are 2 entries over :-( usually only one, html 0 :-((((

        Kontext "DocumentCalc"
        DocumentCalc.UseMenu

    case "DRAW"

        Kontext "DocumentDraw"
        DocumentDraw.UseMenu

    case "IMPRESS"

        Kontext "DocumentImpress"
        DocumentImpress.UseMenu

    case "MATH" :

        Kontext "DocumentMath"
        DocumentMath.UseMenu

    case "DATABASE" :

        Kontext "DATABASE"
        Database.UseMenu

    case "BASIC" :

        Kontext "BasicIde"
        BasicIde.UseMenu

    case "NONE" :

        Kontext "DocumentBackground"
        DocumentBackground.UseMenu

    end select

end function

'*******************************************************************************

function hOpenContextMenu()

    '/// very global function: depends on 'gApplication' just opens the Context Menu ///'
    Select Case ( gApplication )
    Case "WRITER"

        Kontext "DocumentWriter"
        DocumentWriter.openContextMenu

    Case "MASTERDOCUMENT"

        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.openContextMenu

    Case "HTML"

        Kontext "DocumentWriterWeb"
        DocumentWriterWeb.openContextMenu

    case "CALC"

        Kontext "DocumentCalc"
        DocumentCalc.openContextMenu

    case "DRAW"

        Kontext "DocumentDraw"
        DocumentDraw.openContextMenu

    case "IMPRESS"

        Kontext "DocumentImpress"
        DocumentImpress.openContextMenu

    case "MATH" :

        Kontext "DocumentMath"
        DocumentMath.openContextMenu

    end select

end function

'*******************************************************************************

function hGetWindowCaption( sApplication as string, optional bAll as boolean )

    '///<b> function hGetWindowCaption () </b>///'
    '///+ just returns the string from the application main window ///'
    '///+ If optional parameter = true, thenn go throught all applications and get caption; return array ///'

    ' BEWARE: THIS FUNCTION MODIFIES ITS FUNCTION PARAMETER UNDER CERTAIN
    '         CONDITIONS!

    dim sTemp as string
    dim aApplication() as string
    dim aTemp(10) as string
    dim iTimes as integer
    dim i as integer

    aApplication = array("SWRITER","SGLOBAL","SWEB","SCALC","SDRAW","SIMPRESS","SMATH","BASIC","DATABASE","SOFFICE")
    if (isMissing(bAll) OR (NOT bAll)) then
        iTimes = 0
    else
        iTimes = uBound(aApplication())
        sApplication = aApplication(0)
    endif
    for i=0 to iTimes
        Select Case Ucase(sApplication)
            Case "SWRITER"
                Kontext "DocumentWriter"
                try
                    sTemp = DocumentWriter.caption
                catch
                endcatch
            Case "SGLOBAL"
                Kontext "DocumentMasterDoc"
                try
                    sTemp = DocumentMasterDoc.caption
                catch
                endcatch
            Case "SWEB"
                Kontext "DocumentWriterWeb"
                try
                    sTemp = DocumentWriterWeb.caption
                catch
                endcatch
            case "SCALC"
                Kontext "DocumentCalc"
                try
                    sTemp = DocumentCalc.caption
                catch
                endcatch
            case "SDRAW"
                Kontext "DocumentDraw"
                try
                    sTemp = DocumentDraw.caption
                catch
                endcatch
            case "SIMPRESS"
                Kontext "DocumentImpress"
                try
                    sTemp = DocumentImpress.caption
                catch
                endcatch
            case "SMATH"
                Kontext "DocumentMath"
                try
                    sTemp = DocumentMath.caption
                catch
                endcatch
            Case "BASIC"
                Kontext "BasicIDE"
                try
                    sTemp = BasicIDE.caption
                catch
                endcatch
            Case "DATABASE"
                Kontext "DATABASE"
                try
                    sTemp = DataBase.caption
                catch
                endcatch
            Case "SOFFICE"
                Kontext "DocumentBackground"
                try
                    sTemp = DocumentBackground.caption
                catch
                endcatch
            case else
                qaErrorLog "sApplication not defined: '" + sApplication + "'"
                sTemp = ""
        end select
        aTemp(i) = sTemp
        try
            sApplication=aApplication(i+1)
        catch
        endcatch
    next i
    if (iTimes > 0) then
        hGetWindowCaption = aTemp()
    else
        hGetWindowCaption = sTemp
    endif

end function

'*******************************************************************************

function fSplitWindowTitle( sInString as string )

    dim a, b, x, i, iProductname as integer
    dim sTemp() as string
    dim sProductname() as string
    dim sTempReturn(2) as string
    dim sTempReturn2(3) as string
    dim bTestToolCommunication as boolean
    dim bDebugVersion as boolean ' aka 'nonpro'
    dim bError as boolean

    ' a) 'Untitled2 - OpenOffice.org Draw' (2)
    ' b) 'Untitled1 - OpenOffice.org Writer TTD :localhost[1034]' (4)
    ' c) 'Untitled1 - StarOffice Writer [680m72(Build:8863)]' (3)
    ' d) 'Untitled2 - StarOffice Draw [680m72(Build:8863)] TTD :localhost[1034]' (5)

    bTestToolCommunication = FALSE
    bDebugVersion = FALSE
    bError = FALSE
    if (sInString <> "") then
        ' ' - ' separates Filename from Vendor
        a = instr(sInString, " - ") ' get Document file name or Untitledi
        if (a > 1) then
            if (instr(a+1,sInString, " - ") > a) then
                qaErrorLog "Argh!"
                bError = TRUE
            endif
            sTempReturn(0) = left(sInString, a-1)                        ' (0) Filename
        else
            sTempReturn(0) = ""
            bError = TRUE
        endif

        ' seperate second string by spaces
        sTemp() = split(right(sInString, len(sInString)-(a+2)), " ") ' Parts are now separated by spaces
        x = uBound(sTemp())
        sProductname() = split(gProductname, " ")
        iProductname = uBound(sProductname())
        x = x - iProductname

        sTempReturn(1) = sTemp(uBound(sTemp()))                        ' (1) Application
        sTempReturn(2) = gProductname 'sTemp(0)                                        ' (2) Vendor

        ' if ']' at end... (b,c,d)
        if (right(sInString,1) = "]") then
            ' if ')' at second last position it is a nonpro!
            if (right(sInString,2) = ")]") then
                ' normal detection works with spaces... (c)
                bDebugVersion = TRUE
            else
                ' remove TestTool communication string (b,d)
                ' it is the last 2 space seperated parts!
                bTestToolCommunication = TRUE
                if (left(sTemp(2),1)="[") then
                    bDebugVersion = TRUE
                endif
            endif
        else
            ' normal detection with spaces works... (a)
        endif

        ' sanity checks, to warn if assumptions are not met!
        select case x
        case 1:
            if ((bTestToolCommunication) OR (bDebugVersion)) then
                qaErrorLog "t_menu.inc::fSplitWindowTitle: Unknown count of Window Title Strings! 1"
                bError = TRUE
            endif
        case 2:
            if ((bTestToolCommunication) OR (NOT bDebugVersion)) then
                qaErrorLog "t_menu.inc::fSplitWindowTitle: Unknown count of Window Title Strings! 2"
                bError = TRUE
            endif
        case 3:
            if ((NOT bTestToolCommunication) OR (bDebugVersion)) then
                qaErrorLog "t_menu.inc::fSplitWindowTitle: Unknown count of Window Title Strings! 3"
                bError = TRUE
            endif
        case 4:
            if ((NOT bTestToolCommunication) OR (NOT bDebugVersion)) then
                qaErrorLog "t_menu.inc::fSplitWindowTitle: Unknown count of Window Title Strings! 4"
                bError = TRUE
            endif
        case else:
            qaErrorLog "t_menu.inc::fSplitWindowTitle: Unknown count of Window Title Strings! 0"
            bError = TRUE
        end select

        if bError then printlog sInString
        
        if (x > 1) then ' there is more to evaluate
            ' save the current computed values
            for i=0 to 2
                sTempReturn2(i) = sTempReturn(i)
            next i
            ' make return array bigger
            redim sTempReturn(4) as string
            ' copy data back
            for i=0 to 2
                sTempReturn(i) = sTempReturn2(i)
            next i
            ' set remaining values, if exist
            ' debug version string
            if bDebugVersion then
                sTempReturn(3) = mid(sTemp(2),2,len(sTemp(2))-2) ' optional (3) Debug Version
            else
                sTempReturn(3) = ""
            endif
            ' TestTool Communication string
            if bTestToolCommunication then
                ' if true, then always the two last strings
                sTempReturn(4) = sTemp(x-1) + " " + sTemp(x)        ' optional (4) TestTool Communication
            else
                sTempReturn(4) = ""
            endif
        endif
    else
        redim sTempReturn(0) as string
        sTempReturn(0) = ""
    endif

    fSplitWindowTitle = sTempReturn()

end function

