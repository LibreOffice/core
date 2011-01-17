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
'* short description : Functional-Tests for Tools-Options-Writer-DefaultFont, Print, Table
'*
'\***********************************************************************

sub wr_o_2
    Dim temp as boolean

    Call tToolsOptionsWriterDefaultfont("BASICFONTS")
    Call tToolsOptionsWriterPrint
    ' is located in wr_o_a.inc
    Call tToolsOptionsWriterPrint1
    Call tToolsOptionsWriterTable
    Call tToolsOptionsWriterTable1
    ' is located in wr_o_a.inc
    Call tToolsOptionsWriterTableMove
    Call tToolsOptionsWriterTableInsert
    Call tToolsOptionsWriterTableEffects
    Call tToolsOptionsWriterTableEffects2
end sub

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterDefaultfont (WestOrEast as string)

    Dim bCheckboxes(1) as boolean    ' Checkbox states
    Dim iListBox(5) as integer         ' ListBox
    Dim sListBox(5) as String
    Dim iListBoxSize(5) as integer         ' ListBox
    Dim sListBoxSize(5) as String
    Dim iTempListBox(5) as integer
    Dim sTempListBox(5) as String
    Dim iTempListBoxSize(5) as integer
    Dim sTempListBoxSize(5) as String
    Dim sInitFont(5) as String
    Dim sInitFontSize(5) as String
    Dim i as integer
    Dim x as integer
    Dim sFileName as string
    Dim sTempS as String

    '/// Got called two times ///'
    '///+ 1. for the western- ///'
    '///+ 2. for the asian    font page ///'

    Call hNewDocument
    ' Remember the init-font directly after installation
    ToolsOptions
    Call hToolsOptions ("WRITER",WestOrEast)
    Standard.Click
    sInitFont(1) = StandardSchrift.GetSelText
    sInitFont(2) = Ueberschrift.GetSelText
    sInitFont(3) = Liste.GetSelText
    sInitFont(4) = Beschriftung.GetSelText
    sInitFont(5) = Verzeichnis.GetSelText
    sInitFontSize(1) = StandardSchriftGroesse.GetSelText
    sInitFontSize(2) = UeberschriftGroesse.GetSelText
    sInitFontSize(3) = ListeGroesse.GetSelText
    sInitFontSize(4) = BeschriftungGroesse.GetSelText
    sInitFontSize(5) = VerzeichnisGroesse.GetSelText
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)

    if (Ucase(WestOrEast) = "BASICFONTSASIAN") then
        printlog "------------ ASIAN FONTS -------------"
        warnlog "TBO: don't know how it get's handeld, so don't pay ANY attention to warnlogs :-)"
        sFileName = gTesttoolPath + "writer\optional\input\options\options2.sxw"
    else
        printlog "------------ WESTERN FONTS -------------"
        Call ActiveDeactivateAsianSupport(False)
        sFileName = gTesttoolPath + "writer\optional\input\options\options2.sxw"
    end if

    '/// use a document to check the font with styles///'
    '///+ use 'input\\writer\\optional\\options\\options2.sxw' ///
    hFileOpenLocally(  sFileName )
    
    printlog "- Tools/Options/Writer: DefaultFont"

    printlog ("/// - save states ///")
    ToolsOptions
    Call hToolsOptions ("WRITER",WestOrEast)

    bCheckboxes(1) = AktuellesDokument.IsChecked
    iListBox(1) = StandardSchrift.GetSelIndex
    iListBox(2) = Ueberschrift.GetSelIndex
    iListBox(3) = Liste.GetSelIndex
    iListBox(4) = Beschriftung.GetSelIndex
    iListBox(5) = Verzeichnis.GetSelIndex
    sListBox(1) = StandardSchrift.GetSelText
    sListBox(2) = Ueberschrift.GetSelText
    sListBox(3) = Liste.GetSelText
    sListBox(4) = Beschriftung.GetSelText
    sListBox(5) = Verzeichnis.GetSelText

    iListBoxSize(1) = StandardSchriftGroesse.GetSelIndex
    iListBoxSize(2) = UeberschriftGroesse.GetSelIndex
    iListBoxSize(3) = ListeGroesse.GetSelIndex
    iListBoxSize(4) = BeschriftungGroesse.GetSelIndex
    iListBoxSize(5) = VerzeichnisGroesse.GetSelIndex
    sListBoxSize(1) = StandardSchriftGroesse.GetSelText
    sListBoxSize(2) = UeberschriftGroesse.GetSelText
    sListBoxSize(3) = ListeGroesse.GetSelText
    sListBoxSize(4) = BeschriftungGroesse.GetSelText
    sListBoxSize(5) = VerzeichnisGroesse.GetSelText

    printlog ("'/// - all states inverting ///")

    AktuellesDokument.UnCheck
    if ( StandardSchrift.GetItemCount > iListBox(1) ) then StandardSchrift.Select(iListBox(1) + 2) else StandardSchrift.Select(iListBox(1)-1)
    if ( Ueberschrift.GetItemCount    > iListBox(2) and iListBox(2) > 1 ) then Ueberschrift.Select(iListBox(2)-1) else Ueberschrift.Select(iListBox(2)+1)
    if ( Liste.GetItemCount           > iListBox(3) ) then Liste.Select          (iListBox(3) + 1) else Liste.Select(iListBox(3)-1)
    if ( Beschriftung.GetItemCount    > iListBox(4) and iListBox(4) > 1 ) then Beschriftung.Select   (iListBox(4)-1) else Beschriftung.Select(iListBox(4)+1)
    if ( Verzeichnis.GetItemCount     > iListBox(5) ) then Verzeichnis.Select    (iListBox(5) + 1) else Verzeichnis.Select(iListBox(5)-1)

    if ( StandardSchriftGroesse.GetItemCount > iListBoxSize(1) ) then StandardSchriftGroesse.Select(iListBoxSize(1) + 2) else StandardSchriftGroesse.Select(iListBox(1)-1)
    if ( UeberschriftGroesse.GetItemCount    > iListBoxSize(2) and iListBoxSize(2) > 1 ) then UeberschriftGroesse.Select(iListBoxSize(2)-1) else UeberschriftGroesse.Select(iListBoxSize(2)+1)
    if ( ListeGroesse.GetItemCount           > iListBoxSize(3) ) then ListeGroesse.Select          (iListBoxSize(3) + 1) else ListeGroesse.Select(iListBoxSize(3)-1)
    if ( BeschriftungGroesse.GetItemCount    > iListBoxSize(4) and iListBoxSize(4) > 1 ) then BeschriftungGroesse.Select   (iListBoxSize(4)-1) else BeschriftungGroesse.Select(iListBoxSize(4)+1)
    if ( VerzeichnisGroesse.GetItemCount     > iListBoxSize(5) ) then VerzeichnisGroesse.Select    (iListBoxSize(5) + 1) else VerzeichnisGroesse.Select(iListBoxSize(5)-1)

    iTempListBox(1) = StandardSchrift.GetSelIndex
    iTempListBox(2) = Ueberschrift.GetSelIndex
    iTempListBox(3) = Liste.GetSelIndex
    iTempListBox(4) = Beschriftung.GetSelIndex
    iTempListBox(5) = Verzeichnis.GetSelIndex

    sTempListBox(1) = StandardSchrift.GetSelText
    sTempListBox(2) = Ueberschrift.GetSelText
    sTempListBox(3) = Liste.GetSelText
    sTempListBox(4) = Beschriftung.GetSelText
    sTempListBox(5) = Verzeichnis.GetSelText

    iTempListBoxSize(1) = StandardSchriftGroesse.GetSelIndex
    iTempListBoxSize(2) = UeberschriftGroesse.GetSelIndex
    iTempListBoxSize(3) = ListeGroesse.GetSelIndex
    iTempListBoxSize(4) = BeschriftungGroesse.GetSelIndex
    iTempListBoxSize(5) = VerzeichnisGroesse.GetSelIndex

    sTempListBoxSize(1) = StandardSchriftGroesse.GetSelText
    sTempListBoxSize(2) = UeberschriftGroesse.GetSelText
    sTempListBoxSize(3) = ListeGroesse.GetSelText
    sTempListBoxSize(4) = BeschriftungGroesse.GetSelText
    sTempListBoxSize(5) = VerzeichnisGroesse.GetSelText

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)

    Printlog "'/// - check whether fonts get changed in doc. ///"
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Up>", 5
    Call wTypeKeys "<Home><Right>"
    sTempS = wGetCharacterFont()
    if sTempS <> sTempListBox(1) then Warnlog "Default font: state changed  is: "+sTempS+" ;should: "+sTempListBox(1)+" : "+iTempListBox(1)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFont()
    if sTempS <> sTempListBox(2) then Warnlog "Heading font: state changed  is: "+sTempS+" ;should: "+sTempListBox(2)+" : "+iTempListBox(2)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFont()
    if sTempS <> sTempListBox(3) then Warnlog "List font: state changed  is: "+sTempS+" ;should: "+sTempListBox(3)+" : "+iTempListBox(3)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFont()
    if sTempS <> sTempListBox(4) then Warnlog "Caption font: state changed  is: "+sTempS+" ;should: "+sTempListBox(4)+" : "+iTempListBox(4)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFont()
    if sTempS <> sTempListBox(5) then Warnlog "Index font: state changed  is: "+sTempS+" ;should: "+sTempListBox(5)+" : "+iTempListBox(5)

    Call wTypeKeys "<Mod1 Home><Right>"
    sTempS = wGetCharacterFontSize()
    if sTempS <> sTempListBoxSize(1) then Warnlog "Default fontsize: state changed  is: "+sTempS+" ;should: "+sTempListBoxSize(1)+" : "+iTempListBoxSize(1)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFontSize()
    if sTempS <> sTempListBoxSize(2) then Warnlog "Heading fontsize: state changed  is: "+sTempS+" ;should: "+sTempListBoxSize(2)+" : "+iTempListBoxSize(2)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFontSize()
    if sTempS <> sTempListBoxSize(3) then Warnlog "List fontsize: state changed  is: "+sTempS+" ;should: "+sTempListBoxSize(3)+" : "+iTempListBoxSize(3)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFontSize()
    if sTempS <> sTempListBoxSize(4) then Warnlog "Caption fontsize: state changed  is: "+sTempS+" ;should: "+sTempListBoxSize(4)+" : "+iTempListBoxSize(4)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFontSize()
    if sTempS <> sTempListBoxSize(5) then Warnlog "Index fontsize: state changed  is: "+sTempS+" ;should: "+sTempListBoxSize(5)+" : "+iTempListBoxSize(5)

    printlog (" - SO quit - start"   )

    Call wOfficeRestart
    WaitSlot (2)

    printlog ("'/// - checking states ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , WestOrEast )

    if ( AktuellesDokument.IsChecked = TRUE ) then WarnLog "Current document only state changed"
    if ( StandardSchrift.GetSelIndex <> iTempListBox(1) ) then WarnLog "Default font state changed"
    if ( Ueberschrift.GetSelIndex    <> iTempListBox(2) ) then WarnLog "Heading font    state changed"
    if ( Liste.GetSelIndex           <> iTempListBox(3) ) then WarnLog "List font           state changed"
    if ( Beschriftung.GetSelIndex    <> iTempListBox(4) ) then WarnLog "Caption font    state changed"
    if ( Verzeichnis.GetSelIndex     <> iTempListBox(5) ) then WarnLog "Index     state changed"

    if ( StandardSchriftGroesse.GetSelIndex <> iTempListBoxSize(1) ) then WarnLog "Default fontsize state changed"
    if ( UeberschriftGroesse.GetSelIndex    <> iTempListBoxSize(2) ) then WarnLog "Heading fontsize    state changed"
    if ( ListeGroesse.GetSelIndex           <> iTempListBoxSize(3) ) then WarnLog "List fontsize           state changed"
    if ( BeschriftungGroesse.GetSelIndex    <> iTempListBoxSize(4) ) then WarnLog "Caption fontsize    state changed"
    if ( VerzeichnisGroesse.GetSelIndex     <> iTempListBoxSize(5) ) then WarnLog "Index fontsize     state changed"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)

    Printlog "'///  - verify global settings don't affect saved Document ///"
    '///+ use input\\writer\\optional\\options\\options2.sxw ///
    hFileOpenLocally(  sFileName )
    
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Up>", 5
    Call wTypeKeys "<Home><Right>"

    sTempS = wGetCharacterFont()
    if sTempS <> sListBox(1) then Warnlog "Default font: Writer changed  Font !?! is: "+sTempS+" ;should: "+sListBox(1)+" : "+iListBox(1)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFont()
    if sTempS <> sListBox(2) then Warnlog "Heading font: Writer changed  Font !?! is: "+sTempS+" ;should: "+sListBox(2)+" : "+iListBox(2)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFont()
    if sTempS <> sListBox(3) then Warnlog "List font: Writer changed  Font !?! is: "+sTempS+" ;should: "+sListBox(3)+" : "+iListBox(3)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFont()
    if sTempS <> sListBox(4) then Warnlog "Caption font: Writer changed  Font !?! is: "+sTempS+" ;should: "+sListBox(4)+" : "+iListBox(4)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFont()
    if sTempS <> sListBox(5) then Warnlog "Index font: Writer changed  Font !?! is: "+sTempS+" ;should: "+sListBox(5)+" : "+iListBox(5)

    Call wTypeKeys "<Mod1 Home><Right>"
    sTempS = wGetCharacterFontSize()
    if sTempS <> sListBoxSize(1) then Warnlog "Default font: Writer changed  Font !?! is: "+sTempS+" ;should: "+sListBoxSize(1)+" : "+iListBoxSize(1)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFontSize()
    if sTempS <> sListBoxSize(2) then Warnlog "Heading font: Writer changed  Font !?! is: "+sTempS+" ;should: "+sListBoxSize(2)+" : "+iListBoxSize(2)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFontSize()
    if sTempS <> sListBoxSize(3) then Warnlog "List font: Writer changed  Font !?! is: "+sTempS+" ;should: "+sListBoxSize(3)+" : "+iListBoxSize(3)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFontSize()
    if sTempS <> sListBoxSize(4) then Warnlog "Caption font: Writer changed  Font !?! is: "+sTempS+" ;should: "+sListBoxSize(4)+" : "+iListBoxSize(4)
    Call wTypeKeys "<Down>"
    sTempS = wGetCharacterFontSize()
    if sTempS <> sListBoxSize(5) then Warnlog "Index font: Writer changed  Font !?! is: "+sTempS+" ;should: "+sListBoxSize(5)+" : "+iListBoxSize(5)

    Call hCloseDocument
    Call hNewDocument

    printlog ("'/// - all UnCheck -> o ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , WestOrEast )

    if AktuellesDokument.IsEnabled = true then
        AktuellesDokument.Check
    else
        ' Workraound: Sometimes checkbox is disabled...don't know why
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.Cancel
        WaitSlot (1)
        ToolsOptions
        Call hToolsOptions ( "WRITER" , WestOrEast )
        AktuellesDokument.Check
    end if

    StandardSchrift.Select(1)
    Ueberschrift.Select(1)
    Liste.Select(1)
    Beschriftung.Select(1)
    Verzeichnis.Select(1)

    StandardSchriftGroesse.Select(1)
    UeberschriftGroesse.Select(1)
    ListeGroesse.Select(1)
    BeschriftungGroesse.Select(1)
    VerzeichnisGroesse.Select(1)
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)

    printlog ("'/// - check if all UnChecked ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , WestOrEast )

    if ( AktuellesDokument.IsChecked <> True ) then WarnLog "Current document only is not checked anymore"
    if ( StandardSchrift.GetSelIndex <> 1 ) then WarnLog "Default font is not item 1"
    if ( Ueberschrift.GetSelIndex    <> 1 ) then WarnLog "Heading font    is not item 1"
    if ( Liste.GetSelIndex           <> 1 ) then WarnLog "List font           is not item 1"
    if ( Beschriftung.GetSelIndex    <> 1 ) then WarnLog "Caption font    is not item 1"
    if ( Verzeichnis.GetSelIndex     <> 1 ) then WarnLog "Index font     is not item 1"

    if ( StandardSchriftGroesse.GetSelIndex <> 1 ) then WarnLog "Default fontsize is not item 1"
    if ( UeberschriftGroesse.GetSelIndex    <> 1 ) then WarnLog "Heading fontsize    is not item 1"
    if ( ListeGroesse.GetSelIndex           <> 1 ) then WarnLog "List fontsize           is not item 1"
    if ( BeschriftungGroesse.GetSelIndex    <> 1 ) then WarnLog "Caption fontsize    is not item 1"
    if ( VerzeichnisGroesse.GetSelIndex     <> 1 ) then WarnLog "Index fontsize     is not item 1"

    Printlog ("'/// - all Check -> x ///")

    Sleep 1
    AktuellesDokument.Check
    StandardSchrift.Select(StandardSchrift.GetItemCount)
    Ueberschrift.Select   (Ueberschrift.GetItemCount   )
    Liste.Select          (Liste.GetItemCount          )
    Beschriftung.Select   (Beschriftung.GetItemCount   )
    Verzeichnis.Select    (Verzeichnis.GetItemCount    )

    StandardSchriftGroesse.Select(StandardSchriftGroesse.GetItemCount)
    UeberschriftGroesse.Select   (UeberschriftGroesse.GetItemCount   )
    ListeGroesse.Select          (ListeGroesse.GetItemCount          )
    BeschriftungGroesse.Select   (BeschriftungGroesse.GetItemCount   )
    VerzeichnisGroesse.Select    (VerzeichnisGroesse.GetItemCount    )
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)

    printlog ("'/// - check if all Checked ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , WestOrEast )

    if not AktuellesDokument.IsChecked then WarnLog "AktuellesDokument o"
    if ( StandardSchrift.GetSelIndex <> StandardSchrift.GetItemCount ) then WarnLog "Default font is not item (GetItemCount):"+StandardSchrift.GetSelIndex
    if ( Ueberschrift.GetSelIndex    <> Ueberschrift.GetItemCount    ) then WarnLog "Heading font    is not item (GetItemCount)"
    if ( Liste.GetSelIndex           <> Liste.GetItemCount           ) then WarnLog "List font           is not item (GetItemCount)"
    if ( Beschriftung.GetSelIndex    <> Beschriftung.GetItemCount    ) then WarnLog "Caption font    is not item (GetItemCount)"
    if ( Verzeichnis.GetSelIndex     <> Verzeichnis.GetItemCount     ) then WarnLog "Index font     is not item (GetItemCount)"

    if ( StandardSchriftGroesse.GetSelIndex <> StandardSchriftGroesse.GetItemCount ) then WarnLog "Default fontsize is not item (GetItemCount):"+StandardSchrift.GetSelIndex
    if ( UeberschriftGroesse.GetSelIndex    <> UeberschriftGroesse.GetItemCount    ) then WarnLog "Heading fontsize    is not item (GetItemCount)"
    if ( ListeGroesse.GetSelIndex           <> ListeGroesse.GetItemCount           ) then WarnLog "List fontsize           is not item (GetItemCount)"
    if ( BeschriftungGroesse.GetSelIndex    <> BeschriftungGroesse.GetItemCount    ) then WarnLog "Caption fontsize    is not item (GetItemCount)"
    if ( VerzeichnisGroesse.GetSelIndex     <> VerzeichnisGroesse.GetItemCount     ) then WarnLog "Index fontsize     is not item (GetItemCount)"

    printlog ("'/// - check Default Button ///")
    Standard.Click ' mal schauen, ob vorhanden...

    if ( StandardSchrift.GetSelText <> sInitFont(1) ) then WarnLog "Default not init font!"
    if ( Ueberschrift.GetSelText    <> sInitFont(2) ) then WarnLog "Heading    not init font! BUGID: 89816"
    if ( Liste.GetSelText           <> sInitFont(3) ) then WarnLog "List           not init font!"
    if ( Beschriftung.GetSelText    <> sInitFont(4) ) then WarnLog "Caption    not init font!"
    if ( Verzeichnis.GetSelText     <> sInitFont(5) ) then WarnLog "Index     not init font!"

    if ( StandardSchriftGroesse.GetSelText <> sInitFontSize(1) ) then WarnLog "Default fontsize not init font!"
    if ( UeberschriftGroesse.GetSelText    <> sInitFontSize(2) ) then WarnLog "Heading fontsize    not init font! BUGID: 89816"
    if ( ListeGroesse.GetSelText           <> sInitFontSize(3) ) then WarnLog "List fontsize           not init font!"
    if ( BeschriftungGroesse.GetSelText    <> sInitFontSize(4) ) then WarnLog "Caption fontsize    not init font!"
    if ( VerzeichnisGroesse.GetSelText     <> sInitFontSize(5) ) then WarnLog "Index fontsize     not init font!"
    AktuellesDokument.Uncheck

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok

    '/// Open a new docuent and check fonts for default
    Call hNewDocument
    ToolsOptions
    Call hToolsOptions ( "WRITER" , WestOrEast )

    if ( StandardSchrift.GetSelText <> sInitFont(1) ) then warnlog "Default not init font!"
    if ( Ueberschrift.GetSelText    <> sInitFont(2) ) then warnlog "Heading    not init font! BUGID: 89816"
    if ( Liste.GetSelText           <> sInitFont(3) ) then warnlog "List           not init font!"
    if ( Beschriftung.GetSelText    <> sInitFont(4) ) then warnlog "Caption    not init font!"
    if ( Verzeichnis.GetSelText     <> sInitFont(5) ) then warnlog "Index    not init font!"

    if ( StandardSchriftGroesse.GetSelText <> sInitFontSize(1) ) then warnlog "Default fontsize not init font!"
    if ( UeberschriftGroesse.GetSelText    <> sInitFontSize(2) ) then warnlog "Heading fontsize    not init font! BUGID: 89816"
    if ( ListeGroesse.GetSelText           <> sInitFontSize(3) ) then warnlog "List fontsize           not init font!"
    if ( BeschriftungGroesse.GetSelText    <> sInitFontSize(4) ) then warnlog "Caption fontsize    not init font!"
    if ( VerzeichnisGroesse.GetSelText     <> sInitFontSize(5) ) then warnlog "Index fontsize     not init font!"
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel
    Call hCloseDocument
    Call hNewDocument

    printlog ("'/// - restore states ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , WestOrEast )

    if ( bCheckboxes(1) = TRUE ) then  AktuellesDokument.Check else AktuellesDokument.UnCheck
    StandardSchrift.SetText (sListBox(1))
    Ueberschrift.SetText    (sListBox(2))
    Liste.SetText           (sListBox(3))
    Beschriftung.SetText    (sListBox(4))
    Verzeichnis.SetText     (sListBox(5))

    StandardSchriftGroesse.SetText (sListBoxSize(1))
    UeberschriftGroesse.SetText    (sListBoxSize(2))
    ListeGroesse.SetText           (sListBoxSize(3))
    BeschriftungGroesse.SetText    (sListBoxSize(4))
    VerzeichnisGroesse.SetText     (sListBoxSize(5))

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterPrint1
    Dim bCheckboxes(13) as boolean    ' Checkbox states
    Dim iListBox(1) as string         ' ListBox
    Dim iTempListBox(1) as string
    Dim bRadioBut(4) as boolean         ' RadioButtons
    Dim bTempRadioBut(4) as boolean
    Dim i as integer, sWriterFilter as string
    Dim WriterFilterExtension(10) as string
        WriterFilterExtension() = hGetFilternameExtension ( "writer8" )

    ' sMetricField(i) and sTempMetricField(i)
    '(1) = Grafiken
    '(2) = Tabellen
    '(4) = Kontrollfelder
    '(5) = Hintergrund
    '(6) = SchwarzDrucken
    '
    '(7) = LinkeSeiten
    '(8) = RechteSeiten
    '(10)= Prospekt
    '
    '(11)= EinzelneDruckauftraege
    '(12)= AusDruckereinstellung

    ' bRadioBut(i) and bTempRadioBut(i) / RadioButton
    '(1) = Keine
    '(2) = NurNotizen
    '(3) = Dokumentende
    '(4) = Seitenende

    '/// uses "input\\writer\\optional\\options\\options3.sxw" ///
    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\options\options3.sxw" )
    
    printlog "'///- Tools/Options/Writer: PrintWriter ///"

    printlog ("'/// - save states ///")
    ToolsOptions
    Call hToolsOptions ("WRITER","PRINT")

    bCheckboxes(1) = Grafiken.IsChecked
    bCheckboxes(2) = Kontrollfelder.IsChecked
    bCheckboxes(3) = Hintergrund.IsChecked
    bCheckboxes(4) = SchwarzDrucken.IsChecked
    bCheckboxes(5) = HiddenText.IsChecked
    bCheckboxes(6) = TextPlaceholder.IsChecked

    bCheckboxes(7) = LinkeSeiten.IsChecked
    bCheckboxes(8) = RechteSeiten.IsChecked
    bCheckboxes(9)= Prospekt.IsChecked

    bCheckboxes(10)= LeereSeitenDrucken.IsChecked
    bCheckboxes(11)= AusDruckereinstellung.IsChecked

    iListBox(1) = Fax.GetSelText

    bRadioBut(1) = Keine.IsChecked
    bRadioBut(2) = NurNotizen.IsChecked
    bRadioBut(3) = Dokumentende.IsChecked
    bRadioBut(4) = Seitenende.IsChecked

    printlog ("'/// - all states inverting ///")

    if bCheckboxes(1) then Grafiken.UnCheck             else Grafiken.Check
    if bCheckboxes(2) then Kontrollfelder.UnCheck       else Kontrollfelder.Check
    if bCheckboxes(3) then Hintergrund.UnCheck          else Hintergrund.Check
    if bCheckboxes(4) then SchwarzDrucken.UnCheck       else SchwarzDrucken.Check
    if bCheckboxes(5) then HiddenText.UnCheck           else HiddenText.Check
    if bCheckboxes(6) then TextPlaceholder.UnCheck      else TextPlaceholder.Check

    if bCheckboxes(7) then LinkeSeiten.UnCheck          else LinkeSeiten.Check
    if bCheckboxes(8) then RechteSeiten.UnCheck         else RechteSeiten.Check
    if bCheckboxes(9) then Prospekt.UnCheck            else Prospekt.Check

    if bCheckboxes(10) then LeereSeitenDrucken.UnCheck else LeereSeitenDrucken.Check
    if bCheckboxes(11) then AusDruckereinstellung.UnCheck  else AusDruckereinstellung.Check

    if Keine.IsEnabled then
        if (Keine.IsChecked = True) then
            NurNotizen.Check
        else
            Keine.Check
        endif
    endif

    bTempRadioBut(1) = Keine.IsChecked
    bTempRadioBut(2) = NurNotizen.IsChecked
    bTempRadioBut(3) = Dokumentende.IsChecked
    bTempRadioBut(4) = Seitenende.IsChecked

    try
        if (NO_REAL_PRINT = False) then FL_Drucken
        for i = 1 to 10
            Kontext "Druckbestaetigung"
            if Druckbestaetigung.Exists then
                Sleep 1
            else
                i = 100
            end if
        next i
    catch
        print " :::::::::::::::wr2::::::test d1"
    endcatch

    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            printlog Active.Gettext
            try
                Active.Ok
            catch
                Active.Yes
            endcatch
        end if
    end if
    Kontext "TabDruckenWriter"
    wait 500
    if Fax.GetSelText <> iListBox(1) then
        Fax.Select (Fax.GetItemCount)
    else
        Fax.Select(1)
    end if
    iTempListBox(1) = Fax.GetSelIndex

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)
    Call hFileSaveAsWithFilterKill(gOfficepath + "user\work\printtest." & WriterFilterExtension(0) , "writer8")

    printlog ("'/// - SO quit - start ///")

    Call wOfficeRestart

    printlog ("'/// - checking states ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "PRINT" )

    if ( bCheckboxes(1) = Grafiken.IsChecked        ) then WarnLog "Grafiken state changed"
    if ( bCheckboxes(2) = Kontrollfelder.IsChecked  ) then WarnLog "Kontrollfelder state changed"
    if ( bCheckboxes(3) = Hintergrund.IsChecked     ) then WarnLog "Hintergrund state changed"
    if ( bCheckboxes(4) = SchwarzDrucken.IsChecked  ) then WarnLog "SchwarzDrucken state changed"
    if ( bCheckboxes(5) = HiddenText.IsChecked      ) then WarnLog "HiddenText state changed"
    if ( bCheckboxes(6) = TextPlaceholder.IsChecked ) then WarnLog "TextPlaceholder state changed"

    if ( bCheckboxes(7) = LinkeSeiten.IsChecked            ) then WarnLog "LinkeSeiten state changed"
    if ( bCheckboxes(8) = RechteSeiten.IsChecked           ) then WarnLog "RechteSeiten state changed"
    if ( bCheckboxes(9)= Prospekt.IsChecked               ) then WarnLog "Prospekt state changed"

    if ( bCheckboxes(10)= LeereSeitenDrucken.IsChecked  ) then WarnLog "'Print automatically inserted blank pages' state changed"
    if ( bCheckboxes(11)= AusDruckereinstellung.IsChecked  ) then WarnLog "AusDruckereinstellung state changed"

    if ( Fax.GetSelIndex <> iTempListBox(1) ) Then warnlog "Fax state changed"

    if ( bTempRadioBut(1) <> Keine.IsChecked       ) then WarnLog "Keine state changed"
    if ( bTempRadioBut(2) <> NurNotizen.IsChecked  ) then WarnLog "NurNotizen state changed"
    if ( bTempRadioBut(3) <> Dokumentende.IsChecked) then WarnLog "Dokumentende state changed"
    if ( bTempRadioBut(4) <> Seitenende.IsChecked  ) then WarnLog "Seitenende state changed"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)
	' Notify is saved with document
    hFileOpen( gOfficePath & "user\work\printtest." & WriterFilterExtension(0) )
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "PRINT" )

    if ( bTempRadioBut(1) <> Keine.IsChecked       ) then WarnLog "Keine state changed (Not saved with document)"
    if ( bTempRadioBut(2) <> NurNotizen.IsChecked  ) then WarnLog "NurNotizen state changed (Not saved with document)"
    if ( bTempRadioBut(3) <> Dokumentende.IsChecked) then WarnLog "Dokumentende state changed (Not saved with document)"
    if ( bTempRadioBut(4) <> Seitenende.IsChecked  ) then WarnLog "Seitenende state changed (Not saved with document)"

    printlog ("'/// - all UnCheck -> o ///")

    Grafiken.UnCheck
    Kontrollfelder.UnCheck
    Hintergrund.UnCheck
    SchwarzDrucken.UnCheck
    HiddenText.UnCheck
    TextPlaceholder.UnCheck

    LinkeSeiten.UnCheck
    RechteSeiten.UnCheck
    Prospekt.UnCheck

    LeereSeitenDrucken.UnCheck
    AusDruckereinstellung.UnCheck

    if NurNotizen.IsEnabled then
        if (NurNotizen.IsChecked = True) then
            Dokumentende.Check
        else
            NurNotizen.Check
        endif
    endif

    bTempRadioBut(1) = Keine.IsChecked
    bTempRadioBut(2) = NurNotizen.IsChecked
    bTempRadioBut(3) = Dokumentende.IsChecked
    bTempRadioBut(4) = Seitenende.IsChecked

    Fax.Select(1)

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)

    printlog ("'/// - check if all UnChecked ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "PRINT" )

    if Grafiken.IsChecked           then WarnLog "Grafiken is checked"
    if Kontrollfelder.IsChecked     then WarnLog "Kontrollfelder is checked"
    if Hintergrund.IsChecked        then WarnLog "Hintergrund is checked"
    if SchwarzDrucken.IsChecked     then WarnLog "SchwarzDrucken is checked"
    if HiddenText.IsChecked         then WarnLog "HiddenText is checked"
    if TextPlaceholder.IsChecked    then WarnLog "TextPlaceholder is checked"

    if LinkeSeiten.IsChecked    then WarnLog "LinkeSeiten is checked"
    if RechteSeiten.IsChecked   then WarnLog "RechteSeiten is checked"
    if Prospekt.IsChecked       then WarnLog "Prospekt is checked"

    if LeereSeitenDrucken.IsChecked then WarnLog "LeereSeitenDrucken is checked"
    if AusDruckereinstellung.IsChecked  then WarnLog "AusDruckereinstellung is checked"

    if ( bTempRadioBut(1) <> Keine.IsChecked       ) then WarnLog "Keine state changed"
    if ( bTempRadioBut(2) <> NurNotizen.IsChecked  ) then WarnLog "NurNotizen state changed"
    if ( bTempRadioBut(3) <> Dokumentende.IsChecked) then WarnLog "Dokumentende state changed"
    if ( bTempRadioBut(4) <> Seitenende.IsChecked  ) then WarnLog "Seitenende state changed"

    if ( Fax.GetSelIndex <> 1 ) then WarnLog "Fax is not item 1"

    Printlog ("'/// - all Check -> x ///")

    Grafiken.Check
    Kontrollfelder.Check
    Hintergrund.Check
    SchwarzDrucken.Check
    HiddenText.Check
    TextPlaceholder.Check

    LinkeSeiten.Check
    RechteSeiten.Check
    Prospekt.Check

    LeereSeitenDrucken.Check
    AusDruckereinstellung.Check

    if Dokumentende.IsEnabled then
        if (Dokumentende.IsChecked = True) then
            Seitenende.Check
        else
            Dokumentende.Check
        endif
    endif

    bTempRadioBut(1) = Keine.IsChecked
    bTempRadioBut(2) = NurNotizen.IsChecked
    bTempRadioBut(3) = Dokumentende.IsChecked
    bTempRadioBut(4) = Seitenende.IsChecked

    Fax.Select(Fax.GetItemCount)

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)

    printlog ("'/// - check if all Checked ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "PRINT" )

    if not Grafiken.IsChecked           then WarnLog "Grafiken is unchecked"
    if not Kontrollfelder.IsChecked     then WarnLog "Kontrollfelder is unchecked"
    if not Hintergrund.IsChecked        then WarnLog "Hintergrund is unchecked"
    if not SchwarzDrucken.IsChecked     then WarnLog "SchwarzDrucken is unchecked"
    if not HiddenText.IsChecked         then WarnLog "HiddenText is unchecked "
    if not TextPlaceholder.IsChecked    then WarnLog "TextPlaceholder is unchecked"

    if not LinkeSeiten.IsChecked             then WarnLog "LinkeSeiten is unchecked"
    if not RechteSeiten.IsChecked            then WarnLog "RechteSeiten is unchecked"
    if not Prospekt.IsChecked                then WarnLog "Prospekt is unchecked"

    if not LeereSeitenDrucken.IsChecked  then WarnLog "LeereSeitenDrucken is unchecked"
    if not AusDruckereinstellung.IsChecked   then WarnLog "AusDruckereinstellung is unchecked"

    ' This state will be saved with the document
    if ( bTempRadioBut(1) <> Keine.IsChecked       ) then WarnLog "Keine state changed"
    if ( bTempRadioBut(2) <> NurNotizen.IsChecked  ) then WarnLog "NurNotizen state changed"
    if ( bTempRadioBut(3) <> Dokumentende.IsChecked) then WarnLog "Dokumentende state changed"
    if ( bTempRadioBut(4) <> Seitenende.IsChecked  ) then WarnLog "Seitenende state changed"

    if ( Fax.GetSelIndex <> Fax.GetItemCount ) then WarnLog "Fax is not item (GetItemCount)"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel

    printlog ("'/// - restore states ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "PRINT" )

    if ( bCheckboxes(1) = TRUE ) then Grafiken.Check else Grafiken.Uncheck
    if ( bCheckboxes(2) = TRUE ) then Kontrollfelder.Check else Kontrollfelder.UnCheck
    if ( bCheckboxes(3) = TRUE ) then Hintergrund.Check else Hintergrund.Uncheck
    if ( bCheckboxes(4) = TRUE ) then SchwarzDrucken.Check else  SchwarzDrucken.UnCheck
    if ( bCheckboxes(5) = TRUE ) then HiddenText.check else HiddenText.Uncheck
    if ( bCheckboxes(6) = TRUE ) then TextPlaceholder.check else TextPlaceholder.Uncheck

    if ( bCheckboxes(7) = TRUE ) then LinkeSeiten.Check else LinkeSeiten.Uncheck
    if ( bCheckboxes(8) = TRUE ) then RechteSeiten.Check else RechteSeiten.Uncheck
    if ( bCheckboxes(9) = TRUE ) then Prospekt.Check else Prospekt.Uncheck

    if ( bCheckboxes(10) = TRUE ) then LeereSeitenDrucken.Check else LeereSeitenDrucken.UnCheck
    if ( bCheckboxes(11) = TRUE ) then AusDruckereinstellung.Check else AusDruckereinstellung.UnCheck

    try
        Fax.Select (iListBox(1))
    catch
        warnlog "restore fax doesn't work"
    endcatch

    if ( bRadioBut(1) = TRUE ) then Keine.Check
    if ( bRadioBut(2) = TRUE ) then NurNotizen.Check
    if ( bRadioBut(3) = TRUE ) then Dokumentende.Check
    if ( bRadioBut(4) = TRUE ) then Seitenende.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterTable
    Dim sMetricField(4) as string      ' MetricField
    Dim sTempMetricField(4) as string
    Dim bRadioBut(3) as boolean    ' RadioButton states
    Dim bTempRadioBut(3) as boolean

    ' sMetricField(i) and sTempMetricField(i)
    '(1) = VerschiebenZeile
    '(2) = VerschiebenSpalte
    '(3) = EinfuegenZeile
    '(4) = EinfuegenSpalte

    ' bRadioBut(i) and bTempRadioBut(i)
    '(1) = VerhaltenFix
    '(2) = FixProportional
    '(3) = Variabel

    Call hNewDocument
    printlog ("'///- Tools/Options/Writer: Table ///")

    printlog ("'/// - save states ///")
    ToolsOptions
    Call hToolsOptions ("WRITER","Table")

    sMetricField(1) = VerschiebenZeile.GetText
    sMetricField(2) = VerschiebenSpalte.GetText
    sMetricField(3) = EinfuegenZeile.GetText
    sMetricField(4) = EinfuegenSpalte.GetText

    bRadioBut(1) = VerhaltenFix.IsChecked
    bRadioBut(2) = FixProportional.IsChecked
    bRadioBut(3) = Variabel.IsChecked

    if ((bRadioBut(1) Xor bRadioBut(2) Xor bRadioBut(3)) = False) then Warnlog (" RadioButtons have NO init state: " + bRadioBut(1) +", BugID: 83097")

    printlog ("'/// - all states inverting ///")

    VerschiebenZeile.ToMax
    VerschiebenSpalte.ToMax
    EinfuegenZeile.ToMax
    EinfuegenSpalte.ToMax
    if ( VerschiebenZeile.GetText  = sMetricField(1) ) then VerschiebenZeile.ToMin
    if ( VerschiebenSpalte.GetText = sMetricField(2) ) then VerschiebenSpalte.ToMin
    if ( EinfuegenZeile.GetText    = sMetricField(3) ) then EinfuegenZeile.ToMin
    if ( EinfuegenSpalte.GetText   = sMetricField(4) ) then EinfuegenSpalte.ToMin
    sTempMetricField(1) = VerschiebenZeile.GetText
    sTempMetricField(2) = VerschiebenSpalte.GetText
    sTempMetricField(3) = EinfuegenZeile.GetText
    sTempMetricField(4) = EinfuegenSpalte.GetText

    if (VerhaltenFix.IsChecked = True) then FixProportional.Check else VerhaltenFix.Check

    bTempRadioBut(1) = VerhaltenFix.IsChecked
    bTempRadioBut(2) = FixProportional.IsChecked
    bTempRadioBut(3) = Variabel.IsChecked

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    printlog ("'/// - SO quit - start ///")

    Call wOfficeRestart

    printlog ("'/// - checking states ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "Table" )

    if ( VerschiebenZeile.GetText  <> sTempMetricField(1) ) then WarnLog "VerschiebenZeile state changed BugID: 82990 "
    if ( VerschiebenSpalte.GetText <> sTempMetricField(2) ) then WarnLog "VerschiebenSpalte state changed"
    if ( EinfuegenZeile.GetText    <> sTempMetricField(3) ) then WarnLog "EinfuegenZeile state changed"
    if ( EinfuegenSpalte.GetText   <> sTempMetricField(4) ) then WarnLog "EinfuegenSpalte state changed"

    if ( bTempRadioBut(1) <> VerhaltenFix.IsChecked   ) then WarnLog "VerhaltenFix state changed"
    if ( bTempRadioBut(2) <> FixProportional.IsChecked) then WarnLog "FixProportional state changed"
    if ( bTempRadioBut(3) <> Variabel.IsChecked       ) then WarnLog "Variabel state changed"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)

    printlog ("'/// - all UnCheck -> o ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "Table" )

    VerschiebenZeile.ToMin
    VerschiebenSpalte.ToMin
    EinfuegenZeile.ToMin
    EinfuegenSpalte.ToMin
    sTempMetricField(1) = VerschiebenZeile.GetText
    sTempMetricField(2) = VerschiebenSpalte.GetText
    sTempMetricField(3) = EinfuegenZeile.GetText
    sTempMetricField(4) = EinfuegenSpalte.GetText
    printlog ("'/// More test ///'")
    VerschiebenZeile.More
    VerschiebenSpalte.More
    EinfuegenZeile.More
    EinfuegenSpalte.More

    if ( sTempMetricField(1) = VerschiebenZeile.GetText  ) then WarnLog "VerschiebenZeile.  More-Button not working"
    if ( sTempMetricField(2) = VerschiebenSpalte.GetText ) then WarnLog "VerschiebenSpalte. More-Button not working"
    if ( sTempMetricField(3) = EinfuegenZeile.GetText    ) then WarnLog "EinfuegenZeile.     More-Button not working"
    if ( sTempMetricField(4) = EinfuegenSpalte.GetText   ) then WarnLog "EinfuegenSpalte.    More-Button not working"

    sTempMetricField(1) = VerschiebenZeile.GetText
    sTempMetricField(2) = VerschiebenSpalte.GetText
    sTempMetricField(3) = EinfuegenZeile.GetText
    sTempMetricField(4) = EinfuegenSpalte.GetText

    if (FixProportional.IsChecked = True) then Variabel.Check else FixProportional.Check
    bTempRadioBut(1) = VerhaltenFix.IsChecked
    bTempRadioBut(2) = FixProportional.IsChecked
    bTempRadioBut(3) = Variabel.IsChecked

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)

    printlog ("'/// - check if all UnChecked ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "Table" )

    if ( VerschiebenZeile.GetText  <> sTempMetricField(1) ) then WarnLog "VerschiebenZeile is not min."
    if ( VerschiebenSpalte.GetText <> sTempMetricField(2) ) then WarnLog "VerschiebenSpalte is not min."
    if ( EinfuegenZeile.GetText    <> sTempMetricField(3) ) then WarnLog "EinfuegenZeile is not min."
    if ( EinfuegenSpalte.GetText   <> sTempMetricField(4) ) then WarnLog "EinfuegenSpalte is not min."

    if ( bTempRadioBut(1) <> VerhaltenFix.IsChecked   ) then WarnLog "VerhaltenFix state changed"
    if ( bTempRadioBut(2) <> FixProportional.IsChecked) then WarnLog "FixProportional state changed"
    if ( bTempRadioBut(3) <> Variabel.IsChecked       ) then WarnLog "Variabel state changed"

    Printlog ("'/// - all Check -> x ///")

    VerschiebenZeile.ToMax
    VerschiebenSpalte.ToMax
    EinfuegenZeile.ToMax
    EinfuegenSpalte.ToMax
    sTempMetricField(1) = VerschiebenZeile.GetText
    sTempMetricField(2) = VerschiebenSpalte.GetText
    sTempMetricField(3) = EinfuegenZeile.GetText
    sTempMetricField(4) = EinfuegenSpalte.GetText
    printlog ("'/// Less test ///'")
    VerschiebenZeile.Less
    VerschiebenSpalte.Less
    EinfuegenZeile.Less
    EinfuegenSpalte.Less
    if ( sTempMetricField(1) = VerschiebenZeile.GetText  ) then WarnLog "VerschiebenZeile.  Less-Button not working"
    if ( sTempMetricField(2) = VerschiebenSpalte.GetText ) then WarnLog "VerschiebenSpalte. Less-Button not working"
    if ( sTempMetricField(3) = EinfuegenZeile.GetText    ) then WarnLog "EinfuegenZeile.     Less-Button not working"
    if ( sTempMetricField(4) = EinfuegenSpalte.GetText   ) then WarnLog "EinfuegenSpalte.    Less-Button not working"
    sTempMetricField(1) = VerschiebenZeile.GetText
    sTempMetricField(2) = VerschiebenSpalte.GetText
    sTempMetricField(3) = EinfuegenZeile.GetText
    sTempMetricField(4) = EinfuegenSpalte.GetText

    if (Variabel.IsChecked = True) then VerhaltenFix.Check else Variabel.Check
    bTempRadioBut(1) = VerhaltenFix.IsChecked
    bTempRadioBut(2) = FixProportional.IsChecked
    bTempRadioBut(3) = Variabel.IsChecked

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)

    printlog ("'/// - check if all Checked ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "Table" )

    if ( VerschiebenZeile.GetText  <> sTempMetricField(1) ) then WarnLog "VerschiebenZeile is not max."
    if ( VerschiebenSpalte.GetText <> sTempMetricField(2) ) then WarnLog "VerschiebenSpalte is not max."
    if ( EinfuegenZeile.GetText    <> sTempMetricField(3) ) then WarnLog "EinfuegenZeile is not max."
    if ( EinfuegenSpalte.GetText   <> sTempMetricField(4) ) then WarnLog "EinfuegenSpalte is not max."

    if ( bTempRadioBut(1) <> VerhaltenFix.IsChecked   ) then WarnLog "VerhaltenFix state changed"
    if ( bTempRadioBut(2) <> FixProportional.IsChecked) then WarnLog "FixProportional state changed"
    if ( bTempRadioBut(3) <> Variabel.IsChecked       ) then WarnLog "Variabel state changed"

    printlog ("'/// - restore states ///")

    VerschiebenZeile.SetText  (sMetricField(1))
    VerschiebenSpalte.SetText (sMetricField(2))
    EinfuegenZeile.SetText    (sMetricField(3))
    EinfuegenSpalte.SetText   (sMetricField(4))

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterTable1

    '/// There was a bug from OpenOffice where the Office crashed ///
    '/// This testcase check for this bug ///

    Call hNewDocument
    printlog ("'///- Tools/Options/HTML: Table ///")
    ToolsOptions
    Call hToolsOptions ("HTML","Table")
    '/// Check fixed and choose 'OK' ///
    VerhaltenFix.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    WaitSlot (2)

    '/// ///
    ToolsOptions
    Call hToolsOptions ("HTML","Table")
    '/// Reset radiobutton to 'Variable' in Options ///
    Variabel.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase
'------------------------------------------------------------------------------
