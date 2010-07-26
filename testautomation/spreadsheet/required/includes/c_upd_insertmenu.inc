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
'* short description : Resource Test - Insert Menu
'*
'************************************************************************
'*
' #1 tInsertManualBreak
' #1 tInsertCells
' #1 tInsertRowsColumns
' #1 tInsertSheet
' #1 tInsertSheetFromFile
' #1 tInsertLink
' #1 tInsertSpecialCharacters
' #1 tInsertFormattingMark
' #1 tInsertHyperlink
' #1 tInsertFunction
' #1 tInsertFunctionList
' #1 tInsertNamesDefine
' #1 tInsertNamesInsert
' #1 tInsertNamesCreate
' #1 tInsertNamesLabels
' #1 tInsertNote
' #1 tInsertPictureFromFile
' #1 tInsertMovieSound
' #1 tInsertObjectOLE
' #1 tInsertObjectPlugin
' #1 tInsertObjectSound
' #1 tInsertObjectVideo
' #1 tInsertObjectFormular
' #1 tInsertChart
' #1 tInsertFloatingFrame

'*
'\***********************************************************************

sub c_upd_insertmenu

    Printlog Chr(13) + "--------- Insert Menu (c_upd_insertmenu.inc) ---------"
    
    call tInsertManualBreak
    call tInsertCells
    call tInsertRowsColumns
    call tInsertSheet
    call tInsertSheetFromFile
    call tInsertLink
    call tInsertSpecialCharacters
    call tInsertFormattingMark
    call tInsertHyperlink
    call tInsertFunction
    call tInsertFunctionList
    call tInsertNamesDefine
    call tInsertNamesInsert
    call tInsertNamesCreate
    call tInsertNamesLabels
    call tInsertNote
    call tInsertPictureFromFile
    call tInsertMovieSound
    call tInsertObjectOLE
    call tInsertObjectPlugin
    call tInsertObjectSound
    call tInsertObjectVideo
    call tInsertObjectFormular
    call tInsertChart
    call tInsertFloatingFrame

end sub

'-----------------------------------------------------------

testcase tInsertManualBreak
'///<u><b>Insert - Manual Break</b></u>
    
    '/// Opening new spreadsheet document for getting defined starting environment
    printlog " Opening new spreadsheet document for getting defined starting environment"
    call hNewDocument
    '/// Goto cell D5
    printlog " Goto cell D5"
    call fCalcSelectRange ("D5")
    '/// Insert a manual row break by 'Insert – Manual Break – Row Break'
    printlog " Insert a manual row break by 'Insert – Manual Break – Row Break'"
    InsertManuelBreakRowBreak
    '/// Insert a manual column break by 'Insert – Manual Break – Column Break'
    printlog " Insert a manual column break by 'Insert – Manual Break – Column Break'"
    InsertManuelBreakColumnBreak
    '/// Close starting document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertCells
'///<u><b>Insert - Cells</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Insert Cells'- dialog by 'Insert – Cells'
    printlog " Open 'Insert Cells'- dialog by 'Insert – Cells'"
    InsertCells
    Kontext "ZellenEinfuegen"
    Call DialogTest ( ZellenEinfuegen )
    '/// Check option 'Shift cells down'
    printlog " Check option 'Shift cells down'"
    ZellenNachUnten.Check
    '/// Check option 'Shift cells right'
    printlog " Check option 'Shift cells right'"
    ZellenNachRechts.Check
    '/// Check option 'Entire row'
    printlog " Check option 'Entire row'"
    GanzeZeile.Check
    '/// Check option 'Entire column'
    printlog " Check option 'Entire column'"
    GanzeSpalte.Check
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    ZellenEinfuegen.Cancel
    '/// Close starting document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertRowsColumns
'///<u><b>Insert - Rows/Columns</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Insert a new row by 'Insert - Rows'
    printlog " Insert a new row by 'Insert - Rows'"
    InsertRowsCalc
    '/// Insert a new column by 'Insert - Columns'
    printlog " Insert a new column by 'Insert - Columns'"
    InsertColumnsCalc
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertSheet
'///<u><b>Insert - Sheet</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Insert Sheet' – dialog by 'Insert – Sheet'
    printlog " Open 'Insert Sheet' – dialog by 'Insert – Sheet'"
    InsertSheetCalc
    Kontext "TabelleEinfuegenCalc"
    Call DialogTest ( TabelleEinfuegenCalc )
    '/// Check option 'After current sheet'
    printlog " Check option 'After current sheet'"
    Nach.Check
    '/// Set 'Name' to 'Test'
    printlog " Set 'Name' to 'Test'"
    TabellenName.SetText "Test"
    '/// Set number of sheets to 2
    printlog " Set number of sheets to 2"
    Anzahl.More
    '/// Check option 'From File'
    printlog " Check option 'From File'"
    AusDateiErstellen.Check
    '/// Check option 'Link'
    printlog " Check option 'Link'"
    Verknuepfen.Check
    '/// Click on 'Browse' – button
    Durchsuchen.Click
    Kontext "OeffnenDLG"
    '/// Close 'Insert' – dialog with 'Cancel'
    printlog " Close 'Insert' – dialog with 'Cancel'"
    OeffnenDLG.Cancel
    Kontext "TabelleEinfuegenCalc"
    '/// Close 'Insert Sheet' – dialog with 'Cancel'
    printlog " Close 'Insert Sheet' – dialog with 'Cancel'"
    TabelleEinfuegenCalc.Cancel
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertSheetFromFile
'///<u><b>Insert - Sheet From File</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Insert Sheet' – dialog by 'Insert – Sheet From File'
    printlog " Open 'Insert Sheet' – dialog by 'Insert – Sheet From File'"
    InsertSheetFromFile
    Kontext "OeffnenDLG"
    '/// Set filename 'spreadsheet/required/input/twosheets.ods'
    printlog " Set filename 'spreadsheet/required/input/twosheets.ods'"
    Dateiname.setText ( convertpath ( gTesttoolPath & "spreadsheet/required/input/twosheets.ods" ) )
    '/// Close 'Insert' – dialog with 'Insert'
    printlog " Close 'Insert' – dialog with 'Insert'"
    Oeffnen.Click
    Kontext "TabelleEinfuegenCalc"
    '/// Verify that the selected file has two sheets
    printlog " Verify that the selected file has two sheets"
    if Auswahl.GetItemCount <> 2 then
        warnlog "The file has more/less than two sheets"
    end if
    '/// Close 'Insert Sheet' – dialog with 'Insert'
    printlog " Close 'Insert Sheet' – dialog with 'Insert'"
    TabelleEinfuegenCalc.Cancel
    '/// Close document'
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertLink
'///<u><b>Insert - Link</b></u>

    '/// Open new Sreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'External Data' – dialog by 'Insert – Link To External Data'
    printlog " Open 'External Data' – dialog by 'Insert – Link To External Data'"
    InsertLinkToExternalData
    Kontext "ExternalData"
    call Dialogtest (ExternalData)
    '/// Press browse button for URL
    printlog " Press browse button for URL"
    FileChoose.Click
    Kontext "OeffnenDlg"
    '/// Close 'Insert' – dialog with 'Cancel'
    printlog " Close 'Insert' – dialog with 'Cancel'"
    OeffnenDlg.close
    Kontext "ExternalData"
    '/// Check option 'Updata every'
    printlog " Check option 'Updata every'"
    Update.Check
    '/// Increase updatetimer by 1
    printlog " Increase updatetimer by 1"
    UpdateTime.More
    '/// Close 'External Data' – dialog with 'Cancel'
    printlog " Close 'External Data' – dialog with 'Cancel'"
    ExternalData.close
    '/// Close starting document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertSpecialCharacters
'///<u><b>Insert - Special Characters</b></u>

    '///Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Special Characters' – dialog by 'Insert – Special Characters'
    printlog " Open 'Special Characters' – dialog by 'Insert – Special Characters'"
    InsertSpecialCharacterCalc
    Kontext "Sonderzeichen"
    DialogTest ( Sonderzeichen )
    '/// Change 'Font' to next one
    printlog " Change 'Font' to next one"
	if Schrift.GetItemCount = Schrift.GetSelIndex then
			Schrift.Select(Schrift.GetSelIndex -1)
		else Schrift.Select(Schrift.GetSelIndex +1)
	end if
    '/// Change 'Subset' to next one
    printlog " Change 'Subset' to next one"
    If Subset.IsVisible then
		if Subset.GetItemCount = Subset.GetSelIndex then
			Subset.Select(Subset.GetSelIndex -1)
		else Subset.Select(Subset.GetSelIndex +1)
		end if
    end if
    '/// Choose a character by clicking
    printlog " Choose a character by clicking"
    Zeichenanzeige.MouseDown 50,50
    Zeichenanzeige.MouseUp 50,50
    '/// Delete character by 'Delete'
    printlog " Delete character by 'Delete'"
    Loeschen.Click
    '/// Close 'Special Characters' – dialog with 'Cancel'
    printlog " Close 'Special Characters' – dialog with 'Cancel'"
    Sonderzeichen.Cancel
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertFormattingMark
'///<u><b>Insert - Formatting Mark</b></u>

    Dim bAsianLanguage as boolean
    Dim bCTLlanguage as boolean
    
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Enable support for CJK and CTL by 'Tools-Options-Language Settings'
    printlog " Enable support for CJK and CTL by 'Tools-Options-Language Settings'"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    bCTLLanguage = ActiveDeactivateCTLSupport(TRUE)
    '/// Select cell A1
    printlog " Select cell A1"
    call fCalcSelectRange ("A1")
    '/// Enter 'Test' in cell 'A1' to activate editmode
    printlog " Enter 'Test' in cell 'A1' to activate editmode"
    DocumentCalc.TypeKeys "Test"
    '/// Enter in cell 'Insert – Formatting Mark – Non-Breaking Space'
    printlog " Enter in cell 'A' and 'Insert – Formatting Mark – Non-Breaking Space'"
    InsertNonBreakingSpace
    '/// Enter in cell 'Insert – Formatting Mark – Non-Breaking Hyphen'
    printlog " Enter in cell 'Insert – Formatting Mark – Non-Breaking Hyphen'"
    InsertHardHyphen
    '/// Enter in cell 'Insert – Formatting Mark – Optional Hyphen'
    printlog " Enter in cell 'Insert – Formatting Mark – Optional Hyphen'"
    InsertSoftHyphen
    '/// Enter in cell 'Insert – Formatting Mark – No-Width Optional Break'
    printlog " Enter in cell 'Insert – Formatting Mark – No-Width Optional Break'"
    InsertZWSP
    '/// Enter in cell 'Insert – Formatting Mark – No-Width no break'
    printlog " Enter in cell 'Insert – Formatting Mark – No-Width no break'"
    InsertZWNBSP
    '/// Enter in cell 'Insert – Formatting Mark – Left-to-right-mark'
    printlog "/ Enter in cell 'Insert – Formatting Mark – Left-to-right-mark'"
    InsertLRM
    '/// Enter in cell 'Insert – Formatting Mark – right-to-left-mark'
    printlog " Enter in cell 'Insert – Formatting Mark – right-to-left-mark'"
    InsertRLM
    '/// Leave cell with 'Enter'
    printlog " Leave cell with 'Enter'"
    DocumentCalc.TypeKeys "<Return>"
    '/// Set support for CTL and CJK back to defaults
    printlog " Set support for CTL and CJK back to defaults"
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

testcase tInsertHyperlink
'///<u><b>Insert - Hyperlink</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Hyperlink' – dialog via 'Insert – Hyperlink'
    printlog " Open 'Hyperlink' – dialog via 'Insert – Hyperlink'"
    InsertHyperlink

    kontext "HyperlinkDialog"
    if ( HyperlinkDialog.exists( 2 ) ) then
    
        Kontext "Hyperlink"
        '/// Switch to category 'Internet' (first one)
        printlog " Switch to category 'Internet'"
        Auswahl.TypeKeys "<HOME>"
        '/// Choose 'Web'
        printlog " Choose 'Web'"
        Kontext "TabHyperlinkInternet"
        internet.check
        Call DialogTest ( TabHyperlinkInternet,1 )
        '/// Click on 'Target in Document' – button (the target dialog appears)
        printlog " Click on 'Target in Document' – button (the target dialog appears)"
        Target.Click
        Kontext "ZielImDokument"
        Call DialogTest ( ZielImDokument )
        '/// Close 'Target' – dialog with 'Close'
        printlog " Close 'Target' – dialog with 'Close'"
        ZielImDokument.Close
        Kontext "TabHyperlinkInternet"
        '/// Choose 'FTP'
        printlog " Choose 'FTP'"
        FTP.check
        Call DialogTest ( TabHyperlinkInternet,2 )
        '/// Check that the 'Target' – button is disabled
        printlog " Check that the 'Target' – button is disabled"
        if Target.IsEnabled then
            warnlog "The target button is enabled, but should be disabled"
        end if
        '/// Check option 'Anonymous User'
        printlog " Check option 'Anonymous User'"
        AnonymerBenutzer.Check
        '/// Check that 'Login Name' is disabled
        printlog " Check that 'Login Name' is disabled"
        if Loginname.IsEnabled then
            warnlog "The Loginname is enabled, but should be disabled"
        end if
        '/// Chose 'Telnet'
        printlog " Chose 'Telnet'"
        Telnet.check
        Call DialogTest ( TabHyperlinkInternet,3 )
        '/// Check that the 'Target' – button is disabled
        printlog " Check that the 'Target' – button is disabled"
        if Target.IsEnabled then
            warnlog "The target button is enabled, but should be disabled"
        end if
        '/// Switch to category 'Mail & News'
        Kontext "Hyperlink"
        Auswahl.TypeKeys "<DOWN>"
        Kontext "TabHyperlinkMailUndNews"
        Call DialogTest ( TabHyperlinkMailUndNews )
        '/// Click twice on 'Data Source' – button to turn on / off the databasebeamer
        printlog " Click twice on 'Data Source' – button to turn on / off the databasebeamer"
        Adressbuch.click
        Adressbuch.click
        '/// Check option 'Mail'
        printlog " Check option 'Mail'"
        EMail.check
        '/// Check that the editfield for 'Receiver' is enabled
        printlog " Check that the editfield for 'Receiver' is enabled"
        if not Receiver.IsEnabled then
            warnlog "Receiver is disabled but should be enabled"
        end if
        '/// Check that the editfield for 'Subject' is enabled
        printlog " Check that the editfield for 'Subject' is enabled"
        if not Betreff.IsEnabled then
            warnlog "Subject is disabled but should be enabled"
        end if
        '/// Check option 'News'
        printlog " Check option 'News'"
        News.check
        '/// Check that the editfield for 'Subject' is disabled
        printlog " Check that the editfield for 'Subject' is disabled"
        if Betreff.IsEnabled then
            warnlog "Subject is enabled but should be disabled"
        end if
        '/// Switch to category Document
        printlog " Switch to category Document"
        Kontext "Hyperlink"
        Auswahl.TypeKeys "<DOWN>"
        Kontext "TabHyperlinkDokument"
        Call DialogTest ( TabHyperlinkDokument )
        '/// Enter 'Test' in 'Path'
        printlog " Enter 'Test' in 'Path'"
        Path.TypeKeys "Test"
        '/// Click on 'Open' – button
        printlog " Click on 'Open' – button"
        DateiOeffnen.click
        Kontext "OeffnenDlg"
        '/// Close 'Open' – dialog with 'Cancel'
        printlog " Close 'Open' – dialog with 'Cancel'"
        OeffnenDlg.Cancel
        Kontext "TabHyperlinkDokument"
        '/// Click on 'Target in Document' – button (the target dialog appears)
        printlog " Click on 'Target in Document' – button (the target dialog appears)"
        Target.click
        Kontext "ZielImDokument"
        '/// Close 'Target' – dialog with 'Close'
        printlog " Close 'Target' – dialog with 'Close'"
        ZielImDokument.close
        Kontext "Hyperlink"
        '/// Switch to category 'New Document'
        printlog " Switch to category 'New Document'"
        Auswahl.TypeKeys "<DOWN>"
        Kontext "TabHyperlinkNeuesDokument"
        Call DialogTest ( TabHyperlinkNeuesDokument )
        '/// Check 'Edit Now'
        printlog " Click 'Edit Now'"
        SofortBearbeiten.check
        '/// Check 'Edit Later'
        printlog " Click 'Edit Later'"
        SpaeterBearbeiten.check
        '/// Click on 'Select Path' – button
        printlog " Click on 'Select Path' – button"
        Pfad.click
        Kontext "OeffnenDlg"
        '/// Close 'Select Path' – dialog with 'Cancel'
        printlog " Close 'Select Path' – dialog with 'Cancel'"
        OeffnenDlg.Cancel
        Kontext "TabHyperlinkNeuesDokument"
        '/// Check that the 'Frame' listbox has items
        printlog " Check that the 'Frame' listbox has items"
        if Frame.GetItemCount < 1 then
            warnlog "The Frame combobox is empty"
        end if
        '/// Check that the 'Form' listbox has 2 entries
        printlog " Check that the 'Form' listbox has 2 entries"
        if Form.GetItemCount <> 2 then
            warnlog "The Form combobox has not 2 items"
        end if
        '/// Close 'Hyperlink' – dialog with 'Close'
        printlog " Close 'Hyperlink' – dialog with 'Close'"
        kontext "HyperlinkDialog"
        HyperlinkDialog.Close()
    else
        warnlog( "Failed to open <HyperlinkDialog>" )
    endif
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertFunction
'///<u><b>Insert - Function</b></u>

    Dim ifunctionCount as integer
    Dim ifunctionIndex as integer

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open the 'Function Wizard' via 'Insert – Function'
    printlog " Open the 'Function Wizard' via 'Insert – Function'"
    InsertFunction
    Kontext "FunctionWizard"
    '/// Check that there are 13 entries in the category listbox
    printlog " Check that there are 13 entries in the category listbox"
    if CategoryLB.GetItemCount <> 13 then
        warnlog "There are " &  CategoryLB.GetItemCount & " categories instead of 13"
    end if
    '/// Choose the second entry (all) in the category listbox
    printlog " Choose the second entry (all) in the category listbox"
    CategoryLB.Select(2)
    '/// Choose all entries in the function list and click 'Next' button
    printlog " Choose all entries in the function list"
    ifunctionCount = FunctionLB.getItemCount      
    for ifunctionIndex = 1 To ifunctionCount
        CategoryLB.Select(2)
        FunctionLB.Select(ifunctionIndex)
'        sleep(1)
        printlog FunctionLB.GetSelText
        call DialogTest ( FunctionWizard,ifunctionIndex )
        '/// Click on 'Next' – button
        Forward.Click
        call DialogTest ( FunctionWizard,(ifunctionCount + ifunctionIndex) )
        Formel.typekeys "<MOD1 A><DELETE>", true
    next ifunctionIndex
    '/// Check option 'Array'
    printlog " Check option 'Array'"
    Matrix.check
    '/// Choose the third entry (database) in the category listbox
    printlog " Choose the third entry (database) in the category listbox"
    CategoryLB.Select(3)
    '/// Choose first function
    printlog " Choose first function"
    FunctionLB.Select (1)
    '/// Click on 'Next' button
    printlog " Click on 'Next' button"
    Forward.Click
    '/// Click on 'Shrink' – button; the range chooser appears
    printlog " Click on 'Shrink' – button; the range chooser appears"
    Shrink1.click
    '/// Enter 'A1' in rangechooser
    printlog " Enter 'A1' in rangechooser"
    Editfield1.SetText ("A1")    
    '/// Click on 'Maximize' – button
    printlog " Click on 'Maximize' – button"
    maximize.click
    '/// Click on 'fx' – button
    printlog " Click on 'fx' – button"
    Functionsbutton1.click
    '/// Switch to tabpage 'Structure'
    printlog " Switch to tabpage 'Structure'"
    Kontext "FunctionWizardTabControl"
    FunctionWizardTabControl.setpage TabFunctionWizardStructure
    Kontext "FunctionWizard"
    call DialogTest ( FunctionWizard,((ifunctionCount *2)+1) )
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    FunctionWizard.Cancel
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertFunctionList
'///<u><b>Insert - Function List</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Check if 'function list' is already opened
    printlog " Check if 'function list' is already opened"
    Kontext "FunctionsCalc"
    if FunctionsCalc.Exists(2) then
        printlog "Functions List was already open"
    else
        '/// Open 'function list' by 'Insert – Function List'
        printlog " Open 'function list' by 'Insert – Function List'"
        InsertFunctionList
    end if
    '/// Check that there are 13 categories in the listbox
    printlog " Check that there are 13 categories in the listbox"
    if Category.GetItemCount <> 13 then
        warnlog "The count of categories is " & Category.GetItemCount & " instead of 13"
    end if
    '/// Click on second function
    printlog " Click on second function"
    FunctionList.Select(2)
    call DialogTest ( FunctionsCalc )
    '/// Click on 'Insert' – button to insert the function into the cell
    printlog " Click on 'Insert' – button to insert the function into the cell"
    FunctionInsert.click
    '/// Press 'ESC' to leave insert mode
    printlog " Press 'ESC' to leave insert mode"
    kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<ESCAPE>"
    '/// Close function list by 'Insert – Function list'
    printlog " Close function list by 'Insert – Function list'"
    InsertFunctionList
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertNamesDefine
'///<u><b>Insert - Names – Define</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Define Names' -dialog by 'Insert – Names – Define'
    printlog " Open 'Define Names' -dialog by 'Insert – Names – Define'"
    InsertNamesDefine
    Kontext "NamenFestlegen"
    '/// Click on 'More' – button
    printlog " Click on 'More' – button"
    Zusaetze.Click
    DialogTest ( NamenFestlegen )
    '/// Check that the 'Add' – button is disabled
    printlog " Check that the 'Add' – button is disabled"
    if Hinzufuegen.IsEnabled then
        warnlog "'Add' is enabled but should be disabled"
    end if
    '/// Check that the 'Delete' – button is disabled
    printlog " Check that the 'Delete' – button is disabled"
    if Loeschen.IsEnabled then
        warnlog "'Delete' is enabled but should be disabled"
    end if
    '/// Check that the 'Shrink' – button is disabled
    printlog " Check that the 'Shrink' – button is disabled"
    if ZugeordnetZuButton.IsEnabled then
        warnlog "'Shrink' is enabled but should be disabled"
    end if
    '/// Check that the 'Print Range' option is disabled
    printlog " Check that the 'Print Range' option is disabled"
    if Druckbereich.IsEnabled then
        warnlog "'Printrange' is enabled but should be disabled"
    end if
    '/// Check that the 'Filter' option is disabled
    printlog " Check that the 'Filter' option is disabled"
    if Filter.IsEnabled then
        warnlog "'Filter' is enabled but should be disabled"
    end if
    '/// Check that the 'Repeat Column' option is disabled
    printlog " Check that the 'Repeat Column' option is disabled"
    if Wiederholungsspalte.IsEnabled then
        warnlog "'Repeat Column' is enabled but should be disabled"
    end if
    '/// Check that the 'Repeat Row' option is disabled
    printlog " Check that the 'Repeat Row' option is disabled"
    if Wiederholungszeile.IsEnabled then
        warnlog "'Repeat Row' is enabled but should be disabled"
    end if
    '/// Enter the name 'Test'
    printlog " Enter the name 'Test'"
    BereichsName.SetText "Test"
    '/// Press the 'Shrink' – button
    printlog " Press the 'Shrink' – button"
    ZugeordnetZuButton.click
    '/// Click on 'Maximize' – button
    printlog " Click on 'Maximize' – button"
    ZugeordnetZuButton.click
    '/// Check the 'Print Range' option
    printlog " Check the 'Print Range' option"
    Druckbereich.check
    '/// Check the 'Filter' option
    printlog " Check the 'Filter' option"
    Filter.check
    '/// Check the 'Repeat Column' option
    printlog " Check the 'Repeat Column' option"
    Wiederholungsspalte.check
    '/// Check the 'Repeat Row' option
    printlog " Check the 'Repeat Row' option"
    Wiederholungszeile.check
    '/// Press the 'Add' – button
    printlog " Press the 'Add' – button"
    Hinzufuegen.click
    '/// Choose the 'Test' name
    printlog " Choose the 'Test' name"
    BereichsName.Select "Test"
    '/// Press the 'Delete' – button
    printlog " Press the 'Delete' – button"
    Loeschen.click
    '/// On appearing messagebox click 'Yes'
    printlog " On appearing messagebox click 'Yes'"
    kontext "Active"
     if Active.exists( 2 ) then
    	 Active.Yes
     end if
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    Kontext "NamenFestlegen"
    NamenFestlegen.Cancel
    '/// Close the document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertNamesInsert
'///<u><b>Insert - Names – Define</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Select cell A1
    printlog " Select cell A1"
    call fCalcSelectRange ("A1")
    '/// Open 'Define Names' -dialog by 'Insert – Names – Define'
    printlog " Open 'Define Names' -dialog by 'Insert – Names – Define'"
    InsertNamesDefine
    Kontext "NamenFestlegen"
    '/// Enter the name 'Test'
    printlog " Enter the name 'Test'"
    BereichsName.SetText "Test"
    '/// Press the 'Add' – button
    printlog " Press the 'Add' – button"
    Hinzufuegen.click
    '/// Close dialog with 'OK'
    printlog " Close dialog with 'OK'"
    NamenFestlegen.OK
    '/// Select cell D4
    printlog " Select cell D4"
    call fCalcSelectRange ("D4")
    '/// Open 'Insert Names' – dialog by 'Insert – Names – Insert'
    printlog " Open 'Insert Names' – dialog by 'Insert – Names – Insert'"
    InsertNamesInsert
    Kontext "NamenEinfuegen"
    Call DialogTest ( NamenEinfuegen )
    '/// Select name 'Test'
    printlog " Select name 'Test'"
    NamenAuswahl.Select "Test"
    '/// Press 'OK' – button (the dialog will be closed)
    printlog " Press 'OK' – button (the dialog will be closed)"
    NamenEinfuegen.OK
    '/// Press 'Enter' to leave the editmode of the cell
    printlog " Press 'Enter' to leave the editmode of the cell"
    kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<ESCAPE>"
    '/// Open 'Define Names' -dialog by 'Insert – Names – Insert'
    printlog " Open 'Define Names' -dialog by 'Insert – Names – Insert'"
    InsertNamesInsert
    Kontext "NamenEinfuegen"
    '/// Press 'Insert All' – button (the dialog will be closed)
    printlog " Press 'Insert All' – button (the dialog will be closed)"
    Liste.click
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertNamesCreate
'///<u><b>Insert - Names – Create</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Create Names' – dialog by 'Insert – Names – Create'
    printlog " Open 'Create Names' – dialog by 'Insert – Names – Create'"
    InsertNameCreate
    Kontext "NamenErzeugen"
    DialogTest ( NamenErzeugen )
    '/// Check option 'Top row'
    printlog " Check option 'Top row'"
    Kopfzeile.check
    '/// Check option 'Left column'
    printlog " Check option 'Left column'"
    LinkerSpalte.check
    '/// Check option 'Bottom row'
    printlog " Check option 'Bottom row'"
    Fusszeile.check
    '/// Check option 'Right column'
    printlog " Check option 'Right column'"
    RechterSpalte.check
    '/// Press 'OK' button
    printlog " Press 'OK' button"
    NamenErzeugen.OK
    '/// Close warningbox with 'OK'
    printlog " Close warningbox with 'OK'"
    Kontext
    if active.exists (1) then
        active.ok
    end if
    '/// Enter 'Create Names Test' in cell A1
    printlog " Enter 'Create Names Test' in cell A1"
    call fCalcSelectRange ("A1")
    DocumentCalc.TypeKeys "Create Names Test<RETURN>"
    '/// Select cells A1:A4
    printlog " Select cells A1:A4"
    call fCalcSelectRange ("A1:A4")
    '/// Open 'Create Names' – dialog by 'Insert – Names – Create'
    printlog " Open 'Create Names' – dialog by 'Insert – Names – Create'"
    InsertNameCreate
    Kontext "NamenErzeugen"
    '/// Check that option 'Top row' is selected
    printlog " Check that option 'Top row' is selected"
    if not Kopfzeile.IsChecked then
        warnlog "The header was not autodetected"
    end if
    '/// Press 'OK' button
    printlog " Press 'OK' button"
    NamenErzeugen.OK
    '/// Select cells A1:A3
    printlog " Select cells A1:A3"
    call fCalcSelectRange ("A1:A3")
    '/// Open 'Create Names' – dialog by 'Insert – Names – Create'
    printlog " Open 'Create Names' – dialog by 'Insert – Names – Create'"
    InsertNameCreate
    Kontext "NamenErzeugen"
    '/// Press 'OK' button
    printlog " Press 'OK' button"
    NamenErzeugen.OK
    '/// Close replacing messagebox with 'Cancel'
    printlog " Close replacing messagebox with 'Cancel'"
    Kontext
    if active.exists (1) then
        active.cancel
    end if
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertNamesLabels
'///<u><b>Insert - Names – Labels</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Define Label Range' – dialog by 'Insert – Names – Labels'
    printlog " Open 'Define Label Range' – dialog by 'Insert – Names – Labels'"
    InsertNamesLabels
    Kontext "BeschriftungsbereichFestlegen"
    DialogTest ( BeschriftungsbereichFestlegen )
    '/// Press 'Shrink' button for range
    printlog " Press 'Shrink' button for range"
    ButtonZellbereich.Click
    '/// Press 'Maximize' button on rangechooser
    printlog " Press 'Maximize' button on rangechooser"
    ButtonZellbereich.Click
    '/// Press 'Shrink' button for data range
    printlog " Press 'Shrink' button for data range"
    ButtonDatenbereich.Click
    '/// Press 'Maximize' button on rangechooser
    printlog " Press 'Maximize' button on rangechooser"
    ButtonDatenbereich.Click
    '/// Switch between 'Contains ...' options
    printlog " Switch between 'Contains ...' options"
    Zeilenkoepfe.Check
    Spaltenkoepfe.Check
    '/// Press 'Add' – button
    printlog " Press 'Add' – button"
    Hinzufuegen.Click
    '/// Select range
    printlog " Select range"
    Bereich.Select (2)
    '/// Press 'Delete' – button
    printlog " Press 'Delete' – button"
    Loeschen.Click
    '/// Confirm mesagebox with 'Yes'
    printlog " Confirm mesagebox with 'Yes'"
    Kontext
    if active.exists then
        active.Yes
    end if
    Kontext "BeschriftungsbereichFestlegen"
    '/// Close 'Define Label Range' – dialog with 'Cancel'
    printlog " Close 'Define Label Range' – dialog with 'Cancel'"
    BeschriftungsbereichFestlegen.Cancel
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertNote
'///<u><b>Insert - Note</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Insert a note via 'Insert – Note'
    printlog " Insert a note via 'Insert – Note'"
    InsertNoteCalc
    Kontext
    '/// Enter 'This is a <Return> Testtext' into the note
    printlog " Enter 'This is a <Return> Testtext' into the note"
    DocumentCalc.TypeKeys "This is a <Return> Testtext"
    '/// Leave note with <Escape>
    printlog " Leave note with <Escape>"
    DocumentCalc.TypeKeys "<Escape>"
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertPictureFromFile
'///<u><b>Insert - Picture – From File</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Insert Picture' – dialog by 'Insert – Picture – From File'
    printlog " Open 'Insert Picture' – dialog by 'Insert – Picture – From File'"
    InsertGraphicsFromFile
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
    '/// Close dialog with 'Open'
    printlog " Close dialog with 'Open'"
    Oeffnen.Click
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertMovieSound
'///<u><b>Insert - Movie and Sound</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Insert Movie and Sound' – dialog by 'Insert – Movie and Sound'
    printlog " Open 'Insert Movie and Sound' – dialog by 'Insert – Movie and Sound'"
    InsertMovieAndSound
    Kontext "OeffnenDLG"
    '/// Verify that there are more than one filter available
    printlog " Verify that there are more than one filter available"
    if Dateityp.GetItemCount < 2 then
        warnlog "The filterlist is empty"
    end if
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    OeffnenDLG.Cancel
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertObjectOLE
'///<u><b>Insert - Object – OLE Object</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Insert OLE Object' – dialog  by 'Insert – Object – OLE Object'
    printlog " Open 'Insert OLE Object' – dialog  by 'Insert – Object – OLE Object'"
    InsertObjectOLEObject
    Kontext "OLEObjektEinfuegen"
    '/// Choose 'Create new'
    printlog " Choose 'Create new'"
    NeuErstellen.Check
    Call DialogTest (OLEObjektEinfuegen, 1)
    '/// Check that there are more than 1 entries (amount is dependant from operating system)
    printlog " Check that there are more than 1 entries (amount is dependant from operating system)"
    if Objekttyp.getitemcount < 2 then 
        warnlog "There are no OLE-Objects in the listbox"
    end if
    '/// Choose 'Create from file'
    printlog " Choose 'Create from file'"
    AusDateierstellen.Check
    Call DialogTest (OLEObjektEinfuegen, 2)
    '/// Click on 'Search' – button
    printlog " Click on 'Search' – button"
    Durchsuchen.Click
    Kontext "OeffnenDLG"
    '/// Close 'Open' – dialog with 'Cancel'
    printlog " Close 'Open' – dialog with 'Cancel'"
    OeffnenDLG.Cancel
    Kontext "OLEObjektEinfuegen"
    '/// Check option 'Link to file'
    printlog " Check option 'Link to file'"
    Verknuepfen.check
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    OLEObjektEinfuegen.Cancel
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertObjectPlugin
'///<u><b>Insert - Object – Plug-in</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Insert Plug-in' – dialog by 'Insert – Object – Plug-in'
    printlog " Open 'Insert Plug-in' – dialog by 'Insert – Object – Plug-in'"
    InsertObjectPlugIn
    Kontext "PlugInEinfuegen"
    Call DialogTest (PlugInEinfuegen)
    '/// Click on 'Browse' – button
    printlog " Click on 'Browse' – button"
    DurchSuchen.Click
    Kontext "OeffnenDLG"
    '/// Click on 'Default Directory' – button
    printlog " Click on 'Default Directory' – button"
    Standard.click
    '/// Click on 'Create New Directory' – button
    printlog " Click on 'Create New Directory' – button"
    NeuerOrdner.Click
    Kontext "NeuerOrdner"
    '/// Close namedialog with 'Cancel'
    printlog " Close namedialog with 'Cancel'"
    NeuerOrdner.cancel
    Kontext "OeffnenDLG"
    '/// Close 'Open' – dialog with 'Cancel'
    printlog " Close 'Open' – dialog with 'Cancel'"
    OeffnenDlg.Cancel
    '/// Close 'Insert Plug-in' – dialog with 'Cancel'
    printlog " Close 'Insert Plug-in' – dialog with 'Cancel'"
    Kontext "PluginEinfuegen"
    PlugInEinfuegen.Cancel
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertObjectSound
'///<u><b>Insert - Object – Sound</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    try
        '/// If enabled (depends on additional instalations on the OS) open the 'Insert Sound' – dialog by 'Insert – Object – Sound'
        printlog " If enabled (depends on additional instalations on the OS) open the 'Insert Sound' – dialog by 'Insert – Object – Sound'"
        InsertObjectSound
        Kontext "OeffnenDlg"
        '/// Click on 'Default Directory' – button
        printlog " Click on 'Default Directory' – button"
        Standard.click
        '/// Click on 'Create New Directory' – button
        printlog " Click on 'Create New Directory' – button"
        NeuerOrdner.Click
        Kontext "NeuerOrdner"
        '/// Close namedialog with 'Cancel'
        printlog " Close namedialog with 'Cancel'"
        NeuerOrdner.cancel
        Kontext "OeffnenDLG"
        '/// Close 'Open' – dialog with 'Cancel'
        printlog " Close 'Open' – dialog with 'Cancel'"
        OeffnenDlg.Cancel
    catch
        printlog "Insert - Object - Sound is not possible on this machine"
    endcatch
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertObjectVideo
'///<u><b>Insert - Object – Video</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
try
        '/// If enabled (depends on additional instalations on the OS) open the 'Insert Video' – dialog by 'Insert – Object – Sound'
        printlog " If enabled (depends on additional instalations on the OS) open the 'Insert Video' – dialog by 'Insert – Object – Sound'"
        InsertObjectVideo
        Kontext "OeffnenDlg"
        '/// Click on 'Default Directory' – button
        printlog " Click on 'Default Directory' – button"
        Standard.click
        '/// Click on 'Create New Directory' – button
        printlog " Click on 'Create New Directory' – button"
        NeuerOrdner.Click
        Kontext "NeuerOrdner"
        '/// Close namedialog with 'Cancel'
        printlog " Close namedialog with 'Cancel'"
        NeuerOrdner.cancel
        Kontext "OeffnenDLG"
        '/// Close 'Open' – dialog with 'Cancel'
        printlog " Close 'Open' – dialog with 'Cancel'"
        OeffnenDlg.Cancel
    catch
        printlog "Insert - Object - Video is not possible on this machine"
    endcatch
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertObjectFormular
'///<u><b>Insert - Object – Formular</b></u>

    '/// Open new Spraedsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Insert Formular by 'Insert – Object – Formular'
    printlog " Insert Formular by 'Insert – Object – Formular'"
    InsertObjectFormulaCalc
    Kontext "SelectionMath"
    if SelectionMath.Exists(2) then
        call DialogTest (SelectionMath)
    else
        try
            Kontext "DocumentCalc"
            ViewSelectionCalc
            Kontext "SelectionMath"
            call DialogTest (SelectionMath)
        catch
            qaerrorlog "#i93427# Math Selection Dialog is not available"
        endcatch
    end if
    '/// Close Formular with 'Esc'
    printlog " Close Formular with 'Esc'"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"
    '/// Close document
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertChart
'///<u><b>Insert - Chart</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Chart Wizard' by 'Insert – Chart'
    printlog " Open 'Chart Wizard' by 'Insert – Chart'"
    InsertChart
    Kontext "ChartWizard"
    if ChartWizard.Exists(1) then
        Call Dialogtest(ChartWizard)
        '/// Close 'Chart Wizard' with 'Cancel'
        ChartWizard.Cancel
    else
        warnlog "Chart wizard is not available"
    end if
    '/// Close document 
    printlog " Close starting document"
    call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tInsertFloatingFrame
'///<u><b>Insert - Floating Frame</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"   
    call hNewDocument
    '/// Open 'Floating Frame Properties' – dialog by 'Insert – Floating Frame'
    printlog " Open 'Floating Frame Properties' – dialog by 'Insert – Floating Frame'"
    InsertFloatingFrame
    Kontext "TabEigenschaften"
    Dialogtest (TabEigenschaften)
    '/// Click on 'Browse' – button for 'Contents'
    printlog " Click on 'Browse' – button for 'Contents'"
    Oeffnen.Click
    Kontext "OeffnenDlg"
    '/// Click on 'Default Directory' – button
    printlog " Click on 'Default Directory' – button"
    Standard.click
    '/// Click on 'Create New Directory' – button
    printlog " Click on 'Create New Directory' – button"
    NeuerOrdner.Click
    Kontext "NeuerOrdner"
    '/// Close namedialog with 'Cancel'
    printlog " Close namedialog with 'Cancel'"
    NeuerOrdner.cancel
    Kontext "OeffnenDLG"
    '/// Close 'Select File for Floating Frame' – dialog with 'Cancel'
    printlog " Close 'Select File for Floating Frame' – dialog with 'Cancel'"
    OeffnenDlg.Cancel
    Kontext "TabEigenschaften"
    '/// Check 'Scrollbar' on
    printlog " Check 'Scrollbar' on"
    An.Check
    '/// Check 'Scrollbar' off
    printlog " Check 'Scrollbar' off"
    Aus.Check
    '/// Check 'Scrollbar' automatic
    printlog " Check 'Scrollbar' automatic"
    Automatisch.Check
    '/// Check 'Border Off'
    printlog " Check 'Border Off'"
    UmrandungAus.Check
    '/// Check 'Border On'
    printlog " Check 'Border On'"
    UmrandungAn.Check
    '/// Uncheck 'Default'
    printlog " Uncheck 'Default'"
    BreiteStandard.UnCheck
    '/// Increase 'Width' by 1
    printlog " Increase 'Width' by 1"
    Breite.More
    '/// Increase 'Height' by 1
    printlog " Increase 'Height' by 1"
    Hoehe.More
    '/// Check 'Default'
    printlog " Check 'Default'"
    HoeheStandard.Check
    '/// Close 'Floating Frame Properties' – dialog  with 'Cancel'
    printlog " Close 'Floating Frame Properties' – dialog  with 'Cancel'"
    TabEigenschaften.Cancel
    '/// Close document 
    printlog " Close starting document"
    call hCloseDocument

endcase


