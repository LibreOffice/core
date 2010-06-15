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
'* Owner : wolfram.garten@sun.com
'*
'* short description :
'*
'***********************************************************************************
' #1 tiToolsSpellchecking
' #1 tiToolsSpellcheckingAutomatic
' #1 tiToolsThesaurus
' #1 tiToolsHyphenation
' #1 tiToolsAutoCorrect
' #1 tChineseTranslation
' #1 tiToolsMacro
' #1 tiToolsGallery
' #1 tiToolsEyedropper
' #1 tiToolsOptions
'\**********************************************************************************


testcase tiToolsSpellchecking

    if not gOOO then ' Spellcheck doesn't work in OOo builds.
        Call hNewDocument
        WaitSlot (2000)    'sleep 2
        call hSetSpellHypLanguage
        Call hTextrahmenErstellen ("Whaaaat", 10, 10, 30, 40)
        sleep 1
        ToolsSpellCheck
        WaitSlot (1000)    'sleep 1
        Kontext "MessageBox"
        if MessageBox.exists(2) then
            qaerrorlog "Messagebox : " + MessageBox.gettext() + " appear."
            qaerrorlog "Maybe no spellchecking for this languages is available."
            MessageBox.OK
        else
            Kontext "Rechtschreibung"
            if Rechtschreibung.exists then
                Call DialogTest ( Rechtschreibung )
                Rechtschreibung.Close
            else
                warnlog " Spellcheck dialog didn't came up :-("
            end if
        end if
        sleep 1
        Kontext "Messagebox"
        if Messagebox.exists (5) then
            warnlog "Shouldn't be any messagebox after pressing close in spellchecker"
            Messagebox.OK
            sleep (2)
            Kontext
        end if
        Call hCloseDocument
    else goto endsub
    endif
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsSpellcheckingAutomatic
    Call hNewDocument
    ToolsSpellcheckAutoSpellcheck
    Call hTextrahmenErstellen ("What", 10, 10, 30, 40)
    sleep 2
    ToolsSpellcheckAutoSpellcheck
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsThesaurus
    if not gOOO then ' Thesaurus doesn't work in OOo builds.

        dim sFileName as String

        call hSetSpellHypLanguage
        if (gApplication = "IMPRESS") then
            sFileName = (ConvertPath (gTesttoolPath + "graphics\required\input\engtext.odp"))
        else
            sFileName = (ConvertPath (gTesttoolPath + "graphics\required\input\engtext.odg"))
        end if
        if hFileExists ( sFileName ) = FALSE then
            warnlog "The language-file was not found or accessible! The test ends."
            goto endsub
        end if
        Call hFileOpen (sFileName)
        sleep (2)

        hTypeKeys "<TAB><RETURN>"
        hTypeKeys "<END><SHIFT HOME>"

        '   Call hTextrahmenErstellen ("SimpleTest" + "<Mod1 Shift left>", 10, 10, 30, 40)
        try
            ExtrasThesaurusDraw
            Kontext "Thesaurus"
            Call DialogTest ( Thesaurus )
            Sprache.Click
            Kontext "SpracheAuswaehlen"
            Call DialogTest ( SpracheAuswaehlen )
            SpracheAuswaehlen.cancel
            Kontext "Thesaurus"
            Nachschlagen.Click
            kontext
            if Messagebox.exists (5) then
                printlog "Messagebox: word not in thesaurus: '"+Messagebox.gettext+"'"
                Messagebox.ok
            end if
            sleep 1
            Kontext "Thesaurus"
            Thesaurus.Cancel
        catch
            warnlog "Thesaurus didn't work :-("
        endcatch
        sleep 1
        Call hCloseDocument
    else goto endsub
    endif
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsHyphenation
    Call hNewDocument
    ToolsLanguageHyphenationDraw
    WaitSlot (2000)    'sleep 2
    ToolsLanguageHyphenationDraw
    Call hCloseDocument
endcase

testcase tiToolsAutoCorrect
    dim iLanguage as integer ' for resetting the language
    Call hNewDocument
    WaitSlot (1000)    'sleep 1
    ToolsAutocorrect
    WaitSlot (2000)    'sleep 1
    Kontext
    Messagebox.SetPage TabErsetzung
    Kontext "TabErsetzung"
    Call DialogTest ( TabErsetzung )
    iLanguage = WelcheSprache.GetSelIndex
    WelcheSprache.Select 1 ' select language with empty list
    Kuerzel.SetText "a"
    ErsetzenDurch.SetText "b"
    Neu.Click
    sleep 1
    Loeschen.Click
    sleep 1
    try
        Loeschen.Click
    catch
        printlog "ok was CRASH before" '#
    endcatch
    WelcheSprache.select (iLanguage)
    Kontext
    Messagebox.SetPage TabAusnahmen
    Kontext "TabAusnahmen"
    Call DialogTest ( TabAusnahmen )
    Abkuerzungen.settext "Lala"
    AbkuerzungenNeu.click
    AbkuerzungenLoeschen.click
    Woerter.settext "LALA"
    WoerterAutomatisch.Check
    WoerterNeu.click
    WoerterLoeschen.click
    WoerterAutomatisch.UnCheck
    Kontext
    Messagebox.SetPage TabOptionen
    Kontext "TabOptionen"
    Call DialogTest ( TabOptionen )
    Kontext
    Messagebox.SetPage TabLocalizedOptions
    Kontext "TabLocalizedOptions"  ' 1a
    SingleQuotesReplace.Check
    SingleQuotesStart.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 1 )
    Sonderzeichen.Cancel
    Kontext "TabLocalizedOptions"  ' 1b
    SingleQuotesEnd.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 2 )
    Sonderzeichen.Cancel
    Kontext "TabLocalizedOptions"  ' 1s
    SingleQuotesDefault.Click

    Kontext "TabLocalizedOptions"  ' 2a
    DoubleQuotesStart.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 3 )
    Sonderzeichen.Cancel
    Kontext "TabLocalizedOptions"  ' 2b
    DoubleQuotesEnd.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 4 )
    Sonderzeichen.Cancel
    Kontext "TabLocalizedOptions"  ' 2s
    DoubleQuotesDefault.Click
    SingleQuotesReplace.UnCheck
    TabLocalizedOptions.cancel
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tChineseTranslation

    qaerrorlog( "#i89634# - Chinese Translation dialog does not close" )
    goto endsub

    dim sFileName   as string
    dim bSavedAsianSupport as boolean

    if uCase(gApplication) = "IMPRESS" then
        sFileName = "graphics\required\input\tchinese.odp"
    else
        sFileName = "graphics\required\input\tchinese.odg"
    end if

    Call hNewDocument
    WaitSlot (2000)    'sleep 1
    bSavedAsianSupport = ActiveDeactivateAsianSupport(TRUE)
    Call hFileOpen ( ConvertPath(gTesttoolPath + sFileName) )
    sleep (2)
    Kontext "Standardbar"
    if Bearbeiten.GetState(2) <> 1 then
        Bearbeiten.Click '0 = not pressed. 1 = pressed.
        Kontext
        if Active.Exists(1) then
            Active.Yes
        else
            warnlog "No messagebox after making document editable? - Test canceled here"
            goto endsub
        end if
    end if
    if uCase(gApplication) = "IMPRESS" then
        Kontext "DocumentImpress"
    else
        Kontext "DocumentDraw"
    end if
    EditSelectAll
    hTypeKeys "<RETURN>"
    hTypeKeys "<MOD1 HOME><RIGHT><RIGHT><SHIFT RIGHT RIGHT>"
    ToolsChineseTranslation
    WaitSlot (2000)    'sleep 1
    kontext "ChineseTranslation"
    Call DialogTest ( ChineseTranslation )
    EditTerms.Click
    kontext "ChineseDictionary"
    Call DialogTest ( ChineseDictionary )
    ChineseDictionary.Ok
    kontext "ChineseTranslation"
    ChineseTranslation.OK
    kontext
    if Messagebox.exists (5) then
        printlog "Messagebox: "+Messagebox.gettext+"'"
        Messagebox.ok
    end if
    ActiveDeactivateAsianSupport(bSavedAsianSupport)
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsMacro
    Call hNewDocument
    WaitSlot (2000)    'sleep 2
    ToolsMacro
    Kontext "Makro"
    Call DialogTest ( Makro )
    Verwalten.Click

    Kontext
    Messagebox.SetPage TabModule
    Kontext "TabModule"
    Call DialogTest ( TabModule )

    Kontext
    Messagebox.SetPage TabBibliotheken
    Kontext "TabBibliotheken"
    Call DialogTest ( TabBibliotheken )
    Hinzufuegen.Click
    Kontext "Messagebox"
    if Messagebox.Exists (5) then
        if Messagebox.GetRT = 304 then
            Warnlog Messagebox.Gettext
            Messagebox.Ok
        end if
    end if
    Kontext "OeffnenDlg"
    OeffnenDlg.Cancel
    Kontext "TabBibliotheken"
    Neu.Click
    kontext "NeueBibliothek"
    sleep 1 'Bibliotheksname
    NeueBibliothek.cancel
    Kontext "TabBibliotheken"
    TabBibliotheken.Close

    Kontext "Makro"
    Makro.Cancel
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsGallery
    Call hNewDocument
    ToolsGallery
    WaitSlot (2000)    'sleep 1
    ToolsGallery
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsEyedropper
    Call hNewDocument
    ToolsEyedropper
    Kontext "Pipette"
    Call DialogTest (Pipette)
    Pipette.Close
    sleep 1
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsOptions
    Call hNewDocument
    ToolsOptions
    WaitSlot (2000)    'sleep 1
    kontext "OptionenDlg"
    OptionenDlg.Close
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
