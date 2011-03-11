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
'* Owner : wolfram.garten@oracle.com
'*
'* short description : some tools (Functions)
'*
'\******************************************************************************

function hFindSpellHypLanguage (optional sBooks()) as string
    printlog "print all available languages that have a language module"
    dim iListLength as integer
    dim i as integer
    dim sTemp as string

    printlog "only necessary for asian languages"
    if (bAsianLan or (iSprache=55)) then
        printlog "Tools->Options"
        ToolsOptions
        printlog "select from section 'Language Settings' the item 'Writing Aids'"
        hToolsOptions ("LANGUAGESETTINGS","WRITINGAIDS")
        printlog "click button 'Edit...' in section 'Available language modules'"
        SprachmoduleBearbeiten.click
        kontext "ModuleBearbeiten"
        printlog "print all entries from listbox 'Language'"
        for i = 1 to Sprache.GetItemCount
            sTemp = Sprache.GetItemText(i)
            if (NOT isMissing(sBooks())) then
                listAppend(sBooks(), sTemp)
            endif
            printlog " return the first entry in the listbox "
            if i = 1 then hFindSpellHypLanguage = sTemp
        next i
        printlog "close dialog 'Edit Modules'"
        ModuleBearbeiten.Close
        Kontext "ExtrasOptionenDlg"
        printlog "close dialog 'Options - '"
        hCloseDialog( ExtrasOptionenDlg, "ok" )
    endif
end function

'-------------------------------------------------------------------------------
function GetDecimalSeperator ( sDummy$ ) as String

    printlog "Input : number with fractionmark from 'NumericField' as String "
    printlog "+ Output: '.' or ',' as String                                  "
    dim i1, i2 as integer

    printlog "get position of fraction mark / get IT"
    i1 = instr (sDummy$, ",")
    i2 = instr (sDummy$, ".")
    if i1 > i2 then GetDecimalSeperator = "," else GetDecimalSeperator = "."
end function

'-------------------------------------------------------------------------------
function LiberalMeasurement ( sShould$, sActual$) as Boolean

    printlog " Input : (1. Should, 2. Actual) as Number with or without MeasurementUnit 'NumericField' as String "
    printlog "+          if input has no MeasurementUnit i take it as 'cm' (was the default in old tests) "
    printlog "+ Output: Boolean are they likely the same?"
    printlog " NEEDED: mathematical proofment of iTolerance, by now just some guesses :-| "
    printlog " reason for this function:"
    printlog "+   because SO counts internaly in 'twip???s' 'twentieth of a point' there are some rounding errors "
    printlog "+ there are also some rounding errors because of the internal representatio of floating point numbers in computers "
    printlog "+   now lets try to get rid of them and have a nicer output in tests... "
    printlog " measurement units are defined in http://gsl.openoffice.org/source/browse/gsl/vcl/source/src/units.src "

    dim iTolerance as Double

    LiberalMeasurement = False

    if (sShould$ = sActual$) then
        LiberalMeasurement = True
    else
        printlog "check if measunit is the same"
        if (GetMeasUnit(sShould$) <> GetMeasUnit(sActual$) ) then
            warnlog "In function LiberalMeasurement the measUnit is different, compare not possible yet"
        else
            printlog "set factor for liberality"
            printlog "took units from http://gsl.openoffice.org/source/browse/gsl/vcl/source/src/units.src"
            select case GetMeasUnit(sShould$)
            case "mm", "ミリ", "公厘" : iTolerance = 2.0             '01, 81,     88
            case "cm","セン�?","厘米","公分"  : iTolerance = 0.5       '01, 81, 86, 88
            case chr$(34) : iTolerance = 2.5
            case "pi","ピクセル" : iTolerance = 2.5                 '01, 81
            case "pt",  "�?イント" : iTolerance = 2.5                '01, 81
            case "" : iTolerance = 1.5 ' cm is presubposition in old functions
            case else
                iTolerance = 2.5
                qaErrorLog "This Unit is not available in this function. '" + GetMeasUnit(sShould$) + "'"
            end select
            printlog "have to get the measurem unit, cause the offset is different for each"
            printlog "!!! val(str()) is important because of double calculating actions !!! #110996#"
            if ( val(str(StrToDouble(sShould$)+iTolerance)) >= StrToDouble(sActual$) ) AND (val(str(StrToDouble ( sShould$ )-iTolerance)) <= StrToDouble ( sActual$ )) then
                LiberalMeasurement = True
            else
                LiberalMeasurement = False
            end if
        end if
    end if
end function

'-------------------------------------------------------------------------------
function GetMeasUnit ( sWert$ ) as String

    dim iBounder as integer
    printlog " Input : Number with or without MeasurementUnit 'NumericField' as String "
    printlog "+ Output: Initials of MeasurementUnit as String or "" when only a number "

    iBounder = -1
    do
        inc iBounder
    loop until ( isNumeric(mid (sWert$, len(sWert$)-iBounder, 1)) OR (len(sWert$) <= (iBounder + 1)) )

    if (len(sWert$) <= (iBounder + 1)) then
        if isNumeric(left (sWert$, 1)) then
            GetMeasUnit = right (sWert$, iBounder)
        else
            GetMeasUnit = sWert$
        endif
    else
        GetMeasUnit = right (sWert$, iBounder)
    endif
end function

'-------------------------------------------------------------------------------
function StrToDouble ( sWert$ ) as Double

    Dim sDummy$
    dim i, i1, i2 as integer
    dim a as integer
    dim b as integer
    dim c as double
    dim n as integer

    printlog " Input : {'a[. ,]b[mm cm  pi pt]' with a, b as integer} as String "
    printlog "+ Output: a[. , ]b as double "

    printlog "get rid of measure unit, the only single character is '' all others are two chars"
    printlog "there was a problem, if there is NO meas.unit!!"
    if (isNumeric (sWert$) = FALSE) then
        if (  StrComp (right (sWert$, 1), chr$(34) ) = 0 ) then
            sDummy$ = Left ( sWert$, Len(sWert$)-1 )
        else
            sDummy$ = Left ( sWert$, Len(sWert$)-2 )
        endif
    else
        sDummy$ = sWert$
    endif
    printlog "get position of fraction mark"
    i1 = instr (sDummy$, ",")  ' wrong output
    i2 = instr (sDummy$, ".")
    if i1 > i2 then i = i1 else i = i2
    printlog " in front of decimal seperator:"
    try
        a = val (left (sDummy$,i-1))
    catch
        'printlog sWert$ + ":" + sDummy$ + ":" + i + ":" + i1+ ":" + i2
    endcatch
    printlog "after the decimal seperator"
    n = (len (sDummy$)-i)
    b = val (right (sDummy$, n) )
    c = b * 10 ^ -n
    'printlog "-------------- :"+sWert$ +" :'"+a+"' :"+n+" :"+b+" :'"+c+"':"
    ' !!! val(str()) is important because of double calculating actions !!! #110996#
    StrToDouble = val(str(a + c))
end function

'-------------------------------------------------------------------------------
function fGetPositionX () as string

    fGetPositionX = ""
    try
        ContextPositionAndSize

        kontext
        active.SetPage TabPositionAndSize
        kontext "TabPositionAndSize"
        if ( TabPositionAndSize.exists( 5 ) ) then
            fGetPositionX = PositionX.GetText()
            TabPositionAndSize.OK()
        else
            warnlog( "Couldn't switch to <TabPositionAndSize>" )
        endif
    catch
        warnlog "couldn't call 'ContextPositionAndSize' no object selected ?"
    endcatch

end function

'-------------------------------------------------------------------------------
function setStartCurrentPage(optional bState as boolean) as boolean

    printlog " tools->options "
    ToolsOptions
    printlog "+ select in section 'Presentation' tabpage 'general' "
    hToolsOptions ("IMPRESS","General")
    printlog "+ check the checkbox 'Always with current page' "
    setStartCurrentPage = MitAktuellerSeite.isChecked
    if bState then
        MitAktuellerSeite.Check
    else
        MitAktuellerSeite.UnCheck
    endif
    Kontext "ExtrasOptionenDlg"
    printlog "+ close dialog 'Options - Presenation - General' with OK "
    hCloseDialog( ExtrasOptionenDlg, "ok" )
end function

'-------------------------------------------------------------------------------
function fIsDocumentWritable() as boolean

    Kontext "Standardbar"
    if Bearbeiten.GetState( 2 ) <> 1 then
        fIsDocumentWritable = false
    else
        fIsDocumentWritable = true
    endif

end function

'-------------------------------------------------------------------------------
function fMakeDocumentWritable() as boolean

    printlog( "Remove write protection from current file" )

    Kontext "Standardbar"
    sleep ( 1 )
    if Bearbeiten.GetState(2) <> 1 then
        Bearbeiten.Click
        Kontext
        if Active.Exists(1) then
            Active.Yes
            fMakeDocumentWritable = true
        else
            warnlog "No messagebox after making document editable?"
            fMakeDocumentWritable = false
        endif
    else
        printlog "Document is already writable."
        fMakeDocumentWritable = true
    endif
    sleep(1)
end function

'-------------------------------------------------------------------------------
function fGetSizeXY (sX as string, sY as string, bGet as boolean) as Boolean
    dim sTx as string
    dim sTy as string
    dim bReturn as boolean

    bReturn = True
    try
        printlog "Trying to open Position and size Dialog.."
        ContextPositionAndSize
    catch
        warnlog "couldn't call 'ContextPositionAndSize' no object selected ?"
    endcatch
    kontext
    active.SetPage TabPositionAndSize
    kontext "TabPositionAndSize"
    printlog "Getting some sizes from Position and Size dialog."
    if TabPositionAndSize.exists (5) then
        sTx = Width.GetText
        printlog "Width, sTx=" & sTx
        sTy = Height.GetText
        printlog "Height, sTy=" & sTy
        TabPositionAndSize.OK
    else
        warnlog "Couldn't switch tab page :-( "
    endif
    if bGet then  ' Get the Values only
        sY = sTy
        printlog "sY=" & sY
        sX = sTx
        printlog "sX=" & sX
    else          ' Get the Values and COMPARE them
        if (LiberalMeasurement (sX,sTx) <> TRUE) then
            warnlog "width is different :-( XXXXXXXXXXXXX should: '"+sX+"' is: '"+sTx+"'" + "eventually a result of i35519"
            bReturn = False
        endif
        if (LiberalMeasurement (sY,sTy) <> TRUE) then
            warnlog "hight is different :-( xxxxxxxxxxxx should: '"+sY+"' is: '"+sTy+"'" + "eventually a result of i35519"
            bReturn = False
        endif
        bGet = bReturn
    endif
end function

'-------------------------------------------------------------------------

function hCallExport ( cFileName as String , sFilter as String, optional bSelection as boolean ) as Boolean

    const RC_FAILURE = -1
    dim bExportSelectionOnly as boolean

    hCallExport() = false
    
    printlog( "Exporting file with provided filter" )

    ' Handle infamous optional parameter
    if ( IsMissing( bSelection ) ) then 
        bExportSelectionOnly = false
    else
        bExportSelectionOnly = bSelection
    endif

    if ( hUseAsyncSlot( "FileExport" ) <> RC_FAILURE ) then

        Kontext "ExportierenDlg"
        if ( ExportierenDlg.exists( 3 ) ) then
            try
                Dateityp.Select sFilter

                if ( selektion.exists() ) then
                    if ( selektion.isEnabled() ) then
                        if ( bExportSelectionOnly ) then
                            selektion.check()
                        else
                            selektion.unCheck()
                        endif
                    else
                        printlog( "Cannot set <seletion>, it is disabled" )
                    endif
                else
                    if ( bExportSelectionOnly ) then
                        warnlog( "It was requested to export only the current selection but the option is disabled" )
                    endif
                endif

                AutomatischeDateinamenserweiterung.check()
                Dateiname.SetText( cFileName )
                Speichern.Click()

                kontext "AlienWarning"
                if AlienWarning.exists(5) then
                    warnlog "#i41983# Alien Warning on export not allowed"
                    hCloseDialog( AlienWarning, "ok" )
                endif

                Kontext "Active"
                hCloseDialog( Active, "yes,optional" )

                hCallExport = true
            catch
                warnlog( "Filter could not be selectd, it might be missing: " & sFilter )
                hCloseDialog( ExportierenDlg, "cancel" )
            endcatch
        else
            warnlog( "<ExportierenDlg> not open" )
        endif
    else
        warnlog( "Slot <FileExport> is blocked" )
    endif

end function

'-------------------------------------------------------------------------
function checkexppdfwaitmax10sec

    dim i as integer
    kontext "Standardbar"
    i = 0
    do
        i = i + 1
        sleep 1
        if (ExportAsPDF.isEnabled = TRUE) then i = 15
    loop while ((i < 15))
    if (ExportAsPDF.isEnabled = FALSE) then
        Warnlog "ExportAsPDF was NOT ok. Waited " + i + " seconds."
    endif
    sleep (3)
end function

'-------------------------------------------------------------------------------
function fCompareTwoValues(a as string, b as string) as boolean

    dim c as boolean

    c = val(str(StrToDouble(a))) <> val(str(StrToDouble(b)))
    c = c AND (GetMeasUnit(a) <> GetMeasUnit(b))
    fCompareTwoValues = c
end function

'-------------------------------------------------------------------------------
function fConvertBackslashToSlash (sInput as string) as string

    dim i as integer
    dim sTemp as string
    dim sI as string
    dim x as integer

    sTemp = ""
    x = len (sInput)
    for i = 1 to x
        sI = mid(sInput, i, 1)
        if (sI = "\") then
            sI = "/"
        endif
        sTemp = sTemp + sI
    next i
    fConvertBackslashToSlash = sTemp
end function

'-------------------------------------------------------------------------------
function hScreenFontAntialiasing (bEnable as boolean) as boolean

    ToolsOptions
    hToolsOptions ("STAROFFICE", "VIEW")
    hScreenFontAntialiasing = FontAntiAliasing.IsChecked
    if (bEnable) then
        FontAntiAliasing.Check
    else
        FontAntiAliasing.Uncheck
    endif
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
end function

'-------------------------------------------------------------------------------
function fSaveLoadAllFormats (NewFileDir as String)

    Dim iFileTypeCounter as Integer
    Dim SavedFile(30) as String
    Dim iCounter as Integer

    printlog "Save the document in different formats..."
    FileSaveAs
    kontext "ExportierenDlg"
    For iFileTypeCounter = 1 to Dateityp.GetItemCount
        sleep (1)
        if iFileTypeCounter > 1 then
            WaitSlot (2000)
            FileSaveAs
            kontext "ExportierenDlg"
        endif
        Dateiname.SetText (ConvertPath (NewFileDir) + "file" + iFileTypeCounter)
        Dateityp.Select (iFileTypeCounter)
        sleep (1)
        Printlog "   Saving file: " + (ConvertPath (NewFileDir) + ("file" + (iFileTypeCounter) + "." + left(right(Dateityp.GetSelText,4),3)))
        SavedFile(iFileTypeCounter) = ("file" + (iFileTypeCounter) + "." + left(right(Dateityp.GetSelText,4),3))
        Speichern.Click
        Kontext "Active"
        if Active.Exists(2) then Active.Yes ' File already exists, overwrite?
        'printlog " Saved file ( SavedFile(" + iFileTypeCounter + ") ) as: '" + SavedFile(iFileTypeCounter) +"'."
        Kontext "AlienWarning"
        if AlienWarning.Exists(2) then AlienWarning.OK
        kontext "DocumentImpress"
    Next iFileTypeCounter
    printlog "Close the file."
    FileClose

    printlog "Load the different files."
    iCounter = 0
    For iCounter = 1 to (iFileTypeCounter-1)
        Printlog "   Will try to open: " + (ConvertPath (NewFileDir) + SavedFile(iCounter))
        CALL hFileOpen(ConvertPath (NewFileDir) + SavedFile(iCounter))
        CALL hCloseDocument
        printlog "   Will try to delete: " + (ConvertPath (NewFileDir) + SavedFile(iCounter))
        app.Kill (ConvertPath (NewFileDir) + SavedFile(iCounter))
    Next iCounter

end function
'-------------------------------------------------------------------------------
function setCharacterLanguage(sLanguage as string) as boolean

    setCharacterLanguage = FALSE
    FormatCharacter
    WaitSlot (1000)
    Kontext
    Messagebox.SetPage TabFont
    kontext "TabFont"
    sleep 1
    printlog "sLanguage = " + sLanguage
    if (bAsianLan) then  'Eastern languages  'OR
        try
            printlog "LanguageWest.GetSelText = " + LanguageWest.GetSelText
            LanguageWest.select (sLanguage) 'East
        catch
            printlog "Language.GetSelText = " + Language.GetSelText
            Language.select (sLanguage) 'East
        endcatch

        setCharacterLanguage = TRUE
    elseif (iSprache = 07) then
        printlog "LanguageWest.GetSelText = " + LanguageWest.GetSelText
        LanguageWest.select (sLanguage)
    else
        try
            printlog "LanguageWest.GetSelText = " + LanguageWest.GetSelText
            LanguageWest.select (sLanguage)
        catch
            printlog "Language.GetSelText = " + Language.GetSelText
            Language.select (sLanguage)
        endcatch
        setCharacterLanguage = TRUE
    end if
    TabFont.Ok
    sleep 1
end function

'-------------------------------------------------------------------------------
function toggleGermanSpellchecking as string

    printlog " activate old german spellchecking "
    printlog "+ Tools->Options "
    ToolsOptions
    printlog "+ select tabpage 'writing aids' in category 'Languagesettings' "
    hToolsOptions("LANGUAGESETTINGS","WRITINGAIDS")
    kontext "TabLinguistik"
    printlog "+ hopefully it never changes for any reason between the languages!: select the 8th entry 'German spelling - old' "
    printlog " - 'German Spelling - old' ?= " + Optionen.getItemText(8)
    Optionen.select(8)
    printlog "+ default is 'unselected' - i can't check it automatically - so i depend on it! "
    printlog "+ press [space] to select it"
    Optionen.typeKeys "<space>"
    Kontext "ExtrasOptionenDlg"
    printlog "+ close options with OK button "
    ExtrasOptionenDlg.OK
end function

'-------------------------------------------------------------------------------
function sAnalyseContextMenu(iItems as integer, optional iError as long) as integer

    dim i as integer
    dim y as integer
    dim w as integer
    dim x as integer
    dim z as integer
    dim f as string
    dim iSlot as integer
    dim iSpecialCharacterEntry as integer
    dim bNoContextMenu as boolean
    dim iTemp as long
    dim sCandidates(5) as string
    dim bDifferent as boolean
    dim iInternError as long
    dim iError1 as long  ' misplaced
    'i22192: context menu opens not on cursor position
    dim iError2 as long  ' no context menu

    printlog "goto start of textbox "
    call hTypeKeys "<mod1 home>"
    printlog "for every word, check the context menu to get suggestions for correction "
    for i = 0 to (iItems-1)
        printlog " copy current word to clipboard "
        call hTypeKeys "<Shift mod1 right>"
        EditCopy
        sCandidates(1) = getClipboardText()
        if (" " = right(sCandidates(1),1)) then
            sCandidates(1) = left(sCandidates(1),len(sCandidates(1))-1)
        end if
        call hTypeKeys "<mod1 left>"
        printlog " open context menu "
        printlog "   About to call the ContextMenu."
        call hOpenContextMenu()
        sleep 3
        printlog "   Just opened ContextMenu."
        ' collecting criteria for underlining:
        ' 1st one: is word selected? yes: underlined;

        printlog " If the string vnd.sun.search:SubMenu (the SunSearch-menu) is found in the menu, we'll skip that word. "

        'Get first entry.
        f = MenuGetItemCommand (MenuGetItemID (1))
        printlog "f = '" + f + "'."
        'If it's "vnd.sun.search:SubMenu" , then skip the word. Printlog "Word not underlined, Search-Toolbar active."
        if f <> "vnd.sun.search:SubMenu" then
            try          ' WorkAround ##
                editcopy
                sCandidates(2) = getClipboardText()
                '            printlog "******************* " + getclipboardtext()
            catch
                sCandidates(2) = ""
                '            printlog "###################################################"
            endcatch
            ' if (1) is different from nonempty (2) then the wrong word is selected
            if (sCandidates(1) <> sCandidates(2)) then
                if ("" <> sCandidates(2)) then
                    '                printlog "############ " + sCandidates(1) + " ################## " + sCandidates(2) + " #####################"
                    bDifferent = TRUE
                    iError1 = iError1 + (2^i)
                else
                    bDifferent = false
                    ' no word is selected... a) not underlined b) no context menu open
                end if
            else
                '        printlog "******************* " + sCandidates(1)
                bDifferent = FALSE
            end if
            ' check if context menu opened
            try
                x = hMenuItemGetCount
                ' successfully opened context menu
                bNoContextMenu = false
            catch
                ' context menu not open
                bNoContextMenu = true
                iError2 = iError2 + (2^i)
                ' in writer it would work... :-( #i23568#
                '       warnlog ""+i+" C: " + x + ";------ " + getClipboardText + " -------- "
            endcatch
            ' if context menu open do....
            if (not bNoContextMenu) then
                '        printlog ""+i+" C: " + x + ";------ " + getClipboardText + " -------- " + hMenuItemGetText(1)
                printlog " analyze context menu entries "
                for y = 1 to x
                    z = hMenuGetItemId(y)
                    if (1 = y) then iSlot = z  ' criteria for WorkAround
                    if (z = 27019) then iSpecialCharacterEntry = y    ' entry to select for WorkAround
                    Printlog ("---i: "+ y +"; " + z + "  ; " +hMenuItemGetText(y) + " ; " + hMenuGetItemCommand(y))
                next y
                printlog " if first slot not a spelling suggestion -> WorkAround 112919 "
                printlog " close Context Menu "
                if (iSlot <> 10456) then
                    if (not bDifferent) then  ' WorkAround ##
                        '                qaerrorlog "" + iSlot + " UNDERLINED"
                        iTemp = iTemp + (2^i)
                    end if
                    call hMenuClose()
                else
                    '                printlog "" + iSlot + " not underlined"
                    'InsertSpecialCharacterDraw
                    hMenuSelectNr(iSpecialCharacterEntry)  ' because of bug #112919#
                    kontext "Sonderzeichen"
                    Sonderzeichen.Cancel '
                end if
            end if
        else
            Printlog "Word not underlined, Search-Toolbar active."
            call hMenuClose()
        end if
        Sleep (1)
        printlog " goto next word with keys [strg]+[right] "
        call hTypeKeys "<mod1 right>"
    next i
    printlog " leave textbox edit mode "

    iInternError = iError1 OR iError2
    if (iError1 > 0) then
        qaErrorLog "#i22192#: context menu opens not on cursor position"
        printlog "" + sLongToBinary(iError1, 11)
    end if
    if (iError2 > 0) then
        qaErrorLog "#i23568# context menu doesn't open in redlining mode before a punctuation mark"
        printlog "" + sLongToBinary(iError2, 11)
    end if
    if (not isMissing(iError)) then
        iError = iInternError
    end if
    sAnalyseContextMenu = iTemp
end function

'-------------------------------------------------------------------------------
function sLongToBinary(iTempIn as long, iCount as integer) as string

    ' lsb left !
    dim sTemp as string
    dim i as integer
    dim iMask as long
    dim itemp as long

    itemp = itempin

    for i = 1 to iCount
        iMask = iMask + (2^(i-1))
    next i

    sTemp = ""
    iTemp = Itemp AND iMask
    for i = 1 to iCount
        if ((iTemp MOD 2) = 1) then
            sTemp = sTemp + "1"
        else
            sTemp = sTemp + "0"
        end if
        iTemp = INT (iTemp / 2)
    next i
    sLongToBinary = sTemp
end function

'-------------------------------------------------------------------------------
function sBinaryToLong(sTempIn as String) as long

    ' lsb left !
    dim iTemp as long
    dim i as integer
    dim sTemp as string

    sTemp = sTempin

    for i = 1 to len(sTemp)
        if (mid(sTemp, i, 1) = "1") then
            iTemp = itemp + (2^(i-1))
        end if
    next i
    sBinaryToLong = iTemp
end function

'-------------------------------------------------------------------------------
function fGetIntoDictionary as boolean
    dim bFound as boolean
    dim iBooks as integer
    dim i as integer

    iBooks = Benutzerwoerterbuch.GetItemCount
    i=0
    bFound=TRUE
    while (bFound AND (i < iBooks))
        inc i
        Benutzerwoerterbuch.select i
        printlog Benutzerwoerterbuch.getSelText + i
        try
            Bearbeiten.Click
            bFound = FALSE
        catch
            printLog "wIgLi" + i
        endcatch
    wend
    fGetIntoDictionary = bFound
end function

'-------------------------------------------------------------------------------
function hSelectInList (window, sEntry as String) as Boolean

    printlog " alternativ method to 'hDoubleClickInList' (without mouse) "
    printlog "+ window: name of list "
    printlog "+ sEntry: string to find in list "
    printlog "+ ReturnValue: if found: TRUE; else FALSE "
    Dim i as Integer
    Dim sTemp as String
    Dim sLastTemp as String

    printlog " go through list from bottom and stop on the entry sEntry "
    window.TypeKeys "<End>"
    sTemp = ""
    do
        sLastTemp = sTemp
        sTemp = window.GetText
        window.TypeKeys "<Up>"
    loop while ((sEntry <> sTemp) AND (sLastTemp <> sTemp))
    printlog " press key [Return] "
    if (sEntry = sTemp) then
        window.TypeKeys "<Return>"
        hSelectInList = TRUE
    else
        hSelectInList = FALSE
    endif
end function

'-------------------------------------------------------------------------------
function hWalkTheStyles2 (atemp)

    'function hWalkTheStyles2 (bSet as boolean, aSettings(), atemp as variant) as string
    dim i as integer
    dim x as integer
    dim itemp
    dim bSet
    dim aSettings(5,5)
    '    dim atemp

    printlog " Organizer "
    i=1
    Kontext
    printlog aSettings(i,3)
    printlog aSettings(i,2)
    printlog val(aSettings(i,1))
    printlog atemp
    Messagebox.SetPage TabArea
    kontext "TabArea"
    atemp = Hatching
    printlog atemp

    if aSettings(i,3) then
        itemp = val(aSettings(i,1))
        printlog isobject(atemp)
        printlog isNumeric(atemp)
        Hatching.check
        atemp.check

    else
        '        aSettings(i,1).Uncheck
    endif
    i=2
    Kontext
    Messagebox.SetPage TabSchatten
    kontext "TabSchatten"

    Kontext
    Messagebox.SetPage TabVerwalten
    kontext "TabVerwalten"
    printlog " Line "
    i=2
    Kontext
    Messagebox.SetPage TabLinie
    kontext "TabLinie"
    'Context: *Line; Line Styles; Arrow Styles
    printlog " Area "
    i=3
    Kontext
    Messagebox.SetPage TabArea
    kontext "TabArea"
    'Context: *Area; *Shadow; Transparency; Colors; Gradients; Hatching; Bitmaps
    printlog " Shadowing "
    i=4
    Kontext
    Messagebox.SetPage TabSchatten
    kontext "TabSchatten"
    printlog " Transparency "
    i=5
    Kontext
    Messagebox.SetPage TabTransparenz
    kontext "TabTransparenz"
    printlog " Font "
    i=6
    Kontext
    Messagebox.SetPage TabFont
    kontext "TabFont"
    'Context: *Font; *Font Effect; Position
    printlog " Font Effect "
    i=7
    Kontext
    Messagebox.SetPage TabFontEffects
    kontext "TabFontEffects"
    printlog " Indents & Spacing "
    i=8
    Kontext
    Messagebox.SetPage TabEinzuegeUndAbstaende
    kontext "TabEinzuegeUndAbstaende"
    'Context: *Indents & Spacing; *Alignment; *Tabs
    printlog " Text "
    i=9
    Kontext
    Messagebox.SetPage TabText
    Kontext "TabText"
    'Context: *Text; *Text Animation
    printlog " Text Animation "
    i=10
    Kontext
    Messagebox.SetPage TabLauftext
    Kontext "TabLauftext"
    printlog " Dimensioning "
    i=11
    Kontext
    Messagebox.SetPage TabBemassung
    Kontext "TabBemassung"
    printlog " Connector "
    i=12
    Kontext
    Messagebox.setpage TabVerbinder
    Kontext "TabVerbinder"
    printlog " Alignment "
    i=13
    Kontext
    Messagebox.setpage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"
    printlog " Tabs "
    i=14
    Kontext
    Messagebox.setpage TabTabulator
    Kontext "TabTabulator"

    '    printlog " switch to tabpage 'Bullets' "
    '      Messagebox.SetPage TabBullet
    '      Kontext "TabBullet"
    '      sleep 1
    '      Call DialogTest (TabBullet)
    '      sleep 1
    '      Kontext
    '    printlog " switch to tabpage 'Numbering Type' "
    '      Messagebox.SetPage TabNumerierungsart
    '      Kontext "TabNumerierungsart"
    '      sleep 1
    '      Call DialogTest (TabNumerierungsart)
    '      sleep 1
    '      Kontext
    '    printlog " switch to tabpage 'Graphics' "
    '      Messagebox.SetPage TabGrafiken
    '      Kontext "TabGrafiken"
    '      sleep 1
    '      Call DialogTest (TabGrafiken)
    '      sleep 1
    '      Kontext
    '    printlog " switch to tabpage 'Customize' "
    '      Messagebox.SetPage TabOptionenNumerierung
    '      Kontext "TabOptionenNumerierung"
    '      sleep 1
    '      Call DialogTest (TabOptionenNumerierung)
    '      sleep 1
end function

'-------------------------------------------------------------------------------
function fGetSlideNumber (optional sCompare as integer) as integer

    printlog " PRESUPPOSITION: open Navigator "
    printlog "+ ENTRY: with or without a string "
    printlog "+ if string is given, it is compared with the actual selected slidename in the navigator, if not equal print warnlog "
    printlog "+ RETURN:  selected slidename in the navigator / empty string if navvigator is not open "
    printlog "+ EXIT: kontext on  DocumentPresentation "

    Kontext "NavigatorDraw"
    printlog "Checking if navigator is open, closing and opening for updating.."
    if NavigatorDraw.exists (5) then
        ViewNavigator ' to Workaround not updated navi :-(
        sleep 3
        ViewNavigator
        sleep 3
        printlog " check in list, if the page changed "

    else
        printlog "If Navigator is not open, opening it now."
        ViewNavigator
    endif
    sleep (1)
    printlog "Getting current slide number from navigator."
    fGetSlideNumber = val (right (Liste.GetSelText, 1))
    printlog "fGetSlideNumber has the value " & fGetSlideNumber
    printlog "Checking if slidenumber fits to Compare number, if this is given behind procedure call."
    if (isMissing (sCompare) = False) then ' if optional parameter exists
        if fGetSlideNumber <> sCompare then
            printlog "Warnlog if Slidenumber is not what it should be."
            Warnlog "Slide Number is '" + fGetSlideNumber + "'; should: '" + sCompare + "'"
        endif
    endif
    Kontext "DocumentPresentation"
end function

'-------------------------------------------------------------------------------
function fGetSlideCount (optional iCount as integer) as integer

    printlog " purpose: open navigator in impress and check/get number of slides from listbox "
    printlog "+ input  : optional number of slides, to compare to: if different warnlog "
    printlog "+ output : number of slides in presentation "
    dim i as integer

    printlog " open navigator "
    Kontext "Navigator"
    if Navigator.exists then
        Printlog "Navigator: open :-)"
    else
        Printlog "Navigator: NOT available :-( will be opened now!"
        ViewNavigator
    endif
    Sleep 1
    printlog " count rows in list of navigator: usually number of slides "
    Kontext "NavigatorDraw"
    i = Liste.GetItemCount
    if (isMissing(iCount) = FALSE) then
        if (i <> iCount) then
            Warnlog "Error! Expected slides: '" + iCount + "'; but are '" + i +"'"
        else
            Printlog "ok"
        endif
    endif
    printlog " close navigator "
    ViewNavigator
    fGetSlideCount = i
end function

'-------------------------------------------------------------------------------
function fGetSlideName (optional sCompare as string) as string

    printlog " PRESUPPOSITION: open Navigator "
    printlog "+ ENTRY: with or without a string "
    printlog "+ if string is given, it is compared with the actual selected slidename in the navigator, if not equal print warnlog "
    printlog "+ RETURN:  selected slidename in the navigator / empty string if navvigator is not open "
    printlog "+ EXIT: kontext on  DocumentPresentation "

    Kontext "NavigatorDraw"
    if NavigatorDraw.exists (5) then
        sleep 3
        printlog "check in list, if the page changed"
        fGetSlideName = Liste.GetSelText
    else
        warnlog "Navigator not open! in function fGetSlideName TBO"
        Kontext "NavigatorDraw"
        fGetSlideName = ""
    endif
    if (isMissing (sCompare) = False) then ' if optional parameter exists
        printlog "fGetSlideName is: " & fGetSlideName
        printlog "sCompare is: " & sCompare
        if fGetSlideName <> sCompare then
            warnlog "   Slide Name is '" + fGetSlideName + "'; should be: '" + sCompare + "'"
        endif
    endif
    Kontext "DocumentPresentation"
end function

'------------------------------------------------------------------------------
function fGetSetPageBackground (iSelect as integer, iWhere as integer) as integer

    printlog " Get or Set the Page Background via stylist (iWhere = 0) or format menue (...= 1) "
    printlog "+ if iSelect > 0 then set, else get "
    printlog "+ return selected color number or -1 on error "

    if (iWhere = 0) then
        printlog " Stylist -> Background -> Kontext menu -> modify -> Area -> Color "
        fGetSetPageBackground = -1 ' worst case
        Kontext "Stylist"
        if Stylist.NotExists (5) then
            FormatStylist
            Kontext "Stylist"
            if Stylist.NotExists (5) then warnlog "Could not open stylist :-("
        end if
        Praesentationsvorlagen.Click
        sleep 1
        Vorlagenliste.TypeKeys "<PAGEDOWN>"
        hDoubleClickInList (vorlagenliste, glLocale(5), TRUE)
        sleep 1
        vorlagenliste.OpenContextMenu
        sleep 1
        hMenuSelectNr (1)
    else
        printlog " Format -> Page -> Background -> Color "'FormatPage
        sleep 1
        try          ' this was just paranoia to find a not mentioned messagebox
            FormatSlideDraw
        catch
            warnlog "slooooow slot TBO :-("
            exit function
        endcatch
        sleep 1
        Kontext
        if (active.getrt = 373) then
            Active.SetPage TabArea
        else
            warnlog active.getrt
            if (active.getrt = 304) then
                warnlog active.gettext
            endif
        endif        ' paranoia end ----------------------------------------------
    endif

    kontext "TabArea"
    if TabArea.exists then
        FillOptions.Select 2 ' Select "Colour"
        if (iSelect > 1) then   ' Select the entry
            '         Color.Check
            if (iSelect < ColourList.GetItemCount) then
                ColourList.Select iSelect
            else
                warnlog "Select entry is larger than list :-("
            endif
            fGetSetPageBackground = ColourList.GetSelIndex
            if fGetSetPageBackground = 0 then
                warnlog "There were no color selected in the list."
            endif
            TabArea.OK
            sleep 2
            kontext
            if (active.exists (2)) then
                warnlog "active about <changing the background for all pages ?>: '" + active.gettext + "'"
                active.yes
            else
                printlog "No message about 'changing the background for all pages ?' :-("
            endif
        else ' yust read the selected entry
            if FillOptions.GetSelIndex = 2 then
                fGetSetPageBackground = ColourList.GetSelIndex
                TabArea.Cancel
            else
                warnlog "Can't get value, because something different than color is selected :-("
            endif
        endif
    else
        kontext "TabFont"
        if TabFont.exists then
            Warnlog "Something wrong with the word " + glLocale(5) + ". It was either not found or wrong."
        else
            warnlog "Error: Can't get context menu ?"
        endif
    endif

    if (iWhere = 0) then
        sleep 1 ' ABSOLUT NECESSARY !!! (TBO) else crash on UNIX on following command!!!!
        FormatStylist ' closing
    endif
    sleep 4
end function

'------------------------------------------------------------------------------
function CreateTextSetEffectAndAngle

    kontext "DocumentImpress"
    SetClipBoard "Revenue"
    DocumentImpress.TypeKeys "<MOD1 V>"
    SlideShowCustomAnimation
    Kontext "Tasks"
    WaitSlot (1000)
    EffectAdd.Click
    kontext
    printlog " Switch to TabPage: Entrance "
    active.setPage(TabEntrance)
    kontext "TabEntrance"
    if TabEntrance.exists(5) then
        printlog " select in the listbox 'Effects' the second entry"
        Effects.select (24)
        printlog " select speed 'Fast' -> fourth item in list "
        Speed.Select 2
        TabEntrance.OK
    end if
    kontext "tasks"
    EffectStart.TypeKeys "<HOME><DOWN>" 'Select the second entry.

    kontext "DocumentImpress"
    FormatPositionAndSize
    WaitSlot (1000)
    kontext
    active.setPage(TabDrehung)
    kontext "TabDrehung"

    Winkel.TypeKeys "45"
    TabDrehung.OK
    WaitSlot (1000)
    kontext "DocumentImpress"
end function

'-------------------------------------------------------------------------------
function fGetPresentationStyle (optional sCompare as integer) as integer

    printlog "+ ENTRY: with or without a string "
    printlog "+ if string is given, it is compared with the LAST CHARACTER of the actual selected style in the stylist, if not equal print warnlog "
    printlog "+ RETURN:  LAST CHARACTER of the actual selected style in the stylist "
    dim sTemp as integer
    dim sTemp0 as string

    sTemp = (-1)
    printlog " open stylist if not already open: Format->Stylist "
    kontext "Stylist"
    if (Stylist.exists = FALSE) then
        try
            FormatStylist
        catch
            sleep 1
        endcatch
    endif
    kontext "Stylist"
    if Stylist.exists(5) then
        sTemp0 = Vorlagenliste.GetSeltext
        sTemp = val(right (sTemp0, 1))
        if (isMissing (sCompare) = False) then ' if optional parameter exists
            if sTemp <> sCompare then
                Warnlog "Style Name's last character is '" + sTemp + "'; should be: '" + sCompare + "'"
            endif
        endif
        FormatStylist
    else
        Warnlog "The Stylist could not be opened for unknown reasons  :-("
    endif
    fGetPresentationStyle = sTemp
end function

'-------------------------------------------------------------------------------
function hPrepareSearchBUG

    '   warnlog "TBO: WA for bug #101974#"
    '      Kontext "DocumentImpressOutlineView"
    '      DocumentImpressOutlineView.TypeKeys ("<mod1 home>")
end function

'-------------------------------------------------------------------------------
function makeNumOutOfText ( sNum as String ) as String

    Dim sDummy as String
    Dim iComma as Integer

    iComma = Instr ( sNum, "," )
    if iComma <> 0 then
        sDummy = Left ( sNum, iComma-1 ) + "." + Mid ( sNum, iComma+1, len ( sNum )-2 )
    else
        sDummy = Left ( sNum, len (sNum)-2 )
    end if
    makeNumOutOfText = sDummy
end function

'-------------------------------------------------------------------------
function wIgnorierenlisteLoeschen as boolean

    Dim i as integer
    Dim j as integer
    dim iBooks as integer

    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","WRITINGAIDS")
    Sleep 3
    if (fGetIntoDictionary) then
        qaErrorLog "wIgLi"
        wIgnorierenlisteLoeschen = FALSE
        exit function
    end if
    Kontext "BenutzerwoerterbuchBearbeiten"
    sleep 1
    iBooks = Buch.GetItemCount
    for i = 1 to iBooks
        Buch.Select i
        if Left$(Buch.GetSelText,13)="IgnoreAllList" then
            sleep 2
            while (Loeschen.IsEnabled)
                Loeschen.Click
                sleep 1
            wend
        end if
    next i
    Kontext "BenutzerwoerterbuchBearbeiten"
    BenutzerwoerterbuchBearbeiten.Cancel
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    wIgnorierenlisteLoeschen = TRUE
end function

'-------------------------------------------------------------------------------
function optionstest

    dim filedialogue as boolean
    dim lala as integer
    dim optsound as integer
    dim os as integer
    dim oa as integer
    dim odc as integer
    dim ota as integer
    dim ets as integer
    dim etspeed as integer
    dim etrep as integer
    dim etshap as integer
    dim etgt as integer

    Kontext "Tasks"
    EffectOptions.Click
    kontext "TabEffect"
    if TabEffect.Exists(5) then
        optsound = Sound.GetItemCount
        for os = 1 to optsound
            Sound.Select os
            kontext "OeffnenDlg"
            if OeffnenDlg.Exists (5) then
                filedialogue = TRUE
                OeffnenDlg.Close
                kontext "TabEffect"
            else
                kontext "TabEffect"
            endif
        next os
        if AfterAnimation.isEnabled AND AfterAnimation.isVisible then
            for oa = 1 to AfterAnimation.GetItemCount
                AfterAnimation.Select oa
                if DimColor.isEnabled then
                    for odc = 1 to DimColor.GetItemCount
                        DimColor.Select odc
                    next odc
                endif
                if DelayBetweenCharacters.isEnabled then
                    for odc = 1 to DelayBetweenCharacters.GetItemCount
                        DelayBetweenCharacters.Select odc
                    next odc
                endif
            next oa
        else
            if DelayBetweenCharacters.isEnabled then
                for odc = 1 to DelayBetweenCharacters.GetItemCount
                    DelayBetweenCharacters.Select odc
                next odc
            endif
        endif
        for ota = 1 to TextAnimation.GetItemCount
            TextAnimation.Select ota
        next ota
        printlog " switch to TabPage 'Timing' "
        Kontext
        Active.SetPage TabTiming
        kontext "TabTiming"
        if TabTiming.Exists(5) then
            for ets = 1 to TimingStart.GetItemCount
                TimingStart.Select ets
            next ets
            if Delay.isVisible AND Delay.isEnabled then
                Delay.GetText
            else
                Warnlog "Delay in Effect Options were not to be found."
            endif
            if Speed.isVisible AND Speed.isEnabled then
                for etspeed = 1 to Speed.GetItemCount
                    Speed.Select etspeed
                next etspeed
            else
                printlog " No Speed-entry for this effect."
            endif
            if Repeat.isVisible AND Repeat.isEnabled then
                for etrep = 1 to Speed.GetItemCount
                    Repeat.Select etrep
                next etrep
            else
                Printlog "Repeat in Effect Options were not to be found."
            endif
            Rewind.Check
            Rewind.UnCheck
            TriggerAnimate.IsChecked
            TriggerStart.IsChecked
            if Shape.isVisible AND Shape.isEnabled then
                for etshap = 1 to Shape.GetItemCount
                    Shape.Select etshap
                next etshap
            else
                Warnlog "Shape in Effect Options were not to be found."
            endif
        else
            warnlog "Impress:Tasks Pane:Custom Animation:Effect Options: Timing TabPage didn't work."
        endif
        printlog " switch to TabPage 'Timing' "
        Kontext
        active.setPage TabTextAnimation
        kontext "TabTextAnimation"
        if TabTextAnimation.Exists(5) then
            lala = GroupText.GetItemCount
            for etgt = 1 to lala
                GroupText.Select etgt
                if AutomaticallyAfter.IsEnabled then
                    AutomaticallyAfter.Check
                    AutomaticallyAfter.TypeKeys "<UP>"
                endif
                if AnimateAttachedShape.IsEnabled then
                    AnimateAttachedShape.Check
                    if AnimateAttachedShape.IsChecked = FALSE then
                        Warnlog "AnimateAttachedShape should have been checked"
                    endif
                endif
                if InreverseOrder.IsEnabled then
                    InreverseOrder.Check
                    if InreverseOrder.IsChecked = FALSE then
                        Warnlog "InreverseOrder should have been checked"
                    endif
                endif
            next etgt
            TabTextAnimation.Cancel
        else
            warnlog "Impress:Tasks Pane:Custom Animation:Effect Options: TextAnimation TabPage didn't work."
        endif
    else
        warnlog "Impress:Tasks Pane:Custom Animation:... button didn't work."
    endif
    Kontext "Tasks"

end function

'-------------------------------------------------------------------------------
function optionstest2

    dim filedialogue as boolean
    dim lala as integer
    dim optsound as integer
    dim os as integer
    dim oa as integer
    dim odc as integer
    dim ota as integer
    dim ets as integer
    dim etspeed as integer
    dim etrep as integer
    dim etshap as integer
    dim etgt as integer

    Kontext "Tasks"
    EffectOptions.Click
    kontext "TabEffect"
    if TabEffect.Exists(5) then
        Sound.Select 5
        sleep 4
        if Play.IsEnabled then
            Play.Click
        else
            warnlog "Play should have been enabled after selecting a sound."
        endif
        AfterAnimation.Select 2
        if DimColor.isEnabled then
            DimColor.Select 5
        else
            Warnlog "DimColor should have been enabled"
        endif
        TextAnimation.Select 3
        if DelayBetweenCharacters.isEnabled then
            DelayBetweenCharacters.More 5
        else
            Warnlog "DelayBetweenCharacters should have been enabled"
        endif
        printlog " switch to TabPage 'Timing' "
        Kontext
        Active.SetPage TabTiming
        kontext "TabTiming"
        if TabTiming.Exists(5) then
            TimingStart.Select 2
            if Delay.isVisible AND Delay.isEnabled then
                Delay.More 5
            else
                Warnlog "Delay in Effect Options were not to be found."
            endif
            if Speed.isVisible AND Speed.isEnabled then
                Speed.Select 3
            else
                Printlog "Speed in Effect Options were not to be found."
            endif
            if Repeat.isVisible then
                if Repeat.isEnabled then
                    for etrep = 1 to Speed.GetItemCount
                        Repeat.Select etrep
                    next etrep
                else
                    Warnlog "Repeat in Effect Options were not enabled."
                endif
            else
                Warnlog "Repeat in Effect Options were not visible."
            endif
            if Rewind.isVisible then
                if Rewind.isEnabled then
                    Rewind.Check
                else
                    Printlog "Rewind in Effect Options were not to be found."
                endif
            else
                Printlog "Rewind in Effect Options were not to be found."
            endif
            if Rewind.isVisible then
                if Rewind.isEnabled then
                    Rewind.Check
                    Rewind.UnCheck
                else
                    Warnlog "Rewind in Effect Options were not enabled."
                endif
            else
                Warnlog "Rewind in Effect Options were not visible."
            endif
            TriggerAnimate.IsChecked
            TriggerStart.IsChecked
            if Shape.isVisible then
                if Shape.isEnabled then
                    for etshap = 1 to Shape.GetItemCount
                        Shape.Select etshap
                    next etshap
                else
                    Warnlog "Shape in Effect Options were not to be found."
                endif
            else
                Warnlog "Shape in Effect Options were not to be found."
            endif
        else
            warnlog "Impress:Tasks Pane:Custom Animation:Effect Options: Timing TabPage didn't work."
        endif
        printlog " switch to TabPage 'Timing' "
        Kontext
        active.setPage TabTextAnimation
        kontext "TabTextAnimation"
        if TabTextAnimation.Exists(5) then
            lala = GroupText.GetItemCount
            for etgt = 1 to lala
                GroupText.Select etgt
                if AutomaticallyAfter.IsEnabled then
                    AutomaticallyAfter.Check
                    AutomaticallyAfter.TypeKeys "<UP>"
                endif
                if AnimateAttachedShape.IsEnabled then
                    AnimateAttachedShape.Check
                    if AnimateAttachedShape.IsChecked = FALSE then
                        Warnlog "AnimateAttachedShape should have been checked"
                    endif
                endif
                if InreverseOrder.IsEnabled then
                    InreverseOrder.Check
                    if InreverseOrder.IsChecked = FALSE then
                        Warnlog "InreverseOrder should have been checked"
                    endif
                endif
            next etgt
            TabTextAnimation.Cancel
        else
            warnlog "Impress:Tasks Pane:Custom Animation:Effect Options: TextAnimation TabPage didn't work."
        endif
    else
        warnlog "Impress:Tasks Pane:Custom Animation:... button didn't work."
    endif
    Kontext "Tasks"

end function
'-------------------------------------------------------------------------------
