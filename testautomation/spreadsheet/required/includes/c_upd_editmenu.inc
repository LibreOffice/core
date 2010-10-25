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
'* short description : Resource Test - Edit Menu
'*
'************************************************************************
'*
' #1 tEditUndoRedo     
' #1 tEditRepeat
' #1 tEditCut
' #1 tEditCopy
' #1 tEditPaste
' #1 tEditPasteSpecial
' #1 tEditSelectAll
' #1 tEditChanges
' #1 tEditCompareDocuments
' #1 tEditFindAndReplace
' #1 tEditHeadersAndFooters
'*
'\***********************************************************************

sub c_upd_editmenu1

    Printlog Chr(13) + "--------- EDIT Menu Part I (c_upd_editmenu.inc) ---------"
    
    call tEditUndoRedo
    call tEditRepeat
    call tEditCut
    call tEditCopy
    call tEditPaste
    call tEditPasteSpecial
    call tEditSelectAll
    call tEditChanges
    call tEditCompareDocuments
    call tEditFindAndReplace
    call tEditHeadersAndFooters

end sub

'-----------------------------------------------------------

testcase tEditUndoRedo
'///<u><b>Edit - UndoRedo</b></u>
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Enter some text
    printlog " Enter some text"
    kontext "DocumentCalc"
    DocumentCalc.Typekeys "Test<RETURN>"
    '/// Edit - Undo
    editUndo
'    sleep(1)
    if fCalcGetCellValue ("A1") = "" then
        printlog "  - Undo is working"
    else 
        warnlog "Undo doesn't work"
    end if
    editRedo
'    sleep(1)
    if fCalcGetCellValue ("A1") = "Test" then
        printlog "  - Redo is working"
    else 
        warnlog "Redo doesn't work"
    end if
    '/// Close document
    printlog " Close document"
    hCloseDocument
    
endcase

'-----------------------------------------------------------

testcase tEditRepeat
'///<u><b>Edit - Repeat</b></u>
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Enter some text
    printlog " Enter some text"
    kontext "DocumentCalc"
    DocumentCalc.Typekeys "Test<RETURN>"
    '/// Click on cell A2
    printlog " Click on cell A2"
    call fCalcSelectCell ("A2")
    '/// Repeat insertion via Edit - Repeat
    printlog " Repeat insertion via Edit - Repeat"
    EditRepeat
    if fCalcGetCellValue ("A2") = "Test" then
        printlog "  - Repeat is working"
    else 
        warnlog "Repeat doesn't work"
    end if
    '/// Close document
    printlog " Close document"
    hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditCut
'///<u><b>Edit - Cut</b></u>
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Enter some text
    printlog " Enter some text"
    kontext "DocumentCalc"
    DocumentCalc.Typekeys "Cutted Text<RETURN>"
    '/// Click on cell A1
    printlog " Click on cell A1"
    call fCalcSelectCell ("A1")
    '/// Make sure that the clipboard is empty
    printlog " Make sure that the clipboard is empty"
    setclipboard = ""
    '/// Cut cellcontent
    printlog " Cut cellcontent" 
    EditCut
    if GetClipboardText = "Cutted Text" AND fCalcGetCellValue ("A1") = "" then
        printlog "  - Cutting is working"
    else
        warnlog "Cutting does not work correctly"
    end if
    '/// Close document
    printlog " Close document"
    hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditCopy
'///<u><b>Edit - Copy</b></u>
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Enter some text
    printlog " Enter some text"
    kontext "DocumentCalc"
    DocumentCalc.Typekeys "Copied Text<RETURN>"
    '/// Click on cell A1
    printlog " Click on cell A1"
    call fCalcSelectCell ("A1")
    '/// Make sure that the clipboard is empty
    printlog " Make sure that the clipboard is empty"
    setclipboard = ""
    '/// Copy cellcontent
    printlog " Copy cellcontent" 
    EditCopy
    if GetClipboardText = "Copied Text" AND fCalcGetCellValue ("A1") = "Copied Text" then
        printlog "  - Copying is working"
    else
        warnlog "Copying does not work correctly"
    end if
    '/// Close document
    printlog " Close document"
    hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditPaste
'///<u><b>Edit - Paste</b></u>
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Goto cell A1
    printlog " Gotocell A1"
    call fCalcSelectCell ("A1")
    '/// Fill clipboard with text
    printlog " Fill clipboard with text"
    setclipboard ("Pasted Text")
    '/// Paste clipboard
    printlog " Paste clipboard"
    EditPaste
    Kontext "TextImport"
    if TextImport.Exists(2) then
        call DialogTest (TextImport)
        '/// Close text import dialog with OK.
        printlog " Close text import dialog with OK."
        TextImport.OK
    end if        
    '/// Make sure that the clipboard is empty
    printlog " Make sure that the clipboard is empty"
    setclipboard = ""
    if fCalcGetCellValue ("A1") = "Pasted Text" then
        printlog "  - Pasting is working"
    else
        warnlog "Pasting does not work correctly"
    end if
    '/// Check overwrite messagebox via copy cellcontent and paste into itself
    printlog " Check overwrite messagebox via copy cellcontent and paste into itself"
    EditCopy
    EditPaste
    try
       Kontext "CellWarning"
       Dialogtest (CellWarning)
       '///<li>Cancel overwrite warning</li>///
       CellWarning.No
       printlog "  - Paste overwrite MsgBox tested"
    catch
       warnlog "MsgBox for overwrite warning doesn't exist"
    endcatch
    '/// Close document
    printlog " Close document"
    hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditPasteSpecial
'///<u><b>Edit - PasteSpecial</b></u>
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Enter some text
    printlog " Enter some text"
    kontext "DocumentCalc"
    DocumentCalc.Typekeys "Pasted Text<RETURN>"
    '/// Click on cell A1
    printlog " Click on cell A1"
    call fCalcSelectCell ("A1")
    '/// Copy cellcontent
    printlog " Copy cellcontent" 
    EditCopy
    '/// Paste Special the clipboard
    printlog " Paste Special the clipboard"
    EditPasteSpecialCalc
    kontext "InhalteEinfuegenCalc"
    call DialogTest (InhalteEinfuegenCalc)
    '/// Cancel PasteSpecial-Dialog
    printlog " Cancel PasteSpecial-Dialog"
    InhalteEinfuegenCalc.Cancel
    if fCalcGetCellValue ("A1") = "Pasted Text" then
        printlog "  - PasteSpecial of spreadsheet content is working"
    else
        warnlog "PasteSpecial of spreadsheet content does not work correctly"
    end if
    '/// Insert a picture from file Insert - Graphic - From File
    printlog " Insert a picture from file Insert - Graphic - From File"
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
    '/// Choose gTesttoolpath + global\input\graf_inp\baer.tif
    printlog " Choose gTesttoolpath + global\input\graf_inp\baer.tif"
    Dateiname.Settext (convertpath( gTesttoolpath + "global\input\graf_inp\baer.tif" ))
    '/// Click OK
    printlog " Click OK"
    Oeffnen.Click
    kontext "DocumentCalc"
    '/// Cut the still selected graphic via Edit - Cut
    printlog " Cut the still selected graphic via Edit - Cut"
    EditCut
    '/// PasteSpecial the cutted graphic
    printlog " PasteSpecial the cutted graphic"
    EditPasteSpecialCalc
    kontext "InhaltEinfuegen"
    call DialogTest (InhaltEinfuegen)
    printlog "  - PasteSpecial of other content is working"
    '/// Cancel the PasteSpecial dialog
    printlog " Cancel the PasteSpecial dialog"
    InhaltEinfuegen.Cancel
    '/// Close document
    printlog " Close document"
    hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditSelectAll
'///<u><b>Edit - SelectAll</b></u>
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Select whole sheet via Edit - Select All
    printlog " Select whole sheet via Edit - Select All"
    EditSelectAll
    '/// Check in areafield if the whole sheet is selected (A1:AMJ1048576)
    printlog " Check in areafield if the whole sheet is selected (A1:AMJ1048576)"
    kontext "RechenleisteCalc"
    if Bereich.getSelText = "A1:AMJ1048576" then
        printlog "  - SelectAll is working"
    else
       warnlog "SelectAll is not working as expected. The area is " & Bereich.getSelText & " instead of A1:AMJ1048576"
    end if
    '/// Close document
    printlog " Close document"
    hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditChanges
'///<u><b>Edit - Changes</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    
    '/// Activate recording mode via Edit Changes Record
    printlog " Activate recording mode via Edit Changes Record"
    EditChangesRecordCalc
    'If changerecording is initially activated (which should not be) a messagebox appears
    kontext
    if active.exists then
        active.Yes
        warnlog "Unexpected state of the office - Redlining was initialy enabled"
        EditChangesRecordCalc
    end if
    
    '/// Enter some text
    printlog " Enter some text"
    kontext "DocumentCalc"
    DocumentCalc.Typekeys "Recorded Text1<RETURN>Recorded Text2<RETURN>"
    
    '/// Open Protect changes dialog via Edit - Changes - Protect
    printlog " Open Protect changes dialog via Edit - Changes - Protect"
    EditChangesProtectTraceCalc
    Kontext ("PasswortEingeben")
    call Dialogtest (PasswortEingeben)
    '/// Cancel Passwort dialog
    printlog " Cancel Passwort dialog"
    PasswortEingeben.Cancel
    
    '/// Open Edit - Changes - Show dialog
    printlog " Open Edit - Changes - Show dialog"
    kontext "DocumentCalc"
    EditChangesShowCalc
    kontext "ShowChanges"
    call Dialogtest (ShowChanges)
    '/// Check all checkboxes
    printlog " Check all checkboxes"
    ShowChangesCalc.Check
    ShowDate.Check
    ShowAuthor.Check
    ShowRange.Check
    ShowComment.Check
    ShowAcceptedChanges.Check
    ShowRejectedChanges.Check
    '/// Click on range chooser button
    printlog " Click on range chooser button"
    ShrinkRange.Click
    '/// Enter range A1:B3
    printlog " Enter range A1:B3"
    RangeCalc.SetText "A1:B3"
    '/// Click on enlarge button
    printlog " Click on enlarge button"
    Shrink.Click
    '/// Click on 'Clock' button
    printlog " Click on 'Clock' button"
    Now1.Click
    '/// Choose 'Between' in date listbox
    printlog " Choose 'Between' in date listbox"
    DateList.Select 5
    '/// Now the second datefields are enabled, click on second 'Clock' button
    printlog " Now the second datefields are enabled, click on second 'Clock' button"
    Now2.Click
    '/// Choose first author from list
    printlog " Choose first author from list"
    AuthorList.Select 1
    '/// Enter comment 'Test' in comment field
    printlog " Enter comment 'Test' in comment field"
    EditComment.SetText "Test"
    '/// Close Edit - Changes - Show dialog with cancel
    printlog " Close Edit - Changes - Show dialog with cancel"
    ShowChanges.Cancel
    
    '/// Open dialog Edit - Changes - Accept or Discard changes
    printlog " Open dialog Edit - Changes - Accept or Discard changes"
    EditChangesAcceptOrDiscardCalc
    Kontext "AenderungenAnzeigen"
    '/// Choose tabpage List
    printlog " Choose tabpage List"
    Kontext "TabControl"
    TabControl.setpage TabListe
    Kontext "TabListe"
    Call Dialogtest (TabListe)
    '/// Choose tabpage Filter
    printlog " Choose tabpage Filter"
    Kontext "TabControl"
    TabControl.setpage TabFilter
    Call Dialogtest (TabFilter)
    '/// Close dialog Edit - Changes - Accept or Discard changes
    printlog " Close dialog Edit - Changes - Accept or Discard changes"
    Kontext "AenderungenAnzeigen"
    AenderungenAnzeigen.close
    
    '/// Select cell A1
    printlog " Select cell A1"
    call fCalcSelectRange ("A1")
    '/// Open Comments dialog via Edit - Changes - Comments
    printlog " Open Comments dialog via Edit - Changes - Comments"
    EditChangesCommentsCalc
    Kontext "Kommentar"
    call Dialogtest( Kommentar )
    '/// Click on arrow-right-button
    printlog " Click on arrow-right-button"
    Vor.Click
    '/// Click on arrow-left-button
    printlog " Click on arrow-left-button"
    Zurueck.Click
    '/// Enter comment "Test"
    printlog " Enter comment 'Test'"
    KommentarText.SetText "Test"
    '/// Close Comments dialog with cancel
    printlog " Close Comments dialog with cancel"
    Kommentar.cancel
    
    '/// Choose Edit - Changes - Merge Documents
    printlog " Choose Edit - Changes - Merge Documents"
    EditChangesMergeDocument
    '/// Enter filename 'gtesttoolpath +spreadsheet\required\input\merge.ods'
    printlog " Enter filename 'gtesttoolpath +spreadsheet\required\input\merge.ods'"
    kontext "OeffnenDlg"
    Dateiname.settext convertpath(gtesttoolpath +"spreadsheet\required\input\merge.ods")
    '/// Click Open
    printlog " Click Open"
    Oeffnen.Click
    '/// Now the redlining dialog should appear. Close dialog with windowcloser
    printlog " Now the redlining dialog should appear. Close dialog with windowcloser"
    kontext "AenderungenAnzeigen"
    AenderungenAnzeigen.close
    
    '/// Close document
    printlog " Close document"
    hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditCompareDocuments
'///<u><b>Edit - Compare Documents</b></u>
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Enter some text
    printlog " Enter some text"
    kontext "DocumentCalc"
    DocumentCalc.Typekeys "Inserted Text<RETURN>"
    '/// Choose Edit - Compare Documents
    printlog " Choose Edit - Compare Documents"
    EditCompareDocumentCalc
    '/// Enter filename 'gtesttoolpath +spreadsheet\required\input\merge.ods'
    printlog " Enter filename 'gtesttoolpath +spreadsheet\required\input\merge.ods'"
    kontext "OeffnenDlg"
    Dateiname.settext convertpath(gtesttoolpath +"spreadsheet\required\input\merge.ods")
    '/// Click Open
    printlog " Click Open"
    Oeffnen.Click
    '/// Now the redlining dialog should appear. Close dialog with windowcloser
    printlog " Now the redlining dialog should appear. Close dialog with windowcloser"
    kontext "AenderungenAnzeigen"
    AenderungenAnzeigen.close
    '/// Close document
    printlog " Close document"
    hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditFindAndReplace
'///<u><b>Edit - Find And Replace</b></u>

    Dim bAsianLanguage as boolean
    
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Enable 'Asian Language support' ON for getting all controls on dialog
    printlog " Enable 'Asian Language support' ON for getting all controls on dialog"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    '/// Enter some text
    printlog " Enter some text"
    kontext "DocumentCalc"
    DocumentCalc.Typekeys "Inserted Text<RETURN><UP>"
    '/// Choose Edit - Find And Replace
    printlog " Choose Edit - Find And Replace"
    EditSearchAndReplace
    kontext "FindAndReplace"
    '/// Click 'More' button
    printlog " Click 'More' button"
    More.Click
    call DialogTest ( FindAndReplace )
    '/// By default all checkboxes have to be unchecked.
    printlog " Uncheck all checkboxes (unchecked is also default)"
    if not MatchCase.IsEnabled then    'SoundsLike disables MatchCase
        SoundsLike.UnCheck
    end if 
    if MatchCase.IsEnabled then
        MatchCase.UnCheck
    end if
    if WholeWordsOnly.IsEnabled then
        WholeWordsOnly.UnCheck
    end if
    if CurrentSelectionOnly.IsEnabled then
        CurrentSelectionOnly.UnCheck
    end if
    if Backwards.IsEnabled then
        Backwards.UnCheck
    end if
    if RegularExpressions.IsEnabled then
        RegularExpressions.UnCheck
    end if
    if SearchForStyles.IsEnabled then
        SearchForStyles.UnCheck
    end if
    if SearchInAllSheets.IsEnabled then
        SearchInAllSheets.UnCheck
    end if
    '/// Check SimilaritySearch
    printlog " Check SimilaritySearch"
    if SimilaritySearch.IsEnabled then
        SimilaritySearch.Check
    end if
    '/// Click ...button for similaritysearch
    printlog " Click ...button for similaritysearch"
    If SimilaritySearchFor.IsEnabled then 
        SimilaritySearchFor.Click
    else
        SimilaritySearch.Check
        SimilaritySearchFor.Click
    end if
    kontext "Aehnlichkeitssuche"
    call DialogTest ( Aehnlichkeitssuche )
    '/// Use every control on opened similaritydialog
    printlog " Use every control on opened similaritydialog"
    ZeichenTauschen.More
    ZeichenHinzufuegen.Less
    ZeichenEntfernen.More
    Kombinieren.Click
    '/// Close similaritydialog with cancel
    printlog " Close similaritydialog with cancel"
    Aehnlichkeitssuche.Cancel
    kontext "FindAndReplace"
    SimilaritySearch.UnCheck
    If MatchFullHalfWidth.Isvisible then
        MatchFullHalfWidth.Uncheck
    end if
    If SoundsLike.Isvisible then
        SoundsLike.Check
        SoundsLikeOptions.Click
        kontext "TabJapaneseFind"
        call DialogTest ( TabJapaneseFind )
        TabJapaneseFind.Cancel
        kontext "FindAndReplace"
        SoundsLike.UnCheck
    end if    
    Rows.Check
    SearchIn.Select 1
    '/// Close Dialog
    printlog " Close Dialog"
    FindAndReplace.Close
    '/// Set the 'Asian Language support' to default
    printlog " Set the 'Asian Language support' to default"
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if
    '/// Close document
    printlog " Close document"
    hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditHeadersAndFooters
'///<u><b>Edit - Headers and Footers</b></u>

    Dim bAsianLanguage as boolean
    Dim bCTLlanguage as boolean

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Enable 'Asian and CTL Language support' ON for getting all controls on dialog
    printlog " Enable 'Asian and CTL Language support' ON for getting all controls on dialog"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    bCTLLanguage = ActiveDeactivateCTLSupport(TRUE)
    '/// Disable 'Same content left/right' on Format-Page to get all resources
    printlog " Disable 'Same content left/right' on Format-Page to get all resources"
    '/// - Open Format-Page dialog
    FormatPageCalc
    '/// - Switch to tabpage Header
    Kontext
    Active.SetPage TabKopfzeile
    Kontext
    if Active.Exists(2) then
        if Active.GetRT = 304 then
            Printlog Active.GetText
            Active.Yes
            sleep(1)
        end if
    end if
    '/// - Uncheck 'Same content left/right'
    Kontext "TabKopfzeile"
    InhaltLinksRechtsGleich.uncheck
    '/// - Switch to tabpage Footer
    Kontext
    Active.SetPage TabFusszeile
    Kontext "TabFussZeile"
    '/// - Uncheck 'Same content left/right'
    InhaltLinksRechtsGleich.uncheck
    '/// - Close Dialog with OK
    TabFusszeile.Ok
    
    '/// Open Edit - Headers & Footers
    printlog " Open Edit - Headers & Footers"
    EditHeadersAndFooters
    '/// Choose tabpage Header (right)
    printlog " Choose tabpage Header (right)"
    Kontext
    Active.SetPage TabKopfzeileCalc
    Kontext "TabKopfzeileCalc"
    Call DialogTest ( TabKopfzeileCalc )
    '/// Choose in Listbox Header the first entry (none)
    printlog " Choose in Listbox Header the first entry (none)"
    PredefinedHeader.Select 1
    '/// Enter '1' in left area field
    printlog " Enter '1' in left area field"
    LinkerBereich.typeKeys ("1")
    '/// Enter '1' in middle area field
    printlog " Enter '1' in middlearea field"
    MittlererBereich.typeKeys ("1")
    '/// Enter '1' in right area field
    printlog " Enter '1' in right area field"
    RechterBereich.typeKeys ("1")
    '/// Open fontdialog by clicking on Text Attributes button
    printlog " Open fontdialog by clicking on Text Attributes button"
    Textattribute.Click
    '/// Now the Dialog for Text Attributes opened. Choose tabpage Font
    printlog " Now the Dialog for Text Attributes opened. Choose tabpage Font"
    Kontext
    Active.setpage TabFont
    Kontext "TabFont"
    Call DialogTest ( TabFont)
    '/// Count number of western fonts (amount is dependent from system but should be more than one)
    printlog " Count number of western fonts (amount is dependent from system but should be more than one)"
    if Fontwest.GetItemCount < 1 then warnlog "The Fontwest listbox is empty"
    '/// Count number of western Typeface (amount is dependent from font but should be more than one)
    printlog " Count number of western Typeface (amount is dependent from font but should be more than one)"
    if StyleWest.GetItemCount < 1 then warnlog "The Stylewest listbox is empty"
    '/// Count number of western Size (amount is dependent from font but should be more than one)
    printlog " Count number of western Size (amount is dependent from font but should be more than one)"
    if SizeWest.GetItemCount < 1 then warnlog "The Sizewest listbox is empty"
    '/// Count number of western Language (amount is dependent from system but should be more than one)
    printlog " Count number of western Language (amount is dependent from system but should be more than one)"
    if LanguageWest.GetItemCount < 1 then warnlog "The Languagewest listbox is empty"
    '/// Count number of asian fonts (amount is dependent from system but should be more than one)
    printlog " Count number of asian fonts (amount is dependent from system but should be more than one)"
    if FontEast.GetItemCount < 1 then warnlog "The Fonteast listbox is empty"
    '/// Count number of asian Typeface (amount is dependent from font but should be more than one)
    printlog " Count number of asian Typeface (amount is dependent from font but should be more than one)"
    if StyleEast.GetItemCount < 1 then warnlog "The Styleeast listbox is empty"
    '/// Count number of asian Size (amount is dependent from font but should be more than one)
    printlog " Count number of asian Size (amount is dependent from font but should be more than one)"
    if SizeEast.GetItemCount < 1 then warnlog "The SizeEast listbox is empty"
    '/// Count number of asian Language (amount is dependent from system but should be more than one)
    printlog " Count number of asian Language (amount is dependent from system but should be more than one)"
    if LanguageEast.GetItemCount < 1 then warnlog "The Languageeast listbox is empty"
    '/// Count number of CTL fonts (amount is dependent from system but should be more than one)
    printlog " Count number of CTL fonts (amount is dependent from system but should be more than one)"
    if FontCTL.GetItemCount < 1 then warnlog "The FontCTL listbox is empty"
    '/// Count number of CTL Typeface (amount is dependent from font but should be more than one)
    printlog " Count number of CTL Typeface (amount is dependent from font but should be more than one)"
    if StyleCTL.GetItemCount < 1 then warnlog "The StyleCTL listbox is empty"
    '/// Count number of CTL Size (amount is dependent from font but should be more than one)
    printlog " Count number of CTL Size (amount is dependent from font but should be more than one)"
    if SizeCTL.GetItemCount < 1 then warnlog "The SizeCTL listbox is empty"
    '/// Count number of CTL Language (amount is dependent from system but should be more than one)
    printlog " Count number of CTL Language (amount is dependent from system but should be more than one)"
    if LanguageCTL.GetItemCount < 1 then warnlog "The LanguageCTL listbox is empty"
    '/// Choose tabpage Font Effects
    printlog " Choose tabpage Font Effects"
    Kontext
    Active.setpage TabFontEffects
    Kontext "TabFontEffects"
    Call DialogTest ( TabFontEffects)
    '/// Choose second entry for Underlining
    printlog " Choose second entry for Underlining"
    Underline.Select 2
    '/// Count number of Color
    printlog " Count number of Color"
    if Color.GetItemCount < 1 then warnlog "The Color listbox is empty"
    '/// Choose second entry for Strikethrough
    printlog " Choose second entry for Strikethrough"
    StrikeThrough.Select 2
    '/// Check Individual words
    printlog " Check Individual words"
    IndividualWords.Check
    '/// Check Outline
    printlog " Check Outline"
    Outline.Check
    '/// Check Shadow
    printlog " Check Shadow"
    Shadow.Check
    '/// Choose second entry for Relief
    printlog " Choose second entry for Relief"
    Relief.Select 2
    '/// Verify that Outline and Shadow are now disabled
    printlog " Verify that Outline and Shadow are now disabled"
    if Outline.IsEnabled then warnlog "Outline is enabled but should be disabled"
    if Shadow.IsEnabled then warnlog "Shadow is enabled but should be disabled"
    '/// Choose second entry for Emphasis mark
    printlog " Choose second entry for Emphasis mark"
    Emphasis.Select 2
    '/// Verify that Position is enabled
    printlog " Verify that Position is enabled"
    if not Position.IsEnabled then warnlog "Position is disabled but should be enabled"
    '/// Count number of Font colors
    printlog " Count number of Font colors"
    if FontColor.GetItemCount < 1 then warnlog "The FontColor listbox is empty"
    '/// Choose tabpage Font position
    printlog " Choose tabpage Font position"
    Kontext
    Active.setpage TabFontPosition
    Kontext "TabFontPosition"
    Call DialogTest ( TabFontPosition)
    '/// Check Superscript
    printlog " Check Superscript"
    SuperScript.Check
    '/// Check that Relative Font Size is enabled
    printlog " Check that Relative Font Size is enabled"
    if not RelativeFontSize.IsEnabled then warnlog "The RelativeFontSize is disabled but should be enabled"
    '/// Uncheck Automatic
    printlog " Uncheck Automatic"
    Automatic.UnCheck
    '/// Raise Raise/Lower by 1
    printlog " Raise Raise/Lower by 1"
    RaiseLower.More
    '/// Check Normal
    printlog " Check Normal"
    Normal.Check
    '/// Verify that Relative font size is disabled
    printlog " Verify that Relative font size is disabled"
    if RelativeFontSize.IsEnabled then warnlog "RelativeFontSize is enabled but should be disabled"
    '/// Verify that Raise is disabled
    printlog " Verify that Raise is disabled"
    if RaiseLower.IsEnabled then warnlog "Raise is enabled but should be disabled"
    '/// Verify that Automatic is disabled
    printlog " Verify that Automatic is disabled"
    if Automatic.IsEnabled then warnlog "Automatic is enabled but should be disabled"
    '/// Check Subscript
    printlog " Check Subscript"
    Subscript.Check
    '/// Verify that Relative font size is enabled
    printlog " Verify that Relative font size is enabled"
    if not RelativeFontSize.IsEnabled then warnlog "The RelativeFontSize is disabled but should be enabled"
    '/// Check Automatic
    printlog " Check Automatic"
    Automatic.Check
    '/// Verify that Raise is disabled
    printlog " Verify that Raise is disabled"
    if RaiseLower.IsEnabled then warnlog "Raise is enabled but should be disabled"
    '/// Lower Scale width by 1
    printlog " Lower Scale width by 1"
    ScaleWith.Less
    '/// Change Spacing to second entry
    printlog " Change Spacing to second entry"
    Spacing.Select 2
    '/// Verify that SpacingBy is enabled
    printlog " Verify that SpacingBy is enabled"
    if not SpacingBy.IsEnabled then warnlog "The SpacingBy is disabled but should be enabled"
    '/// Change Spacing to first entry
    printlog " Change Spacing to first entry"
    Spacing.Select 1
    '/// Verify that SpacingBy is disabled
    printlog " Verify that SpacingBy is disabled"
    if SpacingBy.IsEnabled then warnlog "SpacingBy is enabled but should be disabled"
    '/// Check Pair kerning
    printlog " Check Pair kerning"
    PairKerning.Check
    '/// Close Text Attributes dialog with Cancel
    printlog " Close Text Attributes dialog with Cancel"
    TabFontPosition.Cancel

    Kontext "TabKopfzeileCalc"
    '/// Open menu on File button
    printlog " Open menu on File button"
'Control not usable by testtool (#i85788')
'    Datei.Open ()
    '/// Choose second entry
    printlog " Choose second entry"
'    MenuSelect 2
    '/// Close menu
    printlog " Close menu"
'    MenuSelect (0)
    '/// Click button for Sheetname
    printlog " Click button for Sheetname"
    Tabelle.Click
    '/// Click button for Page
    printlog " Click button for Page"
    Seitennummer.Click
    '/// Click button for Pages
    printlog " Click button for Pages"
    Seitenanzahl.Click
    '/// Click button for Date
    printlog " Click button for Date"
    Datum.Click
    '/// Click button for Time
    printlog " Click button for Time"
    Zeit.Click

    '/// Choose tabpage Header (left)
    printlog " Choose tabpage Header (left)"
    Kontext
    Active.SetPage TabKopfzeileCalcLinks
    Kontext "TabKopfzeileCalcLinks"
    Call DialogTest ( TabKopfzeileCalcLinks )
    '/// Choose in Listbox Header the first entry (none)
    printlog " Choose in Listbox Header the first entry (none)"
    PredefinedHeaderLeft.Select 1    
    '/// Enter 'Test' in left area field
    printlog " Enter 'Test' in left area field"
    LinkerBereich.typeKeys ("Test")
    '/// Enter 'Test' in middle area field
    printlog " Enter 'Test' in middlearea field"
    MittlererBereich.typeKeys ("Test")
    '/// Enter 'Test' in right area field
    printlog " Enter 'Test' in right area field"
    RechterBereich.typeKeys ("Test")
    '/// Open fontdialog by clicking on Text Attributes button
    printlog " Open fontdialog by clicking on Text Attributes button"
    TextAttribute.Click
    '/// Now the Dialog for Text Attributes opened. Choose tabpage Font
    printlog " Now the Dialog for Text Attributes opened. Choose tabpage Font"
    Kontext
    Active.setpage TabFont
    Kontext "TabFont"
    '/// Close fontdialog with Cancel
    printlog " Close fontdialog with Cancel"
    TabFont.Cancel
    Kontext "TabKopfzeileCalcLinks"
    '/// Open menu on File button
    printlog " Open menu on File button"
'Control not usable by testtool (#i85788')
'    Datei.Open ()
    '/// Choose second entry
    printlog " Choose second entry"
'    MenuSelect 2
    '/// Close menu
    printlog " Close menu"
'    MenuSelect (0)
    '/// Click button for Sheetname
    printlog " Click button for Sheetname"
    Tabelle.Click
    '/// Click button for Page
    printlog " Click button for Page"
    Seitennummer.Click
    '/// Click button for Pages
    printlog " Click button for Pages"
    Seitenanzahl.click
    '/// Click button for Date
    printlog " Click button for Date"
    Datum.Click
    '/// Click button for Time
    printlog " Click button for Time"
    Zeit.Click
    
    '/// Choose tabpage Footer (right)
    printlog " Choose tabpage Footer (right)"
    Kontext
    Active.SetPage TabFusszeileCalc
    Kontext "TabFusszeileCalc"
    Call DialogTest ( TabFusszeileCalc )
    '/// Choose in Listbox Footer the first entry (none)
    printlog " Choose in Listbox Footer the first entry (none)"
    PredefinedFooter.Select 1    
    '/// Enter 'Test' in left area field
    printlog " Enter 'Test' in left area field"
    LinkerBereich.typeKeys ("Test")
    '/// Enter 'Test' in middle area field
    printlog " Enter 'Test' in middlearea field"
    MittlererBereich.typeKeys ("Test")
    '/// Enter 'Test' in right area field
    printlog " Enter 'Test' in right area field"
    RechterBereich.typeKeys ("Test")
    '/// Open fontdialog by clicking on Text Attributes button
    printlog " Open fontdialog by clicking on Text Attributes button"
    TextAttribute.Click
    '/// Now the Dialog for Text Attributes opened. Choose tabpage Font
    printlog " Now the Dialog for Text Attributes opened. Choose tabpage Font"
    Kontext
    Active.setpage TabFont
    Kontext "TabFont"
    '/// Close fontdialog with Cancel
    printlog " Close fontdialog with Cancel"
    TabFont.Cancel
    Kontext "TabFusszeileCalc"
    '/// Open menu on File button
    printlog " Open menu on File button"
'Control not usable by testtool (#i85788')
'    Datei.Open ()
    '/// Choose second entry
    printlog " Choose second entry"
'    MenuSelect 2
    '/// Close menu
    printlog " Close menu"
'    MenuSelect (0)
    '/// Click button for Sheetname
    printlog " Click button for Sheetname"
    Tabelle.Click
    '/// Click button for Page
    printlog " Click button for Page"
    Seitennummer.Click
    '/// Click button for Pages
    printlog " Click button for Pages"
    Seitenanzahl.click
    '/// Click button for Date
    printlog " Click button for Date"
    Datum.Click
    '/// Click button for Time
    printlog " Click button for Time"
    Zeit.Click

    '/// Choose tabpage Footer (left)
    printlog " Choose tabpage Footer (left)"
    Kontext
    Active.SetPage TabFusszeileCalcLinks
    Kontext "TabFusszeileCalcLinks"
    Call DialogTest ( TabFusszeileCalcLinks )
    '/// Choose in Listbox Footer the first entry (none)
    printlog " Choose in Listbox Footer the first entry (none)"
    PredefinedFooterLeft.Select 1    
    '/// Enter 'Test' in left area field
    printlog " Enter 'Test' in left area field"
    LinkerBereich.typeKeys ("Test")
    '/// Enter 'Test' in middle area field
    printlog " Enter 'Test' in middlearea field"
    MittlererBereich.typeKeys ("Test")
    '/// Enter 'Test' in right area field
    printlog " Enter 'Test' in right area field"
    RechterBereich.typeKeys ("Test")
    '/// Open fontdialog by clicking on Text Attributes button
    printlog " Open fontdialog by clicking on Text Attributes button"
    TextAttribute.Click
    '/// Now the Dialog for Text Attributes opened. Choose tabpage Font
    printlog " Now the Dialog for Text Attributes opened. Choose tabpage Font"
    Kontext
    Active.setpage TabFont
    Kontext "TabFont"
    '/// Close fontdialog with Cancel
    printlog " Close fontdialog with Cancel"
    TabFont.Cancel
    Kontext "TabFusszeileCalcLinks"
    '/// Open menu on File button
    printlog " Open menu on File button"
'Control not usable by testtool (#i85788')
'    Datei.Open ()
    '/// Choose second entry
    printlog " Choose second entry"
'    MenuSelect 2
    '/// Close menu
    printlog " Close menu"
'    MenuSelect (0)
    '/// Click button for Sheetname
    printlog " Click button for Sheetname"
    Tabelle.Click
    '/// Click button for Page
    printlog " Click button for Page"
    Seitennummer.Click
    '/// Click button for Pages
    printlog " Click button for Pages"
    Seitenanzahl.click
    '/// Click button for Date
    printlog " Click button for Date"
    Datum.Click
    '/// Click button for Time
    printlog " Click button for Time"
    Zeit.Click
    
    '/// Close Headers/Footers-dialog with cancel
    printlog " Close Headers/Footers-dialog with cancel"
    TabFusszeileCalcLinks.close
    '/// Set the 'Asian and CTL Language support' to default
    printlog " Set the 'Asian and CTL Language support' to default"
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if
    if bCTLLanguage = FALSE then
        Call ActiveDeactivateCTLSupport(FALSE)
    end if
     '/// Close document
    printlog " Close document"
    hCloseDocument
    
endcase

'-----------------------------------------------------------



 
