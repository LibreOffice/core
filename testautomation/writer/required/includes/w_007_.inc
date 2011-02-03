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
'* owner : helge.delfs@oracle.com
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
    if ( gOOO ) then
        printlog "No spellchecking in OOo"
        goto endsub
    end if

    printlog " Open document where the formatting is set to the language we wish to test."
    TheFile = Convertpath (gTesttoolpath + "writer\optional\input\spellcheck\spellcheck_" + iSprache + ".odt")
    if ( not FileExists ( TheFile ) ) then
        Warnlog "Found no Document for the language you're testing in, Please inform the Test-Developer."
        goto endsub
    end if
    hFileOpenLocally( TheFile )

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

    printlog " Tools / Spellcheck / Check"
    ToolsSpellcheckWriter
    Kontext "Active"
    if ( Active.Exists( 1 ) ) then
        if Active.GetRT = 304 then
            try
                Active.Yes  ' Start from beginning ?
                if Active.exists then
                    if Active.GetRT = 304 then Active.Ok
                    Warnlog "Dialog 'Spellcheck' not up!"
                    hFileCloseAll()
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
            Kontext "HyphenationOptions"
            DialogTest ( HyphenationOptions )
            HyphenationOptions.Cancel
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
    printlog " Close 'Spellcheck' dialog"
    Spellcheck.Close
    wait 500
    PrintLog "Close active documents"
    hFileCloseAll()

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
    if ( not FileExists ( TheFile ) ) then
        Warnlog "Found no Document for the language you're testing in, Please inform the Test-Developer."
        goto endsub
    end if
    call hFileOpenLocally( TheFile )

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
    printlog " Close 'Thesaurus' dialog"
    Thesaurus.Cancel
    PrintLog "Close active documents"
    hFileCloseAll()
    
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

    if ( not FileExists( DieDatei ) ) then
        Warnlog "No defined Errortext for language: " + isprache + "! Exit!"
        goto endsub
    end if

    Select Case gApplication
    Case "WRITER"
        hFileOpenLocally( DieDatei )

    Case "MASTERDOCUMENT"
        if wInsertDocumentinMasterDoc(DieDatei) = False then
            Warnlog "Loading document did not work!"
            goto endsub
        else
            Call hNewDocument
            printlog " Open new document"
            hUseAsyncSlot( "EditPaste" )
        end if

    Case "HTML"
        printlog "- No Hyphenation in WriterWeb!"
        goto endsub
    end select

    printlog " Insert file 'writer\required\input\linguistik\0 + [OfficeLanguage] + '.odt'"
    Sleep 1
    Call wTypeKeys ("<Mod1 Home>")

    hUseAsyncSlot( "ToolsLanguageHyphenate" )
    printlog " Tools / Hyphenate"
    Kontext "Active"
    try
        Active.Yes
    catch
    endcatch
    if ( Active.Exists( 1 ) )then
        if Active.GetRT = 304 then
            Active.Ok
            ' Force pagesize change to bringup Hyphenation-dialog
            hUseAsyncSlot( "FormatPageWriter" )
            Kontext
            active.SetPage TabSeite
            Kontext "TabSeite"
            Breite.Settext "15"
            TabSeite.Ok

            hUseAsyncSlot( "ToolsLanguageHyphenate" )
        end if
    end if

    Kontext "Hyphenation"
    printlog " Close upcoming 'Hyphenation' dialog"
    if Hyphenation.Exists then
        Call DialogTest ( Hyphenation )
        Hyphenation.Cancel
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

    PrintLog "Close active documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
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
    if ( WordCount.Exists( 1 ) ) then
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
    printlog " Tabpage Options: With entry 10 button 'Edit' should be enabled"
    Einstellungen.TypeKeys "<Down>",9
    Wait 500
    if Bearbeiten.IsEnabled then
        Bearbeiten.Click
        Kontext "Sonderzeichen"
        DialogTest (Sonderzeichen)
        Sonderzeichen.Cancel
    else
        Warnlog "- Button 'Edit' on Entry 10 disabled!"
    end if

    Kontext "TabOptionenAutokorrektur"
    printlog " Tabpage Options: With entry 16 button 'Edit' should be enabled"
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
    printlog " Tabpage Options: With entry 17 button 'Edit' should be enabled"
    Einstellungen.TypeKeys "<Down>",1
    Wait 500
    if Bearbeiten.IsEnabled then
        Bearbeiten.Click
        Kontext "Zusammenfassen"
        DialogTest (Zusammenfassen)
        Zusammenfassen.Cancel
        Kontext "TabOptionenAutokorrektur"
    else
        Warnlog "- Button 'Edit' on Entry 17 disabled!"
    end if

    Kontext
    active.SetPage TabLocalizedOptions
    Kontext "TabLocalizedOptions"
    SingleQuotesStart.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 1 )
    Sonderzeichen.Cancel

    Kontext "TabLocalizedOptions"
    SingleQuotesEnd.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 2 )
    Sonderzeichen.Cancel

    Kontext "TabLocalizedOptions"
    SingleQuotesDefault.Click

    Kontext "TabLocalizedOptions"
    DoubleQuotesStart.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 3 )
    Sonderzeichen.Cancel

    Kontext "TabLocalizedOptions"
    DoubleQuotesEnd.Click
    Kontext "Sonderzeichen"
    Call DialogTest ( Sonderzeichen, 4 )
    Sonderzeichen.Cancel

    Kontext "TabLocalizedOptions"
    DoubleQuotesDefault.Click

    Kontext
    Active.Setpage TabWortergaenzung
    Kontext "TabWortergaenzung"
    Call DialogTest ( TabWortergaenzung )
    printlog " Close Autocorrect dialogue"
    TabWortergaenzung.Cancel

    printlog " Close active document"
    Call hCloseDocument
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
    printlog " Close active document"
    Call hCloseDocument
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
    printlog " Close active document"
    Call hCloseDocument
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
    printlog " Close active document"
    Call hCloseDocument
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

    printlog " Close active document"
    Call hCloseDocument
endcase

' -------------------------------------------

testcase tToolsMailMergeWizard
    Dim i as integer

    PrintLog "- Tools / Mail Merge"
    if gApplication = "HTML" then goto endsub

    Call hNewDocument
    printlog " Open new document"
    ToolsMailMergeWizard

    printlog " Tools / Mail Merge Wizard"
    Kontext "MailMergeWizard"
    if ( MailMergeWizard.Exists( 5 ) ) then
        printlog( "Dialog <MailmergeWizard> is available" )
    else
        Warnlog( "FATAL: Dialog <MailmergeWizard> not available" )
        Call hCloseDocument
        goto endsub
    end if

    Call DialogTest ( MailMergeWizard )
    Kontext "TabMailMergeSourceDocument"
    Call DialogTest ( TabMailMergeSourceDocument )
    Wait 500
    printlog " Click document 'Browse' button"
    BrowseDocument.Click

    Kontext "OeffnenDlg"
    if ( OeffnenDlg.Exists( 1 ) ) then
        OeffnenDlg.Cancel
    else
        Warnlog "Filedialog not up on document 'Browse' button"
    end if

    Kontext "TabMailMergeSourceDocument"
    printlog " Click template 'Browse' button"
    BrowseTemplate.Click
    
    Kontext "Neu"
    if ( Neu.exists( 1 ) ) then
        Call DialogTest ( Neu )
        Neu.Cancel
    else
        warnlog( "Dialog <Neu> is not available" )
    end if

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
    
    Kontext "Active"
    if ( Active.Exists( 1 ) ) then
        if Active.GetRT = 304 then
            Warnlog Active.Gettext
            Active.Ok
        end if
    end if
   
    Kontext "SelectAddressList"
    if ( SelectAddressList.exists( 2 ) ) then
        Call DialogTest ( SelectAddressList )
    else
        Warnlog "Problems selecting addresses"
        Kontext "MailMergeWizard"
        MailMergeWizard.Cancel
        Call hCloseDocument
        goto endsub
    end if

    printlog " In 'Select address list' dialog click 'Add'"
    Add.Click

    Kontext "OeffnenDlg"
    if ( OeffnenDlg.Exists( 1 ) ) then
        OeffnenDlg.Cancel
    else
        Warnlog "Filedialog not up on template 'Browse' button"
    end if

    Kontext "SelectAddressList"
    if ( SelectAddressList.exists( 1 ) ) then
        printlog " In 'Select address list' dialog click 'Create'"
        Create.Click
    else
        warnlog( "Dialog <SelectAddressList> is not available" )
    end if

    Kontext "NewAddressList"
    if ( NewAddressList.exists( 1 ) ) then
        Call DialogTest ( NewAddressList )
        printlog " In 'New address' dialog click 'Find' button"
        Find.Click
    else
        warnlog( "Dialog <NewAddressList> is not available" )
    end if

    Kontext "FindEntry"
    if ( FindEntry.Exists( 1 ) ) then
        Call DialogTest ( FindEntry )
        FindEntry.Cancel
    else
        Warnlog "Dialog 'Find entry' not up"
    end if

    printlog " In 'New address' dialog click 'Customize' button"
    Kontext "NewAddressList"
    Customize.Click

    Kontext "CustomizeAddressList"
    if ( CustomizeAddressList.Exists( 1 ) ) then
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

    Kontext "NewAddressList"
    if ( NewAddressList.exists( 1 ) ) then
        NewAddressList.Cancel
    else
        warnlog( "Dialog <NewAddressList> is not available" )
    end if

    Kontext "SelectAddressList"
    if ( SelectAddressList.exists( 1 ) ) then
        SelectAddressList.Cancel
    else
        warnlog( "Dialog <SelectAddressList> is not available" )
    end if

    Kontext "MailMergeWizard"
    if ( MailMergeWizard.exists( 1 ) ) then
        MailMergeWizard.Cancel
    else
        warnlog( "Dialog <MailMergeWizard> is not available" )
    end if

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
    sleep 1

    Kontext "Active"
    if ( Active.Exists( 2 ) ) then
        if active.GetRT = 304 then
            Warnlog Active.Gettext
            Active.Ok
        end if
    end if
    Sleep 5
    
    Kontext
    Active.Setpage TabVerzeichnisseVerzeichnis

    printlog " Close upcoming Index dialog"
    Kontext "TabVerzeichnisseVerzeichnis"
    TabVerzeichnisseVerzeichnis.OK

    printlog " Tools / Update / All Indexes"
    hUseAsyncSlot( "ToolsUpdateAllIndexes" )

    printlog " Tools / Update / Current Index"
    ToolsUpdateCurrentIndex

    printlog "  - Update Fields"
    Call wTypeKeys ("<Up><Home><Tab>")

    printlog " Insert / Fields / Author"
    InsertFieldsAuthor

    printlog " Insert / Fields / Time"
    InsertFieldsTime

    printlog " Tools / Update / Fields"
    hUseAsyncSlot( "ToolsUpdateFields" )

    printlog "  - Update links"
    printlog " Insert a graphic from file"
    Call hGrafikVerknuepftEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    if ( hUseAsyncSlot( "ToolsUpdateLinks" ) = -1 ) then
        Warnlog "Links can't be updated!"
    end if

    printlog "  - Update / Reformat Pages  with  Tools / Page formatting"
    ToolsPageFormatting

    printlog "  - update all   with   Tools / Update / Update All"
    ToolsUpdateUpdateAll

    printlog " Close all active documents"
    hFileCloseAll()
endcase

' -------------------------------------------

testcase tToolsHangulHanjaConversion
    Dim bAsianLanguage as Boolean
    Dim sTestFile as String

    if ( gApplication = "MASTERDOCUMENT" ) then
        goto endsub
    end if

    select case gApplication
    case "HTML"   : sTestFile = gTesttoolpath & "writer\required\input\hangul.html"
    case "WRITER" : sTestFile = gTesttoolpath & "writer\required\input\hangul.sxw"
    case default  : goto endsub
    end select

    PrintLog "- Tools / HangulHanjaConversion"

    printlog " Enable 'Asian Language support' ON"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)

    printlog " Open the test file"
    hFileOpenLocally(sTestFile)
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

    Dim bAsianLanguage as Boolean
    Dim sTestFile as String

    gApplication = "HTML"
    PrintLog "- Tools / HangulHanjaConversion"
    sTestFile = Convertpath (gTesttoolpath + "writer\required\input\hangul.html")

    printlog " Enable 'Asian Language support' ON"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)

    printlog " Open the test file"
    hFileOpenLocally(sTestFile)
    Call wTypeKeys("<MOD1 Home>")

    printlog " Tools / HangulHanjaConversion"
    if ( hUseAsyncSlot( "ToolsLanguageHangulHanjaConversion" ) = -1 ) then
        Warnlog "- Format / HangulHanjaConversion is disabled or hidden!"
        bAsianLanguage = ActiveDeactivateAsianSupport(False)
        Call hCloseDocument
        goto endsub
    end if

    Kontext "HangulHanjaConversion"
    if ( HangulHanjaConversion.Exists( 1 ) ) then
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

    PrintLog "Close active documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
    gApplication = "WRITER"
endcase

' -------------------------------------------

testcase tMasterDocToolsHyphenation
    Dim sTestFile as string
    Dim LingFile as string

    gApplication = "MASTERDOCUMENT"
    PrintLog "- Tools / Hyphenation"

    ' As long as OOo has no modules for that, disabled
    if ( gOOO ) then
        printlog "No hyphenation in OOo"
        goto endsub
    end if

    select case iSprache
    case 1,01
        LingFile = "01-44-hyph.dat"
        sTestFile = gTesttoolpath + "writer\required\input\linguistik\001.odt"
    case 7,07
        sTestFile = gTesttoolpath + "writer\required\input\linguistik\007.odt"
    case 3,03
        sTestFile = gTesttoolpath + "writer\required\input\linguistik\003.odt"
    case 81
        Printlog "- No Hyphenation in Japanese!"
        goto endsub
    case 82
        Printlog "- No Hyphenation in Korean!"
        goto endsub
    case 86
        Printlog "- No Hyphenation in Chinese (simplified)!"
        goto endsub
    case 88
        Printlog "- No Hyphenation in Chinese (traditional)!"
        goto endsub
    case else
        sTestFile = gTesttoolpath + "writer\required\input\linguistik\0" + trim$(str$(iSprache)) + ".odt"
        LingFile = str$(iSprache) + "-hyph.dat"
    end select

    sTestFile = convertpath( sTestFile )
    if ( not FileExists( sTestFile ) )then
        Warnlog "No defined Errortext for language: " + isprache + "! Exit!"
        goto endsub
    end if

    if ( not wInsertDocumentinMasterDoc( sTestFile ) ) then
        Warnlog "Loading document did not work!"
        goto endsub
    else
        Call hNewDocument
        printlog " Open new document"
        hUseAsyncSlot( "EditPaste" )
    end if

    
    printlog " Insert file 'writer\required\input\linguistik\0 + [OfficeLanguage] + '.odt'"
    Sleep 1
    Call wTypeKeys ("<Mod1 Home>")

    printlog " Tools / Hyphenate"
    hUseAsyncSlot( "ToolsLanguageHyphenate" )
    
    ' This is probably a workaround to make the hyphenation dialog work in any case.
    
    ' We really only know of three possible dialogs (in order of appearance):
    ' 1: Continue checking at the beginning of the document (Yes/No)
    ' 2: Hyphenation for language not installed (Ok)
    ' 3: Hyphenation completed (Ok)
    
    ' I have not changed the code, only beautified it a little but i feel the outcome of
    ' this test is somewhat random.
    Kontext "Active"
    if ( Active.exists( 1 ) ) then
    
        ' JSK: These messagboxes were originally handled silently.
        '      So i have no idea which of those is expected and which is not...
        printlog( "Messagebox: " & Active.getText() )
        try
            printlog( "Trying to close messageboy with <YES>" )
            Active.Yes
        catch
            printlog( "<YES> failed... (Whatever that means)" )
        endcatch
        
        if ( Active.Exists() ) then
            printlog( "Messagebx: " & Active.getText() )
            if ( Active.GetRT() = 304 ) then
                Active.Ok
                ' Force pagesize change to bringup Hyphenation-dialog
                hUseAsyncSlot( "FormatPageWriter" )
                
                Kontext
                active.SetPage TabSeite
                
                Kontext "TabSeite"
                Breite.Settext "15"
                TabSeite.Ok
                hUseAsyncSlot( "ToolsLanguageHyphenate" )
            end if
        end if
    end if

    Kontext "Hyphenation"
    printlog " Close upcoming 'Hyphenation' dialog"
    if ( Hyphenation.Exists( 2 ) ) then
        Call DialogTest ( Hyphenation )
        Hyphenation.Cancel
    else
        Warnlog "Dialog 'Hyphenation' not up!"
        Kontext "Active"
        if ( Active.Exists() ) then
            try
                Active.Ok
            catch
            endcatch
        end if
    end if

    printlog " Close active documents"
    hFileCloseAll()
    gApplication = "WRITER"
endcase

' -------------------------------------------
