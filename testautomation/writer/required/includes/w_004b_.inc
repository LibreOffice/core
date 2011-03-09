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
'* owner : Helge.Delfs@Sun.Com
'*
'* short description : Testcases with connection to the Insert-menu part II.
'*
'\***********************************************************************

sub w_004b_

    printLog Chr(13) + "---------  Insert Menu ( w_004_ ) ----------"

    gApplication = "WRITER"
    call tInsertHorizontalLine
    call tInsertGraphicsFromFile
    call tInsertAllGraphicsFromFile
    call tInsertObjectOLEObject
    call tInsertObjectPlugIn
    call tInsertObjectSound
    call tInsertObjectVideo
    call tInsertObjectFormula
    call tInsertObjectChart
    call tInsertFile

    gApplication = "HTML"
    call tHtmlDocInsertFields
    call tHtmlDocInsertFieldsOther
    call tHtmlDocInsertSection
    call tHtmlDocInsertFrame
    call tHtmlDocInsertGraphicsFromFile
    gApplication = "WRITER"

end sub

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

    Kontext "Active"
    if Active.Exists( 1 ) then
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
            if Active.Exists() then
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

    printlog( "Load files of all known graphics formats" )

    dim iCurrentFile as integer
    dim GraphicFile as String
    dim Filtername as String

    printlog( "Insert a sample of commonly used file formats into a writer document (not linked)" )

    gApplication = "WRITER"
    Call hNewDocument

    for iCurrentFile = 1 to 25
        Kontext "GrafikEinfuegenDlg"
        if ( not GrafikEinfuegenDlg.exists() ) then
            InsertGraphicsFromFile
            Kontext "GrafikEinfuegenDlg"
        endif

        if ( GrafikEinfuegenDlg.Exists( 2 ) ) then
            ' the list of files expected to be found in the samples directory
            select case iCurrentFile
                case 1  :  GraphicFile = "desp.bmp"
                case 2  :  GraphicFile = "columbia.dxf"
                case 3  :  GraphicFile = "extrud.emf"
                case 4  :  GraphicFile = "ball.eps"
                case 5  :  GraphicFile = "airshw.met"
                case 6  :  GraphicFile = "eface.gif"
                case 7  :  GraphicFile = "purzel.gif"         ' Animated Gif
                case 8  :  GraphicFile = "borabora.jpg"
                case 9  :  GraphicFile = "grafix1.pbm"
                case 10 :  GraphicFile = "circle.pcx"
                case 11 :  GraphicFile = "papagei1.pcd"
                case 12 :  GraphicFile = "newtonme.pct"
                case 13 :  GraphicFile = "grafix2.pgm"
                case 14 :  GraphicFile = "obst.png"
                case 15 :  GraphicFile = "london.ppm"
                case 16 :  GraphicFile = "tiger.psd"
                case 17 :  GraphicFile = "grafix3.ras"
                case 18 :  GraphicFile = "mickym.sgf"
                case 19 :  GraphicFile = "faxpfeil.sgv"
                case 20 :  GraphicFile = "picture.svm"
                case 21 :  GraphicFile = "pilz.tga"
                case 22 :  GraphicFile = "stabler.tif"
                case 23 :  GraphicFile = "krokodil.wmf"
                case 24 :  GraphicFile = "grafix4.xbm"
                case 25 :  GraphicFile = "papagei2.xpm"
            end select

            GraphicFile = ConvertPath (gTesttoolPath & "global\input\graf_inp\" & GraphicFile)
            printlog( "Current file " + iCurrentFile + ": " & GraphicFile )
            printlog( " * Set filter to <All formats>" )
            DateiName.SetText( GraphicFile )
            DateiTyp.Select( 1 )
            WaitSlot()

            printlog( " * Uncheck Preview- and Link-checkbox, load the file" )
            Vorschau.unCheck
            Verknuepfen.unCheck()
            Oeffnen.click()
            WaitSlot( 2000 )

            Kontext "Active"
            if ( Active.Exists() ) then
                Warnlog( "Unexpected messagebox: " & Active.GetText() )
                Warnlog( "Affected file: " & GraphicFile )
                Active.OK
            else
                try
                    EditCopy
                    WaitSlot( 2000 )
                    EditDeleteContents
                    WaitSlot( 2000 )
                catch
                    Warnlog( "Graphic file not selected/Copy or Delete is disabled: " & GraphicFile )
                    gMouseClick (90, 90)
                endcatch
            endif
        else
            warnlog( "Insert Graphics Dialog is not open, aborting" )
            goto endsub
        endif

    skipformat:
    next iCurrentFile

    printlog( "" )
    printlog( "Test the PCD-Filters directly (3 samples)" )

    for iCurrentFile = 1 to 3

        printlog( "" )

        Kontext "GrafikEinfuegenDlg"
        if ( not GrafikEinfuegenDlg.exists() ) then
            InsertGraphicsFromFile
            Kontext "GrafikEinfuegenDlg"
        endif

        if ( GrafikEinfuegenDlg.exists( 2 ) ) then

            select case iCurrentFile
            case 1 : Filtername = "PCD - Kodak Photo CD (768x512) (*.pcd)"
            case 2 : Filtername = "PCD - Kodak Photo CD (384x256) (*.pcd)"
            case 3 : Filtername = "PCD - Kodak Photo CD (192x128) (*.pcd)"
            end select

            GraphicFile = ConvertPath( gTesttoolPath & "global\input\graf_inp\papagei1.pcd" )

            printlog( "Current filter: " & Filtername )
            printlog( "Current file..: " & GraphicFile )
            DateiName.SetText ( GraphicFile )
            DateiTyp.Select( Filtername )

            printlog( " * Uncheck Preview- and Link-checkbox, load the file" )
            Vorschau.Uncheck()
            Verknuepfen.unCheck()
            Oeffnen.click()
            WaitSlot( 2000 )

            Kontext "Active"
            if ( Active.Exists() )then
                Warnlog( "Unexpected messagebox: " & Active.GetText() )
                Warnlog( "Affected file..: " & GraphicFile )
                Warnlog( "Affected filter: " & Filtername )
                Active.OK()
            else
                try
                    EditCopy
                    WaitSlot( 2000 )
                    EditDeleteContents
                    WaitSlot( 2000 )
                catch
                    Warnlog( "Problems with filtertype : " & Filtername )
                    gMouseClick (90, 90)
                endcatch
            endif
        else
            warnlog( "Insert Graphics Dialog is not open. Aborting" )
            goto endsub
        endif

    next iCurrentFile

    Kontext "GrafikEinfuegenDlg"
    if GrafikEinfuegenDlg.Exists then
        GrafikEinfuegenDlg.Cancel
    endif

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
    InsertFileWriter
    WaitSlot( 2000 )

    printlog " Insert / File"
    Kontext "OeffnenDlg"

    printlog " Check 'File dialog'"
    if OeffnenDlg.Exists( 2 ) then
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

                Kontext "ScriptSelector"
                if ( ScriptSelector.exists( 3 ) ) then
                    ScriptSelector.Close
                else
                    warnlog( "Dialog <ScriptSelector> is not available" )
                endif
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

    Auswaehlen.Click
    Kontext "OeffnenDlg"
    if ( OeffnenDlg.exists( 1 ) ) then
        OeffnenDlg.Cancel
    else
        warnlog( "Dialog <OeffnenDlg> is not available" )
    endif

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

    Kontext "Active"
    if Active.Exists( 1 ) then
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

