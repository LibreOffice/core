'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
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
'/******************************************************************************
'*
'* owner : helge.delfs@oracle.com
'*
'* short description : Writer-Tools
'*
'\******************************************************************************

sub wDokSchreiben ( OutputText , optional iLoop as Integer )
    Dim i as integer
    If lcase(OutputText) = "<mod1 a>" then
        If iSprache = 34 then OutputText = "<Mod1 E>"
    end if
    If IsMissing(iLoop) = True then iLoop = 1
    For i = 1 to iLoop
        Select Case Ucase(gApplication)
        Case "WRITER"
            Kontext "DocumentWriter"
            DocumentWriter.TypeKeys OutputText

        Case "MASTERDOCUMENT"
            Kontext "DocumentMasterDoc"
            DocumentMasterDoc.TypeKeys OutputText

        Case "HTML"
            Kontext "DocumentWriterWeb"
            DocumentWriterWeb.TypeKeys OutputText

        case "DRAW"
            Call hTextrahmenErstellen (OutputText,35,35,50,20)
        case "IMPRESS"
            Call hTextrahmenErstellen (OutputText,35,35,50,20)

        end select
        wait 500
    next i
end sub

' -----------------------------------------------------------------------

sub wTypeKeys ( OutputText , optional iLoop as Integer )

    Dim i as integer
    If lcase(OutputText) = "<mod1 a>" then
        If iSprache = 34 then OutputText = "<Mod1 E>"
    end if
    If IsMissing(iLoop) = True then iLoop = 1
    For i = 1 to iLoop
        Select Case Ucase(gApplication)
        Case "WRITER"
            Kontext "DocumentWriter"
            DocumentWriter.TypeKeys OutputText

        Case "MASTERDOCUMENT"
            Kontext "DocumentMasterDoc"
            DocumentMasterDoc.TypeKeys OutputText

        Case "HTML"
            Kontext "DocumentWriterWeb"
            DocumentWriterWeb.TypeKeys OutputText

        case "DRAW"
            Call hTextrahmenErstellen (OutputText,35,35,50,20)
        case "IMPRESS"
            Call hTextrahmenErstellen (OutputText,35,35,50,20)
        end select
        wait 500
    next i

end sub

' -----------------------------------------------------------------------

function wBlindtextEinfuegen() as boolean
    '/// This functions inserts a Dummytext and returns true if it was inserted
    Dim BlindText as string, bTemp as boolean
    bTemp = False
    select case iSprache
    case 01    : BlindText = "DT<F3>"
    case 03    : BlindText = "TE<F3>"
    case 07    : BlindText = "DT<F3>"
    case 31    : BlindText = "BT<F3>"
    case 34    : BlindText = "TE<F3>"
    case 33    : BlindText = "TEX<F3>"
    case 39    : BlindText = "TE<F3>"
    case 45    : BlindText = "ET<F3>"
    case 46    : BlindText = "BT<F3>"
    case 48    : BlindText = "TW<F3>"
    case 49    : BlindText = "BT<F3>"
    case 55    : BlindText = "TP<F3>"
    case 81    : BlindText = "DT<F3>"
    case 82    : BlindText = "BT<F3>"
    case 86    : BlindText = "DT<F3>"
    case 88    : BlindText = "BT<F3>"
    case else : QAErrorlog "No Shortcut for Dummytext available!"
    end select

    if BlindText > "" then
        Call wTypeKeys BlindText
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                Warnlog Active.Gettext
                Active.Ok
            else
                bTemp = True
            end if
        else
            bTemp = True
        end if
    end if

    wBlindtextEinfuegen = bTemp

end function

' -----------------------------------------------------------------------

sub wZeichenobjektEinfuegen ( Objekt$, xStart%, yStart%, xEnde%, yEnde%, optional sCloseToolbar as boolean, optional bLeaveSelected as boolean )

    Kontext "DrawBar"
    if Not DrawBar.Exists then Call hToolbarSelect("Drawing", true)
    Sleep 2
    if IsMissing(bLeaveSelected) then
        bLeaveSelected = false
    endif

    Objekt = UCase( Objekt )
    select case Objekt
    case "RECHTECK" : Rechteck.Click
    case "LINIE"     : Linie.Click
    case "ELLIPSE"  : Ellipse.Click
    case "TEXT"      : Textobjekt.Click
    case "VTEXT"     :
        try
            VerticalText.Click
        catch
            if gAsianSup = True then
                Warnlog "Unable to insert Vertical Text! Asian Support in Options activated ?"
            end if
        endcatch
    case "LAUFTEXT" : Lauftext.Click
    case "LEGENDE"  : Legende.Click
    case "VLEGENDE" :
        try
            VerticalCallout.Click
        catch
            Warnlog "Unable to insert Vertical Callout! Asian Support in Options activated ?"
        endcatch
    end select
    Sleep (2)

    Select Case gApplication
    Case "WRITER"
        Kontext "DocumentWriter"
        DocumentWriter.MouseDown xStart%, yStart%
        DocumentWriter.MouseMove xEnde%, yEnde%
        DocumentWriter.MouseUp xEnde%, yEnde%
        wait 100
        Kontext "Drawbar"
        if IsMissing(sCloseToolbar) then
            Drawbar.Close
        else
            if sCloseToolbar = true then Drawbar.Close
        end if
        if Objekt <> "TEXT" and Objekt <> "VTEXT" then
            Kontext "DocumentWriter"
            if bLeaveSelected = false then
                Call gMouseClick (10, 10)
                wait 100
            endif
        end if

    Case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.MouseDown xStart%, yStart%
        DocumentMasterDoc.MouseMove xEnde%, yEnde%
        DocumentMasterDoc.MouseUp xEnde%, yEnde%
        wait 100
        Kontext "Drawbar"
        if IsMissing(sCloseToolbar) then
            Drawbar.Close
        else
            if sCloseToolbar = true then Drawbar.Close
        end if
        if Objekt <> "TEXT" and Objekt <> "VTEXT" then
            Kontext "DocumentMasterDoc"
            if bLeaveSelected = false then
                Call gMouseClick (10, 10)
                wait 100
            endif
        end if
    end select
    sleep (1)
end sub

' -----------------------------------------------------------------------

sub wObjektSelektieren ( xStart%, yStart%, xEnde%, yEnde% )
    Call hToolbarSelect("DRAWING",true)
    if Auswahl.Getstate(2) = 0 then
        Auswahl.Click
    endif
    Select Case gApplication
    Case "WRITER"
        Kontext "DocumentWriter"
        DocumentWriter.MouseDown xStart%, yStart%
        wait 500
        DocumentWriter.MouseMove xEnde%, yEnde%
        DocumentWriter.MouseUp xEnde%, yEnde%
    Case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.MouseDown xStart%, yStart%
        wait 500
        DocumentMasterDoc.MouseMove xEnde%, yEnde%
        DocumentMasterDoc.MouseUp xEnde%, yEnde%
    end select
    wait 200
end sub

' -----------------------------------------------------------------------

sub wFindSelectObjectBelow ( xStart%, yStart%, xEnde%, yEnde% )
    WL_TB_ZF_Auswahl
    Select Case gApplication
    Case "WRITER"
        Kontext "DocumentWriter"
        DocumentWriter.MouseMove xStart%, yStart%
        DocumentWriter.MouseDown xStart%, yStart%
        DocumentWriter.MouseUp xStart%, yStart%

        do while (Mousepointer<>"Normalpointer")
            DocumentWriter.MouseMove xEnde%, yEnde%
            DocumentWriter.MouseDown xEnde%, yEnde%
            DocumentWriter.MouseUp
        loop

        i = 1
        while ((getMouseStyle = 0) AND (i<80))
            sleep 1
            inc (i)
            printlog getMouseStyle
            if (getMouseStyle <> 0) then i = 80
        wend

    Case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.MouseDown xStart%, yStart%
        DocumentMasterDoc.MouseMove xEnde%, yEnde%
        DocumentMasterDoc.MouseUp xEnde%, yEnde%
    end select
    wait 200
end sub

' -----------------------------------------------------------------------

function WortAusWoerterbuchLoeschen ( Aufnahme$ ) as Boolean
    Dim i as integer : Dim AnzahlBuecher as string
    Dim j as integer

    ToolsOptions
    Call hToolsOptions("LanguageSettings","WritingAids")
    WortAusWoerterbuchLoeschen = FALSE
    for j = 1 to Benutzerwoerterbuch.GetItemCount
        try
            Bearbeiten.Click
            exit for
        catch
            Benutzerwoerterbuch.Select j
        endcatch
    next j
    Kontext "BenutzerwoerterbuchBearbeiten"
    AnzahlBuecher = Buch.GetItemCount

    for i= 1 to AnzahlBuecher
        Buch.Select i
        printlog "      - search word in module: " + Buch.GetSelText
        Kontext "BenutzerwoerterbuchBearbeiten"
        Inhalt.SetText Aufnahme$
        Sleep 1
        if Entfernen.IsEnabled then
            Entfernen.Click
            WortAusWoerterbuchLoeschen = TRUE
            i = AnzahlBuecher + 1
        end if
    next i
    if WortAusWoerterbuchLoeschen = FALSE then Warnlog "The added word has not been found in modules"
    BenutzerwoerterbuchBearbeiten.Close
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
end function

' -----------------------------------------------------------------------

function wIgnorierenlisteLoeschen
    Dim i as integer : Dim j as integer : Dim AlleBuecher as integer

    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","WRITINGAIDS")
    Sleep 3
    for j = 1 to Benutzerwoerterbuch.GetItemCount
        try
            Bearbeiten.Click
            exit for
        catch
            Benutzerwoerterbuch.Select j
        endcatch
    next j
    Kontext "BenutzerwoerterbuchBearbeiten"
    sleep 1
    AlleBuecher = Buch.GetItemCount
    for i = 1 to AlleBuecher
        Buch.Select i
        if Left$(Buch.GetSelText,13)="IgnoreAllList" then
            sleep 2
            for j = 1 to 100
                if Loeschen.IsEnabled then
                    Loeschen.Click
                    sleep 1
                else
                    Kontext "Active"
                    if Active.Exists then
                        Warnlog Active.Gettext
                        Active.Ok
                    end if
                    Kontext "BenutzerwoerterbuchBearbeiten"
                    BenutzerwoerterbuchBearbeiten.Cancel
                    sleep 1
                    j=101
                end if
                wait 500
            next j
            i=AlleBuecher + 1
        end if
    next i
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
end function

' -----------------------------------------------------------------------

sub hLeisteUmschalten(welche as integer)
    Sleep 2
    Select Case welche
    case 1:
        try                                             ' Textobjectbar
            Kontext "NumObjectbar"
            NumObjectbar.SetNextToolbox
        catch
        endcatch
        Sleep 1
        if gApplication = "HTML" then
            Kontext "TextObjectbar"
        else
            Kontext "TextObjectbar"
        end if
        Sleep 1
        try
            Fett.Click    ' Sonst sind Comboboxen fȭr Testtool nicht sichtbar
            Sleep 1
            Fett.Click
        catch
            Warnlog "Error on switching the function bar!"
        endcatch
        Sleep 1

    case 2:
        Select Case gApplication
            ' Numbering-Objectbar
        Case "HTML"
            Kontext "TextObjectbar"
            try
                TextObjectbar.SetNextToolbox
            catch
            endcatch

        Case else
            Kontext "TextObjectbar"
            try
                TextObjectbar.SetNextToolbox
            catch
            endcatch
        end select
        Sleep 1
        Kontext "NumObjectbar"
        try
            Hinunterstufen.Click
        catch
            try
                Hochstufen.Click
            catch
                Warnlog "Error on switching the function bar!"
            endcatch
        endcatch
        Sleep 2
    end select
end sub

' -----------------------------------------------------------------------

sub hGrafikleisteUmschalten(welche as integer)
    Select Case welche
    case 1:
        try
            Kontext "GraphicObjectbar"
            Wait 500
            GraphicObjectbar.SetNextToolBox
        catch
        endcatch
        Kontext "FrameObjectbar"
        Sleep 1

    case 2:
        try
            Kontext "FrameObjectbar"
            Wait 500
            FrameObjectbar.SetNextToolBox
        catch
        endcatch
        Kontext "GraphicObjectbar"
        Sleep 1
    end select
end sub

' -----------------------------------------------------------------------

sub wEinfuegenHyperlink(welcheSeite as integer)
    Kontext "Hyperlink"
    Auswahl.MouseDown 2,2
    Auswahl.MouseUp 2,2
    wait 500
    Auswahl.TypeKeys "<Down>",1
    wait 500
    Auswahl.TypeKeys "<Up>",4
    Sleep 1

    Select case welcheSeite
    case 1:
        Kontext "TabHyperlinkInternet"

    case 2:
        Auswahl.TypeKeys "<Down>",1
        Kontext "TabHyperlinkMailUndNews"

    case 3:
        Auswahl.TypeKeys "<Down>",2
        Kontext "TabHyperlinkDokument"

    case 4:
        Auswahl.TypeKeys "<Down>",3
        Kontext "TabHyperlinkNeuesDokument"

    end select
end sub

' -----------------------------------------------------------------------

sub wNavigatorAuswahl(Gruppe as integer, Eintrag as integer,optional LeaveFocus as boolean)
    dim j as integer, WelcherEintrag as integer
    Dim MinEntrys as integer

    If IsMissing(LeaveFocus) = True then LeaveFocus = False

    Select Case Ucase(gApplication)
    Case "CALC"
        Kontext "NavigatorCalc"
        if NavigatorCalc.NotExists then ViewNavigator
        wait 500
        Kontext "NavigatorCalc"
        MinEntrys = 8
    Case "WRITER", "HTML", "HTML"
        Kontext "NavigatorWriter"
        if NavigatorWriter.NotExists then ViewNavigator
        wait 500
        Kontext "NavigatorWriter"
        MinEntrys = 11
    Case "MASTERDOCUMENT"
        Kontext "NavigatorGlobalDoc"
        if NavigatorGlobalDoc.NotExists then ViewNavigator
        wait 500
        Kontext "NavigatorGlobalDoc"
        if Liste.IsVisible then
            Kontext "GlobaldokumentToolbox"
            Umschalten.Click
            Kontext "NavigatorWriter"
        end if
        MinEntrys = 11
    Case Else ' for all others!!!!
        Warnlog "Not supported application!"
        exit sub
    end select

    Sleep 2

    Select Case gApplication
    Case "CALC"
        if (Liste.GetItemCount < MinEntrys) then Umschalten.Click
        if Liste.GetItemCount >MinEntrys then
            For j= 1 to 9
                Liste.Select j
                Liste.TypeKeys "-"
            next j
        end if

        Liste.Select Gruppe
        if Eintrag >0 then
            WelcherEintrag = Gruppe + Eintrag
            Liste.TypeKeys "+"
            Liste.Select WelcherEintrag
            Liste.TypeKeys "<Return>"
        end if

    Case "WRITER","HTML"
        if (Auswahlliste.GetItemCount < MinEntrys) then
            try
                Inhaltsansicht.Click
            catch
            endcatch
        end if
        if Auswahlliste.GetItemCount >MinEntrys then
            For j= 1 to 12
                Auswahlliste.Select j
                Auswahlliste.TypeKeys "-"
            next j
        end if

        Auswahlliste.Select Gruppe
        if Eintrag >0 then
            WelcherEintrag = Gruppe + Eintrag
            Auswahlliste.TypeKeys "+"
            Auswahlliste.Select WelcherEintrag
            Auswahlliste.TypeKeys "<Return>"
        end if

    Case "MASTERDOCUMENT"
        if Liste.IsVisible = True then
            Umschalten.Click
        end if

        if Auswahlliste.GetItemCount >MinEntrys then
            For j= 1 to 12
                Auswahlliste.Select j
                Auswahlliste.TypeKeys "-"
            next j
        end if

        Auswahlliste.Select Gruppe
        if Eintrag >0 then
            WelcherEintrag = Gruppe + Eintrag
            Auswahlliste.TypeKeys "+"
            Auswahlliste.Select WelcherEintrag
            Auswahlliste.TypeKeys "<Return>"
        end if
    end select

    if LeaveFocus = False then
        Select Case Ucase(gApplication)
        Case "CALC"
            Kontext "DocumentCalc"
        Case "HTML", "HTML"
            Kontext "DocumentWriter"
        Case "WRITER"
            Kontext "DocumentWriter"
        Case "MASTERDOCUMENT"
            Kontext "DocumentMasterDoc"
        end select
    end if
end sub

' ---------------------------------------------------------------

function wNavigatorClose
    '/// Close Navigator
    Kontext "Navigator"
    if Navigator.Exists(2) then
        Navigator.Close
        WaitSlot()
    end if
end function

' ---------------------------------------------------------------

sub wClearDocument
    Dim i as integer : Dim j as integer
    '/// Remove all Text, Drawing-objects or Objects from document ///
    Call wTypeKeys "<PageUp><Home>"
    Call wTypeKeys "<Backspace>",5
    DocumentWriter.MouseDown ( 30,30 )
    DocumentWriter.MouseMove ( 75,60 )
    wait 500
    DocumentWriter.MouseUp ( 75, 60 )
    Sleep 1
    Call wTypeKeys "<Delete>"
    Sleep 3

    Kontext "NavigatorWriter"
    if Not NavigatorWriter.Exists then ViewNavigator
    wait 500

    Kontext "NavigatorWriter"
    if Auswahlliste.GetItemCount <11 then Inhaltsansicht.Click  ' Grundzustand

    if Auswahlliste.GetItemCount >11 then
        For j= 1 to 12
            Auswahlliste.Select j
            Auswahlliste.TypeKeys "-"
        next j
    end if
    wait 500
    for i = 1 to 12
        Auswahlliste.Select i
        Auswahlliste.TypeKeys "+"
        if Auswahlliste.GetItemCount > 11 then
            for j = 1 to 5 ' maximal 5 Elemente in einer Gruppe
                Auswahlliste.TypeKeys "<Down>"
                Auswahlliste.TypeKeys "<Delete>"
                wait 500
                if Auswahlliste.GetItemCount > 11 then Auswahlliste.Select i
            next j
        end if
    next i
    if j= 5 then Warnlog "Try another way to clear page !"

    Kontext "Navigator"
    try
        Navigator.Close
    catch
        Errorlog
        ViewNavigator
    endcatch

    try
        Call wTypeKeys "<Mod1 A>"
    catch
    endcatch
    Call wTypeKeys "<Delete>"
    DocumentWriter.MouseDown ( 50,10 )
    DocumentWriter.MouseUp ( 50,10 )
    Sleep 2
    Call wTypeKeys "<Home><PageUp>"
    Sleep 5
end sub

' ---------------------------------------------------------------

sub wMarkObjects(delete as boolean)
    ' The drawing objects has to be selected before deleting them
    Select Case gApplication
    Case "WRITER"
        Kontext "DocumentWriter"
    Case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"
    Case "HTML"
        Kontext "DocumentWriterWeb"
    end select

    Call gMouseClick 50,50
    Call wTypeKeys "<PageUp><Home>"
    Call wTypeKeys "<Backspace>",5
    Call gMouseClick 50,50
    Select Case gApplication
    Case "WRITER"
        DocumentWriter.TypeKeys "<Shift F4>"

    Case "MASTERDOCUMENT"
        DocumentMasterDoc.TypeKeys "<Shift F4>"

    Case "HTML"
        DocumentWriterWeb.TypeKeys "<Shift F4>"
    end select

    Sleep 1
    if delete = TRUE then
        Call wTypeKeys "<Delete>"
        Sleep 3
    end if
end sub

' -----------------------------------------------------------------------

sub hExportUmstellung ( WasDenn as Boolean )
    if Ucase(gApplication) = "HTML" then
        ToolsOptions
        Call hToolsOptions("LoadSave","HTMLCompatibility")
        if WasDenn = TRUE then
            'Select Case iSprache
            ' Case 81 : Export.Select "Microsoft Internet Explorer 4.0"
            ' Case else : Export.Select "MS Internet Explorer 4.0"
            'end select
            try
                Export.Select "Microsoft Internet Explorer"
            catch
                Warnlog "Unable to choose browser for export !"
            endcatch
        else
            Export.Select "Netscape Navigator"
        end if
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK
    end if
end sub

' -----------------------------------------------------------------------

sub wSetMacroSecurityLevel ( wLevel as integer ) as boolean
    ToolsOptions
    Call hToolsOptions("StarOffice","Security")
    wSetMacroSecurityLevel = False
    try
        MacroSecurity.Click
    catch
        Warnlog "Unable to set macro security level (Button 'Macro Security' not found)"
        Goto EarlyExit
    endcatch

    Kontext
    Active.SetPage TabSecurityLevel
    Kontext "TabSecurityLevel"

    Select Case wLevel
    Case 1:     VeryHigh.Check
    Case 2:     High.Check
    Case 3:     Medium.Check
    Case 4:     Low.Check
    Case else:  Warnlog "Wrong level number entered. Unknown"
        goto EarlyExit
    end select
    TabSecurityLevel.Ok
    wSetMacroSecurityLevel = True

    EarlyExit:
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

end sub

' -----------------------------------------------------------------------

sub wInsertFrame
    InsertFrame
    Kontext
    Active.SetPage TabType
    Kontext "TabType"
    TabType.OK
    Sleep 3
end sub

' -----------------------------------------------------------------------

sub wRectangleToClipboard
    Dim tempApplication as string
    tempApplication = gApplication
    gApplication = "DRAW"
    Call hNewDocument
    Call hRechteckErstellen ( 45, 45, 60, 60 )
    ' select the drawn object
    try
        EditSelectAll
    catch
        warnlog "Unable to select object. Maybe not drawn!"
    endcatch
    ' copy the selected object
    try
        EditCopy
    catch
        Warnlog "Unable to copy object. Maybe not selected!"
    endcatch

    Call hCloseDocument
    gApplication = tempApplication
    Select Case gApplication
    Case "WRITER"
        Kontext "DocumentWriter"
    Case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"
    end select
    Call gMouseClick 10,10
end sub

' -----------------------------------------------------------------------

sub wRectangleToClipboardHTML
    gApplication = "DRAW"
    Call hNewDocument
    Call hRechteckErstellen ( 45, 45, 60, 60 )
    EditSelectAll
    EditCopy
    Call hCloseDocument
    gApplication = "HTML"
    Kontext "DocumentWriterWeb"
    Call gMouseClick
end sub

' -----------------------------------------------------------------------

function MachMirDenEintrag(WoDenn as integer,WieLang as integer, optional Welches as string)
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Down>",WoDenn
    Call wTypeKeys "<Mod1 Shift Right>" , WieLang
    Kontext "VerzeichniseintragEinfuegen"
    if Welches > "" then
        Select case Welches
        case "Inhalt" : Verzeichnis.Select 1
        case "Stich"  : Verzeichnis.Select 2
        case "Benutz" : Verzeichnis.Select 3
        end select
    end if
    Verzeichniseintrag.TypeKeys "<Up>"
    Wait 500
    EintragOk.Click
end function

' -----------------------------------------------------------------------

sub wOptionsUndo ( sOption$ )
    Dim i as integer
    Dim sTempSeparator as string
    Dim sTempUnit as integer

    Printlog "  - Initial state"

    Call hNewDocument

    Kontext "ExtrasOptionenDlg"
    if Not ExtrasOptionenDlg.Exists then
        ToolsOptions
    end if

    Call hToolsOptions("WRITER","General")
    if Instr(Tabulatorenabstand.Gettext, ",") then
        sTempSeparator = ","
    else
        sTempSeparator = "."
    end if
    sTempUnit = Masseinheit.GetSelIndex

    if sOption$ = "Laden" OR sOption$ = "All" then
        ' Update
        AufNachfrage.Check
        Feldbefehle.Check
        Diagramme.Uncheck
        ' Settings
        Select Case sTempUnit
        Case 1:     'Millimeter
            Tabulatorenabstand.Settext "12" & sTempSeparator & "51"
        Case 2:     ' Centimeter
            Tabulatorenabstand.Settext "1" & sTempSeparator & "25"
        Case 3:     ' Inch
            Tabulatorenabstand.Settext "0" & sTempSeparator & "49"
        Case 4:     ' Pica
            Tabulatorenabstand.Settext "2" & sTempSeparator & "95"
        Case 5:     ' Point
            Tabulatorenabstand.Settext "35" & sTempSeparator & "5"
        end select
    end if

    Call hToolsOptions("WRITER","View")
    if sOption$ = "Inhalte" OR sOption$ = "All" then
        ' Guides
        Hilfslinien.Uncheck
        FarbigeHandles.Check
        GrosseHandles.Uncheck
        ' View
        HorizontaleBildlaufleiste.Check
        VertikaleBildlaufleiste.Check
        Lineal.Check
        HorizontalesLineal.Check
        VertikalesLineal.Check
        WeichesScrollen.Uncheck
        ' Display
        GrafikenUndObjekte.Check
        Zeichnungen.Check
        Feldname.Uncheck
        Notizen.Check
    end if

    if sOption$ = "Cursor" OR sOption$ = "All" then
        Call hToolsOptions("WRITER","FormattingAids")
        ' Display of
        Absatzenden.UnCheck
        WeicheTrenner.Check
        Leerzeichen.UnCheck
        GeschLeerzeichen.Check
        Tabulatoren.UnCheck
        Umbrueche.UnCheck
        VersteckterText.Uncheck
        VersteckteAbsatze.Check
        'Direct-Cursor
        DirektCursor.Uncheck
        Absatzausrichtung.Check
        'Cursor in protected areas
        Zonen_Cursor.Check
    end if

    if sOption$ = "Raster" OR sOption$ = "All" then
        Call hToolsOptions("WRITER","Grid")
        ' Grid
        FangrasterBenutzen.Uncheck
        RasterSichtbar.Uncheck
        ' Resolution
        AchsenSynchronisieren.Uncheck
        Select Case sTempUnit
        Case 1:     'Millimeter
            RasterAufloesungXAchse.SetText "10" & sTempSeparator & "00"
            RasterAufloesungYAchse.SetText "10" & sTempSeparator & "00"
        Case 2:     ' Centimeter
            RasterAufloesungXAchse.SetText "1" & sTempSeparator & "00"
            RasterAufloesungYAchse.SetText "1" & sTempSeparator & "00"
        Case 3:     ' Inch
            RasterAufloesungXAchse.SetText "0" & sTempSeparator & "39"
            RasterAufloesungYAchse.SetText "0" & sTempSeparator & "39"
        Case 4:     ' Pica
            RasterAufloesungXAchse.SetText "2" & sTempSeparator & "36"
            RasterAufloesungYAchse.SetText "2" & sTempSeparator & "36"
        Case 5:     ' Point
            RasterAufloesungXAchse.SetText "28" & sTempSeparator & "3"
            RasterAufloesungYAchse.SetText "28" & sTempSeparator & "3"
        end select
        RasterUnterteilungXAchse.ToMin
        RasterUnterteilungXAchse.More 1
        RasterUnterteilungYAchse.ToMin
        RasterUnterteilungYAchse.More 1
    end if

    if sOption$ = "Grundschriften" OR sOption$ = "All" then
        Call hToolsOptions("WRITER","BasicFonts")
        Standard.Click
        AktuellesDokument.Uncheck
    end if

    if sOption$ = "Drucken" OR sOption$ = "All" then
        Call hToolsOptions("WRITER","Print")
        ' Contents
        Grafiken.Check
        Kontrollfelder.Check
        Hintergrund.Check
        SchwarzDrucken.Uncheck
        ' Pages
        LinkeSeiten.Check
        RechteSeiten.Check
        Prospekt.Uncheck
        ' Notes
        Keine.Check
        ' Other
        LeereSeitenDrucken.Check
        AusDruckereinstellung.Uncheck
        Fax.SetNoSelection
    end if

    if sOption$ = "Tabelle" OR sOption$ = "All" then
        Call hToolsOptions("WRITER","Table")
        ' Default
        Ueberschrift.Check
        Wiederholen.Check
        NichtTrennen.Uncheck
        Umrandung.Check
        ' Input in Tables
        AutomatischeZahlenerkennung.Check
        Zahlenformaterkennung.Check
        AutomatischeAusrichtung.Check
        AutomatischeZahlenerkennung.UnCheck
        ' Keyboard handling
        Select Case sTempUnit
        Case 1:     'Millimeter
            VerschiebenZeile.SetText "4" & sTempSeparator & "99"
            VerschiebenSpalte.SetText "4" & sTempSeparator & "99"
            EinfuegenZeile.SetText "4" & sTempSeparator & "99"
            EinfuegenSpalte.SetText "24" & sTempSeparator & "99"
        Case 2:     ' Centimeter
            VerschiebenZeile.SetText "0" & sTempSeparator & "50"
            VerschiebenSpalte.SetText "0" & sTempSeparator & "50"
            EinfuegenZeile.SetText "0" & sTempSeparator & "50"
            EinfuegenSpalte.SetText "2" & sTempSeparator & "50"
        Case 3:     ' Inch
            VerschiebenZeile.SetText "0" & sTempSeparator & "20"
            VerschiebenSpalte.SetText "0" & sTempSeparator & "20"
            EinfuegenZeile.SetText "0" & sTempSeparator & "20"
            EinfuegenSpalte.SetText "0" & sTempSeparator & "98"
        Case 4:     ' Pica
            VerschiebenZeile.SetText "1" & sTempSeparator & "18"
            VerschiebenSpalte.SetText "1" & sTempSeparator & "18"
            EinfuegenZeile.SetText "1" & sTempSeparator & "18"
            EinfuegenSpalte.SetText "5" & sTempSeparator & "90"
        Case 5:     ' Point
            VerschiebenZeile.SetText "14" & sTempSeparator & "2"
            VerschiebenSpalte.SetText "14" & sTempSeparator & "2"
            EinfuegenZeile.SetText "14" & sTempSeparator & "2"
            EinfuegenSpalte.SetText "70" & sTempSeparator & "9"
        end select
        Sleep 1
        ' Behaviour of rows/columns
        Variabel.Check
    end if

    if sOption$ = "Aenderung" OR sOption$ = "All" then
        Call hToolsOptions("WRITER","Changes")
        ' Text display
        EinfuegenAttribute.Select 4
        EinfuegenFarbe.Select 2
        LoeschenAttribute.Select 4
        LoeschenFarbe.Select 2
        AendernAttribute.Select 2
        AendernFarbe.Select 2
        ' Lines changed
        Zeilenmarkierung.Select 2
        Zeilenfarbe.Select 1
    end if

    if sOption$ = "AutoCaption" OR sOption$ = "All" then
        Call hToolsOptions("WRITER","Autocaption")
        for i = 1 to ObjectList.GetItemCount
            ObjectList.Select i
            ' uncheck only if checked, else it might crash on macosx, see issue 111883 for further information
            if ObjectList.getState (i) > 1 then
                ObjectList.UnCheck
            endif
        next i
    end if

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Call hCloseDocument
end sub

' -----------------------------------------------------------------------

function wInsertDocumentinMasterDoc(DocumentName as string) as boolean
    Call hNewDocument
    hFileOpenLocally( DocumentName)
    Sleep 2
    Call wTypeKeys "<Mod1 A>"
    Sleep 2
    EditCopy
    Call hCloseDocument
    wInsertDocumentinMasterDoc = True
end function

' -----------------------------------------------------------------------

function CheckForFilters() as boolean
    Kontext "GraphicFilterBar"
    if not GraphicFilterBar.Exists then
        Warnlog "- Toolbox has been closed!"
        Kontext "GraphicObjectbar"
        if Filter.IsEnabled then
            GraphicObjectbar.TearOff Filter
            Kontext "GraphicFilterBar"
            GraphicFilterBar.Move ( 20, 20 )
            CheckForFilters = True
        else
            Warnlog "- Though graphic is selected, the filter button in objectbar is disabled!"
            Select Case gApplication
            Case "WRITER"
                Kontext "DocumentWriter"
                DocumentWriter.MouseDoubleClick 10,10
                DocumentWriter.MouseDown 50,20
                DocumentWriter.MouseUp 50,20
            case else
                Kontext "DocumentMasterDoc"
                DocumentMasterDoc.MouseDoubleClick 10,10
                DocumentMasterDoc.MouseDown 50,20
                DocumentMasterDoc.MouseUp 50,20
            end select
            Wait 500
            Kontext "GraphicObjectbar"
            if Filter.IsEnabled then
                GraphicObjectbar.TearOff Filter
                Kontext "GraphicFilterBar"
                GraphicFilterBar.Move ( 20, 20 )
                CheckForFilters = True
            else
                Warnlog "- Reselecting the graphic didn't enable the button in the toolbar! No further test!"
                CheckForFilters = False
            end if
        end if
    else
        CheckForFilters=True
    end if
    Kontext "GraphicFilterBar"
end function

' -----------------------------------------------------------------------

sub hInsertFloatingFrame
    Sleep 3
    InsertFloatingFrame
    Sleep 3
    Kontext "TabEigenschaften"
    FrameName.SetText "Hallo"
    TabEigenschaften.OK
    Sleep 2
    Kontext "OeffnenDlg"
    if OeffnenDlg.Exists(2) then OeffnenDlg.Cancel
end sub

' -----------------------------------------------------------------------

function wStyleCreate( sStyleName as string, sType as string, optional NotFromSelection as boolean ) as boolean
    '/// This function creates a new style through stylist

    Kontext "Stylist"
    If Not Stylist.Exists then FormatStylist

    select case sType
    case "Paragraph":	Absatzvorlagen.Click
    case "Character":	Zeichenvorlagen.Click
    case "Frame":		Rahmenvorlagen.Click
    case "Page":		Seitenvorlagen.Click
    case "Numbering":	Numerierungsvorlagen.Click
    end select

    if IsMissing ( NotFromSelection ) then
        wait 500
        AusSelektion.OpenMenu
        Sleep 1
        Call hMenuSelectNr(1)
        Sleep 1
        wait 500
        Kontext "VorlageErzeugen"
        Vorlagenname.Settext sStyleName
        VorlageErzeugen.Ok
    else
        if NotFromSelection = true then
            VorlagenListe.TypeKeys "<Mod1 End>"
            Vorlagenliste.TypeKeys "<Mod1 Home>"
            Vorlagenliste.OpenContextMenu
            Sleep 1
            Call hMenuSelectNr(1)
            wait 500
            Kontext
            Active.SetPage TabVerwalten
            wait 500
            Kontext "TabVerwalten"
            VorlagenName.SetText sStyleName
            wait 500
            TabVerwalten.OK
        end if
    end if

    Kontext "Stylist"

    if wStyleSelect ( sStyleName, sType ) = true then
        wStyleCreate = False
    else
        wStyleCreate = True
    end if
    Stylist.Close
end function

' -----------------------------------------------------------------------

function wStyleSelect( sStyleName as string, optional sType as string ) as boolean
    '/// This function selects a given style in Stylist
    '/// function leaves Stylist opened

    Dim i as integer

    Kontext "Stylist"
    If Not Stylist.Exists then FormatStylist

    wait 500
    Gruppenliste.Select 2	'-> 'All Styles' has to be selected
    wait 500
    if IsMissing (sType) then
        Absatzvorlagen.Click
    else
        select case sType
        case "Paragraph":	Absatzvorlagen.Click
        case "Character":	Zeichenvorlagen.Click
        case "Frame":		Rahmenvorlagen.Click
        case "Page":		Seitenvorlagen.Click
        case "Numbering":	Numerierungsvorlagen.Click
        end select
    end if

    Sleep 1
    Vorlagenliste.Select 1
    wait 500
    if Vorlagenliste.GetSelText <> sStyleName then
        For i = 1 to 200
            if Vorlagenliste.GetSelText = sStyleName then
                wStyleSelect = true
                i = 202
            else
                Vorlagenliste.TypeKeys "<Down>"
            end if
        next i
        if i = 202 then wStyleSelect = true
    else
        wStyleSelect = true
    end if

end function

' -----------------------------------------------------------------------

function wStyleDelete ( sStyleName as string, sType as string ) as boolean
    '/// This function selects a given style in Stylist
    '/// function leaves Stylist opened

    Dim i as integer

    Kontext "Stylist"
    If Not Stylist.Exists then FormatStylist

    select case sType
    case "Paragraph":	Absatzvorlagen.Click
    case "Character":	Zeichenvorlagen.Click
    case "Frame":		Rahmenvorlagen.Click
    case "Page":		Seitenvorlagen.Click
    case "Numbering":	Numerierungsvorlagen.Click
    end select

    Vorlagenliste.TypeKeys "<Mod1 End>"
    Vorlagenliste.TypeKeys "<Mod1 Home>"
    if Vorlagenliste.GetSelText <> sStyleName then
        For i = 1 to 200
            if Vorlagenliste.GetSelText = sStyleName then
                Vorlagenliste.OpenContextMenu
                Sleep 1
                Call hMenuSelectNr(3)
                wait 500
                Kontext "Active"
                if Active.Exists then
                    if Active.GetRT = 304 then
                        Active.Yes
                        i = 203
                    end if
                end if
            else
                Vorlagenliste.TypeKeys "<Down>"
            end if
        next i
        if i = 204 then wStyleDelete = true
    else
        Vorlagenliste.OpenContextMenu
        Sleep 1
        Call hMenuSelectNr(3)
        wait 500
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                Active.Yes
            end if
        end if
        wStyleDelete = true
    end if

end function

' -----------------------------------------------------------------------

function wStyleSet( sStyleName as string, optional sType as string ) as boolean
    '/// This function sets a given style in Stylist
    '/// function leaves Stylist opened

    Dim i as integer

    Kontext "Stylist"
    If Not Stylist.Exists then FormatStylist

    wait 500
    Gruppenliste.Select 2	'-> 'All Styles' has to be selected
    wait 500
    if IsMissing (sType) then
        Absatzvorlagen.Click
    else
        select case sType
        case "Paragraph":	Absatzvorlagen.Click
        case "Character":	Zeichenvorlagen.Click
        case "Frame":		Rahmenvorlagen.Click
        case "Page":		Seitenvorlagen.Click
        case "Numbering":	Numerierungsvorlagen.Click
        end select
    end if

    Sleep 1
    Vorlagenliste.Select 1
    wait 500
    if Vorlagenliste.GetSelText <> sStyleName then
        For i = 1 to 200
            if Vorlagenliste.GetSelText = sStyleName then
                wStyleSet = true
                Vorlagenliste.TypeKeys "<Return>"
                exit for
            else
                Vorlagenliste.TypeKeys "<Down>"
            end if
        next i
    else
        wStyleSet = true
    end if

end function

' -----------------------------------------------------------------------

function wStyleGet( sStyleName as string, optional sType as string ) as boolean
    '/// This function checks if a given style is selected in Stylist
    '/// function leaves Stylist opened

    Dim i as integer

    Kontext "Stylist"
    If Not Stylist.Exists then FormatStylist

    wait 500
    Gruppenliste.Select 2	'-> 'All Styles' has to be selected
    wait 500
    if IsMissing (sType) then
        Absatzvorlagen.Click
    else
        select case sType
        case "Paragraph":	Absatzvorlagen.Click
        case "Character":	Zeichenvorlagen.Click
        case "Frame":		Rahmenvorlagen.Click
        case "Page":		Seitenvorlagen.Click
        case "Numbering":	Numerierungsvorlagen.Click
        end select
    end if

    wait 500
    if Vorlagenliste.GetSelText = sStyleName then
        wStyleGet = true
    end if

end function
