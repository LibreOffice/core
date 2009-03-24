'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_004_.inc,v $
'*
'* $Revision: 1.3 $
'*
'* last change: $Author: rt $ $Date: 2008-09-04 09:21:45 $
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
'/************************************************************************
'*
'* owner : fredrik.haegg@sun.com
'*
'* short description :  Testcases with connection to the Insert-menu.
'*
'\***********************************************************************

sub w_004_

    printLog Chr(13) + "---------  Insert Menu ( w_004_ ) ----------"
    gApplication = "WRITER"

    Call tInsertManualBreak
    Call tInsertFields

    Call tInsertFieldsOther
    Call tInsertSpecialCharacter

    Call tInsertSection
    Call tInsertHyperlink

    Call tInsertFootnote
    Call tInsertCaption
    Call tInsertBookmark
    Call tInsertCrossReference
    Call tInsertNote
    Call tInsertScript
    Call tInsertIndexesEntry
    Call tInsertIndexesIndex
    Call tInsertIndexesBibliographyEntry

    Call tInsertEnvelope

    Call tInsertFrame
    Call tInsertTable
    Call tInsertHorizontalLine
    Call tInsertGraphicsFromFile
    Call tInsertAllGraphicsFromFile

    Call tInsertObjectOLEObject
    Call tInsertObjectPlugIn
    Call tInsertObjectSound
    Call tInsertObjectVideo
    Call tInsertObjectFormula
    Call tInsertObjectChart
    Call tInsertFile

    'Writer Web
    gApplication = "HTML"
    Call tHtmlDocInsertFields
    Call tHtmlDocInsertFieldsOther
    Call tHtmlDocInsertSection
    Call tHtmlDocInsertFrame
    Call tHtmlDocInsertGraphicsFromFile
    gApplication = "WRITER"

end sub

'-----------------------------------------------------------

testcase tInsertManualBreak
    PrintLog "- Insert / Manual Break"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Manual break"
    InsertManualBreak

    printlog " Close 'Insert break' dialog with cancel"
    Kontext "UmbruchEinfuegen"
    Call DialogTest ( UmbruchEinfuegen )
    UmbruchEinfuegen.Cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertFields
    PrintLog "- Insert / Fields"

    printlog " Open new document"
    Call hNewDocument

    try
        printlog "Insert / Fields / Date"
        InsertFieldsDate
    catch
        Warnlog "Unable to execute: Insert / Fields / Date "
    endcatch
    printlog " Insert / Fields / Date"
    Call wTypeKeys (" <Return>")
    try
        printlog "Insert / Fields / Time"
        InsertFieldsTime
    catch
        Warnlog "Unable to execute: Insert / Fields / Time"
    endcatch
    printlog " Insert / Fields / Time"
    Call wTypeKeys (" <Return>")
    try
        printlog "Insert / Fields / Page Number"
        InsertFieldsPageNumbers
    catch
        Warnlog "Unable to execute: Insert / Fields / Page numbers"
    endcatch
    printlog " Insert / Fields / Page numbers"
    Call wTypeKeys (" <Return>")
    try
        printlog "Insert / Fields / Page Count"
        InsertFieldsPageCount
    catch
        Warnlog "Unable to execute: Insert / Fields / Page Count"
    endcatch
    printlog " Insert / Fields / Page count"
    Call wTypeKeys (" <Return>")
    try
        printlog "Insert / Fields / Title"
        InsertFieldsTitle
    catch
        Warnlog "Unable to execute: Insert / Fields / Title"
    endcatch
    printlog " Insert / Fields / Title"
    Call wTypeKeys (" <Return>")
    try
        printlog "Insert / Fields / Author"
        InsertFieldsAuthor
    catch
        Warnlog "Unable to execute: Insert / Fields / Author"
    endcatch
    printlog " Insert / Fields / Author"
    if gApplication <> "HTML" then
        try
            printlog "Insert / Fields / Subject"
            InsertFieldsSubject
        catch
            Warnlog "Unable to execute: Insert / Fields / Subject"
        endcatch
        printlog " If document not HTML then  'Insert / Fields / Subject"
        Call wTypeKeys (" <Return>")
        printlog " Press key 'enter'"
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertFieldsOther
    Dim i as integer
    Dim j as integer
    printlog "- Insert / Fields / Other"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Fields / Other"
    InsertFieldsOther

    Kontext
    printlog " In dialog step through all tabpages"
    Active.SetPage TabDokumentFeldbefehle
    Kontext "TabDokumentFeldbefehle"
    Call Dialogtest ( TabDokumentFeldbefehle )

    Kontext
    Active.SetPage TabDokumentinfoFeldbefehle
    Kontext "TabDokumentinfoFeldbefehle"
    Call Dialogtest ( TabDokumentinfoFeldbefehle )

    Kontext
    Active.SetPage TabVariablen
    Kontext "TabVariablen"
    Call Dialogtest ( TabVariablen )

    if gApplication <> "HTML" then
        Kontext
        Active.SetPage TabReferenzen
        Kontext "TabReferenzen"
        Call Dialogtest ( TabReferenzen )

        Kontext
        Active.SetPage TabFunktionen
        Kontext "TabFunktionen"
        Call Dialogtest ( TabFunktionen )
        for i=1 to Feldtyp.GetItemCount
            Feldtyp.Select i
            if MakroButton.IsEnabled then
                MakroButton.Click
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
                Sleep 5
                Kontext "ScriptSelector"
                Sleep 1
                ScriptSelector.Close
                Kontext "TabFunktionen"
                i=100
            end if
        next i

        Kontext
        Active.SetPage TabDatenbank
        Kontext "TabDatenbank"
        Call Dialogtest ( TabDatenbank )

        printlog " Close dialog"
        TabDatenbank.Close
    else
        printlog " Close dialog"
        TabVariablen.Close
    end if
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertSpecialCharacter
    PrintLog "- Insert / Special Character"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Special Character"
    InsertSpecialCharacterWriter

    Kontext "Sonderzeichen"
    Call DialogTest (Sonderzeichen)
    Sonderzeichen.Cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertSection
    PrintLog "- Insert / Section"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Section"
    InsertSection
    Kontext

    printlog "  In dialog step through all tabpages"
    printlog "-  Tabpage Section"
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Call DialogTest (TabBereiche)
    Verknuepfung.Check
    if gUseSysDlg = False then
        Auswaehlen.Click
        Kontext "OeffnenDlg"
        OeffnenDlg.Cancel
    end if
    Wait 500
    Kontext
    printlog "-  Tabpage Columns"
    Active.Setpage TabSpalten
    Kontext "TabSpalten"
    Call DialogTest (TabSpalten)

    Kontext
    printlog "-  Tabpage Background"
    Active.Setpage TabHintergrund
    Kontext "TabHintergrund"
    Call DialogTest (TabHintergrund)

    if gApplication <> "HTML" then
        printlog "Tabpage Indents"
        Kontext
        Active.Setpage TabSectionIndent
        Kontext "TabSectionIndent"
        Call DialogTest (TabSectionIndent)

        printlog "-  Tabpage Footnotes/Endnotes"
        Kontext
        Active.Setpage TabFussEndnoten
        Kontext "TabFussEndnoten"
        Call DialogTest (TabFussEndnoten)
        TabFussEndnoten.Cancel
    else
        TabHintergrund.Cancel
    end if
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertHyperlink
    Dim i as integer
    PrintLog "- Insert / Hyperlink"

    printlog " Open new document"
    Call hNewDocument

    printlog "-  Internet"
    printlog " Insert / Hyperlink"
    InsertHyperlink
    printlog "  In 'Hyperlink' dialog step through all pages"
    Call wEinfuegenHyperlink(1)
    Call DialogTest ( TabHyperlinkInternet )

    Kontext "Hyperlink"
    printlog "-  Mail & News"
    Call wEinfuegenHyperlink(2)
    wait 500
    Call DialogTest ( TabHyperlinkMailUndNews )

    Kontext "Hyperlink"
    printlog "-  Document"
    Call wEinfuegenHyperlink(3)
    wait 500
    Call DialogTest ( TabHyperlinkDokument )
    if gUseSysDlg = False then
        DateiOeffnen.Click
        Sleep 1
        Kontext "OeffnenDlg"
        OeffnenDlg.Cancel
        Sleep 1
    end if

    Kontext "Hyperlink"
    printlog "-  New Document"
    Call wEinfuegenHyperlink(4)
    wait 500
    Call DialogTest ( TabHyperlinkNeuesDokument )
    if gUseSysDlg = False then
        Pfad.Click
        Sleep 1
        Kontext "OeffnenDlg"
        OeffnenDlg.Cancel
    end if

    Kontext "TabHyperlinkNeuesDokument"
    TabHyperlinkNeuesDokument.Cancel
    printlog " Close 'Hyperlink' dialog"

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertFootnote
    PrintLog "- Insert / Footnote"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Footnote"
    InsertFootnote
    Kontext "FussnoteEinfuegen"
    Call DialogTest (FussnoteEinfuegen)
    Durchsuchen.Click
    printlog " In 'Insert / Footnote' dialog click on browse button"

    printlog " Close 'Special Character' dialog"
    Kontext "Sonderzeichen"
    Sonderzeichen.Cancel

    printlog " Close 'Insert Footnote' dialog"
    Kontext "FussnoteEinfuegen"
    FussnoteEinfuegen.Cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertCaption
    PrintLog "- Insert / Caption"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert graphic '..\writer\required\input\graphics\jolink.jpg"
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    try
        printlog " Insert / Caption"
        InsertCaption
        Kontext "Beschriftung"
        Call DialogTest ( Beschriftung )
        Sleep 1
        printlog " In 'Caption' dialog click 'Options'"
        WeitereOptionen.Click
        Kontext "NummernkreisOptionen"
        Call DialogTest ( NummernkreisOptionen )
        printlog " Close 'Number Range Options' dialog"
        NummernkreisOptionen.Cancel
        Sleep 1
        Kontext "Beschriftung"
        printlog " Close 'Caption' dialog"
        Beschriftung.Cancel
        Sleep 2
    catch
        Warnlog "Insert / Caption is disabled!"
    endcatch

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertBookmark
    PrintLog "- Insert / Bookmark"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Bookmark"
    InsertBookmark
    Kontext "TextmarkeEinfuegen"
    Call DialogTest (TextmarkeEinfuegen)
    wait 500
    printlog " Close 'Insert Bookmark' dialog"
    TextmarkeEinfuegen.Cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertNote
    dim sNoteText as String
    PrintLog "- Insert / Note"

    Call hNewDocument

    printlog " Insert / Note"
    InsertNoteWriter
    Kontext "DocumentWriter"
    sNoteText = "Testnote inserted by testtool !"
    Call wTypeKeys (sNoteText)

    Call wNavigatorAuswahl( 11, 1 )
    Kontext "NavigatorWriter"
    if Auswahlliste.Gettext <> sNoteText then
        warnlog ("Note was not inserted correctly (Focus problem?")
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertCrossReference
    Dim i as integer
    Dim j as integer
    printlog "- Insert / Reference Field"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if
    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Cross reference"
    InsertReferenceField

    Kontext "TabReferenzen"
    if NOT TabReferenzen.IsVisible then Warnlog "Reference Tabpage isn't default!"

    printlog " Step through all tabpages"
    Kontext
    Active.SetPage TabFunktionen
    Kontext "TabFunktionen"
    for i=1 to Feldtyp.GetItemCount
        Feldtyp.Select i
        if MakroButton.IsEnabled then
            MakroButton.Click
            ' if no JRE is installed a messagebox appears
            j = 0
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
            Kontext "ScriptSelector"
            Sleep 1
            ScriptSelector.Close
            Kontext "TabFunktionen"
            i=100
        end if
    next i

    Kontext
    Active.SetPage TabDokumentinfoFeldbefehle
    Kontext "TabDokumentinfoFeldbefehle"

    Kontext
    Active.SetPage TabVariablen
    Kontext "TabVariablen"

    Kontext
    Active.SetPage TabDatenbank
    Kontext "TabDatenbank"

    Kontext
    Active.SetPage TabDokumentFeldbefehle
    Kontext "TabDokumentFeldbefehle"

    printlog " Close dialog"
    TabDokumentFeldbefehle.Close

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertScript
    PrintLog "- Insert / Script"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Script"
    InsertScript
    Kontext "ScriptEinfuegen"
    Call DialogTest (ScriptEinfuegen)
    wait 500
    ScriptEinfuegen.Cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertIndexesEntry
    qaErrorLog "#158646# - Insert Indexes Entry causes crash."
    goto endsub

    PrintLog "- Insert / Indexes / Entry"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if
    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Indexes and Tables / Entry"
    InsertIndexesEntry
    Kontext "VerzeichniseintragEinfuegen"
    Call DialogTest (VerzeichniseintragEinfuegen)
    printlog " Click 'Insert'"
    EintragNeu.Click

    Kontext "BenutzerverzeichnisDefinieren"
    Call DialogTest (BenutzerverzeichnisDefinieren)
    printlog " Close dialog 'Insert Index Entry'"
    BenutzerverzeichnisDefinieren.Cancel
    Kontext "VerzeichniseintragEinfuegen"
    EinfuegenSchliessen.Click

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertIndexesIndex
    Dim i as integer
    Printlog "- Insert / Indexes / Index"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if
    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Indexes and Tables / Indexes and Tables"
    InsertIndexes
    WaitSlot (5000)
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Warnlog Active.Gettext
            Active.Ok
        end if
    end if
    Sleep 1
    printlog " Step through all Tabpages"
    Kontext
    printlog "-  TabPage Indexes"
    Active.Setpage TabVerzeichnisseVerzeichnis
    Kontext "TabVerzeichnisseVerzeichnis"
    For i= 1 to VerzeichnisTyp.GetItemCount
        Sleep 1
        if i = 2 then printlog "Alphabetical index see bug #i33671 (ok in m54)"
        VerzeichnisTyp.Select i
        printlog "- " + VerzeichnisTyp.GetSelText
        Call DialogTest (TabVerzeichnisseVerzeichnis)
        printlog "-  Entry's to " + VerzeichnisTyp.GetSelText

        if i = 1 then
            ErzeugenAusGliederung.Check
        end if

        if i = 2 then
            if gUseSysDlg = False then
                Konkordanzdatei.Check
                Kontext "KonkordanzdateiOeffnen"
                if KonkordanzdateiOeffnen.Exists then
                    KonkordanzdateiOeffnen.Cancel
                    Warnlog "Open-Dialog comes up at checking the checkbox!"
                end if
                Kontext "TabVerzeichnisseVerzeichnis"
                if KonkordanzAuswahl.IsEnabled then
                    KonkordanzAuswahl.Click
                    wait 500
                    Call hMenuSelectNr ( 1 )
                    Kontext "KonkordanzdateiOeffnen"
                    if KonkordanzdateiOeffnen.Exists then
                        KonkordanzdateiOeffnen.Cancel
                    else
                        Warnlog "No Open-Dialog selecting Concordance-File->Open!"
                    end if
                    Sleep 1
                    Kontext "TabVerzeichnisseVerzeichnis"
                    KonkordanzAuswahl.Click
                    wait 500
                    Call hMenuSelectNr ( 2 )
                    Kontext "SpeichernDlg"
                    if SpeichernDlg.Exists then
                        SpeichernDlg.Cancel
                    else
                        Warnlog "No Open-Dialog selecting Concordance-File->New!"
                    end if
                else
                    Warnlog "- Concordance-File couldn't be selected, though checkbox is checked!"
                end if
                Sleep 1
            end if
        end if

        Kontext
        Active.Setpage TabVerzeichnisseEintraege
        Kontext "TabVerzeichnisseEintraege"
        Call DialogTest (TabVerzeichnisseEintraege)
        Kontext
        Active.Setpage TabVerzeichnisseVerzeichnis
        Kontext "TabVerzeichnisseVerzeichnis"
    next i
    Sleep 1

    Kontext
    printlog "-  Tabpage Styles"
    Active.Setpage TabVerzeichnisseVorlagen
    Kontext "TabVerzeichnisseVorlagen"
    Call DialogTest (TabVerzeichnisseVorlagen)
    Sleep 1
    printlog "- Tabpage Columns"
    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"
    Call DialogTest (TabSpalten)
    Sleep 1
    printlog "- Tabpage Background"
    Kontext
    Active.Setpage TabHintergrund
    Kontext "TabHintergrund"
    TabHintergrund.Cancel
    Sleep 1
    printlog " Close active document"
    Call hCloseDocument

    Sleep 2
    printlog " Open new document"
    Call hNewDocument
    Sleep 2

    printlog " View Zoom"
    ViewZoom

    Kontext "Massstab"
    printlog " Check if 100% Zoom is checked ( Had a bug changed after opening Index dialog )"
    if Massstab.Exists then
        if Not Vergroesserung100.IsChecked then
            Warnlog "100% Zoom is not checked.Maybe Bug#95871! Set Zoom to 100%!"
            Vergroesserung100.Check
            Massstab.ok
        else
            Massstab.Cancel
            printlog " Close 'Zoom' dialog"
        end if
    else
        Warnlog "Dialog 'Zoom' does not exist!"
    end if
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertIndexesBibliographyEntry
    Printlog "- Insert / Indexes / Bibliography Entry"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if
    printlog " Open new document"
    Call hNewDocument
    Sleep 2

    printlog " Insert / Indexes and Tables / Bibliography Entry "
    InsertIndexesBibliographyEntry
    Kontext "Active"
    if Active.Exists then
        Do
            try
                if Active.GetRT = 304 then
                    Warnlog Active.Gettext
                    Active.Ok
                end if
            catch
                exit do
            endcatch
        Loop
    end if

    Kontext "LiteratureintragEinfuegen"
    Call DialogTest ( LiteratureintragEinfuegen )
    printlog " In dialog 'Insert Bibliography Entry' click 'Edit'"
    EintragBearbeiten.Click

    Kontext "LiteratureintragDefinieren"
    Call DialogTest ( LiteratureintragDefinieren )
    printlog " Close dialog 'Define Bibliography Entry'"
    LiteratureintragDefinieren.Close
    Kontext "LiteratureintragEinfuegen"
    printlog " Close dialog 'Insert Bibliography Entry'"
    EintragSchliessen.Click
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertEnvelope
    PrintLog "- Insert / Envelope"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Envelope"
    InsertEnvelope

    Kontext
    active.SetPage TabEnvelope
    Kontext "TabEnvelope"
    Call DialogTest (TabEnvelope)

    Kontext
    active.SetPage TabFormatEnvelope
    Kontext "TabFormatEnvelope"
    Call DialogTest (TabFormatEnvelope)
    printlog " Step through all tabpages and click all buttons"

    printlog "-  Edit Addressee"
    Sleep 1
    AddresseeEdit.Click
    wait 500
    Call hMenuSelectNr(1)
    Kontext
    Sleep 1
    Active.Setpage TabFont
    Kontext "TabFont"
    TabFont.Cancel

    Kontext "TabFormatEnvelope"
    Sleep 1
    SenderEdit.Click
    wait 500
    Call hMenuSelectNr(2)
    Kontext
    Sleep 1
    Active.Setpage TabEinzuegeUndAbstaende
    Kontext "TabEinzuegeUndAbstaende"
    TabEinzuegeUndAbstaende.Cancel

    Kontext "TabFormatEnvelope"
    printlog "-  Edit Sender"
    Sleep 1
    SenderEdit.Click
    wait 500
    Call hMenuSelectNr(1)
    Kontext
    Sleep 1
    Active.Setpage TabFont
    Kontext "TabFont"
    TabFont.Cancel

    Kontext "TabFormatEnvelope"
    Sleep 1
    SenderEdit.Click
    wait 500
    Call hMenuSelectNr(2)
    Kontext
    Sleep 1
    Active.Setpage TabEinzuegeUndAbstaende
    Kontext "TabEinzuegeUndAbstaende"
    TabEinzuegeUndAbstaende.Cancel

    Kontext
    Sleep 2
    active.SetPage TabDruckerBriefumschlag
    Kontext "TabDruckerBriefumschlag"
    Call DialogTest (TabDruckerBriefumschlag)
    if gPlatgroup = "w95" then
        Einrichten.Click
        Kontext "DruckerEinrichten"
        if DruckerEinrichten.Exists then
            DruckerEinrichten.Cancel
        else
            Warnlog "No Default-Printer!"
        end if
        Kontext "TabDruckerBriefumschlag"
    end if
    printlog " Close dialog"
    TabDruckerBriefumschlag.Close

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertFrame
    PrintLog "- Insert / Frame"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Frame"
    InsertFrame
    WaitSlot (2000)

    printlog " Step through all tabpages"
    Kontext
    active.SetPage TabType
    Kontext "TabType"
    Call DialogTest (TabType)
    Sleep 1

    Kontext
    active.SetPage TabZusaetze
    Kontext "TabZusaetze"
    Call DialogTest (TabZusaetze)
    Sleep 1

    Kontext
    active.SetPage TabUmlauf
    Kontext "TabUmlauf"
    Call DialogTest (TabUmlauf)
    Sleep 1

    Kontext
    active.SetPage TabUmrandung
    Kontext "TabUmrandung"
    Call DialogTest (TabUmrandung)
    Sleep 1

    Kontext
    active.SetPage TabHintergrund
    Kontext "TabHintergrund"
    Call DialogTest (TabHintergrund, 1)
    Sleep 1
    try
        Als.Select (2)
        Call DialogTest (TabHintergrund, 2)
        Sleep 1
    catch
        Warnlog "Maybe missing combobox! (#i29517)"
    endcatch

    if gApplication <> "HTML" then
        Kontext
        active.SetPage TabHyperlinkRahmen
        Kontext "TabHyperlinkRahmen"
        Call DialogTest (TabHyperlinkRahmen)
        Durchsuchen.Click

        if gUSeSysDlg = False then
            Kontext "OeffnenDlg"
            OeffnenDlg.Cancel
        end if

        Kontext
        active.SetPage TabSpalten
        Kontext "TabSpalten"
        Call DialogTest (TabSpalten)
        printlog " Close dialog"
        TabSpalten.Cancel
    else
        printlog " Close dialog"
        TabHintergrund.Cancel
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertTable
    PrintLog "- Insert / Table"

    printlog " Open new document"
    Call hNewDocument
    printlog " Insert / Table"
    InsertTableWriter

    Kontext "TabelleEinfuegenWriter"
    Call DialogTest (TabelleEinfuegenWriter)
    if Spalten.Gettext <> "2" then Warnlog "- Default for Size Columns isn't 2 , it is " + Spalten.Gettext
    if Zeilen.Gettext <> "2" then Warnlog "- Default for Size Row isn't 2, it is " + Zeilen.Gettext
    printlog " In 'insert Table' dialog click 'AutoFormat'"
    Autoformat.Click
    Kontext "AutoformatTabelle"

    printlog " In 'AutoFormat' dialog click 'More'"
    Zusaetze.Click
    Call DialogTest ( AutoformatTabelle )

    if Formatbox.GetItemCount > 2 then
        Formatbox.Select(3)
        Zusaetze.Open
        Umbenennen.Click
        Kontext "AutoformatHinzufuegenWriter"
        Call DialogTest ( AutoformatHinzufuegenWriter )
        TabellenName.SetText "Hallo"
        AutoformatHinzufuegenWriter.Cancel
        Kontext "AutoFormatTabelle"
    else
        Warnlog "Only " + Formatbox.GetItemCount + " Table formats to select ?"
    end if
    printlog " Close dialog 'More'"
    Zusaetze.Close
    printlog " Close dialog 'AutoFormat'"
    AutoformatTabelle.Cancel

    Kontext "TabelleEinfuegenWriter"
    printlog " Close dialog ''"
    TabelleEinfuegenWriter.Cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertHorizontalLine
    PrintLog "- Insert / Horizontal Line"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Horizontal Line"
    InsertHorizontalLine
    Kontext "HorizontaleLinieEinfuegen"
    Call DialogTest (HorizontaleLinieEinfuegen)
    printlog " Close dialog 'Insert Horizontal Ruler'"
    HorizontaleLinieEinfuegen.Cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertGraphicsFromFile
    PrintLog "- Insert / Graphics / From File"
    Dim j as integer

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Graphics from file"
    InsertGraphicsFromFile

    Kontext "GrafikEinfuegenDlg"
    printlog " Select as graphic to insert: '...\writer\required\input\graphics\jolink.jpg"
    Dateiname.Settext ConvertPath ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    Oeffnen.Click
    sleep 1

    Kontext "Active"
    if Active.Exists then
        Warnlog Active.Gettext
        Active.Ok
        Kontext "GrafikEinfuegenDlg"
        GrafikEinfuegenDlg.Cancel
        Call hCloseDocument
        goto endsub
    else
        printlog " select again 'Insert / Grahics from file'"
        InsertGraphicsFromFile
        WaitSlot (2000)
        printlog " Step through all tabpages"
        Kontext
        active.SetPage TabType
        wait 500
        Kontext "TabType"
        Call DialogTest ( TabType )

        Kontext
        active.SetPage TabZusaetze
        wait 500
        Kontext "TabZusaetze"
        Call DialogTest ( TabZusaetze )

        Kontext
        active.SetPage TabUmlauf
        wait 500
        Kontext "TabUmlauf"
        Call DialogTest ( TabUmlauf )

        Kontext
        active.SetPage TabHyperlinkRahmen
        wait 500
        Kontext "TabHyperlinkRahmen"
        Call DialogTest ( TabHyperlinkRahmen )

        Kontext
        active.SetPage TabGrafik
        wait 500
        Kontext "TabGrafik"
        Call DialogTest ( TabGrafik )

        Kontext
        active.SetPage TabUmrandung
        wait 500
        Kontext "TabUmrandung"
        Call DialogTest ( TabUmrandung )

        Kontext
        active.SetPage TabMakro
        ' if no JRE is installed a messagebox appears
        j = 0
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
        wait 500
        Kontext "TabMakro"
        Call DialogTest ( TabMakro )

        if gApplication <> "HTML" then
            Kontext
            active.SetPage TabZuschneiden
            wait 500
            Kontext "TabZuschneiden"
            Call DialogTest ( TabZuschneiden )

            Kontext
            active.SetPage TabHintergrund
            wait 500
            Kontext "TabHintergrund"
            Call DialogTest ( TabHintergrund )
            printlog " Close dialog"
            TabHintergrund.Cancel
        else
            printlog " Close dialog"
            TabMakro.Cancel
        end if
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertAllGraphicsFromFile
    Dim i as integer
    Dim quellpfad as string
    Dim DieDatei as string
    Dim Ziel( 5000 ) as String
    printlog "Insert / Graphics / From File (all Formats)"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert all graphic formats from directory: global\input\graf_inp\"
    quellpfad = ConvertPath(gtesttoolpath + "global\input\graf_inp\")

    GetFileList(quellpfad, "*.*" , Ziel())
    for i=1 to ListCount(Ziel())
        printlog "- Graphic: " + Right(Ziel(i),3) + "-Format"
        Call hGrafikEinfuegen ( Ziel(i) )
        Sleep 5
        Call wTypeKeys("<Delete>")
        Call gMouseDoubleClick (10,10)
        Sleep 5
    next i

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertObjectOLEObject
    PrintLog "- Insert / Object / OLE-Objekt"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Object / OLE-Object"
    InsertObjectOLEObject

    Kontext "OLEObjektEinfuegen"
    printlog " Check both radiobuttons 'Create now' and 'Create from file'"
    NeuErstellen.Check
    Call DialogTest (OLEObjektEinfuegen, 1)
    AusDateiErstellen.Check
    Call DialogTest (OLEObjektEinfuegen, 2)
    OLEObjektEinfuegen.Cancel

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertObjectPlugIn
    PrintLog "- Insert / Object / Plug-In"

    printlog " Open new document"
    Call hNewDocument

    printlog " Close 'Plug in' dialog"
    InsertObjectPlugIn

    Kontext "PlugInEinfuegen"
    if PlugInEinfuegen.Exists then
        Call DialogTest (PlugInEinfuegen)
        PlugInEinfuegen.Cancel
    else
        Warnlog "Dialog 'Plugin' is not up!"
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertObjectSound
    Printlog "- Insert / Object / Sound"
    printlog " Open new document"
    Call hNewDocument

    try
        if gUseSysDlg = False then
            InsertObjectSound
            printlog " Insert / Object / Sound"
            Kontext "OeffnenDlg"
            if OeffnenDlg.Exists then
                OeffnenDlg.Cancel
            else
                Warnlog "Dialog to insert Sound isn't up!"
            end if
        end if
    catch
        'Warnlog "- Menu disabled or no Sound-PlugIn installed !?"
    endcatch
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertObjectVideo
    Printlog "- Insert / Object / Video"

    printlog " Open new document"
    Call hNewDocument

    try
        if gUseSysDlg = False then
            InsertObjectVideo
            printlog " Insert / Object / Video"
            Kontext "OeffnenDlg"
            if OeffnenDlg.Exists then
                OeffnenDlg.Cancel
            else
                Warnlog "Dialog to insert Video isn't up!"
            end if
        end if
    catch
        'Warnlog "- Menu disabled or no Video-PlugIn installed !?"
    endcatch
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertObjectFormula
    PrintLog "- Insert / Object / Formula"

    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Object / Formular"
    InsertObjectFormula
    WaitSlot (2000)
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Warnlog Active.Gettext
            Active.Ok
        end if
    end if
    Sleep 1

    printlog " Click in 'Math' document"
    Call gMouseClick(1,1)
    sleep 1
    printlog "  'Delete math-object in document"
    Call wTypeKeys("<Delete>")

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertObjectChart
    PrintLog "- Insert / Object / Chart"

    printlog " Open new document"
    Call hNewDocument
    ' Chart-Wizard will be invoked if data ranges exist
    ' Therefor a table will be inserted
    printlog " Insert / Table"
    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"
    TabelleEinfuegenWriter.Ok

    printlog " Insert / Object / Chart"
    InsertObjectChart

    Kontext "ChartWizard"
    if ChartWizard.Exists then
        printlog "- Chart-Wizard is up=>OK"
        Call DialogTest ( ChartWizard )
        printlog " Cancel Chart-Wizard"
        ChartWizard.Cancel
    else
        warnlog "Chart-Wizard is not up though a table has been inserted"
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tInsertFile
    PrintLog "- Insert / File"

    printlog " Open new document"
    Call hNewDocument
    if gUseSysDlg = False then
        InsertFileWriter
        printlog " Insert / File"
        WaitSlot (3000)
        Kontext "OeffnenDlg"
        WaitSlot (2000)
        printlog " Check 'File dialog'"
        if OeffnenDlg.Exists then
            Call DialogTest ( OeffnenDlg )
            try
                Standard.Click
            catch
                Exceptlog
            endcatch

            Kontext "Active"
            if Active.Exists then
                Warnlog Active.Gettext
                Active.Ok
            else
                Kontext "OeffnenDlg"
                OeffnenDlg.Cancel
            end if
            Sleep 1
        end if
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tHtmlDocInsertFields
    PrintLog "- Insert / Fields"

    gApplication = "HTML"
    printlog " Open new document"
    Call hNewDocument

    try
        printlog "Insert / Fields / Date"
        InsertFieldsDate
    catch
        Warnlog "Unable to execute: Insert / Fields / Date "
    endcatch
    printlog " Insert / Fields / Date"
    Call wTypeKeys (" <Return>")
    try
        printlog "Insert / Fields / Time"
        InsertFieldsTime
    catch
        Warnlog "Unable to execute: Insert / Fields / Time"
    endcatch
    printlog " Insert / Fields / Time"
    Call wTypeKeys (" <Return>")
    try
        printlog "Insert / Fields / Page Number"
        InsertFieldsPageNumbers
    catch
        Warnlog "Unable to execute: Insert / Fields / Page numbers"
    endcatch
    printlog " Insert / Fields / Page numbers"
    Call wTypeKeys (" <Return>")
    try
        printlog "Insert / Fields / Page Count"
        InsertFieldsPageCount
    catch
        Warnlog "Unable to execute: Insert / Fields / Page Count"
    endcatch
    printlog " Insert / Fields / Page count"
    Call wTypeKeys (" <Return>")
    try
        printlog "Insert / Fields / Title"
        InsertFieldsTitle
    catch
        Warnlog "Unable to execute: Insert / Fields / Title"
    endcatch
    printlog " Insert / Fields / Title"
    Call wTypeKeys (" <Return>")
    try
        printlog "Insert / Fields / Author"
        InsertFieldsAuthor
    catch
        Warnlog "Unable to execute: Insert / Fields / Author"
    endcatch
    printlog " Insert / Fields / Author"
    if gApplication <> "HTML" then
        try
            printlog "Insert / Fields / Subject"
            InsertFieldsSubject
        catch
            Warnlog "Unable to execute: Insert / Fields / Subject"
        endcatch
        printlog " If document not HTML then  'Insert / Fields / Subject"
        Call wTypeKeys (" <Return>")
        printlog " Press key 'enter'"
    end if

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tHtmlDocInsertFieldsOther
    Dim i as integer
    Dim j as integer
    printlog "- Insert / Fields / Other"

    gApplication = "HTML"
    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Fields / Other"
    InsertFieldsOther

    Kontext
    printlog " In dialog step through all tabpages"
    Active.SetPage TabDokumentFeldbefehle
    Kontext "TabDokumentFeldbefehle"
    Call Dialogtest ( TabDokumentFeldbefehle )

    Kontext
    Active.SetPage TabDokumentinfoFeldbefehle
    Kontext "TabDokumentinfoFeldbefehle"
    Call Dialogtest ( TabDokumentinfoFeldbefehle )

    Kontext
    Active.SetPage TabVariablen
    Kontext "TabVariablen"
    Call Dialogtest ( TabVariablen )

    if gApplication <> "HTML" then
        Kontext
        Active.SetPage TabReferenzen
        Kontext "TabReferenzen"
        Call Dialogtest ( TabReferenzen )

        Kontext
        Active.SetPage TabFunktionen
        Kontext "TabFunktionen"
        Call Dialogtest ( TabFunktionen )
        for i=1 to Feldtyp.GetItemCount
            Feldtyp.Select i
            if MakroButton.IsEnabled then
                MakroButton.Click
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
                Sleep 5
                Kontext "ScriptSelector"
                Sleep 1
                ScriptSelector.Close
                Kontext "TabFunktionen"
                i=100
            end if
        next i

        Kontext
        Active.SetPage TabDatenbank
        Kontext "TabDatenbank"
        Call Dialogtest ( TabDatenbank )

        printlog " Close dialog"
        TabDatenbank.Close
    else
        printlog " Close dialog"
        TabVariablen.Close
    end if
    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tHtmlDocInsertSection
    PrintLog "- Insert / Section"

    gApplication = "HTML"
    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Section"
    InsertSection
    Kontext

    printlog "  In dialog step through all tabpages"
    printlog "-  Tabpage Section"
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Call DialogTest (TabBereiche)
    Verknuepfung.Check
    if gUseSysDlg = False then
        Auswaehlen.Click
        Kontext "OeffnenDlg"
        OeffnenDlg.Cancel
    end if
    Wait 500
    Kontext
    printlog "-  Tabpage Columns"
    Active.Setpage TabSpalten
    Kontext "TabSpalten"
    Call DialogTest (TabSpalten)

    Kontext
    printlog "-  Tabpage Background"
    Active.Setpage TabHintergrund
    Kontext "TabHintergrund"
    Call DialogTest (TabHintergrund)

    if gApplication <> "HTML" then
        printlog "Tabpage Indents"
        Kontext
        Active.Setpage TabSectionIndent
        Kontext "TabSectionIndent"
        Call DialogTest (TabSectionIndent)

        printlog "-  Tabpage Footnotes/Endnotes"
        Kontext
        Active.Setpage TabFussEndnoten
        Kontext "TabFussEndnoten"
        Call DialogTest (TabFussEndnoten)
        TabFussEndnoten.Cancel
    else
        TabHintergrund.Cancel
    end if
    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tHtmlDocInsertFrame
    PrintLog "- Insert / Frame"

    gApplication = "HTML"
    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Frame"
    InsertFrame
    WaitSlot (2000)

    printlog " Step through all tabpages"
    Kontext
    active.SetPage TabType
    Kontext "TabType"
    Call DialogTest (TabType)
    Sleep 1

    Kontext
    active.SetPage TabZusaetze
    Kontext "TabZusaetze"
    Call DialogTest (TabZusaetze)
    Sleep 1

    Kontext
    active.SetPage TabUmlauf
    Kontext "TabUmlauf"
    Call DialogTest (TabUmlauf)
    Sleep 1

    Kontext
    active.SetPage TabUmrandung
    Kontext "TabUmrandung"
    Call DialogTest (TabUmrandung)
    Sleep 1

    Kontext
    active.SetPage TabHintergrund
    Kontext "TabHintergrund"
    Call DialogTest (TabHintergrund, 1)
    Sleep 1
    try
        Als.Select (2)
        Call DialogTest (TabHintergrund, 2)
        Sleep 1
    catch
        Warnlog "Maybe missing combobox! (#i29517)"
    endcatch

    if gApplication <> "HTML" then
        Kontext
        active.SetPage TabHyperlinkRahmen
        Kontext "TabHyperlinkRahmen"
        Call DialogTest (TabHyperlinkRahmen)
        Durchsuchen.Click

        if gUSeSysDlg = False then
            Kontext "OeffnenDlg"
            OeffnenDlg.Cancel
        end if

        Kontext
        active.SetPage TabSpalten
        Kontext "TabSpalten"
        Call DialogTest (TabSpalten)
        printlog " Close dialog"
        TabSpalten.Cancel
    else
        printlog " Close dialog"
        TabHintergrund.Cancel
    end if

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tHtmlDocInsertGraphicsFromFile
    PrintLog "- Insert / Graphics / From File"
    Dim j as integer

    gApplication = "HTML"
    printlog " Open new document"
    Call hNewDocument

    printlog " Insert / Graphics from file"
    InsertGraphicsFromFile

    Kontext "GrafikEinfuegenDlg"
    printlog " Select as graphic to insert: '...\writer\required\input\graphics\jolink.jpg"
    Dateiname.Settext ConvertPath ( gTesttoolPath + "writer\required\input\graphics\jolink.jpg" )
    Oeffnen.Click
    sleep 1

    Kontext "Active"
    if Active.Exists then
        Warnlog Active.Gettext
        Active.Ok
        Kontext "GrafikEinfuegenDlg"
        GrafikEinfuegenDlg.Cancel
        Call hCloseDocument
        goto endsub
    else
        printlog " select again 'Insert / Grahics from file'"
        InsertGraphicsFromFile
        WaitSlot (2000)
        printlog " Step through all tabpages"
        Kontext
        active.SetPage TabType
        wait 500
        Kontext "TabType"
        Call DialogTest ( TabType )

        Kontext
        active.SetPage TabZusaetze
        wait 500
        Kontext "TabZusaetze"
        Call DialogTest ( TabZusaetze )

        Kontext
        active.SetPage TabUmlauf
        wait 500
        Kontext "TabUmlauf"
        Call DialogTest ( TabUmlauf )

        Kontext
        active.SetPage TabHyperlinkRahmen
        wait 500
        Kontext "TabHyperlinkRahmen"
        Call DialogTest ( TabHyperlinkRahmen )

        Kontext
        active.SetPage TabGrafik
        wait 500
        Kontext "TabGrafik"
        Call DialogTest ( TabGrafik )

        Kontext
        active.SetPage TabUmrandung
        wait 500
        Kontext "TabUmrandung"
        Call DialogTest ( TabUmrandung )

        Kontext
        active.SetPage TabMakro
        ' if no JRE is installed a messagebox appears
        j = 0
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
        wait 500
        Kontext "TabMakro"
        Call DialogTest ( TabMakro )

        if gApplication <> "HTML" then
            Kontext
            active.SetPage TabZuschneiden
            wait 500
            Kontext "TabZuschneiden"
            Call DialogTest ( TabZuschneiden )

            Kontext
            active.SetPage TabHintergrund
            wait 500
            Kontext "TabHintergrund"
            Call DialogTest ( TabHintergrund )
            printlog " Close dialog"
            TabHintergrund.Cancel
        else
            printlog " Close dialog"
            TabMakro.Cancel
        end if
    end if

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------
