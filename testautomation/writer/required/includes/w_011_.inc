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
'* short description:  Testcases for the Toolbar.
'*
'\******************************************************************

sub w_011_

    printLog Chr(13) + "---------   Toolbar ( w_011_.inc )   ----------"
    gApplication = "WRITER"
    'Objektleiste Grafik
    Call tGraphicFilters

    'Objektleiste Zeichenobjekt
    Call tZOEditPoints
    Call tZOLine
    Call tZOArea
    Call tZORest
    Call tZOText
    Call tZOVerticalText

end sub

'-----------------------------------------------------------

testcase tGraphicFilters
    printlog "- Objectbar Graphic (Filters)"
    if gApplication = "HTML" then
        printlog "This test applies not to HTML-Document"
        Printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog "Open new document"
    Call hNewDocument
    printlog "Insert graphic '/writer/required/input/grafik/jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )

    printlog "- Select all Filters"
    Kontext "GraphicObjectbar"
    printlog "In graphic toolbox taer off 'Filter' Toolbox"
    if Not GraphicObjectbar.Exists then hToolbarSelect ( "Picture", true )
    try
        Filter.TearOff
    catch
        Warnlog "Unable to tear off 'Filter'-Toolbar"
        Call hCloseDocument
        goto endsub
    endcatch

    Kontext "GraphicFilterBar"
    try
        GraphicFilterBar.Move ( 20, 20 )
    catch
        Warnlog "Unable to move toolbar"
    endcatch

    printlog "-  Invert"
    printlog "Select 'Invert'"
    if Invert.IsEnabled then
        Invert.Click
    else
        Warnlog "-  'Invert' is disabled !"
    end if

    if CheckForFilters=False then
        printlog "Close active document"
        Call hCloseDocument
        goto endsub
    end if
    printlog "Click 'Smooth'"
    printlog "-  Smooth"
    if Smooth.IsEnabled then
        Smooth.Click
    else
        Warnlog "-  'Smooth' is disabled!"
    end if

    if CheckForFilters=False then
        printlog "Close active document"
        Call hCloseDocument
        goto endsub
    end if
    printlog "-  Sharpen"
    printlog "Click 'Sharpen'"
    if Sharpen.IsEnabled then
        Sharpen.Click
    else
        Warnlog "-  'Sharpen' is disabled!"
    end if

    UseBindings
    if CheckForFilters=False then
        printlog "Close active document"
        Call hCloseDocument
        goto endsub
    end if
    printlog "-  Remove Noise"
    printlog "Click 'Remove Noise'"
    if Remove.IsEnabled then
        Remove.Click
    else
        Warnlog "-  'Remove Noise' is disabled!"
    end if
    sleep (1)

    if ( CheckForFilters=False ) then
        printlog "Close active document"
        Call hCloseDocument
        goto endsub
    end if
    printlog "-  Solarization"
    printlog "Click 'Solarization'"
    if Solarization.IsEnabled then
        Solarization.Click
        Kontext "Solarization"
        printlog "Cancel upcoming 'Solarization' dialog"
        if Solarization.Exists then
            Call Dialogtest( Solarization )
            Solarization.Cancel
        else
            Warnlog "-  Dialog 'Solarisation' has not been opened! Try a second time!"
            Kontext "GraphicFilterBar"
            if Solarization.IsEnabled then
                Solarization.Click
                Kontext "Solarization"
                if Solarization.Exists then
                    Call Dialogtest( Solarization )
                    Solarization.Cancel
                else
                    Warnlog "-  Dialog 'Solarisation' has not been opened!"
                end if
            end if
        end if
    else
        Warnlog "-  'Solarization' is disabled!"
    end if

    if CheckForFilters=False then
        printlog "Close active document"
        Call hCloseDocument
        goto endsub
    end if
    printlog "-  Aging"
    printlog "Click 'Aging'"
    if Aging.IsEnabled then
        Aging.Click
        printlog "Cancel upcoming 'Aging' dialog"
        Kontext "Aging"
        if Aging.Exists then
            Call Dialogtest( Aging )
            Aging.Cancel
        else
            Warnlog "-  Dialog 'Aging' has not been opened! Try a second time!"
            Kontext "GraphicFilterBar"
            if Aging.IsEnabled then
                Aging.Click
                Kontext "Aging"
                if Aging.Exists then
                    Call Dialogtest( Aging )
                    Aging.Cancel
                else
                    Warnlog "-  Dialog 'Aging' has not been opened!"
                end if
            end if
        end if
    else
        Warnlog "-  'Aging' is disabled!"
    end if

    if CheckForFilters=False then
        printlog "Close active document"
        Call hCloseDocument
        goto endsub
    end if
    printlog "-  Posterize"
    printlog "Click 'Posterize'"
    if Posterize.IsEnabled then
        Posterize.Click
        printlog "Cancel upcoming 'Posterize' dialog"
        Kontext "Posterize"
        if Posterize.Exists then
            Call Dialogtest( Posterize )
            Posterize.Cancel
        else
            Warnlog "-  Dialog 'Posterize' has not been opened! Try a second time!"
            Kontext "GraphicFilterBar"
            if Posterize.IsEnabled then
                Posterize.Click
                Kontext "Posterize"
                if Posterize.Exists then
                    Call Dialogtest( Posterize )
                    Posterize.Cancel
                else
                    Warnlog "-  Dialog 'Posterize' has not been opened!"
                end if
            end if
        end if
    else
        Warnlog "-  'Posterize' is disabled!"
    end if

    if CheckForFilters=False then
        printlog "Close active document"
        Call hCloseDocument
        goto endsub
    end if
    printlog "Click 'Pop Art'"
    printlog "-  Pop Art"
    if Art.IsEnabled then
        Art.Click
    else
        Warnlog "-  'Pop Art' is disabled!"
    end if

    if CheckForFilters=False then
        printlog "Close active document"
        Call hCloseDocument
        goto endsub
    end if
    printlog "-  Charcoal Sketch"
    printlog "Click 'Charcoal Sketch'"
    if CharcoalSketch.IsEnabled then
        CharcoalSketch.Click
    else
        Warnlog "-  'Charcoal Sketch' is disabled!"
    end if

    if CheckForFilters=False then
        printlog "Close active document"
        Call hCloseDocument
        goto endsub
    end if
    printlog "-  Relief"
    printlog "Click 'Relief'"
    if Relief.IsEnabled then
        Relief.Click
        Kontext "Relief"
        if Relief.Exists then
            Call Dialogtest( Relief )
            Relief.Cancel
        else
            printlog "Cancel upcoming 'relief' dialog"
            Warnlog "-  Dialog 'Relief' has not been opened! Try a second time!"
            Kontext "GraphicFilterBar"
            if Relief.IsEnabled then
                Relief.Click
                Kontext "Relief"
                if Relief.Exists then
                    Call Dialogtest( Relief )
                    Relief.Cancel
                else
                    Warnlog "-  Dialog 'Relief' has not been opened!"
                end if
            end if
        end if
    else
        Warnlog "-  'Relief' is disabled!"
    end if

    if CheckForFilters=False then
        Printlog "Close active document"
        Call hCloseDocument
        goto endsub
    end if
    printlog "-  Mosaic"
    Printlog "Clock 'Mosaic'"
    if Mosaic.IsEnabled then
        Mosaic.Click
        Kontext "Mosaic"
        Printlog "Cancel upcoming 'Mosaic' dialog"
        if Mosaic.Exists then
            Call Dialogtest( Mosaic )
            Mosaic.Cancel
        else
            Warnlog "-  Dialog 'Mosaic' has not been opened!"
        end if
    else
        Warnlog "-  'Mosaic' is disabled!"
    end if

    Printlog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tZOEditPoints
    printlog "- Draw Functions-Objectbar: Edit Points"
    if gApplication = "HTML" then
        Printlog "This test does not apply to HTML documents"
        Printlog "Not in WriterWeb!"
        goto endsub
    end if
    Printlog "Open new document"
    Call hNewDocument
    Printlog "Insert a drawing object (rectangle)"
    Call wZeichenobjektEinfuegen ( "Rechteck", 30, 30, 50, 60 )
    Printlog "Select drawing object with shortcut 'Shift F4'"
    Call wTypeKeys ( "<Shift F4>" )

    Kontext "Drawbar"
    if Not Drawbar.Exists then Call hToolbarSelect("Drawing", true)

    Sleep 1
    Printlog "In Toolbox click 'Edit Points'"
    try
        Points.Click
    catch
        Warnlog "#i56594#Shortcut selecting objects doesn't work"
        Printlog "Close active document"
        Call hCloseDocument
        goto endsub
    endcatch
    Sleep 1
    Printlog "Click 'Edit Points' in toolbox again"
    Points.Click
    Printlog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tZOLine
    printlog "- Draw Functions-Objectbar: all within Line"
    if gApplication = "HTML" then
        Printlog "This test does not apply to HTML documents"
        Printlog "Not in WriterWeb!"
        goto endsub
    end if
    printlog "Open new document"
    Call hNewDocument

    printlog "Insert a drawing object ( rectangle )"
    Call wZeichenobjektEinfuegen ( "Rechteck", 30, 30, 50, 60 )
    Call gMouseClick ( 25, 25 )
    printlog "Select drawing object with shortcut 'Shift F4'"
    Call wTypeKeys ( "<Shift F4>" )

    Kontext "DrawingObjectbar"
    if Not DrawingObjectbar.Exists then hToolbarSelect("GRAPHIC", true)

    printlog "  - Line-Tabdialog"
    wait 500
    try
        Linie.Click
        printlog "In Toolbox click 'Line'"
    catch
        Warnlog "#i56594#Shortcut selecting objects doesn't work"
        Call hCloseDocument
        goto endsub
    endcatch
    Kontext
    printlog "Step through all tabpages in upcoming dialog"
    wait 500
    Active.SetPage TabLinie
    Kontext "TabLinie"
    TabLinie.OK
    Kontext "DrawingObjectbar"
    if Not DrawingObjectbar.Exists then hToolbarSelect("GRAPHIC", true)

    printlog "  - Arrow Styles"
    Linienendenstil.TearOff
    Kontext "Linienenden"
    Call Dialogtest(Linienenden)
    printlog "Close dialog"
    Linienenden.Close

    Sleep 1
    printlog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tZOArea
    printlog "- Draw Functions-Objectbar: all within rectangle"
    if gApplication = "HTML" then
        printlog "This test applies not to HTML document"
        Printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog "Open new document"
    Call hNewDocument
    printlog "Insert a draw object ( rectangle )"
    Call wZeichenobjektEinfuegen ( "Rechteck", 30, 30, 50, 60 )
    ' Set focus to document
    Call gMouseMove2 ( 50, 25 )
    Call gMouseClick ( 50, 25 )
    printlog "Select drawing object with shortcut 'Shift F4'"
    Call wTypeKeys ( "<Shift F4>" )

    Kontext "DrawingObjectbar"
    if Not DrawingObjectbar.Exists then hToolbarSelect("GRAPHIC", true)
    printlog "  - Rectangle-Tabdialog"
    wait 500
    try
        printlog "In toolbox click 'Area'"
        Flaeche.Click
    catch
        Warnlog "#i56594#Shortcut selecting objects doesn't work"
        Call hCloseDocument
        goto endsub
    endcatch
    printlog "Close upcoming dialog"
    Kontext
    Active.SetPage TabArea
    Kontext "TabArea"
    TabArea.OK

    printlog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tZORest

    printlog "- Draw Functions-Objectbar: Rest"
    if gApplication = "HTML" then
        printlog "This test does not apply to HTML documents"
        Printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog "Open new document"
    Call hNewDocument
    printlog "Insert a draw object ( rectangle )"
    Call wZeichenobjektEinfuegen ( "Rechteck", 30, 30, 50, 60 )
    ' Set focus to document
    Call gMouseMove2 ( 50, 25 )
    Call gMouseClick ( 50, 25 )
    printlog "Select drawing object with shortcut 'Shift F4'"
    Call wTypeKeys ( "<Shift F4>" )

    printlog "  - Object rotation mode"
    Sleep 2
    try
        printlog "Intoolbox click 'Object rotation mode'"
        Kontext "DrawingObjectbar"
        Drehen.Click
        wait 500
    catch
        Warnlog "Shortcut selecting objects doesn't work"
        Call hCloseDocument
        goto endsub
    endcatch

    printlog "  - Change Anchor"
    printlog "Click 'Change Anchor' in Toolbox and select 1st entry in anchor list"
    VerankerungWechseln.Click
    Sleep 1
    Call hMenuSelectNr (1)
    wait 500

    VerankerungWechseln.Click
    printlog "Click 'Change Anchor' in Toolbox and select 2nd entry in anchor list"
    Sleep 1
    Call hMenuSelectNr (2)
    wait 500

    VerankerungWechseln.Click
    Sleep 1
    printlog "Click 'Change Anchor' in Toolbox and select 3rd entry in anchor list"
    Call hMenuSelectNr (3)
    wait 500

    printlog "  - To Background"
    printlog "Click 'To Background' in toolbox"
    InDenHintergrund.Click
    Sleep 2
    printlog "  - To Foreground"
    printlog "Click 'To Foreground' in toolbox"
    InDenVordergrund.Click

    printlog "  - Send to back"
    printlog "Click 'Send To back' in toolbox"
    GanzNachHinten.Click

    printlog "  - Bring to Front"
    printlog "Click 'To Background' in toolbox"
    GanzNachVorn.Click

    printlog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tZOText
    printlog "- Draw Functions-Objectbar: Text"
    if gApplication = "HTML" then
        printlog "This test does not apply to HTML documents"
        Printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog "Open new document"
    Call hNewDocument

    Call wZeichenobjektEinfuegen ( "Text", 30, 30, 50, 60 )
    printlog "Insert draw object ( Text )"
    printlog "Insert some text to textbox"
    Call wTypeKeys ("Draw Functions-Objectbar: Text")
    printlog "Select the text in textbox with <Strg A>"
    Call wTypeKeys ("<Mod1 A>", 3)
    Sleep 4

    Kontext "DrawTextObjectBar"
    if DrawTextObjectBar.Exists = false then
        Call hToolbarSelect("TextObject", true)
    end if

    printlog "  - Font Size"
    printlog "For font size select 9th entry"
    try
        Schriftgroesse.Select 9
        Schriftgroesse.TypeKeys "<Return>"
    catch
        Warnlog "Unable to change font size"
    endcatch
    Wait 500

    printlog "  - Font"
    printlog "In object bar select 6th entry in combobox 'Font'"
    try
        Schriftart.Select 6
    catch
        Warnlog "Unable to select font in object bar"
    endcatch
    Schriftart.TypeKeys "<Return>"
    Wait 500

    printlog "  - Bold"
    printlog "Click 'Bold' in object bar"
    Fett.Click
    Wait 500

    printlog "  - Italic"
    printlog "Click 'Italic' in object bar"
    Kursiv.Click
    Wait 500

    printlog "  - Underline"
    printlog "Click 'Underline' in object bar"
    Unterstrichen.Click
    Wait 500

    printlog "  - Align Left"
    printlog "Click Align left in object bar"
    Linksbuendig.Click
    Wait 500

    printlog "  - Align Right"
    printlog "Click 'Align right' in object bar"
    Rechtsbuendig.Click
    Wait 500

    printlog "  - Centered"
    printlog "Click 'Centered' in object bar"
    Zentriert.Click
    Wait 500

    printlog "  - Justify"
    printlog "Click 'Justify' in object bar"
    Blocksatz.Click
    Wait 500

    printlog "  - Superscript"
    printlog "Click 'Superscript' in object bar"
    try
        Hochgestellt.Click
        Wait 500
    catch
        Warnlog "- Superscript is probably faded out!"
    endcatch

    printlog "  - Subscript"
    try
        printlog "Click 'Subscript' in object bar"
        Tiefgestellt.Click
        Wait 500
    catch
        Warnlog "- Subscript is probably faded out!"
    endcatch

    printlog "  - Font attributes"
    try
        printlog "Click 'Font attributes' in object bar"
        Zeichenformat.Click
        wait 500
        printlog "Close upcoming dialog"
        Kontext
        Active.Setpage TabFont
        Kontext "TabFont"
        TabFont.OK
        Sleep 2
        Kontext "TextObjectbar"
    catch
        Warnlog "- Font attributes is probably faded out!"
    endcatch

    printlog "  - Format:Paragraph"
    Sleep 3
    try
        printlog "Click Format:Paragraph in object bar"
        Absatzformat.Click
        wait 500
        printlog "Close upcoming dialog"
        Kontext
        Active.Setpage TabEinzuegeUndAbstaende
        Kontext "TabEinzuegeUndAbstaende"
        TabEinzuegeUndAbstaende.OK
        Sleep 2
    catch
        Warnlog "- Format:Paragraph is probably faded out!"
    endcatch

    printlog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tZOVerticalText
    printlog "- Draw Functions-Objectbar: Vertical Text"
    if gAsianSup = False then goto endsub
    if gApplication = "HTML" then
        printlog "This test applies not to HTML document and only if Asian Support is enabled"
        Printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog "Open new document"
    Call hNewDocument

    printlog "Insert Draw object ( 'vertical Text' )"
    Call wZeichenobjektEinfuegen ( "VText", 30, 30, 50, 60 )
    printlog "Insert some text in textbox and select it"
    Call wTypeKeys ("Checking vertical text")
    Call wTypeKeys ("<Shift Home>")
    Sleep 4

    printlog "In object bar select 6th entry in combobox 'Font'"
    Call hToolbarSelect ( "Textobject", true  )
    printlog "  - Font"
    Schriftart.Select 6
    Schriftart.TypeKeys "<Return>"
    Wait 500

    printlog "  - Font Size"
    printlog "For font size select 9th entry"
    Schriftgroesse.Select 9
    Schriftgroesse.TypeKeys "<Return>"
    Wait 500

    printlog "  - Bold"
    printlog "Click 'Bold' in object bar"
    Fett.Click
    Wait 500

    printlog "  - Italic"
    printlog "Click 'Italic' in object bar"
    Kursiv.Click
    Wait 500

    printlog "  - Underline"
    printlog "Click 'Underline' in object bar"
    Unterstrichen.Click
    Wait 500

    printlog "  - Align Left"
    printlog "Click 'Align left' in object bar"
    Linksbuendig.Click
    Wait 500

    printlog "  - Align Right"
    printlog "Click 'Align Right' in object bar"
    Rechtsbuendig.Click
    Wait 500

    printlog "  - Centered"
    printlog "Click 'Centered' in object bar"
    Zentriert.Click
    Wait 500

    printlog "  - Justify"
    printlog "Click 'Justify' in object bar"
    Blocksatz.Click
    Wait 500

    printlog "  - Superscript"
    try
        printlog "Click 'Superscript' in object bar"
        Hochgestellt.Click
        Wait 500
    catch
        Warnlog "- Superscript is probably faded out!"
    endcatch

    printlog "  - Subscript"
    try
        printlog "Click 'Subscript' in object bar"
        Tiefgestellt.Click
        Wait 500
    catch
        Warnlog "- Subscript is probably faded out!"
    endcatch

    printlog "  - Font attributes"
    try
        printlog "Click 'Font attributes' in object bar"
        Zeichenformat.Click
        wait 500
        Kontext
        Active.Setpage TabFont
        Kontext "TabFont"
        TabFont.OK
        Sleep 2
        Kontext "TextObjectbar"
    catch
        Warnlog "- Font attributes is probably faded out!"
    endcatch

    printlog "  - Format:Paragraph"
    Sleep 3

    try
        printlog "Click 'Textdirection:Left To Right' in Objectbar"
        TextdirectionLeftToRight.Click
        wait 500
    catch
        Warnlog "Textdirection Left To Right probably faded out!"
    endcatch

    try
        printlog "Click 'Textdirection:Top to Bottom' in Objectbar"
        TextdirectionTopToBottom.Click
        wait 500
    catch
        Warnlog "Textdirection Top to Bottom probably faded out!"
    endcatch

    try
        printlog "Click 'Format: Paragraph' in object bar."
        Absatzformat.Click
        wait 500
        printlog "Close upcoming dialog"
        Kontext
        Active.Setpage TabEinzuegeUndAbstaende
        Kontext "TabEinzuegeUndAbstaende"
        TabEinzuegeUndAbstaende.OK
        Sleep 2
    catch
        Warnlog "- Format:Paragraph is probably faded out!"
    endcatch

    printlog "Close active document"
    Call hCloseDocument
endcase
