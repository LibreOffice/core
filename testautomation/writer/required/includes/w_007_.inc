'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_007_.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:41:47 $
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
'* owner : fredrik.haegg@sun.com
'*
'* short description : Testcases for the Tools-Menu
'*
'\***********************************************************************

sub w_007_

    printLog Chr(13) + "--------- Tools Menu (w_007_.inc) ----------"
    gApplication = "WRITER"

    Call tToolsSpellcheck
    Call tToolsThesaurus
    Call tToolsHyphenation
    Call tToolsWordCount
    Call tToolsAutocorrectAutoformat
    Call tToolsOutlineNumbering
    Call tToolsLineNumbering
    Call tToolsFootnotes
    Call tToolsGallery
    Call tToolsMailMergeWizard
    Call tToolsTextInTables
    Call tToolsSort
    Call tToolsCalculate
    Call tToolsUpdate
    Call tToolsOptionsTest
    Call tToolsHangulHanjaConversion
    Call tToolsChineseTranslation

    'Writer Web
    gApplication = "HTML"
    Call tHtmlDocToolsHangulHanjaConversion

    'Writer Master Document
    gApplication = "MASTERDOCUMENT"
    Call tMasterDocToolsHyphenation
    gApplication = "WRITER"

end sub

' -------------------------------------------

testcase tToolsSpellcheck
    PrintLog "- Tools / Spellcheck"
    Dim i as integer
    Dim TheFile as string
    Dim IsWriteable as boolean

    ' As long as OOo has no modules for that, disabled
    if gOOO = True then
        printlog "No spellchecking in OOo"
        goto endsub
    end if

    printlog " Open document where the formatting is set to the language we wish to test."
    TheFile = Convertpath (gTesttoolpath + "writer\optional\input\spellcheck\spellcheck_" + iSprache + ".odt")
    if hFileExists ( TheFile ) <> TRUE then
        Warnlog "Found no Document for the language you're testing in, Please inform the Test-Developer."
        goto endsub
    end if
    call hFileOpen (TheFile,false)
    Call sMakeReadOnlyDocumentEditable

    printlog " Insert some faulty text"
    select case iSprache
    case 01 : Call wTypeKeys ("This is a spellchecktestttttt<Home>")
    case 33 : Call wTypeKeys ("Je Suiss la Penneeee<Home>")
    case 34 : Call wTypeKeys ("Habla Espanollll<Home>")
    case 39 : Call wTypeKeys ("Don Cara Mio, capitoliooo?<Home>")
    case 49 : Call wTypeKeys ("Dies ist ein Rechtschreibtexxxxt<Home>")
    case 46 : Call wTypeKeys ("Det här är en rättstavningstexxxxt<Home>")
    case 07 : Printlog "- No Spellcheck in Russian!"
        goto endsub
    case 48 : Printlog "- No Spellcheck in Polish!"
        goto endsub
    case 55 : Printlog "- No Spellcheck in Brazilian!"
        goto endsub
    case 81 : Printlog "- No Spellcheck in Japanese!"
        goto endsub
    case 82 : Printlog "- No Spellcheck in Korean!"
        goto endsub
    case 86 : Printlog "- No Spellcheck in Chinese (simplified)!"
        goto endsub
    case 88 : Printlog "- No Spellcheck in Chinese (traditional)!"
        goto endsub
    case 03, 30, 35, 36, 37, 42, 47, 51, 90
        Printlog "- No Spellcheck available !"
        goto endsub
    case else
    end select

    ToolsSpellcheckWriter

    printlog " Tools / Spellcheck / Check"
    WaitSlot (500)
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Yes  ' Start from beginning ?
                if Active.exists then
                    if Active.GetRT = 304 then Active.Ok
                    Warnlog "Dialog 'Spellcheck' not up!"
                    Call hCloseDocument
                    goto endsub
                end if
            catch
                Active.Ok
                Warnlog "Dialog 'Spellcheck' not up!"
                Call hCloseDocument
                goto endsub
            endcatch
        end if
    end if
    printlog " Spellcheck dialog has to come up"
    Kontext "Spellcheck"
    DialogTest ( Spellcheck )
    printlog " Click 'Options'"
    SpellcheckOptions.Click
    Wait 500
    kontext "TabLinguistik"
    DialogTest ( TabLinguistik )
    printlog " Step through all options in list and if 'Edit' is enabled click 'Edit'"
    for i = 1 to 12
        kontext "TabLinguistik"
        Optionen.TypeKeys "<Down>"
        if OptionenBearbeiten.IsEnabled then
            OptionenBearbeiten.Click
            wait 500
            Kontext "SilbentrennungOptionen"
            DialogTest ( SilbentrennungOptionen )
            SilbentrennungOptionen.Cancel
        end if
    next i
    printlog " Click 'New' in Options dialog"
    Neu.Click
    wait 500
    Kontext "BenutzerwoerterbuchAnlegen"
    Call DialogTest ( BenutzerwoerterbuchAnlegen )
    BenutzerwoerterbuchAnlegen.Close

    printlog " Check if there s a writeable dictionary and select it"
    Kontext "TabLinguistik"
    for i = 1 to Benutzerwoerterbuch.GetItemCount
        Benutzerwoerterbuch.Select i
        if Bearbeiten.IsEnabled = true then
            IsWriteable = true
            exit for
        end if
    next i

    if IsWriteable = true then
        Bearbeiten.Click
        printlog " Click 'Edit' in 'Options dialog'"

        Kontext "BenutzerwoerterbuchBearbeiten"
        if BenutzerwoerterbuchBearbeiten.Exists(1) then
            Call DialogTest ( BenutzerwoerterbuchBearbeiten )
            BenutzerwoerterbuchBearbeiten.Close
        else
            Warnlog "Dialog 'Edit Dictionary' is not up!"
        end if
    else
        Warnlog "All dictionarys are write protected!"
    end if
    printlog " Close 'Options' dialog"
    Kontext "TabLinguistik"
    TabLinguistik.Close
    wait 500
    Kontext "Spellcheck"
    Spellcheck.Close
    printlog " Close 'Spellcheck' dialog"
    wait 500
    Call hCloseDocument
    printlog " Close active document"

endcase

' -------------------------------------------

testcase tToolsThesaurus
    PrintLog "- Tools / Thesaurus"
    Dim TheFile as string

    ' As long as OOo has no modules for that, disabled
    if gOOO = True then
        printlog "No spellchecking in OOo"
        goto endsub
    end if

    select case iSprache
    case 07 : Printlog "- No Thesaurus in Russian!"
        goto endsub
    case 48 : Printlog "- No Thesaurus in Polish!"
        goto endsub
    case 55 : Printlog "- No Thesaurus in Brazilian!"
        goto endsub
    case 81 : Printlog "- No Thesaurus in Japanese!"
        goto endsub
    case 82 : Printlog "- No Thesaurus in Korean!"
        goto endsub
    case 86 : Printlog "- No Thesaurus in Chinese (simplified)!"
        goto endsub
    case 88 : Printlog "- No Thesaurus in Chinese (traditional)!"
        goto endsub
    case 03, 30, 35, 36, 37, 42, 47, 51, 90
        Printlog "- No Thesaurus available !"
        goto endsub
    end select

    printlog " Open document where the formatting is set to the language we wish to test."
    TheFile = Convertpath (gTesttoolpath + "writer\optional\input\spellcheck\spellcheck_" + iSprache + ".odt")
    if hFileExists ( TheFile ) <> TRUE then
        Warnlog "Found no Document for the language you're testing in, Please inform the Test-Developer."
        goto endsub
    end if
    call hFileOpen (TheFile,false)
    Call sMakeReadOnlyDocumentEditable

    printlog " Insert some faulty text"
    select case iSprache
    case 01 : Call wTypeKeys ("This is a spellchecktestttttt<Home>")
    case 33 : Call wTypeKeys ("Je Suiss la Penneeee<Home>")
    case 34 : Call wTypeKeys ("Habla Espanollll<Home>")
    case 39 : Call wTypeKeys ("Don Cara Mio, capitoliooo?<Home>")
    case 49 : Call wTypeKeys ("Dies ist ein Rechtschreibtexxxxt<Home>")
    case 46 : Call wTypeKeys ("Det här är en rättstavningstexxxxt<Home>")
    case 07 : Printlog "- No Thesaurus in Russian!"
        goto endsub
    case 48 : Printlog "- No Thesaurus in Polish!"
        goto endsub
    case 55 : Printlog "- No Thesaurus in Brazilian!"
        goto endsub
    case 81 : Printlog "- No Thesaurus in Japanese!"
        goto endsub
    case 82 : Printlog "- No Thesaurus in Korean!"
        goto endsub
    case 86 : Printlog "- No Thesaurus in Chinese (simplified)!"
        goto endsub
    case 88 : Printlog "- No Thesaurus in Chinese (traditional)!"
        goto endsub
    case 03, 30, 35, 36, 37, 42, 47, 51, 90
        Printlog "- No Thesaurus available !"
        goto endsub
    case else
    end select

    ToolsLanguageThesaurus
    Kontext "Active"
    if Active.Exists then
        Active.Ok
        Warnlog "Dialog 'Thesaurus' not up!"
        Call hCloseDocument
        goto endsub
    end if

    Kontext "Thesaurus"
    DialogTest ( Thesaurus )
    printlog " In 'Thesaurus' dialog click 'Language'"
    Sprache.Click
    Kontext "SpracheAuswaehlen"
    DialogTest ( SpracheAuswaehlen )
    printlog " Close 'Language' dialog"
    SpracheAuswaehlen.Cancel
    Kontext "Thesaurus"
    Thesaurus.Cancel
    printlog " Close 'Thesaurus' dialog"
    Call hCloseDocument
    printlog " Close active document"

endcase

' -------------------------------------------

testcase tToolsHyphenation
    Dim DieDatei as string
    Dim LingFile as string

    PrintLog "- Tools / Hyphenation"
    ' As long as OOo has no modules for that, disabled
    if gOOO = True then
        printlog "No hyphenation in OOo"
        goto endsub
    end if

    if gBuild < 9239 then
        QAErrorlog "Hyphenation dialog freezes office!"
        goto endsub
    end if

    select case iSprache
    case 1,01 : LingFile = "01-44-hyph.dat"
        DieDatei = ConvertPath( gTesttoolpath + "writer\required\input\linguistik\001.odt" )
    case 7,07 :  DieDatei = ConvertPath( gTesttoolpath + "writer\required\input\linguistik\007.odt" )
    case 3,03 :  DieDatei = ConvertPath( gTesttoolpath + "writer\required\input\linguistik\003.odt" )
    case 81 : Printlog "- No Hyphenation in Japanese!"
        goto endsub
    case 82 : Printlog "- No Hyphenation in Korean!"
        goto endsub
    case 86 : Printlog "- No Hyphenation in Chinese (simplified)!"
        goto endsub
    case 88 : Printlog "- No Hyphenation in Chinese (traditional)!"
        goto endsub
    case else : DieDatei = ConvertPath( gTesttoolpath + "writer\required\input\linguistik\0" + trim$(str$(iSprache)) + ".odt" )
        LingFile = str$(iSprache) + "-hyph.dat"
    end select

    if hFileExists(DieDatei) = False then
        Warnlog "No defined Errortext for language: " + isprache + "! Exit!"
        goto endsub
    end if

    Select Case gApplication
    Case "WRITER"
        Call hFileOpen ( DieDatei )

    Case "MASTERDOCUMENT"
        if wInsertDocumentinMasterDoc(DieDatei) = False then
            Warnlog "Loading document did not work!"
            goto endsub
        else
            Call hNewDocument
            printlog " Open new document"
            EditPaste
        end if

    Case "HTML"
        printlog "- No Hyphenation in WriterWeb!"
        goto endsub
    end select

    Call sMakeReadOnlyDocumentEditable
    printlog " Insert file 'writer\required\input\linguistik\0 + [OfficeLanguage] + '.odt'"
    Sleep 1
    Call wTypeKeys ("<Mod1 Home>")

    ToolsLanguageHyphenate
    printlog " Tools / Hyphenate"
    Kontext "Active"
    try
        Active.Yes
    catch
    endcatch
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.Ok
            ' Force pagesize change to bringup Hyphenation-dialog
            FormatPageWriter
            Kontext
            active.SetPage TabSeite
            Kontext "TabSeite"
            Breite.Settext "15"
            TabSeite.Ok
            Sleep 1
            ToolsLanguageHyphenate
        end if
    end if

    Kontext "Silbentrennung"
    printlog " Close upcoming 'Hyphenation' dialog"
    if Silbentrennung.Exists then
        Call DialogTest ( Silbentrennung )
        Silbentrennung.Cancel
    else
        Warnlog "Dialog 'Hyphenation' not up!"
        Kontext "Active"
        if Active.Exists then
            try
                Active.Ok
            catch
            endcatch
        end if
    end if

    Call hCloseDocument
    printlog " Close active document"
    if gApplication = "MASTERDOCUMENT" then Call hCloseDocument
endcase

' -------------------------------------------

testcase tToolsWordCount
    Printlog "- Tools / Word Count"
    printlog " New Text document"
    Call hNewDocument
    printlog "  Tools / Word Count"
    ToolsWordCount
    printlog " If if dialog has been opened"
    Kontext "WordCount"
    if WordCount.Exists then
        Call DialogTest ( WordCount )
        printlog " Close dialog 'WordCount'"
        WordCount.Close
    else
        Warnlog "Dialog 'WordCount' has not been opened!"
    end if
    printlog " Close Text document"
    Call hCloseDocument
endcase

' -------------------------------------------

testcase tToolsAutocorrectAutoformat
    PrintLog "- Tools / Autocorrect / Autoformat"
    Dim i as integer

    Call hNewDocument
    printlog " Open new document"
    ToolsAutoCorrect
    printlog " Tools / Autocorrect/Autoformat"

    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Warnlog Active.Gettext
            Active.Ok
        end if
    end if

    printlog " Step through all tabpages"
    Kontext
    active.SetPage TabErsetzung
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Warnlog Active.Gettext
            Active.Ok
        end if
    end if
    Kontext "TabErsetzung"
    Call DialogTest ( TabErsetzung )

    Kontext
    active.SetPage TabAusnahmen
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Warnlog Active.Gettext
            Active.Ok
        end if
    end if
    Kontext "TabAusnahmen"
    Call DialogTest ( TabAusnahmen )

    Kontext
    active.SetPage TabOptionenAutokorrektur
    Kontext "TabOptionenAutokorrektur"
    Call DialogTest ( TabOptionenAutokorrektur )
    printlog " Step through all options in list"
    printlog " Tabpage Options: With entry 11 button 'Edit' should be enabled"
    Einstellungen.TypeKeys "<Down>",11
    Wait 500
    if Bearbeiten.IsEnabled then
        Bearbeiten.Click
        Kontext "Sonderzeichen"
        DialogTest (Sonderzeichen)
        Sonderzeichen.Cancel
    else
        Warnlog "- Button 'Edit' on Entry 11 disabled!"
    end if

    Kontext "TabOptionenAutokorrektur"
    printlog " Tabpage Options: With entry 17 button 'Edit' should be enabled"
    Einstellungen.TypeKeys "<Down>",6
    Wait 500
    if Bearbeiten.IsEnabled then
        Bearbeiten.Click
        Kontext "Sonderzeichen"
        DialogTest (Sonderzeichen)
        Sonderzeichen.Cancel
    else
        Warnlog "- Button 'Edit' on Entry 16 disabled!"
    end if

    Kontext "TabOptionenAutokorrektur"
    printlog " Tabpage Options: With entry 19 button 'Edit' should be enabled"
    Einstellungen.TypeKeys "<Down>",2
    Wait 500
    if Bearbeiten.IsEnabled then
        Bearbeiten.Click
        Kontext "Zusammenfassen"
        DialogTest (Zusammenfassen)
        Zusammenfassen.Cancel
        Kontext "TabOptionenAutokorrektur"
    else
        Warnlog "- Button 'Edit' on Entry 18 disabled!"
    end if

    Kontext
    active.SetPage TabTypografisch
    Kontext "TabTypografisch"
    EinfachWortAnfang.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 1 )
    Sonderzeichen.Cancel

    Kontext "TabTypografisch"
    EinfachWortEnde.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 2 )
    Sonderzeichen.Cancel

    Kontext "TabTypografisch"
    EinfachStandard.Click

    Kontext "TabTypografisch"
    DoppeltWortAnfang.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 3 )
    Sonderzeichen.Cancel

    Kontext "TabTypografisch"
    DoppeltWortEnde.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 4 )
    Sonderzeichen.Cancel

    Kontext "TabTypografisch"
    DoppeltStandard.Click

    Kontext
    Active.Setpage TabWortergaenzung
    Kontext "TabWortergaenzung"
    Call DialogTest ( TabWortergaenzung )
    printlog " Close Autocorrect dialogue"

    TabWortergaenzung.Cancel

    Call hCloseDocument
    printlog " Close active document"
endcase

' -------------------------------------------

testcase tToolsOutlineNumbering
    PrintLog "- Tools / Outline Numbering"

    Call hNewDocument
    printlog " Open new document"
    ToolsOutlineNumbering
    printlog " Tools / Outline Numbering"
    Kontext
    Active.SetPage TabKapitelnumerierung
    Kontext "TabKapitelnumerierung"
    Call DialogTest ( TabKapitelnumerierung )

    Kontext
    Active.SetPage TabPositionKapitelnumerierung
    Call DialogTest ( TabPositionKapitelnumerierung )
    printlog " Step through tabpages and close dialog"

    TabPositionKapitelnumerierung.Cancel
    Call hCloseDocument
    printlog " Close active document"

endcase

' -------------------------------------------

testcase tToolsLineNumbering
    PrintLog "- Tools / Line Numbering"

    Call hNewDocument
    printlog " Open new document"
    ToolsLineNumbering
    printlog " Tools / Line Numbering"
    Kontext "TabZeilennumerierung"
    printlog " Check checkbox 'Show numbering'"
    Einschalten.Check
    Call DialogTest ( TabZeilennumerierung )
    printlog " Close dialog"
    TabZeilennumerierung.Cancel
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tToolsFootnotes
    PrintLog "- Tools / Footnotes"

    Call hNewDocument
    printlog " Open new document"
    ToolsFootnote
    printlog " Tools / Footnotes"
    Kontext
    Active.Setpage TabFussnoten
    Kontext "TabFussnoten"
    DialogTest ( TabFussnoten )
    printlog " Step through all tabpages"
    Kontext
    Active.SetPage TabEndnoten
    Kontext "TabEndnoten"
    DialogTest ( TabEndnoten )
    printlog " Close dialog"
    TabEndnoten.Cancel
    Call hCloseDocument
    printlog " Close active document"
endcase

' -------------------------------------------

testcase tToolsGallery
    Dim i as integer
    printlog "- Tools / Gallery"
    Call hNewDocument
    printlog " Open new document"
    Kontext "Gallery"
    if Not Gallery.Exists then ToolsGallery
    printlog " Tools / Gallery"
    WaitSlot (3000)
    Kontext "Gallery"
    printlog " Gallery has to be opened"
    if Not Gallery.Exists then
        Warnlog "Gallery has not been opened! No test"
        goto endsub
    else
        printlog " Step through all themes"
        printlog "- Step through all Themes"
        For i = 1 to Gallerys.GetItemCount
            Gallerys.Select i
            Wait 500
        next i
        printlog "- New theme"
    end if

    Kontext
    ToolsGallery
    printlog " Close gallery with Tools / Gallery"
    Kontext "Gallery"
    if Gallery.Exists then Warnlog "Gallery has not been closed!"

    Call hCloseDocument
    printlog " Close active document"

endcase

' -------------------------------------------

testcase tToolsMailMergeWizard
    Dim i as integer

    PrintLog "- Tools / Mail Merge"
    if gApplication = "HTML" then goto endsub

    Call hNewDocument
    printlog " Open new document"
    ToolsMailMergeWizard
    WaitSlot (5000)
    printlog " Tools / Mail Merge Wizard"
    Kontext "MailMergeWizard"
    if Not MailMergeWizard.Exists then
        Warnlog "MailMerge Wizard is not up! Test ends here!"
        Call hCloseDocument
        goto endsub
    end if

    Call DialogTest ( MailMergeWizard )
    Kontext "TabMailMergeSourceDocument"
    Call DialogTest ( TabMailMergeSourceDocument )
    Wait 500
    printlog " Click document 'Browse' button"
    BrowseDocument.Click
    wait 500
    Kontext "OeffnenDlg"
    if Not OeffnenDlg.Exists then
        Warnlog "Filedialog not up on document 'Browse' button"
    else
        OeffnenDlg.Cancel
    end if

    Kontext "TabMailMergeSourceDocument"
    printlog " Click template 'Browse' button"
    BrowseTemplate.Click
    wait 500
    Kontext "Neu"
    Call DialogTest ( Neu )
    Neu.Cancel

    Kontext "TabMailMergeSourceDocument"
    printlog " Check 'Use the current document' radio button"
    UseTheCurrentDocument.Check

    printlog " Click 'Next' button"
    Kontext "MailMergeWizard"
    NextButton.Click
    Sleep 1
    Kontext "TabMailMergeDocumentType"
    Call DialogTest ( TabMailMergeDocumentType )
    wait 500
    printlog " Click 'Next' button"
    Kontext "MailMergeWizard"
    NextButton.Click
    Sleep 1
    Kontext "TabMailMergeAddressBlock"
    Call DialogTest ( TabMailMergeAddressBlock )
    wait 500
    printlog " Click 'Select Address List'"
    SelectAddressListButton.Click
    Wait 500
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Warnlog Active.Gettext
            Active.Ok
        end if
    end if
    Sleep 1
    Kontext "SelectAddressList"
    Call DialogTest ( SelectAddressList )
    if Not SelectAddressList.Exists then
        Warnlog "Problems selecting addresses"
        Kontext "MailMergeWizard"
        MailMergeWizard.Cancel
        Call hCloseDocument
        goto endsub
    end if

    printlog " In 'Select address list' dialog click 'Add'"
    Add.Click
    wait 500
    Kontext "OeffnenDlg"
    if Not OeffnenDlg.Exists then
        Warnlog "Filedialog not up on template 'Browse' button"
    else
        OeffnenDlg.Cancel
    end if

    Kontext "SelectAddressList"
    printlog " In 'Select address list' dialog click 'Create'"
    Create.Click
    wait 500

    Kontext "NewAddressList"
    Call DialogTest ( NewAddressList )
    printlog " In 'New address' dialog click 'Find' button"
    Find.Click
    wait 500

    Kontext "FindEntry"
    if FindEntry.Exists then
        Call DialogTest ( FindEntry )
        FindEntry.Cancel
    else
        Warnlog "Dialog 'Find entry' not up"
    end if

    printlog " In 'New address' dialog click 'Customize' button"
    Kontext "NewAddressList"
    Customize.Click
    wait 500

    Kontext "CustomizeAddressList"
    if CustomizeAddressList.Exists then
        Call DialogTest ( CustomizeAddressList )
        printlog " In 'Customize Addresss List' dialog select first entry in list"
        AddressListElements.Select 1
        printlog " Click 'add' button"
        Add.Click
        Kontext "RenameElement"
        Call DialogTest ( RenameElement )
        printlog " Close 'Add Element' dialog"
        RenameElement.Cancel
        Kontext "CustomizeAddressList"
        printlog " Click 'Rename' button"
        Rename.Click
        Kontext "RenameElement"
        Call DialogTest ( RenameElement )
        RenameElement.Cancel
        wait 500
        Kontext "CustomizeAddressList"
        CustomizeAddressList.Cancel
    else
        Warnlog "Dialog 'Customize Address List' is not up"
    end if
    printlog " Close all open dialogs"
    wait 500
    Kontext "NewAddressList"
    NewAddressList.Cancel
    wait 500
    Kontext "SelectAddressList"
    SelectAddressList.Cancel

    Kontext "MailMergeWizard"
    MailMergeWizard.Cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

' -------------------------------------------

testcase tToolsTextInTables
    PrintLog "- Tools / Text <-> Tables"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert some text and select it"
    Call wTypeKeys ("Hallo dieser Text soll in eine Tabelle")
    Call wTypeKeys ("<Shift Home>")
    ToolsTextInTable
    printlog " Tools / Text<->Table"
    Kontext "TextInTabelleUmwandeln"
    DialogTest ( TextInTabelleUmwandeln )
    printlog " Close upcoming dialog"
    TextInTabelleUmwandeln.Cancel
    printlog " Close active document"
    Call hCloseDocument
endcase

' -------------------------------------------

testcase tToolsSort
    Dim i as integer
    PrintLog "- Tools / Sort"

    Call hNewDocument
    printlog " Open new document"
    For i = 1 to 5
        Call wTypeKeys (i)
        Call wTypeKeys ("<Return>")
    next i
    Call wTypeKeys ("<Mod1 A>")
    ToolsSort
    Kontext "Sortieren"
    Call DialogTest ( Sortieren )
    Sortieren.Cancel
    printlog " Close active document"
    Call hCloseDocument
endcase

' -------------------------------------------

testcase tToolsCalculate
    PrintLog "- Tools / Calculate"

    Call hNewDocument
    printlog " Open new document"
    Call wTypeKeys( "2+4" )
    printlog " Insert text: 2+4"
    EditSelectAll
    printlog " Edit / Select All"
    ToolsCalculate
    printlog " Tools / Calculate"
    Call wTypeKeys ("<End>  Das Ergebnis ist = ")
    printlog " Paste result of calculation with Edit / Paste"
    EditPaste
    Call hCloseDocument
    printlog " Close active document"

endcase

' -------------------------------------------

testcase tToolsUpdate
    PrintLog "- Tools / Update"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb !"
        goto endsub
    end if
    printlog " Open new document"
    Call hNewDocument

    printlog "  - Update Indexes ( all, current )"
    printlog " Add a text to document"
    Call wTypeKeys ("Dieser Text ist zur Aktualisierung")
    printlog " Insert / Indexes"
    InsertIndexes
    WaitSlot (2000)
    Kontext "Active"
    if Active.Exists then
        if active.GetRT = 304 then
            Warnlog Active.Gettext
            Active.Ok
        end if
    end if
    Sleep 2
    Kontext
    Active.Setpage TabVerzeichnisseVerzeichnis
    printlog " Close upcoming Index dialog"
    Kontext "TabVerzeichnisseVerzeichnis"
    TabVerzeichnisseVerzeichnis.OK
    Sleep 1
    printlog " Tools / Update / All Indexes"
    ToolsUpdateAllIndexes
    WaitSlot (1000)
    printlog " Tools / Update / Current Index"
    ToolsUpdateCurrentIndex

    printlog "  - Update Fields"
    Call wTypeKeys ("<Up><Home><Tab>")
    printlog " Insert / Fields / Author"

    InsertFieldsAuthor
    printlog " Insert / Fields / Time"
    InsertFieldsTime
    WaitSlot (1000)
    ToolsUpdateFields
    printlog " Tools / Update / Fields"

    printlog "  - Update links"
    printlog " Insert a graphic from file"
    Call hGrafikVerknuepftEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    try
        printlog " Tools / Update / Links"
        ToolsUpdateLinks
    catch
        Warnlog "Links can't be updated!"
    endcatch

    printlog "  - Update / Reformat Pages  with  Tools / Page formatting"
    ToolsPageFormatting

    printlog "  - update all   with   Tools / Update / Update All"
    ToolsUpdateUpdateAll

    printlog " Close all active documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' -------------------------------------------

testcase tToolsHangulHanjaConversion

    WarnLog "#i96838# - tToolsHangulHanjaConversion outcommented due to unreability."
    goto endsub

    Dim bAsianLanguage as Boolean
    Dim sTestFile as String

    if gApplication = "HTML" then
        sTestFile = Convertpath (gTesttoolpath + "writer\required\input\hangul.html")
    else
        if Ucase(gApplication) = "MASTERDOCUMENT" then
            goto endsub
        else
            sTestFile = Convertpath (gTesttoolpath + "writer\required\input\hangul.sxw")
        end if
    end if

    PrintLog "- Tools / HangulHanjaConversion"

    printlog " Enable 'Asian Language support' ON"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)

    printlog " Open the test file"
    Call hFileOpen (sTestFile)
    Call sMakeReadOnlyDocumentEditable
    Call wTypeKeys("<MOD1 Home>")

    printlog " Tools / HangulHanjaConversion"
    try
        ToolsLanguageHangulHanjaConversion
    catch
        Warnlog "- Format / HangulHanjaConversion is disabled or hidden!"
        bAsianLanguage = ActiveDeactivateAsianSupport(False)
        Call hCloseDocument
        goto endsub
    endcatch

    Kontext "HangulHanjaConversion"
    if HangulHanjaConversion.Exists then
        Call DialogTest ( HangulHanjaConversion )
    else
        Warnlog "- Format / HangulHanjaConversion doesn't work. Dialog is not up"
        bAsianLanguage = ActiveDeactivateAsianSupport(False)
        Call hCloseDocument
        goto endsub
    end if

    printlog " Click button 'Options'"
    try
        Options.Click
        Kontext "HangulHanjaOptions"
        Call DialogTest ( HangulHanjaOptions )
    catch
        Warnlog "- Something is wrong in the button 'Options' !"
        Call hCloseDocument
        goto endsub
    endcatch

    NewDictionary.Click
    Kontext "HangulHanjaNewDictionary"
    Call DialogTest ( HangulHanjaNewDictionary )
    HangulHanjaNewDictionary.Close

    Kontext "HangulHanjaOptions"
    HangulHanjaOptions.Cancel

    Kontext "HangulHanjaConversion"
    printlog " Close HangulHanja dialog"
    HangulHanjaConversion.Close

    printlog " Set the 'Asian Language support' to default"
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

    printlog " Close all active documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' -------------------------------------------

testcase tToolsChineseTranslation

    Dim bAsianLanguage as Boolean

    PrintLog "- Tools / Language / Chinese Translation"

    printlog " Open a new document"
    Call hNewDocument

    printlog " Enable 'Asian Language support' ON"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)

    printlog " Tools / Language / Chinese Translation"
    try
        ToolsLanguageChineseTranslation
    catch
        Warnlog "- Tools/language / Chinese Translation is disabled or hidden!"
        Call hCloseDocument
        goto endsub
    endcatch

    Kontext "ChineseTranslation"
    if ChineseTranslation.Exists then
        Call DialogTest ( ChineseTranslation )
    else
        Warnlog "#i41364#Tools-> Language -> Chinese Translation does not work!"
        Call hCloseDocument
        goto endsub
    end if

    printlog " Press 'Edit Terms' button , a new dialog should appear"
    try
        EditTerms.Click
    catch
        Warnlog "- Something is wrong in the button 'Edit Terms' !"
        Kontext "ChineseTranslation"
        ChineseTranslation.Cancel
        Call hCloseDocument
        goto endsub
    endcatch

    Kontext "ChineseDictionary"
    if ChineseDictionary.Exists then
        Call DialogTest ( ChineseDictionary )
    else
        Warnlog "Button 'Edit Terms' does not bring up the dialog!"
        Kontext "ChineseTranslation"
        ChineseTranslation.Cancel
        Call hCloseDocument
        goto endsub
    end if
    ChineseDictionary.Cancel

    Kontext "ChineseTranslation"
    ChineseTranslation.Cancel

    printlog " Set the 'Asian Language support' to default"
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

' -------------------------------------------

testcase tHtmlDocToolsHangulHanjaConversion
    WarnLog "#96838# - tToolsHangulHanjaConversion outcommented due to unreability."
    goto endsub

    Dim bAsianLanguage as Boolean
    Dim sTestFile as String

    gApplication = "HTML"
    PrintLog "- Tools / HangulHanjaConversion"
    sTestFile = Convertpath (gTesttoolpath + "writer\required\input\hangul.html")

    printlog " Enable 'Asian Language support' ON"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)

    printlog " Open the test file"
    Call hFileOpen (sTestFile)
    Call sMakeReadOnlyDocumentEditable
    Call wTypeKeys("<MOD1 Home>")

    printlog " Tools / HangulHanjaConversion"
    try
        ToolsLanguageHangulHanjaConversion
    catch
        Warnlog "- Format / HangulHanjaConversion is disabled or hidden!"
        bAsianLanguage = ActiveDeactivateAsianSupport(False)
        Call hCloseDocument
        goto endsub
    endcatch

    Kontext "HangulHanjaConversion"
    if HangulHanjaConversion.Exists then
        Call DialogTest ( HangulHanjaConversion )
    else
        Warnlog "- Format / HangulHanjaConversion doesn't work. Dialog is not up"
        bAsianLanguage = ActiveDeactivateAsianSupport(False)
        Call hCloseDocument
        goto endsub
    end if

    printlog " Click button 'Options'"
    try
        Options.Click
        Kontext "HangulHanjaOptions"
        Call DialogTest ( HangulHanjaOptions )
    catch
        Warnlog "- Something is wrong in the button 'Options' !"
        Call hCloseDocument
        goto endsub
    endcatch

    NewDictionary.Click
    Kontext "HangulHanjaNewDictionary"
    Call DialogTest ( HangulHanjaNewDictionary )
    HangulHanjaNewDictionary.Close

    Kontext "HangulHanjaOptions"
    HangulHanjaOptions.Cancel

    Kontext "HangulHanjaConversion"
    printlog " Close HangulHanja dialog"
    HangulHanjaConversion.Close

    printlog " Set the 'Asian Language support' to default"
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

' -------------------------------------------

testcase tMasterDocToolsHyphenation
    Dim DieDatei as string
    Dim LingFile as string

    gApplication = "MASTERDOCUMENT"
    PrintLog "- Tools / Hyphenation"
    ' As long as OOo has no modules for that, disabled
    if gOOO = True then
        printlog "No hyphenation in OOo"
        goto endsub
    end if

    if gBuild < 9239 then
        QAErrorlog "Hyphenation dialog freezes office!"
        goto endsub
    end if

    select case iSprache
    case 1,01 : LingFile = "01-44-hyph.dat"
        DieDatei = ConvertPath( gTesttoolpath + "writer\required\input\linguistik\001.odt" )
    case 7,07 :  DieDatei = ConvertPath( gTesttoolpath + "writer\required\input\linguistik\007.odt" )
    case 3,03 :  DieDatei = ConvertPath( gTesttoolpath + "writer\required\input\linguistik\003.odt" )
    case 81 : Printlog "- No Hyphenation in Japanese!"
        goto endsub
    case 82 : Printlog "- No Hyphenation in Korean!"
        goto endsub
    case 86 : Printlog "- No Hyphenation in Chinese (simplified)!"
        goto endsub
    case 88 : Printlog "- No Hyphenation in Chinese (traditional)!"
        goto endsub
    case else : DieDatei = ConvertPath( gTesttoolpath + "writer\required\input\linguistik\0" + trim$(str$(iSprache)) + ".odt" )
        LingFile = str$(iSprache) + "-hyph.dat"
    end select

    if hFileExists(DieDatei) = False then
        Warnlog "No defined Errortext for language: " + isprache + "! Exit!"
        goto endsub
    end if

    if wInsertDocumentinMasterDoc(DieDatei) = False then
        Warnlog "Loading document did not work!"
        goto endsub
    else
        Call hNewDocument
        printlog " Open new document"
        EditPaste
    end if

    Call sMakeReadOnlyDocumentEditable
    printlog " Insert file 'writer\required\input\linguistik\0 + [OfficeLanguage] + '.odt'"
    Sleep 1
    Call wTypeKeys ("<Mod1 Home>")

    ToolsLanguageHyphenate
    printlog " Tools / Hyphenate"
    Kontext "Active"
    try
        Active.Yes
    catch
    endcatch
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.Ok
            ' Force pagesize change to bringup Hyphenation-dialog
            FormatPageWriter
            Kontext
            active.SetPage TabSeite
            Kontext "TabSeite"
            Breite.Settext "15"
            TabSeite.Ok
            Sleep 1
            ToolsLanguageHyphenate
        end if
    end if

    Kontext "Silbentrennung"
    printlog " Close upcoming 'Hyphenation' dialog"
    if Silbentrennung.Exists then
        Call DialogTest ( Silbentrennung )
        Silbentrennung.Cancel
    else
        Warnlog "Dialog 'Hyphenation' not up!"
        Kontext "Active"
        if Active.Exists then
            try
                Active.Ok
            catch
            endcatch
        end if
    end if

    printlog " Close active documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
    gApplication = "WRITER"
endcase
