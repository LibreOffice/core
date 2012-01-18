'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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
