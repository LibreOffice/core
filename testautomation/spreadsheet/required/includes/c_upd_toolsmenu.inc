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
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : Resource Test - Tools Menu Part I
'*
'\***********************************************************************

sub c_upd_toolsmenu

    Printlog Chr(13) + "--------- Tools Menu Part I (c_upd_toolsmenu.inc) ---------"
    
    call tToolsSpellcheck
    call tToolsHangulHanjaConversion
    call tToolsLanguageChineseTranslation
    call tToolsLanguageThesaurus
    call tToolsLanguageHyphenation
    call tToolsDetective   
    call tToolsAutoCorrect
    call tToolsGoalSeek
    call tToolsScenario
    call tToolsProtectSheet
    call tToolsProtectDocument
    call tToolsCellContents
    call tToolsGallery
    call tToolsMediaPlayer

end sub

'-----------------------------------------------------------

testcase tToolsSpellcheck
'///<u><b>Tools - Spellcheck</b></u>

    Dim iDictionaries as integer
    
    printlog " Load document from 'gTesttoolPath + spreadsheet\required\input\english.ods"
    hFileOpenLocally( gTesttoolPath & "spreadsheet\required\input\english.ods" )
    '/// Enter 'fgsdftk' in a cell
    printlog " Enter 'fgsdftk' in a cell"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "fgsdftk<Return><Up>"
    '/// Open 'Spellcheck' – dialog via 'Tools – Spellcheck'
    printlog " Open 'Spellcheck' – dialog via 'Tools – Spellcheck'"
    try
        ToolsSpellcheck
    catch
        'for some languages there is no spellchecker available.
        warnlog "Spellchecking not possible"
        call hCloseDocument
        goto endsub
    endcatch 
    ' If 'blabla' is already in the dictionary (because of a former failed testrun)
    ' there will be a messagebox that the spellchecking has finished. The test will abort.
    Kontext "Active"
    If Active.Exists then
        warnlog "It seems that a former testrun has failed, test will abort"
        printlog "Messagebox: " & Active.GetText
        Active.OK
        call hCloseDocument
        goto endsub
    end if
    Kontext "Spellcheck"
    DialogTest ( Spellcheck )
    '/// Verify that the 'Add' – button shows dictionaries
    printlog " Verify that the 'Add' – button shows dictionaries"
    qaErrorlog "Due to problems on Sols the AddToDictionary-Button is not tested"
'    try
'        AddToDictionary.Click
'        sleep 1
'        If hMenuItemGetCount < 1 then
'            warnlog "There are no dictionaries available"
'        end if
'    catch
'        warnlog "The menu on the ADD button could not be opened"
'    endcatch
    '/// Verify that the 'Dictionary language' – combobox has items
    printlog " Verify that the 'Dictionary language' – combobox has items"
    If DictionaryLanguage.GetItemCount < 1 then
        warnlog "There are no languages available"
    end if
    '/// Open 'Options'- dialog by clicking 'Options' – button
    printlog " Open 'Options'- dialog by clicking 'Options' – button"
    SpellcheckOptions.Click
    Kontext "TabLinguistik"
    DialogTest ( TabLinguistik )
    'Memorize the amount of available dictionaries
    iDictionaries = Benutzerwoerterbuch.GetItemCount
    Kontext "TabLinguistik"
    '/// Open 'New Dictionary' – dialog by clicking 'New' – button
    printlog " Open 'New Dictionary' – dialog by clicking 'New' – button"
    Neu.Click
    Kontext "BenutzerwoerterbuchAnlegen"
    Call DialogTest ( BenutzerwoerterbuchAnlegen )
    '/// Verify that the 'Language' combobox has items
    printlog " Verify that the 'Language' combobox has items"
    If Sprache.GetItemCount < 1 then
        warnlog "There are no languages available"
    end if
    '/// Enter name 'Testdictionary'
    printlog " Enter name 'Testdictionary'"
    Woerterbuchname.TypeKeys "Testdictionary"
    '/// Uncheck 'Exception'
    printlog " Uncheck 'Exception'"
    Ausnahme.UnCheck
    '/// Close dialog with 'OK'
    printlog " Close dialog with 'OK'"
    BenutzerwoerterbuchAnlegen.OK
    Kontext "TabLinguistik"
    '/// Select 'Testdictionary'
    Benutzerwoerterbuch.Select (iDictionaries + 1)
    '/// Click on 'Edit' – button to open 'Edit Custom Dictionary' – dialog
    printlog " Click on 'Edit' – button to open 'Edit Custom Dictionary' – dialog"
    Bearbeiten.Click
    Kontext "BenutzerwoerterbuchBearbeiten"
    Call DialogTest ( BenutzerwoerterbuchBearbeiten )
    '/// Verify that the 'Book' combobox has items
    printlog " Verify that the 'Book' combobox has items"
    If Buch.GetItemCount < 1 then
        warnlog "There are no dictionaries available"
    end if
    '/// Verify that the 'Language' combobox has items
    printlog " Verify that the 'Language' combobox has items"
    If Sprache.GetItemCount < 1 then
        warnlog "There are no languages available"
    end if
    '/// Close dialog with 'Close'
    printlog " Close dialog with 'Close'"
    BenutzerwoerterbuchBearbeiten.Close
    Kontext "TabLinguistik"
    '/// Click on 'Delete' button
    printlog " Click on 'Delete' button"
    Loeschen.Click
    '/// Close messageboc with 'Yes'
    printlog " Close messageboc with 'Yes'"
    Kontext "Active"
    If Active.Exists then
        Active.Yes
    end if
    Kontext "TabLinguistik"
    '/// Select ninth option (Minimal number of ...)
    printlog " Select ninth option (Minimal number of ...)"
    Optionen.Select (9)
    '/// Click on 'Edit' – button
    printlog " Click on 'Edit' – button"
    OptionenBearbeiten.Click
    Kontext "HyphenationOptions"
    Call DialogTest ( HyphenationOptions )
    '/// Decrease 'Minimal word length'
    printlog " Decrease 'Minimal word length'"
    ZeichenVorUmbruch.More
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    HyphenationOptions.Cancel
    Kontext "TabLinguistik"
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    TabLinguistik.Cancel
    Kontext "Spellcheck"
    '/// Close 'Spellcheck' – dialog with 'Close'
    printlog " Close 'Spellcheck' – dialog with 'Close'"
    Spellcheck.Close
    '/// Close starting document
    printlog " Close starting document"
    call hCloseDocument
    hFileDelete( gLastWorkFile )

endcase

'-----------------------------------------------------------

testcase tToolsHangulHanjaConversion
'///<u><b>Tools - Hangul/Hanja Conversion</b></u>

    Dim bAsianLanguage as boolean
    Dim bCTLlanguage as boolean

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enable language support for CJK and CTL (Tools-Options-Language Support)
    printlog " Enable language support for CJK and CTL (Tools-Options-Language Support)"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    bCTLLanguage = ActiveDeactivateCTLSupport(TRUE)
    '/// Load document from 'gTesttoolPath + "spreadsheet\required\input\korean.ods"'
    printlog " Load document from 'gTesttoolPath + spreadsheet\required\input\korean.ods"
    hFileOpenLocally( gTesttoolPath & "spreadsheet\required\input\korean.ods" )
    '/// Select cell B1
    printlog " Select cell B1"
    Call fCalcSelectRange ("B1")
    '/// Open 'Hangul/Hanja Conversion' dialog via 'Tools – Language – Hangul/Hanja Conversion'
    printlog " Open 'Hangul/Hanja Conversion' dialog via 'Tools – Language – Hangul/Hanja Conversion'"
    ToolsLanguageHangulHanjaConversion
    Kontext "HangulHanjaConversion"
    if HangulHanjaConversion.Exists (5) then
        Call DialogTest ( HangulHanjaConversion )
    else 
        call hCloseDocument
        goto endsub
    end if
    '/// Click on 'Options'  button to open 'Hangul/Hanja Options' dialog
    printlog " Click on 'Options'  button to open 'Hangul/Hanja Options' dialog"
    Options.Click
    Kontext "HangulHanjaOptions"
    if HangulHanjaOptions.Exists (5) then
        Call DialogTest ( HangulHanjaOptions )
    else 
        warnlog " Hangul/Hanja Options' dialog could not be open"
        HangulHanjaConversion.Cancel
        call hCloseDocument
        goto endsub
    end if
    '/// Click on 'New' button
    printlog " Click on 'New' button"
    NewDictionary.Click
    Kontext "HangulHanjaNewDictionary"
    if HangulHanjaNewDictionary.Exists (5) then
        Call DialogTest ( HangulHanjaNewDictionary )
    else 
        warnlog " HangulHanjaNewDictionary dialog could not be opened"
        Kontext "HangulHanjaOptions"
        HangulHanjaOptions.Cancel
        Kontext "HangulHanjaConversion"
        HangulHanjaConversion.Cancel
        call hCloseDocument
        goto endsub
    end if
    '/// Enter string 'Dictionary_added_by_Testtool'
    printlog " Enter string 'Dictionary_added_by_Testtool'"
    DictionaryName.Settext "Dictionary_added_by_Testtool"
    '/// Close dialog with 'OK'
    printlog " Close dialog with 'OK'"
    HangulHanjaNewDictionary.OK
    sleep 1
    Kontext "HangulHanjaOptions"
    '/// Click on 'Edit' button
    printlog " Click on 'Edit' button"
    if HangulHanjaOptions.Exists (5) then
        EditDictionary.Click
        sleep 1
    else 
        warnlog " Edit button could not be accessed"
        HangulHanjaConversion.Cancel
        call hCloseDocument
        goto endsub
    end if
    Kontext "HangulHanjaEditDictionary"
    if HangulHanjaEditDictionary.Exists (5) then
        Call DialogTest ( HangulHanjaEditDictionary )
    else 
        warnlog " HangulHanjaEditDictionary dialog could not be opened"
        Kontext "HangulHanjaOptions"
        HangulHanjaOptions.Cancel
        Kontext "HangulHanjaConversion"
        HangulHanjaConversion.Cancel
        call hCloseDocument
        goto endsub
    end if
    '/// Close 'Edit Custom Dictionary' dialog with 'Close'
    printlog " Close 'Edit Custom Dictionary' dialog with 'Close'"
    HangulHanjaEditDictionary.Close
    Kontext "HangulHanjaOptions"
    '/// Click on 'Delete' button
    printlog " Click on 'Delete' button"
    DeleteDictionary.Click
    '/// UnCheck 'Ignore post-positional words'
    printlog " UnCheck 'Ignore post-positional words'"
    IgnorePostPositionalWord.UnCheck
    '/// Check 'Show entries'
    printlog " Check 'Show entries'"
    ShowEntries.Check
    '/// Check 'Replace Unique Entries'
    printlog " Check 'Replace Unique Entries'"
    ReplaceUniqueEntries.Check
    '/// Close 'Hangul/Hanja Options' dialog with 'Cancel'
    printlog " Close 'Hangul/Hanja Options' dialog with 'Cancel'"
    HangulHanjaOptions.Cancel
    Kontext "HangulHanjaConversion"
    '/// Close 'Hangul/Hanja Conversion' dialog with 'Close'
    printlog " Close 'Hangul/Hanja Conversion' dialog with 'Close'"
    HangulHanjaConversion.Close
    '/// Set support for CJK and CTL back to default (Tools – Options – Language Support)
    printlog " Set support for CJK and CTL back to default (Tools – Options – Language Support)"
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if
    if bCTLLanguage = FALSE then
        Call ActiveDeactivateCTLSupport(FALSE)
    end if
    '/// Close document
    printlog " Close document"
    call hCloseDocument
    hFileDelete( gLastWorkFile )

endcase

'-----------------------------------------------------------

testcase tToolsLanguageChineseTranslation
'///<u><b>Tools – Language – Chinese Translation</b></u>

    Dim bAsianLanguage as boolean
    Dim bCTLlanguage as boolean

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enable language support for CJK and CTL (Tools-Options-Language Support)
    printlog " Enable language support for CJK and CTL (Tools-Options-Language Support)"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    bCTLLanguage = ActiveDeactivateCTLSupport(TRUE)
    '/// Open 'Chinese Translation' dialog by 'Tools – Language – Chinese Translation'
    printlog " Open 'Chinese Translation' dialog by 'Tools – Language – Chinese Translation'"
    ToolsLanguageChineseTranslation
    Kontext "ChineseTranslation"
    Call DialogTest ( ChineseTranslation )
    '/// Click on 'Edit Terms' button to open 'Edit Dictionary' dialog
    printlog " Click on 'Edit Terms' button to open 'Edit Dictionary' dialog"
    EditTerms.Click
    Kontext "ChineseDictionary"
    Call DialogTest ( ChineseDictionary )
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    ChineseDictionary.Cancel
    Kontext "ChineseTranslation"
    '/// Close 'Chinese Translation' dialog with 'Cancel'
    printlog " Close 'Chinese Translation' dialog with 'Cancel'"
    ChineseTranslation.Cancel
    '/// Set support for CJK and CTL back to default (Tools – Options – Language Support)
    printlog " Set support for CJK and CTL back to default (Tools – Options – Language Support)"
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if
    if bCTLLanguage = FALSE then
        Call ActiveDeactivateCTLSupport(FALSE)
    end if
    '/// Close document
    printlog " Close document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tToolsLanguageThesaurus
'///<u><b>Tools – Language – Chinese Translation</b></u>

    printlog " Load document from 'gTesttoolPath + spreadsheet\required\input\english.ods"
    hFileOpenLocally( gTesttoolPath & "spreadsheet\required\input\english.ods" )
    Kontext "DocumentCalc"
    printlog " Enter 'test' in a cell"
    Call fCalcSelectRange ("B2")
    DocumentCalc.TypeKeys "test<Return><Up>"
    printlog " Open 'Thesaurus' dialog by 'Tools – Language – Thesaurus"
    try
        ToolsLanguageThesaurus
    catch
        warnlog "Thesaurus did not work"
        call hCloseDocument
        goto endsub
    endcatch
    Kontext "Thesaurus"
    DialogTest ( Thesaurus )
    printlog " Check if the cellcontent test is correctly read"
    if CurrentWord.GetSelText <> "test" then
        warnlog "The cellcontent is not test but is " & CurrentWord.GetSelText
    end if
    printlog " Check that there are alternatives available"
    if Alternatives.GetItemCount < 1 then
        warnlog "There are no alternatives available"
    else
        Alternatives.Select (1)
        sleep (1)
        if ReplaceWith.GetText = "" then
            warnlog "The repacle with field is empty"
        end if
    end if
    printlog " Close 'Thesaurus' with 'Cancel'"
    Thesaurus.Cancel
    printlog " Close document"
    call hCloseDocument
    hFileDelete( gLastWorkFile )

endcase

'-----------------------------------------------------------

testcase tToolsLanguageHyphenation
'///<u><b>Tools – Language – Hyphenation</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Enter 'blabla' in a cell
    printlog " Enter 'blabla' in a cell"
    Call fCalcSelectRange ("B2")
    DocumentCalc.TypeKeys "blabla<Return><Up>"
    '/// Use 'Tools – Language – Hyphenation' to open 'Format Cells' dialog
    printlog " Use 'Tools – Language – Hyphenation' to open 'Format Cells' dialog"
    ToolsLanguageHyphenateCalc
    Kontext "TabAusrichtung"
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    TabAusrichtung.Cancel
    '/// Close document
    printlog " Close document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tToolsDetective
'///<u><b>Tools – Detective</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enter '1' in cell A1
    printlog " Enter '1' in cell A1"
    Call fCalcSelectRange ("A1")
    DocumentCalc.TypeKeys "1<Return>"
    '/// Enter '=A1' in cell C4
    printlog " Enter '=A1' in cell C4"
    Call fCalcSelectRange ("C4")
    DocumentCalc.TypeKeys "=A1<Return>"
    '/// Select cell C4
    printlog " Select cell C4"
    Call fCalcSelectRange ("C4")
    '/// Enable 'Trace Precedents' by 'Tools – Detective – Trace Precedents'
    printlog " Enable 'Trace Precedents' by 'Tools – Detective – Trace Precedents'"
    ToolsDetectiveTracePrecedents
    '/// Disable 'Trace Precedents' by 'Tools – Detective – Remove Precedents'
    printlog " Disable 'Trace Precedents' by 'Tools – Detective – Remove Precedents'"
    ToolsDetectiveRemovePrecedents
    '/// Select cell A1
    printlog " Select cell A1"
    Call fCalcSelectRange ("A1")
    '/// Enable 'Trace Dependents' by 'Tools – Detective – Trace Dependents'
    printlog " Enable 'Trace Dependents' by 'Tools – Detective – Trace Dependents'"
    ToolsDetectiveTraceDependents
    '/// Disable 'Trace Dependents' by 'Tools – Detective – Remove Dependents'
    printlog " Disable 'Trace Dependents' by 'Tools – Detective – Remove Dependents'"
    ToolsDetectiveRemoveDependents
    '/// Remove 'All Traces' by 'Tools – Detective – Remove All Traces'
    printlog " Remove 'All Traces' by 'Tools – Detective – Remove All Traces'"
    ToolsDetectiveRemoveAllTraces
    '/// Enter '=C4/0' in cell C7
    printlog " Enter '=C4/0' in cell C7"
    Call fCalcSelectRange ("C7")
    DocumentCalc.TypeKeys "=C4/0<Return>"
    '/// Select cell C7
    printlog " Select cell C7"
    Call fCalcSelectRange ("C7")
    '/// Enable 'Trace Error' by 'Tools – Detective – Trace Error'
    printlog " Enable 'Trace Error' by 'Tools – Detective – Trace Error'"
    ToolsDetectiveTraceError
    '/// Enable 'Mark Invalid Data' by 'Tools – Detective – Mark Invalid Data'
    printlog " Enable 'Mark Invalid Data' by 'Tools – Detective – Mark Invalid Data'"
    ToolsDetectiveMarkInvalidData
    '/// Refresh traces by 'Tools – Detective – Refresh Traces'
    printlog " Refresh traces by 'Tools – Detective – Refresh Traces'"
    ToolsDetectiveRefreshTraces
    '/// Switch mode of autorefreshing by 'Tools – Detective – AutoRefresh'
    printlog " Switch mode of autorefreshing by 'Tools – Detective – AutoRefresh'"
    ToolsDetectiveAutoUpdate
    '/// Switch mode back to default for autorefreshing by 'Tools – Detective – AutoRefresh'
    printlog " Switch mode back to default for autorefreshing by 'Tools – Detective – AutoRefresh'"
    ToolsDetectiveAutoUpdate
    '/// Switch 'Fill Mode' by 'Tools – Detective – Fill Mode'
    printlog " Switch 'Fill Mode' by 'Tools – Detective – Fill Mode'"
    ToolsDetectiveFillMode
    '/// Switch 'Fill Mode' back to default by 'Tools – Detective – Fill Mode'
    printlog " Switch 'Fill Mode' back to default by 'Tools – Detective – Fill Mode'"
    ToolsDetectiveFillMode
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tToolsAutoCorrect
'///<u><b>Tools – AutoCorrect</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Open 'AutoCorrect' dialog by 'Tools – AutoCorrect'
    printlog " Open 'AutoCorrect' dialog by 'Tools – AutoCorrect'"
    ToolsAutocorrect
    '/// Switch to tabpage 'Replace'
    printlog " Switch to tabpage 'Replace'"
    Kontext
    active.SetPage TabErsetzung
    Kontext "TabErsetzung"
    Call DialogTest ( TabErsetzung )
    '/// Verify that the language combobox has items
    printlog " Verify that the language combobox has items"
    if WelcheSprache.GetItemCount < 1 then
        warnlog "The Language listbox is empty"
    end if
    '/// Enter string 'Testerentrie' in 'Replace' field
    printlog " Enter string 'Testerentrie' in 'Replace' field"
    Kuerzel.SetText "Testerentrie"
    '/// Enter string 'Testentrie' in 'With' field
    printlog " Enter string 'Testentrie' in 'With' field"
    ErsetzenDurch.SetText "Testentrie"
    '/// Click on 'New' button
    printlog " Click on 'New' button"
    Neu.Click
    '/// Click on 'Delete' button
    printlog " Click on 'Delete' button"
    Loeschen.Click
    '/// Switch to tabpage 'Exceptions'
    printlog " Switch to tabpage 'Exceptions'"
    Kontext
    active.SetPage TabAusnahmen
    Kontext "TabAusnahmen"
    Call DialogTest ( TabAusnahmen )
    '/// Enter string 'olala' as 'Abbreviation'
    printlog " Enter string 'olala' as 'Abbreviation'"
    Abkuerzungen.SetText "olala"
    '/// Press 'New' button
    printlog " Press 'New' button"
    AbkuerzungenNeu.click
    '/// Press 'Delete' button
    printlog " Press 'Delete' button"
    AbkuerzungenLoeschen.click
    '/// Enter string 'TTLauncher' in 'Words with ...'
    printlog " Enter string 'TTLauncher' in 'Words with ...'"
    Woerter.SetText "TTLauncher"
    '/// Press 'New' button
    printlog " Press 'New' button"
    WoerterNeu.click
    '/// Press 'Delete' button
    printlog " Press 'Delete' button"
    WoerterLoeschen.click
    '/// Switch to tabpage 'Options'
    printlog " Switch to tabpage 'Options'"
    Kontext
    active.SetPage TabOptionen
    Kontext "TabOptionen"
    Call DialogTest ( TabOptionen )
    '/// Switch to tabpage 'Custom Quotes'
    printlog " Switch to tabpage 'Custom Quotes'"
    Kontext
    active.SetPage TabLocalizedOptions
    Kontext "TabLocalizedOptions"
    Call DialogTest ( TabLocalizedOptions )
    '/// Press on button for 'Start quote' for 'Single quotes'
    printlog " Press on button for 'Start quote' for 'Single quotes'"
    SingleQuotesStart.Click
    Kontext "Sonderzeichen"
    '/// Close 'Start quote' dialog with 'Cancel'
    printlog " Close 'Start quote' dialog with 'Cancel'"
    Sonderzeichen.Cancel
    Kontext "TabLocalizedOptions"
    '/// Press on button for 'End quote' for 'Single quotes'
    printlog " Press on button for 'End quote' for 'Single quotes'"
    SingleQuotesEnd.Click
    Kontext "Sonderzeichen"
    '/// Close 'End quote' dialog with 'Cancel'
    printlog " Close 'End quote' dialog with 'Cancel'"
    Sonderzeichen.Cancel
    Kontext "TabLocalizedOptions"
    '/// Click on 'Default' button for 'Single quotes'
    printlog " Click on 'Default' button for 'Single quotes'"
    SingleQuotesDefault.Click
    '/// Press on button for 'Start quote' for 'Double quotes'
    printlog " Press on button for 'Start quote' for 'Double quotes'"
    DoubleQuotesStart.Click
    Kontext "Sonderzeichen"
    '/// Close 'Start quote' dialog with 'Cancel'
    printlog " Close 'Start quote' dialog with 'Cancel'"
    Sonderzeichen.Cancel
    Kontext "TabLocalizedOptions"
    '/// Press on button for 'End quote' for 'Double quotes'
    printlog " Press on button for 'End quote' for 'Double quotes'"
    DoubleQuotesEnd.Click
    Kontext "Sonderzeichen"
    '/// Close 'End quote' dialog with 'Cancel'
    printlog " Close 'End quote' dialog with 'Cancel'"
    Sonderzeichen.Cancel
    Kontext "TabLocalizedOptions"
    '/// Click on 'Default' button for 'Double quotes'
    printlog " Click on 'Default' button for 'Double quotes'"
    DoubleQuotesDefault.Click
    '/// Close 'AutoCorrect' dialog with 'Cancel'
    printlog " Close 'AutoCorrect' dialog with 'Cancel'"
    TabLocalizedOptions.cancel
    '/// Close document
    printlog " Close document"
    call hCloseDocument 

endcase

'-----------------------------------------------------------

testcase tToolsGoalSeek
'///<u><b>Tools - Goal Seek</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Goal Seek' dialog by 'Tools – Goal Seek'
    printlog " Open 'Goal Seek' dialog by 'Tools – Goal Seek'"
    ToolsGoalSeek
    Kontext "Zielwertsuche"
    DialogTest ( Zielwertsuche )
    '/// Click on 'Shrink' button for 'Formula Cell'
    printlog " Click on 'Shrink' button for 'Formula Cell'"
    Vorgaben1.Click
    '/// Close rangechooser with 'Shrink' button
    printlog " Close rangechooser with 'Shrink' button"
    Vorgaben1.Click
    '/// Click on 'Shrink' button for 'Variable Cell'
    printlog " Click on 'Shrink' button for 'Variable Cell'"
    Vorgaben2.Click
    '/// Close rangechooser with 'Shrink' button
    printlog " Close rangechooser with 'Shrink' button"
    Vorgaben2.Click
    '/// Close 'Goal Seek' dialog with 'Cancel'
    printlog " Close 'Goal Seek' dialog with 'Cancel'"
    Zielwertsuche.Cancel
    '/// Close document
    printlog " Close document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tToolsScenario
'///<u><b>Tools - Scenario</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enter '1' in A1, '2' in A2, '3' in A3
    printlog " Enter '1' in A1, '2' in A2, '3' in A3"
    Call fCalcSelectRange ("A1")
    DocumentCalc.TypeKeys "1<Return>2<Return>3<Return>"
    '/// Select cells A1:A3
    printlog " Select cells A1:A3"
    Call fCalcSelectRange ("A1:A3")
    '/// Open 'Create Scenario' dialog by 'Tools – Scenario'
    printlog " Open 'Create Scenario' dialog by 'Tools – Scenario'"
    ToolsScenarios
    Kontext "SzenarioAnlegen"
    DialogTest ( SzenarioAnlegen )
    '/// Enter 'Scenario_by_Testtool' as 'Name of Scenario'
    printlog " Enter 'Scenario_by_Testtool' as 'Name of Scenario'"
    SzenarioName.SetText "Scenario_by_Testtool"
    '/// Enter 'Comment_by_Testtool' as 'Comment'
    printlog " Enter 'Comment_by_Testtool' as 'Comment'"
    Kommentar.SetText "Comment_by_Testtool"
    '/// Verify that the color combobox has items
    printlog " Verify that the color combobox has items"
    If Rahmenfarbe.GetItemCount < 1 then
        warnlog "The colorbox is empty"
    end if
    '/// Check 'Display border'
    printlog " Check 'Display border'"
    RahmenAnzeigen.Check
    '/// Check 'Copy back'
    printlog " Check 'Copy back'"
    Zurueckkopieren.Check
    '/// UnCheck 'Copy entire sheet'
    printlog " UnCheck 'Copy entire sheet'"
    GanzeTabelle.UnCheck
    '/// Check 'PreventChanges'
    printlog " Check 'PreventChanges'"
    PreventChanges.Check
    '/// Close 'Create Scenario' dialog with 'Cancel'
    printlog " Close 'Create Scenario' dialog with 'Cancel'"
    SzenarioAnlegen.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tToolsProtectSheet
'///<u><b>Tools - Protect – Sheet</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Protect Sheet' dialog by 'Tools – Protect – Sheet'
    printlog " Open 'Protect Sheet' dialog by 'Tools – Protect – Sheet'"
    ToolsProtectDocumentSheet
    Kontext "TabelleSchuetzen"
    DialogTest ( TabelleSchuetzen,1 )
    '/// Enter 'Secret' as 'Password'
    printlog " Enter 'Secret' as 'Password'"
    Passwort.SetText "Secret"
    '/// Enter 'Secred' as 'Confirm'
    printlog " Enter 'Secred' as 'Confirm'"
    Bestaetigung.SetText "Secred"
    '/// Press 'OK' button
    printlog " Press 'OK' button"
    TabelleSchuetzen.Ok
    '/// Close errormessage with 'OK'
    printlog " Close errormessage with 'OK'"
    Kontext "Active"
    if Active.Exists then
       DialogTest ( Active )
       Active.OK
    else
        warnlog "There is no error message by confirming a wrong password!"
    end if
    '/// Enter 'Secret' as 'Confirm'
    printlog " Enter 'Secret' as 'Confirm'"
    Kontext "TabelleSchuetzen"
    Bestaetigung.SetText "Secret"
    '/// Press 'OK' button
    printlog " Press 'OK' button"
    TabelleSchuetzen.Ok
    '/// Open 'Unprotect Sheet' dialog by 'Tools - Protect – Sheet'
    printlog " Open 'Unprotect Sheet' dialog by 'Tools - Protect – Sheet'"
    ToolsProtectDocumentSheet
    Kontext "TabelleSchuetzen"
    DialogTest ( TabelleSchuetzen,2 )
    '/// Enter string 'Secret'
    printlog " Enter string 'Secret'"
    Passwort.setText "Secret"
    '/// Press 'OK' button
    printlog " Press 'OK' button"
    TabelleSchuetzen.Ok
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tToolsProtectDocument
'///<u><b>Tools - Protect – Document</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Protect Document' dialog by 'Tools – Protect – Document'
    printlog " Open 'Protect Document' dialog by 'Tools – Protect – Document'"
    ToolsProtectDocumentDocument
    Kontext "DokumentSchuetzen"
    DialogTest ( DokumentSchuetzen,1 )
    '/// Enter 'Secret' as 'Password'
    printlog " Enter 'Secret' as 'Password'"
    Passwort.setText "Secret"
    '/// Enter 'Secred' as 'Confirm'
    printlog " Enter 'Secred' as 'Confirm'"
    Bestaetigung.SetText "Secred"
    '/// Press 'OK' button
    printlog " Press 'OK' button"
    DokumentSchuetzen.ok
    '/// Close errormessage with 'OK'
    printlog " Close errormessage with 'OK'"
    Kontext "Active"
    if Active.Exists then
       DialogTest ( Active )
       Active.OK
    else
        warnlog "There is no error message by confirming a wrong password!"
    end if
    Kontext "DokumentSchuetzen"
    '/// Enter 'Secret' as 'Confirm'
    printlog " Enter 'Secret' as 'Confirm'"
    Bestaetigung.SetText "Secret"
    '/// Press 'OK' button
    printlog " Press 'OK' button"
    DokumentSchuetzen.OK
    '/// Open 'Unprotect Document' dialog by 'Tools - Protect – Document'
    printlog " Open 'Unprotect Document' dialog by 'Tools - Protect – Document'"
    ToolsProtectDocumentDocument
    Kontext "DokumentSchuetzen"
    DialogTest ( DokumentSchuetzen,2 )
    '/// Enter string 'Secret'
    printlog " Enter string 'Secret'"
    Passwort.setText "Secret"
    '/// Press 'OK' button
    printlog " Press 'OK' button"
    DokumentSchuetzen.OK
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tToolsCellContents
'///<u><b>Tools - Cell Contents</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Recalculate cells by 'Tools – Cell Contents – Recalculate'
    printlog " Recalculate cells by 'Tools – Cell Contents – Recalculate'"
    ToolsCellContentsRecalculate
    '/// Switch 'AutoCalculate' mode by  'Tools – Cell Contents – AutoCalculate'
    printlog " Switch 'AutoCalculate' mode by  'Tools – Cell Contents – AutoCalculate'"
    ToolsCellContentsAutoCalculate
    '/// Switch 'AutoCalculate' mode back by  'Tools – Cell Contents – AutoCalculate'
    printlog " Switch 'AutoCalculate' mode back by  'Tools – Cell Contents – AutoCalculate'"
    ToolsCellContentsAutoCalculate
    '/// Switch 'AutoInput' mode by  'Tools – Cell Contents – AutoInput'
    printlog " Switch 'AutoInput' mode by  'Tools – Cell Contents – AutoInput'"
    ToolsCellContentsAutoInput
    '/// Switch 'AutoInput' mode back by  'Tools – Cell Contents – AutoInput'
    printlog " Switch 'AutoInput' mode back by  'Tools – Cell Contents – AutoInput'"
    ToolsCellContentsAutoInput
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tToolsGallery
'///<u><b>Tools - Gallery</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enable 'Gallery' by 'Tools – Gallery'
    printlog " Enable 'Gallery' by 'Tools – Gallery'"
    Kontext "Gallery"
	if Not Gallery.Exists then ToolsGallery
    Kontext "Gallery"
    '/// Verify that there are 'Themes' available (more than one)
    printlog " Verify that there are 'Themes' available (more than one)"
    If Gallerys.getItemCount < 1 then
        warnlog "There are no items in the Gallerylist"
    end if
    '/// Click on 'New Theme' button
    printlog " Click on 'New Theme' button"
    NewTheme.Click
    '/// Switch to tabpage 'General'
    printlog " Switch to tabpage 'General'"
    Kontext
    Active.SetPage TabAllgemeinGallery
    Kontext "TabAllgemeinGallery"
    Call DialogTest ( TabAllgemeinGallery )
    '/// Switch to tabpage 'Files'
    printlog " Switch to tabpage 'Files'"
    Kontext
    Active.SetPage TabDateien
    Kontext "TabDateien"
    Call DialogTest ( TabDateien )
    '/// Verify that the 'File type' combobox has items
    printlog " Verify that the 'File type' combobox has items"
    If Dateityp.GetItemCount < 1 then
        warnlog "There are no filetypes available"
    end if
    '/// Click on 'Find Files' button
    printlog " Click on 'Find Files' button"
    Suche.Click
    Kontext "OeffnenDlg"
    '/// Close 'Select Path' dialog with 'Cancel'
    printlog " Close 'Select Path' dialog with 'Cancel'"
    OeffnenDlg.Cancel
    Kontext "TabDateien"
    '/// Click on 'Add' button to open 'Gallery' dialog
    printlog " Click on 'Add' button to open 'Gallery' dialog"
    Hinzufuegen.Click
    Kontext "GrafikEinfuegenDlg"
    '/// Close 'Gallery' dialog with 'Cancel'
    printlog " Close 'Gallery' dialog with 'Cancel'"
    GrafikEinfuegenDlg.Cancel
    Kontext "TabDateien"
    '/// Close 'Properties' dialog with 'Cancel'
    printlog " Close 'Properties' dialog with 'Cancel'"
    TabDateien.Cancel
    '/// Disable 'Gallery' by 'Tools – Gallery'
    printlog " Disable 'Gallery' by 'Tools – Gallery'"
    ToolsGallery
    Kontext "Gallery"
    if Gallery.Exists then 
        Warnlog "The Gallery has not been closed!"
    end if
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tToolsMediaPlayer
'///<u><b>Tools - Media Player</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Media Player' by 'Tools – Media Player'
    printlog " Open 'Media Player' by 'Tools – Media Player'"
    ToolsMediaPlayer
    Kontext "Mplayer"
    Call DialogTest ( Mplayer )
    '/// Click on 'Open' button
    printlog " Click on 'Open' button"
    Mopen.Click
    Kontext "OeffnenDlg"
    '/// Verify that there are more than one filter available
    printlog " Verify that there are more than one filter available"
    If DateiTyp.GetItemCount < 1 then
        warnlog "There are no filters available"
    end if
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    OeffnenDlg.Cancel
    '/// Close 'Media Player' by 'Tools – Media Player'
    printlog " Close 'Media Player' by 'Tools – Media Player'"
    ToolsMediaPlayer
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------




