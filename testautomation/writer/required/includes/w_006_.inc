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
'* short description : Call all functions in table menu
'*
'\***********************************************************************

sub w_006_

    printLog Chr(13) + "--------- Table Menu (w_006_.inc) ----------"
    gApplication = "WRITER"

    Call tFormatTable
    Call tFormatSplitTables
    Call tFormatJoinTables
    Call tFormatNumberFormat
    Call tTableCell
    Call tTableRow
    Call tFormatAutoformatTable

end sub

'---------------------------------------------------------------------------------------------------------

testcase tFormatTable
    PrintLog "- Format / Table"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert a table"
    Call hTabelleEinfuegen

    printlog " Format / Table"
    TableProperties

    Kontext
    printlog " In dialog step through all tabpages"
    Active.SetPage TabTabelle
    Kontext "TabTabelle"
    Call DialogTest ( TabTabelle )

    Kontext
    Active.SetPage TabTextflussTabelle
    Kontext "TabTextflussTabelle"
    Call DialogTest ( TabTextflussTabelle )

    Kontext
    Active.SetPage TabSpaltenTabelle
    Kontext "TabSpaltenTabelle"
    Call DialogTest ( TabSpaltenTabelle )

    Kontext
    Active.SetPage TabUmrandung
    Kontext "TabUmrandung"
    Call DialogTest ( TabUmrandung )

    Kontext
    Active.SetPage TabHintergrund
    Kontext "TabHintergrund"
    Call DialogTest ( TabHintergrund )

    printlog " Close dialog"
    TabHintergrund.Cancel

    printlog " Close active document"
    Call hCloseDocument

endcase

'----------------------------------------------------------

testcase tFormatSplitTables
    PrintLog "- Format / Split Table"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a table"
    Call hTabelleEinfuegen
    printlog " Select all"
    Call wTypeKeys ("<Mod1 a>")
    Sleep 1
    printlog " Format / Split Table"
    FormatSplitTable
    Kontext "TabelleAuftrennen"
    Call DialogTest ( TabelleAuftrennen )
    printlog " Close dialog"
    TabelleAuftrennen.Cancel
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatJoinTables
    PrintLog "- Format / Join Table"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a table"
    Call hTabelleEinfuegen
    printlog " Point cursor under the table"
    Call wTypeKeys ("<Down><Down>")
    printlog " Insert a table"
    Call hTabelleEinfuegen
    printlog " Point cursor under the table"
    Call wTypeKeys ("<Down><Down>")
    printlog " Insert a table"
    Call hTabelleEinfuegen
    printlog " Point cursor in 3rd table"
    Call wTypeKeys ("<Up><Up>")
    printlog " Format / Merge Tables"
    FormatMergeTables
    Kontext "TabellenVerbinden"
    Call DialogTest (TabellenVerbinden)
    printlog " Close dialog"
    TabellenVerbinden.Cancel
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatNumberFormat
    printLog "- Format / Number Format"
    printlog " Open new document"
    Call hNewDocument
    printlog " Format / Number Format"
    Call hTabelleEinfuegen
    FormatNumberFormat
    Kontext "Zahlenformat"
    Call DialogTest ( Zahlenformat )
    printlog " Close dialog"
    Zahlenformat.Cancel
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tTableCell
    PrintLog "- Format / Cell"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a table"
    Call hTabelleEinfuegen

    printlog " Table / Column / Select"
    TableSelectColumn
    printlog " Table / Merge Cells"
    TableMergeCell
    printlog " Table / Split Cells"
    TableSplitCell

    Kontext "ZellenTeilen"
    Call DialogTest ( ZellenTeilen )
    printlog " Close dialog 'Split Table'"
    ZellenTeilen.OK
    printlog " Table / Protect Cells"
    TableCellProtect

    Call wTypeKeys ("<Left>")
    printlog " Open Contextmenu"
    printlog " Select 'Cell->Unprotect"
    Kontext "DocumentWriter"
    Call wTypeKeys "<Shift F10>"
    wait (500)
    Call hMenuFindSelect (".uno:CellMenu", true, 9, false)   'Find "CellMenu" and call the slot.
    wait (500)
    try
        Call hMenuFindSelect (20519, true, 1, false)   'Find "UnProtect" and call the slot.
    catch
        Warnlog "Unable to unprotect cell"
    endcatch
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tTableRow
    PrintLog "- Format / Row"
    printlog " Open new document"
    Call hNewDocument
    printlog "  Insert a table"
    Call hTabelleEinfuegen

    printlog " Format / Row / Height"
    FormatRowHeight
    Kontext "ZellenHoehe"
    Hoehe.SetText "2,5"
    Call DialogTest ( ZellenHoehe )
    printlog " Close dialog"
    ZellenHoehe.OK
    printlog " Format / Column / Select"
    TableSelectColumn
    WaitSlot (500)
    printlog " Format / Row / Space equally"
    FormatRowSpaceEqually
    WaitSlot (500)
    printlog " Format / Row / Select"
    TableSelectEntireRow
    WaitSlot (500)
    printlog " Format / Row / Optimal Height"
    TableAutoFitSetOptimalRowHeight
    WaitSlot (500)
    printlog " Format / Row / Insert"
    FormatRowInsert
    Kontext "EinfuegenZeilen"
    Anzahl.SetText "3"
    Call DialogTest ( EinfuegenZeilen )
    printlog " Close dialog"
    EinfuegenZeilen.OK
    printlog " Format / Row / Delete"
    FormatRowDelete

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatAutoformatTable
    PrintLog "- Format / Autoformat - Table"

    Call hNewDocument
    printlog " Open new document"
    Sleep 1
    printlog " Insert a table"
    Call hTabelleEinfuegen

    printlog " Format / Autoformat"
    FormatAutoformat
    WaitSlot (2000)

    Kontext "AutoformatTabelle"
    printlog " In dialog click 'More..'"
    Zusaetze.Click
    Call DialogTest ( AutoformatTabelle )

    printlog " Click 'Add'"
    Einfuegen.Click
    Kontext "AutoformatHinzufuegenWriter"
    Call DialogTest ( AutoformatHinzufuegenWriter )
    TabellenName.SetText "Hallo"
    printlog " Enter a new table name"
    AutoformatHinzufuegenWriter.OK

    Kontext "AutoformatTabelle"
    printlog " Click 'Rename'"
    Umbenennen.Click
    Kontext "AutoformatHinzufuegenWriter"
    Call DialogTest ( AutoformatHinzufuegenWriter )
    printlog " Enter an new table name"
    TabellenName.SetText "Hallo1"
    AutoformatHinzufuegenWriter.OK
    Kontext "AutoformatTabelle"
    printlog " Click 'Delete'"
    Loeschen.Click
    Kontext
    Active.OK
    printlog " Close dialog"
    AutoformatTabelle.Cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

