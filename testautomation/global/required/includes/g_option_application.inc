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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : update and resouce test for all option pages (at tools/options)
'*
'\***********************************************************************

global sErrMes as String
global iSectionNumber as Integer

sub tToolsOptionsTestFirst
    dim sPreviousApplication as string

    sPreviousApplication = gApplication

    ' To prevent users from running just a subsection from this file, without THIS
    ' subroutine, in every subsection the value of 'iSectionNumber' is checked.
    ' The test will exit with warnlog, if iSectionNumber is 0
    sErrMes = ""
    select case gApplication
    case "WRITER","MASTERDOCUMENT","HTML"   : iSectionNumber = 8
    case "IMPRESS","DRAW","MATH","CALC"     : iSectionNumber = 7
    end select

    ' these tabpages only exist in the respective application
    gApplication = "WRITER"
    iSectionNumber = 8
    Call tToolsOptionsTextDocument

    gApplication = "HTML"
    iSectionNumber = 8
    Call tToolsOptionsHTMLDocument

    gApplication = "CALC"
    iSectionNumber = 7
    Call tToolsOptionsSpreadsheet

    gApplication = "IMPRESS"
    iSectionNumber = 7
    Call tToolsOptionsPresentation

    gApplication = "DRAW"
    iSectionNumber = 7
    Call tToolsOptionsDrawing

    gApplication = "MATH"
    iSectionNumber = 7
    Call tToolsOptionsFormula

    gApplication = sPreviousApplication
end sub

'-------------------------------------------------------------------------

testcase tToolsOptionsTextdocument
    dim EchteAnzahl as Integer, i as Integer
    dim gefunden as Boolean

    PrintLog "- Tools/Options/Text document  ( Extras/Optionen/Textdokument )"
    if (0 = iSectionNumber) then
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
    '/// Create a new document
    if gApplication <> "BACKGROUND" then
        Call hNewDocument
    end if
    '/// <i>Tools</i> / <i>Options</i> / <i>Text document</i>
    ToolsOptions
    Kontext "Active"
    if Active.Exists (3) then
        if Active.GetText <> sErrMes then Warnlog "A messagebox exists => BUG : " + Active.GetText
        Active.OK
    end if
    ToPosInOptionlist ( 4 )
    EchteAnzahl = Optionsliste.GetItemCount - iSectionNumber
    if EchteAnzahl = 10 AND ( gAsianSup = TRUE OR gCTLSup = TRUE ) then
        Warnlog "There are only 10 pages in 'text document'-section, but asian support or CTL support are activated! => a section is missing?"
    else
        printlog "Asian support and CTL support are not activated ( on languages-page )!"
    end if
    if bDebugVersion = TRUE  then EchteAnzahl = EchteAnzahl - 1
    if gAsianSup     = FALSE then EchteAnzahl = EchteAnzahl + 1
    if gCTLSup       = FALSE then EchteAnzahl = EchteAnzahl + 1
    for i=1 to EchteAnzahl
        gefunden = FALSE
        Kontext "OptionenDlg"
        Optionsliste.TypeKeys "<Down>"
        if gAsianSup = FALSE AND gCTLSup = FALSE AND i=6 then i=i+2
        if gAsianSup = FALSE AND gCTLSup = TRUE  AND i=6 then i=i+1
        if gAsianSup = TRUE  AND gCTLSup = FALSE AND i=7 then i=i+1
        select case i
        case 1 : Kontext "TabLaden"
            '///+<ul><li>General</li>
            printlog "  - General / Allgemein"
            if TabLaden.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabLaden )
            end if
        case 2 : Kontext "TabInhalteWriter"
            '///+<li>View</li>
            printlog "  - View / Ansicht"
            if TabInhalteWriter.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabInhalteWriter )
            end if
        case 3 : Kontext "TabDirektCursor"
            '///+<li>Formating aids</li>
            printlog "  - Formating aids / Formatierungshilfen"
            if TabDirektCursor.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabDirektCursor )
            end if
        case 4 : Kontext "TabRaster"
            '///+<li>Grid</li>
            printlog "  - Grid / Raster"
            if TabRaster.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabRaster )
            end if
        case 5 : Kontext "TabGrundschriften"
            '///+<li>Default fonts</li>
            printlog "  - Default Fonts / Grundschriften"
            if TabGrundschriften.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabGrundschriften )
            end if
        case 6 : Kontext "TabGrundschriftenAsian"
            '///+<li>Default fonts asian</li>
            printlog "  - Default Fonts Asian / Grundschriften Asiatisch"
            if TabGrundschriftenAsian.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabGrundschriftenAsian )
            end if
        case 7 : Kontext "TabGrundschriftenCTL"
            '///+<li>Default fonts CTL</li>
            printlog "  - Default Fonts CTL / Grundschriften CTL"
            if TabGrundschriftenCTL.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabGrundschriftenCTL )
            end if
        case 8 : Kontext "TabDruckenWriter"
            '///+<li>Print</li>
            printlog "  - Print / Drucken"
            if TabDruckenWriter.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabDruckenWriter )
            end if
        case 9 : Kontext "TabTabelleOptionen"
            '///+<li>Table</li>
            printlog "  - Table / Tabellen"
            if TabTabelleOptionen.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabTabelleOptionen )
            end if
        case 10: Kontext "TabAenderungenWriter"
            '///+<li>Changes</li>
            printlog "  - Changes / Aenderungen"
            if TabAenderungenWriter.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabAenderungenWriter )
            end if
        case 11: Kontext "TabCompatibility"
            '///+<li>Compatibility</li>
            printlog "  - Compatibility"
            if TabCompatibility.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabCompatibility )
            end if
        case 12: Kontext "TabAutoCaption"
            '///+<li>AutoCaption</li>
            printlog "  - AutoCaption"
            if TabAutoCaption.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabAutoCaption )
            end if
        case 13: Kontext "TabMailMergeEMail"
            '///+<li>Mail Merge E-mail</li></ul>
            printlog "  - Mail Merge E-mail"
            if TabMailMergeEMail.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabMailMergeEMail )
                ServerAuthentication.click
                kontext "ServerAuthentication"
                Call DialogTest ( ServerAuthentication )
                ServerAuthentication.cancel
                Kontext "TabMailMergeEMail"
                TestSettings.click

                ' in case java is not enabled
                sleep( 2 )
                try
                    Kontext "Messagebox"
                    if ( MessageBox.exists()) then
                        sleep(2)
                        warnlog(Messagebox.GetText())
                        try
                            Messagebox.Cancel()
                            sleep( 2 )
                        catch
                            Messagebox.OK() ' if "OK" works, we are in an undefined state
                            warnlog( "" )
                        endcatch
                    end if
                catch
                    warnlog( "A messagebox was displayed but coul not be closed." )
                endcatch

                Kontext "TestaccountSettings"
                Call DialogTest ( TestaccountSettings )
                TestaccountSettings.cancel
            end if
        end select
        if gefunden=FALSE then
            Warnlog "Error with the " + i + ".page, perhaps this page is deleted or change the place!"
            EchteAnzahl = EchteAnzahl - 1
            if EchteAnzahl < 12 then
                i=13
            else
                i=1-1
            end if
        end if
    next i
    '/// Close the options dialog
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
    '/// Close the document
    if gApplication <> "BACKGROUND" then
        Call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsHTMLDocument
    dim EchteAnzahl as Integer, i as Integer
    dim gefunden as Boolean

    PrintLog "- Tools/Options/HTML-Document  ( Extras/Optionen/HTML-Dokument )"
    if (0 = iSectionNumber) then
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
    '/// Create a new document
    if gApplication <> "BACKGROUND" then
        Call hNewDocument
    end if
    '/// <i>Tools</i> / <i>options</i> / <i>HTML Document</i>
    ToolsOptions
    ToPosInOptionlist ( 5 )
    i = 6
    if bDebugVersion = TRUE  then i = i + 1  ' here exists the special debug-tabpage, too
    EchteAnzahl = OptionTabPageZaehler (i)
    if bDebugVersion = TRUE  then EchteAnzahl = EchteAnzahl - 1
    for i=1 to EchteAnzahl
        gefunden = FALSE
        Kontext "OptionenDlg"
        Optionsliste.TypeKeys "<Down>"
        select case i
        case 1 : Kontext "TabInhalteHTML"
            '/// View
            printlog "  - View / Ansicht"
            if TabInhalteHTML.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabInhalteHTML )
            end if
        case 2 : Kontext "TabDirektCursor"
            '/// Formating aids
            printlog "  - Formatting aids / Formatierungshilfen"
            if TabDirektCursor.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabDirektCursor )
            end if
        case 3 : Kontext "TabRaster"
            '/// Grid
            printlog "  - Grid / Raster"
            if TabRaster.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabRaster )
            end if
        case 4 : Kontext "TabDruckenHTML"
            '/// Print
            printlog "  - Print / Drucken"
            if TabDruckenHTML.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabDruckenHTML )
            end if
        case 5 : Kontext "TabTabelleOptionen"
            '/// Table
            printlog "  - Table / Tabelle"
            if TabTabelleOptionen.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabTabelleOptionen )
            end if
        case 6 : Kontext "TabHintergrund"
            '/// Background
            printlog "  - Background / Hintergrund"
            Sleep 1
            if TabHintergrund.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabHintergrund )
            end if
        end select
        if gefunden=FALSE then
            Warnlog "Error with the " + i + ".page, perhaps this page is deleted or change the place!"
            EchteAnzahl = EchteAnzahl - 1
            if EchteAnzahl < 6 then
                i=11
            else
                i=1-1
            end if
        end if
    next i
    '/// Close the options dialog
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
    '/// Close the document
    if gApplication <> "BACKGROUND" then
        Call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsSpreadsheet
    dim EchteAnzahl as Integer, i as Integer
    dim gefunden as Boolean

    PrintLog "- Tools/Options/Spreadsheet  ( Extras/Optionen/Tabellendokument )"
    if (0 = iSectionNumber) then
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
    '/// Create a new document
    if gApplication <> "BACKGROUND" then
        Call hNewDocument
    end if
    '/// <i>Tools</i> / <i>Options</i> / <i>Spreadsheet</i>
    ToolsOptions
    ToPosInOptionlist ( 4 )
    EchteAnzahl = OptionTabPageZaehler ( 7, FALSE )
    for i=1 to EchteAnzahl
        gefunden = FALSE
        Kontext "OptionenDlg"
        Optionsliste.TypeKeys "<Down>"
        select case i
        case 1 : Kontext "TabLayoutCalc"
            '/// General
            printlog "  - General / Allgemein"
            if TabLayoutCalc.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabLayoutCalc )
            end if
        case 2 : Kontext "TabInhaltCalc"
            '/// View
            printlog "  - View / Ansicht"
            if TabInhaltCalc.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabInhaltCalc )
            end if
        case 3 : Kontext "TabBerechnen"
            '/// Calculate
            printlog "  - Calculate / Berechnen"
            if TabBerechnen.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabBerechnen )
            end if
        case 4 : Kontext "TabSortierlisten"
            '/// Sort lists
            printlog "  - Sort Lists / Sortierliste"
            if TabSortierlisten.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabSortierlisten )
            end if
        case 5 : Kontext "TabAenderungenCalc"
            '/// Changes
            printlog "  - Changes / Aenderungen"
            if TabAenderungenCalc.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabAenderungenCalc )
            end if
        case 6 : Kontext "TabRaster"
            '/// Grid
            printlog "  - Grid / Raster"
            if TabRaster.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabRaster )
            end if
        case 7 : Kontext "TabPrintCalcOptions"
            '/// Print
            printlog "  - Print / Drucken"
            if TabPrintCalcOptions.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabPrintCalcOptions )
            end if
        end select
        if gefunden=FALSE then
            Warnlog "Error with the " + i + ".page, perhaps this page is deleted or change the place!"
            EchteAnzahl = EchteAnzahl - 1
            if EchteAnzahl < 7 then
                i=11
            else
                i=1-1
            end if
        end if
    next i
    '/// Close the options dialog
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
    '/// Close the document
    if gApplication <> "BACKGROUND" then
        Call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsPresentation
    dim EchteAnzahl as Integer, i as Integer
    dim gefunden as Boolean
    dim iTemp as integer

    PrintLog "- Tools/Options/Presentation  ( Extras/Optionen/Praesentation )"
    if (0 = iSectionNumber) then
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
    '/// Create a new document
    if gApplication <> "BACKGROUND" then
        Call hNewDocument
    end if
    kontext "DocumentImpress"
    if DocumentImpress.exists(5) then
        printlog "  - An application of type Presentation is available."
        '--- workaround #i48383#
        DocumentImpress.MouseDown (10,10)
        DocumentImpress.MouseUp (10,10)
    else
        qaErrorLog "  - No application of type Presentation is available"
    end if
    '/// <i>Tools</i> / <i>Options</i> / <i>Presentation</i>
    ToolsOptions
    ToPosInOptionlist ( 4 )
    EchteAnzahl = OptionTabPageZaehler ( 4, FALSE )
    for i=1 to EchteAnzahl
        gefunden = FALSE
        Kontext "OptionenDlg"
        Optionsliste.TypeKeys "<Down>"
        select case i
        case 1 : Kontext "TabSonstigesDraw"
            '/// General
            printlog "  - General / Allgemein"
            if TabSonstigesDraw.exists then
                if TabSonstigesDraw.IsVisible then
                    gefunden = TRUE
                    Call DialogTest ( TabSonstigesDraw )
                end if
            else
                ' Trying to get root cause on some systems it seems the backing window is used. #i48383#
                Kontext "OptionenDlg"
                OptionenDlg.SnapShot convertPath(gOfficePath + "user/work/"+gApplication+".bmp")
                qaErrorLog "#i48383# Trying to get root cause for wrong window; check picture: " + convertPath(gOfficePath + "user/work/"+gApplication+".bmp")
                for iTemp = 1 to Optionsliste.getItemCount
                    qaErrorLog "" + iTemp + ": '" + Optionsliste.getItemText(iTemp) + "'"
                next iTemp
                gefunden = TRUE
            end if
        case 2 : Kontext "TabInhalteDraw"
            '/// View
            printlog "  - View / Ansicht"
            if TabInhalteDraw.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabInhalteDraw )
            end if
        case 3 : Kontext "TabRaster"
            '/// Grid
            printlog "  - Grid / Raster"
            if TabRaster.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabRaster )
            end if
        case 4 : Kontext "TabDruckenDraw"
            '/// Print
            printlog "  - Print / Drucken"
            if TabDruckenDraw.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabDruckenDraw )
            end if
        end select
        if gefunden=FALSE then
            Warnlog "Error with the " + i + ".page, perhaps this page is deleted or change the place!"
            EchteAnzahl = EchteAnzahl - 1
            if EchteAnzahl < 4 then
                i=11
            else
                i=1-1
            end if
        end if
    next i
    '/// Close the options dialog
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
    '/// Close the document
    if gApplication <> "BACKGROUND" then
        Call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsDrawing
    Dim EchteAnzahl as Integer, i as Integer
    Dim gefunden as Boolean

    PrintLog "- Tools/Options/Drawing  ( Extras/Optionen/Zeichnung )"
    if (0 = iSectionNumber) then
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    end if
    '/// Create a new document
    if gApplication <> "BACKGROUND" then
        Call hNewDocument
    end if
    Kontext "DocumentDraw"
    if DocumentDraw.exists(5) then
        printlog "  - An application of type Draw is available."
        '--- workaround #i48383#
        DocumentDraw.MouseDown (10,10)
        DocumentDraw.MouseUp (10,10)
    else
        qaErrorLog "  - No application of type Draw is available"
    end if
    '/// <i>Tools</i> / <i>Options</i> / <i>Drawing</i>
    ToolsOptions
    ToPosInOptionlist ( 4 )
    EchteAnzahl = OptionTabPageZaehler ( 4, FALSE )
    for i=1 to EchteAnzahl
        gefunden = FALSE
        Kontext "OptionenDlg"
        Optionsliste.TypeKeys "<Down>"
        select case i
        case 1 : Kontext "TabSonstigesDraw"
            '/// General
            printlog "  - General / Allgemein"
            if TabSonstigesDraw.exists then
                if TabSonstigesDraw.IsVisible then
                    gefunden = TRUE
                    Call DialogTest ( TabSonstigesDraw )
                end if
            else
                ' Trying to get root cause on some systems it seems the backing window is used. #i48383#
                Kontext "OptionenDlg"
                qaErrorLog "#i48383# wrong focus on opening options"
                gefunden = TRUE
            end if
        case 2 : Kontext "TabInhalteDraw"
            '/// View
            printlog "  - View / Ansicht"
            if TabInhalteDraw.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabInhalteDraw )
            end if
        case 3 : Kontext "Active"
            if Active.Exists then
                Warnlog "A messagebox exists => BUG  : " + Active.GetText
                try
                    Active.No
                catch
                    Active.OK
                endcatch
            end if
            Kontext "TabRaster"
            '/// Grid
            printlog "  - Grid / Raster"
            if TabRaster.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabRaster )
            end if
        case 4 : Kontext "TabDruckenDraw"
            '/// Print
            printlog "  - Print / Drucken"
            if TabDruckenDraw.IsVisible then
                gefunden = TRUE
                Call DialogTest ( TabDruckenDraw )
            end if
        end select
        if gefunden=FALSE then
            Warnlog "Error with the " + i + ".page, perhaps this page is deleted or change the place!"
            EchteAnzahl = EchteAnzahl - 1
            if EchteAnzahl < 4 then
                i=11
            else
                i=1-1
            end if
        end if
    next i
    '/// Close the options dialog
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
    '/// Close the document
    if gApplication <> "BACKGROUND" then
        Call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tToolsOptionsFormula
    dim EchteAnzahl as Integer, i as Integer

    PrintLog "- Tools/Options/Formula  ( Extras/Optionen/Formel )"
    if (0 = iSectionNumber) then
        warnlog "You can't run this testcase seperated! Ending now! Please run it via: tToolsOptionsTest"
        goto endsub
    endif
    '/// Create a new document
    if gApplication <> "BACKGROUND" then
        Call hNewDocument
    end if
    '/// <i>Tools</i> / <i>Options</i> / <i>Formula</i>
    ToolsOptions
    ToPosInOptionlist ( 4 )
    EchteAnzahl = OptionTabPageZaehler ( 1, FALSE )
    Kontext "OptionenDlg"
    '/// Print
    Optionsliste.TypeKeys "<Down>"
    printlog "  - Print / Drucken"
    Kontext "TabDruckenMath"
    Call DialogTest ( TabDruckenMath )
    '/// Close the options dialog
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
    '/// Close the document
    if gApplication <> "BACKGROUND" then
        Call hCloseDocument
    end if
endcase

