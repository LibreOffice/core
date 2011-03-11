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
'* short description :  Testcases for the Format-Menu
'*
'\******************************************************************

sub w_005_

    printLog Chr(13) + "--------- Menu Format (w_005_.inc) ----------"
    gApplication = "WRITER"

    Call tFormatDefault

    Call tFormatCharacter
    Call tFormatParagraph
    Call tFormatNumberingBullets
    Call tFormatPage

    Call tFormatChangeCase
    Call tFormatRuby
    Call tFormatColumn
    Call tFormatSections

    Call tFormatStylist
    Call tFormatAutoformatWhileTyping

    Call tFormatAnchor
    Call tFormatAnchorArea
    Call tFormatWrap
    Call tFormatWrapArea

    'Writer Web
    gApplication = "HTML"
    Call tHtmlDocFormatParagraph
    Call tHtmlDocFormatPage
    Call tHtmlDocFormatStylist
    gApplication = "WRITER"

end sub

'------------------------------------------------------

testcase tFormatDefault
    PrintLog "- Format / Default"

    printlog " Open new document"
    Call hNewDocument
    printlog " Insert dummy text"
    Call wBlindtextEinfuegen
    printlog " Edit / Select All"
    EditSelectAll
    printlog " Format / Default"
    FormatDefaultWriter
    printlog " Close active document"
    Call hCloseDocument
endcase

'------------------------------------------------------

testcase tFormatSections
    printlog "- Format / Sections"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert / Section"
    InsertSection
    printlog " Step through all tabpages"
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Bereichsliste.SetText "Mein Bereich"
    TabBereiche.OK
    
    hUseAsyncSlot( "FormatSections" )
    Kontext "BereicheBearbeiten"
    Call Dialogtest(BereicheBearbeiten)
    Optionen.Click
    sleep(2)
    Kontext
    Active.SetPage TabSpalten
    Kontext "TabSpalten"
    Call Dialogtest(TabSpalten)
    Kontext
    Active.SetPage TabHintergrund
    Kontext "TabHintergrund"
    Call DialogTest(TabHintergrund, 1)
    Als.Select(2)
    Call DialogTest (TabHintergrund,2)
    Durchsuchen.Click
    sleep(2)
    Kontext
    If Active.Exists then
        if Active.GetRT = 304 then
            Warnlog Active.GetText
            Active.Ok
        end if
    end If
    
    Kontext "GrafikEinfuegenDlg"
    if ( GrafikEinfuegenDlg.exists( 1 ) ) then
        GrafikEinfuegenDlg.Cancel
        WaitSlot ' NOOP
    else
        warnlog( "Dialog <GrafikEinfuegenDlg> is not available" )
    endif
    
    Kontext "TabHintergrund"
    TabHintergrund.Cancel
    
    Kontext "BereicheBearbeiten"
    if ( BereicheBearbeiten.exists( 1 ) ) then
        BereicheBearbeiten.Cancel
    else
        warnlog( "Dialog <BereicheBearbeiten> is not available" )
    endif
    printlog " Close active document"
    Call hCloseDocument
endcase

'------------------------------------------------------

testcase tFormatCharacter
    PrintLog "- Format / Character"
    Dim j as integer

    printlog " Open new document"
    Call hNewDocument

    printlog " Format / Character"
    FormatCharacter

    printlog " Step through all tabpages"
    Kontext
    active.SetPage TabFont
    kontext "TabFont"
    Call DialogTest ( TabFont )

    Kontext
    active.SetPage TabFontEffects
    kontext "TabFontEffects"
    Call DialogTest ( TabFontEffects )

    Kontext
    Active.SetPage TabFontPosition
    Kontext "TabFontPosition"
    Call DialogTest ( TabFontPosition )

    if gAsianSup = True then
        try
            Kontext
            Active.SetPage TabAsianLayout
            Kontext "TabAsianLayout"
            Call DialogTest ( TabAsianLayout )
        catch
            Warnlog "Tabpage 'Asian Layout' is not visible! Activated in Options ?"
        endcatch
    end if

    Kontext
    active.SetPage TabHyperlinkZeichen
    Kontext "TabHyperlinkZeichen"
    Call DialogTest ( TabHyperlinkZeichen )

    Durchsuchen.Click
    Kontext "OeffnenDlg"
    if OeffnenDlg.Exists( 1 ) then
        OeffnenDlg.Cancel
    else
        Warnlog "File-Open dialog is not up!"
    end if

    Kontext "TabHyperlinkZeichen"
    Ereignisse.Click
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
    Kontext "MakroZuweisen"
    Call DialogTest ( MakroZuweisen )
    MakroZuweisen.Cancel
    Kontext
    active.SetPage TabHintergrund
    kontext "TabHintergrund"
    Call DialogTest ( TabHintergrund )
    printlog " Close dialog"
    TabHintergrund.Close

    printlog " Close active document"
    Call hCloseDocument
endcase

'------------------------------------------------

testcase tFormatChangeCase
    printlog "- Format / Change Case"
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert some text in document and select text"
    Call wTypeKeys ("call him now ! please")
    Call wTypeKeys ("<Shift Home>")

    printlog " Format / Case Characters / Upper"
    hUseAsyncSlot( "FormatChangeCaseUpper" )
    EditCopy
    if GetClipboardtext() <> "CALL HIM NOW ! PLEASE" then
        Warnlog "Sentence not 'CALL HIM NOW ! PLEASE' but " & GetClipboardtext()
    else
        printlog "  " & GetClipboardtext()
    endif

    printlog " Format / Case Characters / Lower"
    hUseAsyncSlot( "FormatChangeCaseLower" )
    EditCopy
    if GetClipboardtext() <> "call him now ! please" then
        Warnlog "Sentence not 'call him now ! please' but " & GetClipboardtext()
    else
        printlog "  " & GetClipboardtext()
    endif

    printlog " Format / Case Characters / Sentence Case"
    FormatChangeCaseToSentence
    EditCopy
    if GetClipboardtext() <> "Call him now ! Please" then
        Warnlog "Sentence not 'Call him now ! Please' but " & GetClipboardtext()
    else
        printlog "  " & GetClipboardtext()
    endif

    printlog " Format / Case Characters / Capitalize every word"
    FormatChangeCaseCapitalizeEveryWord
    EditCopy
    if GetClipboardtext() <> "Call Him Now ! Please" then
        Warnlog "Sentence not 'Call Him Now ! Please' but " & GetClipboardtext()
    else
        printlog "  " & GetClipboardtext()
    endif


    printlog " Format / Case Characters / tOGGLE cASE"
    FormatChangeCaseToggleCase
    EditCopy
    if GetClipboardtext() <> "cALL hIM nOW ! pLEASE" then
        Warnlog "Sentence not 'cALL hIM nOW ! pLEASE' but " & GetClipboardtext()
    else
        printlog "  " & GetClipboardtext()
    endif


    if ( gAsianSup ) then
        try
            FormatChangeCaseHalfWidth
            printlog " Format / Case Characters / Half Width"
        catch
            Warnlog "Format / Change Case / Half Width does not work. Maybe no Asian support!"
        endcatch
        WaitSlot()
        
        try
            printlog " Format / Case Characters / Full Width"
            FormatChangeCaseFullWidth
        catch
            Warnlog "Format / Change Case / Full Width does not work. Maybe no Asian support!"
        endcatch
        WaitSlot()
        
        try
            printlog " Format / Case Characters / Hiragana"
            FormatChangeCaseHiragana
        catch
            Warnlog "Format / Change Case / Hiragana does not work. Maybe no Asian support!"
        endcatch
        WaitSlot()
        
        try
            printlog " Format / Case Characters / Katagana"
            FormatChangeCaseKatagana
        catch
            Warnlog "Format / Change Case / Katagana does not work. Maybe no Asian support!"
        endcatch
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'------------------------------------------------

testcase tFormatRuby
    printlog "- Format / Ruby"
    if gAsianSup = True then
        if gApplication = "HTML" then
            printlog "Not in WriterWeb!"
            goto endsub
        end if
        Call hNewDocument
        printlog " Open new document"
        Call wTypeKeys ("Hello World!")
        Call wTypeKeys ("<Shift Home>")
        printlog " Insert some text in document"

        try
            printlog " Format / Ruby"
            FormatRuby
        catch
            Warnlog "Format / Ruby does not work. Maybe No Asian support!"
            Call hCloseDocument
            goto endsub
        endcatch
        Kontext "RubyDialog"
        Call DialogTest ( RubyDialog )

        printlog " Open Stylist"
        CallStylist.Click

        Kontext "Stylist"
        if Stylist.Exists( 1 ) then
            Stylist.close
        else
            Warnlog "Stylist has not been opened!"
        end if

        printlog " Close Stylist and Ruby dialog"
        Kontext "RubyDialog"
        DialogClose.Click

        printlog " Close active document"
        Call hCloseDocument
    end if
endcase

'------------------------------------------------

testcase tFormatParagraph
    PrintLog "- Format / Paragraph"

    printlog " Open new document"
    Call hNewDocument
    printlog " Format / Paragraph"
    FormatParagraph

    Kontext
    active.SetPage TabEinzuegeUndAbstaende
    kontext "TabEinzuegeUndAbstaende"
    Call DialogTest ( TabEinzuegeUndAbstaende )
    printlog " Step through all tabpages"

    Kontext
    Active.SetPage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"
    Call DialogTest ( TabAusrichtungAbsatz )

    Kontext
    active.SetPage TabNumerierungAbsatz
    kontext "TabNumerierungAbsatz"
    Call DialogTest ( TabNumerierungAbsatz )

    Kontext
    active.SetPage TabUmrandung
    kontext "TabUmrandung"
    Call DialogTest ( TabUmrandung )

    if gApplication <> "HTML" then
        Kontext
        active.SetPage TabTextfluss
        kontext "TabTextfluss"
        Call DialogTest ( TabTextfluss )

        if ( gAsianSup ) then
            try
                Kontext
                Active.SetPage TabAsianTypography
                Kontext "TabAsianTypography"
                Call DialogTest ( TabAsianTypography )
            catch
                Warnlog "Tabpage 'Asian Typography' is not visible! Activated in Options ?"
            endcatch
        end if

        Kontext
        active.SetPage TabTabulator
        kontext "TabTabulator"
        Call DialogTest ( TabTabulator )

        Kontext
        active.SetPage TabInitialen
        kontext "TabInitialen"
        Anzeigen.Check
        Call DialogTest ( TabInitialen )

        Kontext
        active.SetPage TabHintergrund
        Kontext "TabHintergrund"
        Call DialogTest ( TabHintergrund, 1 )
        Als.Select 2
        Kontext "TabHintergrund"
        Call DialogTest ( TabHintergrund, 2 )
        Durchsuchen.Click
        Kontext "GrafikEinfuegenDlg"
        GrafikEinfuegenDlg.Cancel

        printlog " Close dialog"
        kontext "TabHintergrund"
        TabHintergrund.Close
    else
        printlog " Close dialog"
        TabUmrandung.Close
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'------------------------------------------------

testcase tFormatPage
    PrintLog "- Format / Page"

    printlog " Open new document"
    Call hNewDocument

    printlog " Format / Page"
    FormatPageWriter

    Kontext
    printlog " Step through all tabpages"
    active.SetPage TabVerwalten
    kontext "TabVerwalten"
    Call DialogTest ( TabVerwalten )

    Kontext
    active.SetPage TabSeite
    kontext "TabSeite"
    Call DialogTest ( TabSeite )

    Kontext
    active.SetPage TabHintergrund
    Kontext "Active"
    If Active.GetRT = 304 then Active.Yes         ' Messagebox abfangen-> Seitenr�nder au�erhalb des Bereichs

    Kontext
    active.SetPage TabHintergrund
    kontext "TabHintergrund"
    Call DialogTest ( TabHintergrund, 1 )
    Als.Select 2
    Call DialogTest ( TabHintergrund, 2 )
    Durchsuchen.Click
    
    Kontext "GrafikEinfuegenDlg"
    if ( GrafikEinfuegenDlg.exists( 1 ) ) then
        GrafikEinfuegenDlg.Cancel
    else
        warnlog( "Dialog <GrafikEinfuegenDlg> is not available" )
    endif

    Kontext
    active.SetPage TabKopfzeile
    kontext "TabKopfzeile"
    Call DialogTest ( TabKopfzeile )
    if gApplication <> "HTML" then
        KopfzeileMitAbstand.Check
        Zusaetze.Click

        Kontext
        active.SetPage TabUmrandung
        kontext "TabUmrandung"
        Call DialogTest ( TabUmrandung )
        Kontext
        active.SetPage TabHintergrund
        kontext "TabHintergrund"
        Call DialogTest ( TabHintergrund )
        Als.Select 2
        Call DialogTest ( TabHintergrund )
        TabHintergrund.Close
    end if
    Kontext
    active.SetPage TabFusszeile
    kontext "TabFusszeile"
    Call DialogTest ( TabFusszeile )
    if gApplication <> "HTML" then
        FusszeileMitAbstand.Check
        Zusaetze.Click

        Kontext
        active.SetPage TabUmrandung
        kontext "TabUmrandung"
        Call DialogTest ( TabUmrandung )
        Kontext
        active.SetPage TabHintergrund
        kontext "TabHintergrund"
        Call DialogTest ( TabHintergrund, 1 )
        Als.Select 2
        Call DialogTest ( TabHintergrund, 2 )
        TabHintergrund.Close

        Kontext
        active.SetPage TabUmrandung
        kontext "TabUmrandung"
        Call DialogTest ( TabUmrandung )

        Kontext
        active.SetPage TabSpalten
        kontext "TabSpalten"
        Call DialogTest ( TabSpalten )

        if gAsianSup = True then
            Kontext
            active.Setpage TabGrid
            Kontext "TabGrid"
            Call DialogTest ( TabGrid )
        end if

        Kontext
        active.SetPage TabFussnote
        kontext "TabFussnote"
        Call DialogTest ( TabFussnote )
        printlog " Close dialog"
        TabFussnote.Close
    else
        printlog " Close dialog"
        TabFussZeile.Close
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'--------------------------------------------------

testcase tFormatNumberingBullets
    dim referencecounter as integer
    PrintLog "- Format / Numbering / Bullets"

    printlog " Open new document"
    Call hNewDocument

    printlog " Format / Numbering/Bullets"
    FormatNumberingBullets
    WaitSlot (5000)

    printlog " Step through all tabpages"
    Kontext
    Active.SetPage TabBullet
    Kontext "TabBullet"
    Call DialogTest ( TabBullet )

    Kontext
    Active.SetPage TabNumerierungsart
    Kontext "TabNumerierungsart"
    Call DialogTest ( TabNumerierungsart )

    Kontext
    Active.SetPage TabNumerierung
    Kontext "TabNumerierung"
    Call DialogTest ( TabNumerierung )

    Kontext
    Active.SetPage TabGrafiken
    Kontext "TabGrafiken"
    Call DialogTest ( TabGrafiken )

    Kontext
    Active.SetPage TabPositionNumerierung
    Kontext "TabPositionNumerierung"
    Call DialogTest ( TabPositionNumerierung )

    Kontext
    Active.SetPage TabOptionenNumerierung
    Kontext "TabOptionenNumerierung"
    Call DialogTest ( TabOptionenNumerierung )
    Numerierung.Select 9

    try
        Auswahl.Click
    catch
        QAErrorlog "Numbering option 'Linked graphics' is missing"
        TabOptionenNumerierung.Cancel
        Call hCloseDocument
        goto endsub
    endcatch
    Sleep 2
    Call hMenuSelectNr (1)
    Sleep 2
    Kontext "OeffnenDlg"
    OeffnenDlg.Cancel

    Kontext "TabOptionenNumerierung"

    Sleep 1

    try
        Auswahl.Click
        referencecounter = hMenuItemGetCount
        wait (350)
        do while (hMenuItemGetCount <> "2")
            referencecounter = hMenuItemGetCount
            wait (100)
        loop
        Call hMenuSelectNr(2)
        referencecounter = hMenuItemGetCount
        wait (100)
        do while (hMenuItemGetCount <> referencecounter)
            referencecounter = hMenuItemGetCount
            wait (150)
        loop
        Call hMenuClose
    catch
        Exceptlog
        Call hMenuClose
    endcatch

    printlog " Close dialog"
    TabOptionenNumerierung.Cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

'------------------------------------------------------

testcase tFormatStylist
    PrintLog "- Format / Stylist"

    if gPlatgroup = "unx" then
        QAErrorlog "Doesn't run on unix"
        goto endsub
    end if
    printlog " Open new document"
    Call hNewDocument
    Kontext "Gestalter"
    If Not Gestalter.Exists then FormatStylist
    printlog " Format / Stylist"
    Kontext "Gestalter"
    Call DialogTest ( Gestalter )
    Zeichenvorlagen.Click
    printlog " Click all Styles-buttons in Stylist"
    if gApplication <> "HTML" then
        WaitSlot()
        Rahmenvorlagen.Click
        WaitSlot()
        Seitenvorlagen.Click
    end if
    WaitSlot()
    Numerierungsvorlagen.Click
    WaitSlot()
    Absatzvorlagen.Click
    WaitSlot()

    TBGestalterRight.Click Giesskanne
    WaitSlot()

    AusSelektion.OpenMenu
    Sleep 1
    hMenuSelectNr(1)
    
    Kontext "VorlageErzeugen"
    if ( VorlageErzeugen.exists( 1 ) ) then
        DialogTest ( VorlageErzeugen )
        VorlageErzeugen.Cancel
    else
        warnlog( "Dialog <VorlageErzeugen> is not available" )
    endif

    Kontext "Gestalter"
    AusSelektion.OpenMenu
    Sleep 1
    hMenuSelectNr(2)

    AusSelektion.OpenMenu
    Sleep 1
    hMenuSelectNr(3)
    
    Kontext "Neu"
    if ( Neu.exists( 10 ) ) then
        DialogTest ( Neu )
        Neu.Cancel
    else
        warnlog( "Dialog <Neu> is not available" )
    endif

    printlog " Close Stylist"
    Kontext "Gestalter"
    Gestalter.Close

    printlog " Close active document"
    Call hCloseDocument
endcase

'------------------------------------------------------

testcase tFormatAutoformatWhileTyping
    PrintLog "- Format / Autoformat / "

    printlog " Open new document"
    Call hNewDocument

    printlog " Uncheck 'Format / Autoformat / While typing'"
    FormatAutoformatWhileTyping
    WaitSlot (2000)
    printlog " Check 'Format / Autoformat / While typing'"
    FormatAutoformatWhileTyping
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatColumn
    PrintLog "- Format / Column"

    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a table"
    Call hTabelleEinfuegen
    Wait 500
    printlog " Format / column / Width"
    FormatColumnWidthWriter
    WaitSlot (500)
    Kontext "SpaltenBreite"
    Breite.SetText "2,5"
    Call DialogTest ( SpaltenBreite )
    printlog " Close dialog"
    SpaltenBreite.OK
    Wait 500
    hSetDocumentContext()
    printlog " Enter some text in document"
    Call wTypeKeys ("Dies ist ein langer Text, HALLO HALLO")
    Wait 500
    printlog " Format / Column / Optimal Width"
    FormatColumnOptimalWidthWriter
    WaitSlot (500)
    printlog " Format / Row / Select"
    TableSelectEntireRow
    WaitSlot (500)
    printlog " Format / Column / Space equally"
    FormatColumnSpaceEqually
    WaitSlot (500)
    printlog " Format / Column / Select"
    TableSelectColumn
    WaitSlot (500)
    printlog " Format / Cloumn / Delete"
    TableDeleteColumns
    WaitSlot (500)
    hSetDocumentContext()
    printlog " Press 'up' twice to point cursor in the table"
    Call wTypeKeys ("<Up>",2)
    wait 500
    printlog " Format / Column / Insert "
    FormatColumnInsert
    WaitSlot (500)
    Kontext "EinfuegenSpalten"
    Anzahl.SetText "3"
    Call DialogTest ( EinfuegenSpalten )
    printlog " Close dialog"
    EinfuegenSpalten.OK
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatAnchor

    PrintLog "- Format / Anchor"
	
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a graphic from file '..\writer\required\input\graphics\jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )

    try
        FormatAnchorToPage
        printlog " Format / Anchor / to page"
    catch
        Warnlog "'Page' couldn't be executed!"
    endcatch

    Sleep 1
    try
        FormatAnchorToParagraph
    catch
        printlog "  Format / Anchor / to paragraph"
        Warnlog "'to paragraph' couldn't be executed!"
    endcatch

    WaitSlot (1000)
    try
        FormatAnchorToCharacter
        printlog " Format / Anchor / to character"
    catch
        Warnlog "'To Character' couldn't be executed!"
    endcatch
    WaitSlot (1000)

    try
        FormatAnchorAsCharacter
        printlog " Format / Anchor / As Character"
    catch
        Warnlog "'as Character' couldn't be executed!"
    endcatch
    WaitSlot (1000)

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatWrap
    PrintLog "- Format / Wrap"
    if gApplication = "HTML" then
        printlog "Not in Writer/Web!"
        goto endsub
    end if
    printlog " Open new document"
    Call hNewDocument
    printlog " Insert a graphic from file '..\writer\required\input\graphics\jolink.jpg'"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    try
        FormatWrapNoWrap
        printlog " Format / Wrap / No wrap"
    catch
        Warnlog "Error on 'No Wrap'"
    endcatch
    WaitSlot (1000)

    try
        FormatWrapPageWrap
        printlog " Format / Wrap / Page Wrap"
    catch
        Warnlog "Error on 'Page Wrap'"
    endcatch
    WaitSlot (1000)

    try
        FormatWrapOptimalPageWrap
        printlog " Format / Wrap / optimal page wrap"
    catch
        Warnlog "Error on 'Optimal Page Wrap'"
    endcatch
    WaitSlot (1000)

    try
        FormatWrapThrough
        printlog " Format / Wrap / Through"
    catch
        Warnlog "Error on 'Wrap Through'"
    endcatch
    WaitSlot (1000)

    try
        FormatWrapInBackground
        printlog " Format / Wrap / In Background "
    catch
        Warnlog "Error on 'In Background'"
    endcatch
    WaitSlot (1000)

    try
        FormatWrapFirstParagraph
        printlog " Format / Wrap / First paragraph"
    catch
        Warnlog "Error on 'First Paragraph'"
    endcatch
    WaitSlot (1000)

    try
        FormatWrapContour
        printlog " Format / Wrap / Contour"
        Warnlog "Error on 'Contour'"
    catch
        printlog "Contour Wrap disabled->OK"
    endcatch
    WaitSlot (1000)

    try
        FormatWrapEditContour
        printlog " Format / Wrap / Edit Contour"
        WaitSlot (5000)
        Kontext "KonturEditor"
        Dialogtest ( KonturEditor )
        Sleep 2
        KonturEditor.Close
        printlog " Close contour editor"
        Kontext
        if Active.Exists (2) then Active.Yes
    catch
        Warnlog "#85505#Error on 'Edit Contur'"
    endcatch

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatAnchorArea

    PrintLog "- Format / Anchor drawing objects"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if
    printlog "crashed because of i32539 (fixed in m54)"

    printlog " Open new document"
    Call hNewDocument
    printlog " Paste a rectangle from draw via clipboard"
    Call wRectangleToClipboard

    hUseAsyncSlot( "EditPaste" )

    try
        printlog " Format / Anchor / To page"
        FormatAnchorToPage
    catch
        Warnlog "Clipboard problem between applications!"
        Call wZeichenobjektEinfuegen ( "Rechteck", 40, 40, 50, 50 )
        Call gMouseClick ( 45, 45 )
        FormatAnchorToPage
    endcatch

    WaitSlot (1000)
    printlog " Format / Anchor / to paragraph"
    FormatAnchorToParagraph
    WaitSlot (1000)
    printlog " Format / Anchor / As character"
    FormatAnchorAsCharacter
    WaitSlot (1000)

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFormatWrapArea
    PrintLog "- Format / Wrap drawing objects"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if
    printlog "crashed because of i32539 (fixed in m54)"

    printlog " Open new document"
    Call hNewDocument
    printlog " Paste a rectangle from draw via clipboard"
    Call wRectangleToClipboard

    hUseAsyncSlot( "EditPaste" )

    try
        printlog " Format / Wrap / No wrap"
        FormatWrapNoWrap
    catch
        Warnlog "Clipboard problem between applications!"
        Call wZeichenobjektEinfuegen ( "Rechteck", 40, 40, 50, 50 )
        Call gMouseClick ( 45, 45 )
        FormatWrapNoWrap
    endcatch

    WaitSlot (1000)
    printlog " Format / Wrap / Contour"
    try
        FormatWrapContour  ' should be disabled with 'No wrap'
        Warnlog "Format / Wrap / Contour is disabled!"
    catch
    endcatch
    WaitSlot (1000)
    printlog " Format / Wrap / Page wrap"
    try
        FormatWrapPageWrap
    catch
        Warnlog "Format / Wrap / Page Wrap is disabled!"
    endcatch
    WaitSlot (1000)
    printlog " Format / Wrap / Contour"
    try
        FormatWrapContour
    catch
        Warnlog "Format / Wrap / Contour is disabled!"
    endcatch
    WaitSlot (1000)
    printlog " Format / Wrap / optimal page wrap"
    try
        FormatWrapOptimalPageWrap
    catch
        Warnlog "Format / Wrap / Optimal page wrap is disabled!"
    endcatch
    WaitSlot (1000)
    printlog " Format / Wrap / Through"
    try
        FormatWrapThrough
    catch
        Warnlog "Format / Wrap /Through is disabled!"
    endcatch
    WaitSlot (1000)
    printlog " Format / Wrap / In Background"
    try
        FormatWrapInBackground
    catch
        Warnlog "Format / Wrap / In Background is disabled!"
    endcatch
    WaitSlot (1000)
    printlog " Format / Wrap / First paragraph"
    try
        FormatWrapFirstParagraph
    catch
        Warnlog "Format / Wrap / First paragraph is disabled!"
    endcatch
    sleep (2)
    try
        printlog " Format / Wrap / Edit"
        FormatWrapEdit
    catch
        Warnlog "Drawing object in wrong format from the clipboard!"
    endcatch
    Kontext "TabUmlauf"
    Call DialogTest ( TabUmlauf )
    printlog " Close upcoming dialog"
    TabUmlauf.Cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tHtmlDocFormatParagraph
    PrintLog "- Format / Paragraph"
    gApplication = "HTML"
    printlog " Open new document"
    Call hNewDocument
    printlog " Format / Paragraph"
    FormatParagraph

    Kontext
    active.SetPage TabEinzuegeUndAbstaende
    kontext "TabEinzuegeUndAbstaende"
    Call DialogTest ( TabEinzuegeUndAbstaende )
    printlog " Step through all tabpages"

    Kontext
    Active.SetPage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"
    Call DialogTest ( TabAusrichtungAbsatz )

    Kontext
    active.SetPage TabNumerierungAbsatz
    kontext "TabNumerierungAbsatz"
    Call DialogTest ( TabNumerierungAbsatz )

    Kontext
    active.SetPage TabUmrandung
    kontext "TabUmrandung"
    Call DialogTest ( TabUmrandung )
    printlog " Close dialog"
    TabUmrandung.Close

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'------------------------------------------------

testcase tHtmlDocFormatPage
    PrintLog "- Format / Page"
    gApplication = "HTML"
    printlog " Open new document"
    Call hNewDocument

    printlog " Format / Page"
    FormatPageWriter

    Kontext
    printlog " Step through all tabpages"
    active.SetPage TabVerwalten
    kontext "TabVerwalten"
    Call DialogTest ( TabVerwalten )

    Kontext
    active.SetPage TabSeite
    kontext "TabSeite"
    Call DialogTest ( TabSeite )

    Kontext
    active.SetPage TabHintergrund
    Kontext "Active"
    If Active.GetRT = 304 then Active.Yes         ' Messagebox abfangen-> Seitenr�nder au�erhalb des Bereichs

    Kontext
    active.SetPage TabHintergrund
    kontext "TabHintergrund"
    Call DialogTest ( TabHintergrund, 1 )
    Als.Select 2
    Call DialogTest ( TabHintergrund, 2 )
    Durchsuchen.Click

    Kontext "GrafikEinfuegenDlg"
    if ( GrafikEinfuegenDlg.exists( 1 ) ) then
        GrafikEinfuegenDlg.Cancel
    else
        warnlog( "Dialog <GrafikEinfuegenDlg> is not available" )
    endif

    Kontext
    active.SetPage TabKopfzeile
    kontext "TabKopfzeile"
    Call DialogTest ( TabKopfzeile )
    Kontext
    active.SetPage TabFusszeile
    kontext "TabFusszeile"
    Call DialogTest ( TabFusszeile )
    printlog " Close dialog"
    TabFussZeile.Close

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'--------------------------------------------------

testcase tHtmlDocFormatStylist
    PrintLog "- Format / Stylist"
    gApplication = "HTML"
    if gPlatgroup = "unx" then
        QAErrorlog "Doesn't run on unix"
        goto endsub
    end if
    printlog " Open new document"
    Call hNewDocument
    Kontext "Gestalter"
    If Not Gestalter.Exists then FormatStylist
    printlog " Format / Stylist"
    Kontext "Gestalter"
    Call DialogTest ( Gestalter )
    Zeichenvorlagen.Click
    printlog " Click all Styles-buttons in Stylist"
    WaitSlot()
    
    Numerierungsvorlagen.Click
    WaitSlot()
    
    Absatzvorlagen.Click
    WaitSlot()

    TBGestalterRight.Click Giesskanne
    WaitSlot()

    AusSelektion.OpenMenu
    wait( 500 )
    hMenuSelectNr( 1 )

    Kontext "VorlageErzeugen"
    if ( VorlageErzeugen.exists( 1 ) ) then
        DialogTest ( VorlageErzeugen )
        VorlageErzeugen.Cancel
    else
        warnlog( "Dialog <VorlageErzeugen> is not available" )
    endif
    

    Kontext "Gestalter"
    if ( Gestalter.exists( 1 ) ) then
        AusSelektion.OpenMenu
        wait( 500 )
        hMenuSelectNr(2)

        AusSelektion.OpenMenu
        wait( 500 )
        hMenuSelectNr(3)
        
        Kontext "Neu"
        if ( Neu.exists( 10 ) ) then
            DialogTest ( Neu )
            Neu.Cancel
        else
            warnlog( "Dialog <Neu> is not available" )
        endif

        printlog " Close Stylist"
        Kontext "Gestalter"
        Gestalter.Close
    else
        warnlog( "Dialog <Gestalter> is not available" )
    endif

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'------------------------------------------------------
