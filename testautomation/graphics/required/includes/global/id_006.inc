'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: id_006.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-08-28 11:45:15 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
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
        '/// open application ///'
           Call hNewDocument
            WaitSlot (2000)    'sleep 2
        '/// call subroutine 'hSetSpellHypLanguage' for setting the default language in th eoptions, to enable it for languages, which don't provide a dictionary (usually asian ones) ///'
           call hSetSpellHypLanguage
        '/// create textframe with content ///'
           Call hTextrahmenErstellen ("Whaaaat", 10, 10, 30, 40)
           sleep 1
        '/// Tools->Spellcheck->Check ///'
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
                            '/// close dialog 'Spellcheck' ///'
                            Rechtschreibung.Close
                        else
                            warnlog " Spellcheck dialog didn't came up :-("
                        end if
                end if
                sleep 1
                '/// say OK to messagebox about 'Spellcheck has been completed' ///'
                Kontext "Messagebox"
                if Messagebox.exists (5) then
                    warnlog "Shouldn't be any messagebox after pressing close in spellchecker"
                    Messagebox.OK
                    sleep (2)
                    Kontext
                end if
            '/// close application ///'
        Call hCloseDocument
    else goto endsub
    endif
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsSpellcheckingAutomatic
    '/// open application ///'
    Call hNewDocument
    '/// Tools->Spellcheck->AutoSpellcheck ///'
    ToolsSpellcheckAutoSpellcheck
    '/// create textframe with text ///'
    Call hTextrahmenErstellen ("What", 10, 10, 30, 40)
    sleep 2
    '/// Tools->Spellcheck->AutoSpellcheck ///'
    ToolsSpellcheckAutoSpellcheck
    '/// close application ///'
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsThesaurus
    qaerrorlog "#i93133#: Thesaurus not active in loaded bugdoc"
    goto endsub
    if not gOOO then ' Thesaurus doesn't work in OOo builds.

    dim sFileName as String
    
    '/// call subroutine 'hSetSpellHypLanguage' for setting the default language in the options, to enable it for languages, which don't provide a dictionary (usually asian ones) ///'
    call hSetSpellHypLanguage
    '/// open application-specific document which contains an American-formatted textframe with text. And select the text ///'
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
            '/// Tools->Thesaurus ///'
            ExtrasThesaurusDraw
            Kontext "Thesaurus"
                Call DialogTest ( Thesaurus )
                '/// click button 'language' ///'
                Sprache.Click
                Kontext "SpracheAuswaehlen"
                    Call DialogTest ( SpracheAuswaehlen )
                    '/// cancel dialog 'select language' ///'
                    SpracheAuswaehlen.cancel
            Kontext "Thesaurus"
                '/// click button 'search' ///'
                Nachschlagen.Click
                kontext
                '/// if messagebox exist, say OK; (word not found) ///'
                if Messagebox.exists (5) then
                    printlog "Messagebox: word not in thesaurus: '"+Messagebox.gettext+"'"
                    Messagebox.ok
                end if
                sleep 1
                Kontext "Thesaurus"
                    '/// cancel dialog 'Thesaurus' ///'
                    Thesaurus.Cancel
        catch
            warnlog "Thesaurus didn't work :-("
        endcatch
        sleep 1
        '/// close application ///'
        Call hCloseDocument
    else goto endsub
    endif
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsHyphenation
    '/// open application ///'
    Call hNewDocument
    '/// Tools->Hyphenation ///'
    ExtrasSilbentrennungDraw
    WaitSlot (2000)    'sleep 2
    '/// Tools->Hyphenation ///'
    ExtrasSilbentrennungDraw
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tiToolsAutoCorrect
    dim iLanguage as integer ' for resetting the language
    '/// open application ///'
    Call hNewDocument
    WaitSlot (1000)    'sleep 1
    '/// Tools->Autocorrect ///'
    ToolsAutocorrect
    WaitSlot (2000)    'sleep 1
    Kontext
    '/// select tabpage 'Replace' ///'
    Messagebox.SetPage TabErsetzung
    Kontext "TabErsetzung"
        Call DialogTest ( TabErsetzung )
        '/// remember the language, that is selected in the 'Replacements and exceptions for Language' Listbox ///'
        iLanguage = WelcheSprache.GetSelIndex
        '///+ change the language to the 1st from the top ///'
        WelcheSprache.Select 1 ' select language with empty list
        '///+ type something into the field 'replace' ///'
        Kuerzel.SetText "a"
        '///+ type something into the field 'with' ///'
        ErsetzenDurch.SetText "b"
        '///+ click button 'new' ///'
        Neu.Click
        sleep 1
        '///+ click button 'delete' ///'
        Loeschen.Click
        sleep 1
        try
            '///+ click button 'delete' again ///'
            Loeschen.Click
        catch
            printlog "ok was CRASH before" '#
        endcatch
        '///+ if nothing happens it is ok, was a Crash before :-( ///'
        '///+ restore cthe remembered language ///'
        WelcheSprache.select (iLanguage)
        Kontext
        '/// select tabpage 'Exception' ///'
        Messagebox.SetPage TabAusnahmen
            Kontext "TabAusnahmen"
            Call DialogTest ( TabAusnahmen )
            '/// type something into the field 'Abbreviations' ///'
            Abkuerzungen.settext "Lala"
            '///+ click button 'new' in 'Abbreviations' ///'
            AbkuerzungenNeu.click
            '///+ click button 'delete' in 'Abbreviations' ///'
            AbkuerzungenLoeschen.click
            '/// type something into the field 'Word with TWo INitial CApitals' ///'
            Woerter.settext "LALA"
            '///+ check the checkbox 'AutoInclude' in 'Word with TWo INitial CApitals' ///'
            WoerterAutomatisch.Check
            '///+ click button 'new' in 'Word with TWo INitial CApitals' ///'
            WoerterNeu.click
            '///+ click button 'delete' in 'Word with TWo INitial CApitals' ///'
            WoerterLoeschen.click
            '///+ UNcheck the checkbox 'AutoInclude' in 'Word with TWo INitial CApitals' ///'
            WoerterAutomatisch.UnCheck
            Kontext
            '/// select tabpage 'Options' ///'
            Messagebox.SetPage TabOptionen
                Kontext "TabOptionen"
                Call DialogTest ( TabOptionen )
                Kontext
            '/// select tabpage 'Custom Quotes' ///'
            Messagebox.SetPage TabTypografisch
                Kontext "TabTypografisch"  ' 1a
                    '/// in the part of 'single quotes': ///'
                    '///+ check 'replace' ///'
                    '///+ click 'start quote' ///'
                    EinfacheErsetzen.Check
                    EinfachWortAnfang.Click
                    Kontext "Sonderzeichen"
                        Call DialogTest ( Sonderzeichen, 1 )
                        '///+  cancel dialog 'start quote' ///'
                        Sonderzeichen.Cancel
                Kontext "TabTypografisch"  ' 1b
                    '///+ click 'end quote' ///'
                    EinfachWortEnde.Click
                    Kontext "Sonderzeichen"
                        Call DialogTest ( Sonderzeichen, 2 )
                        '///+  cancel dialog 'end quote' ///'
                        Sonderzeichen.Cancel
                Kontext "TabTypografisch"  ' 1s
                    '///+ click button 'default' ///'
                    EinfachStandard.Click

                Kontext "TabTypografisch"  ' 2a
                    '/// in the part of 'double quotes': ///'
                    '///+ click 'start quote' ///'
                    DoppeltWortAnfang.Click
                    Kontext "Sonderzeichen"
                        Call DialogTest ( Sonderzeichen, 3 )
                        '///+  cancel dialog 'start quote' ///'
                        Sonderzeichen.Cancel
                Kontext "TabTypografisch"  ' 2b
                    '///+ click 'end quote' ///'
                    DoppeltWortEnde.Click
                    Kontext "Sonderzeichen"
                        Call DialogTest ( Sonderzeichen, 4 )
                        '///+  cancel dialog 'end quote' ///'
                        Sonderzeichen.Cancel
                Kontext "TabTypografisch"  ' 2s
                    '///+ click button 'default' ///'
                    DoppeltStandard.Click
                    '///+ UNcheck 'replace' ///'
                    EinfacheErsetzen.UnCheck
                    '/// cancel dialog 'AtorCorrect' ///'
                    TabTypografisch.cancel
    '/// close application ///'
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

    '/// Open application ///'
    Call hNewDocument
    WaitSlot (2000)    'sleep 1
    '/// Save old state and Activate Support for Asian language ///'
    bSavedAsianSupport = ActiveDeactivateAsianSupport(TRUE)
    '/// Open Document ///'
    Call hFileOpen ( ConvertPath(gTesttoolPath + sFileName) )
    '/// If write-protected - open as Temp-file ///'
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
    '/// Select all, Press RETURN to enter text in Editmode ///'
    '/// Move marker to top of the text, go two steps right, mark two characters ///'
    EditSelectAll
    hTypeKeys "<RETURN>"
    hTypeKeys "<MOD1 HOME><RIGHT><RIGHT><SHIFT RIGHT RIGHT>"
    '/// Open Chinesetranslation ///'
    ToolsChineseTranslation
    WaitSlot (2000)    'sleep 1
    kontext "ChineseTranslation"
        '/// Check if everything is there ///'
        Call DialogTest ( ChineseTranslation )
        '/// Click on EditTerms ///'
        EditTerms.Click
        kontext "ChineseDictionary"
            '/// Check if everything is there ///'
            Call DialogTest ( ChineseDictionary )
            '/// Close dialog 'ChineseDictionary' with 'OK' ///'
            ChineseDictionary.Ok
    kontext "ChineseTranslation"
        '/// Close dialog 'Chinese' with 'OK' ///'
        ChineseTranslation.OK
    kontext
        '/// if messagebox exist, say OK; ('Spellcheck completed' or 'Word not found') ///'
        if Messagebox.exists (5) then
            printlog "Messagebox: "+Messagebox.gettext+"'"
            Messagebox.ok
        end if
    '/// Restore old state for Asian language ///'
    ActiveDeactivateAsianSupport(bSavedAsianSupport)
    '/// Close application ///'
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsMacro
    '/// open application ///'
    Call hNewDocument
    WaitSlot (2000)    'sleep 2
    '/// Tools->Macro ///'
    ToolsMacro
    Kontext "Makro"
        Call DialogTest ( Makro )
        '/// click button 'organizer...' ///'
        Verwalten.Click

    Kontext
        '/// switch to tabpage 'Modules' ///'
        Messagebox.SetPage TabModule
        Kontext "TabModule"
            Call DialogTest ( TabModule )

    Kontext
        '/// switch to tabpage 'Libraries' ///'
        Messagebox.SetPage TabBibliotheken
        Kontext "TabBibliotheken"
            Call DialogTest ( TabBibliotheken )
            '/// click lbutton 'append' ///'
            Hinzufuegen.Click
            Kontext "Messagebox"
                if Messagebox.Exists (5) then
                    if Messagebox.GetRT = 304 then
                        Warnlog Messagebox.Gettext
                        Messagebox.Ok
                    end if
                end if
            Kontext "OeffnenDlg"
                '/// cancel dialog 'append libraries' ///'
                OeffnenDlg.Cancel
        Kontext "TabBibliotheken"
            '/// click button 'new' ///'
            Neu.Click
            kontext "NeueBibliothek"
                sleep 1 'Bibliotheksname
                '/// cancel dialog 'new library' ///'
                NeueBibliothek.cancel
        Kontext "TabBibliotheken"
            '/// close dialog 'macro organizer' ///'
            TabBibliotheken.Close

    Kontext "Makro"
        '/// close dialog 'macro' ///'
        Makro.Cancel
    '/// close application ///'
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsGallery
    '/// open application ///'
    Call hNewDocument
    '/// Tools->Gallery ///'
    ToolsGallery
    WaitSlot (2000)    'sleep 1
    '/// Tools->Gallery ///'
    ToolsGallery
    '/// close application ///'
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsEyedropper
    '/// open application ///'
    Call hNewDocument
    '/// Tools->Eyedropper ///'
    ToolsEyedropper
    Kontext "Pipette"
        Call DialogTest (Pipette)
        '/// close dialog 'Eyedropper' ///'
        Pipette.Close
        sleep 1
    '/// close application ///'
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiToolsOptions
    '/// open application ///'
    Call hNewDocument
    '/// Tools->Options ///'
    ToolsOptions
    WaitSlot (2000)    'sleep 1
    kontext "OptionenDlg"
        '/// close dialog 'Options' ///'
        OptionenDlg.Close
    '/// close application ///'
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
