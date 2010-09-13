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
'* short description : Resource Test - Edit Menu Part II
'*
'************************************************************************
'*
' #1 tEditFill
' #1 tEditDeleteContent
' #1 tEditDeleteCells
' #1 tEditSheet
' #1 tEditDeleteManualBreak
' #1 tEditLinks
' #1 tEditPlugIn
' #1 tEditImageMap
' #1 tEditObjectEdit
'*
'\***********************************************************************

sub c_upd_editmenu2

    Printlog Chr(13) + "--------- EDIT Menu Part II (c_upd_editmenu2.inc) ---------"
    
    call tEditFill
    call tEditDeleteContent
    call tEditDeleteCells
    call tEditSheet
    call tEditDeleteManualBreak
    call tEditLinks
    call tEditPlugIn
    call tEditImageMap
    call tEditObjectEdit

end sub

'-----------------------------------------------------------

testcase tEditFill
'///<u><b>Edit - Fill</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Enter '1' in cell C5
    printlog " Enter '1' in cell C5"
    Kontext "DocumentCalc"
    call fCalcSelectCell ("C5")
    DocumentCalc.typekeys "1<Return>"
    '/// Select cells C5:C10
    printlog " Select cells C5:C10"
    call fCalcSelectCell ("C5:C10")
    '/// Fill selected area via 'Edit - Fill – Down'
    printlog " Fill selected area via 'Edit - Fill – Down'"
    EditFillDown
    '/// Select cells C1:C5
    printlog " Select cells C1:C5"
    call fCalcSelectCell ("C1:C5")
    '/// Fill selected area via 'Edit – Fill – Up'
    printlog " Fill selected area via 'Edit – Fill – Up'"
    EditFillUp
    '/// Select cells A5:C5
    printlog " Select cells A5:C5"
    call fCalcSelectCell ("A5:C5")
    '/// Fill selected area via 'Edit – Fill – Left'
    printlog " Fill selected area via 'Edit – Fill – Left'"
    EditFillLeft
    '/// Select cells C5:F5
    printlog " Select cells C5:F5"
    call fCalcSelectCell ("C5:F5")
    '/// Fill selected area via 'Edit – Fill – Right'
    printlog " Fill selected area via 'Edit – Fill – Right'"
    EditFillRight
    '/// Select cell A1:F10
    printlog " Select cell A1:F10"
    call fCalcSelectCell ("A1:F10")
    '/// Open fill-series-dialog via 'Edit – Fill – Series'
    printlog " Open fill-series-dialog via 'Edit – Fill – Series'"
    EditFillSeries
    Kontext "ReiheFuellen"
    DialogTest ( ReiheFuellen )
    '/// Check Direction 'Right'
    printlog " Check Direction 'Right'"
    Rechts.Check
    '/// Check Direction 'Up'
    printlog " Check Direction 'Up'"
    Oben.Check
    '/// Check Direction 'Left'
    printlog " Check Direction 'Left'"
    Links.Check
    '/// Check Direction 'Down'
    printlog " Check Direction 'Down'"
    Unten.Check
    '/// Check Series Type 'Linear'
    printlog " Check Series Type 'Linear'"
    Arithmetisch.Check
    '/// Check Series Type 'Growth'
    printlog " Check Series Type 'Growth'"
    Geometrisch.Check
    '/// Check Series Type 'AutoFill'
    printlog " Check Series Type 'AutoFill'"
    AutoAusfuellen.Check
    '/// Check Series Type 'Date'
    printlog " Check Series Type 'Date'"
    Datum.Check
    '/// Check Time Unit 'Day'
    printlog " Check Time Unit 'Day'"
    Tag.Check
    '/// Check Time Unit 'Weekday'
    printlog " Check Time Unit 'Weekday'"
    WochenTag.Check
    '/// Check Time Unit 'Month'
    printlog " Check Time Unit 'Month'"
    Monat.Check
    '/// Check Time Unit 'Year'
    printlog " Check Time Unit 'Year'"
    Jahr.Check
    '/// Verify that 'Start Value' is disabled
    printlog " Verify that 'Start Value' is disabled"
    if StartWert.IsEnabled then warnlog "StartValue is enabled but should be disabled"
    '/// Enter End value '10'
    printlog " Enter End value '10'"
    EndWert.SetText 10
    '/// Enter Increment '2'
    printlog " Enter Increment '2'"
    Inkrement.SetText 2
    '/// Close dialog via 'Cancel'
    printlog " Close dialog via 'Cancel'"
    ReiheFuellen.Cancel
    '/// Select all sheets by context menu on sheettab
    printlog " Select all sheets by context menu on sheettab"
    AlleTabellenAuswaehlen
    '/// Open 'Fill Sheets' dialog by 'Edit - Fill - Sheets'
    printlog " Open 'Fill Sheets' dialog by 'Edit - Fill - Sheets'"
    EditFillSheet
    Kontext "TabellenFuellen"
    Dialogtest ( TabellenFuellen )
    '/// Uncheck 'Paste all'
    printlog " Uncheck 'Paste all'"
    AuswahlAllesEinfuegen.UnCheck
    '/// Check 'Text'
    printlog " Check 'Text'"
    AuswahlZeichenketten.Check
    '/// Check 'Numbers'
    printlog " Check 'Numbers'"
    AuswahlZahlen.Check
    '/// Check 'Date & Time'
    printlog " Check 'Date & Time'"
    AuswahlDatumZeit.Check
    '/// Check 'Formulas'
    printlog " Check 'Formulas'"
    AuswahlFormeln.Check
    '/// Check 'Notes'
    printlog " Check 'Notes'"
    AuswahlNotizen.Check
    '/// Check 'Formats'
    printlog " Check 'Formats'"
    AuswahlFormate.Check
    '/// Check 'Skip empty cells'
    printlog " Check 'Skip empty cells'"
    LeerzeilenUeberspringen.Check
    '/// Verify that 'Transpose' is disabled
    printlog " Verify that 'Transpose' is disabled"
    if Transponieren.IsEnabled then warnlog "Transpose is enabled but should be disabled"
    '/// Check 'Link'
    printlog " Check 'Link'"
    Verknuepfen.Check
    '/// Check 'None'
    printlog " Check 'None'"
    RechenopperationenKeine.Check
    '/// Check 'Add'
    printlog " Check 'Add'"
    RechenopperationenAddieren.Check
    '/// Check 'Subtract'
    printlog " Check 'Subtract'"
    RechenopperationenSubtrahieren.Check
    '/// Check 'Multiply'
    printlog " Check 'Multiply'"
    RechenopperationenMultiplizieren.Check
    '/// Check 'Divide'
    printlog " Check 'Divide'"
    RechenopperationenDividieren.Check
    '/// Verify that 'Dont shift' is disabled
    printlog " Verify that 'Dont shift' is disabled"
    if NichtVerschieben.IsEnabled then warnlog "Transpose is enabled but should be disabled"
    '/// Verify that 'Down' is disabled
    printlog " Verify that 'Down' is disabled"
    if NachUnten.IsEnabled then warnlog "Transpose is enabled but should be disabled"
    '/// Verify that 'Right' is disabled
    printlog " Verify that 'Right' is disabled"
    if NachRechts.IsEnabled then warnlog "Transpose is enabled but should be disabled"
    '/// Close 'Fill Sheets' dialog with 'Cancel'
    printlog " Close 'Fill Sheets' dialog with 'Cancel'"
    TabellenFuellen.Cancel
    '/// Close document
    printlog " Close document"
    hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditDeleteContent
'///<u><b>Edit - Delete Content</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Open 'Delete Content' dialog via 'Edit – Delete content'
    printlog " Open 'Delete Content' dialog via 'Edit – Delete content'"
    EditDeleteContents
    Kontext "InhalteLoeschen"
    DialogTest ( InhalteLoeschen )
    '/// Check 'Delete All'
    printlog " Check 'Delete All'"
    Alles.Check
    '/// Check that all other controls are now disabled
    printlog " Check that all other controls are now disabled"
    if Zeichenketten.IsEnabled then warnlog "Option is enabled but should be disabled"
    '/// Uncheck 'Delete All'
    printlog " Uncheck 'Delete All'"
    Alles.UnCheck
    '/// Check 'Text'
    printlog " Check 'Text'"
    Zeichenketten.Check
    '/// Check 'Numbers'
    printlog " Check 'Numbers'"
    Zahlen.Check
    '/// Check 'Date & Time'
    printlog " Check 'Date & Time'"
    DatumZeit.Check
    '/// Check 'Formulas'
    printlog " Check 'Formulas'"
    Formeln.Check
    '/// Check 'Notes'
    printlog " Check 'Notes'"
    Notizen.Check
    '/// Check 'Formats'
    printlog " Check 'Formats'"
    Formate.Check
    '/// Check 'Objects'
    printlog " Check 'Objects'"
    Objekte.Check
    '/// Close dialog via 'Cancel'
    printlog " Close dialog via 'Cancel'"
    InhalteLoeschen.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditDeleteCells
'///<u><b>Edit - Delete Cells</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Open 'Delete Cells' dialog via 'Edit – Delete Cells'
    printlog " Open 'Delete Cells' dialog via 'Edit – Delete Cells'"
    EditDeleteCells
    Kontext "ZellenLoeschen"
    DialogTest ( ZellenLoeschen )
    '/// Check 'Shift cells up'
    printlog " Check 'Shift cells up'"
    Zellennachoben.Check
    '/// Check 'Shift cells left'
    printlog " Check 'Shift cells left'"
    Zellennachlinks.Check  
    '/// Check 'Delete entire row'
    printlog " Check 'Delete entire row'"
    GanzeZeilen.Check
    '/// Check 'Delete entire column'
    printlog " Check 'Delete entire column'"
    GanzeSpalten.Check
    '/// Close dialog via 'Cancel'
    printlog " Close dialog via 'Cancel'"
    ZellenLoeschen.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditSheet
'///<u><b>Edit - Sheet</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Open 'Edit – Sheet – Move/Copy Sheet' dialog
    printlog " Open 'Edit – Sheet – Move/Copy Sheet' dialog"
    EditCopyMoveSheet
    Kontext "TabelleVerschiebenKopieren"
    DialogTest ( TabelleVerschiebenKopieren )
    '/// Check that there are more than one entry in listbox 'To document'
    printlog " Check that there are more than one entry in listbox 'To document'"
    if InsDokument.GetItemCount < 1 then warnlog "The listbox is empty"
    '/// Check that there are four entries in 'Insert before'
    printlog " Check that there are four entries in 'Insert before'" 
    if EinfuegenVor.GetItemCount < 1 then warnlog "The listbox is empty"
    '/// Check 'Copy'
    printlog " Check 'Copy'"
    Kopieren.Check
    '/// Close dialog via 'Cancel'
    printlog " Close dialog via 'Cancel'"
    TabelleVerschiebenKopieren.Cancel
    '/// Open 'Edit – Sheet – Select' dialog
    printlog " Open 'Edit – Sheet – Select' dialog"
    EditSheetSelect
    Kontext "SelectSheets"
    DialogTest ( SelectSheets )
    '/// Check that there are three entries in listbox
    printlog " Check that there are three entries in listbox"
    if not SheetSelectionBox.GetItemCount = 3 then warnlog "The amount of sheets is wrong, should be 3 but is" & SheetSelectionBox.GetItemCount
    '/// Close dialog via 'Cancel'
    printlog " Close dialog via 'Cancel'"
    SelectSheets.Cancel
    '/// Choose 'Edit – Sheet – Delete'
    printlog " Choose 'Edit – Sheet – Delete'"
    EditDeleteSheet
    '/// Now a messagebox appears. Close messagebox with 'No'
    printlog " Now a messagebox appears. Close messagebox with 'No'"
    Kontext
    if active.exists then
       DialogTest ( Active )   
       Active.No
    else
       warnlog "Edit::Sheet::Delete - no removal warning appeared!"
    end if
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditDeleteManualBreak
'///<u><b>Edit - Delete - Manual Break</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Select cell H10
    printlog " Select cell H10"
    call fCalcSelectRange ("H10")
    '/// Insert a manual rowbreak via 'Insert – Manual Break – Row Break'
    printlog " Insert a manual rowbreak via 'Insert – Manual Break – Row Break'"
    InsertManuelBreakRowBreak
    '/// Insert a manual columnbreak via 'Insert – Manual Break – Column Break'
    printlog " Insert a manual columnbreak via 'Insert – Manual Break – Column Break'"
    InsertManuelBreakColumnBreak
    '/// Delete manual rowbreak via 'Edit – Delete Manual Break – Row Break'
    printlog " Delete manual rowbreak via 'Edit – Delete Manual Break – Row Break'"
    EditRemoveManualBreakRowBreak
    '/// Delete manual columnbreak via 'Edit – Delete Manual Break – Column Break'
    printlog " Delete manual columnbreak via 'Edit – Delete Manual Break – Column Break'"
    EditRemoveManualBreakColumnBreak
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditLinks
'///<u><b>Edit - Links</b></u>
    
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open Insert graphic dialog via 'Insert – Picture – From File'
    printlog " Open Insert graphic dialog via 'Insert – Picture – From File'"
    InsertGraphicsFromFile
    '/// Set filename ' global/input/graf_inp/borabora.jpg'
    printlog " Set filename ' global/input/graf_inp/borabora.jpg'"
    Kontext "GrafikEinfuegenDlg"
    Dateiname.SetText ConvertPath ( gTesttoolPath + "global\input\graf_inp\borabora.jpg" )
    '/// Check 'Link'
    printlog " Check 'Link'"
    Link.Check
    '/// Close dialog with 'OK'
    printlog " Close dialog with 'OK'"
    Oeffnen.Click
    '/// Open 'Edit – Links' dialog
    printlog " Open 'Edit – Links' dialog"
    Kontext "DocumentCalc"
    EditLinksCalc
    Kontext "VerknuepfungenBearbeiten"
    VerknuepfungenBearbeiten.Exists (2)
    Call DialogTest ( VerknuepfungenBearbeiten )
    '/// Click on 'Update' – button
    printlog " Click on 'Update' – button"
    Aktualisieren.Click
    '/// Click on 'Modify...' - button
    printlog " Click on 'Modify...' - button"
    Aendern.Click
    '/// Close 'Link graphics' – dialog with 'Cancel'
    printlog " Close 'Link graphics' – dialog with 'Cancel'"
    Kontext "GrafikEinfuegenDLG"
    Call Dialogtest ( GrafikEinfuegenDLG )
    GrafikEinfuegenDLG.Cancel
    '/// Click on 'Break Link' – button
    printlog " Click on 'Break Link' – button"
    Kontext "VerknuepfungenBearbeiten"
    loesen.Click
    '/// Close messagebox with 'Yes'
    printlog " Close messagebox with 'Yes'"
    Kontext
    If Active.Exists then
        Active.Yes
    else
        Warnlog "No request by deleting the link"
    end if
    Kontext "VerknuepfungenBearbeiten"
    '/// Close 'Edit Links' – dialog
    printlog " Close 'Edit Links' – dialog"
    VerknuepfungenBearbeiten.Close
    '/// Close document
    printlog " Close document"
    call hCloseDocument
    '/// Load testfile ' spreadsheet/required/input/linkclient.ods ' and allow update
    printlog " Load testfile ' spreadsheet/required/input/linkclient.ods ' and allow update"
    Call hFileOpen ( ConvertPath ( gTesttoolPath + "spreadsheet\required\input\linkclient.ods"))
	printlog " Confirm Update Links dialog with YES"
	Kontext
	If Active.Exists then
		Active.Yes
	else
		warnlog "No dialog for updating links exists"
	end if
    '/// Open 'Edit – Links' dialog
    printlog " Open 'Edit – Links' dialog"
    EditLinksCalc
    Kontext "VerknuepfungenBearbeiten"
    '/// Click on 'Update' – button
    printlog " Click on 'Update' – button"
    Aktualisieren.Click
    '/// Click on 'Modify...' - button to open 'Modify Link' dialog
    printlog " Click on 'Modify...' - button to open 'Modify Link' dialog"
    Aendern.Click
    Kontext "ModifyLink"
    Call Dialogtest ( ModifyLink )
    '/// Verify that the 'Application' editfield is available
    printlog " Verify that the 'Application' editfield is available"
    ModifyDDEApplication.GetText
    '/// Verify that the 'File' editfield is available
    printlog " Verify that the 'File' editfield is available"
    ModifyDDEFile.GetText
    '/// Verify that the 'Category' editfield is available
    printlog " Verify that the 'Category' editfield is available"
    ModifyDDECategory.GetText
    '/// Close 'Modify Link' – dialog with 'Cancel'
    printlog " Close 'Modify Link' – dialog with 'Cancel'"
    ModifyLink.Cancel
    Kontext "VerknuepfungenBearbeiten"
    '/// Check that updateradiobuttons are enabled
    printlog " Check that updateradiobuttons are enabled"
    if not Automatisch.IsEnabled then
        warnlog "Updateradiobutton is disabled but should be enabled"
    end if
    '/// Click on 'Break Link' – button
    printlog " Click on 'Break Link' – button"
    Loesen.Click
    '/// Accept messagebox with 'Yes'
    printlog " Accept messagebox with 'Yes'"
    Kontext
    If Active.Exists then
        Active.Yes
    end if
    Kontext "VerknuepfungenBearbeiten"
    '/// Check that updateradiobuttons are disabled
    printlog " Check that updateradiobuttons are disabled"
    if Manuell.IsEnabled then
        warnlog "Updateradiobutton is enabled but should be disabled"
    end if
    '/// Close 'Edit Links' – dialog
    printlog " Close 'Edit Links' – dialog"
    VerknuepfungenBearbeiten.Close
    '/// Close document
    printlog " Close document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditPlugIn
'///<u><b>Edit - PlugIn</b></u>
    
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Edit – PlugIn
    printlog " Edit – PlugIn"
    EditPlugIn
    '/// Edit – PlugIn
    printlog " Edit – PlugIn"
    EditPlugIn
    '/// Close document
    printlog " Close document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditImageMap
'///<u><b>Edit - ImageMap</b></u>
    
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    call hNewDocument
    '/// Open Insert graphic dialog via 'Insert – Picture – From File'
    printlog " Open Insert graphic dialog via 'Insert – Picture – From File'"
    InsertGraphicsFromFile
    '/// Set filename ' global/input/graf_inp/borabora.jpg'
    printlog " Set filename ' global/input/graf_inp/borabora.jpg'"
    Kontext "GrafikEinfuegenDlg"
    Dateiname.SetText ConvertPath ( gTesttoolPath + "global\input\graf_inp\borabora.jpg" )
    '/// Close dialog with 'OK'
    printlog " Close dialog with 'OK'"
    Oeffnen.Click
    '/// Open ImageMap – dialog via 'Edit – ImageMap'
    printlog " Open ImageMap – dialog via 'Edit – ImageMap'"
    EditImageMap
    Kontext "ImageMapEditor"
    Call Dialogtest ( ImageMapEditor )
    '/// Click on Open button
    printlog " Click on Open button"
    Oeffnen.Click
    Kontext "GeneralFileDialog"
    '/// Close 'Open'-dialog with Cancel
    printlog " Close 'Open'-dialog with Cancel"
    GeneralFileDialog.Cancel
    Kontext "ImageMapEditor"
    '/// Click on 'Save'-button
    printlog " Click on 'Save'-button"
    Speichern.Click
    Kontext "SpeichernDlg"
    '/// Close 'Save'-dialog with Cancel
    printlog " Close 'Save'-dialog with Cancel"
    SpeichernDlg.Cancel
    Kontext "ImageMapEditor"
    '/// Click on 'Rectangle'button and draw a rectangle by mouse
    printlog " Click on 'Ellipse'button and draw an ellipse by mouse"
    Ellipse.Click
    Dokument.MouseDown 50,50
    Dokument.MouseMove 50,50,10,10
    Dokument.MouseUp 10,10
    '/// Click on 'Active'-button
    printlog " Click on 'Active'-button"
    Aktiv.Click
    '/// Click on 'Macro...'-button
    printlog " Click on 'Macro...'-button"
'    warnlog "#i102720# => crash, therefor this functionality is outcommented"
    Makro.Click
    Kontext "MakroZuweisen"
    Call DialogTest ( MakroZuweisen )
    '/// Close 'Assign Macro'-dialog with Cancel
    printlog " Close 'Assign Macro'-dialog with Cancel"
    MakroZuweisen.Cancel
    Kontext "ImageMapEditor"
    '/// Click on 'Properties'-button
    printlog " Click on 'Properties'-button"
    Eigenschaften.Click
    Kontext "ImageMapHyperlink"
    Call DialogTest ( ImageMapHyperlink )
    '/// Verify that the 'URL' editfield is available
    printlog " Verify that the 'URL' editfield is available"
    URL.GetText
    '/// Verify that the 'Alternativetext' editfield is available
    printlog " Verify that the 'Alternativetext' editfield is available"
    Alternativetext.GetText
    '/// Verify that the 'Frame' listbox has items
    printlog " Verify that the 'Frame' listbox has items"
    Frame.GetItemCount
    '/// Verify that the 'Description' editfield is available
    printlog " Verify that the 'Description' editfield is available"
    Description.GetText
    '/// Close 'Properties'-dialog with 'Cancel'
    printlog " Close 'Properties'-dialog with 'Cancel"
    ImageMapHyperlink.Cancel
    Kontext "ImageMapEditor"
    '/// Click on 'UNDO'-button
    printlog " Click on 'UNDO'-button"
    Rueckgaengig.Click
    '/// Close ImageMap with windowcloser
    printlog " Close ImageMap with windowcloser"
    ImageMapEditor.Close
    '/// Click on 'No'-button on savemessagebox
    printlog " Click on 'No'-button on savemessagebox"
    Kontext
    if active.exists then
    Active.No
    end if
    '/// Close document
    printlog " Close document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tEditObjectEdit
'///<u><b>Edit - Object - Edit</b></u>
    
    '/// Load testdocument 'spreadsheet/required/input/tEditObject.ods'
    printlog " Load testdocument 'spreadsheet/required/input/tEditObject.ods'"
    hFileOpenLocally( gTesttoolPath + "spreadsheet\required\input\tEditObject.ods" )
    '/// Select OLE-object
    printlog " Select OLE-object"
    fselectFirstOLE
    '/// Activate OLE-object by Edit – Object – Edit
    printlog " Activate OLE-object by Edit – Object – Edit"
    EditObjectEdit
    '/// Deactivate OLE-object
    printlog " Deactivate OLE-object"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<ESCAPE>" , 2
    '/// Select OLE-object
    printlog " Select OLE-object"
    fselectFirstOLE
    '/// Open 'Save As' – dialog by Edit – Object – Save As
    printlog " Open 'Save As' – dialog by Edit – Object – Save As"
    EditObjectSaveCopyAsCalc
    Kontext "SpeichernDlg"
    '/// Close dialog with Cancel
    printlog " Close dialog with Cancel"
    SpeichernDlg.Cancel
    '/// Close document
    printlog " Close document"
    call hCloseDocument

endcase
