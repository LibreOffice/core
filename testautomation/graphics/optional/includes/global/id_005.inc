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
'* Owner : wolfram.garten@sun.com
'*
'* short description :
'*
'***********************************************************************************
' #1 tiFormatDefault
' #1 tiFormatLine
' #1 tdFormatArea
' #1 tiFormatText
' #1 tiFormatPositionAndSize
' #1 tiFormatCharacter
' #1 tiFormatControlForm
' #1 tiFormatDimensions
' #1 tiFormatConnector
' #1 tiFormat3D_Effects
' #1 tiFormatNumberingBullets
' #1 tiFormatCaseCharacter
' #1 tiFormatParagraph
' #1 tiFormatPage
' #1 tiFormatStylesAndFormatting
' #1 tiFormatStylesSlideDesign
' #1 tiFormatFontwork
' #1 tiFormatGroup
' #1 hWalkTheStyles
'\**********************************************************************************

testcase tiFormatDefault

    Call hNewDocument
    gMouseClick 50,50
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    FormatStandardDraw
    Call hCloseDocument
endcase

testcase tiFormatLine
    hNewDocument
    gMouseClick 50,50
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    FormatLine
    Kontext
    Messagebox.SetPage TabLinie
    kontext "TabLinie"
    Call DialogTest ( TabLinie )

    Kontext
    Messagebox.SetPage TabLinienstile
    kontext "TabLinienstile"
    Call DialogTest ( TabLinienstile )
    Hinzufuegen.click
    Kontext "NameDLG"
    Call DialogTest ( NameDlg )
    NameDlg.Cancel

    kontext "TabLinienstile"
    Aendern.Click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    NameDlg.Cancel
    kontext "TabLinienstile"
    Loeschen.Click
    Kontext "Messagebox"
    Messagebox.no

    kontext "TabLinienstile"
    Oeffnen.click
    Kontext "OeffnenDLG"
    call Dialogtest (OeffnenDLG)
    OeffnenDLG.Cancel
    kontext "TabLinienstile"
    Speichern.click
    Kontext "SpeichernDLG"
    call Dialogtest (SpeichernDLG)
    SpeichernDLG.Cancel
    Kontext
    Messagebox.SetPage TabLinienenden
    kontext "TabLinienenden"
    Call DialogTest ( TabLinienenden )
    Hinzufuegen.Click
    Kontext "NameDLG"
    Call DialogTest ( NameDlg )
    NameDlg.Cancel

    kontext "TabLinienenden"
    Aendern.Click
    Kontext "Messagebox"
    try
        Messagebox.OK
    catch
        'print "TabLinienenden"
    endcatch

    kontext "NameDlg"
    Call DialogTest ( NameDlg )
    NameDlg.Cancel

    kontext "TabLinienenden"
    Loeschen.Click
    Kontext "Messagebox"
    Messagebox.no

    kontext "TabLinienenden"
    Oeffnen.click
    Kontext "OeffnenDLG"
    call Dialogtest (OeffnenDLG)
    OeffnenDLG.Cancel
    kontext "TabLinienenden"
    Speichern.click
    Kontext "SpeichernDLG"
    call Dialogtest (SpeichernDLG)
    SpeichernDlg.Cancel
    kontext "TabLinienenden"
    TabLinienenden.cancel
    Call hCloseDocument
endcase

testcase tdFormatArea
    Call hNewDocument
    gMouseClick 50,50
    Call hRechteckErstellen (15,15,65,65)
    gMouseClick 30,30
    FormatArea
    WaitSlot (1000)
    Kontext
    Messagebox.SetPage TabArea
    Kontext "TabArea"
    Call DialogTest ( TabArea )
    Kontext
    Messagebox.SetPage TabSchatten
    kontext "TabSchatten"
    Anzeigen.Check
    Call DialogTest ( TabSchatten )
    Kontext
    Messagebox.SetPage TabFarben
    kontext "TabFarben"
    Farbe.select 1
    Farbmodell.Select 1
    Call DialogTest ( TabFarben,1 )
    Farbmodell.Select 2
    Call DialogTest ( TabFarben,2 )

    Hinzufuegen.click
    Kontext "Messagebox"
    Messagebox.OK
    kontext "NameDlg"
    Call DialogTest ( NameDlg )
    NameDlg.Cancel
    kontext "TabFarben"
    Loeschen.click
    Kontext "Messagebox"
    Messagebox.no

    kontext "TabFarben"
    sleep 1
    Speichern.click
    Kontext "SpeichernDLG"
    call Dialogtest (SpeichernDLG)
    SpeichernDlg.Cancel
    sleep 1

    kontext "TabFarben"
    Oeffnen.click
    Kontext "OeffnenDLG"
    call Dialogtest (OeffnenDLG)
    OeffnenDLG.Cancel
    Kontext "TabFarben"

    Kontext
    Messagebox.SetPage TabFarbverlaeufe
    kontext "TabFarbverlaeufe"
    Call DialogTest ( TabFarbverlaeufe )

    Hinzufuegen.click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    NameDlg.Cancel

    kontext "TabFarbverlaeufe"
    Aendern.Click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    NameDlg.Cancel

    kontext "TabFarbverlaeufe"
    loeschen.click
    try
        kontext "Messagebox"
        Messagebox.no
    catch
        warnlog "nobody cares about deleting a gradient :-("
    endcatch

    kontext "TabFarbverlaeufe"
    Oeffnen.click
    Kontext "OeffnenDLG"
    call Dialogtest (OeffnenDLG)
    OeffnenDLG.Cancel
    kontext "TabFarbverlaeufe"
    Speichern.click
    Kontext "SpeichernDLG"
    call Dialogtest (SpeichernDLG)
    SpeichernDLG.Cancel

    Kontext
    Messagebox.SetPage TabSchraffuren
    kontext "TabSchraffuren"
    Call DialogTest ( TabSchraffuren)

    Hinzufuegen.click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    NameDlg.Cancel

    kontext "TabSchraffuren"
    Aendern.Click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    NameDlg.Cancel

    kontext "TabSchraffuren"
    Loeschen.click
    kontext "Messagebox"
    Messagebox.no

    kontext "TabSchraffuren"
    Oeffnen.click
    Kontext "OeffnenDLG"
    call Dialogtest (OeffnenDLG)
    OeffnenDLG.Cancel
    kontext "TabSchraffuren"
    Speichern.click
    Kontext "SpeichernDLG"
    call Dialogtest (SpeichernDLG)
    SpeichernDLG.Cancel

    Kontext
    Messagebox.SetPage TabBitmap
    kontext "TabBitmap"
    Call DialogTest ( TabBitmap )
    zurueck.click
    sleep 1
    hinzufuegen.click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    NameDlg.cancel

    kontext "TabBitmap"
    try
        Aendern.Click
        Kontext "NameDlg"
        Call DialogTest ( NameDlg )
        NameDlg.cancel
    catch
        WarnLog "Control is disabled - modify bitmap"
    endcatch
    kontext "TabBitmap"
    Import.Click
    try
        Kontext "GrafikEinfuegenDlg"
        Call DialogTest ( GrafikEinfuegenDlg )
        Kontext "GrafikEinfuegenDlg"
        GrafikEinfuegenDlg.Cancel
    catch
        Warnlog "Insert graphic does not work"
    endcatch

    kontext "TabBitmap"
    loeschen.click
    kontext "Messagebox"
    Messagebox.no

    kontext "TabBitmap"
    Oeffnen.click
    Kontext "OeffnenDLG"
    call Dialogtest (OeffnenDLG)
    OeffnenDLG.Cancel
    kontext "TabBitmap"
    Speichern.click
    Kontext "SpeichernDLG"
    call Dialogtest (SpeichernDLG)
    SpeichernDLG.Cancel
    kontext "TabBitmap"
    TabBitmap.Cancel
    Call hCloseDocument
endcase

testcase tiFormatText
    Call  hNewDocument
    FormatTextDraw
    Kontext
    Messagebox.SetPage TabText
    Kontext "TabText"
    DialogTest ( TabText )
    Kontext
    Messagebox.SetPage TabLauftext
    Kontext "TabLauftext"
    DialogTest ( TabLauftext )
    TabLauftext.Cancel
    Call hCloseDocument
endcase

testcase tiFormatPositionAndSize
    Call  hNewDocument
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    ContextPositionAndSize
    Kontext
    Messagebox.setpage TabPositionAndSize
    Kontext "TabPositionAndSize"
    call Dialogtest ( TabPositionAndSize )
    kontext "PositionPosition"
    PositionPosition.TypeKeys ("<right>", 2)
    kontext "SizePosition"
    SizePosition.TypeKeys ("<down>", 2)
    Kontext
    Messagebox.setPage TabDrehung
    Kontext "TabDrehung"
    call Dialogtest ( TabDrehung )
    Kontext
    Messagebox.setpage TabSchraegstellen
    Kontext "TabSchraegstellen"
    call Dialogtest ( TabSchraegstellen )
    TabSchraegstellen.cancel
    Call hCloseDocument
endcase

testcase tiFormatCharacter
    Call  hNewDocument
    FormatCharacter
    WaitSlot (1000)
    Kontext
    Messagebox.SetPage TabFont
    kontext "TabFont"
    sleep 1
    Call DialogTest ( TabFont )
    Kontext
    Messagebox.SetPage TabFontEffects
    kontext "TabFontEffects"
    sleep 1
    Call DialogTest ( TabFontEffects )
    sleep 1
    Kontext
    Messagebox.SetPage TabFontPosition
    Kontext "TabFontPosition"
    sleep 1
    Call DialogTest ( TabFontPosition )
    sleep 2
    TabFontPosition.Cancel
    Call hCloseDocument
endcase

testcase tiFormatControlForm

    printlog "testcase: check if controls are available"

    printlog "open new document"
    Call  hNewDocument

    'click in the document to get the focus into the document
    if ( UCase(gApplication) = "DRAW" ) then
        Kontext "DocumentDraw"
        DocumentDraw.MouseDown(50,50)
        DocumentDraw.MouseUp(50,50)
    else 'Impress
        Kontext "DocumentImpress"
        DocumentImpress.MouseDown(50,50)
        DocumentImpress.MouseUp(50,50)
    endif

    printlog "open the form controls toolbar"
    call hToolbarSelect("FormControls",true)

    kontext "FormControls"
    printlog "insert a PushButton"
    Pushbutton.Click
    Sleep 1
    gMouseMove (50, 20,70, 40)

    printlog "open the control properties dialog"
    FormatControl

    Kontext "ControlPropertiesDialog"
    WaitSlot (1000)
    printlog "close the control properties dialog"
    ControlPropertiesDialog.Close

    printlog "open the form properties dialog"
    FormatForm
    Kontext "ControlPropertiesDialog"
    WaitSlot (1000)
    printlog "close the form properties dialog"
    ControlPropertiesDialog.Close

    printlog "close the form control toolbar"
    call hToolbarSelect("FormControls",false)

    printlog "close application"
    Call hCloseDocument

endcase

testcase tiFormatDimensions
    Call  hNewDocument
    FormatDimensioning
    Kontext "Bemassung"
    DialogTest ( Bemassung )
    Bemassung.Cancel
    Call hCloseDocument
endcase

testcase tiFormatConnector
    Call  hNewDocument
    FormatConnector
    Kontext "Verbinder"
    DialogTest ( Verbinder )
    Verbinder.Cancel
    Call hCloseDocument
endcase

testcase tiFormat3D_Effects
    Call  hNewDocument
    Format3D_Effects
    Kontext "Drei_D_Effekte"
    Call DialogTest ( Drei_D_Effekte,1 )
    Geometrie.Click
    Call DialogTest ( Drei_D_Effekte,2 )
    Darstellung.Click
    Call DialogTest ( Drei_D_Effekte,3 )
    Beleuchtung.Click
    Call DialogTest ( Drei_D_Effekte,4 )
    Texturen.Click
    Call DialogTest ( Drei_D_Effekte,5 )
    Material.Click
    Call DialogTest ( Drei_D_Effekte,6 )
    Kontext "Drei_D_Effekte"
    Drei_D_Effekte.Close
    Call hCloseDocument
endcase

'---------------------------------------------------------------------------------------

testcase tiFormatNumberingBullets
    Call hNewDocument
    WaitSlot (2000)
    FormatNumberingBulletsDraw
    WaitSlot (2000)
    Kontext
    Messagebox.SetPage TabBullet
    Kontext "TabBullet"
    Call DialogTest ( TabBullet )
    Kontext
    Messagebox.SetPage TabNumerierungsart
    Kontext "TabNumerierungsart"
    Call DialogTest ( TabNumerierungsart )
    Kontext
    Messagebox.SetPage TabGrafiken
    Kontext "TabGrafiken"
    Call DialogTest ( TabGrafiken )
    Kontext
    Messagebox.SetPage TabPositionNumerierung
    Kontext "TabPositionNumerierung"
    Call DialogTest ( TabPositionNumerierung )
    Kontext
    Messagebox.SetPage TabOptionenNumerierung
    Kontext "TabOptionenNumerierung"
    Call DialogTest ( TabOptionenNumerierung )
    Numerierung.Select 9 ' last one always ? -> graphics
    TabOptionenNumerierung.MouseDown 50,60
    TabOptionenNumerierung.MouseUp 50,60
    Auswahl.TypeKeys "<SPACE>"
    hMenuSelectNr (1)
    sleep 3
    Kontext "OeffnenDlg"
    OeffnenDlg.Cancel
    sleep 1
    sleep 1
    Kontext
    Messagebox.SetPage TabOptionenNumerierung
    Kontext "TabOptionenNumerierung"
    sleep 1
    try
        Auswahl.TypeKeys "<SPACE>"
        hMenuSelectNr (2)
        hMenuSelectNr (3)
        Sleep 2
    catch
        warnlog "couldn't do something :-) (1)"
        Exceptlog
        Call hMenuClose
    endcatch
    TabOptionenNumerierung.Cancel
    sleep 1
    Call hCloseDocument
endcase

'---------------------------------------------------------------------------------------

testcase tiFormatCaseCharacter
    Call  hNewDocument
    Call hTextrahmenErstellen ("testit",20,20,50,30)
    sleep 1
    hTypeKeys "<left>"

    FormatChangeCaseUpper
    WaitSlot (1000)
    FormatChangeCaseLower
    WaitSlot (2000)
    if bAsianLan then
        if not gAsianSup then
            qaerrorlog "This is an asian language-office, but asian support was disabled in a previous test?"
        end if
        try
            FormatChangeCaseHalfWidth
        catch
            Warnlog "Format / Change Case / Half Width does not work."
        endcatch
        WaitSlot (1000)
        try
            FormatChangeCaseFullWidth
        catch
            Warnlog "Format / Change Case / Full Width does not work!"
        endcatch
        sleep 1
        try
            FormatChangeCaseHiragana
        catch
            Warnlog "Format / Change Case / Hiragana does not work."
        endcatch
        sleep 1
        try
            FormatChangeCaseKatagana
        catch
            Warnlog "Format / Change Case / Katagana does not work."
        endcatch
    end if
    Call hCloseDocument
endcase

'---------------------------------------------------------------------------------------

testcase tiFormatParagraph
    Call  hNewDocument
    FormatParagraph
    Kontext
    Messagebox.SetPage TabEinzuegeUndAbstaende
    kontext "TabEinzuegeUndAbstaende"
    Call DialogTest ( TabEinzuegeUndAbstaende )
    Kontext
    Messagebox.SetPage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"
    Call DialogTest ( TabAusrichtungAbsatz )
    Kontext
    Messagebox.SetPage TabTabulator
    kontext "TabTabulator"
    Call DialogTest ( TabTabulator )
    TabTabulator.Cancel
    Call hCloseDocument
endcase

'---------------------------------------------------------------------------------------

testcase tiFormatPage
    Call hNewDocument
    FormatSlideDraw
    kontext
    if Messagebox.exists (5) then
        Messagebox.SetPage TabSeite
        Kontext "TabSeite"
        if TabSeite.exists (5) then
            Call Dialogtest (TabSeite)
        else
            warnlog "nope :-(1"
        endif
        sleep 1
        kontext
        Messagebox.SetPage TabArea
        sleep 1
        kontext
        if messagebox.GetRT = 304 then
            printlog "active about pagesize != printersettings, will say NO: " + Messagebox.GetText
            try
                Messagebox.No
            catch
                warnlog messagebox.getText
                Messagebox.ok ' should be Error loading BASIC of document ##?
                kontext
                if messagebox.GetRT = 304 then
                    try
                        warnlog messagebox.getText
                        Messagebox.ok
                    catch
                        printlog "not expected state."
                    endcatch
                endif
            endcatch
        endif
        sleep 1
        kontext
        Messagebox.SetPage TabArea
        Kontext "TabArea"
        if TabArea.exists (5) then
            Call Dialogtest (TabArea)
        endif
        sleep 1
        TabArea.Cancel
    else
        warnlog "FormatPage doesn't come up with dialog :-("
    endif
    Call hCloseDocument
endcase

'---------------------------------------------------------------------------------------

testcase tiFormatStylesAndFormatting
    Dim sTemp as String
    dim sSettings(20,3) ' Control_name; control_type; value
    dim i as integer
    dim abctemp

    Call  hNewDocument
    sleep 5

    hTextrahmenErstellen ("I love Wednesdays...",20,20,80,40)
    sleep 1
    printlog "Checking if TextObjectBar is up"
    Kontext "TextObjectbar"
    if TextObjectbar.Exists Then
        printlog "TextObjectbar.Exists = " + TextObjectbar.Exists
    else
        ViewToolbarsTextFormatting
    endif
    FormatStylist
    WaitSlot (1000)
    Kontext "Stylist"
    if (Stylist.NotExists) then
        qaErrorLog "There is no stylist open, trying again now"
        FormatStylist
    end if
    WaitSlot (1000)
    Vorlagenliste.TypeKeys "<End>"
    Vorlagenliste.TypeKeys "<Up>"
    Vorlagenliste.TypeKeys "<Up>"
    sleep 1
    Vorlagenliste.OpenContextMenu
    sleep 1
    hMenuSelectNr (1)
    sleep 1

    Kontext
    if Messagebox.exists (5) then
        try
            Messagebox.SetPage TabVerwalten
            Kontext "TabVerwalten"
            TabVerwalten.TypeKeys "<TAB>"
            VorlagenName.setText("1Test")
            sTemp = VorlagenName.getText
            VerknuepftMit.getSelText
            Bereich.getSelText
            TabVerwalten.OK
        catch
            warnlog "Under Gnome we have a focus problem here."
        endcatch
    end if
    sleep 1
    Kontext "Stylist"
    Vorlagenliste.TypeKeys "<Home>"  'to go to the style we've created ourselves.
    sleep 1
    Vorlagenliste.OpenContextMenu
    sleep 1
    hMenuSelectNr (2) 'modify...
    sleep 1
    Kontext
    if Messagebox.exists (5) then
        try
            Messagebox.SetPage TabVerwalten
            Kontext "TabVerwalten"
            VorlagenName.setText("2Test")
            TabVerwalten.OK
        catch
            warnlog "Under Gnome we have a focus problem here."
        endcatch
    end if

    sleep 3
    Kontext "Stylist"
    Vorlagenliste.TypeKeys "<Home>"  'to go to the style we've created ourselves.
    sleep 1
    try
        Vorlagenliste.TypeKeys "<Delete>" 'To delete the style.
        Kontext "Active" 'do you really wish to delete?
        Active.YES
        sleep 2
    catch
        Warnlog "Couldnt delete the new Style, or maybe wrong position?"
    endcatch
    Kontext "Stylist"
    if (Stylist.NotExists) then
        ErrorLog "There was no Stylist open, should be."
    else
        if lcase(gPlatform) = "osx" then
            hTypekeys "<mod1 t>"
        else
            hTypekeys "<F11>"
        endif
        Kontext "Stylist"
        if (Stylist.Exists) then
            ErrorLog "The Stylist should be closed now."
        endif
    endif
    Call hCloseDocument
endcase

'---------------------------------------------------------------------------------------

testcase tiFormatFontwork
    Call hNewDocument
    Call hTextrahmenErstellen ("Flightplanning via www.aua.com is hard!",20,20,50,30)
    sleep 1
    FormatFontwork
    Kontext "FontWork"
    if FontWork.exists (5) then
        DialogTest ( FontWork )
        sleep 1
        FontWork.Close
    else
        warnlog "FontWork didn't came up :-("
    endif
    Call hCloseDocument
endcase

'---------------------------------------------------------------------------------------

testcase tiFormatGroup
    Call hNewDocument
    hRechteckErstellen ( 10, 10, 20, 20 )
    hRechteckErstellen ( 30, 30, 40, 40 )
    EditSelectAll
    FormatGroupDraw
    WaitSlot (1000)
    FormatEditGroupDraw
    WaitSlot (1000)
    FormatExitGroupDraw
    WaitSlot (1000)
    FormatUngroupDraw
    WaitSlot (1000)
    Call hCloseDocument
endcase

'---------------------------------------------------------------------------------------

testcase tiFormatStylesSlideDesign
    ' create recktanglr; click outside ?
    Call hNewDocument
    WaitSlot (3000)
    FormatModifyLayout ' is OK : Format->Styles->Slide Design; 27064; SID_PRESENTATION_LAYOUT
    WaitSlot (1000)
    Kontext "Seitenvorlage"
    Call DialogTest ( Seitenvorlage )
    HintergrundseiteAustauschen.check
    DeleteUnusedBackgrounds.check
    Laden.Click
    kontext "Neu"
    Zusaetze.click
    sleep 1
    kontext "Neu"
    try
        Vorschau.check
    catch
        printlog "Preview wasn't checkable :-( hopfully now:"
        Zusaetze.click
        sleep 1
        Vorschau.check
        printlog "... OK :-)"
    endcatch
    Neu.cancel
    Kontext "Seitenvorlage"
    Seitenvorlage.Cancel
    sleep 2
    Call hCloseDocument
endcase

'---------------------------------------------------------------------------------------
