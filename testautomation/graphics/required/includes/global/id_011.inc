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
'\******************************************************************************
sub id_011

    printLog Chr(13) + "---------    id_011    ----------"

    call tdBezierToolbar
    call tiDrawObjectBar
    call tiTextToolbar
    call tiGraphicsObjectBar
    call tiGluepointToolbar

end sub
'-------------------------------------------------------------------------------

testcase tiTextToolbar
    Dim iWaitIndex as integer
    Dim Zufall as integer
    Dim i as integer

    printlog "open application"
    Call hNewDocument
    Call sSelectEmptyLayout
    printlog "create a textframe with text"
    hTextrahmenErstellen ("This is a Text, which will be formatted in several ways", 20,20,40,80)
    printlog "The 'Object Toolbar' now changes"
    sleep 2
    printlog "select of the text with <STRG> + <A>"
    EditSelectAll
    sleep 2
    printlog "Call the textobjectbar from the View-Toolbars-menu"
    Kontext "TextObjectbar"
    if TextObjectbar.Exists <> TRUE then  'isVisible
        ViewToolbarsTextFormatting
    end if
    WaitSlot (3000)    'sleep (3)
    Kontext "TextObjectbar"
    Printlog "- choose random font"
    randomize
    Zufall=((2*Rnd)+1)      ' wird nicht auf den verfuegbaren bereich getreckt :-( TODO TBO!
    Schriftart.GetItemcount
    Schriftart.GetSelText
    Schriftart.Select (Zufall)
    printlog "select any font you like in the listbox 'Font' on 'Object Toolbar'"
    Printlog Schriftart.GetSelText + " chosen"
    Kontext "TextObjectbar"
    sleep 2
    Printlog "- Change size of font"
    printlog "select any font you like in the listbox 'Size' on 'Object Toolbar'"
    Schriftgroesse.Select (Zufall)
    Printlog Schriftgroesse.GetSelText + " chosen"
    Kontext "TextObjectbar"
    Printlog "- Font attribute bold"
    printlog "click button 'Bold'"
    Fett.Click
    sleep 2
    Kontext "TextObjectbar"
    Printlog "- Font attribute cursive"
    printlog "click button 'Italic'"
    Kursiv.Click
    sleep 2
    Kontext "TextObjectbar"
    Printlog "- Font attribute cursiv"
    printlog "click button 'Underline'"
    Unterstrichen.Click
    sleep 2
    Kontext "TextObjectbar"
    Printlog "- font attribute color"
    printlog "click button 'Font Color' and tearoff the box"
    sleep 3
    FontColorGraphics.TearOff
    Kontext "TB_Farbe"
    TB_Farbe.Move 20, 20
    Sleep 2
    printlog "double click in the middle of the box"
    TB_Farbe.MouseDoubleClick 50, 50
    Sleep 2
    printlog "close box 'Font Color'"
    TB_Farbe.Close

    Kontext "TextObjectbar"
    Printlog "- Allign text left"
    printlog "click button 'Left'"
    Linksbuendig.Click
    sleep 2

    Kontext "TextObjectbar"
    Printlog "- Allign text centered"
    printlog "click button 'Centered'"
    Zentriert.Click
    sleep 2

    Kontext "TextObjectbar"
    Printlog "- align text to right"
    printlog "click button 'Right'"
    Rechtsbuendig.Click
    sleep 2

    Kontext "TextObjectbar"
    Printlog "- Justified"
    printlog "click button 'Justified'"
    Blocksatz.Click
    sleep 2

    Kontext "TextObjectbar"
    if TextObjectbar.isEnabled <> TRUE then
        warnlog "Couldn't access TextObjectbar - known bug with bars - FHA"
        ViewToolbarsTextFormatting
    end if
    WaitSlot (2000)    'sleep 2
    Printlog "- Raising Font spacing"
    printlog "click button 'Increase Spacing'"
    kontext "TextObjectbar"
    sleep 1
    printlog "Check if the bar is there. If not - wait for maximum 10 seconds)"
    iWaitIndex = 0
    do while NOT TextObjectbar.isEnabled AND iWaitIndex < 10
        sleep(1)
        iWaitIndex = iWaitIndex + 1
    loop
    if NOT TextObjectbar.isEnabled AND iWaitIndex = 10 then
        warnlog "Dialogue TextObjectbar didnt work. Ending testcase."
        Call hCloseDocument
        goto endsub
    end if
    printlog "Activate the Font spacing-buttons"
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
    printlog "click button 'Decrease Spacing'"
    ZeilenabstandVerringern.Click
    sleep 2

    Kontext "TextObjectbar"
    Printlog "- Numbering and Bullets"
    sleep 2
    printlog "click button 'Bullets On/Off'"
    BulletsOnOff.Click
    sleep 2
    printlog "click button 'Bullets On/Off'"
    BulletsOnOff.Click
    '-----------------------------------------------------------------------
    Kontext "TextObjectBar"
    Printlog "- Increase Font /Reduce Font"
    sleep 2
    printlog "- Click on Increase Font"
    IncreaseFont.Click
    sleep 2
    printlog "- Click on Reduce Font"
    ReduceFont.Click
    '-----------------------------------------------------------------------
    Kontext "TextObjectbar"
    Printlog "- Text direction from left to right"
    sleep 2
    Printlog "- click button 'Text direction from left to right'"
    try
        TextdirectionLeftToRight.Click
        printlog "hor does work :-)"
        if (gAsianSup = FALSE) then
            warnlog " this doesn't have to work if asian lang support is disabled :-)"
        end if
    catch
        if (gAsianSup = TRUE) then
            warnlog "hor does NOT work :-("
        end if
    endcatch
    Printlog "- click button 'Text direction from top to bottom"
    try
        TextdirectionTopToBottom.Click
        printlog "ver does work :-)"
        if (gAsianSup = FALSE) then
            warnlog " this doesnt have to work if asian lang support is disabled :-)"
        end if
    catch
        if (gAsianSup = TRUE) then
            warnlog "ver does NOT work :-( , AsianLanguage support is enabled !"
        end if
    endcatch

    Printlog "- open character dialog"
    Kontext "TextObjectbar"
    sleep 2
    printlog "click button 'Character'"
    Zeichenformat.Click
    sleep 2
    Kontext
    Active.SetPage TabFont
    Kontext "TabFont"
    sleep 2
    if TabFont.Exists Then
        printlog "close dialog 'character'"
        TabFont.Cancel
        Printlog "- Tabfont exists"
    else
        Warnlog "- No dialog exists "
    end if

    Kontext "TextObjectbar"
    sleep 2
    Printlog "- call properties for paragraph using text object toolbar"
    printlog "click button 'Paragraph'"
    Absatzformat.Click
    sleep 2
    Kontext
    Active.SetPage TabTabulator
    Kontext "TabTabulator"
    if TabTabulator.Exists Then
        Printlog "- TabTabulator exists"
        printlog "close dialog 'Paragraph'"
        TabTabulator.Cancel
    else
        Warnlog "- TabTabulator does not exist"
    end if

    if (UCase(gApplication)) = "IMPRESS" then  ' IMPRESS only
        printlog "if the Application is Impress:"

        Kontext "TextObjectbar"
        printlog "click button 'Numbering Symbols'"
        OutlineBullet.Click
        kontext
        if active.exists (5) then
            messagebox.SetPage TabOptionenNumerierung
            kontext "TabOptionenNumerierung"
                printlog "cancel dialog 'Numbering/Bullets'"
                if (TabOptionenNumerierung.exists (5) ) then
                    Printlog "Numbering/Bullets window came up :-)"
                    sleep 2
                    TabOptionenNumerierung.cancel
                else
                    warnlog "no Numbering/Bullets window came up :-("
                end if
                sleep 2
        else
            warnlog "no Numbering/Bullets window came up :-( 2"
        end if

        Printlog "-change order of outline points"
        sleep 3
        printlog "View->Master View->Outline View"
        ViewWorkspaceOutlineView
        WaitSlot (2000)    'sleep (3)
        Kontext "DocumentImpressOutlineView"
        sleep 1
        printlog "type something"
        DocumentImpressOutlineView.TypeKeys "Bla bla bla <RETURN><TAB>bla bla bla bla bla <MOD1 SHIFT LEFT>"
        sleep 1
        Kontext "TextObjectbar"
        printlog "click button 'Promote'"
        HierachieRunter.Click
        sleep 1
        Printlog "- Move back down"
        printlog "click button 'Denote'"
        HierachieHoch.Click
        sleep 1
        printlog "click button 'Denote'"
        HierachieHoch.Click
        sleep 1
        Printlog "- Move paragraph up"
        printlog "click button 'Move Up', twice to get two slides"
        AbsatzHoch.Click
        sleep 1
        Printlog "- move paragraph back down and switch to drawing view"
        printlog "click button 'Move Down'"
        AbsatzRunter.Click
        sleep 1

        Kontext "Vorschau"
        if Vorschau.Exists then
            printlog "- - - - preview window is open, hope there is no problem"
            '    Vorschau.Close
        end if
    
    else '      DRAW only
        Kontext "TextObjectbar"
        Printlog "- Double"
        printlog "click button 'Double'"
        LineSpacing2.Click
        sleep 2

        Kontext "TextObjectbar"
        Printlog "- 1.5 lines"
        printlog "click button '1.5 lines'"
        LineSpacing15.Click
        sleep 2

        Kontext "TextObjectbar"
        Printlog "- Single"
        printlog "click button 'Single'"
        LineSpacing1.Click
        sleep 2
    end if

    printlog "Check if the bar is there. If not - wait for maximum 10 seconds)"
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
    end if
    printlog "Reset the original value of the Font spacing-buttons"
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
    printlog "Close the TextFormatting Toolbar"
    ViewToolbarsTextFormatting
    WaitSlot (2000)    'sleep 1
    printlog "Clear the document from content"
    if (gApplication = "IMPRESS") then
        Kontext "DocumentImpressOutlineView"
        DocumentImpressOutlineView.TypeKeys "<ESCAPE>"
        EditSelectAll
        DocumentImpressOutlineView.TypeKeys "<DELETE>"
        sleep (3)
        ViewWorkspaceDrawingView
        Kontext "DocumentImpress"
        WaitSlot (2000)    'sleep 1
        DocumentImpress.TypeKeys "<ESCAPE>"
        EditSelectAll
        DocumentImpress.TypeKeys "<DELETE>"
    else
        Kontext "DocumentDraw"
        DocumentDraw.TypeKeys "<ESCAPE>"
        EditSelectAll
        DocumentDraw.TypeKeys "<DELETE>"
    end if
    Printlog "Test ended."
    printlog "close application"
    Call hCloseDocument
endcase 'tiTextToolbar

'-------------------------------------------------------------------------------'

testcase tiDrawObjectBar
    Dim a as string
    Dim Zaehler as integer
    Dim i as integer
    Dim x as integer

    printlog "open application"
    Call hNewDocument
    printlog "create rectangle to test the part for draw objects"
    Call hRechteckErstellen (20,20,70,70)
    sleep 1
    Printlog "- call format line using graphic object toolbar"
    Kontext "DrawingObjectbar"
    printlog "Check if the Graphic-Toolbar is there. Which it should be."
    printlog "If it's not - then we call it from the View-Toolbars"
    if DrawingObjectbar.Exists = FALSE then
        ViewToolbarsGraphic
    end if
    if DrawingObjectbar.Exists = FALSE then
        warnlog "Drawing-Objectbar should have been opened, but wasnt"
        ViewToolbarsGraphic
    end if
    sleep 3
    printlog "click button 'line' on 'Object Toolbar'"
    Linie.Click
    sleep 2
    Kontext
    Active.SetPage TabLinie
    Kontext "TabLinie"
    if TabLinie.Exists Then
        printlog "cancel dialog 'line'"
        TabLinie.Cancel
        Printlog "- TabLinie exists "
    else
        Warnlog "- TabLinie does not exist"
    end if

    '--------------------------- Linienendenstil --------------------
    Printlog "- Style of line ends"
    Kontext "DrawingObjectbar"
    sleep 1
    printlog "click button 'Arrow Style' and tearoff the box"
    Linienendenstil.TearOff
    sleep 2
    Kontext "Linienenden"
    if Linienenden.Exists Then
        Printlog "- dialog exists"
        printlog "close toolbox 'arrowheads'"
        Linienenden.Close
    else
        Warnlog "- Arrowheads does not exist"
    end if

    '--------------------------- Linienstil -------------------------
    Printlog "- choose style of lines using graphic object toolbar"

    Kontext "DrawingObjectbar"
    sleep 1
    printlog "select 2nd entry from the top in listbox 'Line Style'"
    Linienstil.Select 2
    Printlog Linienstil.GetSelText + " chosen"
    printlog "select last entry in listbox 'Line Style'"
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
    printlog "set 'Line Width' to 0,5"
    Linienbreite.SetText "0,5"
    sleep 2
    printlog "check if the value is accepted or not"
    if Linienbreite.GetText = a Then
        Warnlog "- changes in edit field did not happen; is: '"+Linienbreite.GetText+"', should be : '"+"0,5"+"'"
    else
        Printlog "- Width of lines changed"
    end if

    '--------------------------- Linienfarbe ------------------------
    Printlog "- change line color"
    Kontext "DrawingObjectbar"
    sleep 1
    Zaehler = Linienfarbe.GetItemCount
    randomize
    i = Int(Zaehler*Rnd+1)
    printlog "select some entry in the listbox 'Line Color'"
    Linienfarbe.Select i
    Printlog Linienfarbe.GetSelText + " chosen"

    '--------------------------- Flaechenformatierung ---------------
    Printlog "- Call Format area using graphic object toolbar"
    Kontext "DrawingObjectbar"
    sleep 1
    printlog "click button 'Area'"
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
        printlog "cancel dialog 'Area'"
        TabArea.Cancel
    else
        Warnlog "- TabArea does not exist"
    end if

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
        printlog "for every item in the listbox 'Area Style' select the last item in the listbox 'Area Filling'"
        if AreaStyle.GetSelIndex > 1 then
            Printlog "     Area Filling " + AreaFilling.GetSelIndex + " - " + AreaFilling.GetItemCount
            if (i <> 1) AND (AreaFilling.GetItemCount <> 0) then
                if (AreaFilling.GetSelIndex = 0) AND (AreaFilling.GetItemCount > 0) then
                    printlog "default item is 0 => means nothing; NO BUG! 100909"
                end if
                AreaFilling.Select (AreaFilling.GetItemCount)
                Printlog "           Selected: " + AreaFilling.GetSelIndex + " - " + AreaFilling.GetSelText
            end if
        end if
    next i

    '---------------------------- Schatten --------------------------
    Printlog "- Assign shadow using graphic object toolbar"
    Kontext "DrawingObjectbar"
    sleep 1
    printlog "click button 'Shadow'"
    Schatten.Click
    sleep 1
    Printlog "- shadow assigned"

        '---------------------------- Praesentationsflyer ---------------
    printlog "if Application is Impress:"
    if (UCase(gApplication)) = "IMPRESS" then  ' IMPRESS only
        Printlog "- Call presentation flyer"
        Kontext "CommonTaskbar" ' first check , if presentation flyer is up! if not -> make it up :-)
        printlog "if 'Presentation Box' is not visible, click button 'Presentation Box on/off'"
        if CommonTaskbar.Exists Then
            printlog "- flyer is already visible :-)"
        else
            printlog "- flyer wasn't visible :-( -will be now!"
            Kontext "DrawingObjectbar"
            sleep 1
            ViewToolbarsPresentation    ' put it up again!
        end if
    end if
    printlog "Make sure the Graphics-Toolbar is visible"
    if ((UCase(gApplication)) = "IMPRESS") then    ' IMPRESS only
        Kontext "DrawingObjectbar"
        if DrawingObjectbar.isVisible = FALSE then
            ViewToolbarsGraphic
        end if
    end if
    printlog "close application"
    Call hCloseDocument
endcase 'tiDrawObjectBar

'------------------------------------------------------------------------------'

testcase tiGraphicsObjectBar
    dim i as integer
    printlog "open application"
    Call hNewDocument

    printlog "insert a graphic global\input\graf_inp\desp.bmp"
    hGrafikeinfuegen ConvertPath (gTesttoolPath + "global\input\graf_inp\desp.bmp")
    sleep 1
    Kontext "GraphicObjectbar"
    if GraphicObjectbar.Exists = FALSE then
    ViewToolbarsPicture
    end if
    sleep 1
    printlog "click button 'Filter' and tearoff the box in the Objectbar"
    Filter.TearOff
    sleep 1
    Kontext "GraphicFilterBar"
    sleep 1
    Printlog "invert"
    printlog "click button 'Invert' in window 'Filters'"
    Invert.Click
    sleep 3
    Printlog "smooth"
    printlog "click button 'Smooth' in window 'Filters'"
    Smooth.Click
    sleep 3
    Printlog "sharpen"
    printlog "click button 'Sharpen' in window 'Filters'"
    Sharpen.Click
    sleep 3
    Printlog "remove noise"
    printlog "click button 'RemoveNoise' in window 'Filters'"
    Remove.Click
    sleep 3
    Printlog "solarization"
    printlog "click button 'Solarization' in window 'Filters'"
    Solarization.Click
    sleep 3
    Kontext "Solarization"
    if Solarization.exists (5) then
        sleep 1
        Call Dialogtest ( Solarization )
        sleep 1
        printlog " press 'more' and 'less' in field 'Trashold Value' on dialog 'Solarization'"
        Value.More
        Value.Less
        printlog "check checkbox 'Invert' on dialog 'Solarization'"
        Invert.Check
        printlog "close dialog 'Solarization' with OK"
        Solarization.OK
    else
        warnlog "solarization didn't came up :-("
    end if
    sleep 1
    Kontext "GraphicFilterBar"
    sleep 1
    Printlog "aging"
    printlog "click button 'Aging' in window 'Filters'"
    Aging.Click
    Kontext "Aging"
    sleep 1
    Call Dialogtest ( Aging )
    printlog " press 'more' and 'less' in field 'Aging degree' on dialog 'Aging'"
    AgingDegree.More
    AgingDegree.Less
    sleep 1
    printlog "close dialog 'Aging' with OK"
    Aging.OK
    sleep 1
    Kontext "GraphicFilterBar"
    sleep 1
    Printlog "poster"
    printlog "click button 'Posterize' in window 'Filters'"
    Posterize.Click
    sleep 1
    Kontext "Posterize"
    sleep 1
    Call Dialogtest ( Posterize )
    printlog " press 'more' and 'less' in field 'Aging degree' on dialog 'Posterize'"
    PosterColors.More
    PosterColors.Less
    sleep 1
    printlog "close dialog 'Posterize' with OK"
    Posterize.OK
    sleep 2
    kontext "GraphicFilterBar"
    sleep 1
    Printlog "pop"
    printlog "click button 'PopArt' in window 'Filters'"
    Art.Click
    sleep 3
    Printlog "charcoal"
    printlog "click button 'CharcoalSketch' in window 'Filters'"
    CharcoalSketch.Click
    sleep 3
    Printlog "relief"
    printlog "click button 'Relief' in window 'Filters'"
    Relief.Click
    Kontext "Relief"
    sleep 1
    Call Dialogtest ( Relief )
    LightSource.TypeKeys "<left><up>"
    printlog "close dialog 'Relief' with OK"
    Relief.OK
    sleep 3
    Kontext "GraphicFilterBar"
    Printlog "mos"
    printlog "click button 'Mosaic' in window 'Filters'"
    Mosaic.Click
    sleep 1
    Kontext "Mosaic"
    sleep 1
    Call Dialogtest ( Mosaic )
    printlog " press 'more' and 'less' in field 'Width' on dialog 'Mosaic'"
    Width.More
    Width.Less
    printlog " press 'more' and 'less' in field 'Height' on dialog 'Mosaic'"
    Height.More
    Height.Less
    printlog " check checkbox 'Enhance Edges' on dialog 'Mosaic'"
    EnhanceEdges.Check
    printlog "close dialog 'Mosaic' with OK"
    Mosaic.OK
    sleep 3
    Kontext "GraphicFilterBar"
    printlog "close dialog 'Filters'"
    GraphicFilterBar.Close

    Kontext "GraphicObjectbar"
    if GraphicObjectbar.Exists = FALSE then ViewToolbarsPicture
        sleep 1
        printlog "select every entry in the listbox 'Graphics mode'"
    for i = 1 to Grafikmodus.GetItemCount
        Grafikmodus.select i
        sleep 1
    next i

    printlog " Click on the icon for 'ColorSettings' to open the colorbar"
    ColorSettings.Click
    Kontext "ColorBar"
    sleep (2)
    printlog " press 'more' and 'less' in field 'Red' on colorbar"
    try
        Rotanteil.More
        Rotanteil.Less
    catch
        warnlog "not working from testtool redvalue "+   rotanteil.GetRT
    endcatch
    printlog " press 'more' and 'less' in field 'Green' on colorbar"
    try
        Gruenanteil.More
        Gruenanteil.Less
    catch
        warnlog "not working from testtool Greenvalue."
    endcatch
    printlog " press 'more' and 'less' in field 'Blue' on colorbar"
    try
        Blauanteil.More
        Blauanteil.Less
    catch
        warnlog "not working from testtool Bluevalue."
    endcatch
    printlog " press 'more' and 'less' in field 'Brightness' on colorbar"
    try
        Helligkeit.More
        Helligkeit.Less
    catch
        warnlog "not working from testtool Brightness."
    endcatch
    printlog " press 'more' and 'less' in field 'Contrast' on colorbar"
    try
        Kontrast.More
        Kontrast.Less
    catch
        warnlog "not working from testtool Contrast."
    endcatch
    printlog " press 'more' and 'less' in field 'Gamma' on colorbar"
    try
        Gamma.More
        Gamma.Less
    catch
        warnlog "not working from testtool Gamma."
    endcatch

    ColorBar.Close
    Kontext "GraphicObjectbar"

    printlog " press 'more' and 'less' in field 'Transparency' on objectbar"
    try
        Transparenz.More
        Transparenz.Less
    catch
        warnlog "Not working from testtool Transparency."
    endcatch

    printlog "press button 'Crop'  on objectbar"
    Crop.click
    printlog "Opening Dialog via Slot"
    FormatCropPicture

    kontext "TabZuschneiden"
    printlog " check radiobutton 'Keep scale' on dialog 'Crop'"
    GroesseBeibehalten.Check
    printlog " check radiobutton 'Keep Image Size' on dialog 'Crop'"
    MassstabBeibehalten.Check
    printlog " press 'more' and 'less' in field 'Left' in section 'Crop' on dialog 'Crop'"
    Links.More
    Links.Less
    printlog " press 'more' and 'less' in field 'Right' in section 'Crop' on dialog 'Crop'"
    Rechts.More
    Rechts.Less
    printlog " press 'more' and 'less' in field 'Top' in section 'Crop' on dialog 'Crop'"
    Oben.More
    Oben.Less
    printlog " press 'more' and 'less' in field 'Bottom' in section 'Crop' on dialog 'Crop'"
    Unten.More
    Unten.Less
    printlog " press 'more' and 'less' in field 'Width' in section 'Scale' on dialog 'Crop'"
    MassstabBreite.More
    MassstabBreite.Less
    printlog " press 'more' and 'less' in field 'Height' in section 'Scale' on dialog 'Crop'"
    MassstabHoehe.More
    MassstabHoehe.Less
    printlog " press 'more' and 'less' in field 'Width' in section 'Image size' on dialog 'Crop'"
    GroesseBreite.More
    GroesseBreite.Less
    printlog " press 'more' and 'less' in field 'Height' in section 'Image size' on dialog 'Crop'"
    GroesseHoehe.More
    GroesseHoehe.Less
    printlog " click button 'Original Size' on dialog 'Crop'"
    Originalgroesse.Click
    printlog "cancel dialog 'Crop'"
    TabZuschneiden.Cancel

    printlog "close application"
    Call hCloseDocument
endcase 'tiGraphicsObjectBar

'-------------------------------------------------------------------------------'

testcase tiGluepointToolbar
    printlog "open application"
    Call hNewDocument

    printlog "create 2 rectangles"
    Call hRechteckErstellen 20,20,40,40
    Call hRechteckErstellen 60,60,80,80
    sleep 2
    printlog "View->Toolbars->Option Bar"
    ViewToolbarsOptionbar
    printlog "at the bottom of the office has a toolbar to appear"
    sleep 2

    Kontext "Optionsbar"
    if Optionsbar.Exists = False Then
        ViewToolbarsOptionbar
        Kontext "Optionsbar"
        if Optionsbar.Exists = False Then
            warnlog "Can't open Optionsbar."
        end if
    end if
    sleep 2
    Kontext "Toolbar"
    sleep 1
    printlog "click button 'Connector' on Toolbar"
    Verbinder.Click
    printlog "make a line with the mouse"
    gMouseMove 30,30,70,70
    sleep 3
    Kontext "Toolbar"
    GluePoints.Click
    sleep 1
    printlog "open Glue Points Toolbar"
    Kontext "Gluepointsobjectbar"
    if Gluepointsobjectbar.Exists = False Then
        ViewToolbarsGluepoints
    end if
    sleep 3
    '-------------------------------------- Klebepunkt einfuegen ----
    try
        printlog "click button 'Insert Glue Point' on Objecttoolbar"
        PunkteEinfuegen.Click
        sleep 2
        printlog "click 2 times onto the created line"
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
        printlog "click button 'Exit Direction Left' on Objecttoolbar"
        Links.Click
        Printlog "- gluepoint left works"
    catch
        Warnlog "- gluepoint left does not work"
    endcatch
    sleep 1
    '-------------------------------------- Rechts ------------------
    try
        printlog "click button 'Exit Direction Right' on Objecttoolbar"
        Rechts.Click
        Printlog "- gluepoint right works"
    catch
        Warnlog "- gluepoint right does not work"
    endcatch
    sleep 1
    '-------------------------------------- Oben --------------------
    try
        printlog "click button 'Exit Direction Top' on Objecttoolbar"
        Oben.Click
        Printlog "- gluepoint top works"
    catch
        Warnlog "- gluepoint top does not work"
    endcatch
    sleep 1
    '-------------------------------------- Unten -------------------
    try
        printlog "click button 'Exit Direction Bottom' on Objecttoolbar"
        Unten.Click
        Printlog "- gluepoint bottom works"
    catch
        Warnlog "- gluepoint bottom does not work"
    endcatch
    sleep 1
    '---------------------- Position an Objekt anpassen -------------
    try
        printlog "click button 'Glue Point Relative' on Objecttoolbar"
        PositionAnObjektAnpassen.Click
        Printlog "- align position to object works"
        gMouseClick 25,30      ' if you don't click onto an existing point, the state changes back :-[
    catch
        Warnlog "- align position to object does not work"
    endcatch
    Kontext "Gluepointsobjectbar"
    sleep 1
    '------------------------------------- Horizontal links ---------
    if PositionAnObjektAnpassen.exists then printlog "PositionAnObjektAnpassen = Exists"
    if PositionAnObjektAnpassen.GetState(2) <> 0 then PositionAnObjektAnpassen.Click ' make unpressed!
    '0 = not pressed. 1 = pressed.
    sleep 2
    try
        Kontext "Gluepointsobjectbar"
        printlog "click button 'Glue Point Horizontal Left' on Objecttoolbar"
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
            end if
        next i
    endcatch
    sleep 2
    '------------------------------------- Horizontal rechts --------
    try
        printlog "click button 'Glue Point Horizontal Right' on Objecttoolbar"
        HorizontalRechts.Click
        Printlog "- Align horizontal right does work"
    catch
        Warnlog "- Align horizontal right does notwork"
    endcatch
    sleep 1
    '------------------------------------- Horizontal zentriert -----
    try
        printlog "click button 'Glue Point Horizontal Center' on Objecttoolbar"
        HorizontalZentriert.Click
        Printlog "- align horizontal center does work"
    catch
        Warnlog "- align horizontal center does work"
    endcatch
    sleep 1
    '------------------------------------- Vertikal oben ------------
    try
        printlog "click button 'Glue Point Vertical Top' on Objecttoolbar"
        VertikalOben.Click
        sleep 1
        Printlog "- Align vertical top does work"
    catch
        Warnlog "- Align vertical top does not work"
    endcatch
    '------------------------------------- Vertikal unten -----------
    try
        printlog "click button 'Glue Point Vertical Bottom' on Objecttoolbar"
        VertikalUnten.Click
        sleep 1
        printlog "- Align vertical bottom does work"
    catch
        Warnlog "- Align vertical bottom does not work"
    endcatch
    '------------------------------------- Vertikal zentriert -------
    try
        printlog "click button 'Glue Point Vertical Center' on Objecttoolbar"
        VertikalZentriert.Click
        sleep 1
        Printlog "- Align vertical center does work"
    catch
        Warnlog "- Align vertical center does not work"
    endcatch
    Printlog "- End of testing gluepoints"
    printlog "Close the Option Bar via View->Toolbars->Option Bar"
    ViewToolbarsOptionbar
    sleep 2
    Kontext "Optionsbar"
    if Optionsbar.Exists = True Then
        warnlog "Couldnt close Optionsbar."
    end if
    printlog "close application"
    Call hCloseDocument
endcase 'tiGluepointToolbar

'----------------------------------------------------------------------------'

testcase tdBezierToolbar
    printlog "Open the Application"
    Call hNewDocument
    printlog "Create a rectangle."
    Call hRechteckErstellen ( 10, 10, 30, 40 )
    sleep (1)
    printlog "Convert the recttangle to an 'Polygon-object' via the context-menu for the object"
    Call hOpenContextMenu
    printlog "Convert to"
    Call hMenuSelectNr (11)
    printlog "Polygon-object"
    Call hMenuSelectNr (2)   
    sleep (1)
    printlog "In the context-menu, select 'Edit Points"
    Call hOpenContextMenu
    printlog "Select entry nr 5 (Edit Points)"
    Call hMenuSelectNr (5)  
    sleep (2)
    Kontext "Bezierobjectbar"
    if Bezierobjectbar.Exists <> TRUE then
        ViewToolbarsBezier
        Sleep (2)
        if Bezierobjectbar.Exists <> TRUE then
            warnlog "Bezierobjectbar did not show up. Check why."
        end if
    end if

    '----------------------------------- Punkte verschieben ------------------------------------
    printlog "Select a part of the object with the keyboard"
    printlog "Selects the fourth vector."
    hTypeKeys "<MOD1 TAB>", 4   
    printlog "Moves the selected vector 40 steps."
    hTypeKeys "<RIGHT>", 40

    printlog "The 'Object Toolbar' now changes"
    Kontext "Bezierobjectbar"
    if Bezierobjectbar.Exists <> TRUE then
        ViewToolbarsBezier
        Sleep 2
    end if
    if Bezierobjectbar.Exists <> TRUE then
        Warnlog "The Bezier-Objectbar should have been opened, but wasnt."
        ViewToolbarsBezier
        Sleep 2
    end if

    Kontext "Bezierobjectbar"
    sleep 1
    Printlog "- Move points"
    printlog "click button 'Move points' on 'Object Toolbar'"
    Verschieben.Click
    sleep 2
    hTypeKeys "<MOD1 TAB>"
    printlog "Marks the selected vector."
    hTypeKeys "<MOD1 SHIFT SPACE>"
    printlog "Moves the vector 20 with arrow right, twenty times."
    hTypeKeys "<RIGHT>", 20   

    Printlog "- Insert points"
    printlog "Click button 'Insert points' on 'Object Toolbar'"
    Kontext "Bezierobjectbar"
    Einfuegen.Click
    sleep 2
    Bezierobjectbar.Move 20, 20
    sleep (1)
    Printlog "- Delete points"
    printlog "Select another part of the object with spanning a selection with the mouse"
    gMouseMove 25,25,45,45
    printlog "Select the second next vector."
    hTypeKeys "<MOD1 TAB>", 2
    printlog "Mark the selected vector."
    hTypeKeys "<MOD1 SHIFT SPACE>"
    printlog "Select and mark the next vector."
    hTypeKeys "<MOD1 TAB><SHIFT SPACE>"  
    
    sleep 2
    Kontext "Bezierobjectbar"
    sleep 2
    Printlog "- Convert into curve"
    printlog "click button 'convert to curve' on 'Object Toolbar'"
    InKurve.Click
    sleep 2
    Printlog "- Place edge point"
    printlog "click button 'corner point' on 'Object Toolbar'"
    Ecke.Click
    sleep 2
    Printlog "- Smooth transition"
    printlog "click button 'smooth transition' on 'Object Toolbar'"
    Glatt.Click
    sleep 2
    Printlog "- Symetric transition"
    printlog "click button 'symmetric transition' on 'Object Toolbar'"
    Symmetrisch.Click
    sleep 2

    printlog "Click button 'Eliminate points' on 'Object Toolbar'"
    Kontext "Bezierobjectbar"
    PunkteReduzieren.Click
    sleep 2
    
    printlog "Mark another point"
    hTypeKeys "<MOD1 TAB>"
    hTypeKeys "<MOD1 SHIFT SPACE>"

    printlog "click button 'split curve' on 'Object Toolbar'"
    Kontext "Bezierobjectbar"
    try
        Auftrennen.Click
    catch
        Warnlog "- 'Break' could not be executed"
    endcatch
    sleep 2

    printlog "Mark all points"
    hTypeKeys "<MOD1 TAB>"
    hTypeKeys "<MOD1 A>"

    Printlog "- Close bezier"
    Kontext "Bezierobjectbar"
    try
        printlog "click button 'close bezier' on 'Object Toolbar'"
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

    printlog "Mark all points"
    hTypeKeys "<MOD1 TAB>"
    hTypeKeys "<MOD1 A>"

    printlog "Click button 'Delete points' on 'Object Toolbar'"
    Kontext "Bezierobjectbar"
    Printlog "- Break curve"
    if ( hClickButton( Loeschen ) = -1 ) then
        warnlog( "Unable to click button <Loeschen>" )
    endif

    printlog "Uncheck on the symbol 'Curve' on the toolbar 'Main Toolbar'"
    Kontext "Toolbar"
    sleep 2
    Toolbar.OpenContextMenu   ' Enable forms button in menuebar
    sleep 2
    hMenuselectNr (1)
    sleep 2
    hMenuItemUnCheck (7)
    sleep 2
    printlog "Close application"
    Call hCloseDocument
endcase 'tdBezierToolbar

'------------------------------------------------------------------------------