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
'* short description :  Testcases for the Objectbars.
'*
'\******************************************************************

sub w_010_1

    printLog Chr(13) + "--------- Objectbars . Part two. (w_010_1.inc) ----------"
    gApplication = "WRITER"

    '   ** Objectbar Table
    Call tInsertFormula
    Call tMergeCells
    Call tSplitCells
    Call tOptimize
    Call tInsertCellRow
    Call tDeleteRow
    Call tDeleteColumn

    '   ** Graphics ( same as OLE-Objects and Frames )
    Call tGraphicWrap
    Call tGraphicAlign
    Call tGraphicFlip
    Call tGraphicArrange
    Call tGraphicBorders
    Call tGraphicProperties
    Call tGraphicAnchor

    '   ** Formula Bar
    Call tFormulaCancel
    Call tFormulaApply

    'Writer Web
    gApplication = "HTML"
    Call tHtmlDocGraphicWrap
    Call tHtmlDocGraphicAlign
    Call tHtmlDocGraphicProperties
    gApplication = "WRITER"

end sub
'-----------------------------------------------------------

testcase tInsertFormula
    printlog "- Table-Objectbar: Formula border in Writer"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a table"
    Call hTabelleEinfuegen
    printlog " Click 'Sum' in object bar"
    Call hToolbarSelect ( "Table", true )
    if TableObjectbar.IsVisible = true then
        try
            FormelEingeben.Click
        catch
            Warnlog "Unable to access 'Formula'-Button on toolbar!"
            Call hCloseDocument
            goto endsub
        endcatch
    else
        Warnlog "TableObjectbar is not visible!"
        Call hCloseDocument
        goto endsub
    end if

    Sleep 1
    Kontext "OL_SW_Rechenleiste"
    if OL_SW_Rechenleiste.NotExists then
        warnlog "Formula border doesn't come up"
    else
        printlog " Click 'Cancel' in object bar"
        Verwerfen.Click
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tMergeCells
    printlog "- Table-Objectbar: Merge Cells"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a table"
    Call hTabelleEinfuegen
    printlog " Hit <Shift Right> 2 times"
    Call wTypeKeys ("<Shift Right>", 2)
    Sleep 2
    printlog " Click 'Merge cells' in object bar"
    Call hToolbarSelect ( "Table", true )
    if TableObjectbar.IsVisible = true then
        ZellenVerbinden.Click
    else
        Warnlog "TableObjectbar is not visible!"
        Call hCloseDocument
        goto endsub
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tSplitCells
    printlog "- Table-Objectbar: Split Cells"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a table"
    Call hTabelleEinfuegen
    Call hToolbarSelect ( "Table", true )
    printlog " Click 'Split Cells' in object bar"
    if TableObjectbar.IsVisible = true then
        ZelleTeilen.Click
    else
        Warnlog "TableObjectbar is not visible!"
        Call hCloseDocument
        goto endsub
    end if
    printlog " Cancel upcoming dialog"
    Kontext "ZellenTeilen"
    ZellenTeilen.Cancel
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tOptimize
    printlog "- Table-Objectbar: Optimize Table"
    printlog " Open new document"
    Call hNewDocument

    printlog " Insert a table and select all in document"
    Call hTabelleEinfuegen
    Call wTypeKeys ("<Mod1 A>")
    Call hToolbarSelect ( "Table", true )
    wait 500
    printlog " Tear Off toolbar 'Optimize'"
    Kontext "TableObjectBar"
    try
        Optimieren.TearOff
    catch
        Warnlog "Unable to Tear Off Toolbox 'Optimize'"
        Call hCloseDocument
        goto endsub
    endcatch
    Call wTypeKeys ("<Mod1 a>")
    Sleep 1

    Kontext "OptimizeTablebar"
    printlog " Click 'Space Columns Equally'"
    SpaltenGleichmaessig.Click
    printlog " Click 'Space Rows Equally'"
    ZeilenGleichmaessig.Click
    Sleep 2
    printlog " Click 'Optimal Row Height'"
    OptimaleZeilenhoehe.Click

    Call wTypeKeys ("Optimize Table")
    printlog " Enter some text"
    Sleep 1
    Kontext "OptimizeTablebar"
    printlog " Click 'Optimal Column Width'"
    OptimaleSpaltenbreite.Click
    Sleep 1
    printlog " Close toolbar 'Optimize'"
    OptimizeTablebar.Close
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertCellRow
    printlog "- Table-Objectbar: Insert Cell"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert Table"
    Call hTabelleEinfuegen
    Call hToolbarSelect ( "Table", true )
    if TableObjectbar.IsVisible = true then
        ZeileEinfuegen.Click
    else
        Warnlog "TableObjectbar is not visible!"
        Call hCloseDocument
        goto endsub
    end if

    printlog " Click 'Insert Column' in object bar"
    Sleep 1
    printlog "- Table-Objectbar: Insert Row"
    printlog " Click 'Insert Row' in object bar"
    SpalteEinfuegen.Click
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tDeleteRow
    printlog "- Table-Objectbar: Delete Row"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a table"
    Call hTabelleEinfuegen
    Call hToolbarSelect ( "Table", true )
    printlog " Click 'Delete Row'"
    if TableObjectbar.IsVisible = true then
        ZeileLoeschen.Click
    else
        Warnlog "TableObjectbar is not visible!"
        Call hCloseDocument
        goto endsub
    end if

    Sleep 1
    printlog " Click 'Delete Row'"
    ZeileLoeschen.Click
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tDeleteColumn
    dim i as Integer
    printlog "- Table-Objectbar: Delete Column"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a table"
    Call hTabelleEinfuegen
    printlog " Click 'Delete Column' 2 times"
    for i=1 to 2
        Call hToolbarSelect ( "Table", true )
        if TableObjectbar.IsVisible = true then
            SpalteLoeschen.Click
        else
            Warnlog "TableObjectbar is not visible!"
            Call hCloseDocument
            goto endsub
        end if
        Sleep 1
    next i
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tGraphicWrap
    printlog "- Grafik-(Rahmen)-Objectbar: Wrap"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert graphic 'writer/input/grafik/jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    Call hToolbarSelect("Frame", true)
    try
        printlog " Check 'Wrap Off' in object bar"
        NoWrap.Click
        Sleep 1
    catch
        Warnlog "'Wrap Off' in Objectbar couldn't be checked"
    endcatch

    if gApplication <> "HTML" then
        try
            printlog " Check 'Wrap through' in object bar"
            Durchlauf.Click
            Sleep 1
        catch
            Warnlog "'Wrap through' in Objectbar couldn't be checked"
        endcatch

        try
            printlog " Check 'Wrap On' in object bar"
            Wrap.Click
            Sleep 1
        catch
            Warnlog "'Wrap on' in Objectbar couldn't be checked"
        endcatch
    else
        Call hToolbarSelect("Picture", true)
        try
            printlog " Check 'Wrap Left' in object bar"
            WrapLeft.Click
            Sleep 1
        catch
            Warnlog "'Wrap Left' in Objectbar couldn't be checked"
        endcatch

        try
            printlog " Check 'Wrap Right' in object bar"
            WrapRight.Click
            Sleep 1
        catch
            Warnlog "'Wrap Right' in Objectbar couldn't be checked"
        endcatch
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tGraphicAlign
    printlog "- Graphic-(Frame)-Objectbar: Align"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert graphic 'writer/input/grafik/jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )

    Call hToolbarSelect("Frame", true)
    try
        printlog " Click 'Left' in object bar"
        Links.Click
        Sleep 1
    catch
        Warnlog "'Left' in Objectbar couldn't be checked"
    endcatch

    try
        printlog " Click 'Right' in object bar"
        Rechts.Click
        Sleep 1
    catch
        Warnlog "'Right' in Objectbar couldn't be checked"
    endcatch

    ' Only Left and Right in WriterWeb
    if gApplication = "HTML" then
        printlog " Close active document"
        Call hCloseDocument
        goto endsub
    end if

    try
        printlog " Click 'Centered' in object bar (Not WriterWeb)"
        Zentriert.Click
        Sleep 1
    catch
        Warnlog "'Centered' in Objectbar couldn't be checked"
    endcatch

    try
        printlog " Click 'Center' in object bar (Not WriterWeb)"
        Mitte.Click
        Sleep 1
    catch
        Warnlog "'Center' in Objectbar couldn't be checked"
    endcatch

    try
        printlog " Click 'Top' in object bar (Not WriterWeb)"
        Oben.Click
        Sleep 1
    catch
        Warnlog "'Top' in Objectbar couldn't be checked"
    endcatch

    try
        printlog " Click 'Bottom' in object bar (Not WriterWeb)"
        Unten.Click
        Sleep 1
    catch
        Warnlog "'Bottom' auf Objectbar couldn't be checked"
    endcatch

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tGraphicFlip
    printlog "- Graphic-(Frame)-Objectbar: Flip"
    if gApplication = "HTML" then
        Printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument
    printlog " Insert graphic 'writer/input/grafik/jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )

    Call hToolbarSelect("Picture", true)
    try
        printlog " Click 'Flip Horizontal' in object bar"
        HorizontalSpiegeln.Click
        Sleep 1
    catch
        Warnlog "'Flip Horizontal' in Objectbar couldn't be checked"
    endcatch

    try
        printlog " Click 'Flip Vertical' in object bar"
        VertikalSpiegeln.Click
        Sleep 1
    catch
        Warnlog "'Flip vertical' in Objectbar couldn't be checked"
    endcatch

    try
        printlog " Click 'Flip Horizontal' in object bar"
        HorizontalSpiegeln.Click
        Sleep 1
        printlog " Click 'Flip Vertical' in object bar"
        VertikalSpiegeln.Click
        Sleep 1
    catch
    endcatch

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tGraphicArrange
    printlog "- Graphic-(Frame)-Objectbar: Arrange"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert graphic 'writer/input/grafik/jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )

    Call hToolbarSelect("Frame", true)
    try
        printlog " Click 'Send to back' in object bar"
        GanzNachHinten.Click
        Sleep 1
    catch
        Warnlog "'Send to back' in Objectbar couldn't be checked"
    endcatch

    try
        printlog " Click 'Bring to front' in object bar"
        GanzNachVorn.Click
        Sleep 1
    catch
        Warnlog "'Bring to front' in Objectbar couldn't be checked"
    endcatch

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tGraphicBorders
    printlog "- Graphic-(Frame)-Objectbar: Border"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert graphic 'writer/input/grafik/jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    Call hToolbarSelect("Frame", true)
    try
        printlog " Tear off Toolbox Border"
        Umrandung.TearOff
        Sleep 1
        Kontext "TB_Umrandung"
        TB_Umrandung.Move ( 20, 20 )
        printlog " Close toolbox"
        TB_Umrandung.Close
        Sleep 1
    catch
        Warnlog "Error in Toolbox Border"
    endcatch

    try
        Kontext "FrameObjectbar"
        printlog " Tear off toolbox 'Borderstyle'"
        Linienstil.TearOff
        Sleep 1
        Kontext "TB_Umrandungsstil"
        TB_Umrandungsstil.Move ( 20, 20 )
        printlog " Close toolbox"
        TB_Umrandungsstil.Close
        Sleep 1
    catch
        Warnlog "Error in Toolbox Borderstyle"
    endcatch

    try
        Kontext "FrameObjectbar"
        printlog " Tear off Toolbox 'Color'"
        LinienFarbe.TearOff
        Sleep 1
        Kontext "TB_Farbe"
        TB_Farbe.Move ( 20, 20 )
        TB_Farbe.Close
        printlog " Close toolbox"
        Sleep 1
    catch
        warnlog "Error in Toolbox Colour"
    endcatch

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tGraphicProperties
    printlog "- Graphic-(Frame)-Objectbar: Properties"
    Dim j as integer

    printlog " Open new document"
    Call hNewDocument
    printlog " Insert graphic 'writer/input/grafik/jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    Call hToolbarSelect("Picture", true)
    try
        printlog " Click 'Graphic Properties' in object bar"
        InsertGraphic.Click
    catch
        Warnlog "Error in Objectproperties in Objectbar"
        printlog " Close active document"
        Call hCloseDocument
        goto endsub
    endcatch

    printlog " In 'Graphic' dialog step through all tabpages"
    Kontext
    active.SetPage TabType
    Kontext "TabType"
    Call DialogTest ( TabType )

    Kontext
    active.SetPage TabZusaetze
    Kontext "TabZusaetze"
    Call DialogTest ( TabZusaetze )

    Kontext
    active.SetPage TabUmlauf
    Kontext "TabUmlauf"
    Call DialogTest ( TabUmlauf )

    Kontext
    active.SetPage TabHyperlinkRahmen
    Kontext "TabHyperlinkRahmen"
    Call DialogTest ( TabHyperlinkRahmen )

    Kontext
    active.SetPage TabGrafik
    Kontext "TabGrafik"
    Call DialogTest ( TabGrafik )

    Kontext
    active.SetPage TabUmrandung
    Kontext "TabUmrandung"
    Call DialogTest ( TabUmrandung )

    Kontext
    active.SetPage TabMakro
    ' if no JRE is installed a messagebox appears
    Do
        j = j + 1
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                if j = 1 then Warnlog Active.Gettext
                Active.Ok
            else
                exit do
            end if
        else
            exit do
        end if
    Loop
    Kontext "TabMakro"
    Call DialogTest ( TabMakro )

    printlog " Close 'Graphic' dialog"
    if gApplication <> "HTML" then
        Kontext
        active.SetPage TabHintergrund
        Kontext "TabHintergrund"
        Call DialogTest ( TabHintergrund )

        Kontext
        active.SetPage TabZuschneiden
        Kontext "TabZuschneiden"
        Call DialogTest ( TabZuschneiden )
        TabZuschneiden.Cancel
    else
        TabMakro.Cancel
    end if
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tGraphicAnchor

    printlog "- Graphic-(Frame)-Objectbar: Anchor"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert graphic 'writer/input/grafik/jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )

    Call hToolbarSelect("Frame", true)
    Sleep 2
    printlog " Select Anchor 'to page'"
    try
        VerankerungWechseln.Click
        printlog " Click 'Change Anchor' in object bar"
    catch
        Warnlog "Error on anchoring objects"
        Call hCloseDocument
        printlog " Close active document"
        goto endsub
    endcatch
    Sleep 1
    MenuSelect ( MenuGetItemID (1) )
    Sleep 1
    printlog " Select Anchor 'Paragraph'"
    Kontext "FrameObjectbar"
    VerankerungWechseln.Click
    MenuSelect ( MenuGetItemID (2) )
    Sleep 1
    printlog " Select Anchor 'To Character'"
    Kontext "FrameObjectbar"
    VerankerungWechseln.Click
    MenuSelect ( MenuGetItemID (3) )
    Sleep 1
    printlog " Select Anchor 'As Character'"
    Kontext "FrameObjectbar"
    VerankerungWechseln.Click
    MenuSelect ( MenuGetItemID (4) )
    Sleep 1

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormulaCancel
    printlog "- Formula Writer: Cancel"
    printlog " Open new document"
    Call hNewDocument
    printlog " Press <F2>"
    Call wTypeKeys ("<F2>")

    printlog " enter a formula"
    Kontext "OL_SW_Rechenleiste"
    Eingabefeld.TypeKeys "=summe(a+b)"
    printlog " Click 'Cancel' in formula bar"
    Verwerfen.Click
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormulaApply
    printlog "- Formula Writer: Apply"
    Call hNewDocument
    printlog " Open new document"
    Call hTabelleEinfuegen
    printlog " Insert a table"
    Call wTypeKeys ("12<Right>13<Down>")
    printlog " Write in document 12<Cursor Right>13<Cursor Down>"
    Call hToolbarSelect ( "Table", true )
    printlog " Click 'Sum' in object bar"
    if TableObjectbar.IsVisible = true then
        FormelEingeben.Click
    else
        Warnlog "TableObjectbar is not visible!"
        Call hCloseDocument
        goto endsub
    end if

    Kontext "OL_SW_Rechenleiste"
    Eingabefeld.TypeKeys "(" + Chr(60)+ "A1:B1"
    Eingabefeld.TypeKeys Chr(62) + ")"
    printlog " Enter as formula (A1:B1) and click 'Apply'"
    Uebernehmen.Click
    Sleep 2
    printlog " Check result of formula"
    Call wTypeKeys ("<Mod1 A>")
    EditCopy
    if GetClipboardText <> "25" then
        warnlog "Not possible to calculate in tables anymore. Result isn't 25, is: " + GetClipboardText
    end if
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tHtmlDocGraphicWrap
    printlog "- Grafik-(Rahmen)-Objectbar: Wrap"
    gApplication = "HTML"

    printlog " Open new document"
    Call hNewDocument

    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    printlog " Insert graphic 'writer/input/grafik/jolink.jpg'"
    Call hToolbarSelect("Frame", true)
    try
        printlog " Check 'Wrap Off' in object bar"
        NoWrap.Click
        Sleep 1
    catch
        Warnlog "'Wrap Off' in Objectbar couldn't be checked"
    endcatch

    Call hToolbarSelect("Picture", true)
    try
        WrapLeft.Click
        printlog " Check 'Wrap Left' in object bar"
        Sleep 1
    catch
        Warnlog "'Wrap Left' in Objectbar couldn't be checked"
    endcatch

    try
        printlog " Check 'Wrap Right' in object bar"
        WrapRight.Click
        Sleep 1
    catch
        Warnlog "'Wrap Right' in Objectbar couldn't be checked"
    endcatch

    Call hCloseDocument
    printlog " Close active document"
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tHtmlDocGraphicAlign
    printlog "- Graphic-(Frame)-Objectbar: Align"
    gApplication = "HTML"

    printlog " Open new document"
    Call hNewDocument
    printlog " Insert graphic 'writer/input/grafik/jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )

    Call hToolbarSelect("Frame", true)
    try
        Links.Click
        printlog " Click 'Left' in object bar"
        Sleep 1
    catch
        Warnlog "'Left' in Objectbar couldn't be checked"
    endcatch

    try
        Rechts.Click
        printlog " Click 'Right' in object bar"
        Sleep 1
    catch
        Warnlog "'Right' in Objectbar couldn't be checked"
    endcatch

    ' Only Left and Right in WriterWeb
    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tHtmlDocGraphicProperties
    printlog "- Graphic-(Frame)-Objectbar: Properties"
    Dim j as integer

    gApplication = "HTML"
    printlog " Open new document"
    Call hNewDocument

    printlog " Insert graphic 'writer/input/grafik/jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    Call hToolbarSelect("Picture", true)
    try
        printlog " Click 'Graphic Properties' in object bar"
        InsertGraphic.Click
    catch
        Warnlog "Error in Objectproperties in Objectbar"
        printlog " Close active document"
        Call hCloseDocument
        goto endsub
    endcatch

    printlog " In 'Graphic' dialog step through all tabpages"
    Kontext
    active.SetPage TabType
    Kontext "TabType"
    Call DialogTest ( TabType )

    Kontext
    active.SetPage TabZusaetze
    Kontext "TabZusaetze"
    Call DialogTest ( TabZusaetze )

    Kontext
    active.SetPage TabUmlauf
    Kontext "TabUmlauf"
    Call DialogTest ( TabUmlauf )

    Kontext
    active.SetPage TabHyperlinkRahmen
    Kontext "TabHyperlinkRahmen"
    Call DialogTest ( TabHyperlinkRahmen )

    Kontext
    active.SetPage TabGrafik
    Kontext "TabGrafik"
    Call DialogTest ( TabGrafik )

    Kontext
    active.SetPage TabUmrandung
    Kontext "TabUmrandung"
    Call DialogTest ( TabUmrandung )

    Kontext
    active.SetPage TabMakro
    ' if no JRE is installed a messagebox appears
    Do
        j = j + 1
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                if j = 1 then Warnlog Active.Gettext
                Active.Ok
            else
                exit do
            end if
        else
            exit do
        end if
    Loop
    Kontext "TabMakro"
    Call DialogTest ( TabMakro )
    printlog " Close dialog"
    TabMakro.Cancel

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------
