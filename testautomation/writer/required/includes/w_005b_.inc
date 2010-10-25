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
'* short description :  Testcases for the Format-Menu.
'*
'\***********************************************************************

sub w_005b_

    printLog Chr(13) + "--------- Menu Format / 2 (w_005b_.inc) ----------"
    gApplication = "WRITER"

    Call tFormatAlignment
    Call tFormatAlignmentArea
    Call tFormatAlignmentEdit
    Call tFormatArrange
    Call tFormatArrangeArea
    Call tFormatFlip
    Call tFormatGroup

    '    Object submenu
    Call tFormatPositionAndSize
    Call tFormatPositionAndSizeLegend
    Call tFormatLine
    Call tFormatArea
    Call tFormatText
    Call tFormatFontwork

    '    Frame
    Call hExportUmstellung ( TRUE )
    Call tFormatObject
    Call tFormatArrangeObject
    Call tFormatAlignmentObject
    Call tFormatAnchorObject
    Call tFormatWrapObject
    Call hExportUmstellung ( FALSE )

    'Picture ( jolink.jpg )
    Call tFormatGraphics

    'Editmode with Draw objects
    Call tFormatStyleEdit
    Call tFormatLineSpacing
    Call tFormatParagraphEdit

    'Writer Web
    gApplication = "HTML"
    Call tHtmlDocFormatGraphics
    Call tHtmlDocFormatAlignment
    Call tHtmlDocFormatObject
    gApplication = "WRITER"

end sub

'-----------------------------------------------------------

testcase tFormatGraphics
    PrintLog "- Format / Graphics"
    Dim j as integer

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert a graphic (..\writer\required\input\graphics\jolink.jpg)"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )

    printlog " Format / Graphic"
    if ( hUseAsyncSlot( "FormatGraphics" ) = -1 ) then
        Warnlog "- Format / Graphics disabled or hidden!"
        goto endsub
    end if

    printlog " Step through all tabpages"
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

    if gApplication <> "HTML" then
        Kontext
        active.SetPage TabZuschneiden
        Kontext "TabZuschneiden"
        Call DialogTest ( TabZuschneiden )

        Kontext
        active.SetPage TabHintergrund
        Kontext "TabHintergrund"
        Call DialogTest ( TabHintergrund )
    end if

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
endcase

'-----------------------------------------------------------

testcase tFormatArrange
    PrintLog "- Format / Arrange"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert a graphic from file '..\writer\required\input\graphics\jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    sleep(2)
    try
        printlog " Format / Arrange / Bring to front"
        FormatArrangeBringToFrontCalc
    catch
        Warnlog "'Bring to front' couldn't be executed!"
    endcatch
    WaitSlot()
    try
        FormatArrangeBringForward
        printlog " Format / Arrange / Bring forward"
    catch
        Warnlog "'Bring forward' couldn't be executed!"
    endcatch
    WaitSlot()
    try
        FormatArrangeSetBackward
        printlog " Format / Arrange / Set backward"
    catch
        Warnlog "'Send backward' couldn't be executed!"
    endcatch
    WaitSlot()
    try
        FormatArrangeSendToBack
        printlog " Format / Arrange / Send to back"
    catch
        Warnlog "'Send to back' couldn't be executed!"
        Sleep 1
    endcatch
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatAlignment

    PrintLog "- Format / Alignment"

    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a graphic from file '..\writer\required\input\graphics\jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    sleep(2)
    if ( gApplication = "HTML" ) then
        printlog " Format / Anchor / to Page"
        hUseAsyncSlot( "FormatAnchorToPage" )
    end if
    
    try
        printlog " Format / Alignment / Left"
        FormatAlignmentLeft
        WaitSlot()
    catch
        Warnlog "'Alignment left' couldn't be executed!"
    endcatch

    try
        printlog " Format / Alignment / Right"
        FormatAlignmentRight
        WaitSlot()
    catch
        Warnlog "'Alignment right' couldn't be executed!"
    endcatch

    try
        printlog " Format / Alignment / Top"
        FormatAlignmentTop
        WaitSlot()
    catch
        Warnlog "'Alignment top' couldn't be executed!"
    endcatch

    try
        printlog " Format / Alignment / Center"
        FormatAlignmentCenter
        WaitSlot()
    catch
        Warnlog "'Alignment center' couldn't be executed!"
    endcatch

    try
        printlog " Format / Alignment / Bottom"
        FormatAlignmentBottom
        WaitSlot()
    catch
        Warnlog "'Alignment bottom' couldn't be executed!"
    endcatch

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatObject
    PrintLog "- Format / Object"
    Dim j as integer

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert a frame"
    Call wInsertFrame

    printlog " Format / Frame"
    FormatFrame

    printlog " Step through all tabpages"
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
    active.SetPage TabUmrandung
    Kontext "TabUmrandung"
    Call DialogTest ( TabUmrandung )

    Kontext
    active.SetPage TabHintergrund
    Kontext "TabHintergrund"
    Call DialogTest ( TabHintergrund )

    if gApplication <> "HTML" then
        Kontext
        active.SetPage TabHyperlinkRahmen
        Kontext "TabHyperlinkRahmen"
        Call DialogTest ( TabHyperlinkRahmen )

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
    else
        printlog " Close dialog"
        TabHintergrund.Cancel
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatArrangeObject
    PrintLog "- Format / Arrange Frame"

    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a frame"
    Call wInsertFrame
    printlog " Format / Arrange / Bring to front"
    FormatArrangeBringToFrontCalc : WaitSlot()

    printlog " Format / Arrange / Bring forward"
    FormatArrangeBringForward : WaitSlot()

    printlog " Format / Arrange / Set backward"
    FormatArrangeSetBackward : WaitSlot()

    printlog " Format / Arrange / Send to back "
    FormatArrangeSendToBack : WaitSlot()

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatAlignmentObject
    PrintLog "- Format / Alignment Frame"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a frame"
    Call wInsertFrame
    printlog " Format / Alignment / Left"
    FormatAlignmentLeft : WaitSlot()

    printlog " Format / Alignment / Centered"
    FormatAlignmentCentered : WaitSlot()

    printlog " Format / Alignment / Right"
    FormatAlignmentRight : WaitSlot()

    printlog " Format / Alignment / Top"
    FormatAlignmentTop : WaitSlot()

    printlog " Format / Alignment / Center"
    FormatAlignmentCenter : WaitSlot()

    printlog " Format / Alignment / Bottom"
    FormatAlignmentBottom : WaitSlot()

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatAnchorObject

    PrintLog "- Format / Anchor Frame"

    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a frame"
    Call wInsertFrame
    printlog " Format / Anchor / to page"
    FormatAnchorToPage : WaitSlot()

    printlog " Format / Anchor / to paragraph"
    FormatAnchorToParagraph : WaitSlot()

    printlog " Format / Anchor / to character"
    FormatAnchorToCharacter : WaitSlot()

    printlog " Format / Anchor / as character"
    FormatAnchorAsCharacter : WaitSlot()

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatWrapObject

    PrintLog "- Format / Wrap (Frame)"

    if ( gApplication = "HTML" ) then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert a frame"
    Call wInsertFrame

    printlog " Format / Wrap / no wrap"
    FormatWrapNoWrap : WaitSlot()

    printlog " Format / Wrap / page wrap"
    FormatWrapPageWrap : WaitSlot()

    printlog " Format / Wrap / optimal page wrap"
    FormatWrapOptimalPageWrap : WaitSlot()

    printlog " Format / Wrap / through"
    FormatWrapThrough : WaitSlot()

    printlog " Format / Wrap / in background"
    FormatWrapInBackground : WaitSlot()

    printlog " Format / Wrap / first paragraph"
    FormatWrapFirstParagraph : WaitSlot()

    if ( hUseAsyncSlot( "FormatWrapContour" ) = -1 ) then
        printlog( "Format / Wrap / Contour disabled. Good." )
    else
        warnlog( "Format / Wrap / Contour should not be enabled for frames" )
    end if

    printlog " Format / Wrap / Edit Contour"
    if ( hUseAsyncSlot( "FormatWrapEditContour" ) = -1 ) then
        printlog( "Format / Wrap / Edit Contour disabled. Good." )
    else
        Warnlog( "Wrap / Edit Contour should not be enabled for frames" )

        Kontext "KonturEditor"
        if ( KonturEditor.exists( 2 ) ) then
            KonturEditor.Close()
        else
            warnlog( "Slot <FormatWrapEditContour> is enabled but doesn't trigger the dialog" )
        end if
    end if

    Kontext
    if ( Active.Exists( 2 ) ) then 
        qaerrorlog( "JSK: Undocumented active handled silently - fix it" )
        Active.Yes()
    end if

    printlog " Close active document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatLine
    PrintLog "- Format / Line in drawing object"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a rectangle"
    Call wRectangleToClipboard

    printlog " Edit / Paste"
    hUseAsyncSlot( "EditPaste" )

    printlog " Format / Line"
    if ( hUseAsyncSlot( "FormatLine" ) = -1 ) then
        Warnlog "Clipboard problem between applications!"
        Call wZeichenobjektEinfuegen ( "Rechteck", 40, 40, 50, 50 )
        Call gMouseClick ( 45, 45 )
        if ( hUseAsyncSlot( "FormatLine" ) = -1 ) then
            warnlog( "Failed to execute slot <FormatLine>" )
        end if
    end if

    Kontext
    printlog " Step through all tabpages"
    wait 500
    active.SetPage TabLinie

    kontext "TabLinie"
    Call DialogTest ( TabLinie )

    Kontext
    active.SetPage TabLinienstile

    kontext "TabLinienstile"
    Call DialogTest ( TabLinienstile )
    try
        Aendern.Click
        Kontext "NameDlg"
        Call DialogTest ( NameDlg )
        NameDlg.Cancel
    catch
        Warnlog "Modify is disabled!"
        if Linienstil.GetItemCount = 0 then
            Warnlog "List of 'Line styles is empty! (Bug#99882)"
        end if
    endcatch

    Kontext
    active.SetPage TabLinienenden

    Kontext "Linienstil"
    if Linienstil.Exists then
        Warnlog "Messagebox appears changing to Tabpage 'Arrow Styles!"
        Linienstil.Cancel
    else
        kontext "TabLinienenden"

        Call DialogTest ( TabLinienenden )
        Aendern.Click
        Kontext
        Active.OK
        kontext "NameDlg"
        Call DialogTest ( NameDlg )
        NameDlg.Cancel
    end if

    kontext "TabLinienenden"
    printlog " Close dialog"
    TabLinienenden.cancel
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatArea
    Dim i as integer
    PrintLog "- Format / Area in drawing object"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a rectangle"
    Call wRectangleToClipboard

    hUseAsyncSlot( "EditPaste" )

    printlog " Format / Area"
    if ( hUseAsyncSlot( "FormatArea" ) = -1 ) then
        Warnlog "Clipboard problem between applications!"
        Call wZeichenobjektEinfuegen ( "Rechteck", 40, 40, 50, 50 )
        Call gMouseClick ( 45, 45 )
        FormatArea
    end if

    Kontext
    wait 500
    printlog " Step through all tabpages"
    active.SetPage TabArea
    Kontext "TabArea"
    For i = 1 to 5
        FillOptions.Select i
        Call DialogTest ( TabArea, i )
    next i
    Kontext
    active.SetPage TabSchatten
    kontext "TabSchatten"
    Anzeigen.Check
    Call DialogTest ( TabSchatten )

    Kontext
    active.SetPage TabTransparenz
    kontext "TabTransparenz"
    Call DialogTest ( TabTransparenz )

    Kontext
    active.SetPage TabFarben
    kontext "TabFarben"
    Sleep 1
    Farbmodell.Select 2
    Call DialogTest ( TabFarben )

    Bearbeiten.Click

    Kontext "FarbeDlg"
    if ( FarbeDlg.exists( 1 ) ) then
        Call DialogTest ( FarbeDlg )
        FarbeDlg.cancel
        WaitSlot()
    else
        warnlog( "Dialog <FarbeDlg> is not available" )
    end if

    Kontext
    active.SetPage TabFarbverlaeufe
    kontext "TabFarbverlaeufe"
    Call DialogTest ( TabFarbverlaeufe )
    Aendern.Click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    NameDlg.Cancel

    Kontext
    active.SetPage TabSchraffuren
    kontext "TabSchraffuren"
    Call DialogTest ( TabSchraffuren)
    try
        Aendern.Click
    catch
        Warnlog "Button 'Modify' does not work!"
        if Tabelle.GetItemcount = 0 then
            Warnlog "Table of Hatching's is empty! (Bug#99882)"
        end if
    endcatch

    Kontext "NameDlg"
    if ( NameDlg.exists( 1 ) ) then
        Call DialogTest ( NameDlg )
        NameDlg.Cancel
    else
        warnlog( "Dialog <NameDlg> is not available" )
    end if

    Kontext
    active.SetPage TabBitmap
    kontext "TabBitmap"
    Call DialogTest ( TabBitmap )

    try
        Aendern.Click
    catch
        WarnLog "#36902#Modify is disabled. Wrong default-list loaded"
    endcatch

    Kontext "NameDlg"
    if ( NameDlg.exists( 1 ) ) then
        Call DialogTest ( NameDlg )
        NameDlg.Cancel
    else
        warnlog( "Dialog <NameDlg> is not available" )
    end if

    kontext "TabBitmap"
    Import.Click
    Kontext "GrafikEinfuegenDlg"
    if ( GrafikEinfuegenDlg.exists( 1 ) ) then
        Call DialogTest ( GrafikEinfuegenDlg )
        GrafikEinfuegenDlg.Cancel
    else
        warnlog( "Dialog <GrafikEinfuegenDlg> is not available" )
    end if

    printlog " Close Tabdialog"
    kontext "TabBitmap"
    TabBitmap.Cancel
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatText
    PrintLog "- Format / Text in drawing object"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument
    printlog " Paste a rectangle from draw via clipboard"
    Call wRectangleToClipboard
    EditPaste
    Sleep (1)

    try
        printlog " Format / Text"
        FormatTextWriter
    catch
        Warnlog "Clipboard problem between applications!"
        Call wZeichenobjektEinfuegen ( "Rechteck", 40, 40, 50, 50 )
        Call gMouseClick ( 45, 45 )
        FormatTextWriter
    endcatch

    Kontext
    printlog " Step through all tabpages"
    wait 500
    Active.SetPage TabText
    Kontext "TabText"
    DialogTest ( TabText )

    Kontext
    Active.SetPage TabLauftext
    Kontext "TabLauftext"
    DialogTest ( TabLauftext )

    printlog " Close dialog"
    TabLauftext.Cancel
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatPositionAndSize
    PrintLog "- Format / Position and Size in drawing object"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument
    printlog " Paste a rectangle from draw via clipboard"
    Call wRectangleToClipboard
    hUseAsyncSlot( "EditPaste" )

    printlog " Format Position and Size"
    if ( hUseAsyncSlot( "ContextPositionAndSize" ) = -1 ) then
        Warnlog "Clipboard problem between applications!"
        Call wZeichenobjektEinfuegen ( "Rechteck", 40, 40, 50, 50 )
        Call gMouseClick ( 45, 45 )
        if ( hUseAsyncSlot( "ContextPositionAndSize" ) = -1 ) then
            warnlog( "Failed to execute slot <ContextPositionAndSize>" )
        end if
    end if

    printlog " Step through all tabpages"
    Kontext
    active.setpage TabPositionAndSizeWriter
    Kontext "TabPositionAndSizeWriter"
    call Dialogtest ( TabPositionAndSizeWriter )

    Kontext
    active.setPage TabDrehung
    Kontext "TabDrehung"
    call Dialogtest ( TabDrehung )

    Kontext
    active.setpage TabSchraegstellen
    Kontext "TabSchraegstellen"
    call Dialogtest ( TabSchraegstellen )
    printlog " Close dialog"
    TabSchraegstellen.cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatFlip

    PrintLog "- Format / Flip drawing objects"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument

    printlog " Paste a rectangle from draw via clipboard"
    Call wRectangleToClipboard
    hUseAsyncSlot( "EditPaste" )

    printlog " Format / Flip vertically"
    if ( hUseAsyncSlot( "FormatFlipVertically" ) = -1 ) then
        Warnlog "Clipboard problem between applications!"
        Call wZeichenobjektEinfuegen ( "Rechteck", 40, 40, 50, 50 )
        Call gMouseClick ( 45, 45 )
        if ( hUseAsyncSlot( "FormatFlipVertically" ) = -1 ) then
            warnlog( "Failed to execute slot <FormatFlipVertically>" )
        end if
    end if

    WaitSlot (3000)

    printlog " Format / Flip horizontally"
    hUseAsyncSlot( "FormatFlipHorizontally" )
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatArrangeArea

    PrintLog "- Format / Arrange drawing objects"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument

    printlog " Paste a rectangle from draw via clipboard"
    Call wRectangleToClipboard
    hUseAsyncSlot( "EditPaste" )

    printlog " Format / Arrange / Bring to front"
    if ( hUseAsyncSlot( "FormatArrangeBringToFrontCalc" ) = -1 ) then

        Warnlog "Clipboard problem between applications!"
        Call wZeichenobjektEinfuegen ( "Rechteck", 40, 40, 50, 50 )
        Call gMouseClick ( 45, 45 )
        if ( hUseAsyncSlot( "FormatArrangeBringToFrontCalc" ) = -1 ) then
            warnlog( "Failed to execute slot <FormatArrangeBringToFrontCalc>" )
        end if
    end if

    WaitSlot()
    printlog " Format / Arrange / Bring Forward"
    FormatArrangeBringForward
    WaitSlot()
    printlog " Format / Arrange / Set backward"
    FormatArrangeSetBackward
    WaitSlot()
    printlog " Format / Arrange / Send to back"
    FormatArrangeSendToBack
    WaitSlot()

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatAlignmentArea

    PrintLog "- Format / Alignment drawing objects"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if
    printlog " Open new document"
    Call hNewDocument

    printlog " Paste a rectangle from draw via clipboard"
    Call wZeichenobjektEinfuegen ( "Rechteck", 45, 30, 55, 45 )
    sleep (1)
    Call wObjektSelektieren ( 43, 25, 60, 50 )
    Sleep 1
    try
        printlog " Format / Anchor / As Character"
        FormatAnchorAsCharacter
        WaitSlot()
    catch
        Warnlog "Anchor could not be set to character! No test!"
        Call hCloseDocument
        printlog " Close active document"
        goto endsub
    endcatch

    try
        printlog " Format / Object / Alignment left"
        FormatObjectAlignmentLeft
        WaitSlot()
    catch
        Warnlog "Format / Object / Alignment / Left could not be executed!"
    endcatch

    try
        printlog " Format / Object / Alignment centered"
        FormatObjectAlignmentCentered
        WaitSlot()
    catch
        Warnlog "Format / Object / Alignment / Centered could not be executed!"
    endcatch

    try
        printlog " Format / Object / Alignment right"
        FormatObjectAlignmentRight
        WaitSlot()
    catch
        Warnlog "Format / Object / Alignment / Right could not be executed!"
    endcatch

    try
        printlog " Format / Object / Alignment Top"
        FormatObjectAlignmentTop
        WaitSlot()
    catch
        Warnlog "Format / Object / Alignment / Top could not be executed!"
    endcatch

    try
        printlog " Format / Object / Alignment Center"
        FormatObjectAlignmentCenter
        WaitSlot()
    catch
        Warnlog "Format / Object / Alignment / Center could not be executed!"
    endcatch

    try
        printlog " Format / Object / Alignment bottom"
        FormatObjectAlignmentBottom
        WaitSlot()
    catch
        Warnlog "Format / Object / Alignment / Bottom could not be executed!"
    endcatch

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatGroup
    Dim x%
    Dim y%
    Dim x1%
    Dim y1%

    PrintLog "- Format / Group drawing objects"

    if ( gApplication = "HTML" ) then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert 2 rectangles and select both"
    if gPlatGroup <> "unx" then
        Call wZeichenobjektEinfuegen ( "Rechteck", 25, 25, 30, 30 )
        Call wTypeKeys ( "<Escape>" )
        Call wZeichenobjektEinfuegen ( "Rechteck", 35, 35, 45, 45 )
        Call wTypeKeys ( "<Escape>" )
        Call wObjektSelektieren ( 23, 23, 50, 50 )
        x% = 25
        y% = 25
        x1% = 35
        y1% = 45
    else
        Call wZeichenobjektEinfuegen ( "Rechteck", 50, 60, 30, 30 )
        Call wTypeKeys ( "<Escape>" )
        Call wZeichenobjektEinfuegen ( "Rechteck", 65, 80, 45, 45 )
        Call wTypeKeys ( "<Escape>" )
        Call wObjektSelektieren ( 23, 23, 70, 90 )
        x% = 55
        y% = 55
        x1% = 55
        y1% = 65
    end if

    printlog " Format / Group / Group"
    if ( hUseAsyncSlot( "FormatGroupGroup" ) = -1 ) then
        QAErrorlog "Unable to execute Format / Group / Group. Maybe objects not selected!"
        Call hCloseDocument
        goto endsub
    end if

    printlog " Format / Group / Edit Group"
    if ( hUseAsyncSlot( "FormatGroupEditGroupCalc" ) = -1 ) then
        Warnlog "Unable to execute Format / Group / Group. Maybe objects not selected!"
        Call hCloseDocument
        goto endsub
    end if

    Call wTypeKeys ( "<Shift F4>" )

    printlog " Format / Exit Group"
    if ( hUseAsyncSlot( "FormatExitGroupDraw" ) = -1 ) then
        Warnlog "Unable to execute Format / Group / Exit Group. Maybe objects not selected!"
        Call hCloseDocument
        goto endsub
    end if

    Call wTypeKeys ( "<Shift F4>" )
    
    printlog " Format / Ungroup"
    if ( hUseAsyncSlot( "FormatUngroupDraw" ) = -1 ) then
        Warnlog "Unable to execute Format / Group / Ungroup. Maybe objects not selected!"
        Call hCloseDocument
        goto endsub
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatFontwork

    PrintLog "- Format / Fontwork drawing objects"

    if ( gApplication = "HTML" ) then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument
    printlog " Paste a rectangle from draw via clipboard"
    Call wRectangleToClipboard

    hUseAsyncSlot( "EditPaste" )

    printlog " Format / Font Work"
    if ( hUseAsyncSlot( "FormatFontWork" ) = -1 ) then
        Warnlog "Clipboard problem between applications!"
        Call wZeichenobjektEinfuegen ( "Rechteck", 40, 40, 50, 50 )
        Call gMouseClick ( 45, 45 )
        FormatFontwork
    end if

    Kontext "Fontwork"
    if ( Fontwork.Exists( 1 ) ) then
        call Dialogtest ( Fontwork )
        printlog " Close Fontwork dialog"
        FormatFontwork
    else
        Warnlog "Dialog 'Fontwork' does not exist!"
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatPositionAndSizeLegend

    PrintLog "- Format / Position and Size in Legend"

    if ( gApplication = "HTML" ) then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument
    Kontext "NavigatorWriter"
    if NavigatorWriter.Exists then ViewNavigator

	printlog " Insert a legend"
    Call wZeichenobjektEinfuegen ( "Legende", 45, 35, 50, 50 )
	Call wTypeKeys "<ESCAPE>"
    Call wTypeKeys "<SHIFT F4>"

    Call hUseAsyncSlot( "FormatPositionAndSize" )
    kontext "active"
    if ( active.exists( 1 ) ) then
        printlog( "Dialog open. Good" )
    else
        Warnlog "Format / Position and Size does not work! Maybe object not selected!"
        printlog " Close active document"
        Call hCloseDocument
        goto endsub
    end if

    printlog " In upcoming dialog step through all tabpages"
    Kontext
    active.setpage TabPositionAndSizeWriter

    Kontext "TabPositionAndSizeWriter"
    call Dialogtest ( TabPositionAndSizeWriter )

    Kontext
    active.setpage TabLegende

    Kontext "TabLegende"
    call Dialogtest ( TabLegende )

    printlog " Close dialog"
    TabLegende.Cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatStyleEdit

    PrintLog "- Format / Style Text objects"

    if ( gApplication = "HTML" ) then
        printlog "Not in WriterWeb !"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument

    Kontext "DrawBar"
    if Not DrawBar.Exists then 
        Call hToolbarSelect("Drawing", true)
    end if

    if Drawbar.Exists then
        Textobjekt.Click
        printlog " Insert a textobject from toolbar"
        Sleep 2

        Call gMouseMove(50, 20, 70, 40)
        hUseAsyncSlot( "FormatStyleBold" )

        printlog " Format / Style / Bold"
        Call wTypeKeys ("a")
        printlog " Format / Style / Italic"
        FormatStyleItalic
        WaitSlot()
        printlog " Insert a letter in text object"
        Call wTypeKeys ("a")
        printlog " Format / Style / Underline"
        FormatStyleUnderline
        WaitSlot()

        printlog " Insert a letter in text object"
        Call wTypeKeys ("a")
        printlog " Format /Style / Strike through"
        UseBindings
        FormatStyleStrikeThrough
        WaitSlot()
        printlog " Insert a letter in text object"
        Call wTypeKeys ("b")
        printlog " Format / Style / Shadow"
        UseBindings
        FormatStyleShadow
        WaitSlot()
        printlog " Insert a letter in text object"
        Call wTypeKeys ("b")
        printlog " Format / Style / Outline"
        UseBindings
        FormatStyleOutline
        WaitSlot()
        printlog " Insert a letter in text object"
        Call wTypeKeys ("b")
        printlog " Format / Style / Superscript"
        FormatStyleSuperscript
        WaitSlot()
        printlog " Insert a letter in text object"
        Call wTypeKeys ("a")
        printlog " Format / Style / Subscript "
        FormatStyleSubscript
        WaitSlot()
        printlog " Insert a letter in text object"
        Call wTypeKeys ("a")
        Kontext "Drawbar"
        Drawbar.Close
    else
        Warnlog "#i33132#Toolbar 'Drawbar' is not opened!"
    end if
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatAlignmentEdit
    PrintLog "- Format / Alignment text objects"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if
    Call hNewDocument
    printlog " Open new document"
    Kontext "DrawBar"
    if Not DrawBar.Exists then Call hToolbarSelect("Drawing", true)

    Textobjekt.Click
    printlog " Insert a textbox via 'Draw functions' toolbar"

    Call gMouseMove(50, 20, 70, 40)
    printlog " Insert some text in document"

    Call wTypeKeys ("Ein Wort")
    printlog " Format / Alignment / Left"
    hUseAsyncSlot( "FormatAlignmentLeftText" )
    WaitSlot()
    printlog " Format / Alignment / Right"
    FormatAlignmentRightText
    WaitSlot()
    printlog " Format / Alignment / Centered"
    FormatAlignmentCenteredText
    WaitSlot()
    printlog " Format  Alignment / Justify"
    FormatAlignmentJustifiedText
    WaitSlot()

    Kontext "Drawbar"
    Drawbar.Close
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatLineSpacing
    PrintLog "- Format / Line Spacing text objects"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if
    Call hNewDocument
    printlog " Open new document"
    Kontext "DrawBar"
    if Not DrawBar.Exists then Call hToolbarSelect("Drawing", true)

    Textobjekt.Click
    printlog " In a textbox via 'Draw Functions' toolbox"

    Call gMouseMove(50, 20, 70, 40)
    Sleep 1

    printlog " Format / Spacing / Single line"
    Call wTypeKeys ("Hallo<Return>Andere Zeile")

    printlog " Format / Line Spacing / Single line"
    FormatLineSpacingSingleLine
    WaitSlot()

    printlog " Format / Line Spacing  / 1.5 line"
    FormatLineSpacing15Lines
    WaitSlot()

    printlog " Format / Line spacing / Double"
    FormatLineSpacingDouble
    WaitSlot()

    Kontext "Drawbar"
    Drawbar.Close
    printlog " Close active document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tFormatParagraphEdit
    PrintLog "- Format / Paragraph text objects"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if
    Call hNewDocument
    printlog " Open new document"
    Kontext "DrawBar"
    if Not DrawBar.Exists then Call hToolbarSelect("Drawing", true)

    Textobjekt.Click
    printlog " In a textbox via 'Draw Functions' toolbox"

    Call gMouseMove(50, 20, 70, 40)

    printlog " Format / Paragraph "
    hUseAsyncSlot( "FormatParagraph" )
    printlog " Step through all tabpages"
    Kontext
    Active.SetPage TabEinzuegeUndAbstaende
    Kontext "TabEinzuegeUndAbstaende"
    Call DialogTest ( TabEinzuegeUndAbstaende )

    Kontext
    Active.SetPage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"
    Call DialogTest ( TabAusrichtungAbsatz )

    Kontext
    Active.SetPage TabTabulator
    Kontext "TabTabulator"
    Call DialogTest ( TabTabulator )

    TabTabulator.Cancel
    Sleep 1
    printlog " Close dialog"
    Kontext "Drawbar"
    Drawbar.Close

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tHtmlDocFormatGraphics
    PrintLog "- Format / Graphics"
    Dim j as integer

    gApplication = "HTML"
    printlog " Open new document"
    Call hNewDocument

    printlog " Insert a graphic (..\writer\required\input\graphics\jolink.jpg)"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )

    if ( hUseAsyncSlot( "FormatGraphics" ) = -1 ) then
        Warnlog "- Format / Graphics disabled or hidden!"
        goto endsub
    end if

    printlog " Step through all tabpages"
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

testcase tHtmlDocFormatAlignment

	gApplication = "HTML"
    PrintLog "- Format / Alignment"

    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a graphic from file '..\writer\required\input\graphics\jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    wait 500
    printlog " Format / Anchor / to Page"
    FormatAnchorToPage

    try
        printlog " Format / Alignment / Left"
        FormatAlignmentLeft
        WaitSlot()
    catch
        Warnlog "'Alignment left' couldn't be executed!"
    endcatch

    try
        printlog " Format / Alignment / Right"
        FormatAlignmentRight
        WaitSlot()
    catch
        Warnlog "'Alignment right' couldn't be executed!"
    endcatch

    try
        printlog " Format / Alignment / Top"
        FormatAlignmentTop
        WaitSlot()
    catch
        Warnlog "'Alignment top' couldn't be executed!"
    endcatch

    try
        printlog " Format / Alignment / Center"
        FormatAlignmentCenter
        WaitSlot()
    catch
        Warnlog "'Alignment center' couldn't be executed!"
    endcatch

    try
        printlog " Format / Alignment / Bottom"
        FormatAlignmentBottom
        WaitSlot()
    catch
        Warnlog "'Alignment bottom' couldn't be executed!"
    endcatch

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tHtmlDocFormatObject
    PrintLog "- Format / Object"
    Dim j as integer

    gApplication = "HTML"
    printlog " Open new document"
    Call hNewDocument

    printlog " Insert a frame"
    Call wInsertFrame

    printlog " Format / Frame"
    FormatFrame

    printlog " Step through all tabpages"
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
    active.SetPage TabUmrandung
    Kontext "TabUmrandung"
    Call DialogTest ( TabUmrandung )

    Kontext
    active.SetPage TabHintergrund
    Kontext "TabHintergrund"
    Call DialogTest ( TabHintergrund )

    printlog " Close dialog"
    TabHintergrund.Cancel

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------
