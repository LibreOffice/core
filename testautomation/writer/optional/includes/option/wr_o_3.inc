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
'* short description : Writer options tests
'*
'\***********************************************************************
'*
'*  tToolsOptionsWriterCursor
'*  tToolsOptionsWriterChanges
'*  tToolsOptionsWriterInsert
'*  tToolsOptionsAutocaption
'*  tToolsOptionsWriterCompatibility1
'*  tToolsOptionsWriterCompatibility2
'*  tToolsOptionsWriterCompatibility3
'*
'\***********************************************************************

sub wr_o_3

    Call tToolsOptionsWriterCursor
    Call tToolsOptionsWriterChanges
    Call tToolsOptionsWriterInsert
    Call tToolsOptionsAutocaption
    Call tToolsOptionsWriterCompatibility1
    Call tToolsOptionsWriterCompatibility2
    Call tToolsOptionsWriterCompatibility3

end sub

'------------------------------------------------------------------------------
testcase tToolsOptionsWriterCursor

    Dim irgendwas(17) as boolean        ' Checkbox states
    Dim bRadioBut(4) as boolean         ' RadioButtons
    Dim bTempRadioBut(4) as boolean
    Dim iListBox(1) as integer          ' ListBox
    Dim iTempListBox(1) as integer

    Call hNewDocument
    printlog "'///- Tools/Options/Writer: View 2 ///"

    printlog ("'/// - save states ///")
    ToolsOptions
    Call hToolsOptions ("WRITER","FORMATTINGAIDS")

    irgendwas(1) = DirektCursor.IsChecked
    irgendwas(2) = Zonen_Cursor.IsChecked

    bRadioBut(1) = Absatzausrichtung.IsChecked
    bRadioBut(2) = LinkerAbsatzrand.IsChecked
    bRadioBut(3) = Tabulator.IsChecked
    bRadioBut(4) = TabulatorUndLeerzeichen.IsChecked

    if ((bRadioBut(1) Xor bRadioBut(2) Xor bRadioBut(3) Xor bRadioBut(4)) = False) then Warnlog (" RadioButtons have NO init state: " + bRadioBut(1) +", BugID: Not Yet l.f.?")

    irgendwas(10) = Absatzenden.IsChecked
    irgendwas(11) = WeicheTrenner.IsChecked
    irgendwas(12) = Leerzeichen.IsChecked
    irgendwas(13) = GeschLeerzeichen.IsChecked
    irgendwas(14) = Tabulatoren.IsChecked
    irgendwas(15) = Umbrueche.IsChecked
    irgendwas(16) = VersteckterText.IsChecked
    irgendwas(17) = VersteckteAbsatze.IsChecked

    printlog ("'/// - all states inverting ///")

    if irgendwas(1) then DirektCursor.UnCheck Else DirektCursor.Check
    if irgendwas(2) then Zonen_Cursor.UnCheck Else Zonen_Cursor.Check

    if (Absatzausrichtung.IsChecked = True) then LinkerAbsatzrand.Check Else Absatzausrichtung.Check
    bTempRadioBut(1) = Absatzausrichtung.IsChecked
    bTempRadioBut(2) = LinkerAbsatzrand.IsChecked
    bTempRadioBut(3) = Tabulator.IsChecked
    bTempRadioBut(4) = TabulatorUndLeerzeichen.IsChecked

    if irgendwas(10) then Absatzenden.UnCheck Else Absatzenden.Check
    if irgendwas(11) then WeicheTrenner.UnCheck Else WeicheTrenner.Check
    if irgendwas(12) then Leerzeichen.UnCheck Else Leerzeichen.Check
    if irgendwas(13) then GeschLeerzeichen.UnCheck Else GeschLeerzeichen.Check
    if irgendwas(14) then Tabulatoren.UnCheck Else Tabulatoren.Check
    if irgendwas(15) then Umbrueche.UnCheck Else Umbrueche.Check
    if irgendwas(16) then VersteckterText.UnCheck Else VersteckterText.Check
    if irgendwas(17) then VersteckteAbsatze.UnCheck Else VersteckteAbsatze.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)

    printlog ("'/// - SO quit - start ///")

    Call wOfficeRestart

    printlog ("'/// - checking states ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "FORMATTINGAIDS" )

    if ( irgendwas(1) = DirektCursor.IsChecked ) then WarnLog "DirektCursor state changed BugID: 82955"
    if ( irgendwas(2) = Zonen_Cursor.IsChecked ) then WarnLog "Zonen_Cursor state changed"
    
    if ( bTempRadioBut(1) <> Absatzausrichtung.IsChecked       ) then WarnLog "Absatzausrichtung state changed"
    if ( bTempRadioBut(2) <> LinkerAbsatzrand.IsChecked        ) then WarnLog "LinkerAbsatzrand state changed"
    if ( bTempRadioBut(3) <> Tabulator.IsChecked               ) then WarnLog "Tabulator state changed"
    if ( bTempRadioBut(4) <> TabulatorUndLeerzeichen.IsChecked ) then WarnLog "TabulatorUndLeerzeichen state changed"
    
    if ( irgendwas(10) = Absatzenden.IsChecked  ) then  WarnLog "Absatzenden state changed"
    if ( irgendwas(11) = WeicheTrenner.IsChecked  ) then  WarnLog "WeicheTrenner state changed"
    if ( irgendwas(12) = Leerzeichen.IsChecked  ) then  WarnLog "Leerzeichen state changed"
    if ( irgendwas(13) = GeschLeerzeichen.IsChecked  ) then  WarnLog "GeschLeerzeichen state changed"
    if ( irgendwas(14) = Tabulatoren.IsChecked  ) then  WarnLog "Tabulatoren state changed"
    if ( irgendwas(15) = Umbrueche.IsChecked  ) then  WarnLog "Umbrueche state changed"
    if ( irgendwas(16) = VersteckterText.IsChecked  ) then  WarnLog "VersteckterText state changed"
    if ( irgendwas(17) = VersteckteAbsatze.IsChecked  ) then  WarnLog "VersteckteAbsatze state changed"
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("'/// - all UnCheck -> o ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "FORMATTINGAIDS" )

    DirektCursor.UnCheck
    Zonen_Cursor.UnCheck

    if (LinkerAbsatzrand.IsChecked = True) then Tabulator.Check Else LinkerAbsatzrand.Check
    bTempRadioBut(1) = Absatzausrichtung.IsChecked
    bTempRadioBut(2) = LinkerAbsatzrand.IsChecked
    bTempRadioBut(3) = Tabulator.IsChecked
    bTempRadioBut(4) = TabulatorUndLeerzeichen.IsChecked
    
    Absatzenden.UnCheck
    WeicheTrenner.UnCheck
    Leerzeichen.UnCheck
    GeschLeerzeichen.UnCheck
    Tabulatoren.UnCheck
    Umbrueche.UnCheck
    VersteckterText.UnCheck
    VersteckteAbsatze.UnCheck
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("'/// - check if all UnChecked ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "FORMATTINGAIDS" )
    
    if DirektCursor.IsChecked then WarnLog "DirektCursor x"
    if Zonen_Cursor.IsChecked then WarnLog "Zonen_Cursor x"
       
    if ( bTempRadioBut(1) <> Absatzausrichtung.IsChecked       ) then WarnLog "Absatzausrichtung state changed"
    if ( bTempRadioBut(2) <> LinkerAbsatzrand.IsChecked        ) then WarnLog "LinkerAbsatzrand state changed"
    if ( bTempRadioBut(3) <> Tabulator.IsChecked               ) then WarnLog "Tabulator state changed"
    if ( bTempRadioBut(4) <> TabulatorUndLeerzeichen.IsChecked ) then WarnLog "TabulatorUndLeerzeichen state changed"
    
    if Absatzenden.IsChecked then  WarnLog "Absatzenden x"
    if WeicheTrenner.IsChecked then  WarnLog "WeicheTrenner x"
    if Leerzeichen.IsChecked then  WarnLog "Leerzeichen x"
    if GeschLeerzeichen.IsChecked then  WarnLog "GeschLeerzeichen x"
    if Tabulatoren.IsChecked then  WarnLog "Tabulatoren x"
    if Umbrueche.IsChecked then  WarnLog "Umbrueche x"
    if VersteckterText.IsChecked then  WarnLog "VersteckterText x"
    if VersteckteAbsatze.IsChecked then  WarnLog "VersteckteAbsatze x"
    
    Printlog ("'/// - all Check -> x ///")
    
    DirektCursor.Check
    Zonen_Cursor.Check
    
    if (Tabulator.IsChecked = True) then TabulatorUndLeerzeichen.Check Else Tabulator.Check
    bTempRadioBut(1) = Absatzausrichtung.IsChecked
    bTempRadioBut(2) = LinkerAbsatzrand.IsChecked
    bTempRadioBut(3) = Tabulator.IsChecked
    bTempRadioBut(4) = TabulatorUndLeerzeichen.IsChecked
    
    Absatzenden.Check
    WeicheTrenner.Check
    Leerzeichen.Check
    GeschLeerzeichen.Check
    Tabulatoren.Check
    Umbrueche.Check
    VersteckterText.Check
    VersteckteAbsatze.Check
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("'/// - check if all Checked ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "FORMATTINGAIDS" )
    
    if not DirektCursor.IsChecked then WarnLog "DirektCursor o"
    if not Zonen_Cursor.IsChecked then WarnLog "Zonen_Cursor o"
    
    if ( bTempRadioBut(1) <> Absatzausrichtung.IsChecked       ) then WarnLog "Absatzausrichtung state changed"
    if ( bTempRadioBut(2) <> LinkerAbsatzrand.IsChecked        ) then WarnLog "LinkerAbsatzrand state changed"
    if ( bTempRadioBut(3) <> Tabulator.IsChecked               ) then WarnLog "Tabulator state changed"
    if ( bTempRadioBut(4) <> TabulatorUndLeerzeichen.IsChecked ) then WarnLog "TabulatorUndLeerzeichen state changed"
    
    if Not Absatzenden.IsChecked then  WarnLog "Absatzenden o BugID: 82942 OS"
    if Not WeicheTrenner.IsChecked then  WarnLog "WeicheTrenner o"
    if Not Leerzeichen.IsChecked then  WarnLog "Leerzeichen o"
    if Not GeschLeerzeichen.IsChecked then  WarnLog "GeschLeerzeichen o"
    if Not Tabulatoren.IsChecked then  WarnLog "Tabulatoren o"
    if Not Umbrueche.IsChecked then  WarnLog "Umbrueche o"
    if Not VersteckterText.IsChecked then  WarnLog "VersteckterText o"
    if Not VersteckteAbsatze.IsChecked then  WarnLog "VersteckteAbsatze o"
    
    printlog ("'/// - restore states ///")
    
    if ( irgendwas(1) = TRUE ) then  DirektCursor.Check Else DirektCursor.UnCheck
    if ( irgendwas(2) = TRUE ) then  Zonen_Cursor.Check Else Zonen_Cursor.UnCheck
    
    if ( bRadioBut(1) = TRUE ) then Absatzausrichtung.Check
    if ( bRadioBut(2) = TRUE ) then LinkerAbsatzrand.Check
    if ( bRadioBut(3) = TRUE ) then Tabulator.Check
    if ( bRadioBut(4) = TRUE ) then TabulatorUndLeerzeichen.Check
    
    if ( irgendwas(10) = TRUE ) then  Absatzenden.Check Else Absatzenden.UnCheck
    if ( irgendwas(11) = TRUE ) then  WeicheTrenner.Check Else WeicheTrenner.UnCheck
    if ( irgendwas(12) = TRUE ) then  Leerzeichen.Check Else Leerzeichen.UnCheck
    if ( irgendwas(13) = TRUE ) then  GeschLeerzeichen.Check Else GeschLeerzeichen.UnCheck
    if ( irgendwas(14) = TRUE ) then  Tabulatoren.Check Else Tabulatoren.UnCheck
    if ( irgendwas(15) = TRUE ) then  Umbrueche.Check Else Umbrueche.UnCheck
    if ( irgendwas(16) = TRUE ) then  VersteckterText.Check Else VersteckterText.UnCheck
    if ( irgendwas(17) = TRUE ) then  VersteckteAbsatze.Check Else VersteckteAbsatze.UnCheck
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------
testcase tToolsOptionsWriterChanges
   Dim iListBox(2) as integer            ' ListBox
   Dim iTempListBox(2) as integer
   Dim iListBox2(3,2) as integer         ' ListBox
   Dim iTempListBox2(3,2) as integer

    Call hNewDocument
    Call wBlindtextEinfuegen
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Return>"
    Call hTabelleEinfuegen
    
    Call wTypeKeys ("<Up>" , 2)
    
    printlog ("'///- Tools/Options/Writer: Changes ///")
    
    printlog ("'/// - save states ///")
    ToolsOptions
    Call hToolsOptions ("WRITER","CHANGES")
    
    iListBox2(1,1) = EinfuegenAttribute.GetSelIndex
    iListBox2(1,2) = EinfuegenFarbe.GetSelIndex
    iListBox2(2,1) = LoeschenAttribute.GetSelIndex
    iListBox2(2,2) = LoeschenFarbe.GetSelIndex
    iListBox2(3,1) = AendernAttribute.GetSelIndex
    iListBox2(3,2) = AendernFarbe.GetSelIndex
    
    iListBox(1) = ZeilenMarkierung.GetSelIndex
    iListBox(2) = ZeilenFarbe.GetSelIndex
    
    printlog ("'///   - Change states ///")
    EinfuegenAttribute.Select 1
    EinfuegenFarbe.Select 15
    LoeschenAttribute.Select 5
    LoeschenFarbe.Select 4
    AendernAttribute.Select 8
    AendernFarbe.Select 65
    ZeilenMarkierung.Select 4
    ZeilenFarbe.Select 19
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog "'///   - changes record with current formats  ///"
    UseBindings
    EditChangesRecord
    
    Call wTypeKeys "<Return> Here are some changes"
    DocumentWriter.TypeKeys "<Up>", 8
    DocumentWriter.TypeKeys "<Backspace>", 10
    DocumentWriter.TypeKeys "<Down>", 2
    DocumentWriter.TypeKeys "<Delete>", 10
    DocumentWriter.TypeKeys "<Shift End>"
    FormatCharacter
    Kontext
    Active.SetPage TabFontEffects
    Kontext "TabFontEffects"
    Outline.Check
    Shadow.Check
    TabFontEffects.OK
    
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Down>", 20
    DocumentWriter.TypeKeys "<Up>Die ﾎnderung in einer Tabelle"
    DocumentWriter.TypeKeys "<Tab>", 10
    
    printlog ("'/// - all states inverting ///")
    
    ToolsOptions
    Call hToolsOptions ("WRITER","CHANGES")
    
    if ( EinfuegenAttribute.GetItemCount <> iListBox2(1,1) ) then EinfuegenAttribute.Select (iListBox2(1,1)+1) Else EinfuegenAttribute.Select(iListBox2(1,1)-1)
    if ( EinfuegenFarbe.GetItemCount     <> iListBox2(1,2) ) then EinfuegenFarbe.Select     (EinfuegenFarbe.GetItemCount    ) Else EinfuegenFarbe.Select    (1)
    iTempListBox2(1,1) = EinfuegenAttribute.GetSelIndex
    iTempListBox2(1,2) = EinfuegenFarbe.GetSelIndex
    if ( LoeschenAttribute.GetItemCount <> iListBox2(2,1) ) then LoeschenAttribute.Select (LoeschenAttribute.GetItemCount) Else LoeschenAttribute.Select(1)
    if ( LoeschenFarbe.GetItemCount     <> iListBox2(2,2) ) then LoeschenFarbe.Select     (LoeschenFarbe.GetItemCount    ) Else LoeschenFarbe.Select    (1)
    iTempListBox2(2,1) = LoeschenAttribute.GetSelIndex
    iTempListBox2(2,2) = LoeschenFarbe.GetSelIndex
    if ( AendernAttribute.GetItemCount <> iListBox2(3,1) ) then AendernAttribute.Select (AendernAttribute.GetItemCount) Else AendernAttribute.Select(1)
    if ( AendernFarbe.GetItemCount     <> iListBox2(3,2) ) then AendernFarbe.Select     (AendernFarbe.GetItemCount    ) Else AendernFarbe.Select    (1)
    iTempListBox2(3,1) = AendernAttribute.GetSelIndex
    iTempListBox2(3,2) = AendernFarbe.GetSelIndex    
    if ( ZeilenMarkierung.GetItemCount   <> iListBox(1) ) then ZeilenMarkierung.Select   (ZeilenMarkierung.GetItemCount  ) Else ZeilenMarkierung.Select  (1)
    if ( ZeilenFarbe.GetItemCount        <> iListBox(2) ) then ZeilenFarbe.Select        (ZeilenFarbe.GetItemCount       ) Else ZeilenFarbe.Select       (1)
    iTempListBox(1) = ZeilenMarkierung.GetSelIndex
    iTempListBox(2) = ZeilenFarbe.GetSelIndex
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("'/// - SO quit - start ///")

    Call wOfficeRestart
    WaitSlot (2)

    printlog ("'/// - checking states ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "CHANGES" )

    if ( EinfuegenAttribute.GetSelIndex <> iTempListBox2(1,1) ) then WarnLog "EinfuegenAttribute state changed; is: "+EinfuegenAttribute.GetSelIndex +"; should: "+ iTempListBox2(1,1)
    if ( EinfuegenFarbe.GetSelIndex     <> iTempListBox2(1,2) ) then WarnLog "EinfuegenFarbe state changed"
    if ( LoeschenAttribute.GetSelIndex <> iTempListBox2(2,1) ) then WarnLog "LoeschenAttribute state changed"
    if ( LoeschenFarbe.GetSelIndex     <> iTempListBox2(2,2) ) then WarnLog "LoeschenFarbe state changed"
    if ( AendernAttribute.GetSelIndex <> iTempListBox2(3,1) ) then WarnLog "AendernAttribute state changed"
    if ( AendernFarbe.GetSelIndex     <> iTempListBox2(3,2) ) then WarnLog "AendernFarbe state changed"
    if ( ZeilenMarkierung.GetSelIndex   <> iTempListBox(1) ) then WarnLog "ZeilenMarkierung state changed"
    if ( ZeilenFarbe.GetSelIndex        <> iTempListBox(2) ) then WarnLog "ZeilenFarbe state changed"
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("'/// - all UnCheck -> o ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "CHANGES" )
    
    EinfuegenAttribute.Select(1)
    EinfuegenFarbe.Select    (1)
    LoeschenAttribute.Select (LoeschenAttribute.GetItemCount)
    LoeschenFarbe.Select     (LoeschenFarbe.GetItemCount    )
    AendernAttribute.Select(1)
    AendernFarbe.Select    (1)
    
    ZeilenMarkierung.Select  (1)
    ZeilenFarbe.Select       (1)
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("'/// - check if all UnChecked ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "CHANGES" )
    
    if (EinfuegenAttribute.GetSelIndex <> 1 ) then WarnLog "EinfuegenAttribute is not item 1"
    if (EinfuegenFarbe.GetSelIndex     <> 1 ) then WarnLog "EinfuegenFarbe is not item 1"
    if (LoeschenAttribute.GetSelIndex <> LoeschenAttribute.GetItemCount ) then WarnLog "LoeschenAttribute is not item (GetItemCount)"
    if (LoeschenFarbe.GetSelIndex     <> LoeschenFarbe.GetItemCount     ) then WarnLog "LoeschenFarbe is not item (GetItemCount)"
    if (AendernAttribute.GetSelIndex <> 1 ) then WarnLog "AendernAttribute is not item 1"
    if (AendernFarbe.GetSelIndex     <> 1 ) then WarnLog "AendernFarbe is not item 1"
    if (ZeilenMarkierung.GetSelIndex   <> 1 ) then WarnLog "ZeilenMarkierung is not item 1"
    if (ZeilenFarbe.GetSelIndex        <> 1 ) then WarnLog "ZeilenFarbe is not item 1"
    
    Printlog ("'/// - all Check -> x ///")
    EinfuegenAttribute.Select (EinfuegenAttribute.GetItemCount)
    EinfuegenFarbe.Select     (EinfuegenFarbe.GetItemCount    )
    LoeschenAttribute.Select(1)
    LoeschenFarbe.Select    (1)
    AendernAttribute.Select (AendernAttribute.GetItemCount)
    AendernFarbe.Select     (AendernFarbe.GetItemCount    )
    
    ZeilenMarkierung.Select   (ZeilenMarkierung.GetItemCount  )
    ZeilenFarbe.Select        (ZeilenFarbe.GetItemCount       )
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("'/// - check if all Checked ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "CHANGES" )
    
    if (EinfuegenAttribute.GetSelIndex <> EinfuegenAttribute.GetItemCount ) then WarnLog "EinfuegenAttribute is not item (GetItemCount)"
    if (EinfuegenFarbe.GetSelIndex     <> EinfuegenFarbe.GetItemCount     ) then WarnLog "EinfuegenFarbe is not item (GetItemCount)"
    if (LoeschenAttribute.GetSelIndex <> 1 ) then WarnLog "LoeschenAttribute is not item 1"
    if (LoeschenFarbe.GetSelIndex     <> 1 ) then WarnLog "LoeschenFarbe is not item 1"
    if (AendernAttribute.GetSelIndex <> AendernAttribute.GetItemCount ) then WarnLog "AendernAttribute is not item (GetItemCount)"
    if (AendernFarbe.GetSelIndex     <> AendernFarbe.GetItemCount     ) then WarnLog "AendernFarbe is not item (GetItemCount)"
    
    if (ZeilenMarkierung.GetSelIndex   <> ZeilenMarkierung.GetItemCount   ) then WarnLog "ZeilenMarkierung is not item (GetItemCount)"
    if (ZeilenFarbe.GetSelIndex        <> ZeilenFarbe.GetItemCount        ) then WarnLog "ZeilenFarbe is not item (GetItemCount)"
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel
    
    '--------------------------------------------------------------------------
    'new document starts with unchecked edit changes record!? (i hope always ;-))
    printlog ("'/ Function test: color-real-print / screenshot also possible///'")
    Call hNewDocument

    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "This is normal text"
    DocumentWriter.TypeKeys "<return>"
    DocumentWriter.TypeKeys "The quick brown fox jumps over the lazy dog"
    DocumentWriter.TypeKeys "<return>"

    ToolsOptions
    Call hToolsOptions ( "WRITER" , "CHANGES" )
    EinfuegenAttribute.Select (2) 'bold
    EinfuegenFarbe.Select     (8) 'mangenta
    LoeschenAttribute.Select  (4) 'strikethrough
    LoeschenFarbe.Select      (92)  ' sun3
    AendernAttribute.Select   (8) 'smallcaps
    AendernFarbe.Select       (30)'red4

    ZeilenMarkierung.Select   (4) 'outer margins
    ZeilenFarbe.Select        (52)'turquise4
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok

    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "This is inserted text"
    DocumentWriter.TypeKeys "<return>"
    EditChangesRecord
    DocumentWriter.TypeKeys "The quick brown fox jumps over the lazy dog"
    DocumentWriter.TypeKeys "<return>"
    EditChangesRecord
    DocumentWriter.TypeKeys "This is attribut-changed text"
    DocumentWriter.TypeKeys "<return>"
    DocumentWriter.TypeKeys "The quick brown fox jumps over the lazy dog"
    EditChangesRecord
    DocumentWriter.TypeKeys "<shift home>"
    DocumentWriter.TypeKeys "<mod1 i><end>"
    DocumentWriter.TypeKeys "<return>"
    EditChangesRecord
    DocumentWriter.TypeKeys "This is backspaced text"
    DocumentWriter.TypeKeys "<return>"
    DocumentWriter.TypeKeys "The quick brown fox jumps over the lazy dog"
    DocumentWriter.TypeKeys "<shift home>"
    EditChangesRecord
    DocumentWriter.TypeKeys "<backspace>"
    EditChangesRecord
    DocumentWriter.TypeKeys "<end><return>"
    DocumentWriter.TypeKeys "This is deleted text"
    DocumentWriter.TypeKeys "<return>"
    DocumentWriter.TypeKeys "The quick brown fox jumps over the lazy dog"
    EditChangesRecord
    DocumentWriter.TypeKeys "<shift home>"
    DocumentWriter.TypeKeys "<delete><end>"
    DocumentWriter.TypeKeys "<return>"
    EditChangesRecord
    Call hCloseDocument

    '--------------------------------------------------------------------------
    printlog ("'/// - restore states ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "CHANGES" )

    try
        EinfuegenAttribute.Select (iListBox2(1,1))
    catch
        EinfuegenAttribute.Select (1)
        Warnlog ("EinfuegenAttribute has wrong initialvalue: " + iListBox2(1,1) + ", BugID: 83097")
    endcatch
    try
        EinfuegenFarbe.Select     (iListBox2(1,2))
    catch
        EinfuegenFarbe.Select (1)
        WarnLog ("EinfuegenFarbe has wrong initialvalue: " + iListBox2(1,2))
    endcatch

    try
        printlog       iListBox2(2,1)
        LoeschenAttribute.Select (iListBox2(2,1))
    catch
        LoeschenAttribute.Select (1)
        WarnLog ("LoeschenAttribute has wrong initialvalue: " + iListBox2(2,1) )
    endcatch

    try
        LoeschenFarbe.Select     (iListBox2(2,2))
    catch
        LoeschenFarbe.Select (1)
        WarnLog ("LoeschenFarbe has wrong initialvalue: " + iListBox2(2,2))
    endcatch

    try
        AendernAttribute.Select (iListBox2(3,1))
    catch
        AendernAttribute.Select (1)
        WarnLog ("AendernAttribute has wrong initialvalue: " + iListBox2(3,1) )
    endcatch

    try
        AendernFarbe.Select     (iListBox2(3,2))
    catch
        AendernFarbe.Select (1)
        WarnLog ("AendernFarbe has wrong initialvalue: " + iListBox2(3,2))
    endcatch

    try
        ZeilenMarkierung.Select   (iListBox(1))
    catch
        ZeilenMarkierung.Select (1)
        WarnLog ("ZeilenMarkierung has wrong initialvalue: " + iListBox(3))
    endcatch

    try
        ZeilenFarbe.Select        (iListBox(2))
    catch
        ZeilenFarbe.Select (1)
        WarnLog ("ZeilenFarbe has wrong initialvalue: " + iListBox(4))
    endcatch

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------
testcase tToolsOptionsWriterInsert

    Dim irgendwas(8) as boolean    ' Checkbox states

    Call hNewDocument
    printlog "'///- Tools/Options/Writer: Table ///"

    printlog ("'/// - save states ///")
    ToolsOptions
    Call hToolsOptions ("WRITER","Table")

    irgendwas(2) = Ueberschrift.IsChecked
    irgendwas(3) = Wiederholen.IsChecked
    irgendwas(4) = NichtTrennen.IsChecked
    irgendwas(5) = Umrandung.IsChecked

    irgendwas(6) = AutomatischeZahlenerkennung.IsChecked
    irgendwas(7) = Zahlenformaterkennung.IsChecked
    irgendwas(8) = AutomatischeAusrichtung.IsChecked

    printlog ("'/// - all states inverting ///")

    Ueberschrift.Check
    if irgendwas(3) then Wiederholen.UnCheck                 Else Wiederholen.Check
    if irgendwas(4) then NichtTrennen.UnCheck                Else NichtTrennen.Check
    if irgendwas(5) then Umrandung.UnCheck                   Else Umrandung.Check
    AutomatischeZahlenerkennung.Check
    if irgendwas(7) then Zahlenformaterkennung.UnCheck       Else Zahlenformaterkennung.Check
    if irgendwas(8) then AutomatischeAusrichtung.UnCheck     Else AutomatischeAusrichtung.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("'/// - SO quit - start ///")

    Call wOfficeRestart

    printlog ("'/// - checking states ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "Table" )

    if ( Ueberschrift.IsChecked                = False ) then WarnLog "Ueberschrift state changed"
    if ( irgendwas(3) = Wiederholen.IsChecked                 ) then WarnLog "Wiederholen state changed"
    if ( irgendwas(4) = NichtTrennen.IsChecked                ) then WarnLog "NichtTrennen state changed"
    if ( irgendwas(5) = Umrandung.IsChecked                   ) then WarnLog "Umrandung state changed"

    if ( AutomatischeZahlenerkennung.IsChecked = False ) then WarnLog "AutomatischeZahlenerkennung state changed"
    if ( irgendwas(7) = Zahlenformaterkennung.IsChecked       ) then WarnLog "Zahlenformaterkennung state changed"
    if ( irgendwas(8) = AutomatischeAusrichtung.IsChecked     ) then WarnLog "AutomatischeAusrichtung state changed"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("'/// - all UnCheck -> o ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "Table" )

    Ueberschrift.UnCheck
    NichtTrennen.UnCheck
    Umrandung.UnCheck
    AutomatischeZahlenerkennung.UnCheck

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Printlog "'///    - check in InsertTableWriter ///"
    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"

    if ( Ueberschrift.IsChecked          ) then WarnLog "Ueberschrift x"
    if ( TabelleNichtTrennen.IsChecked ) then WarnLog "NichtTrennen x"
    if ( Umrandung.IsChecked           ) then WarnLog "Umrandung x"

    TabelleEinfuegenWriter.OK

    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Down>1<Tab><Left>"
    FormatParagraph
    Kontext
    Active.SetPage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"
    if NOT Links.IsChecked then Warnlog "Die automatische Ausrichtung der Zahl wurde trotzdem durchgefﾁhrt!"
    TabAusrichtungAbsatz.Cancel

    FormatNumberFormat
    Kontext "Zahlenformat"
    if Kategorie.GetSelIndex <> 11 then Warnlog "Das Zahlenformat ist nicht Text geblieben!"
    Zahlenformat.Cancel
    Call hCloseDocument
    Call hNewDocument

    printlog ("'/// - check if all UnChecked ///")
	Call hNewDocument
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "Table" )

    if Ueberschrift.IsChecked                then WarnLog "Ueberschrift x"
    if NichtTrennen.IsChecked                then WarnLog "NichtTrennen x"
    if Umrandung.IsChecked                   then WarnLog "Umrandung x"

    if AutomatischeZahlenerkennung.IsChecked then WarnLog "AutomatischeZahlenerkennung x"

    Printlog ("'/// - all Check -> x ///")

    Ueberschrift.Check
    Wiederholen.Check
    NichtTrennen.Check
    Umrandung.Check

    AutomatischeZahlenerkennung.Check
    Zahlenformaterkennung.Check
    AutomatischeAusrichtung.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Printlog "'///    - check in InsertTableWriter ///"
    Call hNewDocument
    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"

    if ( Ueberschrift.IsChecked                = FALSE ) then WarnLog "Ueberschrift o"
    if ( UeberschriftWiederholen.IsChecked                 = FALSE ) then WarnLog "Wiederholen o"
    if ( TabelleNichtTrennen.IsChecked                = FALSE ) then WarnLog "NichtTrennen o"
    if ( Umrandung.IsChecked                   = FALSE ) then WarnLog "Umrandung o"

    TabelleEinfuegenWriter.Ok

    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Down>1<Tab><Left>"
    FormatParagraph
    Kontext
    Active.SetPage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"
    if NOT Rechts.IsChecked then Warnlog "Alignment of number didn't change to: right!"
    TabAusrichtungAbsatz.Cancel

    FormatNumberFormat
    Kontext "Zahlenformat"
    if Kategorie.GetSelIndex <> 3 then Warnlog "numberformat didn't get changed!"
    Zahlenformat.Cancel

    Call hCloseDocument
    Call hNewDocument

    printlog ("'/// - check if all Checked ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "Table" )

    if ( Ueberschrift.IsChecked                = FALSE ) then WarnLog "Ueberschrift o"
    if ( Wiederholen.IsChecked                 = FALSE ) then WarnLog "Wiederholen o"
    if ( NichtTrennen.IsChecked                = FALSE ) then WarnLog "NichtTrennen o"
    if ( Umrandung.IsChecked                   = FALSE ) then WarnLog "Umrandung o"

    if ( AutomatischeZahlenerkennung.IsChecked = FALSE ) then WarnLog "AutomatischeZahlenerkennung o"
    if ( Zahlenformaterkennung.IsChecked       = FALSE ) then WarnLog "Zahlenformaterkennung o"
    if ( AutomatischeAusrichtung.IsChecked     = FALSE ) then WarnLog "AutomatischeAusrichtung o"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("'/// - restore states ///")

    Call wOptionsUndo("Table")

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ********************************************************************

testcase tToolsOptionsAutocaption

    Dim Auswahl(20) as String
    Dim i as Integer
    Dim SplitString as string
    Dim ObjectTmp() as string
    printlog "- Tools/Options/StarOffice Writer/AutoCaption"

    Call hNewDocument
    Call wOptionsUndo( "All" )

    ToolsOptions
    Call hToolsOptions("WRITER","AutoCaption")
    Kontext "TabAutocaption"
    Select Case iSprache
        case 36     :   SplitString = "-"
        case else   :   SplitString = " "
    end select
    for i=1 to ObjectList.GetItemCount
        ObjectList.Select i
        ObjectList.Check
        ObjectTmp() = Split(ObjectList.GetSelText, SplitString)
        Auswahl(i) = ObjectTmp(Ubound(ObjectTmp()))
        Category.SetText Auswahl(i)
    next i

    ObjectList.Select 1
    Position.Select Position.GetItemCount   'Select "Caption: Below"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog "   - Check table caption"
    ViewZoom
    Kontext "Massstab"
    Seitenbreite.Check
    Massstab.OK
    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"
    TabelleEinfuegenWriter.OK
    Kontext "DocumentWriter"
    Call wTypeKeys ("<MOD1 Home>")
    Call wTypeKeys ("<DOWN>", 2)
    Call wTypeKeys ("<HOME>")
    Select Case iSprache
        Case 31:
            Call wTypeKeys ("<Mod1 Shift Right>") ', 3
        Case else
            Call wTypeKeys ("<Mod1 Shift Right>")
    end select
    EditCopy
    wait 500
    if Trim$(GetClipboardText) <> Auswahl ( 1 ) then Warnlog "Table caption is wrong. Not " & Auswahl(1) & " but: " & GetClipboardtext
    EditUndo

    printlog "'///   - Check frame caption ///"
    InsertFrame
    Kontext
    Active.SetPage TabType
    Kontext "TabType"
'    if gPlatGroup = "unx" then
    Width.SetText "2"
    TabType.OK
    Call wTypeKeys ("<Return>")
    sleep (1)
    Call wTypeKeys ("<Down><Home>")
    Select Case iSprache
        Case 31:
            Call wTypeKeys ("<Mod1 Shift Right>", 3)
        Case else
            Call wTypeKeys ("<Mod1 Shift Right>")
    end select
    EditCopy
    Wait 500
    if Trim$(GetClipboardText) <> Auswahl (2) then 
        Warnlog "Caption of a frame is wrong.  Not " & Auswahl(2) & " but: " & GetClipboardtext
    endif
    Call wTypeKeys ("<Escape>")
    sleep(1)
    Call wTypeKeys ("<Home><SHIFT END><Delete>")

    printlog "'///   - Check graphics caption ///"
    '///+ "writer\optional\input\graphics\jolink.jpg" ///
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\optional\input\graphics\jolink.jpg" )
        Call wTypeKeys ("<Return>")
        sleep (1)
        Call wTypeKeys ("<MOD1 Home>")
    Select Case iSprache
        Case 31:
            Call wTypeKeys ("<Mod1 Shift Right>", 3)
        Case 82:
            Call wTypeKeys ("<Down><Shift End>")
        Case else
            Call wTypeKeys ("<Mod1 Shift Right>")
    end select
    EditCopy
    if Trim$(GetClipboardText) <> Auswahl (3) then 
        Warnlog "Caption of a graphic is wrong. Not " & Auswahl(3) & " but: " & GetClipboardtext
    endif
    Call hCloseDocument
    
    printlog "'///   - Check caption of all other objects ///"
    for i=1 to 5
        Call hNewDocument
        InsertObjectOLEObject
        Kontext "OLEObjektEinfuegen"
        Objekttyp.Select i
        printlog " - " & Objekttyp.GetSelText
        OLEObjektEinfuegen.OK

        Kontext "DocumentWriter"
        Call gMouseClick ( 90, 90 )
        Sleep 2
        Call wTypeKeys ("<Return>")
        sleep (2)
        Call wTypeKeys ("<MOD1 Home>")
        Select Case i
            Case 1,3,4,5
                Select Case iSprache
                    Case 31
                        Call wTypeKeys ("<Mod1 Shift Right>",3)
                    Case else
                        Call wTypeKeys ("<Mod1 Shift Right>")
                end select
            Case else
                Call wTypeKeys ("<Mod1 Shift Right>")
        end select
        EditCopy
        if Trim$(GetClipboardText) <> Auswahl (i+3) then Warnlog "Caption of a " & Auswahl(i+3) & " is wrong. Not " & Auswahl(i+3) & " but: " & GetClipboardtext
        sleep (1)
        Call hCloseDocument
    next i
    Call hNewDocument
    printlog "'///   - Set defaults ///"
    ViewZoom
    Kontext "Massstab"
    Vergroesserung100.Check
    Massstab.OK

    ToolsOptions
    Call hToolsOptions("WRITER","AutoCaption")
    Kontext "TabAutocaption"
    for i=1 to ObjectList.GetItemCount
        ObjectList.Select i
        ObjectList.Uncheck
    next i

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    sleep (1)
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------

testcase tToolsOptionsWriterCompatibility1

    Dim i as integer

    printlog "Check status change of all checkboxes in StarOffice Writer->Compatibility options" 
    printlog " Test of option 'Use StarOffice 6.0/7 tabstop formatting'"
    Call hNewDocument
    ToolsOptions
    Call hToolsOptions("WRITER","COMPATIBILITY")
    printlog " Check defaults of checkboxes. if default is wrong then set to default"
    Kontext "TabCompatibility"
    For i = 1 to Options.GetItemCount
        Options.Select i
        Select case i
            Case 2,3,7,11
                if Not Options.IsChecked then
                    Warnlog "Default for checkbox " & i & " is wrong (not checked)"
                    Options.Check
                end if
            Case else
                if Options.IsChecked then
                    Warnlog "Default for checkbox " & i & " is wrong (checked)"
                    Options.UnCheck
                end if
        end select
    next i
    printlog " - check all checkboxes"
    For i = 1 to Options.GetItemCount
        Options.Select i
        Options.Check
    next i
    Default.Click
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.Yes
        end if
    end if
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok

    printlog "- save document"
    Call hFileSaveAsKill (gOfficePath + "user\work\tToolsOptionsWriterCompatibility1.odt")
    Call hCloseDocument

    printlog "- restart the office"
    Call wOfficeRestart

    printlog "- load previously saved document"
    Call hNewDocument
    hFileOpen( gOfficePath & "user\work\tToolsOptionsWriterCompatibility1.odt")

    Printlog "check state of all checkboxes and uncheck all checkboxes"
    ToolsOptions
    Call hToolsOptions("WRITER","COMPATIBILITY")
    Kontext "TabCompatibility"
    For i = 1 to Options.GetItemCount
        Options.Select i
        if Not Options.IsChecked then
            Warnlog "Checkbox " & i & " looses state after restart"
        else
            Options.Uncheck
        end if
    next i
    Default.Click
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.Yes
        end if
    end if
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok

    Call hCloseDocument

    '/// restart the office
    Call wOfficeRestart
    WaitSlot (2)

    printlog "check state of all checkboxes and set defaults"
    ToolsOptions
    Call hToolsOptions("WRITER","COMPATIBILITY")
    Kontext "TabCompatibility"
    For i = 1 to Options.GetItemCount
        Options.Select i
        if Options.IsChecked then
            Warnlog "Checkbox " & i & " looses state after restart"
        end if
        Select Case i
            Case 2,3,7,11
                Options.Check
            Case else
                Options.UnCheck
        end select
    next i
    Default.Click
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.Yes
        end if
    end if
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------

testcase tToolsOptionsWriterCompatibility2

    Dim i as integer
    Dim DefaultFailed as boolean
    printlog "Test of option 'Use StarOffice 6.0/7 tabstop formatting'"
    
    Call hNewDocument
    printlog "open document  ./writer/leve11/input/options/compatibility1.doc"
    printlog "check formatting of current opened document"
    hFileOpenLocally(  gTesttoolpath & "writer\optional\input\options\compatibility1.doc" )
    

    Call wTypeKeys ("<Mod1 Home>")
    Call wTypeKeys ("<Down>")
    Call wTypeKeys ("<Shift Right>")

    try
        EditCopy
        if GetClipboardtext <> "A" then
            Warnlog "Seems document has been formatted wrong"
            DefaultFailed = true
        end if
    catch
        Warnlog "Seems document has been formatted wrong"
        DefaultFailed = true
    endcatch

    Call wTypeKeys ("<Home>")
    Call wTypeKeys ("<Down>" ,3)
    Call wTypeKeys ("<Shift Right>")
    try
        EditCopy
        if GetClipboardtext <> "A" then
            Warnlog "Seems document has been formatted wrong"
            DefaultFailed = true
        end if
    catch
        Warnlog "Seems document has been formatted wrong"
        DefaultFailed = true
    endcatch

    if DefaultFailed = false then
        printlog " in Tools->Options->StarOffice Writer->Compatitibility check option 'Use StarOffice 6.0/7 tabstop formatting'"
        ToolsOptions
        Call hToolsOptions("WRITER","COMPATIBILITY")
        printlog " Check defaults of checkboxes. if default is wrong then set to default"
        Kontext "TabCompatibility"
        Options.Select 4
        Options.Check
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.Ok
        printlog " Now check if document has been reformatted"
        Call wTypeKeys ("<Mod1 Home>")
        Call wTypeKeys ("<Down>" , 2)
        Call wTypeKeys ("<Shift Right>")
        try
            EditCopy
            if GetClipboardtext <> "A" then
                Warnlog "Seems document has been formatted wrong"
            end if
        catch
            Warnlog "Seems document has been formatted wrong"
        endcatch

        Call wTypeKeys ("<Home>")
        Call wTypeKeys ("<Down>" , 4)
        Call wTypeKeys ("<Shift Right>")
        try
            EditCopy
            if GetClipboardtext <> "A" then
                Warnlog "Seems document has been formatted wrong"
            end if
        catch
            Warnlog "Seems document has been formatted wrong"
        endcatch

        printlog "in Tools->Options->StarOffice Writer->Compatitibility uncheck option 'Use StarOffice 6.0/7 tabstop formatting'"
        ToolsOptions
        Call hToolsOptions("WRITER","COMPATIBILITY")
        printlog "Check defaults of checkboxes. if default is wrong then set to default"
        Kontext "TabCompatibility"
        Options.Select 4
        Options.UnCheck
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.Ok
    end if

    printlog "Close document"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------------------------------------
testcase tToolsOptionsWriterCompatibility3

    DIM iWords_to_right     AS integer 'variable for counting word-steps
    DIM iOptionsCount       AS integer 'variable for counting options on cmopatibility page
    DIM sDocument           AS string  'variable for saving the doc
    DIM sFoundWord          AS string  'variable for word found one line below

    printlog ">>> Configuration of behavior when a line break is inserted <<<"
    '/// New Writer Doc
    Call hNewDocument
    sleep (1)
    printlog "New writer doc opened"
    ToolsOptions
    Kontext "ExtrasOptionenDLG"
    printlog "Checking Options settings"

    printlog "Tools-Options-OOo Writer-Compatibility"
    Call hToolsOptions("WRITER","COMPATIBILITY")
    Kontext "TabCompatibility"

    printlog "Verify that the entry 'Expand word space on lines with manual line breaks in justified paragraphs' is checked by default"
    iOptionsCount = Options.GetItemCount
    Options.Select iOptionsCount
    if Options.isChecked then
        printlog "Expand word space on lines... is checked"
    else
        warnlog "Warning: Expand word space on lines...is not checked as default!"
        Options.Check
    end if

    printlog " Close dialog with OK"
    Kontext "ExtrasOptionenDLG"
    ExtrasOptionenDLG.OK
    sleep (1)

    printlog " Insert a paragraph with several lines of text"
    Kontext "DocumentWriter"
    DocumentWriter.typekeys ("one two three four five six seven eight nine ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen nineteen twenty twentyone twentytwo twentythree twentyfour twentyfive twentysix")
    printlog "Inserting text"
    Kontext "DocumentWriter"

    DocumentWriter.UseMenu 'use menu to select all
    Call hMenuSelectNr (2) 'edit
    CALL hMenuFindSelect (".uno:SelectAll", true, 11, true)   'Find "Select All" and call the slot.

    printlog "Click the 'Justified icon in the toolbar"
    sleep (1)
    Kontext "TextObjectbar"
    Blocksatz.Click 'setting to justified
    Kontext "DocumentWriter"

    printlog "(Now the paragraph adjustment is 'justified')"
    printlog "Setting text to justified."
    sleep (1)

    printlog " Click into the middle of a line"
    DocumentWriter.TypeKeys "<MOD1 HOME>" 'jumping to beginning of text
    DocumentWriter.TypeKeys "<MOD1 RIGHT>",9,true '9 times to the right
    sleep (1)

    printlog " Insert a line break by pressing <SHIFT>+<ENTER>"
    DocumentWriter.TypeKeys "<SHIFT RETURN>"
    sleep (1)

    printlog " Verify that the remaining words are aligned justified"
    DocumentWriter.TypeKeys "<MOD1 LEFT>" 'going back into remaining text
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<DOWN>" 'one line down
    sleep (1)
    DocumentWriter.TypeKeys "<MOD1 LEFT>" 'one word to the left
    sleep (1)
    DocumentWriter.TypeKeys "<MOD1 SHIFT RIGHT>" 'selecting complete word
    sleep (1)
    DocumentWriter.TypeKeys "<SHIFT LEFT>" 'deleting space
    EditCopy
    if (GetClipboardText = "twenty") OR (GetClipboardText = "nineteen") OR (GetClipboardText = "eighteen") then  'one word or the other, because the system-font might differ from system to system...
        printlog "Correct word found."
    else
        warnlog "wrong word found, formatting broken. Found word was: '" + GetClipboardText + "'."
    end if
    sleep (1)

    printlog "Tools-Options-OOo Writer-Compatibility"
    ToolsOptions
    Kontext "ExtrasOptionenDLG"

    printlog "Uncheck the entry 'Expand word space [...]'"
    Call hToolsOptions("WRITER","COMPATIBILITY")
    Kontext "TabCompatibility"
    iOptionsCount = Options.GetItemCount
    Options.Select iOptionsCount
    if Options.isChecked then
        printlog "Unchecking expand word space... settings now."
        Options.Uncheck 'unchecking
    else
        warnlog "Warning: Expand word space on lines...is not checked anymore!"
    end if
    Kontext "ExtrasOptionenDLG"

    printlog " Close dialog with OK"
    ExtrasOptionenDLG.OK

    printlog " Verify that the remainig words are aligned left"
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<UP>" 'one up
    sleep (1)
    DocumentWriter.TypeKeys "<MOD1 LEFT>" 'one word left to the same starting position as before
    sleep (1)
    DocumentWriter.TypeKeys "<DOWN>" ' one line down
    sleep (1)
    DocumentWriter.TypeKeys "<MOD1 LEFT>" 'one word left
    sleep (1)
    DocumentWriter.TypeKeys "<MOD1 SHIFT RIGHT>" ' selecting the complete word
    sleep (1)
    DocumentWriter.TypeKeys "<SHIFT LEFT>" 'deleting space
    EditCopy
    if (GetClipboardText = "fifteen") OR (GetClipboardText = "fourteen") then
        printlog "Correct word found, alignment ok"
    else
        warnlog "Wrong word found, formatting broken"
    end if
    sleep (1)

    printlog " Save document"
    sDocument = ConvertPath ( gOfficePath + "user\work\test1.odt")
    'deleting old Docs in the directory
    if Dir (sDocument) <> "" then kill sDocument 'deleting old docs
    Call hFileSaveAsKill (sDocument)
    printlog "Document saved"

    printlog "close Document"
    Call hCloseDocument
    printlog "Document Closed"

    printlog "New document"
    Call hNewDocument
    printlog "New Doc opened"

    printlog "Tools-Options-OOo Writer-Compatibility"
    ToolsOptions
    Kontext "ExtrasOptionenDLG"
    Call hToolsOptions("WRITER","COMPATIBILITY")
    Kontext "TabCompatibility"

    printlog "Verify that the entry 'Expand word space [...]' is checked"
    iOptionsCount = Options.GetItemCount
    Options.Select iOptionsCount
    if Options.isChecked then
        printlog "Expand word space on lines... is checked"
    else
        warnlog "Warning: Expand word space on lines...is not checked as default!"
    end if

    printlog "Close dialog with 'Cancel'"
    Kontext "ExtrasOptionenDLG"
    ExtrasOptionenDLG.Cancel

    printlog "Reload recently saved document"
    hFileOpen( sDocument)

    printlog "Tools-Options-OOo Writer-Compatibility"
    ToolsOptions
    Kontext "ExtrasOptionenDLG"
    Call hToolsOptions("WRITER","COMPATIBILITY")
    Kontext "TabCompatibility"

    printlog "Verify that the entry 'Expand word space [...]' is unchecked"
    iOptionsCount = Options.GetItemCount
    Options.Select iOptionsCount
    if Options.isChecked then
        warnlog "Warning: should not be checked in saved file!"
    else
        printlog "Ok, Expand word space on lines... in saved file is checked."
    end if
    sleep (1)

    printlog "Click 'Default'"
    Default.Click

    printlog "In upcoming dialog 'Do you want to change the default template', click 'Yes'"
    sleep (1)
    Kontext "Active"
    if Active.Exists then
        Active.Yes
        printlog "Changing to defaults"
    else
        warnlog "Warning: Messagebox is not up!"
    end if
    sleep (1)
    Kontext "ExtrasOptionenDLG"
    ExtrasOptionenDLG.OK

    printlog " Close document"
    Kontext "DocumentWriter"
    Call hCloseDocument

    printlog " New Writer document"
    Call hNewDocument

    printlog " Insert a paragraph with several lines of text"
    DocumentWriter.typekeys ("one two three four five six seven eight nine ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen nineteen twenty twentyone twentytwo twentythree twentyfour twentyfive twentysix")
    printlog "Inserting text"
    Kontext "DocumentWriter"
    DocumentWriter.UseMenu 'use menu to select all
    Call hMenuSelectNr (2) 'edit
    CALL hMenuFindSelect (".uno:SelectAll", true, 11, true)   'Find "Select All" and call the slot.

    printlog " Format-Paragraph-Tab 'Alignment' Click the 'Justified' radio button" 
    DocumentWriter.UseMenu 'use menu to call format/paragraph
    Call hMenuSelectNr (5) 'Format
    Call hMenuSelectNr (3) 'Paragraph
    sleep (1)
    Kontext
    Active.SetPage TabAusrichtungAbsatz 'changing to correct tabpage
    sleep (1)
    Kontext "TabAusrichtungAbsatz"
    Blocksatz.Check 'checking option for justified

    printlog " Close dialog with 'OK'"
    TabAusrichtungAbsatz.OK

    printlog " Now the paragraph adjustment is 'justified'"
    Kontext "TextObjectbar"
    if Blocksatz.GetState(2) = 1 then
        printlog "Text is justified now" 'this is ok
    else
        warnlog "Warning: Text is not justified!" 'this should not happen
    end if
     Kontext "DocumentWriter"

    printlog " Click into the middle of a line"
    DocumentWriter.TypeKeys "<MOD1 HOME>" 'jumping to beginning of text
    DocumentWriter.TypeKeys "<MOD1 RIGHT>",9,true '9 times to the right
    sleep (1)

    printlog " [via menu]: Insert-Manual Break, select 'Line break', OK"
    DocumentWriter.UseMenu 'use menu to call insert/line break
    Call hMenuSelectNr (4) 'Insert
    Call hMenuSelectNr (1) 'Manual Break
    Kontext "UmbruchEinfuegen"
    Zeilenumbruch.Check
    UmbruchEinfuegen.OK
    printlog "Manual Linebreak inserted"
    Kontext "DocumentWriter"

    printlog " Tools-Options-OOo Writer-Compatibility"
    ToolsOptions 'calling options
    Kontext "ExtrasOptionenDLG"
    Call hToolsOptions("WRITER","COMPATIBILITY")
    Kontext "TabCompatibility"

    printlog " Check the entry 'Expand word space [...]"
    iOptionsCount = Options.GetItemCount
    Options.Select iOptionsCount
    if Options.isChecked then
        printlog "Expand word space.. is checked "
    else
        printlog "Expand word space.. is not checked."
        Options.Check
    end if

    printlog " Click 'Default'"
    Default.Click

    printlog "  In upcoming dialog 'Do you want to change the default template', click 'Yes'"
    sleep (1)
    Kontext "Active"
    if Active.Exists then
        Active.Yes 'Closing dialog clicking yes
        printlog "Changing to defaults"
    else
        warnlog "Warning: Messagebox is not up!"
    end if
    sleep (1)
    Kontext "ExtrasOptionenDLG"
    ExtrasOptionenDLG.OK 'Closing the options

    printlog " Close document"
    Call hCloseDocument

    printlog " New Writer document"
    Call hNewDocument

    printlog " Tools-Options-OOo Writer-Compatibility"
    ToolsOptions 'calling options 
    Kontext "ExtrasOptionenDLG"
    Call hToolsOptions("WRITER","COMPATIBILITY")
    Kontext "TabCompatibility"

    printlog " Verify that the entry 'Expand word space [...]' is checked"
    iOptionsCount = Options.GetItemCount
    Options.Select iOptionsCount
    if Options.isChecked then
        printlog "Expand word space on lines... is checked" ' ok
    else
        warnlog "Warning: Expand word space on lines...is not checked as default!" 'should not happen!
    end if
    Kontext "ExtrasOptionenDLG"
    ExtrasOptionenDLG.Cancel 'leaving the options

    printlog " Close document"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase
' -------------------------------------------------------
