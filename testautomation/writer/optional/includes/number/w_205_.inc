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
'* short description : Testing Number and Bullets
'*
'\***********************************************************************

sub w_205_

    Call tNumberingBullets_1
    Call tFormatNumberingBullets
    Call tFormatNumberingPosition
    Call tFormatNumberingOptions
    Call tFormatNumberingAll

end sub

testcase tNumberingBullets_1

    printlog "Turn several selected paragraphs into numbered list"
	'/// Open a new text document
    Call hNewDocument()
    '/// Insert 3 times dummy text
    Call wBlindTextEinfuegen()
    Call wBlindTextEinfuegen()
    Call wBlindTextEinfuegen()
    '/// Select all inserted text 
    call wTypeKeys "<shift mod1 home>"

    '/// Choose 'Numbering' on 'Objectbar'
    Kontext "TextObjectbar"
    try
        Numerierung.Click
	catch
      	warnlog "Objectbar is not accessible for numbering bullets"
	endcatch
    '/// Check if numbering has been created
    if (hIsNumberingBullets () <> True) then
        Warnlog " - Seems numbering has not been created"
    else
      	printlog " - Numbering has been created"
	end if

    '/// Turn numbereing off via Toolbar
    Printlog "Turn Numbering off"
    OL_Writer_Numerierung
    '/// Check if numbering has been turned off
    if (hIsNumberingBullets () <> False) then
        warnlog " - Seems numbering is still available"
    else
        printlog " - Numbering has been turned off"
    end if

    Printlog "'/// Restart counting via Format->Paragraph->Numbering ///"
    Call hFormatParagraphNumbering ("",1,-1)
    if (hIsNumberingBullets () <> True) then
      	Warnlog "Seems numbering has not been created"
    else
      printlog "Numbering has been created"
    end if

    Printlog "'/// +O+ restart numbering enable///"
    ' checked 'restart numbering' on tabpage toggles icon at Objectbar
    Kontext "NumObjectbar"
    if Not NumObjectBar.Exists then Call hToolbarSelect ( "BULLETSANDNUMBERING", true )
    
    ' Button in objectbar has to be checked
    if NeuStarten.GetState(2) <> 1 then
        warnlog "Button 'Restart Numbering' not pressed!"
        NeuStarten.Click
        sleep (1)
        if NeuStarten.GetState(2) <> 1 then
            warnlog "doesn't work"
        else
            printlog "     --- worked :-)"
        end if
    else
        ' Undo 'restart numbering'
        Printlog "'                          disable"
        NeuStarten.Click
        sleep (1)
        if NeuStarten.GetState(2) <> 0 then
            warnlog "doesn't work "
        else
            printlog "     --- worked :-)"
        end if
    end if
    
    Call hCloseDocument

endcase

'--------------------------------------------------

testcase tFormatNumberingBullets
	Dim i as Integer :
	PrintLog "- Format / Numbering and Bullets"
	
	'/// New document ///
	Call hNewDocument
	
	printlog "  - All bullet entries"
	'/// Insert all bullet entries ///
	for i=1 to 8
		FormatNumberingBullets
		Kontext
		Active.SetPage TabBullet
		Kontext "TabBullet"
		Auswahl.TypeKeys "<Right>", i
		TabBullet.OK
		call wTypeKeys "Bullet <Return><Return>"
	next i
	
	printlog "  - All Numbering entries"
	'/// Insert all numbering entries ///
	for i=1 to 8
		FormatNumberingBullets
		Kontext
		Active.SetPage TabNumerierungsart
		Kontext "TabNumerierungsart"
		Auswahl.TypeKeys "<Right>", i
		TabNumerierungsart.OK
		call wTypeKeys "Nummerierungsart <Return><Return>"
		Sleep 1
	next i
	
	printlog "  - All entries in Level"
	'/// Insert all level entries ///
	for i=1 to 8
		FormatNumberingBullets
		Kontext
		Active.SetPage TabGliederung
		Kontext "TabGliederung"
		Auswahl.TypeKeys "<Right>", i
		TabGliederung.OK
		call wTypeKeys "Gliederung <Return><Return>"
		Sleep 1
	next i
	
	printlog "  - All entries in graphic"
	'/// Insert all graphic entries ///
	for i=1 to 29
		FormatNumberingBullets
		Kontext
		Active.SetPage TabGrafiken
		Kontext "TabGrafiken"
		wait 500
		Auswahl.TypeKeys "<Right>", i
		TabGrafiken.OK
		call wTypeKeys "Grafik <Return><Return>"
		Sleep 1
	next i
	
	printlog "  - Save document and reload"
	'/// Save document, close and reopen previous saved document ///
	Call hFileSaveAsKill ( gOfficePath + "user\work\format4." & WriterFilterExtension(0) )
	Sleep 2
	Call hCloseDocument
	Sleep 2
	hFileOpen( gOfficePath & "user\work\format4." & WriterFilterExtension(0) )
	Sleep 2
	'/// Close document ///
	Call hCloseDocument
endcase

'--------------------------------------------------
testcase tFormatNumberingPosition
    Dim V1 : Dim V2 :  Dim V3 : Dim V4 :  Dim V5 : Dim V6 :  Dim V7 : Dim V8 : Dim V9 : Dim V10
    Dim V11 : Dim V12 :  Dim V13 : Dim V14 :  Dim V15 : Dim V16

    printlog "  - adjust position of Numbering"
    '/// New document ///
    Call hNewDocument
    '/// Select Format / Numbering Bullets and adjust position for each level ///
    FormatNumberingBullets
    Kontext
    Active.SetPage TabPositionNumerierung
    Kontext "TabPositionNumerierung"
    Level.Select 1
    Alignment.Select 2         ' Zentriert
'    if Relativ.IsEnabled then Warnlog "Relativ is enabled with level1!"
    AlignedAt.More 5
    V1 = AlignedAt.GetText
    IndentAt.More 1
    V2 = IndentAt.GetText
    Level.Select 3
    Alignment.Select 3         ' Rechts
'    Relativ.Check
    AlignedAt.Less 5
    V3 = AlignedAt.GetText
'    AbstandTextNumerierung.More 2
'    V4 = AbstandTextNumerierung.GetText
    Level.Select 4
    Alignment.Select 1         ' Links
'    Relativ.Check
    V5 = AlignedAt.GetText
 '   V6 = AbstandTextNumerierung.GetText
    V7 = IndentAt.GetText
    Level.Select 6
    Alignment.Select 3         ' Rechts
'    Relativ.Check
    AlignedAt.More 4
    V8 = AlignedAt.GetText
'    AbstandTextNumerierung.Less 3
'    V9 = AbstandTextNumerierung.GetText
    IndentAt.Less 3
    V10 = IndentAt.GetText
    Level.Select 9
    Alignment.Select 1
'    Relativ.Check
    AlignedAt.Less 2
    V11 = AlignedAt.GetText
'    AbstandTextNumerierung.More 2
'    V12 = AbstandTextNumerierung.GetText
    IndentAt.More 3
    V13 = IndentAt.GetText
    Level.Select 10
    Alignment.Select 3
'    Relativ.Check
    AlignedAt.More 4
    V14 = AlignedAt.GetText
'    AbstandTextNumerierung.Less 3
'    V15 = AbstandTextNumerierung.GetText
    IndentAt.Less 3
    V16 = IndentAt.GetText
    TabPositionNumerierung.OK

    printlog "  - write down numbering"
    '/// write down the numbering ///
    call wTypeKeys "Nummerierung 1. Ebene<Return>"
    call wTypeKeys "<Tab>Nummerierung 2. Ebene<Return>"
    call wTypeKeys "<Tab>Nummerierung 3. Ebene<Return>"
    call wTypeKeys "<Tab>Nummerierung 4. Ebene<Return>"
    call wTypeKeys "<Tab>Nummerierung 5. Ebene<Return>"
    call wTypeKeys "<Tab>Nummerierung 6. Ebene<Return>"
    call wTypeKeys "<Tab>Nummerierung 7. Ebene<Return>"
    call wTypeKeys "<Tab>Nummerierung 8. Ebene<Return>"
    call wTypeKeys "<Tab>Nummerierung 9. Ebene<Return>"
    call wTypeKeys "<Tab>Nummerierung  10. Ebene<Return>"
    call wTypeKeys "<Tab>Immer noch 10. Ebene<Return>"
    call wTypeKeys "<Return><Return>Ende der Nummerierung"
    call wTypeKeys "<Up>", 5

    printlog "  - check numbering-position"
    '/// reopen Format / NumberingBullets and check settings made ///
    FormatNumberingBullets
    Kontext
    Active.SetPage TabPositionNumerierung
    Kontext "TabPositionNumerierung"
    Level.Select 1
'    if Alignment.GetSelIndex <> 2 then Warnlog "Level 1: Wrong alignment:  Should = 2   Ist = "+ Alignment.GetSelIndex
    if V1 <> AlignedAt.GetText then Warnlog "Level 1: Wrong Text <-> Rand:  Should = "+ V1+ "  Is = "+ AlignedAt.GetText
    if V2 <> IndentAt.GetText then Warnlog "Level 1: Wrong indent:  Should = "+ V2+ "  Is = "+ IndentAt.GetText
    Level.Select 3
'    if Not Relativ.IsChecked then Warnlog "Relativ is checked in Level 3!"
    if Alignment.GetSelIndex <> 3 then Warnlog "Level 3: Wrong alignment:  Should = 3   Is = "+ Alignment.GetSelIndex
    if V3 <> AlignedAt.GetText then Warnlog "Level 3: Wrong Text <-> Spacing to text:  Should = "+ V3+ "  Is = "+ AlignedAt.GetText
'    if V4 <> AbstandTextNumerierung.GetText then Warnlog "Level 3: Wrong indent:  Should = "+ V4+ "  Is = "+ AbstandTextNumerierung.GetText
    Level.Select 4
'    if Not Relativ.IsChecked then Warnlog "Relativ is checked in Level 4!"
    if Alignment.GetSelIndex <> 1 then Warnlog "Level 4: Wrong alignment:  Should = 1   Is = "+ Alignment.GetSelIndex
    if V5 <> AlignedAt.GetText then Warnlog "Level 4: Wrong Text <-> Spacing to text:  Should = "+ V5+ "  Is = "+ AlignedAt.GetText
'    if V6 <> AbstandTextNumerierung.GetText then Warnlog "Level 4: Wrong indent:  Should = "+ V6+ "  Is = "+ AbstandTextNumerierung.GetText
    if V7 <> IndentAt.GetText then Warnlog "Level 4: Wrong indent:  Should = "+ V7+ "  Is = "+ IndentAt.GetText
    Level.Select 6
'    if Not Relativ.IsChecked then Warnlog "Relativ is checked in Level 6!"
    if Alignment.GetSelIndex <> 3 then Warnlog "Level 6: Wrong alignment:  Should = 3   Is = "+ Alignment.GetSelIndex
    if V8 <> AlignedAt.GetText then Warnlog "Level 6: Wrong Text <-> Spacing to text:  Should = "+ V8+ "  Is = "+ AlignedAt.GetText
'    if V9 <> AbstandTextNumerierung.GetText then Warnlog "Level 6: Wrong indent:  Should = "+ V9+ "  Is = "+ AbstandTextNumerierung.GetText
    if V10 <> IndentAt.GetText then Warnlog "Level 6: Wrong indent:  Should = "+ V10+ "  Is = "+ IndentAt.GetText
    Level.Select 9
'    if Not Relativ.IsChecked then Warnlog "Relativ is checked in Level 9!"
    if Alignment.GetSelIndex <> 1 then Warnlog "Level 9: Wrong alignment:  Should = 1   Is = "+ Alignment.GetSelIndex
    if V11 <> AlignedAt.GetText then Warnlog "Level 9: Wrong Text <-> Spacing to text:  Should = "+ V11+ "  Is = "+ AlignedAt.GetText
'    if V12 <> AbstandTextNumerierung.GetText then Warnlog "Level 9: Wrong indent:  Should = "+ V12+ "  Is = "+ AbstandTextNumerierung.GetText
    if V13 <> IndentAt.GetText then Warnlog "Level 9: Wrong indent:  Should = "+ V13+ "  Is = "+ IndentAt.GetText
    Level.Select 10
'    if Not Relativ.IsChecked then Warnlog "Relativ is checked in Level 10!"
    if Alignment.GetSelIndex <> 3 then Warnlog "Level 10: Wrong alignment:  Should = 3   Is = "+ Alignment.GetSelIndex
    if V14 <> AlignedAt.GetText then Warnlog "Level 10: Wrong Text <-> Spacing to text:  Should = "+ V14+ "  Is = "+ AlignedAt.GetText
'    if V15 <> AbstandTextNumerierung.GetText then Warnlog "Level 10: Wrong indent:  Should = "+ V15+ "  Is = "+ AbstandTextNumerierung.GetText
    if V16 <> IndentAt.GetText then Warnlog "Level 10: Wrong indent:  Should = "+ V16+ "  Is = "+ IndentAt.GetText
    TabPositionNumerierung.Ok

    printlog "  - check previous saved numbering"
    '/// save document, close it and reload previous saved document ///
    Sleep 1
    Call hFileSaveAsKill ( gOfficePath + "user\work\tFormatNumberingPosition." & WriterFilterExtension(0) )
    Sleep 2
    Call hCloseDocument
    Sleep 2
    hFileOpen( gOfficePath & "user\work\tFormatNumberingPosition." & WriterFilterExtension(0) )

    '/// check settings on Format / NumberingBullets ///
    FormatNumberingBullets
    Kontext
    Active.SetPage TabPositionNumerierung
    Kontext "TabPositionNumerierung"
    Level.Select 1
    
    if Alignment.GetSelIndex <> 2 then Warnlog "Level 1: Wrong alignment:  Should = 2   Ist = "+ Alignment.GetSelIndex
    if V1 <> AlignedAt.GetText then Warnlog "Level 1: Wrong Text <-> Rand:  Should = "+ V1+ "  Is = "+ AlignedAt.GetText
    if V2 <> IndentAt.GetText then Warnlog "Level 1: Wrong indent:  Should = "+ V2+ "  Is = "+ IndentAt.GetText
    Level.Select 3
'    if Not Relativ.IsChecked then Relativ.Check
    if Alignment.GetSelIndex <> 3 then Warnlog "Level 3: Wrong alignment:  Should = 3   Is = "+ Alignment.GetSelIndex
    if V3 <> AlignedAt.GetText then Warnlog "Level 3: Wrong Text <-> Spacing to text:  Should = "+ V3+ "  Is = "+ AlignedAt.GetText
'    if V4 <> AbstandTextNumerierung.GetText then Warnlog "Level 3: Wrong indent:  Should = "+ V4+ "  Is = "+ AbstandTextNumerierung.GetText
    Level.Select 4
'    if Not Relativ.IsChecked then Relativ.Check
    if Alignment.GetSelIndex <> 1 then Warnlog "Level 4: Wrong alignment:  Should = 1   Is = "+ Alignment.GetSelIndex
    if V5 <> AlignedAt.GetText then Warnlog "Level 4: Wrong Text <-> Spacing to text:  Should = "+ V5+ "  Is = "+ AlignedAt.GetText
'    if V6 <> AbstandTextNumerierung.GetText then Warnlog "Level 4: Wrong indent:  Should = "+ V6+ "  Is = "+ AbstandTextNumerierung.GetText
    if V7 <> IndentAt.GetText then Warnlog "Level 4: Wrong indent:  Should = "+ V7+ "  Is = "+ IndentAt.GetText
    Level.Select 6
'    if Not Relativ.IsChecked then Relativ.Check
    if Alignment.GetSelIndex <> 3 then Warnlog "Level 6: Wrong alignment:  Should = 3   Is = "+ Alignment.GetSelIndex
    if V8 <> AlignedAt.GetText then Warnlog "Level 6: Wrong Text <-> Spacing to text:  Should = "+ V8+ "  Is = "+ AlignedAt.GetText
'    if V9 <> AbstandTextNumerierung.GetText then Warnlog "Level 6: Wrong indent:  Should = "+ V9+ "  Is = "+ AbstandTextNumerierung.GetText
    if V10 <> IndentAt.GetText then Warnlog "Level 6: Wrong indent:  Should = "+ V10+ "  Is = "+ IndentAt.GetText
    Level.Select 9
'    if Not Relativ.IsChecked then Relativ.Check
    if Alignment.GetSelIndex <> 1 then Warnlog "Level 9: Wrong alignment:  Should = 1   Is = "+ Alignment.GetSelIndex
    if V11 <> AlignedAt.GetText then Warnlog "Level 9: Wrong Text <-> Spacing to text:  Should = "+ V11+ "  Is = "+ AlignedAt.GetText
'    if V12 <> AbstandTextNumerierung.GetText then Warnlog "Level 9: Wrong indent:  Should = "+ V12+ "  Is = "+ AbstandTextNumerierung.GetText
    if V13 <> IndentAt.GetText then Warnlog "Level 9: Wrong indent:  Should = "+ V13+ "  Is = "+ IndentAt.GetText
    Level.Select 10
'    if Not Relativ.IsChecked then Relativ.Check
    if Alignment.GetSelIndex <> 3 then Warnlog "Level 10: Wrong alignment:  Should = 3   Is = "+ Alignment.GetSelIndex
    if V14 <> AlignedAt.GetText then Warnlog "Level 10: Wrong Text <-> Spacing to text:  Should = "+ V14+ "  Is = "+ AlignedAt.GetText
'    if V15 <> AbstandTextNumerierung.GetText then Warnlog "Level 10: Wrong indent:  Should = "+ V15+ "  Is = "+ AbstandTextNumerierung.GetText
    if V16 <> IndentAt.GetText then Warnlog "Level 10: Wrong indent:  Should = "+ V16+ "  Is = "+ IndentAt.GetText
    TabPositionNumerierung.Cancel
    Sleep 1
    '/// Close document ///
    Call hCloseDocument
endcase

'--------------------------------------------------

testcase tFormatNumberingOptions
    Dim V1 : Dim V2 : Dim V3 : Dim V4 : Dim V5 : Dim V6 : Dim V7 : Dim V8 : Dim V9 : Dim V10
    Dim i as integer

    printlog "  - adjust numbering properties"
    '/// New document ///
    Call hNewDocument
    '/// Select Format / Numbering Bullets and adjust properties for each level ///
    FormatNumberingBullets

    Kontext
    Active.SetPage TabOptionenNumerierung
    Kontext "TabOptionenNumerierung"

    Ebene.Select 1
    Numerierung.Select 11           ' Keine
    Davor.SetText "Vorher "
    Dahinter.SetText "sp�ter "

    Ebene.Select 3
    Numerierung.Select 2            ' A,B,C,...
    Davor.SetText "��� "
    Dahinter.SetText "��� "
    Zeichenvorlage.Select 2
    V9 = Zeichenvorlage.GetSelText
    Vollstaendig.Less 1
    V1 = Vollstaendig.GetText
    BeginnBei.More 4
    V2 = BeginnBei.GetText

    Ebene.Select 5
    Numerierung.Select 8            ' Bullets
    if Davor.IsVisible then Warnlog "'Before' is visible on Bullet-Options"
    if Dahinter.IsVisible then Warnlog "'After' is visible on Bullet-Options"
    Zeichenvorlage.Select 1

    Ebene.Select 7
    Numerierung.Select 9            ' Grafik
    if Davor.IsVisible then Warnlog "'Before' is visible on Graphic-Options"
    if Dahinter.IsVisible then Warnlog "'After' is visible on Graphic-Options"
    Auswahl.Click
    Call hMenuSelectNr (1)
    Kontext "OeffnenDlg"
    Dateiname.Settext Convertpath(gtesttoolpath + "writer\optional\input\graphics\bullet.gif")
    Oeffnen.Click
    wait 500
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Warnlog Active.Gettext
            Active.Ok
        end if
    end if

    Kontext "TabOptionenNumerierung"
    Breite.More 4
    V3 = Breite.GetText
    Hoehe.More 3
    V4 = Hoehe.GetText
    Ausrichtung.Select 4

    Ebene.Select 9
    Numerierung.Select 1            ' 1,2,3,...
    Zeichenvorlage.Select 3
    V10 = Zeichenvorlage.GetSelText

    Ebene.Select 10
    Numerierung.Select 3            ' a,b,c,...
    V5 = Davor.GetText
    V6 = Dahinter.GetText
    Vollstaendig.More 9
    V7 = Vollstaendig.GetText
    BeginnBei.More 3
    V8 = BeginnBei.GetText
    TabOptionenNumerierung.OK

    printlog "  - write down numbering"
    '/// write down numbering ///
    call wTypeKeys "Hallo 1. Ebene<Return>"
    call wTypeKeys "<Tab>Hallo 2. Ebene<Return>"
    call wTypeKeys "<Tab>Hallo 3. Ebene<Return>"
    call wTypeKeys "<Tab>Hallo 4. Ebene<Return>"
    call wTypeKeys "<Tab>Hallo 5. Ebene<Return>"
    call wTypeKeys "<Tab>Hallo 6. Ebene<Return>"
    call wTypeKeys "<Tab>Hallo 7. Ebene<Return>"
    call wTypeKeys "<Tab>Hallo 8. Ebene<Return>"
    call wTypeKeys "<Tab>Hallo 9. Ebene<Return>"
    call wTypeKeys "<Tab>Hallo 10. Ebene<Return>"
    call wTypeKeys "<Tab>Immer noch 10. Ebene<Return>"
    call wTypeKeys "<Return><Return>Ende der Numerierung"
    call wTypeKeys "<Up>", 5

    FormatNumberingBullets
    Kontext
    Active.SetPage TabOptionenNumerierung
    Kontext "TabOptionenNumerierung"
    Ebene.Select 1
    if Numerierung.GetSelIndex <> 11 then Warnlog "Level 1: Wrong Numbering type : Should 11     Is " + Numerierung.GetSelIndex
    if Davor.GetText <> "Vorher " then Warnlog "Level 1: Wrong Text in 'After': Should 'Vorher '    Is " + Davor.GetText
    if Dahinter.GetText <> "sp�ter " then Warnlog "Level 1: Wrong Text in 'Before': Should 'sp�ter '    Is " + Dahinter.GetText
    Ebene.Select 3
    if Numerierung.GetSelIndex <> 2 then Warnlog "Level 3: Wrong Numbering type : Should 2     Is " + Numerierung.GetSelIndex
    if Davor.GetText <> "��� " then Warnlog "Level 3: Wrong Text in 'Before': Should '��� '    Is " + Davor.GetText
    if Dahinter.GetText <> "��� " then Warnlog "Level 3: Wrong Text in 'After': Should '��� '    Is " + Dahinter.GetText
    if Zeichenvorlage.GetSelText <> V9 then Warnlog "Level 3: Wrong Character Style: Should " + V9 + "    Is " + Zeichenvorlage.GetSelText
    if V1 <> Vollstaendig.GetText then Warnlog "Level 3: 'Show Sublevels' Is wrong: Should " + V1 + "   Is: " + Vollstaendig.GetText
    if V2 <> BeginnBei.GetText then Warnlog "Level 3: 'Start at'  Is wrong: Should " + V2 + "   Is: " + BeginnBei.GetText
    Ebene.Select 5
    if Numerierung.GetSelIndex <> 8 then Warnlog "Level 5: Wrong Numbering type : Should 8     Is " + Numerierung.GetSelIndex
    if Zeichenvorlage.GetSelIndex <> 1 then Warnlog "Level 5: Wrong Character Style: Should 1    Is " + Zeichenvorlage.GetSelIndex
    Ebene.Select 7
    if Numerierung.GetSelIndex <> 9 then
        Warnlog "Level 7: Wrong Numbering type : Should 9     Is " + Numerierung.GetSelIndex
    else
        if V3 <> Breite.GetText then Warnlog "Level 7: Wrong graphic width: Should " + V3 + "   Is: " + Breite.GetText
        if V4 <> Hoehe.GetText then Warnlog "Level 7: Wrong graphic height: Should " + V4 + "   Is: " + Hoehe.GetText
        if Ausrichtung.GetSelIndex <> 4 then Warnlog "Level 7: Alignment Is not 4 but " + Ausrichtung.GetSelIndex
    end if
    Ebene.Select 9
    if Numerierung.GetSelIndex <> 1 then Warnlog "Level 9: Wrong Numbering type : Should 1     Is " + Numerierung.GetSelIndex
    if Zeichenvorlage.GetSelText <> V10 then Warnlog "Level 9: Wrong Character Style: Should " + V10 +"    Is " + Zeichenvorlage.GetSelText
    Ebene.Select 10
    if Numerierung.GetSelIndex <> 3 then Warnlog "Level 10: Wrong Numbering type : Should 3     Is " + Numerierung.GetSelIndex
    if Davor.GetText <> V5 then Warnlog "Level 10: Wrong Text in 'Before': Should "+ V5 + "    Is " + Davor.GetText
    if Dahinter.GetText <> V6 then Warnlog "Level 10: Wrong Text in 'After': Should "+ V6 + "    Is " + Dahinter.GetText
    if V7 <> Vollstaendig.GetText then Warnlog "Level 10: 'Show Sublevels' Is wrong: Should " + V7 + "   Is: " + Vollstaendig.GetText
    if V8 <> BeginnBei.GetText then Warnlog "Level 10: 'Start at'  Is wrong: Should " + V8 + "   Is: " + BeginnBei.GetText
    TabOptionenNumerierung.Cancel

    printlog "  - check previous saved numbering"
    '/// save document, close it and reload previous saved document ///
    Call hFileSaveAsKill ( gOfficePath + "user\work\format6." & WriterFilterExtension(0) )
    Sleep 2
    Call hCloseDocument
    Sleep 2
    hFileOpen( gOfficePath & "user\work\format6." & WriterFilterExtension(0) )
    For i = 1 to 15
        call wTypeKeys "<Up>"
        Wait 500
    next i
    call wTypeKeys "<Home>"

    '/// check settings in Format / Numbering Bullets ///
    FormatNumberingBullets
    Kontext
    Active.SetPage TabOptionenNumerierung
    Kontext "TabOptionenNumerierung"
    Ebene.Select 1
    if Numerierung.GetSelIndex <> 11 then Warnlog "Level 1: Wrong Numbering type : Should 11     Is " + Numerierung.GetSelIndex
    if Davor.GetText <> "Vorher " then Warnlog "Level 1: Wrong Text in 'After': Should 'Vorher '    Is " + Davor.GetText
    if Dahinter.GetText <> "sp�ter " then Warnlog "Level 1: Wrong Text in 'Before': Should 'sp�ter '    Is " + Dahinter.GetText
    Ebene.Select 3
    if Numerierung.GetSelIndex <> 2 then Warnlog "Level 3: Wrong Numbering type : Should 2     Is " + Numerierung.GetSelIndex
    if Davor.GetText <> "��� " then Warnlog "Level 3: Wrong Text in 'Before': Should '��� '    Is " + Davor.GetText
    if Dahinter.GetText <> "��� " then Warnlog "Level 3: Wrong Text in 'After': Should '��� '    Is " + Dahinter.GetText
    if Zeichenvorlage.GetSelText <> V9 then Warnlog "Level 3: Wrong Character Style: Should " + V9 + "    Is " + Zeichenvorlage.GetSelText
    if V1 <> Vollstaendig.GetText then Warnlog "Level 3: 'Show Sublevels' Is wrong: Should " + V1 + "   Is: " + Vollstaendig.GetText
    if V2 <> BeginnBei.GetText then Warnlog "Level 3: 'Start at'  Is wrong: Should " + V2 + "   Is: " + BeginnBei.GetText
    Ebene.Select 5
    if Numerierung.GetSelIndex <> 8 then Warnlog "Level 5: Wrong Numbering type : Should 8     Is " + Numerierung.GetSelIndex
    if Zeichenvorlage.GetSelIndex <> 1 then Warnlog "Level 5: Wrong Character Style: Should 1    Is " + Zeichenvorlage.GetSelIndex
    Ebene.Select 7
    if Numerierung.GetSelIndex <> 9 then
        Warnlog "Level 7: Wrong Numbering type : Should 9     Is " + Numerierung.GetSelIndex
    else
        if V3 <> Breite.GetText then Warnlog "Level 7: Wrong graphic width: Should " + V3 + "   Is: " + Breite.GetText
        if V4 <> Hoehe.GetText then Warnlog "Level 7: Wrong graphic height: Should " + V4 + "   Is: " + Hoehe.GetText
        if Ausrichtung.GetSelIndex <> 4 then Warnlog "Level 7: Alignment Is not 4 but " + Ausrichtung.GetSelIndex
    end if
    Ebene.Select 9
    if Numerierung.GetSelIndex <> 1 then Warnlog "Level 9: Wrong Numbering type : Should 1     Is " + Numerierung.GetSelIndex
    if Zeichenvorlage.GetSelText <> V10 then Warnlog "Level 9: Wrong Character Style: Should " + V10 +"    Is " + Zeichenvorlage.GetSelText
    Ebene.Select 10
    if Numerierung.GetSelIndex <> 3 then Warnlog "Level 10: Wrong Numbering type : Should 3     Is " + Numerierung.GetSelIndex
    if Davor.GetText <> V5 then Warnlog "Level 10: Wrong Text in 'Before': Should "+ V5 + "    Is " + Davor.GetText
    if Dahinter.GetText <> V6 then Warnlog "Level 10: Wrong Text in 'After': Should "+ V6 + "    Is " + Dahinter.GetText
    if V7 <> Vollstaendig.GetText then Warnlog "Level 10: 'Show Sublevels' Is wrong: Should " + V7 + "   Is: " + Vollstaendig.GetText
    if V8 <> BeginnBei.GetText then Warnlog "Level 10: 'Start at'  Is wrong: Should " + V8 + "   Is: " + BeginnBei.GetText
    TabOptionenNumerierung.Cancel

    '/// Close document ///
    Call hCloseDocument
endcase

'--------------------------------------------------
testcase tFormatNumberingAll
	Dim i as Integer
	Dim Wert( 20 ) as Integer
	printlog "- Format / Numbering -Bullets in detail"
	printlog "  - adjust different Numbering and check the properties"
	'/// New document ///
	Call hNewDocument
	'/// Select Format / NumberingBullets and adjust different numbering ///
	FormatNumberingBullets
	
	Kontext
	Active.SetPage TabBullet
	Kontext "TabBullet"
	Sleep 1
	Auswahl.TypeKeys "<Right><Down>"
	Kontext
	Active.SetPage TabOptionenNumerierung
	Kontext "TabOptionenNumerierung"
	'/// Level 1 select a bullet ///
	Ebene.Select 1                                               ' 1. Ebene ein Bullet einstellen
	
	Kontext
	Active.SetPage TabOptionenNumerierung
	Kontext "TabOptionenNumerierung"
	'/// Level 2 select a numbering ///
	Ebene.Select 2                                               ' 2. Ebene eine Numerierungsart einstellen
	
	Kontext
	Active.SetPage TabNumerierungsart
	Kontext "TabNumerierungsart"
	Sleep 1
	Auswahl.TypeKeys "<Down>", 2
	
	Kontext
	Active.SetPage TabOptionenNumerierung
	Kontext "TabOptionenNumerierung"
	'/// Level 3 select a numbering ///
	Ebene.Select 3                                               ' 3. Ebene eine Numerierungsart einstellen
	
	Kontext
	Active.SetPage TabNumerierungsart
	Kontext "TabNumerierungsart"
	Sleep 1
	Auswahl.TypeKeys "<Down><Right>", 2
	
	Kontext
	Active.SetPage TabOptionenNumerierung
	Kontext "TabOptionenNumerierung"
	'/// Level 4 select a graphic ///
	Ebene.Select 4                                               ' 4. Ebene eine Grafik einstellen
	
	Kontext
	Active.SetPage TabGrafiken
	Kontext "TabGrafiken"
	Sleep 1
	Auswahl.TypeKeys "<Right>"
															' 5. und 6. Ebene werden nicht besonders eingestellt
	Kontext
	Active.SetPage TabOptionenNumerierung
	Kontext "TabOptionenNumerierung"
	'/// Level 7 select a graphic ///
	Ebene.Select 7                                               ' 7. Ebene eine Grafik einstellen
	
	Kontext
	Active.SetPage TabGrafiken
	Kontext "TabGrafiken"
	Sleep 1
	Auswahl.TypeKeys "<Right><Down>"
	
	Kontext
	Active.SetPage TabOptionenNumerierung
	Kontext "TabOptionenNumerierung"
	'/// Level 8 select a graphic ///
	Ebene.Select 8                                               ' 8. Ebene eine Grafik einstellen
	
	Kontext
	Active.SetPage TabGrafiken
	Kontext "TabGrafiken"
	Sleep 1
	Auswahl.TypeKeys "<Right>", 5
	
	Kontext
	Active.SetPage TabOptionenNumerierung
	Kontext "TabOptionenNumerierung"
	'/// Level 9 select a bullet ///
	Ebene.Select 9                                               ' 9. Ebene ein Bullet einstellen
	
	Kontext
	Active.SetPage TabBullet
	Kontext "TabBullet"
	Sleep 1
	Auswahl.TypeKeys "<Down>"
	
	Kontext
	Active.SetPage TabOptionenNumerierung
	Kontext "TabOptionenNumerierung"
	'/// Level 10 select a numbering ///
	Ebene.Select 10                                              ' 10. Ebene eine Numerierungsart einstellen
	
	Kontext
	Active.SetPage TabNumerierungsart
	Kontext "TabNumerierungsart"
	Sleep 1
	Auswahl.TypeKeys "<Right>", 2
	
	Kontext
	Active.SetPage TabOptionenNumerierung
	Kontext "TabOptionenNumerierung"
	
	for i=1 to 10
		Ebene.Select i
		Wert (i) = Numerierung.GetSelIndex
	next i
	'/// check settings made ///
	
	if Wert(1)  <> 8 then Warnlog "Selected entry => 1. Level Is not Bullets (8. Entry) but entry " + Wert(1)
	if Wert(2)  <> 2 then Warnlog "Selected entry => 2. Level Is not Numbering 1 but entry " + Wert(2)
	if Wert(3)  <> 3 then Warnlog "Selected entry => 3. Level Is not Numbering 2 but entry " + Wert(3)
	if Wert(4)  <> 9 then Warnlog "Selected entry => 4. Level Is not Graphic (9. Entry) but entry " + Wert(4)
	if Wert(5)  <> 8 then Warnlog "Selected entry => 5. Level Is not Bullets (8. Entry) but entry " + Wert(5)
	if Wert(6)  <> 8 then Warnlog "Selected entry => 6. Level Is not Bullets (8. Entry) but entry " + Wert(6)
	if Wert(7)  <> 9 then Warnlog "Selected entry => 7. Level Is not Graphic (9. Entry) but entry " + Wert(7)
	if Wert(8)  <> 9 then Warnlog "Selected entry => 8. Level Is not Graphic (9. Entry) but entry " + Wert(8)
	if Wert(9)  <> 8 then Warnlog "Selected entry => 9. Level Is not Bullets (8. Entry) but entry " + Wert(9)
	if Wert(10) <> 1 then Warnlog "Selected entry => 10. Level Is not Numbering 1 but entry " + Wert(10)
	
	TabOptionenNumerierung.OK
	
	printlog "  - create numbering and check them"
	'/// create numbering and check it ///
	call wTypeKeys "Hallo 1. Ebene<Return>"
	call wTypeKeys "<Tab>Hallo 2. Ebene<Return>"
	call wTypeKeys "<Tab>Hallo 3. Ebene<Return>"
	call wTypeKeys "<Tab>Hallo 4. Ebene<Return>"
	call wTypeKeys "<Tab>Hallo 5. Ebene<Return>"
	call wTypeKeys "<Tab>Hallo 6. Ebene<Return>"
	call wTypeKeys "<Tab>Hallo 7. Ebene<Return>"
	call wTypeKeys "<Tab>Hallo 8. Ebene<Return>"
	call wTypeKeys "<Tab>Hallo 9. Ebene<Return>"
	call wTypeKeys "<Tab>Hallo 10. Ebene<Return>"
	call wTypeKeys "<Tab>Immer noch 10. Ebene<Return>"
	call wTypeKeys "<Return><Return>Ende der Numerierung"
	call wTypeKeys "<Up>", 5
	
	FormatNumberingBullets
	'/// reopen dialog and check settings ///
	Kontext
	Active.SetPage TabOptionenNumerierung
	Kontext "TabOptionenNumerierung"
	for i=1 to 10
		Ebene.Select i
		if Numerierung.GetSelIndex <> Wert(i) then 
			Warnlog "After creating: => Level " + i + " has not numbering " + Wert (i) + " but " + Numerierung.GetSelIndex
		end if
	next i
	TabOptionenNumerierung.Cancel
	
	printlog "  - check previous saved numbering"
	'/// save document, close and reload previous saved document ///
	Sleep 2
	Call hFileSaveAsKill ( gOfficePath + "user\work\format7." & WriterFilterExtension(0) )
	Sleep 2
	Call hCloseDocument
	Sleep 2
	hFileOpen( gOfficePath & "user\work\format7." & WriterFilterExtension(0) )
	Sleep 2
	FormatNumberingBullets
	'/// reopen Format / NumberingBullets and check saved settings ///
	Kontext
	Active.SetPage TabOptionenNumerierung
	Kontext "TabOptionenNumerierung"
	for i=1 to 10
		Ebene.Select i
		if Numerierung.GetSelIndex <> Wert(i) then Warnlog "After saving: => Level " + i + " has not Numbering " + Wert (i) + " but " + Numerierung.GetSelIndex
	next i
	TabOptionenNumerierung.Cancel
	
	'/// close document ///
	Call hCloseDocument
endcase
