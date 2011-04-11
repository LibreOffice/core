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
'*******************************************************************
' #1 tiFormatText
' #1 tdFormatLayer
' #1 tdFormatConnector
' #1 tiFormatPositionAndSize
' #1 tiFormat3D_Effekte
' #1 tdFormatTemplates
' #1 tiFormatAufzaehlungszeichen
'\******************************************************************
testcase tiFormatText
    Dim a as string
    Dim b as string
    Dim n as integer
    Dim i as integer
    Dim ZaehlerEffekt
    dim sTemp as string

    printlog "open application"
    Call hNewDocument
    Call sSelectEmptyLayout
    printlog "create a textbox with text"
    Call hTextrahmenErstellen ("This is a text, which, I hope, in Openoffice, will be automaticly formatted. Should that not be the case, then it doesn't matter.",20,20,70,20)
    WaitSlot (1000)
    printlog "click outside of textbox, to leave edit mode of textbox and deselect it"
    gMouseClick 99,99
    printlog "type key [TAB] to select textbox again"
    hTypeKeys "<TAB>"

    printlog "Format->Position and Size"
    FormatPositionAndSize
    Kontext
    printlog "select tabpage 'Format and Size' on dialog 'Position and Size'"
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    sleep 2
    printlog "get and remember Position: 'Position X'"
    a = PositionX.GetText
    printlog "close dialog 'Position and Size' with OK"
    TabPositionAndSize.OK

    printlog "Format -> Text..."
    FormatTextDraw
    Kontext
    printlog "select tabpage 'Text'"
    Active.SetPage TabText
    Kontext "TabText"
    printlog "Check checkbox 'Fit width to text'"
    BreiteAnTextAnpassen.Check
    printlog "leave dialog with OK"
    TabText.OK
    printlog "Dialog should have been resized and moved now!"

    printlog "Format->Position and Size"
    ContextPositionAndSize
    Kontext
    printlog "select tabpage 'Format and Size' on dialog 'Position and Size'"
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    WaitSlot (2000)
    printlog "compare 'Position X' with saved value - has to be different"
    sTemp = PositionX.GetText
    if (sTemp <> a)  Then
        Printlog "  - Fit to frame does work"
    else
        warnlog "  - Fit to frame doesn't work: expected: '" + a + "' is: '" +  + "'"
    end if
    printlog "get and remember Position: 'Position Y'"
    b=PositionY.GetText
    TabPositionAndSize.OK

    printlog "Format -> Text..."
    FormatTextDraw
    Kontext
    printlog "select tabpage 'Text'"
    Active.SetPage TabText
    Kontext "TabText"
    printlog "UNCheck checkbox 'Fit width to text'"
    BreiteAnTextAnpassen.Uncheck
    printlog "check checkbox 'Fit height to text'"
    HoeheAnTextAnpassen.Check
    printlog "if checkbox 'Fit to frame' is enabled: this state only happens, if 'Fit height to text' and 'Fit width to text' are checked"
    if AmRahmenAnpassen.IsEnabled Then
        WarnLog "  - checkbox 'Fit to frame' is enabled, which is wrong!"
    else
        PrintLog "  - checkbox 'Fit to frame' is disabled."
    end if
    WaitSlot (1000)
    printlog "UNcheck checkbox 'Fit height to text'"
    HoeheAnTextAnpassen.UnCheck
    printlog "UNCheck checkbox 'Fit width to text'"
    BreiteAnTextAnpassen.UnCheck
    printlog "if checkbox 'Fit to frame' is disabled: this state only happens, if 'Fit height to text' and/or 'Fit width to text' are checked"
    if AmRahmenAnpassen.IsEnabled Then
        PrintLog "  - checkbox 'Fit to frame' is enabled."
    else
        WarnLog "  - checkbox 'Fit to frame' is disabled, which is wrong!"
    end if
    printlog "set metric field 'Left' to 10"
    Links.SetText "10"
    printlog "set metric field 'Right' to 10"
    Rechts.SetText "10"
    printlog "set metric field 'Top' to 10"
    Oben.SetText "10"
    printlog "set metric field 'Bottom' to 10"
    Unten.SetText "10"
    printlog "leave dialog with OK"
    TabText.OK

    printlog "Edit -> Select All"
    EditSelectAll
    printlog "type Keys [Strg]+[x] to cut object"
    hTypeKeys "<MOD1 X>"
    printlog "No object exists in dokument right now!"
    printlog "Format -> Text..."
    FormatTextDraw
    Kontext
    printlog "select tabpage 'Text'"
    Active.SetPage TabText
    Kontext "TabText"
    printlog "check checkbox 'Adjust to contour'"
    Konturfluss.Check
    printlog "now every other control on the dialog has to get disabled"
    if ((AmRahmenAnpassen.IsEnabled=False) And (Links.IsEnabled=False) And (Rechts.IsEnabled=False) And (Oben.IsEnabled=False) And (Unten.IsEnabled =False)) Then
        PrintLog "  - Adjust to contour is working"
    else
        WarnLog "  - contourflow aktivated doesn't work it's supposed to"
    end if
    printlog "leave dialog with OK"
    TabText.OK

    printlog "create a textbox with text"
    Call hTextrahmenErstellen ("Just some test-text...",20,20,60,60)
    WaitSlot (5000)
    printlog "Format -> Text..."
    FormatTextDraw
    Kontext
    printlog "select tabpage 'Text Animation'"
    Active.SetPage TabLauftext
    Kontext "TabLauftext"

    ZaehlerEffekt = Effekt.GetItemCount
    if (ZaehlerEffekt <> 5) then
        qaerrorlog "There are now more or less than 5 Effects - Change testcase! " + ZaehlerEffekt
    end if
    for n=1 to ZaehlerEffekt
        Effekt.Select n
        printlog n + ": '" + Effekt.getSelText + "' -------------------------------"
        if (n > 2) then
            printlog "test all directions"
            for i = 1 to 4
                printlog "" + i
                select case i
                    case 1: NachOben.click
                    case 2: NachLinks.click
                    case 3: NachRechts.click
                    case 4: NachUnten.click
                end select
                sFormatTextDrawAnimation
            next i
        end if
        if ((n > 1) AND (n < 5)) then
            printlog "test all Start/End Text visibility"
            for i = 1 to 4
                printlog "" + i
                select case i
                    case 1: TextSichtbarBeimStarten.check : TextSichtbarBeimBeenden.uncheck
                    case 2: TextSichtbarBeimBeenden.check
                    case 3: TextSichtbarBeimStarten.uncheck
                    case 4: TextSichtbarBeimBeenden.uncheck
                end select
                sFormatTextDrawAnimation
            next i
        end if
        if ((n > 1) AND (n < 5)) then
            printlog "test all Animation cycles"
            for i = 1 to 3
                printlog "" + i
                select case i
                    case 1: Endlos.check
                    case 2: Endlos.UnCheck : Anzahl.typeKeys ("<PageUp>")
                    case 3: If Endlos.IsChecked = true then
                               QaErrorLog "#i74067# - Checkbox checked when it shouldn't have been."
                               Endlos.UnCheck
                            end if
                            Anzahl.setText ("1")
                end select
                sFormatTextDrawAnimation
            next i
        end if
        if (n > 2) then
            printlog "test all Increment"
            for i = 1 to 4
                printlog "" + i
                select case i
                    case 1: Pixel.check : Schrittweite.setText("1")
                    case 2: printlog "#114551# Loop, if Step greater than object size"
                    case 3: Pixel.UnCheck : Schrittweite.typeKeys ("<PageDown>")
                    case 4: printlog "#114551# Loop, if Step greater than object size"
                end select
                sFormatTextDrawAnimation
            next i
        end if
        if (n > 1) then
            printlog "test all Delay"
            for i = 1 to 3
                printlog "" + i
                select case i
                    case 1: Automatisch.check
                    case 2: Automatisch.UnCheck : Verzoegerung.setText("1")
                    case 3: Verzoegerung.typeKeys ("<PageUp>")
                end select
                sFormatTextDrawAnimation
            next i
        end if
    next n
    TabLauftext.OK
    Call hCloseDocument
    
endcase 'tiFormatText
'-------------------------------------------------------------------------------
testcase tdFormatLayer

    printlog "open application"
    Call hNewDocument

    printlog "Edit->Layer->Insert (in DRAW also via Insert->Layer)"
    InsertLayer
    Kontext "EbeneEinfuegenDlg"
    EbenenName.SetText " Test 4"
    LayerTitle.SetText " Test no 4"
    LayerDescription.SetText " This is the Description of the Layer."
    EbeneEinfuegenDlg.OK

    printlog "Edit->Layer->Modify (in DRAW also via Format->Layer)"
    FormatLayer
    Kontext "EbeneAendernDlg"
    if EbenenName.GetText = " Test 4" Then
        EbenenName.SetText "Layer 5"
    end if
    if LayerTitle.GetText = " Test no 4" Then
        LayerTitle.SetText "Layer test 5"
    end if
    if LayerDescription.GetText = " This is the Description of the Layer." Then
        LayerDescription.SetText " This is the changed Description of the Layer."
    end if

    Sichtbar.Check
    Druckbar.Check
    Gesperrt.Check
    EbeneAendernDlg.OK

    printlog "Edit->Layer->Modify (in DRAW also via Format->Layer)"
    FormatLayer
    Kontext "EbeneAendernDlg"
    if EbenenName.GetText <> "Layer 5" Then
        WarnLog " Name-Value either didn't change at all, or was empty. Expected: 'Layer 5', but was: " + EbenenName.GetText
    end if
    if LayerTitle.GetText <> "Layer test 5" Then
        WarnLog " Title-Value either didn't change at all, or was empty. Expected: 'Layer test 5', but was: " + LayerTitle.GetText
    end if
    if LayerDescription.GetText <> " This is the changed Description of the Layer." Then
        WarnLog " Description-Value either didn't change at all, or was empty. Expected: ' This is the changed Description of the Layer.', but was: " + LayerDescription.GetText
    end if

    if Sichtbar.IsChecked=False Then
        WarnLog " Value changed: visible"
    end if
    if Druckbar.IsChecked=False Then
        WarnLog " Value changed: printable"
    end if
    if Gesperrt.IsChecked=False then
        WarnLog " Value changed: locked"
    end if
    EbeneAendernDlg.OK

    printlog "close application"
    Call hCloseDocument
    
endcase 'tdFormatLayer
'-------------------------------------------------------------------------------
testcase tdFormatConnector

    Dim Zaehler as integer
    Dim i as integer
    dim sUnit as string
    dim sTemp as string
    dim cDecSep as string

    if (gApplication = "IMPRESS") then 
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

    Call hNewDocument
    printlog "insert connector"
    kontext "Connectorsbar"
    if Connectorsbar.Exists then
        Connectorsbar.Close
    end if

    kontext "Toolbar"
    Sleep (1)
    try
        Verbinder.TearOff  ' insert connector
    catch
        warnlog "bug for GH from TBO; .tearoff doesn't tell success"
    endcatch

    kontext "Connectorsbar"
    Verbinder.click
    gMouseMove ( 10,10,30,30)

    Call hCloseDocument
    printlog "Open file with a finished scenario"
    hFileOpen  ConvertPath (gTesttoolPath + "graphics\required\input\FormatConnector." + ExtensionString)
    sleep (1)
    printlog "Check if the document is writable"
    if fIsDocumentWritable = false then
        ' Make the document writable and check if it's succesfull
        if fMakeDocumentWritable = false then
            warnlog "The document can't be make writeable. Test stopped."
            goto endsub
        end if
    end if
    gMouseClick 1,1

    printlog "Press TAB three times to select the connector."
    hTypekeys "<TAB>"
    hTypekeys "<TAB>"
    hTypekeys "<TAB>"

    WaitSlot (1000)
    printlog "OpenContextMenu(true)"
    hTypeKeys "<SHIFT F10>"   
    sleep (3)
    printlog "Select Connector"
    hMenuSelectNr(2)  
    Kontext "Verbinder"
    Typ.Select 2 ' the one with 2 from 3 posibilities to set a value...
   Sleep (1)
    AnfangHorizontal.SetText "1"
    Verbinder.TypeKeys "<TAB>"
    AnfangVertikal.SetText "1"
    Verbinder.TypeKeys "<TAB>"
    EndeHorizontal.SetText "1"
    Verbinder.TypeKeys "<TAB>"
    EndeVertikal.SetText "1"
    Verbinder.TypeKeys "<TAB>"
    printlog EndeVertikal.getText
    try
        Linie1.SetText "10"
        Verbinder.TypeKeys "<TAB>"
        sTemp = Linie1.getText
        sUnit = GetMeasUnit(sTemp)
        cDecSep = GetDecimalSeperator(sTemp)
    catch
        PrintLog "  Lineoffset not activated -> you have to choose another linetype: 1"
    endcatch
    try
        Linie2.SetText "10"
        Verbinder.TypeKeys "<TAB>"
    catch
        PrintLog "  Lineoffset not activated -> you have to choose another linetype: 2"
    endcatch
    try
        Linie3.SetText "10"
        Verbinder.TypeKeys "<TAB>"
    catch
        PrintLog "  Lineoffset not activated -> you have to choose another linetype: 3 when will this be enabled ?"
    endcatch
    Verbinder.OK

    FormatConnector
    Kontext "Verbinder"
    sTemp = AnfangHorizontal.GetText

    if fCompareTwoValues(sTemp, "1"+cDecSep+"00"+sUnit) Then
        WarnLog "  value not changed"
        printlog "AnfangHorizontal = is '" + sTemp + "' should: '"+"1"+cDecSep+"00"+sUnit+"'"
    end if
    if fCompareTwoValues(AnfangVertikal.GetText, "1"+cDecSep+"00"+sUnit) Then
        WarnLog "  value not changed"
        printlog "AnfangVertikal.GetText = " + AnfangVertikal.GetText
    end if
    if fCompareTwoValues(EndeHorizontal.GetText, "1"+cDecSep+"00"+sUnit) Then
        WarnLog "  value not changed"
        printlog "EndeHorizontal.GetText = " + EndeHorizontal.GetText
    end if
    if fCompareTwoValues(EndeVertikal.GetText, "1"+cDecSep+"00"+sUnit) Then
        WarnLog "  value not changed"
        printlog "EndeHorizontal.GetText = " + EndeHorizontal.GetText
    end if

    Zaehler = Typ.GetItemCount
    for i = 1 to Zaehler
        Typ.Select i
        SetClipboard Typ.GetSelText
        Verbinder.OK
        FormatConnector
        Kontext "Verbinder"
        if GetClipboardText <> Typ.GetSelText Then
            WarnLog "  Selection not saved. Was: " + Typ.GetSelText + " . But should have been: " +GetClipboardText
        end if
    next i
    Verbinder.OK
    Call hCloseDocument
    
endcase 'tdFormatConnector
'-------------------------------------------------------------------------------
testcase tiFormatPositionAndSize

    Dim x      'Variable fuer PositionX
    Dim y      'Variable fuer PositionY
    Dim h      'Variable fuer Hoehe
    Dim b      'Variable fuer Breite
    Dim r      'Variable fuer Radius
    Dim w      'Variable fuer Winkel
    
printlog "New impress doc"
    Call hNewDocument                  
    WaitSlot (3000)
    printlog "create rectangle"
    Call hRechteckErstellen (20,20,70,30)       
    WaitSlot (1000)
    gMouseClick 50,25
    WaitSlot (1000)
    printlog "Format-position and size"
    ContextPositionAndSize        
    WaitSlot (1000)
    Kontext
    printlog "TabPositionAndSize"
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    printlog "put x and y position into clipboard"
    printlog "x = " & PositionX.GetText
    printlog "y = " & PositionY.GetText
    SetClipboard (PositionX.GetText + PositionY.GetText)
    TabPositionAndSize.OK
    WaitSlot (1000)
    printlog "move rectangle down and right"
    hTypeKeys "<DOWN>",10         
    hTypeKeys "<RIGHT>",10
    printlog "open PositionAndSize"
    ContextPositionAndSize
    WaitSlot (1000)
    Kontext
    printlog "TabPositionAndSize"
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    printlog "control position changes with clipboard content"
    if (PositionX.GetText + PositionY.GetText) = GetClipboardText Then WarnLog "Der Dialog hat die Positionsaenderung nicht registriert"
    printlog "set position of rectange using the edit fields"
    PositionX.SetText "2"
    x=PositionX.GetText
    PositionY.SetText "2"
    y=PositionY.GetText
    printlog "check protect"
    ProtectPosition.Check
    TabPositionAndSize.OK
    WaitSlot (1000)
    ContextPositionAndSize
    WaitSlot (1000)
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    printlog "check if values are correct"
    if PositionX.GetText = x And PositionY.GetText = y Then PrintLog " Values could be changed"
    printlog "check if protect works"
    if PositionX.GetText<> x Then
        WarnLog "Protect does not work"
    else
        PrintLog "Protext works, X axis checked"
    end if
    if PositionY.GetText<> y Then
        WarnLog "Protect does not work"
    else
        PrintLog "  Protext does work, checked y axis"
    end if
    printlog "uncheck protect"
    ProtectPosition.UnCheck
    TabPositionAndSize.OK
    WaitSlot (1000)
    ContextPositionAndSize
    WaitSlot (1000)
    Kontext
    printlog "TabPositionAndSize"
    WaitSlot (1000)
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    KeepRatio.UnCheck
    printlog "set width to 10"
    WaitSlot (1000)
    Width.SetText "10"
    printlog "put value into a variable"
    WaitSlot (1000)
    b=Width.GetText
    printlog "Width is " & b
    SetClipboard Width.GetText
    printlog "set heigth to 5"
    Height.SetText "5"
    printlog "put value into variable"
    WaitSlot (1000)
    h=Height.GetText
    printlog "Height is " & h
    printlog "check protect"
    ProtectPosition.Check
    printlog "close dialog"
    TabPositionAndSize.OK
    WaitSlot (1000)
    ContextPositionAndSize
    WaitSlot (1000)
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    WaitSlot (2000)
    printlog "control size changes"
    printlog "Width value is: " & Width.GetText
    printlog "Value from Clipboard is: " & GetClipboardText
    if Width.GetText = GetClipboardText Then
        PrintLog "  Size of rectangle could be changed. It is = " + Width.GetText + "   but it should be = " + b
    else
        WarnLog "  Controls for height and width do not work"
    end if
    printlog "Uncheck the Position-Protection, so we can work with this window."
    if ProtectPosition.isChecked = FALSE then
        printlog "Should be checked since it was checked just 16 rows ago"
        ErrorLog "ProtectPosition was not checked - why?"
    else
       ProtectPosition.Uncheck
    end if

    KeepRatio.Check
    printlog "raise value for width"
    Width.More 5
    if Height.GetText = h Then
    WarnLog "  Keep ratio does not work properly"
    else
    PrintLog "  Keep ratio does work"
    KeepRatio.UnCheck
    end if
    TabPositionAndSize.OK
    WaitSlot (2000)
    gMouseClick 90,90
    WaitSlot (2000)
    printlog "Now trying TEXT..."
    printlog "create textbox"
    Call hTextrahmenErstellen ("Never before have I written such a meaning" ,10,10,60,20) 
    WaitSlot (2000)
    printlog "Format-Postion and Size"
    FormatPositionAndSize
    WaitSlot (1000)
    Kontext
    printlog "TabPositionAndSize"
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    printlog "default value ?: width :-----"+Width.IsEnabled
    printlog "check fit width to text"
    if Width.IsEnabled then FitWidthText.Check
    if Width.IsEnabled = True Then
        WarnLog "  Fit width does not work"
    else
        PrintLog "  Fit width does work"
    end if
    printlog "fit heigth to text"
    FitHeightText.Check
    if Height.IsEnabled = True Then
        WarnLog "  fit heigth doe not work"
    else
        PrintLog "  fit heigth does work"
    end if
    '   PrintLog "  Fit width to text active:   " + FitWidthText.IsChecked
    '   PrintLog "  Fit heigth to text active:   " + FitHeightText.IsChecked
    printlog "uncheck fit width to text"
    FitWidthText.UnCheck
    printlog "uncheck fit heigth to text"
    FitHeightText.Uncheck
    TabPositionAndSize.OK
    WaitSlot (1000)
    hTypeKeys "<escape>",2
    hTypeKeys "<TAB>",2
    WaitSlot (1000)
    ContextPositionAndSize
    WaitSlot (1000)
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    printlog "check x-position of textbox"
    SetClipboard PositionX.GetText
    WaitSlot (1000)
    Kontext
    printlog "set rotation angle to 50 and position x and y to 1"
    Active.SetPage TabDrehung
    Kontext "TabDrehung"
    PositionX.SetText "1"
    PositionY.SetText "1"
    Winkel.SetText "50"
    w=Winkel.GetText
    TabDrehung.OK
    WaitSlot (3000)

    printlog "check position of manipulated textbox"
    ContextPositionAndSize
    WaitSlot (3000)
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    if PositionX = GetClipboardText Then PrintLog "  Position changed due to rotation"
    Kontext
    Active.SetPage TabDrehung
    Kontext "TabDrehung"
    WaitSlot (1000)
    printlog "check rotation angle"
    if Winkel.GetText = w Then PrintLog "  Object rotated"
    TabDrehung.OK

    printlog "RECTANGLE-----2/2"
    printlog "create rectangle"
    Call hRechteckErstellen (10,60,70,90)
    WaitSlot (2000)
    ContextPositionAndSize
    WaitSlot (1000)
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    printlog "put x-axis postion of rectangle into clipboard"
    SetClipboard PositionX.GetText
    Kontext
    printlog "set slant angle to 5"
    Active.SetPage TabSchraegstellen
    Kontext "TabSchraegstellen"
    WaitSlot (1000)
    printlog "set radius to 5"
    Radius.SetText "5"
    r=Radius.GetText
    Winkel.SetText w
    TabSchraegstellen.OK
    WaitSlot (1000)
    ContextPositionAndSize
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    if PositionX <> GetClipboardText Then PrintLog "  Object changed position due to scaling"
    printlog "Before we change tab, we set the Position-marker back to checked "
    ProtectPosition.Check
    Kontext
    printlog "check slant angle and radius"
    Active.SetPage TabSchraegstellen
    Kontext "TabSchraegstellen"
    if Radius.GetText = r Then PrintLog "  Cornwer radius changed"
    if Winkel.GetText =w Then PrintLog "  Object rotated over 45°"
    TabSchraegstellen.OK
    Call hCloseDocument
    
endcase 'tiFormatPositionAndSize
'-------------------------------------------------------------------------------
testcase tiFormat3D_Effekte

    Dim Hor  as string
    Dim Vert  as string
    Dim Rund  as string
    Dim Skal  as string
    Dim Tief  as string
    Dim Winkel  as string                       ' get value for angle
    Dim X as string
    Dim y as integer
    Dim i as integer
    Dim Zaehler as integer
    Dim Posi as string

    Call hNewDocument
    Call sSelectEmptyLayout
    printlog "new document"
    '-----------------------------------RECTANGLE------------------------------------------------------
    printlog "create rectangle"
    Call hRechteckErstellen (20,20,60,70)
    WaitSlot (1000)
    printlog "get position and size of rectangle"
    ContextPositionAndSize                        
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    PositionX.SetText "5"
    X=PositionX.GetText
    printlog "--- created RECTANGLE; xPosition @ "+ x
    TabPositionAndSize.OK
    printlog "convert rectangle into 3d"
    ContextConvertInto3D                          
    WaitSlot (1000)
    Kontext "Drei_D_Effekte"
    if Drei_D_Effekte.exists(3) then
       qaerrorlog "   The 3D-Effects-dialogue was open. Check why."
    else
        printlog "open 3d flyer"
        Format3D_Effects
    end if
    try
        if Perspektive.isvisible then
            printlog "check perspective"
            Perspektive.Click
        else
            warnlog "why not in draw?"
        end if
    catch
        warnlog "something is wrong mit perspective"
    endcatch
    printlog "assign perspective"
    Zuweisen.Click
    Kontext "DocumentImpress"
    hTypeKeys "<MOD1 A>"
    printlog "check position and size"
    ContextPositionAndSize
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    if PositionX.GetText = X Then
        printlog "- Perspective acitvated x: "+PositionX.GetText
    else
        warnlog "- Perspective seems to have a problem, x value has changed!"
    end if
    TabPositionAndSize.OK
    Kontext "Drei_D_Effekte"
    Aktualisieren.Click
    Drei_D_Effekte.Close
    EditSelectAll
    printlog "delete rectangle"
    hTypeKeys "<DELETE>"

    '-----------------------------------RECTANGLE------------------------------------------------------
    printlog "create new rectangle"
    hRechteckErstellen (20,20,60,70)
    printlog "--- created RECTANGLE 2"
    printlog "open 3d flyer"
    Format3D_Effects                              
    Kontext "Drei_D_Effekte"
    printlog "create lathe object"
    Rotationskoerper.Click                        
    WaitSlot (1000)
    Kontext "DocumentImpress"
    printlog "check position and size"
    ContextPositionAndSize                        
        Kontext
        Active.SetPage TabPositionAndSize
        Kontext "TabPositionAndSize"
        if PositionX.GetText = X Then
            Warnlog "- Rectangle not converted into a lath object; x: "+PositionX.GetText
        else
            Printlog "- Creating a lath object seems to work"
        end if
        TabPositionAndSize.OK
    EditSelectAll
    printlog "delete object"
    hTypeKeys "<DELETE>"                         
    Sleep (1)
    '-----------------------------------RECTANGLE------------------------------------------------------
    printlog "create rectangle"
    hRechteckErstellen (20,20,60,70)
    printlog "--- created RECTANGLE 3"
    Kontext "Drei_D_Effekte"
    WaitSlot (1000)
    printlog "convert to 3d in 3d flyer"
    In_3D_Umwandeln.Click                        
    WaitSlot (2000)
    gMouseClick 40,40
    qaerrorlog "#i92910: automation: crash when executing tiFormat3D_Effekte in test g_format"
    'hOpenContextmenu
    'y = hMenuItemGetCount
    'MenuSelect (0)
    'gMouseClick 90,90
    'select case gApplication
    '    case "DRAW" :  if y = 13 then
	'                                Printlog "- 3D object created" 
    '                            else 
    '                                Warnlog " Context-menu-entry-number was wrong. Expected 13, but was " + y + "."
    '                            end if
    '    case else   :   if y = 15 then 
	'                                Printlog "- 3D object created" 
	'                           else 
    '			                       Warnlog " Context-menu-entry-number was wrong. Expected 15, but was " + y + "."
	'		                   end if
    'end select
    
    EditSelectAll
    printlog "delete object"
    hTypeKeys "<DELETE>"                        

    '-----------------------------------ZYLINDER------------------------------------------------------
    Sleep (1)
    printlog "create CYLINDER"
    WL_SD_Zylinder
    printlog "This affects the Vertical-value down to 15"
    gMousemove 30,30,80,80  
    Sleep (1)
    hTypeKeys "<ESC>"
    Kontext "Drei_D_Effekte"
    printlog " geometry"
    Geometrie.Click                              
    WaitSlot (1000)
    '   Endwinkel.More
    printlog "get value for vertical segments (default)"
    Vert=Vertikale_Segmente.GetText
    printlog "get value for horizontal segments (default)"
    Hor =Horizontale_Segmente.GetText
    printlog "get value for edge rounding"
    Rund=Kantenrundung.GetText
    printlog "get value for scale"
    Skal=Tiefenskalierung.GetText
    printlog "get value for depth"
    Tief=Tiefe.GetText
    printlog "get value for angle"
    Winkel=Endwinkel.GetText                    

    printlog "Check if every entry has a value <> "" -> an init value is shown."
    if Vert  ="" then warnlog "Vertical Segment   has no init value"
    if Hor   ="" then warnlog "Horizontal Segment has no init value"
    if Rund  ="" then warnlog "Rounded edges      has no init value"
    if Skal  ="" then warnlog "Scaled depth       has no init value"
    if Tief  ="" then warnlog "Depth              has no init value"
    if Winkel="" then printlog "Rotation angle    has no init value"

    printlog "Go back to the 3d-effects-dialogue and change the value for the cylinder"
    hTypeKeys "<ESC>"                 
    Kontext "Drei_D_Effekte"

    printlog "Check if value for vertical segments has changed in 3d controller"
    if Vertikale_Segmente.GetText = Vert Then
        Printlog "- Vertical segment value read from object's properties in the 3d flyer"
    else
        Warnlog "- 3D flyer could not read vertical segment information from object. It is = '"+ Vertikale_Segmente.GetText + "'  but should be = '" + Vert + "'"
    end if

    printlog "check if value for horizontal segments has changed"
    if Horizontale_Segmente.GetText = Hor Then          
        Printlog "- Horizontal segment value read from object's properties in the 3d flyer"
    else
        Warnlog "- 3D flyer not be able to read horizonal segment information from object. It is = '"+ Horizontale_Segmente.GetText + "'  but should be = '" + Hor + "'"
    end if

    printlog "check if value for edge rounding has changed"
    if Kantenrundung.GetText = Rund Then                
        Printlog "- Rounded edges works"
    else
        Warnlog "- Rounded edges value could not be read by the 3d flyer It is = '" + Kantenrundung.GetText + "'  but should be = '" + rund + "'"
    end if

    printlog "check value for scale"
    if Tiefenskalierung.GetText = Skal Then             
        Printlog "- Scaled depth works"
    else
        Warnlog "- Scale depth value could not be read by the 3d flyer It is = '" + Tiefenskalierung.GetText + "'  but should be = '" + skal + "'"
    end if

    printlog "check value for depth"
    if Tiefe.GetText = Tief Then                        
        Printlog "- Depth value could be read by the 3d flyer"
    else
        Warnlog "- Depth value of object could not be read by the flyer It is = '"+ Tiefe.GetText + "'  but should be = '" + tief + "'"
    end if

    printlog "check value for angle"
    if Endwinkel.GetText = Winkel Then                   
        Printlog "- Angle value could be read by the 3d flyer"
    else
        Warnlog "- Angle value is not correct"
    end if

    printlog "deselect object and check if 3d controller has default values again"
    hTypeKeys "<ESC>"                                    
    Kontext "Drei_D_Effekte"
    Sleep (2)
    Printlog "- check if 3D flyer goes back to default values if object is deselected"
    if Vertikale_Segmente.GetText <> Vert Then 	Warnlog " Vertical Segment   - Default value is wrong is: "+Vertikale_Segmente.GetText+"; should: " +vert
    if Horizontale_Segmente.GetText <> Hor Then 	Warnlog " Horizontal Segment - Default value is wrong"
    if Kantenrundung.GetText <> Rund Then 		Warnlog " Rounded edges        - Default value is wrong"
    if Tiefenskalierung.GetText <> Skal Then 		Warnlog " Scaled depth     - Default value is wrong"
    if Endwinkel.GetText <> Winkel Then 		Warnlog " Rotation angle            - Default value is wrong"
    if Tiefe.GetText<>Tief Then 			Warnlog " Depth                - Default value is wrong"

    hTypeKeys "<TAB>"
    Kontext "Drei_D_Effekte"
    Sleep (2)
    printlog "change horizontal and vertical segments"
    Horizontale_Segmente.SetText "32"
    Vertikale_Segmente.SetText "32"
    Kantenrundung.SetText "100"
    Endwinkel.Settext "300"
    Tiefenskalierung.SetText "500"
    Tiefe.SetText "100"
    printlog "assign changes"
    Zuweisen.Click

    printlog "check value changes"
    WaitSlot (1000)                                            
    if Vertikale_Segmente.GetText = Vert Then Warnlog "- Default value is wrong"
    if Horizontale_Segmente.GetText = Hor Then Warnlog "- Default value is wrong"
    if Kantenrundung.GetText = Rund Then Warnlog "- Default value is wrong"
    if Tiefenskalierung.GetText = Skal Then Warnlog "- Default value is wrong"
    if Endwinkel.GetText = Winkel Then Warnlog "- Default value is wrong"
    if Tiefe.GetText=Tief Then Warnlog "- Default value is wrong"
    EditSelectAll
    printlog "delete cylinder"
    hTypeKeys "<DELETE>"                               

    '-----------------------------------ZYLINDER------------------------------------------------------
    printlog "create new cylinder"
    WL_SD_Zylinder
    gMouseMove (20,20,60,60)
    WaitSlot (3000)
    Kontext "Drei_D_Effekte"
    Printlog "- Change basic geometric parameters for the object"
    Printlog "Segments : Horizontal: 8; Vertical: 8; Rounded edges: 50 %"
    printlog "changing values for cylinder"
    Horizontale_Segmente.SetText "8"
    Vertikale_Segmente.SetText "8"
    Kantenrundung.SetText "50 %"
    Sleep (1)
    Zuweisen.Click
    WaitSlot (2000)
    gMouseClick 90,90
    Sleep (2)
    printlog "select object"
    EditSelectAll                                    
    WaitSlot (1000)
    printlog "control if changes are done"
    Kontext "Drei_D_Effekte"                       
    if Horizontale_Segmente.GetText <> "8" Then Warnlog "- Horizontal segment count is not correct "+Horizontale_Segmente.GetText
    if Vertikale_Segmente.GetText <> "8" Then Warnlog "- Vertical segment count is not correct "+Vertikale_Segmente.GetText
    if Kantenrundung.GetText <> "50 %" Then Warnlog "- Value for rounded edges is not correct "+Kantenrundung.GetText
    Darstellung.Click
    printlog "get number of pssibole render methods"
    Zaehler=Modus.GetItemCount                  
    for i =1 to Zaehler
        Modus.Select i
        printlog "assign every render method"
        Zuweisen.Click                        
        gMouseClick 90,90
        Sleep (1)
        printlog "select object"
        EditSelectAll                                    
        Sleep (1)
        Kontext "Drei_D_Effekte"
        printlog "check if render method has changed"
        if Modus.GetSelIndex <> i Then Warnlog "- Value for render modus did not change"   
    next i
    Printlog "- Check 3d shadow and surface angle"
    Drei_DSchatten.Click
    Papierneigung.SetText "90 degree(s)"
    vert = Papierneigung.getText
    printlog "assign shadow"
    Zuweisen.Click
    printlog "deselect object"
    gMouseClick 90,90                                     
    WaitSlot (1000)
    printlog "select object"
    EditSelectAll
    printlog "check value changes"
    Kontext "Drei_D_Effekte"
    if Papierneigung.GetText <> vert Then Warnlog "- Value for surface angle is not correct"
    printlog "set distance to 10"
    Entfernung.SetText "10"
    Sleep (1)
    hor = Entfernung.getText
    Sleep (1)
    Zuweisen.Click
    if Entfernung.GetText <> hor Then Warnlog "- Value for distance is not correct; should: "+hor+"; is: "+Entfernung.GetText
    WaitSlot (2000)
    hTypeKeys "<TAB>"
    Sleep (2)
    ContextPositionAndSize
    WaitSlot (2000)
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    PositionX.SetText "5,00cm"
    Posi=PositionX.GetText
    TabPositionAndSize.OK
    WaitSlot (1000)
    Kontext "Drei_D_Effekte"

    printlog "set focval length to 50"
    Brennweite.SetText "50"                             
    Zuweisen.Click
    printlog "check change in position of the object"
    ContextPositionAndSize                            
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    If PositionX.GetText = Posi Then
        printlog "- Raising value for focal length has not changed optical appearence of object, ok."
    else
        warnlog "Problem with focal lenght here!"
    endif
    TabPositionAndSize.OK
    WaitSlot (1000)
    Kontext "Drei_D_Effekte"
    Brennweite.SetText "5"
    Zuweisen.Click
    WaitSlot (1000)
    EditSelectAll
    WaitSlot (3000)
    ContextPositionAndSize
    WaitSlot (1000)
        Kontext
        Sleep (1)
        Active.SetPage TabPositionAndSize
        Kontext "TabPositionAndSize"
        Sleep (2)
        if PositionX.GetText = Posi Then 
            printlog "- Lowering focus length value has not changed optical appearence of object, ok."
        else 
            warnlog "problem with focus, check!"
        endif
        TabPositionAndSize.OK
    Kontext "Drei_D_Effekte"
    printlog "close document"
    Drei_D_Effekte.Close
    Call hCloseDocument
    
endcase 'tiFormat3D_Effekte
'------------------------------------------------------------------
testcase tdFormatTemplates

    qaerrorlog "Not yet ready."
    goto endsub

    dim Zaehler as integer
    dim i as integer
    dim x as integer
    dim iWaitIndex as integer
    dim sStyleName as string
    dim bDouble as boolean
    dim sTemp(2) as string

    printlog "new impress document"
    Call hNewDocument

    WaitSlot (5000)
    printlog "format-Style catalog!"
    FormatStylesCatalog
    iWaitIndex = 0
    do while Vorlagenkatalog.NotExists AND iWaitIndex < 10
        sleep (1)
        iWaitIndex = iWaitIndex + 1
    loop
    
    Kontext "Vorlagenkatalog"
    printlog "get item count for templates"
    Vorlagen.GetItemCount
    printlog "select template 2"
    Vorlagen.Select 2

    printlog "check if area changes when selecting a template"
    if Bereich.GetItemCount = Vorlagen.GetItemCount Then
        PrintLog " - Area did changed"
    else
        WarnLog " Number of entrees should be 2, but it is " + Bereich.GetItemCount
    end if
    
    Zaehler = Ansicht.GetItemCount              'Vorlagenliste.GetItemCount

    for i = 1 to Zaehler
        Ansicht.TypeKeys "<DOWN>" ,i
        SetClipboard Ansicht.GetSelText
        WaitSlot (2000)
        Vorlagen.Select 1
        PrintLog   "    "+i+": Offset: '" + Vorlagen.GetSelText+"'"
        if Bereich.GetItemCount=Vorlagen.GetItemCount Then WarnLog " Range has not changed"
        Vorlagen.Select 2
        WaitSlot (1000)
        Ansicht.TypeKeys "<HOME>"
        WaitSlot (1000)
        Ansicht.Select 1
        WaitSlot (1000)
        PrintLog "     : Offset: '"+Vorlagen.GetSelText + "'; Opinion: '" + Ansicht.GetSelText+"'"
        Vorlagenkatalog.OK
        FormatStylesCatalog
        Kontext "Vorlagenkatalog"
    next i
    
    Vorlagen.Select 2
    WaitSlot (2000)
    printlog "select first template"
    Vorlagen.Select 1
    if Neu.IsEnabled =False Then WarnLog " Button must be choosable, but is not"
    Ansicht.Select 1
    WaitSlot (1000)
    printlog "create new template"
    Neu.Click
    Kontext
    Active.SetPage TabVerwalten
    Kontext "TabVerwalten"
    sStyleName = VorlagenName.GetText
    printlog "   created new style: '"+sStyleName +"'"
    TabVerwalten.OK
    PrintLog " New template created"
    Kontext "Vorlagenkatalog"
    WaitSlot (1000)
     ' find newly created style, because they are alphabetical ordered :-( i have to check every entry :-(((((
       Zaehler = Ansicht.GetItemCount
       i=1
       Ansicht.TypeKeys "<home>"
       bDouble = false
       sTemp(1) = Ansicht.GetText
       while (sTemp(1) <> sStyleName) AND (i <= Zaehler)
          Ansicht.TypeKeys "<DOWN>"
          inc i
          if (i > Zaehler) then ' here we are only, if we didn't found 'our' template
             if bDouble then
                qaErrorLog "There are Double Entries in the list! #111652#"
             else
                 warnlog " style not found :-(" ' print out the styles...
             end if
            Ansicht.TypeKeys "<home>"
             for x=0 to zaehler
                printlog "             "+(x+1)+": "+Ansicht.GetText
                if (Loeschen.IsEnabled) then qaErrorLog "^ is delete-able :-)"
                Ansicht.TypeKeys "<DOWN>"
             next x
          end if
          sTemp(2) = sTemp(1)
          sTemp(1) = Ansicht.GetText
          if (sTemp(1) = sTemp(2)) then
                bDouble = true
          end if
       wEnd
       if bDouble then
           i=1
           Ansicht.TypeKeys "<end>"
           sTemp(1) = Ansicht.GetText
           while (sTemp(1) <> sStyleName) AND (i <= Zaehler)
              printlog "             "+(i)+": "+sTemp(1)
              Ansicht.TypeKeys "<UP>"
              inc i
              if (i > Zaehler) then ' here we are only, if we didn't found 'our' template
                warnlog " style not found :-(" ' print out the styles...
                Ansicht.TypeKeys "<end>"
                 for x=0 to zaehler
                    printlog "             "+(x+1)+": "+Ansicht.GetText
                    if (Loeschen.IsEnabled) then qaErrorLog "^ is delete-able :-)"
                    Ansicht.TypeKeys "<UP>"
                 next x
              end if
              sTemp(1) = Ansicht.GetText
           wEnd
       end if
       if (i <= Zaehler) then  printlog "   found created entry for deleting it :-)"
    ' Ansicht.TypeKeys "<DOWN>" ,( i + 1 )           printlog "control if template list has created template included"
      WaitSlot (2000)
     try
        printlog "delete template"
        Loeschen.Click                  
        WaitSlot (1000)
        Kontext "Active"
        if active.exists then
           printlog "ac: "+active.gettext
           try
              Active.Yes
           catch
              warnlog "active-Yes doesn't exist :-( WorkInProgress TBO"
           endcatch
        end if
        PrintLog " Delete did work"
        Kontext "Vorlagenkatalog"
     catch
        WarnLog " Only a new created template can be deleted"
     endcatch
      WaitSlot (1000)
    UseBindings
     Ansicht.Select 1
      WaitSlot (1000)
      printlog "modify template"
    Aendern.Click                    
     WaitSlot (2000)
     Kontext
     Active.SetPage TabSchatten
     Kontext "TabSchatten"
     WaitSlot (2000)
     TabSchatten.Cancel
     WaitSlot (2000)
     Kontext "Vorlagenkatalog"

     printlog "organize templates"
     Verwalten.Click                 
     WaitSlot (2000)
     Kontext "DVVerwalten"
     DVVerwalten.OK
     WaitSlot (1000)
     try
       Kontext "Vorlagenkatalog"
       Vorlagenkatalog.OK
     catch
        Warnlog " - Style Catalog can't close unless the Template Management-dialog is closed"
     endcatch
     PrintLog " FormatStylesCatalog finished"        
     WaitSlot (2000)
     printlog "format modify layout"
     FormatModifyLayout                 
     WaitSlot (2000)
     Kontext "Seitenvorlage"
     printlog "uncheck switch background page"
     if HintergrundseiteAustauschen.IsChecked=True Then HintergrundseiteAustauschen.UnCheck   
    printlog "load"
    Laden.Click
     WaitSlot (2000)
     Kontext "Neu"
     Neu.Cancel
     printlog "choose a page layout"
     Kontext "Seitenvorlage"               
     'Waehlen.Click
     printlog "close dialog"
     Seitenvorlage.OK                

     printlog "close document"
     Call hCloseDocument                

endcase 'tdFormatTemplates
'-------------------------------------------------------------------------------
testcase tiFormatAufzaehlungszeichen
    
    Dim i as Integer
    PrintLog "- Format/Numbering and Bullets"
    
    Call hNewDocument
    Call sSelectEmptyLayout
    WaitSlot (2000)
    Kontext "DocumentImpress"
    hTextrahmenErstellen ("Hello <Return><Return>",20,20,60,60)
    for i=1 to 8
        Kontext "DocumentImpress"
        gMouseClick 30,30
        EditSelectAll
        WaitSlot (1000)
        FormatNumberingBulletsDraw
        WaitSlot (2000)
        Kontext
        Active.SetPage TabBullet
        Kontext "TabBullet"
        Auswahl.TypeKeys "<Right>", i
        TabBullet.OK
        Kontext "DocumentImpress"
    next i
    for i=1 to 8
        Kontext "DocumentImpress"
        gMouseClick 30,30
        EditSelectAll
        WaitSlot (1000)
        FormatNumberingBulletsDraw
        Kontext
        Active.SetPage TabNumerierungsart
        Kontext "TabNumerierungsart"
        Auswahl.TypeKeys "<Right>", i
        TabNumerierungsart.OK
        Kontext "DocumentImpress"
    next i
    for i=1 to 8
        Kontext "DocumentImpress"
        gMouseClick 30,30
        EditSelectAll
        FormatNumberingBulletsDraw
        Kontext
        Active.SetPage TabNumerierungsart
        Kontext "TabNumerierungsart"
        Auswahl.TypeKeys "<Right>", i
        TabNumerierungsart.OK
        Kontext "DocumentImpress"
    next i
    WaitSlot (3000)
    for i=1 to 29
        WaitSlot (3000)
        FormatNumberingBulletsDraw
        WaitSlot (3000)
        Kontext
        Active.SetPage TabGrafiken
        Kontext "TabGrafiken"
        sleep 1
        Auswahl.TypeKeys "<Right>", i
        TabGrafiken.OK
        Kontext "DocumentImpress"
    next i
    
    gMouseClick 90,90
    
    Call hCloseDocument

endcase 'tiFormatAufzaehlungszeichen
