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
'* Owner : wolfram.garten@oracle.com
'*
'* short description :
'*
'**************************************************************************************
' #1 tiFormatArea
' #1 tdFormatArea1
'\*************************************************************************************

testcase tiFormatArea
    Dim i
    Dim Zaehler
    dim Entf$

    printlog "New impress document "
    hNewDocument
    printlog "create rectangle "
    hRechteckErstellen (20,20,60,60)
    Printlog "  - Change Format"
    printlog "change format for the rectangle "
    FormatArea
    Kontext
    active.SetPage TabArea
    Kontext "TabArea"
    printlog "apply bitmap as texture "
    FillOptions.Select 5
    printlog "Select Water "
    BitmapList.Select 3
    printlog "tile bitmap "
    Tile.Check
    Kontext
    active.SetPage TabSchatten
    kontext "TabSchatten"
    printlog "apply shadow "
    Anzeigen.Check
    printlog "set angle to 30ó�����"
    Entfernung.SetText "30"
    printlog "change distance "
    Entf$=Entfernung.GetText
    printlog "select color 15"
    Farbe.Select 15
    printlog "close dialog "
    TabSchatten.OK
    Sleep 3

    Printlog "  - Test various formats"
    printlog "Format area, check if changes are saved for this object "
    FormatArea
    Kontext
    active.SetPage TabArea
    Kontext "TabArea"
    if NOT FillOptions.GetSelIndex = 5 then
        Warnlog "Bitmap is not checked"
    else
        if BitmapList.GetSelIndex <> 3 then Warnlog "Wrong bitmap used. "+ BitmapAuswahl.GetSelIndex
        if NOT Tile.IsChecked then Warnlog "Tile is not checked"
        '      if Original.IsChecked then Warnlog "Original is not checked"
        '      if NOT Relativ.IsChecked then Warnlog "Relative is not checked"
        '      if Breite.GetText <> "90 %" then Warnlog "Width is not 90%, it is:  "+Breite.GetText
        '      if XOffset.GetText <> "30 %" then Warnlog "XOffset is not 30%, it is:  "+XOffset.GetText
        '      if NOT Zeile.IsChecked then Warnlog "Line is not checked"
        '      if Verschiebung.GetText <> "20 %" then Warnlog "Displace is not 20%, it is:  " + Verschiebung.GetText
            Kontext
            active.SetPage TabSchatten
            kontext "TabSchatten"
                if NOT Anzeigen.IsChecked then
                Warnlog "Show shadows is not checked"
            else
            if Entfernung.GetText <> Entf$ then Warnlog "Distance is not " + Entf$ + " it is: "+ Entfernung.GetText
            if Farbe.GetSelIndex <> 15 then Warnlog "Wrong color for shadow. It is: "+ Farbe.GetSelIndex
            end if
            end if
   TabSchatten.Cancel
   printlog "close document "
   hCloseDocument              
    
endcase 'tiFormatArea
'--------------------------------------------------------
testcase tdFormatArea1

    Dim i
    Dim j
    Dim k
    Dim m
    Dim n
    Dim Winkel_1 as string
    Dim ZaehlerFarbe
    Dim ZaehlerVerlauf
    Dim ZaehlerSchraffur
    Dim ZaehlerBitmap
    Dim ColorName$
    ColorName$ = "Bullshit"
    dim Zaehler as integer

    printlog "new document "
    Call hNewDocument
    sleep 3
    printlog "create rectangle "
    Call hRechteckErstellen (30,30,70,70)
    sleep 2
    printlog "format area "
    FormatArea
    Kontext
    Active.SetPage TabArea
    Kontext "TabArea"
    printlog "Select no fill "
    FillOptions.Select 1
    sleep 2
    TabArea.OK
    sleep 1
    FormatArea
    Kontext
    Active.SetPage TabArea
    Kontext "TabArea"
    sleep 1
    printlog "control if changes are still there after closing and reopening dialog "
    printlog "check gradient and select 1 "
    FillOptions.Select 3'
    ZaehlerVerlauf = GradientList.GetItemCount
    for j = 1 To 2
        GradientList.Select j
        TabArea.OK
        FormatArea
        Kontext
        Active.SetPage TabArea
        Kontext "TabArea"
        PrintLog GradientList.GetSelText
    next j
    printlog "select hatching, control after closing and reopening dialog if changes are recognized "
    FillOptions.Select 4
    HatchingList.GetItemCount
    ZaehlerSchraffur = HatchingList.GetItemCount
    for k=1 to 2'ZaehlerSchraffur
        HatchingList.Select k
        TabArea.OK
        FormatArea
        Kontext
        Active.SetPage TabArea
        Kontext "TabArea"
        PrintLog HatchingList.GetSelText
  next k

    printlog "check bitmap, control if changes are permanent after closing and reopening dialog "
    FillOptions.Select 5
    BitmapList.GetItemCount
    ZaehlerBitmap = BitmapList.GetItemCount
    for n=1 to ZaehlerBitmap
        BitmapList.Select n
        TabArea.OK
        FormatArea
        sleep 1
        Kontext
        Active.SetPage TabArea
        Kontext "TabArea"
        PrintLog BitmapList.GetSelText
    next n

    printlog "check tile "
    if Tile.IsChecked = False Then Tile.Check
    '  if Anpassen.IsEnabled = False Then PrintLog " Option Tile activated"
    '  if Anpassen.IsEnabled = True Then WarnLog " Option Tile not activated"
    '  if Relativ.IsChecked = False Then
    '     SetClipboard Breite.GetText
    '     Relativ.Check
    '   If Breite.GetText = GetClipboardText Then
    '   WarnLog "  Relative isnt working"
    '   else
    '   PrintLog "   Relative works"
    '   end if
    '  end if
    '
    '  if Kacheln.IsChecked = True Then Kacheln.UnCheck
    '  Anpassen.Check
    '  if Relativ.IsEnabled = True Then WarnLog "   Option Anpassen konnte nicht aktiviert werden"
    '  if not Relativ.IsEnabled Then PrintLog "   Option Anpassen funktioniert"
    '  Anpassen.Uncheck
    '  Original.Check
    '  if Breite.IsEnabled And Hoehe.IsEnabled = True Then WarnLog "   Button Original funktioniert nicht"
    '  if Breite.IsEnabled And Hoehe.IsEnabled = False Then PrintLog "   Button Original funktioniert"
    '  Kacheln.Check
    '  setClipboard XOffset.GetText
    '  XOffset.More 3
    '  if XOffset.GetText <> GetClipboardText Then PrintLog "   Position XOffset funktioniert"
    '  if XOffset.GetText = GetClipboardText Then WarnLog "    Position XOffset funktioniert nicht"
    '  SetClipboard YOffset.GetText
    '  YOffset.More 5
    '  if YOffset.GetText <> GetClipboardText Then PrintLog "   Position YOffset funktioniert"
    '  if YOffset.GetText = GetClipboardText Then WarnLog "    Position YOffset funktioniert nicht"
    '  PrintLog "        TabArea fertig"
    Kontext
    Active.SetPage TabSchatten
    Kontext "TabSchatten"
    sleep 1
    printlog "uncheck shadow "
    if Anzeigen.IsChecked = True Then Anzeigen.UnCheck
        sleep 1
        printlog "check if shadow related controls are disabled with no shadow "
    if Entfernung.IsEnabled = False Then
        PrintLog "   Show shadow works"
    else
        WarnLog "   Show shadow does not work"
    end if

    printlog "check shadow "
    Anzeigen.Check
    printlog "change distance "
    SetClipboard Entfernung.GetText
    Entfernung.More 4
    if GetClipboardText = Entfernung.GetText Then
        WarnLog "   Distance does not work"
    else
        PrintLog "   Distance does work"
    end if
    printlog "change shadow color "
    Farbe.Select 5

    Kontext
    Active.SetPage TabFarben
    Kontext "TabFarben"
    Farbe.Select 24
    sleep 1
    Farbe.Select 18
    sleep 1
    Farbe.Select 7
    printlog "change color "
    Farbname.SetText "Bullshit"
    Aendern.Click
    sleep 1
    TabFarben.OK
    FormatArea
    Kontext
    Active.SetPage TabArea
    Kontext "TabArea"
    FillOptions.Select 2
    if ColourList.GetSelText = "Bullshit" Then
        PrintLog " Color selection changed "
    else
        WarnLog " Color selection not changed"
    end if
    Kontext
    Active.SetPage TabFarben
    Kontext "TabFarben"
    Farbmodell.Select 1

    Kontext
    printlog "test RGB color model "
    Active.SetPage TabFarben
    Kontext "TabFarben"
    printlog "switch to RGB values " ' 33:RVB,CMJN
    Farbmodell.Select 1
    printlog "   Selected colourmodel RGB?: "+Farbmodell.GetSelText
    if left(Farbmodell.GetSelText,1) <> "R" then warnlog "not RGB selected ? :-("
        try
        R.SetText "50"
        G.SetText "10"
        B.SetText "90"
    catch
        warnlog "RGB text fields are not enabled :-("
    endcatch
    Aendern.Click
    TabFarben.OK
    Kontext "DocumentDraw"
    FormatArea
    Kontext
    Active.SetPage TabFarben
    Kontext "TabFarben"
    if (R.GetText = "50" And G.GetText = "10" And B.GetText = "90") Then
        PrintLog "Changes in RGB done"
    else
        WarnLog " Chancges in RGB values not recognized, instead of R=50 , G=10 und B=90  it is " +R.GetText + " " + G.GetText + " " + B.GetText
    end if
    Farbe.Select 10

    FarbName.SetText "Brighter"
    printlog "add a color "
    Hinzufuegen.Click
    Farbe.Select "Brighter"
    printlog "delete color "
    Loeschen.Click
    Kontext "Active"
    Active.Yes
    Kontext
    Active.SetPage TabFarben
    Kontext "TabFarben"
    printlog "edit color "
    Bearbeiten.Click
    Kontext  "FarbeDlg"
    FarbeDlg.OK
    sleep 2
    Kontext
    Active.SetPage TabFarben
    Kontext "TabFarben"
    TabFarben.OK
    FormatArea
    Kontext
    Active.SetPage TabFarben
    Kontext "TabFarben"
    TabFarben.OK

    FormatArea
    Kontext
    Active.SetPage TabFarbverlaeufe
    Kontext "TabFarbverlaeufe"
    TabFarbverlaeufe.OK
    sleep 2

    FormatArea
    sleep 2
    Kontext
    Active.SetPage TabTransparenz
    Kontext "TabTransparenz"
    printlog "check if controls are disabled if transparence is disabled "
    KeineTransparenz.Check
    Printlog "- No transparency is checked, all controls should be disabled."
    if MFLinTransparenz.IsEnabled Then Warnlog "- Transparency enabled"
    if TransparenzverlaufTyp.IsEnabled Then Warnlog "- Graidient enabled"
    if MFZentrumX.IsEnabled Then Warnlog "- Center X enabled"
    if MFZentrumY.IsEnabled Then Warnlog "- Center Y enabled"
    if MFWinkel.IsEnabled Then Warnlog "- Angle enabled"
    if MFRand.IsEnabled Then Warnlog "- Border enabled"
    if MFStartwert.IsEnabled Then Warnlog "- Start value enabled"
    if MFEndwert.IsEnabled Then Warnlog "- End value enabled"

    printlog "check linear tranparency "
    LineareTransparenz.Check
    Printlog "- Set transparency to linear, all controls should be enabled now"
    sleep 1
    printlog "change values of controls "
    MFLinTransparenz.ToMax
    if MFLinTransparenz.GetText <>"100%" Then Warnlog "- Maximum value not correct"
    MFLinTransparenz.ToMin
    if MFLinTransparenz.GetText <>"0%" Then Warnlog "- Minimum value not correct"
    if TransparenzverlaufTyp.IsEnabled Then Warnlog "- Gradient enabled"
    if MFZentrumX.IsEnabled Then Warnlog "- Centrum X enabled"
    if MFZentrumY.IsEnabled Then Warnlog "- Centrum Y enabled"
    if MFWinkel.IsEnabled Then Warnlog "- Angle enabled"
    if MFRand.IsEnabled Then Warnlog "- Border enabled"
    if MFStartwert.IsEnabled Then Warnlog "- Start value enabled"
    if MFEndwert.IsEnabled Then Warnlog "- End value enabled"

    Transparenzverlauf.Check
    Printlog "- Gradient for transprency enabled"
    if MFLinTransparenz.IsEnabled = "false" Then Warnlog "- Transparency disabled"
    if TransparenzverlaufTyp.IsEnabled = "false"  Then Warnlog "- Gradient disabled"
    if MFZentrumX.IsEnabled = "false" Then Warnlog "- Center X disabled"
    if MFZentrumY.IsEnabled = "false" Then Warnlog "- Center Y disabled"
    if MFWinkel.IsEnabled = "false" Then Warnlog "- Anlge disabled"
    if MFRand.IsEnabled = "false" Then Warnlog "- Border disabled"
    if MFStartwert.IsEnabled = "false" Then Warnlog "- Start value disabled"
    if MFEndwert.IsEnabled = "false" Then Warnlog "- End value disabled"

    Printlog "- Check if changed values are saved"

    Zaehler=TransparenzverlaufTyp.GetItemCount
    for i=1 to Zaehler
        TransparenzverlaufTyp.Select i
        SetClipboard Transparenzverlauftyp.GetSelText
        TabTransparenz.OK
        sleep 2
        FormatArea
        sleep 1
        Kontext
        printlog "check if values are saved after closing and reopening dialog "
        Active.SetPage TabTransparenz
        Kontext "TabTransparenz"
        if not Transparenzverlauftyp.GetSelIndex = i Then Warnlog "- Values are not correct for gradient transparency"
    next i
    TransparenzverlaufTyp.Select 2
    if MFZentrumX.IsEnabled Then Warnlog "- Center X should not be enabled"
    if MFZentrumY.IsEnabled Then Warnlog "- Center Y should not be enabled"
    MFWinkel.SetText "45"
    Winkel_1 = MFWinkel.GetText
    TabTransparenz.OK
    sleep 2
    FormatArea
    sleep 2
    Kontext
    Active.SetPage TabTransparenz
    Kontext "TabTransparenz"
    if TransparenzverlaufTyp.GetSelIndex <>2 Then
        Warnlog "- Second menu-item is not choosen"
    end if
    TransparenzverlaufTyp.Select 3
    if MFWinkel.GetText <> Winkel_1 then Warnlog "- The inserted value is not accepted"
    if MFWinkel.GetText <> Winkel_1 Then TransparenzverlaufTyp.Select 3
    if MFWinkel.IsEnabled Then Warnlog " - Angle should not be active when Radial is choosen"
    sleep 1
    MFZentrumX.SetText "30"
    MFZentrumY.SetText "30"
    TabTransparenz.OK
    sleep 2
    FormatArea
    sleep 2
    Kontext
    Active.SetPage TabTransparenz
    Kontext "TabTransparenz"
    if MFZentrumX.GetText <> "30%" Then Warnlog "- Setted value for Center X is not applied."
    if MFZentrumY.GetText <> "30%" Then Warnlog "- Setted value for Center Y is not applied."
    MFRand.SetText "10"
    MFStartwert.SetText "20"
    MFEndwert.SetText "30"
    TabTransparenz.OK
    sleep 2
    FormatArea
    sleep 2
    Kontext
    Active.SetPage TabTransparenz
    Kontext "TabTransparenz"
    if MFRand.GetText <> "10%" Then Warnlog "- Border-value was not set."
    if MFEndwert.GetText <> "30%" Then Warnlog "- End-value was not set."
    if MFStartwert.GetText <> "20%" Then Warnlog "- Start-value was not set."
    TabTransparenz.OK

    printlog "close document "
    Call hCloseDocument

endcase 'tdFormatArea1

