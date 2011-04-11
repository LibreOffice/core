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
'* short description : Resource Test - Format Menu Part I
'*
'************************************************************************
'*
' #1 tFormatDefaultFormatting
' #1 tFormatCells
' #1 tFormatRow
' #1 tFormatColumn
' #1 tFormatSheet
' #1 tFormatMergeCells
' #1 tFormatPage
'*
'\***********************************************************************

sub c_upd_formatmenu

    Printlog Chr(13) + "--------- Format Menu Part I (c_upd_formatmenu.inc) ---------"
    
    call tFormatDefaultFormatting
    call tFormatCells
    call tFormatRow
    call tFormatColumn
    call tFormatSheet
    call tFormatMergeCells
    call tFormatPage

end sub

'-----------------------------------------------------------

testcase tFormatDefaultFormatting
'///<u><b>Format - Clear Direct Formatting</b></u>

    '/// Opening new spreadsheet document for getting defined starting environment
    printlog " Opening new spreadsheet document for getting defined starting environment"
    call hNewDocument
    '/// Select cell A1:D4
    printlog " Select cell A1:D4"
    call fCalcSelectRange ("A1:D4")
    '/// Use 'Format – Clear Direct Formatting
    printlog " Use 'Format – Clear Direct Formatting"
    FormatDefaultCalc
    '/// Close starting document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatCells
'///<u><b>Format - Cells</b></u>

    Dim bAsianLanguage as boolean
    Dim bCTLlanguage as boolean

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Open 'Format Cells' – dialog by 'Format – Cells'
    printlog " Open 'Format Cells' – dialog by 'Format – Cells'"
    FormatCells
    '/// Switch to tabpage 'Numbers'
    printlog " Switch to tabpage 'Numbers'"
    Kontext
    active.SetPage TabZahlen
    Kontext "TabZahlen"
    Call DialogTest ( TabZahlen )
    '/// Verify that there are 11 items in the 'Category' list
    printlog " Verify that there are 11 items in the 'Category' list"
    if Kategorie.GetItemCount <> 11 then
        warnlog "The number of categories are" & Kategorie.GetItemCount & " instead of 11"
    end if
    '/// Choose category 'Currency' (5th in list)
    printlog " Choose category 'Currency' (5th in list)"
    Kategorie.Select 5
    '/// Verify that the 'Format' now contains a language listbox
    printlog " Verify that the 'Format' now contains a language listbox"
    if FormatSprache.Exists then
        '/// Verify that  the 'Language' listbox has items
        printlog " Verify that  the 'Language' listbox has items"
        if FormatSprache.GetItemCount < 1 then
            warnlog "The language listbox is empty"
        end if
    else
        warnlog "There is no languagelistbox"
    end if
    '/// Type 'Test' in editfield for 'Format code'
    printlog " Type 'Test' in editfield for 'Format code'"
    FormatCode.SetText "Test"
    '/// Press 'Edit Comment' button
    printlog " Press 'Edit Comment' button"
    PBInfo.Click
    '/// Enter 'My Format' in the comment field
    printlog " Enter 'My Format' in the comment field"
    Kommentar.SetText "My Format"
    '/// Press 'Edit Comment' button again to finish the input
    printlog " Press 'Edit Comment' button again to finish the input"
    PBInfo.Click
    '/// Press the 'Add' button
    printlog " Press the 'Add' button"
    PBHinzufuegen
    '/// Press the 'Delete' button
    printlog " Press the 'Delete' button"
    PBEntfernen
    '/// Close 'Format Cells' – dialog with 'Cancel'
    printlog " Close 'Format Cells' – dialog with 'Cancel'"
    TabZahlen.Close

    '/// Enable language support for CJK and CTL (Tools-Options-Language Support)
    printlog " Enable language support for CJK and CTL (Tools-Options-Language Support)"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    bCTLLanguage = ActiveDeactivateCTLSupport(TRUE)
    '/// Open 'Format Cells' – dialog by 'Format – Cells'
    printlog " Open 'Format Cells' – dialog by 'Format – Cells'"
    FormatCells
    '/// Switch to tabpage 'Font'
    printlog " Switch to tabpage 'Font'"
    Kontext
    active.SetPage TabFont
    Kontext "TabFont"
    Call DialogTest ( TabFont,2 )
    '/// Verify that the fontlistbox  for 'Western text font' has items
    printlog " Verify that the fontlistbox  for 'Western text font' has items"
    if FontWest.GetItemCount < 1 then
        warnlog "The fontlistbox is empty"
    end if
    '/// Verify that the Typeface listbox  for 'Western text font' has items
    printlog " Verify that the Typeface listbox  for 'Western text font' has items"
    if StyleWest.GetItemCount < 1 then
        warnlog "The stylelistbox is empty"
    end if
    '/// Verify that the Size listbox  for 'Western text font' has items
    printlog " Verify that the Size listbox  for 'Western text font' has items"
    if SizeWest.GetItemCount < 1 then
        warnlog "The sizelistbox is empty"
    end if
    '/// Verify that the Languag  for 'Western text font'e listbox has items
    printlog " Verify that the Languag  for 'Western text font'e listbox has items"
    if LanguageWest.GetItemCount < 1 then
        warnlog "The languagelistbox is empty"
    end if
    '/// Verify that the fontlistbox  for 'Asian text font' has items
    printlog " Verify that the fontlistbox  for 'Asian text font' has items"
    if FontEast.GetItemCount < 1 then
        warnlog "The fontlistbox is empty"
    end if
    '/// Verify that the Typeface listbox  for 'Asian text font' has items
    printlog " Verify that the Typeface listbox  for 'Asian text font' has items"
    if StyleEast.GetItemCount < 1 then
        warnlog "The stylelistbox is empty"
    end if
    '/// Verify that the Size listbox  for 'Asian text font' has items
    printlog " Verify that the Size listbox  for 'Asian text font' has items"
    if SizeEast.GetItemCount < 1 then
        warnlog "The sizelistbox is empty"
    end if
    '/// Verify that the Language for 'Asian text font'e listbox has items
    printlog " Verify that the Language for 'Asian text font'e listbox has items"
    if LanguageEast.GetItemCount < 1 then
        warnlog "The languagelistbox is empty"
    end if
    '/// Verify that the fontlistbox  for 'CTL font' has items
    printlog " Verify that the fontlistbox  for 'CTL font' has items"
    if FontCTL.GetItemCount < 1 then
        warnlog "The fontlistbox is empty"
    end if
    '/// Verify that the Typeface listbox  for 'CTL font' has items
    printlog " Verify that the Typeface listbox  for 'CTL font' has items"
    if StyleCTL.GetItemCount < 1 then
        warnlog "The stylelistbox is empty"
    end if
    '/// Verify that the Size listbox  for 'CTL font' has items
    printlog " Verify that the Size listbox  for 'CTL font' has items"
    if SizeCTL.GetItemCount < 1 then
        warnlog "The sizelistbox is empty"
    end if
    '/// Verify that the Language listbox  for 'CTL font' has items
    printlog " Verify that the Language listbox  for 'CTL font' has items"
    if LanguageCTL.GetItemCount < 1 then
        warnlog "The languagelistbox is empty"
    end if

    '/// Switch to tabpage 'Font Effects'
    printlog " Switch to tabpage 'Font Effects'"
    Kontext
    active.SetPage TabFontEffects
    Kontext "TabFontEffects"
    Call DialogTest ( TabFontEffects )
    '/// Choose second entry for 'Underlining'
    printlog " Choose second entry for 'Underlining'"
    Underline.Select 2
    '/// Check that the 'Color' listbox is enabled
    printlog " Check that the 'Color' listbox is enabled"
    if not Color.IsEnabled then
        warnlog "The colorbox is not enabled"
    end if
    '/// Choose second entry for 'Strikethrough'
    printlog " Choose second entry for 'Strikethrough'"
    StrikeThrough.Select 2
    '/// Check that the 'IndividualWords' checkbox is enabled
    printlog " Check that the 'IndividualWords' checkbox is enabled"
    if not IndividualWords.IsEnabled then
        warnlog "The checkbox for IndividualWords is not enabled"
    end if
    '/// Choose first entry for 'Relief' (without)
    printlog " Choose first entry for 'Relief' (without)"
    Relief.Select 1
    '/// Check Outline
    printlog " Check Outline"
    if Outline.IsEnabled then
        Outline.Check
    end if
    '/// Check Shadow
    printlog " Check Shadow"
    if Shadow.IsEnabled then
        Shadow.Check
    end if
    '/// Choose second entry for 'Emphasis mark'
    printlog " Choose second entry for 'Emphasis mark'"
    Emphasis.Select 2
    '/// Check that the 'Position' listbox is enabled
    printlog " Check that the 'Position' listbox is enabled"
    if not Position.IsEnabled then
        warnlog "The Position listbox is disabled"
    end if
    '/// Check that the 'Font Color' listbox has items
    printlog " Check that the 'Font Color' listbox has items"
    if FontColor.GetItemCount < 1 then
        warnlog "The fontcolorbox is empty"
    end if

    '/// Switch to tabpage 'Alignment'
    printlog " Switch to tabpage 'Alignment'"
    Kontext
    active.SetPage TabAusrichtung
    Kontext "TabAusrichtung"
    Call DialogTest ( TabAusrichtung )
    '/// Choose second entry for 'Horizontal'
    printlog " Choose second entry for 'Horizontal'"
    ListeHAusrichtung.Select 2
    '/// Increase the 'Indent'
    printlog " Increase the 'Indent'"
    if Einzug.IsEnabled then
        Einzug.More
    end if
    '/// Check that the 'Vertical' listbox has items
    printlog " Check that the 'Vertical' listbox has items"
    if ListeVAusrichtung.GetItemCount < 1 then
        warnlog "The 'Vertical' listbox is empty"
    end if
    '/// Check 'Vertically stacked' checkbox
    printlog " Check 'Vertically stacked' checkbox"
    Gestapelt.Check
    if not VertikaleAusrichtung.IsEnabled then
        warnlog "The 'Asian layout mode is not enabled"
    end if
    '/// Uncheck 'Vertically stacked' checkbox
    printlog " Uncheck 'Vertically stacked' checkbox"
    Gestapelt.Uncheck
    '/// Increse the 'Degrees'
    printlog " Increse the 'Degrees'"
    Grad.More
    '/// Check 'Wrap text automatically' checkbox
    printlog " Check 'Wrap text automatically' checkbox"
    Zeilenumbruch.Check
    '/// Verify that the 'Hyphenation active' checkbox is enabled
    printlog " Verify that the 'Hyphenation active' checkbox is enabled"
    if not Silbentrennung.IsEnabled then
        warnlog "The hypenation checkbox is not enabled"
    end if
    '/// Uncheck 'Wrap text automatically' checkbox
    printlog " Uncheck 'Wrap text automatically' checkbox"
    Zeilenumbruch.UnCheck
    '/// Verify that the 'Shrink to fit cell size' checkbox is enabled
    printlog " Verify that the 'Shrink to fit cell size' checkbox is enabled"
    if not ShrinkToFit.IsEnabled then
        warnlog "The 'Shrink to fit cell size' checkbox is disabled"
    end if
    '/// Check that the 'Text direction' listbox has items
    printlog " Check that the 'Text direction' listbox has items"
    if SetAlignmentOptions.GetItemCount < 1 then
        warnlog "The 'Text direction' listbox is empty"
    end if

    '/// Switch to tabpage 'Asian Typography'
    printlog " Switch to tabpage 'Asian Typography'"
    Kontext
    active.SetPage TabAsianTypography
    Kontext "TabAsianTypography"
    Call DialogTest ( TabAsianTypography )
    '/// Verify that the checkbox for 'Apply list...' exists
    printlog " Verify that the checkbox for 'Apply list...' exists"
    if not ApplyList.Exists then
        warnlog "The checkbox does not exists"
    end if
    '/// Verify that the checkbox for 'Allow hanging punctation' exists
    printlog " Verify that the checkbox for 'Allow hanging punctation' exists"
    if not AllowHangingPunctuation.Exists then
        warnlog "The checkbox does not exists"
    end if
    '/// Verify that the checkbox for 'Apply spacing...' exists
    printlog " Verify that the checkbox for 'Apply spacing...' exists"
    if not ApplySpacingBetween.Exists then
        warnlog "The checkbox does not exists"
    end if

    '/// Switch to tabpage 'Borders'
    printlog " Switch to tabpage 'Borders'"
    Kontext
    active.SetPage TabUmrandung
    Kontext "TabUmrandung"
    Call DialogTest ( TabUmrandung )
    '/// Verify that the 'Line Style' listbox has items
    printlog " Verify that the 'Line Style' listbox has items"
    if Stil.GetItemCount < 1 then
        warnlog "The 'Line Style' listbox is empty"
    end if
    '/// Verify that the 'Color' listbox has items
    printlog " Verify that the 'Color' listbox has items"
    if StilFarbe.GetItemCount < 1 then
        warnlog "The 'Line Color' listbox is empty"
    end if
    '/// Increase 'left'
    printlog " Increase 'left'"
    Links.More
    '/// Decrease 'Right'
    printlog " Decrease 'Right'"
    Rechts.Less
    '/// Increase 'Top'
    printlog " Increase 'Top'"
    Oben.More
    '/// Decrease 'Bottom'
    printlog " Decrease 'Bottom'"
    Unten.Less
    '/// Check checkbox 'Synchronize'
    printlog " Check checkbox 'Synchronize'"
    Synchronisieren.Check
    '/// Choose second position for 'Shadow Style'
    printlog " Choose second position for 'Shadow Style'"
    Position.TypeKeys "<Right>"
    '/// Increase 'Distance'
    printlog " Increase 'Distance'"
    Groesse.More
    '/// Verify that the 'Color' listbox has items
    printlog " Verify that the 'Color' listbox has items"
    if SchattenFarbe.GetItemCount < 1 then 
       warnlog "The listbox for color is empty"
    end if
    
    '/// Switch to tabpage 'Background'
    printlog " Switch to tabpage 'Background'"
    Kontext
    active.SetPage TabHintergrund
    Kontext "TabHintergrund"
    Call DialogTest ( TabHintergrund )

    '/// Switch to tabpage 'Cell Protection'
    printlog " Switch to tabpage 'Cell Protection'"
    Kontext
    active.SetPage TabZellschutz
    Kontext "TabZellschutz"
    Call DialogTest ( TabZellschutz )
    '/// Use every checkbox
    printlog " Use every checkbox"
    AllesAusblenden.Uncheck
    Gesperrt.Check
    FormelAusblenden.Uncheck
    FuerAusdruckAusblenden.Uncheck
    '/// Close 'Format Cells' – dialog with 'Cancel'
    printlog " Close 'Format Cells' – dialog with 'Cancel'"
    TabZellschutz.Close
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

testcase tFormatRow
'///<u><b>Format - Row</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Open 'Row Height' – dialog by 'Format – Row – Height'
    printlog " Open 'Row Height' – dialog by 'Format – Row – Height'"
    FormatRowHeightCalc
    Kontext "ZeilenHoehe"
    DialogTest ( ZeilenHoehe 
    '/// Increase 'Height'
    printlog " Increase 'Height'"
    Hoehe.More
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    ZeilenHoehe.Close
    '/// Open 'Optimal Row Height' – dialog by 'Format – Row – Optimal Height'
    printlog " Open 'Optimal Row Height' – dialog by 'Format – Row – Optimal Height'"
    FormatRowOptimalHeightCalc
    Kontext "ZeileOptimaleHoehe"
    DialogTest ( ZeileOptimaleHoehe )
    '/// Increase 'Height'
    printlog " Increase 'Height'"
    Extra.More
    '/// Check 'Default Value'
    printlog " Check 'Default Value'"
    StandardWert.Check
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    ZeileOptimaleHoehe.Cancel
    '/// Hide row with 'Format – Row – Hide'
    printlog " Hide row with 'Format – Row – Hide'"
    FormatRowHide
    '/// Show row with 'Format – Row – Show'
    printlog " Show row with 'Format – Row – Show'"
    FormatRowShow
    '/// Close document
    printlog " Close document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatColumn
'///<u><b>Format - Column</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Open 'Column Width' – dialog by 'Format – Column – Width'
    printlog " Open 'Column Width' – dialog by 'Format – Column – Width'"
    FormatColumnWidthCalc
    Kontext "SpaltenBreiteCalc"
    DialogTest ( SpaltenBreiteCalc )
    '/// Increase 'Width'
    printlog " Increase 'Width'"
    Breite.More
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    SpaltenBreiteCalc.Cancel
    '/// Open 'Optimal Column Width' – dialog by 'Format – Column – Optimal Width'
    printlog " Open 'Optimal Column Width' – dialog by 'Format – Column – Optimal Width'"
    FormatColumnOptimalWidthCalc
    Kontext "SpalteOptimaleBreite"
    DialogTest ( SpalteOptimaleBreite )
    '/// Increase 'Width'
    printlog " Increase 'Width'"
    Extra.More
    '/// Check 'Default Value'
    printlog " Check 'Default Value'"
    StandardWert.Check
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    SpalteOptimaleBreite.Cancel
    '/// Hide Column with 'Format – Column – Hide'
    printlog " Hide Column with 'Format – Column – Hide'"
    FormatColumnHide
    '/// Show Column with 'Format – Column – Show'
    printlog " Show Column with 'Format – Column – Show'"
    FormatColumnShow
    '/// Close document
    printlog " Close document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatSheet
'///<u><b>Format - Sheet</b></u>

    Dim bAsianLanguage as boolean
    Dim bCTLlanguage as boolean    

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call  hNewDocument
    '/// Open 'Rename Sheet' – dialog by 'Format – Sheet'
    printlog " Open 'Rename Sheet' – dialog by 'Format – Sheet'"
    FormatSheetRename
    Kontext "TabelleUmbenennen"
    DialogTest ( TabelleUmbenennen )
    '/// Close 'Rename Sheet' – dialog with 'Cancel'
    printlog " Close 'Rename Sheet' – dialog with 'Cancel'"
    TabelleUmbenennen.Cancel
    '/// 'Hide' sheet by 'Format – Sheet – Hide'
    printlog " 'Hide' sheet by 'Format – Sheet – Hide'"
    FormatSheetHide
    '/// Open 'Show Sheet' – dialog by 'Format – Sheet – Show'
    printlog " Open 'Show Sheet' – dialog by 'Format – Sheet – Show'"
    FormatSheetShow
    Kontext "TabelleEinblenden"
    DialogTest ( TabelleEinblenden )
    '/// Close 'Show Sheet' – dialog with 'Cancel'
    printlog " Close 'Show Sheet' – dialog with 'Cancel'"
    TabelleEinblenden.Cancel
    '/// Enable language support for CJK and CTL (Tools-Options-Language Support)
    printlog " Enable language support for CJK and CTL (Tools-Options-Language Support)"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    bCTLLanguage = ActiveDeactivateCTLSupport(TRUE)
    '/// Switch sheet layout to 'Right-To-Left' by 'Format – Sheet – Right-To-Left'
    printlog " Switch sheet layout to 'Right-To-Left' by 'Format – Sheet – Right-To-Left'"
    FormatSheetRightToLeft
    '/// Switch sheet layout back
    printlog " Switch sheet layout back"
    FormatSheetRightToLeft
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

testcase tFormatMergeCells
'///<u><b>Format - Merge Cells</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Select cells A1:D4
    printlog " Select cells A1:D4"
    call fCalcSelectRange ("A1:D4")
    '/// 'Merge Cells' by 'Format – Merge Cells'
    printlog " 'Merge Cells' by 'Format – Merge Cells'"
    FormatMergeCells
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatPage
'///<u><b>Format - Page</b></u>

    Dim bAsianLanguage as boolean
    Dim bCTLlanguage as boolean

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call  hNewDocument
    '/// Enable language support for CJK and CTL (Tools-Options-Language Support)
    printlog " Enable language support for CJK and CTL (Tools-Options-Language Support)"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    bCTLLanguage = ActiveDeactivateCTLSupport(TRUE)
    '/// Open 'Page Style' – dialog with 'Format – Page'
    printlog " Open 'Page Style' – dialog with 'Format – Page'"
    FormatPageCalc
    
    '/// Switch to tabpage 'Organizer'
    printlog " Switch to tabpage 'Organizer'"
    Kontext
    active.SetPage TabVerwalten
    Kontext "TabVerwalten"
    Call DialogTest ( TabVerwalten )
    
    '/// Switch to tabpage 'Page'
    printlog " Switch to tabpage 'Page'"
    Kontext
    active.SetPage TabSeite
    Kontext "TabSeite"
    Call DialogTest ( TabSeite )
    '/// Check that the 'Format' listbox has items
    printlog " Check that the 'Format' listbox has items"
    if Papierformat.GetItemCount < 1 then 
        warnlog "No paperformats available"
    end if
    '/// Increase 'Width'
    printlog " Increase 'Width'"
    Breite.More
    '/// Decrease 'Height'
    printlog " Decrease 'Height'"
    Hoehe.Less
    '/// Check option 'Landscape'
    printlog " Check option 'Landscape'"
    Quer.Check
    '/// Check that listbox 'Text direction' has items
    printlog " Check that listbox 'Text direction' has 2 items"
    if Textfluss.GetItemCount <> 2 then
        warnlog "The text direction listbox is empty"
    end if
    '/// Check that listbox 'Paper tray' has items
    printlog " Check that listbox 'Paper tray' has items"
    if Schacht.GetItemCount < 1 then
    warnlog "The paper tray listbox is empty"
    end if
    '/// Increase 'Margins Left'
    printlog " Increase 'Margins Left'"
    Links.More
    '/// Decrease 'Margins Right'
    printlog " Decrease 'Margins Right'"
    Rechts.Less
    '/// Increase 'Margins Top'
    printlog " Increase 'Margins Top'"
    Oben.More
    '/// Decrease 'Margins Bottom'
    printlog " Decrease 'Margins Bottom'"
    Unten.Less
    '/// Check that listbox 'Page layout' has items
    printlog " Check that listbox 'Page layout' has items"
    if Seitenlayout.GetItemCount < 1 then
        warnlog "No pagelayouts available"
    end if
    '/// Check that listbox 'Format' has items
    printlog " Check that listbox 'Format' has items"
    if Numerierung.GetItemCount < 1 then
        warnlog "No formats available"
    end if
    '/// Check 'Table alignment Horizontal'
    printlog " Check 'Table alignment Horizontal'"
    Horizontal.Check
    '/// Check 'Table alignment Vertical'
    printlog " Check 'Table alignment Vertical'"
    Vertikal.Check
    
    '/// Switch to tabpage 'Borders'
    printlog " Switch to tabpage 'Borders'"
    Kontext
    active.SetPage TabUmrandung
    Kontext "TabUmrandung"
    Call DialogTest ( TabUmrandung )
    '/// Verify that listbox 'Style' has items
    printlog " Verify that listbox 'Style' has items"
    if Stil.GetItemCount < 1 then
        warnlog "No styles available"
    end if
    '/// Verify that the listbox 'Color' has items
    printlog " Verify that the listbox 'Color' has items"
    if StilFarbe.GetItemCount < 1 then
        warnlog "No style colors available"
    end if
    '/// Increase 'Spacing to contents Left'
    printlog " Increase 'Spacing to contents Left'"
    Links.More
    '/// Decrease 'Spacing to contents Right'
    printlog " Decrease 'Spacing to contents Right'"
    Rechts.Less
    '/// Increase 'Spacing to contents Top'
    printlog " Increase 'Spacing to contents Top'"
    Oben.More
    '/// Decrease 'Spacing to contents Bottom'
    printlog " Decrease 'Spacing to contents Bottom'"
    Unten.Less
    '/// Check 'Synchronize'
    printlog " Check 'Synchronize'"
    Synchronisieren.Check
    '/// Choose second position for 'Shadow Style'
    printlog " Choose second position for 'Shadow Style'"
    Position.TypeKeys "<Right>"
    '/// Increase 'Distance'
    printlog " Increase 'Distance'"
    Groesse.More
    '/// Verify that the 'Color' listbox has items
    printlog " Verify that the 'Color' listbox has items"
    if SchattenFarbe.GetItemCount < 1 then 
       warnlog "The listbox for color is empty"
    end if

    '/// Switch to tabpage 'Background'
    printlog " Switch to tabpage 'Background'"
    Kontext
    active.SetPage TabHintergrund
    Kontext "TabHintergrund"
    '/// Choose first entry 'Color'
    printlog " Choose first entry 'Color'"
    Als.Select 1
    Call DialogTest ( TabHintergrund,1 )
    '/// Choose next color
    printlog " Choose next color"
    Hintergrundfarbe.TypeKeys "<Down>"
    '/// Choose second entry 'Graphic'
    printlog " Choose second entry 'Graphic'"
    Als.Select 2
    Call DialogTest ( TabHintergrund,2 )
    '/// Click on 'Browse' – button
    printlog " Click on 'Browse' – button"
    Durchsuchen.Click
    Kontext "GrafikEinfuegenDlg"
    Call DialogTest (GrafikEinfuegenDlg)
    '/// Enter filename 'global\input\graf_inp\'
    printlog " Enter filename 'global\input\graf_inp\borabora.jpg'"
    Dateiname.Settext convertpath(gTesttoolpath + "global\input\graf_inp\borabora.jpg")
    '/// Verify that there are more than one filter available
    printlog " Verify that there are more than one filter available"
    if Dateityp.GetItemCount < 2 then
        warnlog "The filterlist is empty"
    end if
    '/// Check 'Link' – option
    printlog " Check 'Link' – option"
    Link.Check
    '/// Check 'Preview' - option
    printlog " Check 'Preview' - option"
    Preview.Check
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    GrafikEinfuegenDlg.Cancel
    Kontext "TabHintergrund"
    '/// Choose 'Type' 'Position'
    printlog " Choose 'Type' 'Position'"
    Position.Check
    '/// Check 'Preview'
    printlog " Check 'Preview'"
    Vorschau.Check

    '/// Switch to tabpage 'Header'
    printlog " Switch to tabpage 'Header'"
    Kontext
    active.SetPage TabKopfzeile
    Kontext "TabKopfzeile"
    Call DialogTest ( TabKopfzeile )
    '/// Check option 'Header on'
    printlog " Check option 'Header on'"
    KopfzeileMitAbstand.Check
    '/// Check option 'Same content left/right'
    printlog " Check option 'Same content left/right'"
    InhaltLinksRechtsGleich.Check
    '/// Increase 'Left margin'
    printlog " Increase 'Left margin'"
    LinkerRandKopfzeile.More
    '/// Decrease 'Right margin'
    printlog " Decrease 'Right margin'"
    RechterRandKopfzeile.Less
    '/// Increase 'Spacing'
    printlog " Increase 'Spacing'"
    Abstand.More
    '/// Decrease 'Height'
    printlog " Decrease 'Height'"
    Hoehe.Less
    '/// Check option 'AutoFit height'
    printlog " Check option 'AutoFit height'"
    Hoehedynamisch.Check
    '/// Click 'More' – button
    printlog " Click 'More' – button"
    Zusaetze.Click
    '/// Switch to tabpage 'Borders'
    printlog " Switch to tabpage 'Borders'"
    Kontext
    Active.SetPage TabUmrandung
    Kontext "TabUmrandung"
    Call DialogTest (TabUmrandung)
    '/// Switch to tabpage 'Background'
    printlog " Switch to tabpage 'Background'"
    Kontext
    active.SetPage TabHintergrund
    Kontext "TabHintergrund"
    '/// Choose first entry 'Color'
    printlog " Choose first entry 'Color'"
    Als.Select 1
    Call DialogTest ( TabHintergrund,1 )
    '/// Choose second entry 'Graphic'
    printlog " Choose second entry 'Graphic'"
    Als.Select 2
    Call DialogTest ( TabHintergrund,2 )
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    TabHintergrund.Cancel
    '/// Press 'Edit' – button
    printlog " Press 'Edit' – button"
    Kontext "TabKopfzeile"
    Bearbeiten.Click
    Kontext "TabKopfzeileCalc"
    Call DialogTest ( TabKopfzeileCalc )
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    TabKopfzeileCalc.Cancel
    '/// Switch to tabpage 'Footer'
    printlog " Switch to tabpage 'Footer'"
    Kontext
    active.SetPage TabFusszeile
    Kontext "TabFusszeile"
    DialogTest (TabFusszeile)
    '/// Check option 'Footer on'
    printlog " Check option 'Footer on'"
    FusszeileMitAbstand.Check
    '/// Check option 'Same content left/right'
    printlog " Check option 'Same content left/right'"
    InhaltLinksRechtsGleich.Check
    '/// Increase 'Left margin'
    printlog " Increase 'Left margin'"
    LinkerRandFusszeile.More
    '/// Decrease 'Right margin'
    printlog " Decrease 'Right margin'"
    RechterRandFusszeile.Less
    '/// Increase 'Spacing'
    printlog " Increase 'Spacing'"
    Abstand.More
    '/// Decrease 'Height'
    printlog " Decrease 'Height'"
    Hoehe.Less
    '/// Check option 'AutoFit height'
    printlog " Check option 'AutoFit height'"
    Hoehedynamisch.Check
    '/// Click 'More' – button
    printlog " Click 'More' – button"
    Zusaetze.Click
    '/// Switch to tabpage 'Borders'
    printlog " Switch to tabpage 'Borders'"
    Kontext
    Active.SetPage TabUmrandung
    Kontext "TabUmrandung"
    '/// Switch to tabpage 'Background'
    printlog " Switch to tabpage 'Background'"
    Kontext
    active.SetPage TabHintergrund
    Kontext "TabHintergrund"
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    TabHintergrund.Cancel
    '/// Press 'Edit' – button
    printlog " Press 'Edit' – button"
    Kontext "TabFusszeile"
    Bearbeiten.Click
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    Kontext "TabFusszeileCalc"
    TabFusszeileCalc.Cancel
    
    '/// Switch to tabpage 'Sheet'
    printlog " Switch to tabpage 'Sheet'"
    Kontext
    active.SetPage TabTabelleCalc
    Kontext "TabTabelleCalc"
    '/// Check option 'Left to right, then down'
    printlog " Check option 'Left to right, then down'"
    SeitenreihenfolgeLinksRechts.Check
    '/// Check option 'First page number'
    printlog " Check option 'First page number'"
    ErsteSeitenzahl.Check
    '/// Increase 'Page number'
    printlog " Increase 'Page number'"
    NummerErsteSeite.More
    '/// Check option 'Column and row headers'
    printlog " Check option 'Column and row headers'"
    ZeilenSpaltenKoepfe.Check
    '/// Check option 'Grid'
    printlog " Check option 'Grid'"
    Tabellengitter.Check
    '/// Check option 'Notes'
    printlog " Check option 'Notes'"
    Notizen.Check
    '/// Check option 'Objects/grphics'
    printlog " Check option 'Objects/grphics'"
    GrafikenObjekte.Check
    '/// Check option 'Charts'
    printlog " Check option 'Charts'"
    Diagramme.Check
    '/// Check option 'Drawing objects'
    printlog " Check option 'Drawing objects'"
    Zeichenobjekte.Check
    '/// Check option 'Formulas'
    printlog " Check option 'Formulas'"
    Formeln.Check
    '/// Check option 'Zero values'
    printlog " Check option 'Zero values'"
    Nullwerte.Check
    '/// Choose first 'Scaling mode' (Reduce/enlarge printout)
    printlog " Choose first 'Scaling mode' (Reduce/enlarge printout)"
    ScalingMode.Select 1
    DialogTest ( TabTabelleCalc,1 )
    '/// Increase 'scaling factor'
    printlog " Increase 'scaling factor'"
    SkalierungProzent.More
    '/// Choose second 'Scaling mode' (Fit print range to width/height)
    printlog " Choose second 'Scaling mode' (Fit print range to width/height)"
    ScalingMode.Select 2
    DialogTest ( TabTabelleCalc,2 )
    '/// Increase 'Width in pages'
    printlog " Increase 'Width in pages'"
    ScalePageWidth.More
    '/// Increase 'Height in pages'
    printlog " Increase 'Height in pages'"
    ScalePageHeight.More
    '/// Choose third 'Scaling mode' (Fit print range on number of pages)
    printlog " Choose third 'Scaling mode' (Fit print range on number of pages)"
    ScalingMode.Select 3
    DialogTest ( TabTabelleCalc,3 )
    '/// Increase 'Number of pages'
    printlog " Increase 'Number of pages'"
    AnzahlMaxSeiten.More
    '/// Close 'Page Style' – dialog with 'Cancel'
    printlog " Close 'Page Style' – dialog with 'Cancel'"
    TabTabelleCalc.Close
    '/// Set support for CJK and CTL back to default (Tools – Options – Language Support)
    printlog " Set support for CJK and CTL back to default (Tools – Options – Language Support)"
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if
    if bCTLLanguage = FALSE then
        Call ActiveDeactivateCTLSupport(FALSE)
    end if
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

