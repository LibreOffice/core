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
'* Owner : wolfram.garten@oracle.com
'*
'* short description :
'*
'*******************************************************************
'*
' #1 tToolsAutocorrectReplace
' #1 tToolsAutocorrectExceptions
' #1 tToolsAutocorrectCustomQuotes
' #1 tToolsAutocorrectOption
'*
'\*******************************************************************

testcase tToolsAutocorrectReplace

    dim i as integer
    dim x as integer
    dim iTemp as integer

    Call hNewDocument

    ToolsAutocorrect
    Kontext
    active.SetPage TabErsetzung
    Kontext "TabErsetzung"
    iTemp = WelcheSprache.GetItemCount
    for i = 1 to iTemp
        WelcheSprache.select i
        sleep 1
        x = Liste.getItemCount
        '         results from an english version:
        '         - 28: English (UK): 807
        '         - 29: English (USA): 807
        '         - 38: French (France): 304
        '         - 43: German (Germany): 100
        '         - 102: Spanish (Spain): 353
        if (x <> 0) then
            printlog " - " + i + ": " + WelcheSprache.getSelText + ": " + x
            '               printlog "    " + Liste.getItemText (5,1) + " - " + Liste.getItemText (5,2)
        endif
    next i
    TabErsetzung.OK

    hCloseDocument
endcase 'tToolsAutocorrectReplace

'-------------------------------------------------------------------------------

testcase tToolsAutocorrectExceptions
    dim i as integer
    dim x as integer
    dim y as integer
    dim iTemp as integer

    Call hNewDocument

    ToolsAutocorrect
    Kontext
    active.SetPage TabAusnahmen
    Kontext "TabAusnahmen"
    iTemp = WelcheSprache.GetItemCount
    for i = 1 to iTemp
        WelcheSprache.select i
        sleep 1
        x = AbkuerzungenListe.getItemCount
        '         results from an english version:
        '             - 28: English (UK): 120
        '             - 28: English (UK): 7
        '             - 29: English (USA): 120
        '             - 29: English (USA): 7
        '             - 38: French (France): 50
        '             - 38: French (France): 1
        '             - 43: German (Germany): 123
        '             - 43: German (Germany): 20
        '             - 102: Spanish (Spain): 115
        '             - 102: Spanish (Spain): 1
        if (x <> 0) then
            printlog " - " + i + ": " + WelcheSprache.getSelText + ": " + x
            '                printlog "    " + AbkuerzungenListe.getItemText (5)
        endif
        y = WoerterListe.getItemCount
        if (y <> 0) then
            printlog " - " + i + ": " + WelcheSprache.getSelText + ": " + y
            '                printlog "    " + WoerterListe.getItemText (5)
        endif
    next i
    TabAusnahmen.OK

    hCloseDocument
endcase 'tToolsAutocorrectExceptions

'-------------------------------------------------------------------------------

testcase tToolsAutocorrectCustomQuotes(bSimple as boolean)

    ' bSinple = true: use simple quotes ;; else Double Quotes
    Dim Vorn$
    Dim Hinten$
    Dim Normal$
    dim sTemp as string
    dim sFileName as string

    if (gApplication = "IMPRESS") then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

    if iSprache = "1" OR iSprache = "33" OR iSprache = "34" OR iSprache = "36" OR iSprache = "39" OR iSprache = "46" OR iSprache = "48" OR iSprache = "49" OR iSprache = "50" OR iSprache = "55" then
        sFileName = (ConvertPath ( gTesttoolPath + "graphics\required\input\rightfont" & iSprache & "." + ExtensionString ))
    elseif bAsianLan = true then
        Printlog "Language-file for Asian language " & iSprache & " is missing. Using standard instead."
        sFileName = (ConvertPath ( gTesttoolPath + "graphics\required\input\rightfont." + ExtensionString))
    else
        warnlog "Language-file for language " & iSprache & " is missing. Using standard instead."
        sFileName = (ConvertPath ( gTesttoolPath + "graphics\required\input\rightfont." + ExtensionString))
    endif

    if FileExists ( sFileName ) = FALSE then
        warnlog "The language-file was not found or accessible! The test ends."
        goto endsub
    endif
    Call hNewDocument

    hFileOpen sFileName
    printlog "File opened: " + sFileName
    Sleep 3

    call fMakeDocumentWritable

    if bSimple then
        Printlog "- Tools/Autocorrect -single quotes"
        Normal$ = Chr(39)
        select case iSprache
        case 01 : Vorn$ = Chr(8216) : Hinten$ = Chr(8217)
        case 03 : Vorn$ = Chr(8216) : Hinten$ = Chr(8217)
        case 07 : qaerrorlog "Test impossible. No quotes in russian!"
            hCloseDocument
            goto endsub
        case 31 : Vorn$ = Chr(8218) : Hinten$ = Chr(8216)
        case 33 : Vorn$ = Chr(8216) : Hinten$ = Chr(8217)
        case 34 : Vorn$ = Chr(8216) : Hinten$ = Chr(8217)
        case 36 : Vorn$ = Chr(187)  : Hinten$ = Chr(171)
        case 39 : Vorn$ = Chr(8216) : Hinten$ = Chr(8217)
        case 45 : Vorn$ = Chr(8217) : Hinten$ = Chr(8217)
        case 46 : Vorn$ = Chr(8217) : Hinten$ = Chr(8217)
        case 48 : Vorn$ = Chr(8216) : Hinten$ = Chr(8217)
        case 49 : Vorn$ = Chr(8218) : Hinten$ = Chr(8216)
        case 50 : Vorn$ = Chr(8216) : Hinten$ = Chr(8217)
        case 55 : Vorn$ = Chr(8216) : Hinten$ = Chr(8217)
        case 81, 82, 86, 88 : Vorn$ = Chr(8216) : Hinten$ = Chr(8217) ' Asian
        case else : Warnlog "- Nothing prepared for this language, but I run: take the warnlogs an insert the numbers here.: "+iSprache
            Vorn$ = Chr(8216) : Hinten$ = Chr(8217)
        end select
    else
        Printlog "- Tools/Autocorrect -double quotes"
        Normal$ = Chr(34)
        select case iSprache
        case 01 : Vorn$ = Chr(8220) : Hinten$ = Chr(8221)
        case 03 : Vorn$ = Chr(8220) : Hinten$ = Chr(8221)
        case 07 : qaerrorlog "Test not possible. No quotes in russian !"
            Call hCloseDocument
            goto endsub
        case 31 : Vorn$ = Chr(8222) : Hinten$ = Chr(8220)
        case 33 : Vorn$ = Chr(171)  : Hinten$ = Chr(187)
        case 34 : Vorn$ = Chr(8220) : Hinten$ = Chr(8221)
        case 36 : Vorn$ = Chr(8222) : Hinten$ = Chr(8221)
        case 39 : Vorn$ = Chr(8220) : Hinten$ = Chr(8221)
        case 45 : Vorn$ = Chr(8221) : Hinten$ = Chr(8221)
        case 46 : Vorn$ = Chr(8221) : Hinten$ = Chr(8221)
        case 48 : Vorn$ = Chr(8222) : Hinten$ = Chr(8221)
        case 49 : Vorn$ = Chr(8222) : Hinten$ = Chr(8220)
        case 50 : Vorn$ = Chr(8220) : Hinten$ = Chr(8221)
        case 55 : Vorn$ = Chr(8220) : Hinten$ = Chr(8221)
        case 81, 82, 86, 88 : Vorn$ = Chr(8220) : Hinten$ = Chr(8221) ' Asian
        case else : Warnlog "- Nothing prepared for this language: "+iSprache
            goto endsub
        end select
    endif
    Printlog "   - replace quotes" '------------------------------------------------------------------------------------------
    ToolsAutocorrect
    Kontext
    active.SetPage TabLocalizedOptions
    Kontext "TabLocalizedOptions"
    if bSimple then
        SingleQuotesReplace.Check
    else
        DoubleQuotesReplace.Check
        DoubleQuotesDefault.Click
    endif
    TabLocalizedOptions.OK
    sleep 5
    gMouseClick 50,50
    sleep 5
    hTextrahmenErstellen ( Normal$ + "Hallo" + Normal$ + " ",20,20,60,30)
    sleep 5
    hTypeKeys ("<Home><Shift Right>")
    sleep 5
    EditCopy
    sTemp = GetClipboardText
    if (sTemp <> Vorn$) then
        Warnlog "The beginning quote seems wrong. Should be :'" + Vorn$ +"':"+ASC(vorn$)+ ":  but it is = :'" + sTemp +"':"+ASC(sTemp)+":"
    endif
    sleep 1
    hTypeKeys "<End><Left><Shift Left>"
    Sleep 1
    EditCopy
    sTemp = GetClipboardText
    if (sTemp <> Hinten$) then
        Warnlog "The end quote seems wrong. Should be :" + Hinten$ +":"+ASC(Hinten$) +":  but it is :" + sTemp + ":"+ASC(sTemp)+":"
    endif

    hTypeKeys "<Home><Shift End><Delete>"

    Printlog "   - dont replace quotes" '------------------------------------------------------------------------------------------
    ToolsAutocorrect
    Kontext
    Active.SetPage TabLocalizedOptions
    Kontext "TabLocalizedOptions"
    if bSimple then
        SingleQuotesReplace.Uncheck
    else
        DoubleQuotesReplace.Uncheck
    endif
    TabLocalizedOptions.OK
    sleep 1
    gMouseClick 50,50
    sleep 1
    hTextrahmenErstellen ( Normal$ + "Hello" + Normal$ + " ",20,20,60,30)
    sleep 1
    hTypeKeys "<Home><Shift Right>"
    Sleep 1
    EditCopy
    sTemp = GetClipboardText
    if (sTemp <> Normal$) then
        Warnlog "The beginning quote seems wrong. Should be :'" + Vorn$ +"':"+ASC(vorn$)+ ":  but it is = :'" + sTemp +"':"+ASC(sTemp)+":"
    endif

    hTypeKeys "<End><Left><Shift Left>"
    Sleep 1
    EditCopy
    sTemp = GetClipboardText
    if (sTemp <> Normal$) then
        Warnlog "The end quote seems wrong. Should be :" + Hinten$ +":"+ASC(Hinten$) +":  but it is :" + sTemp + ":"+ASC(sTemp)+":"
    endif
    hTypeKeys "<Home><Shift End><Delete>"

    Printlog "   - replace quotes" '------------------------------------------------------------------------------------------
    ToolsAutocorrect
    Kontext
    active.SetPage TabLocalizedOptions
    Kontext "TabLocalizedOptions"
    if bSimple then
        SingleQuotesReplace.Check
        SingleQuotesStart.Click
    else
        DoubleQuotesReplace.Check
        DoubleQuotesStart.Click
    endif
    Kontext "Sonderzeichen"
    sleep 1
    Schriftliste.TypeKeys "<Home>"
    Schriftliste.TypeKeys "<Right>", 91 '123
    sleep 1
    Sonderzeichen.OK

    Kontext "TabLocalizedOptions"
    if bSimple then
        SingleQuotesEnd.Click
    else
        DoubleQuotesEnd.Click
    endif
    Kontext "Sonderzeichen"
    sleep 1
    Schriftliste.TypeKeys "<Home>"
    Schriftliste.TypeKeys "<Right>", 93 '125
    sleep 1
    Sonderzeichen.OK

    Kontext "TabLocalizedOptions"
    TabLocalizedOptions.OK
    sleep 5
    gMouseClick 50,50
    sleep 5
    hTextrahmenErstellen ( Normal$ + "Hello" + Normal$ + " ",20,20,60,30)
    sleep 2
    hTypeKeys "<Home><Shift Right>"
    Sleep 5
    EditCopy
    sTemp = GetClipboardText
    if (sTemp <> Chr(123)) then
        Warnlog "Beginning quote is wrong. It should be :" + Chr (123) + ":123:  but it is = " + sTemp +":"+ASC(sTemp)+":"
    endif

    hTypeKeys "<End><Left><Shift Left>"
    Sleep 1
    EditCopy
    sTemp = GetClipboardText
    if (sTemp <> Chr(125)) then
        Warnlog "The end quote is wrong, it should be :" + Chr (125) + ":125:  but it is = " + sTemp +":"+ASC(sTemp)+":"
    endif
    hTypeKeys "<Home><Shift End><Delete>"

    Printlog "   - back to standard" '------------------------------------------------------------------------------------------
    ToolsAutocorrect
    Kontext
    active.SetPage TabLocalizedOptions
    Kontext "TabLocalizedOptions"
    if bSimple then
        SingleQuotesReplace.Check
        SingleQuotesDefault.Click
    else
        DoubleQuotesReplace.Check
        DoubleQuotesDefault.Click
    endif

    TabLocalizedOptions.OK
    sleep 1
    hTextrahmenErstellen ( Normal$ + "Hello" + Normal$ + " ",20,20,60,30)
    sleep 2
    hTypeKeys "<Home><Shift Right>"
    Sleep 1
    EditCopy
    sTemp = GetClipboardText
    if (sTemp <> Vorn$) then
        Warnlog "The beginning quote seems wrong. Should be :" + Vorn$ +":"+ASC(vorn$)+ ":  but it is = :" + sTemp +":"+ASC(sTemp)+":"
    endif

    hTypeKeys "<End><Left><Shift Left>"
    Sleep 1
    EditCopy
    sTemp = GetClipboardText
    if (sTemp <> Hinten$) then
        Warnlog "The end quote seems wrong. Should be :" + Hinten$ +":"+ASC(Hinten$) +":  but it is :" + sTemp + ":"+ASC(sTemp)+":"
    endif
    hTypeKeys "<Home><Shift End><Delete>"

    ToolsAutocorrect
    Kontext
    active.SetPage TabLocalizedOptions
    Kontext "TabLocalizedOptions"
    if bSimple then
        SingleQuotesReplace.UnCheck
    else
        DoubleQuotesReplace.UnCheck
    endif
    TabLocalizedOptions.OK
    sleep 1

    hCloseDocument
endcase 'tToolsAutocorrectCustomQuotes

'-------------------------------------------------------------------------------
testcase tToolsAutocorrectOption

    const cEntryCount = 7
    dim i as integer
    dim iEntryCount as integer
    dim sEntries (20, 2) as string
    dim bState as boolean

    ' this test relies on the following order of entries!!:
    sEntries (1,1) = "Use replacement table"
    sEntries (1,2) = "(C) "
    sEntries (2,1) = "Correct TWo INitial CApitals"
    sEntries (2,2) = "ABer "
    sEntries (3,1) = "Capitalize first letter of every sentence"
    sEntries (3,2) = "Hello. this is a sentence. "
    sEntries (4,1) = "Automatic *bold* and _underline_"
    sEntries (4,2) = "*Hello* " '"_Hello_ "
    sEntries (5,1) = "URL Recognition"
    sEntries (5,2) = "It is www.ksta.de "
    'sEntries (6,1) = "Replace 1st... with 1^st..."
    'sEntries (6,2) = "1st "
    'sEntries (7,1) = "Replace 1/2 ... with ÃÂ» ..."
    'sEntries (7,2) = "1/2 "
    sEntries (6,1) = "Replace dashes"
    sEntries (6,2) = "Hello - Hrbert "
    sEntries (7,1) = "Ignore double spaces"
    sEntries (7,2) = "Hello  this is 2  blanks "

    Printlog "- Tools / Autocorrect -options"
    Call hNewDocument()
    callAutocorrectOptions

    iEntryCount = Einstellungen.getItemCount
    if (iEntryCount <> cEntryCount) then
        warnLog "Unexpected count of entries in Listbox: expected: '" + cEntryCount + "' actual: '" + iEntryCount + "'"
        for i = 1 to iEntryCount
            Printlog " " + i + " - '" + sEntries (i,1) + "' ?= '" + Einstellungen.getItemText(1) + "'"
        next i
    endif
    TabOptionen.OK

    for i = 1 to cEntryCount
        callAutocorrectOptions
        Printlog " " + i + " - '" + sEntries (i,1) + "' ?= '" + Einstellungen.getItemText(i) + "'"
        bState = (i <> cEntryCount)
        Einstellungen.select(i)
        if (bState <> Einstellungen.isChecked) then
            Warnlog "        Entry is '" + bState + "' per default - will be '" + bState + "' now!"
        end if
        ' first test the opposite default value
        if bState then
            Einstellungen.UnCheck
        else
            Einstellungen.Check
        endif
        TabOptionen.OK
        sCheckCheck (i, sEntries(i,2), not bState)
        ' now test and set the default value
        callAutocorrectOptions
        Einstellungen.select(i)
        if bState then
            Einstellungen.Check
        else
            Einstellungen.UnCheck
        endif
        TabOptionen.OK
        sCheckCheck (i, sEntries(i,2), bState)
    next i

    hCloseDocument
    sleep 1
endcase 'tToolsAutocorrectOption

'-------------------------------------------------------------------------------