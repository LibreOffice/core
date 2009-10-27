'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: id_011.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:41 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/******************************************************************************
'*
'* Owner : wolfram.garten@sun.com
'*
'* short description :
'*
'\******************************************************************************

testcase tiTextToolbar

    Dim iWaitIndex as integer
    Dim Zufall as integer
    Dim i as integer

    Call hNewDocument

    hTextrahmenErstellen ("This is a Text, which will be formatted in several ways", 20,20,40,80)
    sleep 2
    EditSelectAll
    sleep 2
    Kontext "TextObjectbar"
    if TextObjectbar.Exists <> TRUE then  'isVisible
        ViewToolbarsTextFormatting
    endif
    WaitSlot (3000)    'sleep (3)
    Kontext "TextObjectbar"
    Printlog "- choose random font"
    randomize
    Zufall=((2*Rnd)+1)      ' wird nicht auf den verfuegbaren bereich getreckt :-( TODO TBO!
    Schriftart.GetItemcount
    Schriftart.GetSelText
    Schriftart.Select (Zufall)
    Printlog Schriftart.GetSelText + " chosen"

    Kontext "TextObjectbar"
    sleep 2
    Printlog "- Change size of font"
    Schriftgroesse.Select (Zufall)
    Printlog Schriftgroesse.GetSelText + " chosen"

    Kontext "TextObjectbar"
    Printlog "- Font attribute bold"
    Fett.Click
    sleep 2

    Kontext "TextObjectbar"
    Printlog "- Font attribute cursive"
    Kursiv.Click
    sleep 2

    Kontext "TextObjectbar"
    Printlog "- Font attribute cursiv"
    Unterstrichen.Click
    sleep 2

    Kontext "TextObjectbar"
    Printlog "- font attribute color"
    sleep 3
    FontColorGraphics.TearOff
    Kontext "TB_Farbe"
    TB_Farbe.Move 20, 20
    Sleep 2
    TB_Farbe.MouseDoubleClick 50, 50
    Sleep 2
    TB_Farbe.Close

    Kontext "TextObjectbar"
    Printlog "- Allign text left"
    Linksbuendig.Click
    sleep 2

    Kontext "TextObjectbar"
    Printlog "- Allign text centered"
    Zentriert.Click
    sleep 2

    Kontext "TextObjectbar"
    Printlog "- align text to right"
    Rechtsbuendig.Click
    sleep 2

    Kontext "TextObjectbar"
    Printlog "- Justified"
    Blocksatz.Click
    sleep 2

    Kontext "TextObjectbar"
    if TextObjectbar.isEnabled <> TRUE then
        warnlog "Couldn't access TextObjectbar - known bug with bars - FHA"
        ViewToolbarsTextFormatting
    endif
    WaitSlot (2000)    'sleep 2
    Printlog "- Raising Font spacing"

    kontext "TextObjectbar"
    sleep 1
    iWaitIndex = 0
    do while NOT TextObjectbar.isEnabled AND iWaitIndex < 10
        sleep(1)
        iWaitIndex = iWaitIndex + 1
    loop
    if NOT TextObjectbar.isEnabled AND iWaitIndex = 10 then
        warnlog "Dialogue TextObjectbar didnt work. Ending testcase."
        Call hCloseDocument
        goto endsub
    endif
    TextObjectbar.OpenContextMenu
    sleep 2
    hMenuselectNr (1)
    sleep 2
    hMenuItemCheck (13)
    sleep 2
    TextObjectbar.OpenContextMenu
    sleep 2
    hMenuselectNr (1)
    sleep 2
    hMenuItemCheck (14)
    sleep 2
    ZeilenabstandErhoehen.Click
    sleep 2

    Kontext "TextObjectbar"
    Printlog "- Decrease Spacing"
    ZeilenabstandVerringern.Click
    sleep 2

    Kontext "TextObjectbar"
    Printlog "- Numbering and Bullets"
    sleep 2
    BulletsOnOff.Click
    sleep 2
    BulletsOnOff.Click

    Kontext "TextObjectBar"
    Printlog "- Increase Font /Reduce Font"
    sleep 2
    printlog "Click on Increase Font"
    IncreaseFont.Click
    sleep 2
    printlog "Click on Reduce Font"
    ReduceFont.Click

    Kontext "TextObjectbar"
    Printlog "- Text direction from left to right"
    sleep 2
    Printlog "- click button 'Text direction from left to right
    try
        TextdirectionLeftToRight.Click
        printlog "hor does work :-)"
        if (NOT gAsianSup) then
            warnlog " this doesn't have to work if asian lang support is disabled :-)"
        endif
    catch
        if (gAsianSup) then
            warnlog "hor does NOT work :-("
        endif
    endcatch
    Printlog "- click button 'Text direction from top to bottom"
    try
        TextdirectionTopToBottom.Click
        printlog "ver does work :-)"
        if (gAsianSup = FALSE) then
            warnlog " this doesnt have to work if asian lang support is disabled :-)"
        endif
    catch
        if (gAsianSup = TRUE) then
            warnlog "ver does NOT work :-( , AsianLanguage support is enabled !"
        endif
    endcatch

    Printlog "- open character dialog"
    Kontext "TextObjectbar"
    sleep 2
    Zeichenformat.Click
    sleep 2

    Kontext
    Active.SetPage TabFont

    Kontext "TabFont"
    sleep 2
    if TabFont.Exists Then
        TabFont.Cancel
        Printlog "- Tabfont exists"
    else
        Warnlog "- No dialog exists "
    endif

    Kontext "TextObjectbar"
    sleep 2
    Printlog "- call properties for paragraph using text object toolbar"
    Absatzformat.Click
    sleep 2

    Kontext
    Active.SetPage TabTabulator

    Kontext "TabTabulator"
    if TabTabulator.Exists Then
        Printlog "- TabTabulator exists"
        TabTabulator.Cancel
    else
        Warnlog "- TabTabulator does not exist"
    endif

    if ( gApplication = "IMPRESS" ) then  ' IMPRESS only

        Kontext "TextObjectbar"
        OutlineBullet.Click
        kontext
        if active.exists (5) then
            messagebox.SetPage TabOptionenNumerierung

            kontext "TabOptionenNumerierung"
            if (TabOptionenNumerierung.exists (5) ) then
                Printlog "Numbering/Bullets window came up :-)"
                TabOptionenNumerierung.cancel
            else
                warnlog "no Numbering/Bullets window came up :-("
            endif
        else
            warnlog "no Numbering/Bullets window came up :-( 2"
        endif

        Printlog "-change order of outline points"
        sleep 3
        ViewWorkspaceOutlineView
        WaitSlot (2000)    'sleep (3)

        Kontext "DocumentImpressOutlineView"
        sleep 1
        DocumentImpressOutlineView.TypeKeys "Bla bla bla <RETURN><TAB>bla bla bla bla bla <MOD1 SHIFT LEFT>"
        sleep 1

        Kontext "TextObjectbar"
        HierachieRunter.Click
        sleep 1
        Printlog "- Move back down"
        HierachieHoch.Click
        sleep 1
        HierachieHoch.Click
        sleep 1
        Printlog "- Move paragraph up"
        AbsatzHoch.Click
        sleep 1
        Printlog "- move paragraph back down and switch to drawing view"
        AbsatzRunter.Click
        sleep 1

        Kontext "Vorschau"
        if Vorschau.Exists then
            printlog "- - - - preview window is open, hope there is no problem"
            '    Vorschau.Close
        endif

    else '      DRAW only
        Kontext "TextObjectbar"
        Printlog "- Double"
        LineSpacing2.Click
        sleep 2

        Kontext "TextObjectbar"
        Printlog "- 1.5 lines"
        LineSpacing15.Click
        sleep 2

        Kontext "TextObjectbar"
        Printlog "- Single"
        LineSpacing1.Click
        sleep 2
    endif

    iWaitIndex = 0

    Kontext "TextObjectbar"
    do while TextObjectbar.isEnabled = FALSE AND iWaitIndex < 10
        sleep(1)
        iWaitIndex = iWaitIndex + 1
    loop
    if TextObjectbar.isEnabled = FALSE AND iWaitIndex = 10 then
        warnlog "Dialogue TextObjectbar didnt work. Ending testcase."
        Call hCloseDocument
        goto endsub
    endif
    sleep 2

    Kontext "TextObjectbar"
    TextObjectbar.OpenContextMenu
    sleep 2
    hMenuselectNr (1)
    sleep 2
    hMenuItemUnCheck (13)
    sleep 2
    TextObjectbar.OpenContextMenu
    sleep 2
    hMenuselectNr (1)
    sleep 2
    hMenuItemUnCheck (14)
    sleep 2
    ViewToolbarsTextFormatting
    WaitSlot (2000)    'sleep 1
    if ( gApplication = "IMPRESS" ) then
        Kontext "DocumentImpressOutlineView"
        DocumentImpressOutlineView.TypeKeys "<ESCAPE>"
        hUseAsyncSlot( "EditSelectAll" )
        DocumentImpressOutlineView.TypeKeys "<DELETE>"
        sleep (3)
        ViewWorkspaceDrawingView

        Kontext "DocumentImpress"
        WaitSlot (2000)    'sleep 1
        DocumentImpress.TypeKeys "<ESCAPE>"
        hUseAsyncSlot( "EditSelectAll" )
        DocumentImpress.TypeKeys "<DELETE>"
    else
        Kontext "DocumentDraw"
        DocumentDraw.TypeKeys "<ESCAPE>"
        hUseAsyncSlot( "EditSelectAll" )
        DocumentDraw.TypeKeys "<DELETE>"
    endif
    Printlog "Test ended."

    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------'

testcase tiDrawObjectBar
    Dim a as string
    Dim Zaehler as integer
    Dim i as integer
    Dim x as integer

    Call hNewDocument

    Call hRechteckErstellen (20,20,70,70)
    sleep 1
    '--------------------------- format line ------------------------
    Printlog "- call format line using graphic object toolbar"
    Kontext "DrawingObjectbar"
    if DrawingObjectbar.Exists = FALSE then
        ViewToolbarsGraphic
    endif
    if DrawingObjectbar.Exists = FALSE then
        warnlog "Drawing-Objectbar should have been opened, but wasnt"
        ViewToolbarsGraphic
    endif

    sleep 3
    Linie.Click
    sleep 2
    Kontext
    Active.SetPage TabLinie
    Kontext "TabLinie"
    if TabLinie.Exists Then
        TabLinie.Cancel
        Printlog "- TabLinie exists "
    else
        Warnlog "- TabLinie does not exist"
    endif

    '--------------------------- Linienendenstil --------------------
    Printlog "- Style of line ends"
    Kontext "DrawingObjectbar"
    sleep 1
    Linienendenstil.TearOff
    sleep 2
    Kontext "Linienenden"
    if Linienenden.Exists Then
        Printlog "- dialog exists"
        Linienenden.Close
    else
        Warnlog "- Arrowheads does not exist"
    endif

    '--------------------------- Linienstil -------------------------
    Printlog "- choose style of lines using graphic object toolbar"

    Kontext "DrawingObjectbar"
    sleep 1
    Linienstil.Select 2
    Printlog Linienstil.GetSelText + " chosen"
    Linienstil.Select Linienstil.GetItemCount
    Printlog  Linienstil.GetSelText + " chosen"
    sleep 2

    '--------------------------- Linienbreite------------------------
    Printlog "- check style of lines using graphic object toolbar"
    Kontext "DrawingObjectbar"
    sleep 1
    a = Linienbreite.GetText
    SetClipboard a
    sleep 1
    Linienbreite.SetText "0,5"
    sleep 2
    if Linienbreite.GetText = a Then
        Warnlog "- changes in edit field did not happen; is: '"+Linienbreite.GetText+"', should be : '"+"0,5"+"'"
    else
        Printlog "- Width of lines changed"
    endif

    '--------------------------- Linienfarbe ------------------------
    Printlog "- change line color"
    Kontext "DrawingObjectbar"
    sleep 1
    Zaehler = Linienfarbe.GetItemCount
    randomize
    i = Int(Zaehler*Rnd+1)
    Linienfarbe.Select i
    Printlog Linienfarbe.GetSelText + " chosen"

    '--------------------------- Flaechenformatierung ---------------
    Printlog "- Call Format area using graphic object toolbar"
    Kontext "DrawingObjectbar"
    sleep 1
    try
        Flaeche.Click
    catch
        printlog "clicking on area took ages :-("
    endcatch
    sleep 5
    Kontext
    Active.SetPage TabArea
    Kontext "TabArea"
    if TabArea.Exists Then
        Printlog "- TabArea exists "
        TabArea.Cancel
    else
        Warnlog "- TabArea does not exist"
    endif

    '---
    sleep 1
    Kontext "DrawingObjectbar"
    sleep 1
    x = AreaStyle.GetItemCount
    i = 1
    for i = i to x
        AreaStyle.Select i
        Printlog "Area Style: (" + i + "/" + x + ") - " + AreaStyle.GetSelText
        sleep 1
        if AreaStyle.GetSelIndex > 1 then
            Printlog "     Area Filling " + AreaFilling.GetSelIndex + " - " + AreaFilling.GetItemCount
            if (i <> 1) AND (AreaFilling.GetItemCount <> 0) then
                if (AreaFilling.GetSelIndex = 0) AND (AreaFilling.GetItemCount > 0) then
                    printlog "default item is 0 => means nothing; NO BUG! 100909"
                endif
                AreaFilling.Select (AreaFilling.GetItemCount)
                Printlog "           Selected: " + AreaFilling.GetSelIndex + " - " + AreaFilling.GetSelText
            endif
        endif
    next i

    '---------------------------- Schatten --------------------------
    Printlog "- Assign shadow using graphic object toolbar"
    Kontext "DrawingObjectbar"
    sleep 1
    Schatten.Click
    sleep 1
    Printlog "- shadow assigned"

    '---------------------------- Praesentationsflyer ---------------
    if ( gApplication = "IMPRESS" ) then  ' IMPRESS only
        Printlog "- Call presentation flyer"
        Kontext "CommonTaskbar" ' first check , if presentation flyer is up! if not -> make it up :-)
        if CommonTaskbar.Exists Then
            printlog "- flyer is already visible :-)"
        else
            printlog "- flyer wasn't visible :-( -will be now!"
            Kontext "DrawingObjectbar"
            sleep 1
            ViewToolbarsPresentation    ' put it up again!
        endif
    endif
    if ((UCase(gApplication)) = "IMPRESS") then    ' IMPRESS only
        Kontext "DrawingObjectbar"
        if DrawingObjectbar.isVisible = FALSE then
            ViewToolbarsGraphic
        endif
    endif
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------'

testcase tiGraphicsObjectBar
    dim i as integer
    Call hNewDocument

    hGrafikeinfuegen ConvertPath (gTesttoolPath & "global\input\graf_inp\desp.bmp")
    sleep 1
    Kontext "GraphicObjectbar"
    if ( NOT GraphicObjectbar.Exists() ) then
        ViewToolbarsPicture
    endif
    sleep 1
    Filter.TearOff
    sleep 1
    Kontext "GraphicFilterBar"
    sleep 1
    Printlog "invert"
    Invert.Click
    sleep 3
    Printlog "smooth"
    Smooth.Click
    sleep 3
    Printlog "sharpen"
    Sharpen.Click
    sleep 3
    Printlog "remove noise"
    Remove.Click
    sleep 3
    Printlog "solarization"
    Solarization.Click
    sleep 3
    Kontext "Solarization"
    if Solarization.exists (5) then
        sleep 1
        Call Dialogtest ( Solarization )
        sleep 1
        Value.More
        Value.Less
        Invert.Check
        Solarization.OK
    else
        warnlog "solarization didn't came up :-("
    endif
    sleep 1
    Kontext "GraphicFilterBar"
    sleep 1
    Printlog "aging"
    Aging.Click
    Kontext "Aging"
    sleep 1
    Call Dialogtest ( Aging )
    AgingDegree.More
    AgingDegree.Less
    sleep 1
    Aging.OK
    sleep 1
    Kontext "GraphicFilterBar"
    sleep 1
    Printlog "poster"
    Posterize.Click
    sleep 1
    Kontext "Posterize"
    sleep 1
    Call Dialogtest ( Posterize )
    PosterColors.More
    PosterColors.Less
    sleep 1
    Posterize.OK
    sleep 2
    kontext "GraphicFilterBar"
    sleep 1
    Printlog "pop"
    Art.Click
    sleep 3
    Printlog "charcoal"
    CharcoalSketch.Click
    sleep 3
    Printlog "relief"
    Relief.Click
    Kontext "Relief"
    sleep 1
    Call Dialogtest ( Relief )
    LightSource.TypeKeys "<left><up>"
    Relief.OK
    sleep 3
    Kontext "GraphicFilterBar"
    Printlog "mos"
    Mosaic.Click
    sleep 1
    Kontext "Mosaic"
    sleep 1
    Call Dialogtest ( Mosaic )
    Width.More
    Width.Less
    Height.More
    Height.Less
    EnhanceEdges.Check
    Mosaic.OK
    sleep 3
    Kontext "GraphicFilterBar"
    GraphicFilterBar.Close

    Kontext "GraphicObjectbar"
    if GraphicObjectbar.Exists = FALSE then
        ViewToolbarsPicture
    endif
    sleep 1
    for i = 1 to Grafikmodus.GetItemCount
        Grafikmodus.select i
        sleep 1
    next i

    ColorSettings.Click
    Kontext "ColorBar"

    try
        Rotanteil.More
        Rotanteil.Less
    catch
        warnlog "not working from testtool redvalue "+   rotanteil.GetRT
    endcatch
    try
        Gruenanteil.More
        Gruenanteil.Less
    catch
        warnlog "not working from testtool Greenvalue."
    endcatch
    try
        Blauanteil.More
        Blauanteil.Less
    catch
        warnlog "not working from testtool Bluevalue."
    endcatch
    try
        Helligkeit.More
        Helligkeit.Less
    catch
        warnlog "not working from testtool Brightness."
    endcatch
    try
        Kontrast.More
        Kontrast.Less
    catch
        warnlog "not working from testtool Contrast."
    endcatch
    try
        Gamma.More
        Gamma.Less
    catch
        warnlog "not working from testtool Gamma."
    endcatch

    ColorBar.Close
    Kontext "GraphicObjectbar"

    try
        Transparenz.More
        Transparenz.Less
    catch
        warnlog "Not working from testtool Transparency."
    endcatch

    Crop.click
    FormatCropPicture

    kontext "TabZuschneiden"
    GroesseBeibehalten.Check
    MassstabBeibehalten.Check
    Links.More
    Links.Less
    Rechts.More
    Rechts.Less
    Oben.More
    Oben.Less
    Unten.More
    Unten.Less
    MassstabBreite.More
    MassstabBreite.Less
    MassstabHoehe.More
    MassstabHoehe.Less
    GroesseBreite.More
    GroesseBreite.Less
    GroesseHoehe.More
    GroesseHoehe.Less
    Originalgroesse.Click
    TabZuschneiden.Cancel

    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------'

testcase tiGluepointToolbar
    Call hNewDocument

    Call hRechteckErstellen 20,20,40,40
    Call hRechteckErstellen 60,60,80,80
    sleep 2
    ViewToolbarsOptionbar
    sleep 2

    Kontext "Optionsbar"
    if Optionsbar.Exists = False Then
        ViewToolbarsOptionbar
        Kontext "Optionsbar"
        if Optionsbar.Exists = False Then
            warnlog "Can't open Optionsbar."
        endif
    endif
    sleep 2
    Kontext "Toolbar"
    sleep 1
    Verbinder.Click
    gMouseMove 30,30,70,70
    sleep 3
    Kontext "Toolbar"
    GluePoints.Click
    sleep 1
    Kontext "Gluepointsobjectbar"
    if Gluepointsobjectbar.Exists = False Then
        ViewToolbarsGluepoints
    endif
    sleep 3
    '-------------------------------------- Klebepunkt einfuegen ----
    try
        PunkteEinfuegen.Click
        sleep 2
        gMouseClick 25,30
        sleep 1
        gMouseclick 25,30
        Printlog "- insert gluepoint works"
    catch
        Warnlog "- gluepoint could not be insert. Following errors might have their reason here"
    endcatch
    sleep 3
    Kontext "Gluepointsobjectbar"

    '-------------------------------------- Links -------------------
    try
        Links.Click
        Printlog "- gluepoint left works"
    catch
        Warnlog "- gluepoint left does not work"
    endcatch
    sleep 1
    '-------------------------------------- Rechts ------------------
    try
        Rechts.Click
        Printlog "- gluepoint right works"
    catch
        Warnlog "- gluepoint right does not work"
    endcatch
    sleep 1
    '-------------------------------------- Oben --------------------
    try
        Oben.Click
        Printlog "- gluepoint top works"
    catch
        Warnlog "- gluepoint top does not work"
    endcatch
    sleep 1
    '-------------------------------------- Unten -------------------
    try
        Unten.Click
        Printlog "- gluepoint bottom works"
    catch
        Warnlog "- gluepoint bottom does not work"
    endcatch
    sleep 1
    '---------------------- Position an Objekt anpassen -------------
    try
        PositionAnObjektAnpassen.Click
        Printlog "- align position to object works"
        gMouseClick 25,30      ' if you don't click onto an existing point, the state changes back :-[
    catch
        Warnlog "- align position to object does not work"
    endcatch
    Kontext "Gluepointsobjectbar"
    sleep 1
    '------------------------------------- Horizontal links ---------
    if PositionAnObjektAnpassen.exists then
        printlog "PositionAnObjektAnpassen = Exists"
    endif
    if PositionAnObjektAnpassen.GetState(2) <> 0 then
        PositionAnObjektAnpassen.Click ' make unpressed!
    endif
    '0 = not pressed. 1 = pressed.
    sleep 2
    try
        Kontext "Gluepointsobjectbar"
        HorizontalLinks.Click
        Printlog "- align horizontal left works"
    catch
        Warnlog "- align horizontal left does not work"
        PositionAnObjektAnpassen.Click ' that's the middle button, it has to be UP/not activated!
        for i = 1 to Gluepointsobjectbar.GetItemCount
            if (Gluepointsobjectbar.GetState ( i, 0 ) <> 0 ) then ' is no seperator
                printlog "----------------------------------------------------------------------"
                printlog "helpid  : " + Gluepointsobjectbar.Getstate ( i, 0 ) + "  number in row: " + i
                printlog "itemtype: " + Gluepointsobjectbar.GetState ( i, 1 )
                printlog "state   : " + Gluepointsobjectbar.GetState ( i, 2 )
                printlog "----------------------------------------------------------------------"
            endif
        next i
    endcatch
    sleep 2
    '------------------------------------- Horizontal rechts --------
    try
        HorizontalRechts.Click
        Printlog "- Align horizontal right does work"
    catch
        Warnlog "- Align horizontal right does notwork"
    endcatch
    sleep 1
    '------------------------------------- Horizontal zentriert -----
    try
        HorizontalZentriert.Click
        Printlog "- align horizontal center does work"
    catch
        Warnlog "- align horizontal center does work"
    endcatch
    sleep 1
    '------------------------------------- Vertikal oben ------------
    try
        VertikalOben.Click
        sleep 1
        Printlog "- Align vertical top does work"
    catch
        Warnlog "- Align vertical top does not work"
    endcatch
    '------------------------------------- Vertikal unten -----------
    try
        VertikalUnten.Click
        sleep 1
        printlog "- Align vertical bottom does work"
    catch
        Warnlog "- Align vertical bottom does not work"
    endcatch
    '------------------------------------- Vertikal zentriert -------
    try
        VertikalZentriert.Click
        sleep 1
        Printlog "- Align vertical center does work"
    catch
        Warnlog "- Align vertical center does not work"
    endcatch

    Printlog "- End of testing gluepoints"

    ViewToolbarsOptionbar
    sleep 2
    Kontext "Optionsbar"
    if Optionsbar.Exists Then
        warnlog "Couldnt close Optionsbar."
    endif
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------'

testcase tdBezierToolbar
    Call hNewDocument

    Call hRechteckErstellen ( 10, 10, 30, 40 )
    sleep (1)

    Call hOpenContextMenu
    sleep (1)

    Call hOpenContextMenu
    sleep (2)

    Kontext "Bezierobjectbar"
    if Bezierobjectbar.Exists <> TRUE then
        ViewToolbarsBezier
        Sleep (2)
        if Bezierobjectbar.Exists <> TRUE then
            warnlog "Bezierobjectbar did not show up. Check why."
        endif
    endif

    '----------------------------------- Punkte verschieben ------------------------------------

    Kontext "Bezierobjectbar"
    if Bezierobjectbar.Exists <> TRUE then
        ViewToolbarsBezier
        Sleep 2
    endif
    if Bezierobjectbar.Exists <> TRUE then
        Warnlog "The Bezier-Objectbar should have been opened, but wasnt."
        ViewToolbarsBezier
        Sleep 2
    endif

    Kontext "Bezierobjectbar"
    sleep 1
    Printlog "- Move points"
    Verschieben.Click
    sleep 2

    hTypeKeys "<MOD1 TAB>"

    Printlog "- Insert points"
    Kontext "Bezierobjectbar"
    Einfuegen.Click
    sleep 2
    Bezierobjectbar.Move 20, 20
    sleep (1)
    Printlog "- Delete points"

    gMouseMove 25,25,45,45


    sleep 2
    Kontext "Bezierobjectbar"
    sleep 2
    Printlog "- Convert into curve"
    InKurve.Click
    sleep 2
    Printlog "- Place edge point"
    Ecke.Click
    sleep 2
    Printlog "- Smooth transition"
    Glatt.Click
    sleep 2
    Printlog "- Symetric transition"
    Symmetrisch.Click
    sleep 2

    Kontext "Bezierobjectbar"
    PunkteReduzieren.Click
    sleep 2

    hTypeKeys "<MOD1 TAB>"
    hTypeKeys "<MOD1 SHIFT SPACE>"

    Kontext "Bezierobjectbar"
    try
        Auftrennen.Click
    catch
        Warnlog "- 'Break' could not be executed"
    endcatch
    sleep 2

    hTypeKeys "<MOD1 TAB>"
    hTypeKeys "<MOD1 A>"

    Printlog "- Close bezier"
    Kontext "Bezierobjectbar"
    try
        Schliessen.Click
    catch
        InKurve.Click
        sleep 2
        try
            Schliessen.Click
        catch
            warnlog "Couldn't push button :-( can't reproduce it now, mostly seen on linux, if i loop this test, it happens only 1/5 of the time ... :-)"
        endcatch
    endcatch
    sleep 2

    hTypeKeys "<MOD1 TAB>"
    hTypeKeys "<MOD1 A>"

    Kontext "Bezierobjectbar"
    Printlog "- Break curve"
    try
        Loeschen.Click
        sleep 2
    catch
        warnlog "Delete didn't work... why?"
    endcatch

    Kontext "Toolbar"
    sleep 2
    Toolbar.OpenContextMenu   ' Enable forms button in menuebar
    sleep 2
    hMenuselectNr (1)
    sleep 2
    hMenuItemUnCheck (7)
    sleep 2
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------'
