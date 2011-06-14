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
'* short description : HTML-Options
'*
'\***********************************************************************

sub wh_o_1

    printLog Chr(13) + "---------    Options   - Tools HTML -    ----------"

    Call tToolsOptionsHTMLContent
    Call tToolsOptionsHTMLLayout  'wrn:1
    Call tToolsOptionsHTMLGrid    'wrn:4

end sub

'------------------------------------------------------------------------------
testcase tToolsOptionsHTMLContent

    Dim bCheckboxStatus(17) as boolean

    printlog "tToolsOptionsHTMLContent datei erstellen zum laden"
    '/// uses "input\\writer\\optional\\options\\options1.sxw"  ///
    '///+ Created Dokument with everything on ///'
    Call hNewDocument

    printlog "'///- Tools/Options/HTML: Content ///"

    printlog ("'/// - save states ///")
    ToolsOptions
    Call hToolsOptions ("HTML","View")

    bCheckboxStatus(1) = GrafikenUndObjekte.IsChecked
    bCheckboxStatus(3) = Zeichnungen.IsChecked
    bCheckboxStatus(4) = Feldnamen.IsChecked
    bCheckboxStatus(5) = Notizen.IsChecked

    'bCheckboxStatus(9) = Felder.IsChecked

    printlog ("'/// - all states inverting ///")

    if bCheckboxStatus(1) Then GrafikenUndObjekte.UnCheck Else GrafikenUndObjekte.Check
    if bCheckboxStatus(3) Then Zeichnungen.UnCheck Else Zeichnungen.Check
    if bCheckboxStatus(4) Then Feldnamen.UnCheck Else Feldnamen.Check
    if bCheckboxStatus(5) Then Notizen.UnCheck Else Notizen.Check
    
    'if bCheckboxStatus(9) Then Felder.UnCheck Else Felder.Check
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("'/// - SO quit - start ///")
    
    Call wOfficeRestart
    
    printlog ("'/// - checking states ///")
    ToolsOptions
    Call hToolsOptions ( "HTML","View" )
    
    if ( bCheckboxStatus(1) = GrafikenUndObjekte.IsChecked ) Then WarnLog "Grafiken state changed"
    if ( bCheckboxStatus(3) = Zeichnungen.IsChecked  ) Then WarnLog "Zeichnungen state changed"
    if ( bCheckboxStatus(4) = Feldnamen.IsChecked  ) Then  WarnLog "Feldnamen state changed"
    if ( bCheckboxStatus(5) = Notizen.IsChecked  ) Then  WarnLog "Notizen state changed"
    
    printlog ("'/// - all UnCheck -> o ///")
    
    GrafikenUndObjekte.UnCheck
    Zeichnungen.UnCheck
    Feldnamen.UnCheck
    Notizen.UnCheck
    
    'Felder.UnCheck
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("'/// - check if all UnChecked ///")
    ToolsOptions
    hToolsOptions ( "HTML","View" )
    
    if GrafikenUndObjekte.IsChecked Then WarnLog "Grafiken x"
    if Zeichnungen.IsChecked Then WarnLog "Zeichnungen x"
    if Feldnamen.IsChecked Then  WarnLog "Feldnamen x"
    if Notizen.IsChecked Then  WarnLog "Notizen x"
    
    Printlog ("'/// - all Check -> x ///")
    GrafikenUndObjekte.Check
    Zeichnungen.Check
    Feldnamen.Check
    Notizen.Check
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("'/// - check if all Checked ///")
    ToolsOptions
    Call hToolsOptions ( "HTML","View" )
    
    if true <> GrafikenUndObjekte.IsChecked Then WarnLog "Grafiken o"
    if True <> Zeichnungen.IsChecked Then WarnLog "Zeichnungen o"
    if True <> Feldnamen.IsChecked Then  WarnLog "Feldnamen o"
    if True <> Notizen.IsChecked Then  WarnLog "Notizen o"
    
    printlog ("'/// - restore states ///")    
    if ( bCheckboxStatus(1) = TRUE ) Then  GrafikenUndObjekte.Check Else GrafikenUndObjekte.UnCheck
    if ( bCheckboxStatus(3) = TRUE ) Then  Zeichnungen.Check Else Zeichnungen.UnCheck
    if ( bCheckboxStatus(4) = TRUE ) Then  Feldnamen.Check Else Feldnamen.UnCheck
    if ( bCheckboxStatus(5) = TRUE ) Then  Notizen.Check Else Notizen.UnCheck
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------
testcase tToolsOptionsHTMLLayout
    Dim bCheckboxStatus(11) as boolean
    Dim iMasseinheit(3) as integer
    Dim iTemp(3) as integer
    '///Open "writer\\optional\\input\\options\\options1.sxw ///
    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\options\options1.sxw" )
    
    
    printlog "'///- Tools/Options/Writer: View ///"
    ToolsOptions
    Call hToolsOptions ( "HTML" , "VIEW" ) '*TabLayoutHTML/ (1)
    
    bCheckboxStatus(1) = Hilfslinien.IsChecked
    bCheckboxStatus(2) = FarbigeHandles.IsChecked
    bCheckboxStatus(3) = GrosseHandles.IsChecked
    bCheckboxStatus(4) = HorizontaleBildlaufleiste.IsChecked
    bCheckboxStatus(5) = VertikaleBildlaufleiste.IsChecked
    bCheckboxStatus(10) = Lineal.IsChecked
    bCheckboxStatus(6) = HorizontalesLineal.IsChecked
    bCheckboxStatus(7) = VertikalesLineal.IsChecked
    bCheckboxStatus(8) = WeichesScrollen.IsChecked
    
    iMasseinheit(2) = HorizontalesLinealMasseinheit.GetSelIndex
    iMasseinheit(3) = VertikalesLinealMasseinheit.GetSelIndex
    
    printlog ("'/// - all states inverting ///")
    
    'this one has 2 places:  ViewTextBoundaries
    if bCheckboxStatus(1) Then Hilfslinien.UnCheck               Else Hilfslinien.Check
    if bCheckboxStatus(2) Then FarbigeHandles.UnCheck            Else FarbigeHandles.Check
    if bCheckboxStatus(3) Then GrosseHandles.UnCheck             Else GrosseHandles.Check
    
    'this one has 2 places:   ViewRuler
    if bCheckboxStatus(4) Then HorizontaleBildlaufleiste.UnCheck Else HorizontaleBildlaufleiste.Check
    if bCheckboxStatus(5) Then VertikaleBildlaufleiste.UnCheck   Else VertikaleBildlaufleiste.Check
    if bCheckboxStatus(10) then Lineal.UnCheck  Else Lineal.Check ' has to be checked to enable Horizontal and Vertical Ruler
    if Lineal.IsChecked then
        if bCheckboxStatus(6) Then HorizontalesLineal.UnCheck        Else HorizontalesLineal.Check
        if bCheckboxStatus(7) Then VertikalesLineal.UnCheck          Else VertikalesLineal.Check
        if ( HorizontalesLinealMasseinheit.GetItemCount <> iMasseinheit(2) ) Then HorizontalesLinealMasseinheit.Select (HorizontalesLinealMasseinheit.GetItemCount) Else HorizontalesLinealMasseinheit.Select (1)
        if ( VertikalesLinealMasseinheit.GetItemCount <> iMasseinheit(3) ) Then VertikalesLinealMasseinheit.Select (VertikalesLinealMasseinheit.GetItemCount) Else VertikalesLinealMasseinheit.Select (1)
        iTemp(2) = HorizontalesLinealMasseinheit.GetSelIndex
        iTemp(3) = VertikalesLinealMasseinheit.GetSelIndex
        '/// if Asian Language enabled there has to be a checkbox 'Right-aligned' for 'Vertical-Ruler' ///
        if gAsianSup = True and VertikalesLineal.IsChecked = True then
            if RechtsAusgerichtet.IsVisible = True then
                if bCheckboxStatus(9) Then RechtsAusgerichtet.Uncheck  Else RechtsAusgerichtet.Check
            else
                Warnlog "- Checkbox 'Right-aligned' is not visible !"
            end if
        end if
    end if
    if bCheckboxStatus(8) Then WeichesScrollen.UnCheck           Else WeichesScrollen.Check
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("'/// - SO quit - start ///")
    
    Call wOfficeRestart
    
    printlog ("'/// - checking states ///" )
    ToolsOptions
    Call hToolsOptions ( "HTML" , "VIEW" ) '*TabLayoutHTML/ (1)
    
    if ( bCheckboxStatus(1) = Hilfslinien.IsChecked               ) Then WarnLog "Hilfslinien state changed"
    if ( bCheckboxStatus(2) = FarbigeHandles.IsChecked            ) Then WarnLog "FarbigeHandles state changed"
    if ( bCheckboxStatus(3) = GrosseHandles.IsChecked             ) Then WarnLog "GrosseHandles state changed"
    if ( bCheckboxStatus(4) = HorizontaleBildlaufleiste.IsChecked ) Then WarnLog "HorizontaleBildlaufleiste state changed"
    if ( bCheckboxStatus(5) = VertikaleBildlaufleiste.IsChecked   ) Then WarnLog "VertikaleBildlaufleiste state changed"
    if ( bCheckboxStatus(8) = WeichesScrollen.IsChecked          ) Then WarnLog "WeichesScrollen state changed"
    if ( bCheckboxStatus(10) = Lineal.IsChecked ) Then
        WarnLog "Ruler state changed"
        if ( bCheckboxStatus(6) = HorizontalesLineal.IsChecked        ) Then WarnLog "HorizontalesLineal state changed"
        if ( bCheckboxStatus(7) = VertikalesLineal.IsChecked         ) Then WarnLog "VertikalesLineal state changed"
        if ( HorizontalesLinealMasseinheit.GetSelIndex <> iTemp(2) ) Then WarnLog "HorizontalesLinealMasseinheit state changed"
        if ( VertikalesLinealMasseinheit.GetSelIndex <> iTemp(3) ) Then WarnLog "VertikalesLinealMasseinheit   state changed"
    
        if gAsianSup = True and VertikalesLineal.IsChecked = True then
            if RechtsAusgerichtet.IsVisible = True then
                if bCheckboxStatus(9) = RechtsAusgerichtet.IsChecked then WarnLog "Right-aligned state changed"
            else
                Warnlog "- Checkbox 'Right-aligned' is not visible !"
            end if
        end if
    end if
    
    printlog ("'/// - all UnCheck -> o ///")
    Hilfslinien.UnCheck
    FarbigeHandles.UnCheck
    GrosseHandles.UnCheck
    
    HorizontaleBildlaufleiste.UnCheck
    VertikaleBildlaufleiste.UnCheck
    Lineal.Check
    HorizontalesLineal.UnCheck
    if gAsianSup = True then
        VertikalesLineal.Check
        RechtsAusgerichtet.Uncheck
    end if
    VertikalesLineal.Uncheck
    HorizontalesLinealMasseinheit.Select (1)
    VertikalesLinealMasseinheit.Select (1)
    Lineal.Uncheck
    WeichesScrollen.Uncheck
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    ViewTextBoundaries
    ViewRuler
   
    printlog ("'/// - check if all UnChecked ///")
    ToolsOptions
    Call hToolsOptions ( "HTML" , "VIEW" ) '*TabLayoutHTML/ (1)
    if Hilfslinien.IsChecked               Then WarnLog "Hilfslinien x"
    if FarbigeHandles.IsChecked            Then WarnLog "FarbigeHandles x"
    if GrosseHandles.IsChecked             Then WarnLog "GrosseHandles x"
    
    if HorizontaleBildlaufleiste.IsChecked Then WarnLog "HorizontaleBildlaufleiste x"
    if VertikaleBildlaufleiste.IsChecked   Then WarnLog "VertikaleBildlaufleiste x"
    if HorizontalesLineal.IsChecked        Then WarnLog "HorizontalesLineal x/ ViewRuler"
    if VertikalesLineal.IsChecked          Then WarnLog "VertikalesLineal x"
    if WeichesScrollen.IsChecked           Then WarnLog "WeichesScrollen x"
    if gAsianSup=True then
        if RechtsAusgerichtet.IsVisible = True then
            if RechtsAusgerichtet.IsEnabled = True then WarnLog "Right-aligned checkbox is enabled"
        else
            Warnlog "- Checkbox 'Right-aligned' is not visible !"
        end if
    end if
    
    if ( HorizontalesLinealMasseinheit.GetSelIndex <> 1 ) Then WarnLog "HorizontalesLinealMasseinheit is not item 1"
    if ( VertikalesLinealMasseinheit.GetSelIndex <> 1 ) Then WarnLog "VertikalesLinealMasseinheit   is not item 1"
    
    Printlog ("'/// - all Check -> x ///")
    Hilfslinien.Check
    FarbigeHandles.Check
    GrosseHandles.Check
    Lineal.Check
    HorizontaleBildlaufleiste.Check
    VertikaleBildlaufleiste.Check
    HorizontalesLineal.Check
    VertikalesLineal.Check
    WeichesScrollen.Check
    
    HorizontalesLinealMasseinheit.Select (2)
    VertikalesLinealMasseinheit.Select (2)
    itemp(2) = HorizontalesLinealMasseinheit.GetSelIndex
    itemp(3) = VertikalesLinealMasseinheit.GetSelIndex
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    ViewTextBoundaries   ' see here :-)
    ViewRuler
    
    printlog ("'/// - check if all Checked ///")
    ToolsOptions
    Call hToolsOptions ( "HTML" , "VIEW" ) '*TabLayoutHTML/ (1)
    if True <> Hilfslinien.IsChecked               Then WarnLog "Hilfslinien o"
    if True <> FarbigeHandles.IsChecked            Then WarnLog "FarbigeHandles o"
    if True <> GrosseHandles.IsChecked             Then WarnLog "GrosseHandles o"
    
    if True <> HorizontaleBildlaufleiste.IsChecked Then WarnLog "HorizontaleBildlaufleiste o"
    if True <> VertikaleBildlaufleiste.IsChecked   Then WarnLog "VertikaleBildlaufleiste o"
    if True <> HorizontalesLineal.IsChecked        Then WarnLog "HorizontalesLineal o/      ViewRuler"
    if True <> VertikalesLineal.IsChecked          Then WarnLog "VertikalesLineal o"
    if True <> WeichesScrollen.IsChecked           Then WarnLog "WeichesScrollen o"
    
    if ( HorizontalesLinealMasseinheit.GetSelIndex <> iTemp(2) ) Then WarnLog "HorizontalesLinealMasseinheit is not item " + iTemp(2) + " it's: " + HorizontalesLinealMasseinheit.GetSelIndex
    if ( VertikalesLinealMasseinheit.GetSelIndex <> iTemp(3) ) Then WarnLog "VertikalesLinealMasseinheit   is not item " + iTemp(3) + " it's: " + VertikalesLinealMasseinheit.GetSelIndex
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel
    
    '------------------------------------------------------------------
    printlog "'/// Lines Visibility Test ///'"
    Call hNewDocument
    
    ' Table insert
    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"
    TabelleEinfuegenWriter.OK
    
    ' changing linewidth to 0
    FormatTable
    Kontext
    active.SetPage TabUmrandung
    Kontext "TabUmrandung"
    Stil.Select 1
    'Stil.Typekeys "<home>"
    TabUmrandung.Ok
    sleep (3)
    
    ' insert graphic
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<down>"
    DocumentWriter.TypeKeys "<down>"
    
    Call hGrafikEinfuegen gTesttoolPath + "writer\optional\input\options\ga000907.gif"
    
    ' see everything
    ToolsOptions
    Call hToolsOptions ( "HTML" , "VIEW" )
    
    '///' Hilfslinien.Check only, when graphic's in motion! ///'
    FarbigeHandles.Check
    GrosseHandles.Check
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok
    
    ToolsOptions
    Call hToolsOptions ( "HTML" , "VIEW" )
    FarbigeHandles.UnCheck
    GrosseHandles.UnCheck
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok
    
    Call hCloseDocument
    
    ToolsOptions
    Call hToolsOptions ( "HTML" , "VIEW" )
    
    if ( bCheckboxStatus(1) =  TRUE ) Then  Hilfslinien.Check               Else Hilfslinien.UnCheck
    if ( bCheckboxStatus(2) =  TRUE ) Then  FarbigeHandles.Check            Else FarbigeHandles.UnCheck
    if ( bCheckboxStatus(3) =  TRUE ) Then  GrosseHandles.Check             Else GrosseHandles.UnCheck
    
    if ( bCheckboxStatus(4) =  TRUE ) Then  HorizontaleBildlaufleiste.Check Else HorizontaleBildlaufleiste.UnCheck
    if ( bCheckboxStatus(5) =  TRUE ) Then  VertikaleBildlaufleiste.Check   Else VertikaleBildlaufleiste.UnCheck
    Lineal.Check
    if ( bCheckboxStatus(6) =  TRUE ) Then  HorizontalesLineal.Check        Else HorizontalesLineal.UnCheck
    if ( bCheckboxStatus(7) = TRUE ) Then  VertikalesLineal.Check          Else VertikalesLineal.UnCheck
    HorizontalesLinealMasseinheit.Select (iMasseinheit(2))
    VertikalesLinealMasseinheit.Select (iMasseinheit(3))
    
    if ( bCheckboxStatus(8) = TRUE ) Then  WeichesScrollen.Check           Else WeichesScrollen.UnCheck
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------
testcase tToolsOptionsHTMLGrid

    Dim bCheckboxStatus(3) as boolean
    Dim sMetricField(4) as string      ' MetricField
    Dim sTempMetricField(4) as string
        
    '/// uses: "writer\\optional\\input\\options\\options1.sxw" ///
    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\options\options1.sxw" )
    
   
    printlog "'///- Tools/Options/HTML: Grid ///"
    printlog ("'/// - save states ///")
    
    ToolsOptions
    Call hToolsOptions ( "HTML","Grid" )
    
    printlog ("'///TabRaster all UnCheck ! ///")
    
    bCheckboxStatus(1) = FangrasterBenutzen.IsChecked
    bCheckboxStatus(2) = RasterSichtbar.IsChecked
    bCheckboxStatus(3) = AchsenSynchronisieren.IsChecked
    
    sMetricField(1) = RasterAufloesungXAchse.GetText
    sMetricField(2) = RasterAufloesungYAchse.GetText
    sMetricField(3) = RasterUnterteilungXAchse.GetText
    sMetricField(4) = RasterUnterteilungYAchse.GetText
    
    printlog ("'/// - all states inverting ///")
    
    if bCheckboxStatus(1) Then FangrasterBenutzen.UnCheck    Else FangrasterBenutzen.Check
    if bCheckboxStatus(2) Then RasterSichtbar.UnCheck        Else RasterSichtbar.Check
    '      if bCheckboxStatus(3) Then AchsenSynchronisieren.UnCheck Else AchsenSynchronisieren.Check
    '   this CheckBox interacts with the MetricBoxes:
    AchsenSynchronisieren.UnCheck
    
    sTempMetricField(1) = RasterAufloesungXAchse.GetText
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(3) = RasterUnterteilungXAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText
    RasterAufloesungXAchse.ToMax
    RasterAufloesungYAchse.ToMin
    RasterUnterteilungXAchse.ToMin
    RasterUnterteilungYAchse.ToMax
    if ( RasterAufloesungXAchse.GetText =   sTempMetricField(1) ) Then RasterAufloesungXAchse.ToMin
    if ( RasterAufloesungYAchse.GetText =   sTempMetricField(2) ) Then RasterAufloesungYAchse.ToMax
    if ( RasterUnterteilungXAchse.GetText = sTempMetricField(3) ) Then RasterUnterteilungXAchse.ToMax
    if ( RasterUnterteilungYAchse.GetText = sTempMetricField(4) ) Then RasterUnterteilungYAchse.ToMin
    sTempMetricField(1) = RasterAufloesungXAchse.GetText
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(3) = RasterUnterteilungXAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("'/// - SO quit - start///" )
    
    Call wOfficeRestart
    
    printlog ("'/// - checking states ///")
    ToolsOptions
    Call hToolsOptions ( "HTML","Grid" )
    
    if (bCheckboxStatus(1) = FangrasterBenutzen.IsChecked    ) Then WarnLog "FangrasterBenutzen state changed BugID: 82944 "
    if (bCheckboxStatus(2) = RasterSichtbar.IsChecked        ) Then WarnLog "RasterSichtbar state changed"
    if ( AchsenSynchronisieren.IsChecked = TRUE ) Then WarnLog "AchsenSynchronisieren state changed"
    if ( RasterAufloesungXAchse.GetText <>   sTempMetricField(1) ) Then WarnLog "RasterAufloesungXAchse state changed"
    if ( RasterAufloesungYAchse.GetText <>   sTempMetricField(2) ) Then WarnLog "RasterAufloesungYAchse state changed"
    if ( RasterUnterteilungXAchse.GetText <> sTempMetricField(3) ) Then WarnLog "RasterUnterteilungXAchse state changed"
    if ( RasterUnterteilungYAchse.GetText <> sTempMetricField(4) ) Then WarnLog "RasterUnterteilungYAchse state changed"
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("'/// - all UnCheck -> o ///")
    ToolsOptions
    Call hToolsOptions ( "HTML","Grid" )
    
    FangrasterBenutzen.UnCheck
    RasterSichtbar.UnCheck
    AchsenSynchronisieren.UnCheck
    RasterAufloesungXAchse.ToMin
    RasterAufloesungYAchse.ToMin
    RasterUnterteilungXAchse.ToMin
    RasterUnterteilungYAchse.ToMin
    sTempMetricField(1) = RasterAufloesungXAchse.GetText
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(3) = RasterUnterteilungXAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText
    printlog ("'/// More test ///'")
    RasterAufloesungXAchse.More
    RasterAufloesungYAchse.More
    RasterUnterteilungXAchse.More
    RasterUnterteilungYAchse.More
    if ( sTempMetricField(1) = RasterAufloesungXAchse.GetText   ) Then WarnLog "RasterAufloesungXAchse  More-Button not working"
    if ( sTempMetricField(2) = RasterAufloesungYAchse.GetText   ) Then WarnLog "RasterAufloesungYAchse  More-Button not working"
    if ( sTempMetricField(3) = RasterUnterteilungXAchse.GetText ) Then WarnLog "RasterUnterteilungXAchse More-Button not working"
    if ( sTempMetricField(4) = RasterUnterteilungYAchse.GetText ) Then WarnLog "RasterUnterteilungYAchse More-Button not working"
    sTempMetricField(1) = RasterAufloesungXAchse.GetText
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(3) = RasterUnterteilungXAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("'/// - check if all UnChecked ///")
    ToolsOptions
    Call hToolsOptions ( "HTML","Grid" )
    
    if FangrasterBenutzen.IsChecked     Then WarnLog "FangrasterBenutzen x"
    if RasterSichtbar.IsChecked         Then WarnLog "RasterSichtbar x"
    if AchsenSynchronisieren.IsChecked  Then WarnLog "AchsenSynchronisieren x"
    if ( RasterAufloesungXAchse.GetText <>   sTempMetricField(1) ) Then WarnLog "RasterAufloesungXAchse is not min."
    if ( RasterAufloesungYAchse.GetText <>   sTempMetricField(2) ) Then WarnLog "RasterAufloesungYAchse is not min."
    if ( RasterUnterteilungXAchse.GetText <> sTempMetricField(3) ) Then WarnLog "RasterUnterteilungXAchse is not min."
    if ( RasterUnterteilungYAchse.GetText <> sTempMetricField(4) ) Then WarnLog "RasterUnterteilungYAchse is not min."
    
    Printlog (" - all Check -> x")
    
    FangrasterBenutzen.Check
    RasterSichtbar.Check
    AchsenSynchronisieren.Check
    
    '///' Synchronize axes check -------------------- ///'
    RasterAufloesungXAchse.ToMax
    wait 500
    sTempMetricField(1) = RasterAufloesungXAchse.GetText
    if RasterAufloesungYAchse.GetText <> sTempMetricField(1) then Warnlog "Y-axis res. differs from X-axis"
    RasterUnterteilungXAchse.ToMax
    wait 500
    sTempMetricField(3) = RasterUnterteilungXAchse.GetText
    if RasterUnterteilungYAchse.GetText <> sTempMetricField(3) then Warnlog "Y-axis sub. differs from X-axis"
    '--------------------------------------------
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("'/// - check if all Checked ///")
    ToolsOptions
    Call hToolsOptions ( "HTML","Grid" )
    
    if True <> FangrasterBenutzen.IsChecked     Then WarnLog "FangrasterBenutzen o"
    if True <> RasterSichtbar.IsChecked         Then WarnLog "RasterSichtbar o"
    if True <> AchsenSynchronisieren.IsChecked  Then WarnLog "AchsenSynchronisieren o"
    if ( RasterAufloesungXAchse.GetText <>   sTempMetricField(1) ) Then WarnLog "RasterAufloesungXAchse is not max.Bug 54934 in 01:"
    if ( RasterAufloesungYAchse.GetText <>   sTempMetricField(2) ) Then WarnLog "RasterAufloesungYAchse is not max."
    if ( RasterUnterteilungXAchse.GetText <> sTempMetricField(3) ) Then WarnLog "RasterUnterteilungXAchse is not max."
    if ( RasterUnterteilungYAchse.GetText <> sTempMetricField(4) ) Then WarnLog "RasterUnterteilungYAchse is not max."
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel
	
	sleep (4)
    
    '-------------------------------------------------------------------------------
    printlog ("'/// Function Test: Grid Resolution ///'")
    '///  ( gTesttoolPath + "writer\optional\input\options\gridtst.sxw" ) ///'
    '/// NOT Possible Without <ALT> + <cursor> stuff :-((((( BugID: 84741 ///'
    '/// uses"writer\optional\input\options\htmltag.html" ///
    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\options\htmltag.html" )
    
    
    '/// check if writeable!///'
    try
        FormatStylist
        FormatStylist
    catch
        '/// make writeable :-)///'
        Kontext "Funktionsleiste"
        Bearbeiten.click
    endcatch
    sleep (3)
    
    ' select grafik element
    Call wNavigatorAuswahl(4,1)
    ViewNavigator
    sleep (2)
    FormatGraphics
    Kontext
    Active.Setpage TabType
    Kontext "TabType"
    AnchorAtPage.Check
    sleep (1)
    if ( 0 <> StrToDouble (HorizontalBy.GetText)) Then WarnLog "HorizontalBy wrong presupposition != 0"
    if ( 0 <> StrToDouble (VerticalBy.GetText)) Then WarnLog   "VerticalBy   wrong presupposition != 0"
    TabType.OK
    
    ' change Grid Resolution
    ToolsOptions
    Call hToolsOptions ( "HTML","Grid" )
    RasterAufloesungXAchse.SetText("2")
    RasterAufloesungYAchse.SetText("3")
    RasterUnterteilungXAchse.SetText("2")
    RasterUnterteilungYAchse.SetText("3")
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    '/// move grafik with <Alt> + <up/down> ///'
    '/// resulting step = resolution / subdivision ///'
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Mod2 Down>"
    DocumentWriter.TypeKeys "<Mod2 Right>"
    DocumentWriter.TypeKeys "<Mod2 Up>"
    DocumentWriter.TypeKeys "<Mod2 Left>"
    DocumentWriter.TypeKeys "<Mod2 Right>"
    DocumentWriter.TypeKeys "<Mod2 Down>"
    
    ' check if @ (1,1) Current-Measure-Unit :-)
    Call wNavigatorAuswahl(4,1)
    ViewNavigator
    sleep (2)
    FormatGraphics
    Kontext
    Active.Setpage TabType
    Kontext "TabType"
    AnchorAtPage.Check
    sleep (1)
    if ( 0 = StrToDouble (HorizontalBy.GetText)) Then
        QAErrorLog "RasterAufloesungXAchse should be 1 is: " + HorizontalBy.GetText
    end if
    if ( 0 = StrToDouble (VerticalBy.GetText)) Then
        QAErrorLog "RasterAufloesungYAchse should be 1 is: " + VerticalBy.GetText
    end if
    TabType.Cancel
    Call hCloseDocument
    '--------------------------------------------------
    
    printlog ("'/// - restore states ///")
    ToolsOptions
    Call hToolsOptions ( "HTML","Grid" )
    
    if ( bCheckboxStatus(1) =  TRUE ) Then FangrasterBenutzen.Check    Else FangrasterBenutzen.UnCheck
    if ( bCheckboxStatus(2) =  TRUE ) Then RasterSichtbar.Check        Else RasterSichtbar.UnCheck
    if ( bCheckboxStatus(3) =  TRUE ) Then AchsenSynchronisieren.Check Else AchsenSynchronisieren.UnCheck
    RasterAufloesungXAchse.SetText   (sMetricField(1))
    RasterAufloesungYAchse.SetText   (sMetricField(2))
    RasterUnterteilungXAchse.SetText (sMetricField(3))
    RasterUnterteilungYAchse.SetText (sMetricField(4))
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

