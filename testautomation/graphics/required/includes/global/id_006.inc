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
'\**********************************************************************************
sub id_Tools
    printLog "---------    id_006    ----------"
    call tiToolsSpellchecking
    call tiToolsSpellcheckingAutomatic
    call tiToolsThesaurus
    call tiToolsHyphenation
    call tiToolsAutoCorrect
    call tChineseTranslation
    call tiToolsMacro
    call tiToolsGallery
    call tiToolsEyedropper    
    call tToolsOptionsTest ' global one
end sub

'-------------------------------------------------------------------------------

testcase tiToolsSpellchecking

    if not gOOO then ' Spellcheck doesn't work in OOo builds.
        printlog "open application"
           Call hNewDocument
            WaitSlot (2000) 
        printlog " call subroutine 'hSetSpellHypLanguage' for setting the default language in th eoptions, to enable it for languages, which don't provide a dictionary (usually asian ones)"
           call hSetSpellHypLanguage
        printlog "create textframe with content"
           Call hTextrahmenErstellen ("Whaaaat", 10, 10, 30, 40)
           sleep 1
        printlog "Tools->Spellcheck->Check"
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
                            printlog "close dialog 'Spellcheck'"
                            Rechtschreibung.Close
                        else
                            warnlog " Spellcheck dialog didn't came up :-("
                        end if
                end if
                sleep 1
                printlog "say OK to messagebox about 'Spellcheck has been completed'"
                Kontext "Messagebox"
                if Messagebox.exists (5) then
                    warnlog "Shouldn't be any messagebox after pressing close in spellchecker"
                    Messagebox.OK
                    sleep (2)
                    Kontext
                end if
            printlog "close application"
        Call hCloseDocument
    else goto endsub
    endif
endcase 'tiToolsSpellchecking

'-------------------------------------------------------------------------------

testcase tiToolsSpellcheckingAutomatic
    printlog "open application"
    Call hNewDocument
    printlog "Tools->Spellcheck->AutoSpellchec"
    ToolsSpellcheckAutoSpellcheck
    printlog " create textframe with text "
    Call hTextrahmenErstellen ("What", 10, 10, 30, 40)
    sleep 2
    printlog "Tools->Spellcheck->AutoSpellcheck "
    ToolsSpellcheckAutoSpellcheck
    printlog "close application "
    Call hCloseDocument
endcase 'tiToolsSpellcheckingAutomatic

'-------------------------------------------------------------------------------

testcase tiToolsThesaurus
    Dim sWord as string
    Dim sExt as string
    Dim sFileName as string

    'for normal text, the thesaurus is enabled,
    'if the format->character->language of the word has a thesaurus
    'unfortunately in textboxes this doesn't work, you can give the word a langauge,
    'but this is been ignored by our thesaurus
    'so especially for asian languages you have to set a default language in the options,
    'to be able to use the thesaurus there - fallback to englisch,
    'because there is no thesaurus for cjk languages

    'As long as OOo has no modules for that, disabled
    if gOOO = True then
        printlog "No spellchecking in OOo"
        goto endsub
    endif

    'First we make sure we test a language where Thesaurus
    select case iSprache
       case 01 : sWord = "Hello"
       case 03 : sWord = "Alo"
       case 07 : printlog "- No Thesaur for Russian available"
                 goto endsub
       case 30 : Printlog "- No Thesaurus available !"
                 goto endsub
       case 31 : sWord = "Guiten"
       case 33 : sWord = "Bonjour"
       case 34 : sWord = "Hola"
       case 35 : Printlog "- No Thesaurus available !"
                 goto endsub
       case 36 : Printlog "- No Thesaurus available !"
                 goto endsub
       case 37 : Printlog "- No Thesaurus available !"
                 goto endsub
       case 39 : sWord = "Ciao"
       case 42 : Printlog "- No Thesaurus available !"
                 goto endsub
       case 45 : sWord = "Hej"
       case 46 : sWord = "Välkommen"
       case 47 : Printlog "- No Thesaurus available !"
                 goto endsub
       case 48 : Printlog "- No Thesaurus in Polish!"
                 goto endsub
       case 49 : sWord = "Hallo"
       case 50 : sWord = "Prosojnica"
       case 51 : Printlog "- No Thesaurus available !"
                 goto endsub
       case 55 : Printlog "- No Thesaurus in Brazilian!"
                 goto endsub
       case 90 : Printlog "- No Thesaurus available !"
                 goto endsub

       case else : if bAsianLan then
                     hSetSpellHypLanguage
                     sWord = "Hello"
                  else
                     Warnlog "For the language  " + iSprache +" nothing is prepared yet: insert text here"
                     sWord = "Hello"
                  endif
    end select
    PrintLog "Thesaurus with 1 word: " + sWord
    if( gApplication = "DRAW" ) then
        sExt = ".odg"
    elseif( Ucase(gApplication) = "IMPRESS" ) then
        sExt = ".odp"
    endif

    hFileOpenLocally( gTesttoolpath + "graphics\required\input\recht_" + iSprache + sExt)
    Call hTextrahmenErstellen (sWord,20,20,50,30)
    sleep 1
    printlog "select the word"
    hTypeKeys "<Home><SHIFT END>"

    try
        ExtrasThesaurusDraw
    catch
        warnlog "No Thesaurus available."
        hCloseDocument()
        goto endsub
    endcatch

    Kontext "Thesaurus"
    if Thesaurus.Exists(3) then
        Thesaurus.Cancel
    else
         warnlog "the thesaurus does not appear"
    end if
    Call hCloseDocument
endcase 'tiToolsThesaurus

'-------------------------------------------------------------------------------

testcase tiToolsHyphenation
    printlog "open application "
    Call hNewDocument
    printlog "Tools->Hyphenation "
    ToolsLanguageHyphenationDraw
    WaitSlot (2000)
    printlog "Tools->Hyphenation "
    ToolsLanguageHyphenationDraw
    printlog "close application "
    Call hCloseDocument
endcase 'tiToolsHyphenation

'-------------------------------------------------------------------------------

testcase tiToolsAutoCorrect
    dim iLanguage as integer ' for resetting the language
    printlog "open application "
    Call hNewDocument
    WaitSlot (1000)   
    printlog "Tools->Autocorrect "
    ToolsAutocorrect
    WaitSlot (2000) 
    Kontext
    printlog "select tabpage 'Replace' "
    Messagebox.SetPage TabErsetzung
    Kontext "TabErsetzung"
    Call DialogTest ( TabErsetzung )
    printlog "remember the language, that is selected in the 'Replacements and exceptions for Language' Listbox "
    iLanguage = WelcheSprache.GetSelIndex
    printlog "+ change the language to the 1st from the top "
    WelcheSprache.Select 1 ' select language with empty list
    printlog "+ type something into the field 'replace' "
    Kuerzel.SetText "a"
    printlog "+ type something into the field 'with' "
    ErsetzenDurch.SetText "b"
    printlog "+ click button 'new' "
    Neu.Click
    sleep 1
    printlog "+ click button 'delete' "
    Loeschen.Click
    sleep 1
    try
        printlog "+ click button 'delete' again "
        Loeschen.Click
    catch
        printlog "ok was CRASH before"
    endcatch
    printlog "+ if nothing happens it is ok, was a Crash before :-( "
    printlog "+ restore cthe remembered language "
    WelcheSprache.select (iLanguage)
    Kontext
    printlog "select tabpage 'Exception' "
    Messagebox.SetPage TabAusnahmen
    Kontext "TabAusnahmen"
    Call DialogTest ( TabAusnahmen )
    printlog "type something into the field 'Abbreviations' "
    Abkuerzungen.settext "Lala"
    printlog "+ click button 'new' in 'Abbreviations' "
    AbkuerzungenNeu.click
    printlog "+ click button 'delete' in 'Abbreviations' "
    AbkuerzungenLoeschen.click
    printlog "type something into the field 'Word with TWo INitial CApitals' "
    Woerter.settext "LALA"
    printlog "+ check the checkbox 'AutoInclude' in 'Word with TWo INitial CApitals' "
    WoerterAutomatisch.Check
    printlog "+ click button 'new' in 'Word with TWo INitial CApitals' "
    WoerterNeu.click
    printlog "+ click button 'delete' in 'Word with TWo INitial CApitals' "
    WoerterLoeschen.click
    printlog "+ UNcheck the checkbox 'AutoInclude' in 'Word with TWo INitial CApitals' "
    WoerterAutomatisch.UnCheck
    Kontext
    printlog "select tabpage 'Options' "
    Messagebox.SetPage TabOptionen
    Kontext "TabOptionen"
    Call DialogTest ( TabOptionen )
    Kontext
    printlog "select tabpage 'Custom Quotes' "
    Messagebox.SetPage TabLocalizedOptions
    Kontext "TabLocalizedOptions"  ' 1a
    printlog "in the part of 'single quotes': "
    printlog "+ check 'replace' "
    printlog "+ click 'start quote' "
    SingleQuotesReplace.Check
    SingleQuotesStart.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 1 )
    printlog "+  cancel dialog 'start quote' "
    Sonderzeichen.Cancel
    Kontext "TabLocalizedOptions"  ' 1b
    printlog "+ click 'end quote' "
    SingleQuotesEnd.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 2 )
    printlog "+  cancel dialog 'end quote' "
    Sonderzeichen.Cancel
    Kontext "TabLocalizedOptions"  ' 1s
    printlog "+ click button 'default' "
    SingleQuotesDefault.Click

    Kontext "TabLocalizedOptions"  ' 2a
    printlog " in the part of 'double quotes': "
    printlog "+ click 'start quote' "
    DoubleQuotesStart.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 3 )
    printlog "+  cancel dialog 'start quote' "
    Sonderzeichen.Cancel
    Kontext "TabLocalizedOptions"  ' 2b
    printlog "+ click 'end quote' "
    DoubleQuotesEnd.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 4 )
    printlog "+  cancel dialog 'end quote' "
    Sonderzeichen.Cancel
    Kontext "TabLocalizedOptions"  ' 2s
    printlog "+ click button 'default' "
    DoubleQuotesDefault.Click
    printlog "+ UNcheck 'replace' "
    SingleQuotesReplace.UnCheck
    printlog " cancel dialog 'AtorCorrect' "
    TabLocalizedOptions.cancel
    printlog " close application "
    Call hCloseDocument

endcase 'tiToolsAutoCorrect
'-------------------------------------------------------------------------------

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

    printlog "Open application "
    Call hNewDocument
    WaitSlot (2000) 
    printlog "Save old state and Activate Support for Asian language "
    bSavedAsianSupport = ActiveDeactivateAsianSupport(TRUE)
    printlog "Open Document "
    Call hFileOpen ( ConvertPath(gTesttoolPath + sFileName) )
    printlog "If write-protected - open as Temp-file "
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
    printlog "Select all, Press RETURN to enter text in Editmode "
    printlog "Move marker to top of the text, go two steps right, mark two characters "
    EditSelectAll
    hTypeKeys "<RETURN>"
    hTypeKeys "<MOD1 HOME><RIGHT><RIGHT><SHIFT RIGHT RIGHT>"
    printlog "Open Chinesetranslation "
    ToolsChineseTranslation
    WaitSlot (2000)    'sleep 1
    kontext "ChineseTranslation"
    printlog " Check if everything is there "
    Call DialogTest ( ChineseTranslation )
    printlog " Click on EditTerms "
    EditTerms.Click
    kontext "ChineseDictionary"
    printlog " Check if everything is there "
    Call DialogTest ( ChineseDictionary )
    printlog " Close dialog 'ChineseDictionary' with 'OK' "
    ChineseDictionary.Ok
    kontext "ChineseTranslation"
    printlog " Close dialog 'Chinese' with 'OK' "
    ChineseTranslation.OK
    kontext
    printlog " if messagebox exist, say OK; ('Spellcheck completed' or 'Word not found') "
    if Messagebox.exists (5) then
        printlog "Messagebox: "+Messagebox.gettext+"'"
        Messagebox.ok
    end if
    printlog " Restore old state for Asian language "
    ActiveDeactivateAsianSupport(bSavedAsianSupport)
    printlog " Close application "
    Call hCloseDocument

endcase 'tChineseTranslation
'-------------------------------------------------------------------------------

testcase tiToolsMacro
    printlog "open application "
    Call hNewDocument
    WaitSlot (2000)    'sleep 2
    printlog "Tools->Macro "
    ToolsMacro
    Kontext "Makro"
    Call DialogTest ( Makro )
    printlog "click button 'organizer...' "
    Verwalten.Click
    Kontext
    printlog "switch to tabpage 'Modules' "
    Messagebox.SetPage TabModule
    Kontext "TabModule"
    Call DialogTest ( TabModule )
    Kontext
    printlog "switch to tabpage 'Libraries' "
    Messagebox.SetPage TabBibliotheken
    Kontext "TabBibliotheken"
    Call DialogTest ( TabBibliotheken )
    printlog "click lbutton 'append' "
    Hinzufuegen.Click
    Kontext "Messagebox"
    if Messagebox.Exists (5) then
        if Messagebox.GetRT = 304 then
            Warnlog Messagebox.Gettext
            Messagebox.Ok
        end if
    end if
    Kontext "OeffnenDlg"
    printlog "cancel dialog 'append libraries' "
    OeffnenDlg.Cancel
    Kontext "TabBibliotheken"
    printlog "click button 'new' "
    Neu.Click
    kontext "NeueBibliothek"
    sleep 1 'Bibliotheksname
    printlog "cancel dialog 'new library' "
    NeueBibliothek.cancel
    Kontext "TabBibliotheken"
    printlog "close dialog 'macro organizer' "
    TabBibliotheken.Close

    Kontext "Makro"
    printlog "close dialog 'macro' "
    Makro.Cancel
    printlog "close application "
    Call hCloseDocument
endcase 'tiToolsMacro

'-------------------------------------------------------------------------------

testcase tiToolsGallery
    printlog "open application "
    Call hNewDocument
    printlog "Tools->Gallery "
    ToolsGallery
    WaitSlot (2000)    'sleep 1
    printlog "Tools->Gallery "
    ToolsGallery
    printlog "close application "
    Call hCloseDocument
endcase'tiToolsGallery

'-------------------------------------------------------------------------------

testcase tiToolsEyedropper
    printlog "open application "
    Call hNewDocument
    printlog "Tools->Eyedropper "
    ToolsEyedropper
    Kontext "Pipette"
        Call DialogTest (Pipette)
        printlog "close dialog 'Eyedropper' "
        Pipette.Close
        sleep 1
    printlog "close application "
    Call hCloseDocument
endcase 'tiToolsEyedropper

