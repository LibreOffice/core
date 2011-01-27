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
'* short description : Writer options test
'*
'************************************************************************
'*
' #1 tToolsOptionsWriterPrint
' #1 tToolsOptionsWriterTableMove
' #1 tToolsOptionsWriterTableInsert
' #1 tToolsOptionsWriterTableEffects
' #1 tToolsOptionsWriterTableEffects2
' #1 tToolsOptionsWriterDirectCursor2
'*
'\***********************************************************************

testcase tToolsOptionsWriterPrint
    ' called after my save/restart testing
    ' The real print with paper to touch - and to get from the printer :-(

    Dim i as integer
    printlog "- Tools/Options/Writer: PrintWriter (real print)"
    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\options\options3.sxw" )
    

    Printlog "  - check all (real print)"
    ToolsOptions
    Call hToolsOptions("WRITER","PRINT")

    Grafiken.Check
    Kontrollfelder.Check
    Hintergrund.Check
    SchwarzDrucken.Check
    LinkeSeiten.Check
    RechteSeiten.Check
    Prospekt.UnCheck

    Seitenende.Check
    AusDruckereinstellung.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    if (NO_REAL_PRINT = False) Then
        Printlog "   - print 1"
        kontext "funktionsleiste"
        Drucken
        for i=1 to 10
            Kontext "Druckbestaetigung"
            if Druckbestaetigung.Exists then
                sleep 1
            else
                i=100
            end if
        next i
        Sleep 3
    end if

    ToolsOptions
    Call hToolsOptions("WRITER","PRINT")

    Printlog "  - uncheck everything"

    Grafiken.UnCheck
    Kontrollfelder.UnCheck
    Hintergrund.UnCheck
    SchwarzDrucken.UnCheck
    LinkeSeiten.Check               ' that's so, otherwise there will be no printing
    RechteSeiten.Check              ' that's so, otherwise there will be no printing
    Prospekt.UnCheck
    Keine.Check
    AusDruckereinstellung.UnCheck

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    Sleep 10

    if (NO_REAL_PRINT = False) Then
        Printlog "   - print 2"
        kontext "funktionsleiste"
        Drucken
        for i=1 to 10
            Kontext "Druckbestaetigung"
            if Druckbestaetigung.Exists then
                sleep 1
            else
                i=100
            end if
        next i
        Sleep 3
    end if

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
    Call wOptionsUndo ( "Drucken" )
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterTableMove
    Dim sLager$
    Dim Spalte1 as Double
    Dim Spalte2 as Double
    Dim Zeile1 as Double
    Dim Zeile2 as Double
    Dim Abstaende(3) as Double
    Dim AbLi as Double
    Dim AbRe as Double
    Dim VerschubZeile as Double
    Dim VerschubSpalte as Double
    Dim sSeparator as string
    Dim sRow1 as string

    ' Hints if there are some errors:
    ' win2000: Start::Systemsteuerung::Laendereinstellungen::Gebietsschema
    ' winXP  : Sart::ControlPanel::Regional and Language Options:: Region Options:: Standards&formats

    '/// New writerdocument
    Call hNewDocument
    ToolsOptions     ' Measurement unit should be set to cm
    Call hToolsOptions("WRITER","GENERAL")
    '/// Set measurement unit to cm in options
    Masseinheit.Select 2
    sSeparator = GetDecimalSeperator ( Tabulatorenabstand.GetText )
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    printlog "'///- Tools / Options / Table: Move"
    '/// Tools / Options / Table
    ToolsOptions
    Call hToolsOptions("WRITER","TABLE")

    Printlog "'///  - check minimum settings"
    '/// check minimum settings
    if gPlatform="solaris" then
        sLager$ = sSeparator + "25" + mUnit
    else
        sLager$ = "0" + sSeparator + "25" + mUnit
    end if
    VerschiebenZeile.ToMin
    if sLager$ <> VerschiebenZeile.GetText then Warnlog "Move cells: Row isn't min ( " + sLager$ + " ) but " +VerschiebenZeile.GetText
    VerschiebenSpalte.ToMin
    if sLager$ <> VerschiebenSpalte.GetText then Warnlog "Move cells: Column isn't min ( " + sLager$ + " ) but " +VerschiebenZeile.GetText

    Printlog "- make settings"
    '/// make the following settings
    '/// set move cells: row to 1.00
    '/// set move cells: column to 1.25
    VerschiebenZeile.SetText "1" + sSeparator + "00"
    VerschiebenSpalte.SetText "1" + sSeparator + "25"
    VerschubZeile = 1.00
    VerschubSpalte = 1.25

    '/// check radiobutton 'Variable'
    Variabel.Check
    Sleep 1
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Printlog "- Insert table"
    '/// Insert a table
    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"
    '///  set rows an columns to 2
    Spalten.SetText "2"
    Zeilen.SetText "2"
    TabelleEinfuegenWriter.OK

    Printlog "remember column width of inserted table"
    Call wTypeKeys "<Left><Mod2 Down>"
    Call wTypeKeys "<Left><Mod2 Up>"

    ' **   Grundwerte der Tabelle holen ( alles schon in Double und rechenbar ) since 05.02.2001 ;-)
    Spalte1 = SpaltenBreiteHolen

    FormatRowHeight
    Kontext "Zellenhoehe"
    sRow1 = Hoehe.GetText
    Zellenhoehe.OK

    Zeile1  = ZeilenHoeheHolen
    Call wTypeKeys "<Right>"
    Spalte2 = SpaltenBreiteHolen
    Call wTypeKeys "<Down>"
    Zeile2  = Zeile1
    Call wTypeKeys "<Up><Left>"
    SeitenAbstaendeHolen ( Abstaende() )
    AbLi = Abstaende(1)
    AbRe = Abstaende(2)

    Printlog "Startwerte"
    Printlog "Width   :   Column 1: " & Spalte1 & " Column 2: " & Spalte2
    Printlog "Height  :   Row 1: " & Zeile1 & " Row 2: " & Zeile2 & Chr(13)

    Printlog "- with <Alt Left> downsize 1. column from right side"
    Call wTypeKeys "<Mod2 Left>"
    Call SpaltenBreiteTesten ( Spalte1-VerschubZeile )
    Call SeitenAbstaendeTesten ( 0.00, VerschubZeile )

    Printlog "- with <Shift Alt Right> downsize 1. column from left side"
    Call wTypeKeys "<Shift Mod2 Right>"
    Call SpaltenBreiteTesten ( Spalte1 - VerschubZeile *2 )
    Call SeitenAbstaendeTesten ( VerschubZeile, VerschubZeile )

    Printlog "- with <Alt Right> upsize 1. column to the right 2 times"
    Call wTypeKeys "<Mod2 Right>"
    Call wTypeKeys "<Mod2 Right>"
    Call SpaltenBreiteTesten ( Spalte1 )
    Call SeitenAbstaendeTesten ( VerschubZeile, 0 )

    Printlog "- check size of 2. column"
    Call wTypeKeys "<Right>"
    Call SpaltenBreiteTesten ( Spalte2 - VerschubZeile )
    Call SeitenAbstaendeTesten ( VerschubZeile, 0 )

    Printlog "- with <Alt Left> upsize 2. column from left"
    Call wTypeKeys "<Mod2 Left>"
    wait 100
    Call SpaltenBreiteTesten ( Spalte2 - VerschubZeile*2 )
    Call SeitenAbstaendeTesten ( VerschubZeile, VerschubZeile )

    Printlog "- with <Shift Alt Right> downsize 2. column from left"
    Call wTypeKeys "<Shift Mod2 Right>"
    Call SpaltenBreiteTesten ( Spalte2 - VerschubZeile*3 )
    Call SeitenAbstaendeTesten ( VerschubZeile*2, VerschubZeile )

    Printlog "- with <Shift Alt Left> upsize 2. column to the left 2 times"
    Call wTypeKeys "<Shift Mod2 Left>"
    Call wTypeKeys "<Shift Mod2 Left>"
    Call SpaltenBreiteTesten ( Spalte2-VerschubZeile )
    Call SeitenAbstaendeTesten ( 0, VerschubZeile )

    Printlog "- with <Shift Alt Left> upsize 2. column to the right"
    Call wTypeKeys "<Shift Mod2 Left>"
    Call SpaltenBreiteTesten ( Spalte2 )
    Call SeitenAbstaendeTesten ( 0, 0 )

    Printlog "- with <Alt Down> upsize 1. column down"
    Call wTypeKeys "<Left><Mod2 Down>"
    Call ZeilenHoeheTesten ( Zeile1+VerschubSpalte )
    Call SeitenAbstaendeTesten ( 0, 0 )

    Printlog "- with <Alt Up> downsize 1. column up"
    Call wTypeKeys "<Mod2 Up>"
    Call ZeilenHoeheTesten ( ( Zeile1 ) )
    Call SeitenAbstaendeTesten ( 0, 0 )

    FormatRowHeight
    Kontext "Zellenhoehe"
    if (StrToDouble ( Hoehe.Gettext ) <> Zeile1 ) then
        Hoehe.More
        printlog "  from now on, there is an offset to the original of 10mm"
        Zeile2 = Zeile2 + ( StrToDouble ( Hoehe.Gettext ) - Zeile2 )
    end if
    Zellenhoehe.OK

    Printlog "- with <Alt Down> upsize 2. column down"
    Call wTypeKeys "<Right><Mod2 Down>"
    Call ZeilenHoeheTesten ( Zeile2+VerschubSpalte*2/2 )
    Call SeitenAbstaendeTesten ( 0, 0 )

    Printlog "- with <Alt Up> downsize 2. column up"
    Call wTypeKeys "<Mod2 Up>"
    Call ZeilenHoeheTesten ( Zeile2 )
    Call SeitenAbstaendeTesten ( 0, 0 )

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
    Call wOptionsUndo "Tabelle"
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterTableInsert
    Dim sLager$
    Dim Spalte1 as Double
    Dim Spalte2 as Double
    Dim Zeile as Double
    Dim VerschubZeile as Double
    Dim VerschubSpalte as Double
    Dim sSeparator as string

    Call hNewDocument

    printlog "- Tools / Options / Table"
    ToolsOptions
    Call hToolsOptions("WRITER","TABLE")

    Printlog "- make settings"
    sSeparator = GetDecimalSeperator(EinfuegenZeile.GetText)
    EinfuegenZeile.SetText "0" + sSeparator + "73"
    VerschubZeile = 0.73
    EinfuegenSpalte.SetText "3" + sSeparator + "15"
    VerschubSpalte = 3.15
    Variabel.Check
    Sleep 1
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Printlog "- Insert a table"
    Call hNewDocument
    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"
    Spalten.SetText "2"
    Zeilen.SetText "2"
    TabelleEinfuegenWriter.OK

    ' **   Grundwerte der Tabelle holen ( alles schon in Double und rechenbar )
    Spalte1 = SpaltenBreiteHolen
    Zeile  = ZeilenHoeheHolen
    Call wTypeKeys "<Right>"
    Spalte2 = SpaltenBreiteHolen
    ' **

    Printlog "- with <Alt Insert><Up> insert a row from top"
    Call wTypeKeys "<Mod2 Insert><Up>"
    Call ZeilenHoeheTesten ( VerschubZeile )
    Call SeitenAbstaendeTesten ( 0.00, 0.00 )
    Call ZeilenTesten ( 3 )
    Call SpaltenTesten ( 2 )

    Printlog "- with <Alt Insert><Down> insert a row from downside"
    Call wTypeKeys "<Down><Mod2 Insert><Down>"
    Call ZeilenHoeheTesten ( VerschubZeile )
    Call ZeilenTesten ( 4 )
    Call SpaltenTesten ( 2 )

    Printlog "- with <Alt Delete><Up> delete 1. row from top"
    Call wTypeKeys "<Mod2 Delete><Up>"
    Call ZeilenHoeheTesten ( 0.01 )
    Call ZeilenTesten ( 3 )
    Call SpaltenTesten ( 2 )

    Printlog "- with <Alt Delete><Down> delete 1. row from the button"
    Call wTypeKeys "<Mod2 Delete><Down>"
    Call ZeilenHoeheTesten ( VerschubZeile )
    Call ZeilenTesten ( 2 )
    Call SpaltenTesten ( 2 )

    Printlog "- with <Alt Insert><Left> insert a column from left side"
    Call wTypeKeys "<Mod2 Insert><Left>"
    Call SpaltenBreiteTesten ( VerschubSpalte )
    Call SeitenAbstaendeTesten ( 0.00, 0.00 )
    Call ZeilenTesten ( 2 )
    Call SpaltenTesten ( 3 )

    Printlog "- with <Alt Insert><Right> insert a rown from right side"
    Call wTypeKeys "<Right><Right><Mod2 Insert><Right>"
    Call SpaltenBreiteTesten ( VerschubSpalte )
    Call ZeilenTesten ( 2 )
    Call SpaltenTesten ( 4 )

    Printlog "- with <Alt Delete><Left> delete 1. Column from left side"
    Call wTypeKeys "<Mod2 Delete><Left>"
    DocumentWriter.TypeKeys "<Up>"
    Call SpaltenBreiteTesten ( Spalte1-VerschubSpalte )
    Call ZeilenTesten ( 2 )
    Call SpaltenTesten ( 3 )
    Call wTypeKeys "<Right>"
    Call SeitenAbstaendeTesten ( VerschubSpalte, 0.00 )

    Printlog "- with <Alt Delete><Right> delete last column from right side"
    Call wTypeKeys "<Right><Right><Mod2 Delete><Right>"
    Call SpaltenBreiteTesten ( Spalte2-VerschubSpalte )
    Call ZeilenTesten ( 2 )
    Call SpaltenTesten ( 2 )
    Call SeitenAbstaendeTesten ( VerschubSpalte, Spalte2 -VerschubSpalte )

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
    Call wOptionsUndo "Tabelle"
endcase

' ************************************************************************

testcase tToolsOptionsWriterTableEffects
    Dim Spalte1 as Double
    Dim Spalte2 as Double
    Dim Spalte3 as Double
    Dim VerschubZeile as Double

    Call hNewDocument

    printlog "- Tools / Options / Table"
    ToolsOptions
    Call hToolsOptions("WRITER","TABLE")
    VerschubZeile = ZahlAusSpinnfield ( VerschiebenZeile.GetText )
    VerhaltenFix.Check
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog "- Fixed: changes affect the adjacent area only"
    Call hNewDocument
    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"
    Spalten.SetText "3"
    Zeilen.SetText "2"
    TabelleEinfuegenWriter.OK

    Spalte1 = SpaltenBreiteHolen
    Call wTypeKeys "<Right>"
    Spalte2 = SpaltenBreiteHolen
    Call wTypeKeys "<Right>"
    Spalte3 = SpaltenBreiteHolen
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Left>", 3

    Printlog "- with <Alt Left> downsize 1. Column from right side"
    Call wTypeKeys "<Mod2 Left>"
    Call SpaltenBreiteTesten ( Spalte1-VerschubZeile )
    Call wTypeKeys "<Right>"
    Call SpaltenBreiteTesten ( Spalte2+VerschubZeile )
    Call wTypeKeys "<Right>"
    Call SpaltenBreiteTesten ( Spalte3 )
    Call SeitenAbstaendeTesten ( 0.00, 0.00 )

    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Left>", 3
    Printlog "- with <Alt Shift Right> no changes should be happen"
    Call wTypeKeys "<Mod2 Shift Left>"
    Call SpaltenBreiteTesten ( Spalte1-VerschubZeile )
    Call wTypeKeys "<Right>"
    Call SpaltenBreiteTesten ( Spalte2+VerschubZeile )
    Call wTypeKeys "<Right>"
    Call SpaltenBreiteTesten ( Spalte3 )
    Call SeitenAbstaendeTesten ( 0.00, 0.00 )

    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Left>", 3
    Printlog "- with <Alt Right> re-establish initial state ///"
    Call wTypeKeys "<Mod2 Right>"
    Call SpaltenBreiteTesten ( Spalte1 )
    Call wTypeKeys "<Right>"
    SpaltenBreiteTesten ( Spalte2 )
    Call wTypeKeys "<Right>"
    SpaltenBreiteTesten ( Spalte3 )
    SeitenAbstaendeTesten ( 0.00, 0.00 )

    Printlog "- with <Alt Down> row will be upsized ///"
    Call wTypeKeys "<Mod2 Down>"
    Call ZeilenHoeheTesten ( 1.17 )
    Call SeitenAbstaendeTesten ( 0.00, 0.00 )

    if iSprache <> 01 then
        Printlog "- with <Alt Up> row gets initial state"
        Call wTypeKeys "<Mod2 Up>"
        Call ZeilenHoeheTesten ( 0.7 )
        Call SeitenAbstaendeTesten ( 0.00, 0.00 )

        Printlog "- with <Alt Shift Up> row will be upsized"
        Call wTypeKeys "<Mod2 Shift Up>"
        Call ZeilenHoeheTesten ( 1.17 )
        Call SeitenAbstaendeTesten ( 0.00, 0.00 )

        Printlog "- with <Alt Shift Down> column gets initial state"
        Call wTypeKeys "<Mod2 Shift Down>"
        Call ZeilenHoeheTesten ( 0.7 )
        Call SeitenAbstaendeTesten ( 0.00, 0.00 )
    end if
    Call hCloseDocument
    Call hNewDocument
    
    printlog "- Fixed, proportional: Changes affect the entire table"
    ToolsOptions
    Call hToolsOptions("WRITER","TABLE"
    FixProportional.Check
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"
    Spalten.SetText "3"
    Zeilen.SetText "2"
    TabelleEinfuegenWriter.OK

    Printlog "- with <Alt Left> downsize 1. column from right"
    Call wTypeKeys "<Mod2 Left>"
    Call SpaltenBreiteTesten ( Spalte1-VerschubZeile )
    Call wTypeKeys "<Right>"
    Call SpaltenBreiteTesten ( Spalte2+VerschubZeile/2 )
    Call wTypeKeys "<Right>"
    Call SpaltenBreiteTesten ( Spalte3+VerschubZeile/2 )
    Call SeitenAbstaendeTesten ( 0.00, 0.00 )

    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Left>", 3
    Printlog "- with <Alt Shift Right> nothing should happen"
    Call wTypeKeys "<Mod2 Shift Left>"
    Call SpaltenBreiteTesten ( Spalte1-VerschubZeile )
    Call wTypeKeys "<Right>"
    Call SpaltenBreiteTesten ( Spalte2+VerschubZeile/2 )
    Call wTypeKeys "<Right>"
    Call SpaltenBreiteTesten ( Spalte3+VerschubZeile/2 )
    Call SeitenAbstaendeTesten ( 0.00, 0.00 )

    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Left>", 3
    Printlog "- with <Alt Right> reach initial state"
    Call wTypeKeys "<Mod2 Right>"
    Call SpaltenBreiteTesten ( Spalte1 )
    Call wTypeKeys "<Right>"
    Call SpaltenBreiteTesten ( Spalte2 )
    Call wTypeKeys "<Right>"
    Call SpaltenBreiteTesten ( Spalte3 )
    Call SeitenAbstaendeTesten ( 0.00, 0.00 )

    if iSprache <> 01 then
        Printlog "- with <Alt Down> row will be upsized"
        Call wTypeKeys "<Mod2 Down>"
        Call ZeilenHoeheTesten ( 1.2 )
        Call SeitenAbstaendeTesten ( 0.00, 0.00 )

        Printlog "- with <Alt Up> column gets intitial state"
        Call wTypeKeys "<Mod2 Up>"
        Call ZeilenHoeheTesten ( 0.7 )
        Call SeitenAbstaendeTesten ( 0.00, 0.00 )

        Printlog "- with <Alt Shift Up> column will be upsized"
        Call wTypeKeys "<Mod2 Shift Up>"
        Call ZeilenHoeheTesten ( 1.2 )
        Call SeitenAbstaendeTesten ( 0.00, 0.00 )

        Printlog "- with <Alt Shift Down> column gets initial state"
        Call wTypeKeys "<Mod2 Shift Down>"
        Call ZeilenHoeheTesten ( 0.7 )
        Call SeitenAbstaendeTesten ( 0.00, 0.00 )
    end if
    Call wOptionsUndo "Tabelle"

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterTableEffects2
    Dim Spalte1 as Double
    Dim vSeparator as string
    Dim VerschubZeile as Double

    vSeparator = GetDecimalSeperator

    printlog "- Tools / Options / Table"
    printlog "- Effect options should work immediately"

    Call hNewDocument

    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"
    Spalten.SetText "3"
    Zeilen.SetText "2"
    TabelleEinfuegenWriter.OK

    Spalte1 = SpaltenBreiteHolen

    ToolsOptions
    Call hToolsOptions("WRITER","TABLE")
    VerschubZeile = ZahlAusSpinnfield ( VerschiebenZeile.GetText )
    if NOT Variabel.IsChecked then
        Warnlog "Variable is not checked so test will be interrupted here"
        goto endsub
    else
        printlog "- change 'Behaviour of columns/rows to 'Fix'"
        VerhaltenFix.Check
    end if
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog "- resize cell with <Alt Left>"
    Call wTypeKeys "<Mod2 Left>"
    Call SpaltenBreiteTesten ( ( Spalte1-VerschubZeile ) )

    FormatTable
    Kontext
    Active.SetPage TabTabelle
    Kontext "TabTabelle"
    if ZahlAusSpinnfield ( NachRechts.GetText ) <> "0" then
        Warnlog "Table size has been changed though 'Behaviour of rows/columns has been set to 'Fix'"
    end if
    TabTabelle.Cancel

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
    Call wOptionsUndo "Tabelle"
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterDirectCursor2
    printlog "- Tools / Options / Direct Cursor / Left paragraph margin"

    Call hNewDocument
    ViewZoom
    Kontext "Massstab"
    Optimal.Check
    Massstab.OK

    Printlog "- set left paragraph margin"
    ToolsOptions
    Call hToolsOptions("WRITER","FORMATTINGAIDS")
    DirektCursor.Check
    LinkerAbsatzrand.Check
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    Printlog "- set cursor to the right"
    Kontext "DocumentWriter"
    DocumentWriter.MouseDoubleClick 80, 60
    DocumentWriter.Mousemove 20, 50
    DocumentWriter.TypeKeys "Right"
    FormatParagraph
    Kontext
    Active.SetPage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"
    if NOT Links.IsChecked then Warnlog "Cursor set to right but it isn't left aligned"
    TabAusrichtungAbsatz.Cancel
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Shift Home>"
    Sleep 1
    EditCopy
    if GetClipboardText = "" then Warnlog "Maybe tabs or spaces inserted"

    ViewZoom
    Kontext "Massstab"
    Vergroesserung100.Check
    Massstab.OK

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
    Call wOptionsUndo "Cursor"
endcase

'------------------------------------------------------------------------------

