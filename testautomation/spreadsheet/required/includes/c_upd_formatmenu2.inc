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
'* short description : Resource Test - Format Menu Part II
'*
'\***********************************************************************

sub c_upd_formatmenu2

    Printlog Chr(13) + "--------- Format Menu Part II (c_upd_formatmenu2.inc) ---------"
    
    call tFormatPrintRanges
    call tFormatCharacter
    call tFormatParagraph
    call tFormatChangeCase
    call tFormatStyles
    call tFormatAutoFormat
    call tFormatConditionalFormatting
    call tFormatAnchor
    call tFormatAlignment
    call tFormatArrange
    call tFormatFlip
    call tFormatGroup
    call tFormatControl   'testcase still under construction

end sub

'-----------------------------------------------------------

testcase tFormatPrintRanges
'///<u><b>Format - Print Ranges</b></u>

    '/// Opening new spreadsheet document for getting defined starting environment
    printlog " Opening new spreadsheet document for getting defined starting environment"
    call hNewDocument
    '/// Define a print range by 'Format – Print Ranges – Define'
    printlog " Define a print range by 'Format – Print Ranges – Define'"
    FormatPrintRangesDefine
    '/// Add a print range by 'Format – Print Ranges – Add'
    printlog " Add a print range by 'Format – Print Ranges – Add'"
    FormatPrintRangesAdd
    '/// Delete a print range by 'Format – Print Ranges – Delete'
    printlog " Delete a print range by 'Format – Print Ranges – Delete'"
    FormatPrintRangesRemove
    '/// Open 'Edit Print Ranges' – dialog by 'Format – Print Ranges – Edit'
    printlog " Open 'Edit Print Ranges' – dialog by 'Format – Print Ranges – Edit'"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    DialogTest ( DruckbereicheBearbeiten )
    '/// Verify that there are four entries in 'Print range'
    printlog " Verify that there are four entries in 'Print range'"
    if not DruckbereichListe.GetItemCount = 4 then
        warnlog "There are " & DruckbereichListe.GetItemCount & " items instead of 4"
    end if
    '/// Press 'Shrink' -button for 'Print Range'
    printlog " Press 'Shrink' -button for 'Print Range'"
    DruckbereichButton.Click
    '/// Enlarge Rangechooser
    printlog " Enlarge Rangechooser"
    DruckbereichButton.Click
    '/// Verify that there are two entries in 'Rows to repeat'
    printlog " Verify that there are two entries in 'Rows to repeat'"
    if not WiederholungszeileListe.GetItemCount = 2 then
        warnlog "There are " & WiederholungszeileListe.GetItemCount & " items instead of 2"
    end if
    '/// Press 'Shrink' -button for 'Rows to repeat'
    printlog " Press 'Shrink' -button for 'Rows to repeat'"
    WiederholungszeileButton.Click
    '/// Enlarge Rangechooser
    printlog " Enlarge Rangechooser"
    WiederholungszeileButton.Click
    '/// Verify that there are two entries in 'Columns to repeat'
    printlog " Verify that there are two entries in 'Columns to repeat'"
    if not WiederholungsspalteListe.GetItemCount = 2 then
        warnlog "There are " & WiederholungsspalteListe.GetItemCount & " items instead of 2"
    end if
    '/// Press 'Shrink' -button for 'Columns to repeat'
    printlog " Press 'Shrink' -button for 'Columns to repeat'"
    WiederholungsspalteButton.Click
    '/// Enlarge Rangechooser
    printlog " Enlarge Rangechooser"
    WiederholungsspalteButton.Click
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    DruckbereicheBearbeiten.Cancel
    '/// Close starting document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatCharacter
'///<u><b>Format - Character</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    Kontext "DocumentCalc"
    '/// Type 'Test'
    printlog " Type 'Test'" 
    DocumentCalc.TypeKeys "Test"
    '/// Open  'Character' – dialog by 'Format – Character'
    printlog " Open  'Character' – dialog by 'Format – Character'"
    FormatCharacter
    '/// Switch to tabpage 'Font'
    printlog " Switch to tabpage 'Font'"
    Kontext
    active.SetPage TabFont
    kontext "TabFont"
    Call DialogTest ( TabFont )
    '/// Switch to tabpage 'Font Effects'
    printlog " Switch to tabpage 'Font Effects'"
    Kontext
    active.SetPage TabFontEffects
    kontext "TabFontEffects"
    Call DialogTest ( TabFontEffects )
    '/// Switch to tabpage 'Font Position'
    printlog " Switch to tabpage 'Font Position'"
    Kontext
    active.SetPage TabFontPosition
    kontext "TabFontPosition"
    Call DialogTest ( TabFontPosition )
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    TabFontPosition.Close
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatParagraph
'///<u><b>Insert - Paragraph</b></u>

    Dim bAsianLanguage as boolean
    Dim bCTLlanguage as boolean

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Enable language support for CJK and CTL (Tools-Options-Language Support)
    printlog " Enable language support for CJK and CTL (Tools-Options-Language Support)"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    bCTLLanguage = ActiveDeactivateCTLSupport(TRUE)
    '/// Insert a note via 'Insert – Note'
    printlog " Insert a note via 'Insert – Note'"
    InsertNoteCalc
    Kontext
    '/// Enter 'Test' in Note
    printlog " Enter 'Test' in Note"
    DocumentCalc.TypeKeys "Test"
    '/// Open 'Paragraph' – dialog by 'Format – Paragraph'
    printlog " Open 'Paragraph' – dialog by 'Format – Paragraph'"
    FormatParagraph
    '/// Switch to tabpage 'Indents & Spacing'
    printlog " Switch to tabpage 'Indents & Spacing'"
    Kontext
    active.SetPage TabEinzuegeUndAbstaende
    Kontext "TabEinzuegeUndAbstaende"
    Call DialogTest ( TabEinzuegeUndAbstaende )
    '/// Increase 'Before text'
    printlog " Increase 'Before text'"
    Vonlinks.More
    '/// Increase 'After text'
    printlog " Increase 'After text'"
    VonRechts.More
    '/// Increase 'First line'
    printlog " Increase 'First line'"
    ErsteZeile.More
    '/// Increase 'Above paragraph'
    printlog " Increase 'Above paragraph'"
    Oben.More
    '/// Increase 'Below paragraph'
    printlog " Increase 'Below paragraph'"
    Unten.More
    '/// Choose last (6) entry in listbox 'Line spacing'
    printlog " Choose last (6) entry in listbox 'Line spacing'"
    Zeilenabstand.Select 6
    '/// Increase 'Line spacing'
    printlog " Increase 'Line spacing'"
    If Von.IsEnabled then
        Von.More
        else qaerrorlog "Line Spacing wrong, maybe the sort of this listbox is different"
    end if
    
    '/// Switch to tabpage 'Alignment'
    printlog " Switch to tabpage 'Alignment'"
    Kontext
    active.SetPage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"
    Call DialogTest ( TabAusrichtungAbsatz )
    '/// Choose option 'Left/Top'
    printlog " Choose option 'Left/Top'"
    Links.Check
    '/// Choose option 'Right/Bottom'
    printlog " Choose option 'Right/Bottom'"
    Rechts.Check
    '/// Choose option 'Center'
    printlog " Choose option 'Center'"
    Zentriert.Check
    '/// Choose option 'Justified'
    printlog " Choose option 'Justified'"
    Blocksatz.Check
    '/// Verify that listbox 'Text direction' has entries
    printlog " Verify that listbox 'Text direction' has entries"
    if TextDirection.GetItemCount < 1 then
        warnlog "The Textdirection listbox is empty"
    end if

    '/// Switch to tabpage 'Asian Typography'
    printlog " Switch to tabpage 'Asian Typography'"
    Kontext
    active.SetPage TabAsianTypography
    Kontext "TabAsianTypography"
    Call DialogTest ( TabAsianTypography )
    '/// Check checkbox for 'Apply list...'
    printlog " Check checkbox for 'Apply list...'"
    ApplyList.Check
    '/// Check checkbox 'Allow...'
    printlog " Check checkbox 'Allow...'"
    AllowHangingPunctuation.Check
    '/// Check checkbox 'Apply spacing...'
    printlog " Check checkbox 'Apply spacing...'"
    ApplySpacingBetween.Check
    
    '/// Switch to tabpage 'Tab'
    printlog " Switch to tabpage 'Tab'"
    Kontext
    active.SetPage TabTabulatoren
    Kontext "TabTabulatoren"
    Call DialogTest ( TabTabulatoren )
    '/// Close 'Paragraph' – dialog with 'Cancel'
    TabTabulatoren.Close
    
    '/// Close Note by <ESC>
    printlog " Close Note by <ESC>"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<ESCAPE>"
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

testcase tFormatChangeCase
'///<u><b>Format - Change Case</b></u>

    Dim bAsianLanguage as boolean
    Dim bCTLlanguage as boolean

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enable language support for CJK and CTL (Tools-Options-Language Support)
    printlog " Enable language support for CJK and CTL (Tools-Options-Language Support)"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    bCTLLanguage = ActiveDeactivateCTLSupport(TRUE)
    '/// Change case by 'Format – Change Case – Uppercase'
    printlog " Change case by 'Format – Change Case – Uppercase'"
    FormatChangeCaseUpper
    '/// Change case by 'Format – Change Case – Lowercase'
    printlog " Change case by 'Format – Change Case – Lowercase'"
    FormatChangeCaseLower
    '/// Change case by 'Format – Change Case – Halfwidth'
    printlog " Change case by 'Format – Change Case – Halfwidth'"
    FormatChangeCaseHalfwidth
    '/// Change case by 'Format – Change Case – Fullwidth'
    printlog " Change case by 'Format – Change Case – Fullwidth'"
    FormatChangeCaseFullwidth
    '/// Change case by 'Format – Change Case – Hiragana'
    printlog " Change case by 'Format – Change Case – Hiragana'"
    FormatChangeCaseHiragana
    '/// Change case by 'Format – Change Case – Katagana'
    printlog " Change case by 'Format – Change Case – Katagana'"
    FormatChangeCaseKatagana
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

testcase tFormatStyles
'///<u><b>Format - Styles and Formatting</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Stylist' by 'Format – Styles and Formatting'
    printlog " Open 'Stylist' by 'Format – Styles and Formatting'"
    FormatStylist
    Kontext "Gestalter"
    if Gestalter.notexists (2) then 
        FormatStylist
        Kontext "Gestalter"
    end if
    '/// Click on button 'Cell Styles'
    printlog " Click on button 'Cell Styles'"
    Zellvorlagen.Click
    DialogTest ( Gestalter,1 )
    '/// Click on button 'Fill Format Mode' to turn mode on
    printlog " Click on button 'Fill Format Mode' to turn mode on"
    Giesskanne.Click
    '/// Click again on button 'Fill Format Mode' to turn mode off
    printlog " Click again on button 'Fill Format Mode' to turn mode off"
    Giesskanne.Click
    '/// Click on button 'NewStyle from Selection'
    printlog " Click on button 'NewStyle from Selection'"
    AusSelektion.Click
    Kontext "VorlageErzeugen"
    DialogTest ( VorlageErzeugen )
    '/// Type name 'Teststyle'
    printlog " Type name 'Teststyle'"
    Vorlagenname.SetText "Teststyle"
    '/// Close 'Create Style' – dialog with 'Cancel'
    printlog " Close 'Create Style' – dialog with 'Cancel'"
    VorlageErzeugen.Cancel
    Kontext "Gestalter"
    '/// Click on button 'Update Style'
    printlog " Click on button 'Update Style'"
    Aktualisieren.Click
    '/// Click on button 'Page Styles'
    printlog " Click on button 'Page Styles'"
    Absatzvorlagen.Click
    '/// Close 'Stylist' by 'Format – Styles and Formatting
    printlog " Close 'Stylist' by 'Format – Styles and Formatting"
    Kontext "Gestalter"
    Gestalter.Close
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatAutoFormat
'///<u><b>Format - AutoFormat/b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Select cells A1:E6
    printlog " Select cells A1:E6"
    Call fCalcSelectRange ("A1:E6")
    '/// Open 'AutoFormat' – dialog by 'Format – AutoFormat'
    printlog " Open 'AutoFormat' – dialog by 'Format – AutoFormat'"
    FormatAutoformat
    Kontext "AutoformatCalc"
    '/// Click on 'More' – button
    Zusaetze.Click
    Call DialogTest ( AutoformatCalc )
    '/// Verify that the 'Format' – listbox has items
    printlog " Verify that the 'Format' – listbox has items"
    if FormatListe.GetItemCount < 1 then
        warnlog "The formatlist is empty"
    end if
    '/// Click on 'Add' – button
    printlog " Click on 'Add' – button"
    Einfuegen.Click
    Kontext "AutoFormatHinzufuegenCalc"
    Call DialogTest ( AutoFormatHinzufuegenCalc )
    '/// Enter name 'Testformat'
    printlog " Enter name 'Testformat'"
    TabellenName.SetText "Testformat"
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    AutoFormatHinzufuegenCalc.Close
    Kontext "AutoformatCalc"
    '/// Select second entry in 'Format' – listbox
    printlog " Select second entry in 'Format' – listbox"
    FormatListe.Select 2
    '/// Click on 'Rename' – button
    printlog " Click on 'Rename' – button"
    Umbenennen.Click
    Kontext "RenameAutoFormat"
    Call DialogTest ( RenameAutoFormat )
    '/// Set format name to 'Newformat'
    printlog " Set format name to 'Newformat'"
    RenameAutoFormatName.SetText "Newformat"
    '/// Close 'Rename AutoFormat' – dialog with 'Cancel'
    printlog " Close 'Rename AutoFormat' – dialog with 'Cancel'"    
    RenameAutoFormat.Cancel
    Kontext "AutoformatCalc"
    '/// Click on 'Delete' – button
    printlog " Click on 'Delete' – button"
    Loeschen.Click
    '/// Close messagebox with 'Yes'
    Kontext
    Active.No
    Kontext "AutoformatCalc"
    '/// Check 'Number format'
    printlog " Check 'Number format'"
    Zahlenformat.Check
    '/// Check 'Borders'
    printlog " Check 'Borders'"
    Umrandung.Check
    '/// Check 'Font'
    printlog " Check 'Font'"
    Schriftart.Check
    '/// Check 'Pattern'
    printlog " Check 'Pattern'"
    Muster.Check
    '/// Check 'Alignment'
    printlog " Check 'Alignment'"
    Ausrichtung.Check
    '/// Check 'AutoFit width and height'
    printlog " Check 'AutoFit width and height'"
    BreiteHoeheAnpassen.Check
    '/// Close 'AutoFormat' – dialog with 'Cancel'
    printlog " Close 'AutoFormat' – dialog with 'Cancel'"
    AutoformatCalc.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatConditionalFormatting
'///<u><b>Format - Conditional Formatting</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Conditional Formatting' – dialog by 'Format – Conditional Formatting'
    printlog " Open 'Conditional Formatting' – dialog by 'Format – Conditional Formatting'"
    FormatConditionalFormatting
    Kontext "BedingteFormatierung"
    Call DialogTest ( BedingteFormatierung )
    '/// Check 'Condition 1'
    printlog " Check 'Condition 1'"
    Bedingung1.Check
    '/// Choose first entry (Cell value is)
    printlog " Choose first entry (Cell value is)"
    PopupList1.Select 1
    '/// Choose first criteria for comparison (equal to)
    printlog " Choose first criteria for comparison (equal to)"
    Vergleich1.Select 1
    '/// Click on 'Shrink' – button
    printlog " Click on 'Shrink' – button"
    Minimieren1.Click
    '/// Close rangechoser by 'Shrink' -button
    printlog " Close rangechoser by 'Shrink' -button"
    Minimieren1.Click
    '/// Set string '12' in editfield
    printlog " Set string '12' in editfield"
    TextField1.SetText "12"
    '/// Verify that the 'Cell style' listbox has items
    printlog " Verify that the 'Cell style' listbox has items"
    if Zellvorlage1.GetItemCount < 1 then
        warnlog "The 'Cell style' listbox is empty"
    end if
	printlog " Click on NewStyle button"
	NewStyle1.Click
	Kontext
    active.SetPage TabVerwalten
	printlog " Close Style dialog"
    Kontext "TabVerwalten"
	TabVerwalten.Close
	Kontext "BedingteFormatierung"
    '/// Check 'Condition 2'
    printlog " Check 'Condition 2'"
    Bedingung2.Check
    '/// Choose second entry 'Formula is'
    printlog " Choose second entry 'Formula is'"
    PopupList2.Select 2
    '/// Click on 'Shrink' – button
    printlog " Click on 'Shrink' – button"
    Minimieren2.Click
    '/// Close rangechoser by 'Shrink' -button
    printlog " Close rangechoser by 'Shrink' -button"
    Minimieren2.Click
    '/// Check 'Condition 3'
    printlog " Check 'Condition 3'"
    Bedingung3.Check
    '/// Choose first entry (Cell value is)
    printlog " Choose first entry (Cell value is)"
    PopupList3.Select 1
    '/// Choose last criteria for comparision (not between)
    printlog " Choose last criteria for comparision (not between)"
    Vergleich3.Select 8
    '/// Click on first 'Shrink' – button
    printlog " Click on first 'Shrink' – button"
    Minimieren3.Click
    '/// Close rangechoser by 'Shrink' -button
    printlog " Close rangechoser by 'Shrink' -button"
    Minimieren3.Click
    '/// Click on second 'Shrink' – button
    printlog " Click on second 'Shrink' – button"
    Minimieren32.Click
    '/// Close rangechoser by 'Shrink' -button
    printlog " Close rangechoser by 'Shrink' -button"
    Minimieren32.Click
    '/// Close 'Conditional Formatting' – dialog with 'Cancel'
    printlog " Close 'Conditional Formatting' – dialog with 'Cancel'"
    BedingteFormatierung.close
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatAnchor
'///<u><b>Format - Anchor</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Go to cell D4
    printlog " Go to cell D4"
    Call fCalcSelectRange ("D4")
    '/// Insert Picture by 'Insert – Picture – From File' global\input\graf_inp\borabora.jpg
    printlog " Insert Picture by 'Insert – Picture – From File' global\input\graf_inp\borabora.jpg"
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
    Dateiname.Settext convertpath(gTesttoolpath + "global\input\graf_inp\borabora.jpg")
    Oeffnen.Click
    '/// Set Anchor to page by 'Format – Anchor – To Page'
    printlog " Set Anchor to page by 'Format – Anchor – To Page'"
    FormatAnchorToPageCalc
    '/// Set Anchor to page by 'Format – Anchor – To Cell'
    printlog " Set Anchor to page by 'Format – Anchor – To Cell'"
    FormatAnchorToCell
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatAlignment
'///<u><b>Format - Alignment</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Set Alignment to default by ' Format -Alignment - Default'
    printlog " Set Alignment to default by ' Format -Alignment - Default'"
    FormatAlignmentHorizontalDefaultCalc
    '/// Set Alignment to Left by ' Format -Alignment - Left'
    printlog " Set Alignment to Left by ' Format -Alignment - Left'"
    FormatAlignmentLeftCalc
    '/// Set Alignment to Centered by ' Format -Alignment – Centered'
    printlog " Set Alignment to Centered by ' Format -Alignment – Centered'"
    FormatAlignmentHorizontalCenterCalc
    '/// Set Alignment to Right by ' Format -Alignment – Right'
    printlog " Set Alignment to Right by ' Format -Alignment – Right'"
    FormatAlignmentRightCalc
    '/// Set Alignment to Justified by ' Format -Alignment – Justified'
    printlog " Set Alignment to Justified by ' Format -Alignment – Justified'"
    FormatAlignmentJustifiedCalc
    '/// Set Alignment to Top by ' Format -Alignment – Top'
    printlog " Set Alignment to Top by ' Format -Alignment – Top'"
    FormatAlignmentTopCalc
    '/// Set Alignment to Center by ' Format -Alignment – Center'
    printlog " Set Alignment to Center by ' Format -Alignment – Center'"
    FormatAlignmentverticalCenterCalc
    '/// Set Alignment to Bottom by ' Format -Alignment – Bottom'
    printlog " Set Alignment to Bottom by ' Format -Alignment – Bottom'"
    FormatAlignmentBottomCalc
    '/// Close document
    printlog " Close document"
    Call hCloseDocument
    
    '/// Load document from 'gTesttoolPath + "spreadsheet\required\input\twoobjects.ods"'
    printlog " Load document from 'gTesttoolPath + spreadsheet\required\input\twoobjects.ods"
    hFileOpenLocally( gTesttoolPath & "spreadsheet\required\input\twoobjects.ods" )
    '/// Select first object by Navigator
    printlog " Select first object by Navigator"
    Call fNavigatorSelectObject ("drawing objects", 1)
    '/// Select additional second drawing object by keyboard Strg-A
    printlog " Select additional second drawing object by keyboard Strg-A"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Mod1 A>"
    '/// Set Alignment to Left by ' Format -Alignment - Left'
    printlog " Set Alignment to Left by ' Format -Alignment - Left'"
    FormatAlignmentObjectLeftCalc
    '/// Set Alignment to Centered by ' Format -Alignment – Centered'
    printlog " Set Alignment to Centered by ' Format -Alignment – Centered'"
    FormatAlignmentObjectCenteredCalc
    '/// Set Alignment to Right by ' Format -Alignment – Right'
    printlog " Set Alignment to Right by ' Format -Alignment – Right'"
    FormatAlignmentObjectRightCalc
    '/// Set Alignment to Top by ' Format -Alignment – Top'
    printlog " Set Alignment to Top by ' Format -Alignment – Top'"
    FormatAlignmentObjectTopCalc
    '/// Set Alignment to Center by ' Format -Alignment – Center'
    printlog " Set Alignment to Center by ' Format -Alignment – Center'"
    FormatAlignmentObjectCenterCalc
    '/// Set Alignment to Bottom by ' Format -Alignment – Bottom'
    printlog " Set Alignment to Bottom by ' Format -Alignment – Bottom'"
    FormatAlignmentObjectBottomCalc
    '/// Close document
    printlog " Close document"
    Call hCloseDocument
    hFileDelete( gLastWorkFile )

endcase

'-----------------------------------------------------------

testcase tFormatArrange
'///<u><b>Format - Arrange</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Insert Picture by 'Insert – Picture – From File' ...
    printlog " Insert Picture by 'Insert – Picture – From File' ..."
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
    Dateiname.Settext convertpath(gTesttoolpath + "global\input\graf_inp\borabora.jpg")
    Oeffnen.Click 
    '/// 'Format – Arrange – Bring to Front'
    printlog " 'Format – Arrange – Bring to Front'"
    FormatArrangeBringToFrontCalc
    '/// 'Format – Arrange – Forward One'
    printlog " 'Format – Arrange – Forward One'"
    FormatArrangeBringForwardCalc
    '/// 'Format – Arrange – Back One'
    printlog " 'Format – Arrange – Back One'"
    FormatArrangeSendBackwardCalc
    '/// 'Format – Arrange – Send to Back'
    printlog " 'Format – Arrange – Send to Back'"
    FormatArrangeSendToBackCalc
    '/// 'Format – Arrange – To Background'
    printlog " 'Format – Arrange – To Background'"
    FormatArrangeToBackgroundCalc
    '/// 'Format – Arrange – To Foreground'
    printlog " 'Format – Arrange – To Foreground'"
    'Sometimes the selection of the graphic get lost, then reselect by Navigator
    try
        FormatArrangeToForegroundCalc
    catch
        Call fNavigatorSelectObject ("graphics", 1)
        FormatArrangeToForegroundCalc
    endcatch
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatFlip
'///<u><b>Format - Flip</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Insert Picture by 'Insert – Picture – From File' ...
    printlog " Insert Picture by 'Insert – Picture – From File' ..."
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
    Dateiname.Settext convertpath(gTesttoolpath + "global\input\graf_inp\borabora.jpg")
    Oeffnen.Click
    '/// Flip picture by 'Format – Flip – Flip Vertically'
    printlog " Flip picture by 'Format – Flip – Flip Vertically'"
    FormatFlipVerticallyCalc
    '/// Flip picture by 'Format – Flip – Flip Object Horizontally'
    printlog " Flip picture by 'Format – Flip – Flip Object Horizontally'"
    FormatFlipHorizontalCalc
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatGroup
'///<u><b>Format - Group</b></u>

    '/// Load document from 'gTesttoolPath + "spreadsheet\required\input\twoobjects.ods"'
    printlog " Load document from 'gTesttoolPath + spreadsheet\required\input\twoobjects.ods"
    hFileOpenLocally( gTesttoolPath & "spreadsheet\required\input\twoobjects.ods" )
    '/// Select first object by Navigator
    printlog " Select first object by Navigator"
    Call fNavigatorSelectObject ("drawing objects", 1)
    '/// Select additional second drawing object by keyboard Strg-A
    printlog " Select additional second drawing object by keyboard Strg-A"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Mod1 A>"
    '/// Group objects by 'Format - Group - Group
    printlog " Group objects by 'Format - Group - Group"
    FormatGroupGroup
    '/// Enter Group by 'Format - Group - Enter'
    printlog " Enter Group by 'Format - Group - Enter'"
    FormatGroupEditGroupCalc
    '/// Leave Group by 'Format - Group - Exit'
    printlog " Leave Group by 'Format - Group - Exit'"
    FormatExitGroupDraw
    '/// Ungroup objects by 'Format - Group - Ungroup'
    printlog " Ungroup objects by 'Format - Group - Ungroup'"
    FormatUngroupDraw
    '/// Close document
    printlog " Close document"
    Call hCloseDocument
    hFileDelete( gLastWorkFile )

endcase

'-----------------------------------------------------------

testcase tFormatControl
'///<u><b>Format - Control</b></u>

qaerrorlog "Testcase still under construction"
goto endsub

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
'/// Insert Control Button
printlog " Insert Control Button"

'/// Open 'Properties' – dialog by 'Format – Control'
printlog " Open 'Properties' – dialog by 'Format – Control'"
'FormatControl
'/// Switch to tabpage 'General'
printlog " Switch to tabpage 'General'"

'/// Switch to tabpage 'Events'
printlog " Switch to tabpage 'Events'"

'/// Close dialog by 'Format – Controls'
printlog " Close dialog by 'Format – Controls'"

'/// Open 'Form Properties' – dialog by 'Format – Form'
printlog " Open 'Form Properties' – dialog by 'Format – Form'"
'FormatForm
'/// Switch to tabpage 'General'
printlog " Switch to tabpage 'General'"

'/// Switch to tabpage 'Data'
printlog " Switch to tabpage 'Data'"

'/// Switch to tabpage 'Events'
printlog " Switch to tabpage 'Events'"

'/// Close dialog by 'Format – Form'
printlog " Close dialog by 'Format – Form'"

    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

