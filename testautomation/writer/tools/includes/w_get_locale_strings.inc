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
'* short description : testcase to get Locale Strings used in Writer Optional-Test
'*
'\***********************************************************************

sub w_get_locale_strings

    Call wAllFieldNames
    Call wUsedInTableTest
    Call wUsedInLoadSaveTests

end sub

'------------------------------------------------------------------------------------------------

testcase wAllFieldNames
    Dim i as integer
    Dim j as integer

    printlog " get all field descriptions (used in w_tools.inc)"
    Call hNewDocument

    InsertFieldsOther
    for i = 1 to 5
        Kontext
        Select Case i
        Case 1
            printlog "Tabpage Document"
            Active.Setpage TabDokumentFeldbefehle
            Kontext "TabDokumentFeldbefehle"
        Case 2
            printlog "Tabpage References"
            Active.Setpage TabReferenzen
            Kontext "TabReferenzen"
        Case 3
            printlog "Tabpage Functions"
            Active.Setpage TabFunktionen
            Kontext "TabFunktionen"
        Case 4
            printlog "Tabpage DocInformation"
            Active.Setpage TabDokumentInfoFeldbefehle
            Kontext "TabDokumentInfoFeldbefehle"
        Case 5
            printlog "Tabpage Variables"
            Active.Setpage TabVariablen
            Kontext "TabVariablen"
        end select

        if i = 4 then
            for j = 1 to Feldtyp.GetItemCount
                Feldtyp.Select j
                Feldtyp.TypeKeys "+"
            next j
        end if

        for j = 1 to Feldtyp.GetItemCount
            Feldtyp.Select j
            printlog "- " & Feldtyp.GetSelText
        next j
    next i
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------

testcase wUsedInTableTest

    Dim pos as integer
    Dim wTableName as string

    printlog " gets all strings which are used in '.\\optional\\w_table.bas'"
    printlog " add these entries into file : .\\optional\input\\table\\locale.txt"
    Call hNewDocument

    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"
    TabelleEinfuegenWriter.Ok
    Sleep 1
    Kontext "TableObjectbar"
    printlog " Table Heading: " & Vorlage2.GetSelText
    Kontext "DocumentWriter"
    Call wTypeKeys "<Down>"
    Sleep 1
    Kontext "TableObjectbar"
    printlog " Table Contents: " & Vorlage2.GetSelText
    Kontext "DocumentWriter"
    wait 500
    FormatNumberFormat
    Kontext "Zahlenformat"
    Kategorie.Select 3
    printlog "Number: " & Kategorie.GetSelText
    Kategorie.Select 11
    printlog "Text: " & Kategorie.GetSelText
    Kategorie.Select 5
    printlog "Currency: " & Kategorie.GetSelText
    Kategorie.Select 6
    printlog "Date: " & Kategorie.GetSelText
    Kategorie.Select 7
    printlog "Time: " & Kategorie.GetSelText
    Kategorie.Select 4
    printlog "Percent: " & Kategorie.GetSelText
    Kategorie.Select 8
    printlog "Scientific: " & Kategorie.GetSelText
    Kategorie.Select 9
    printlog "Fraction: " & Kategorie.GetSelText
    Kategorie.Select 10
    printlog "Boolean Value: " & Kategorie.GetSelText
    Zahlenformat.Cancel
    InsertTableWriter
    Wait 500
    Kontext "TabelleEinfuegenWriter"
    wTableName = TabellenName.Gettext
    pos = Instr(wTableName, "2")
    if pos > 0 then
        wTableName = Left$(wTableName, pos -1)
    end if
    printlog "Table: " & wTableName
    TabelleEinfuegenWriter.Cancel

    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------

testcase wUsedInLoadSaveTests
    Dim i as integer

    Call hNewDocument
    printlog "- All filters from open dialog:"
    FileOpen
    Kontext "OeffnenDlg"
    For i = 1 to Dateityp.GetItemCount
        Dateityp.Select i
        printlog " - " & Dateityp.GetSelText
    next i

    OeffnenDlg.Cancel

    printlog "- All filters from save dialog:"
    FileSaveAs
    Kontext "SpeichernDlg"
    For i = 1 to Dateityp.GetItemCount
        Dateityp.Select i
        printlog " - " & Dateityp.GetSelText
    next i

    SpeichernDlg.Cancel

    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------
