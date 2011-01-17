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
'* short description : Functional-Tests for Tools-Options-Writer -View & Grid
'*
'\***********************************************************************

sub wr_o_1
    printLog Chr(13) + "---------    Options   - Tools Writer -    ----------"

    Call tToolsOptionsWriterView1
    Call tToolsOptionsWriterView2
    Call tToolsOptionsWriterView3
    Call tToolsOptionsWriterView4
    Call tToolsOptionsWriterView5
    Call tToolsOptionsWriterView6
    Call tToolsOptionsWriterView7
    Call tToolsOptionsWriterGrid1
    Call tToolsOptionsWriterGrid2
    Call tToolsOptionsWriterGrid3
    Call tToolsOptionsWriterGrid4

end sub

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterView1
    dim option_value(11) as boolean
    dim measurement_unit(3) as integer
    dim iTemp(3) as integer
    '/// new writerdocument
    Call hNewDocument

    '///- Tools/Options/Writer: View
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "VIEW" )

    option_value(1) = Hilfslinien.IsChecked
    option_value(2) = FarbigeHandles.IsChecked
    option_value(3) = GrosseHandles.IsChecked
    option_value(4) = HorizontaleBildlaufleiste.IsChecked
    option_value(5) = VertikaleBildlaufleiste.IsChecked
    option_value(10) = Lineal.IsChecked
    option_value(6) = HorizontalesLineal.IsChecked
    option_value(7) = VertikalesLineal.IsChecked
    option_value(8) = WeichesScrollen.IsChecked

    measurement_unit(2) = HorizontalesLinealMasseinheit.GetSelIndex
    measurement_unit(3) = VertikalesLinealMasseinheit.GetSelIndex

    '/// - all states inverting
    if option_value(1) = true then Hilfslinien.UnCheck      else Hilfslinien.Check
    if option_value(2) = true then FarbigeHandles.UnCheck   else FarbigeHandles.Check
    if option_value(3) = true then GrosseHandles.UnCheck    else GrosseHandles.Check
    if option_value(4) = true then HorizontaleBildlaufleiste.UnCheck else HorizontaleBildlaufleiste.Check
    if option_value(5) = true then VertikaleBildlaufleiste.UnCheck   else VertikaleBildlaufleiste.Check
    ' has to be checked to enable Horizontal and Vertical Ruler
    if option_value(10) = true then Lineal.UnCheck else Lineal.Check
    if Lineal.IsChecked = true then
        if option_value(6) = true then HorizontalesLineal.UnCheck   else HorizontalesLineal.Check
        if option_value(7) = true then VertikalesLineal.UnCheck     else VertikalesLineal.Check
        if ( HorizontalesLinealMasseinheit.GetItemCount <> measurement_unit(2) ) then
            HorizontalesLinealMasseinheit.Select (HorizontalesLinealMasseinheit.GetItemCount)
        else
            HorizontalesLinealMasseinheit.Select (1)
        end if
        if ( VertikalesLinealMasseinheit.GetItemCount <> measurement_unit(3) ) then
            VertikalesLinealMasseinheit.Select (VertikalesLinealMasseinheit.GetItemCount)
        else
            VertikalesLinealMasseinheit.Select (1)
        end if
        iTemp(2) = HorizontalesLinealMasseinheit.GetSelIndex
        iTemp(3) = VertikalesLinealMasseinheit.GetSelIndex
        '/// if Asian Language enabled there has to be a checkbox 'Right-aligned' for 'Vertical-Ruler' ///
        if bAsianLan=true and VertikalesLineal.IsChecked = true then
            if RechtsAusgerichtet.IsVisible = true then
                if option_value(9)= true then
                    RechtsAusgerichtet.Uncheck
                else
                    RechtsAusgerichtet.Check
                end if
            else
                Warnlog "- Checkbox 'Right-aligned' is not visible !"
            end if
        end if
    end if
    if option_value(8)= true then WeichesScrollen.UnCheck else WeichesScrollen.Check

    '/// close options dialog
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    '/// - SO quit - restart
    Call wOfficeRestart
    '///- Tools/Options/Writer: View
    '/// - checking states
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "VIEW" )
    if ( option_value(1) = Hilfslinien.IsChecked               ) then WarnLog "Hilfslinien state changed"
    if ( option_value(2) = FarbigeHandles.IsChecked            ) then WarnLog "FarbigeHandles state changed"
    if ( option_value(3) = GrosseHandles.IsChecked             ) then WarnLog "GrosseHandles state changed"
    if ( option_value(4) = HorizontaleBildlaufleiste.IsChecked ) then WarnLog "HorizontaleBildlaufleiste state changed"
    if ( option_value(5) = VertikaleBildlaufleiste.IsChecked   ) then WarnLog "VertikaleBildlaufleiste state changed"
    if ( option_value(8) = WeichesScrollen.IsChecked           ) then WarnLog "WeichesScrollen state changed"
    if ( option_value(10) = Lineal.IsChecked ) then
        WarnLog "Ruler state changed"
        if ( option_value(6) = HorizontalesLineal.IsChecked        ) then WarnLog "HorizontalesLineal state changed"
        if ( option_value(7) = VertikalesLineal.IsChecked         ) then WarnLog "VertikalesLineal state changed"
        if ( HorizontalesLinealMasseinheit.GetSelIndex <> iTemp(2) ) then WarnLog "HorizontalesLinealMasseinheit state changed"
        if ( VertikalesLinealMasseinheit.GetSelIndex <> iTemp(3) ) then WarnLog "VertikalesLinealMasseinheit   state changed"

        if bAsianLan=true and VertikalesLineal.IsChecked = true then
            if RechtsAusgerichtet.IsVisible = true then
                if option_value(9) = RechtsAusgerichtet.IsChecked then WarnLog "Right-aligned state changed"
            else
                Warnlog "- Checkbox 'Right-aligned' is not visible !"
            end if
        end if
    end if

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    '/// restore default values
    Call wOptionsUndo( "All" )

    '/// close document
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterView2
    dim option_value(11) as boolean
    dim measurement_unit(3) as integer
    dim iTemp(3) as integer
    '/// new writerdocument 
    Call hNewDocument

    '///- Tools/Options/Writer: View
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "VIEW" )

    '/// remember settings
    option_value(1) = Hilfslinien.IsChecked
    option_value(2) = FarbigeHandles.IsChecked
    option_value(3) = GrosseHandles.IsChecked
    option_value(4) = HorizontaleBildlaufleiste.IsChecked
    option_value(5) = VertikaleBildlaufleiste.IsChecked
    option_value(10) = Lineal.IsChecked
    option_value(6) = HorizontalesLineal.IsChecked
    option_value(7) = VertikalesLineal.IsChecked
    option_value(8) = WeichesScrollen.IsChecked
    
    measurement_unit(2) = HorizontalesLinealMasseinheit.GetSelIndex
    measurement_unit(3) = VertikalesLinealMasseinheit.GetSelIndex

    '/// - uncheck all checkboxes
    Hilfslinien.UnCheck
    FarbigeHandles.UnCheck
    GrosseHandles.UnCheck

    HorizontaleBildlaufleiste.UnCheck
    VertikaleBildlaufleiste.UnCheck
    Lineal.Check
    HorizontalesLineal.UnCheck
    if bAsianLan = true then
        VertikalesLineal.Check
        RechtsAusgerichtet.Uncheck
    end if
    VertikalesLineal.Uncheck
    HorizontalesLinealMasseinheit.Select (1)
    VertikalesLinealMasseinheit.Select (1)
    Lineal.Uncheck
    WeichesScrollen.Uncheck

    '/// close options-dialog
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    '/// check if ViewRuler is disabled
    ViewTextBoundaries
    try
        ViewRuler
        Warnlog "View / Ruler should be disabled !"
    catch
    endcatch

    '/// - SO quit - restart
    Call wOfficeRestart
    '///- Tools/Options/Writer: View
    '/// - checking states
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "VIEW" )
    if Hilfslinien.IsChecked = true                 then WarnLog "Hilfslinien x"
    if FarbigeHandles.IsChecked = true              then WarnLog "FarbigeHandles x"
    if GrosseHandles.IsChecked = true               then WarnLog "GrosseHandles x"
    
    if HorizontaleBildlaufleiste.IsChecked = true   then WarnLog "HorizontaleBildlaufleiste x"
    if VertikaleBildlaufleiste.IsChecked = true     then WarnLog "VertikaleBildlaufleiste x"
    if HorizontalesLineal.IsChecked = true          then WarnLog "HorizontalesLineal x/ ViewRuler"
    if VertikalesLineal.IsChecked = true            then WarnLog "VertikalesLineal x"
    if WeichesScrollen.IsChecked = true             then WarnLog "WeichesScrollen x"
    if bAsianLan=true then
        if RechtsAusgerichtet.IsVisible = true then
            if RechtsAusgerichtet.IsEnabled = true then WarnLog "Right-aligned checkbox is enabled"
        else
            Warnlog "- Checkbox 'Right-aligned' is not visible !"
        end if
    end if

    if ( HorizontalesLinealMasseinheit.GetSelIndex <> 1 ) then WarnLog "HorizontalesLinealMasseinheit is not item 1"
    if ( VertikalesLinealMasseinheit.GetSelIndex <> 1 ) then WarnLog "VertikalesLinealMasseinheit   is not item 1"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    '/// restore default values
    Call wOptionsUndo( "All" )

    '/// close writer document
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterView3
    dim option_value(11) as boolean
    dim measurement_unit(3) as integer
    dim iTemp(3) as integer

    '/// new writer document
    Call hNewDocument

    '///- Tools/Options/Writer: View
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "VIEW" )

    '/// remember all settings
    option_value(1) = Hilfslinien.IsChecked
    option_value(2) = FarbigeHandles.IsChecked
    option_value(3) = GrosseHandles.IsChecked
    option_value(4) = HorizontaleBildlaufleiste.IsChecked
    option_value(5) = VertikaleBildlaufleiste.IsChecked
    option_value(10) = Lineal.IsChecked
    option_value(6) = HorizontalesLineal.IsChecked
    option_value(7) = VertikalesLineal.IsChecked
    option_value(8) = WeichesScrollen.IsChecked
    
    measurement_unit(2) = HorizontalesLinealMasseinheit.GetSelIndex
    measurement_unit(3) = VertikalesLinealMasseinheit.GetSelIndex

    '/// - check all checkboxes
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

    '/// close options dialog
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    ViewTextBoundaries
    ViewRuler

    '/// - SO quit - restart
    Call wOfficeRestart
    '///- Tools/Options/Writer: View
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "VIEW" )
    
    '/// - check if all checked
    if Hilfslinien.IsChecked = false     then WarnLog "Hilfslinien o"
    if FarbigeHandles.IsChecked = false  then WarnLog "FarbigeHandles o"
    if GrosseHandles.IsChecked = false   then WarnLog "GrosseHandles o"

    if HorizontaleBildlaufleiste.IsChecked  = false then WarnLog "HorizontaleBildlaufleiste o"
    if VertikaleBildlaufleiste.IsChecked = false    then WarnLog "VertikaleBildlaufleiste o"
    if HorizontalesLineal.IsChecked = false         then WarnLog "HorizontalesLineal o/      ViewRuler"
    if VertikalesLineal.IsChecked = false           then WarnLog "VertikalesLineal o"
    if WeichesScrollen.IsChecked = false            then WarnLog "WeichesScrollen o"

    if ( HorizontalesLinealMasseinheit.GetSelIndex <> iTemp(2) ) then WarnLog "HorizontalesLinealMasseinheit is not item " + iTemp(2) + " it's: " + HorizontalesLinealMasseinheit.GetSelIndex
    if ( VertikalesLinealMasseinheit.GetSelIndex <> iTemp(3) ) then WarnLog "VertikalesLinealMasseinheit   is not item " + iTemp(3) + " it's: " + VertikalesLinealMasseinheit.GetSelIndex

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    '/// restore default values
    Call wOptionsUndo( "All" )

    '/// close document
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterView4
    dim option_value(11) as boolean
    dim measurement_unit(3) as integer
    dim iTemp(3) as integer
    '/// new writer document
    printlog "- new writer document"
    Call hNewDocument

    '///- Tools/Options/Writer: View
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "VIEW" )

    '/// remember all values
    printlog "- remember all values"
    option_value(1) = Hilfslinien.IsChecked
    option_value(2) = FarbigeHandles.IsChecked
    option_value(3) = GrosseHandles.IsChecked
    option_value(4) = HorizontaleBildlaufleiste.IsChecked
    option_value(5) = VertikaleBildlaufleiste.IsChecked
    option_value(10) = Lineal.IsChecked
    option_value(6) = HorizontalesLineal.IsChecked
    option_value(7) = VertikalesLineal.IsChecked
    option_value(8) = WeichesScrollen.IsChecked
    
    measurement_unit(2) = HorizontalesLinealMasseinheit.GetSelIndex
    measurement_unit(3) = VertikalesLinealMasseinheit.GetSelIndex

    '/// close options dialog
    printlog "- close options dialog"
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    '/// Insert a table
    printlog "- insert a table"
    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"
    TabelleEinfuegenWriter.OK

    '/// changing linewidth to 0
    printlog "- changing linewidth to 0"
    FormatTable
    Kontext
    active.SetPage TabUmrandung
    Kontext "TabUmrandung"
    Stil.Select 1
    TabUmrandung.Ok
    sleep (3)

    '/// insert a graphic
    printlog "- insert a graphic"
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<down>"
    DocumentWriter.TypeKeys "<down>"

    Call hGrafikEinfuegen ( gTesttoolPath + "writer\optional\input\options\ga000907.gif" )

    ' see everything
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "VIEW" )

    '///' Hilfslinien.Check only, when graphic's in motion! ///'
    printlog "- check values"
    FarbigeHandles.Check
    GrosseHandles.Check
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok

    ToolsOptions
    Call hToolsOptions ( "WRITER" , "VIEW" )
    printlog "- uncheck values"
    FarbigeHandles.UnCheck
    GrosseHandles.UnCheck

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    '/// restore default values
    printlog "- set defaultsvalues in options"
    Call wOptionsUndo( "All" )

    '/// close document
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterView5
    dim option_value(17) as boolean

    Call hNewDocument
    '/// open document: writer\\optional\\input\\options\\options1.sxw
    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\options\options1.sxw" )
    
    '///- Tools/Options/Writer: View

    printlog ("'/// - save states ///")
    ToolsOptions
    Call hToolsOptions ("WRITER","VIEW")

    option_value(1) = GrafikenUndObjekte.IsChecked
    option_value(2) = Zeichnungen.IsChecked
    option_value(3) = Feldname.IsChecked
    option_value(4) = Notizen.IsChecked

    printlog ("'/// - all states inverting ///")

    if option_value(1) then GrafikenUndObjekte.UnCheck else GrafikenUndObjekte.Check
    if option_value(2) then Zeichnungen.UnCheck else Zeichnungen.Check
    if option_value(3) then Feldname.UnCheck else Feldname.Check
    if option_value(4) then Notizen.UnCheck else Notizen.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("'/// - SO quit - start ///")

    CALL wOfficeRestart

    printlog ("'/// - checking states ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "VIEW" )

    if ( option_value(1) = GrafikenUndObjekte.IsChecked ) then WarnLog "Graphics and objects state changed"
    if ( option_value(2) = Zeichnungen.IsChecked  ) then WarnLog "Zeichnungen state changed"
    if ( option_value(3) = Feldname.IsChecked  ) then  WarnLog "Feldname state changed"
    if ( option_value(4) = Notizen.IsChecked  ) then  WarnLog "Notizen state changed"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    '/// restore default values
    printlog "- set defaultsvalues in options"
    Call wOptionsUndo( "All" )

    '/// close document
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterView6
    dim option_value(17) as boolean

    Call hNewDocument
    '/// open document: writer\\optional\\input\\options\\options1.sxw
    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\options\options1.sxw" )
    
    '///- Tools/Options/Writer: View

    printlog ("'/// - save states ///")
    ToolsOptions
    Call hToolsOptions ("WRITER","VIEW")

    option_value(1) = GrafikenUndObjekte.IsChecked
    option_value(2) = Zeichnungen.IsChecked
    option_value(3) = Feldname.IsChecked
    option_value(4) = Notizen.IsChecked

    printlog ("'/// - all states unchecked ///")

    GrafikenUndObjekte.UnCheck
    Zeichnungen.UnCheck
    Feldname.UnCheck
    Notizen.UnCheck

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("'/// - SO quit - start ///")

    CALL wOfficeRestart

    printlog ("'/// - checking states ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "VIEW" )

    if GrafikenUndObjekte.IsChecked then WarnLog "Graphics and objects x"
    if Zeichnungen.IsChecked then WarnLog "Zeichnungen x"
    if Feldname.IsChecked then  WarnLog "Feldname x"
    if Notizen.IsChecked then  WarnLog "Notizen x"
    
    printlog ("'/// - restore states ///")
    if ( option_value(1) = TRUE ) Then  GrafikenUndObjekte.Check Else GrafikenUndObjekte.UnCheck
    if ( option_value(2) = TRUE ) Then  Zeichnungen.Check Else Zeichnungen.UnCheck
    if ( option_value(3) = TRUE ) Then  Feldname.Check Else Feldname.UnCheck
    if ( option_value(4) = TRUE ) Then  Notizen.Check Else Notizen.UnCheck

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    '/// restore default values
    printlog "- set defaultsvalues in options"
    Call wOptionsUndo( "All" )

    '/// close document
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterView7
    dim option_value(17) as boolean

    Call hNewDocument
    '/// open document: writer\\optional\\input\\options\\options1.sxw
    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\options\options1.sxw" )
    
    '///- Tools/Options/Writer: View

    printlog ("'/// - save states ///")
    ToolsOptions
    Call hToolsOptions ("WRITER","VIEW")

    option_value(1) = GrafikenUndObjekte.IsChecked
    option_value(2) = Zeichnungen.IsChecked
    option_value(3) = Feldname.IsChecked
    option_value(4) = Notizen.IsChecked

    printlog ("'/// - all Check -> x ///")

    GrafikenUndObjekte.Check
    Zeichnungen.Check
    Feldname.Check
    Notizen.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("'/// - SO quit - start ///")

    CALL wOfficeRestart

    printlog ("'/// - check if all Checked ///")
    ToolsOptions
    hToolsOptions ( "WRITER" , "VIEW" )

    if true <> GrafikenUndObjekte.IsChecked then WarnLog "Graphics and objects o"
    if true <> Zeichnungen.IsChecked then WarnLog "Zeichnungen o"
    if true <> Feldname.IsChecked then  WarnLog "Feldname o"
    if true <> Notizen.IsChecked then  WarnLog "Notizen o"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    '/// restore default values
'    printlog "- set defaultsvalues in options"
'    Call wOptionsUndo( "All" )

    '/// close document
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterGrid1
    dim option_value(3) as boolean
    dim sMetricField(4) as string      ' MetricField
    dim sTempMetricField(4) as string

    '/// new writer document

    Call hNewDocument

    '/// Tools/Options/Writer: Grid
    printlog "- Tools/Options/Writer: Grid"

    '/// remember values
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "GRID" )
    printlog "- remember values"
    option_value(1) = FangrasterBenutzen.IsChecked
    option_value(2) = RasterSichtbar.IsChecked
    option_value(3) = AchsenSynchronisieren.IsChecked
    
    sMetricField(1) = RasterAufloesungXAchse.GetText
    sMetricField(2) = RasterAufloesungYAchse.GetText
    sMetricField(3) = RasterUnterteilungXAchse.GetText
    sMetricField(4) = RasterUnterteilungYAchse.GetText

    '/// - all states inverting
    printlog "- all states inverting"
    
    if option_value(1) then FangrasterBenutzen.UnCheck    else FangrasterBenutzen.Check
    if option_value(2) then RasterSichtbar.UnCheck        else RasterSichtbar.Check
    'if option_value(3) then AchsenSynchronisieren.UnCheck else AchsenSynchronisieren.Check
    'this CheckBox interacts with the MetricBoxes:
    AchsenSynchronisieren.UnCheck

    sTempMetricField(1) = RasterAufloesungXAchse.GetText
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(3) = RasterUnterteilungXAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText
    RasterAufloesungXAchse.ToMax
    RasterAufloesungYAchse.ToMin
    RasterUnterteilungXAchse.ToMin
    RasterUnterteilungYAchse.ToMax
    if ( RasterAufloesungXAchse.GetText =   sTempMetricField(1) ) then RasterAufloesungXAchse.ToMin
    if ( RasterAufloesungYAchse.GetText =   sTempMetricField(2) ) then RasterAufloesungYAchse.ToMax
    if ( RasterUnterteilungXAchse.GetText = sTempMetricField(3) ) then RasterUnterteilungXAchse.ToMax
    if ( RasterUnterteilungYAchse.GetText = sTempMetricField(4) ) then RasterUnterteilungYAchse.ToMin
    sTempMetricField(1) = RasterAufloesungXAchse.GetText
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(3) = RasterUnterteilungXAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    '/// - SO quit - start
    printlog "'- SO quit - start"
    Call wOfficeRestart
    '/// - checking states
    printlog "- checking states"
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "GRID" )

    if (option_value(1) = FangrasterBenutzen.IsChecked    ) then WarnLog "FangrasterBenutzen state changed BugID: 82944 "
    if (option_value(2) = RasterSichtbar.IsChecked        ) then WarnLog "RasterSichtbar state changed"
    'if (option_value(3)=  AchsenSynchronisieren.IsChecked ) then WarnLog "AchsenSynchronisieren state changed"
    if ( AchsenSynchronisieren.IsChecked = true ) then WarnLog "AchsenSynchronisieren state changed"
    if ( RasterAufloesungXAchse.GetText <>   sTempMetricField(1) ) then WarnLog "RasterAufloesungXAchse state changed"
    if ( RasterAufloesungYAchse.GetText <>   sTempMetricField(2) ) then WarnLog "RasterAufloesungYAchse state changed"
    if ( RasterUnterteilungXAchse.GetText <> sTempMetricField(3) ) then WarnLog "RasterUnterteilungXAchse state changed"
    if ( RasterUnterteilungYAchse.GetText <> sTempMetricField(4) ) then WarnLog "RasterUnterteilungYAchse state changed"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK        
    '/// restore default values 
    printlog "- set defaultsvalues in options"
    Call wOptionsUndo( "Grid" )

    '/// close document
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterGrid2
    dim option_value(3) as boolean
    dim sMetricField(4) as string      ' MetricField
    dim sTempMetricField(4) as string

    '/// new writer document
    Call hNewDocument
    '/// Tools/Options/Writer: Grid
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "GRID" )

    '/// remember values set
    option_value(1) = FangrasterBenutzen.IsChecked
    option_value(2) = RasterSichtbar.IsChecked
    option_value(3) = AchsenSynchronisieren.IsChecked

    sMetricField(1) = RasterAufloesungXAchse.GetText
    sMetricField(2) = RasterAufloesungYAchse.GetText
    sMetricField(3) = RasterUnterteilungXAchse.GetText
    sMetricField(4) = RasterUnterteilungYAchse.GetText

    printlog "- uncheck all checkboxes and set metric fields to min"
    '/// - uncheck all checkboxes and set metric fields to min
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

    RasterAufloesungXAchse.More
    RasterAufloesungYAchse.More
    RasterUnterteilungXAchse.More
    RasterUnterteilungYAchse.More
    if ( sTempMetricField(1) = RasterAufloesungXAchse.GetText   ) then WarnLog "RasterAufloesungXAchse  More-Button not working"
    if ( sTempMetricField(2) = RasterAufloesungYAchse.GetText   ) then WarnLog "RasterAufloesungYAchse  More-Button not working"
    if ( sTempMetricField(3) = RasterUnterteilungXAchse.GetText ) then WarnLog "RasterUnterteilungXAchse More-Button not working"
    if ( sTempMetricField(4) = RasterUnterteilungYAchse.GetText ) then WarnLog "RasterUnterteilungYAchse More-Button not working"
    sTempMetricField(1) = RasterAufloesungXAchse.GetText
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(3) = RasterUnterteilungXAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    '/// SO quit - start
    printlog "- SO quit - start"

    Call wOfficeRestart

    printlog ("'/// - checking states ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "GRID" )

    if FangrasterBenutzen.IsChecked     then WarnLog "FangrasterBenutzen x"
    if RasterSichtbar.IsChecked         then WarnLog "RasterSichtbar x"
    if AchsenSynchronisieren.IsChecked  then WarnLog "AchsenSynchronisieren x"
    if ( RasterAufloesungXAchse.GetText <>   sTempMetricField(1) ) then WarnLog "RasterAufloesungXAchse is not min."
    if ( RasterAufloesungYAchse.GetText <>   sTempMetricField(2) ) then WarnLog "RasterAufloesungYAchse is not min."
    if ( RasterUnterteilungXAchse.GetText <> sTempMetricField(3) ) then WarnLog "RasterUnterteilungXAchse is not min."
    if ( RasterUnterteilungYAchse.GetText <> sTempMetricField(4) ) then WarnLog "RasterUnterteilungYAchse is not min."

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    '/// restore default values
    printlog "- set defaultsvalues in options"
    Call wOptionsUndo( "Grid" )

    '/// close document
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterGrid3
    dim option_value(3) as boolean
    dim sMetricField(4) as string      ' MetricField
    dim sTempMetricField(4) as string

    '/// New document
    Call hNewDocument
    printlog "'///- Tools/Options/Writer: Grid ///"
    '/// Tools/Options/Writer: Grid
    printlog ("'/// - save states ///")
    '/// remember values

    ToolsOptions
    Call hToolsOptions ( "WRITER" , "GRID" )
    option_value(1) = FangrasterBenutzen.IsChecked
    option_value(2) = RasterSichtbar.IsChecked
    option_value(3) = AchsenSynchronisieren.IsChecked
    
    sMetricField(1) = RasterAufloesungXAchse.GetText
    sMetricField(2) = RasterAufloesungYAchse.GetText
    sMetricField(3) = RasterUnterteilungXAchse.GetText
    sMetricField(4) = RasterUnterteilungYAchse.GetText

    printlog "- all Check"
    '/// check all values
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
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("'/// - SO quit - start ///")
    Call wOfficeRestart

    printlog ("'/// - checking states ///")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "GRID" )

    if FangrasterBenutzen.IsChecked = false     then WarnLog "FangrasterBenutzen o"
    if RasterSichtbar.IsChecked = false         then WarnLog "RasterSichtbar o"
    if AchsenSynchronisieren.IsChecked = false  then WarnLog "AchsenSynchronisieren o"
    if ( RasterAufloesungXAchse.GetText <>   sTempMetricField(1) ) then WarnLog "RasterAufloesungXAchse is not max.Bug 54934 in 01:"
    if ( RasterAufloesungYAchse.GetText <>   sTempMetricField(2) ) then WarnLog "RasterAufloesungYAchse is not max."
    if ( RasterUnterteilungXAchse.GetText <> sTempMetricField(3) ) then WarnLog "RasterUnterteilungXAchse is not max."
    if ( RasterUnterteilungYAchse.GetText <> sTempMetricField(4) ) then WarnLog "RasterUnterteilungYAchse is not max."

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    '/// restore default values
    printlog "- set defaultsvalues in options"
    Call wOptionsUndo( "Grid" )

    '/// close document
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterGrid4
    dim option_value(3) as boolean
    dim sMetricField(4) as string      ' MetricField
    dim sTempMetricField(4) as string

    '/// new writer document

    Call hNewDocument

    printlog "'///- Tools/Options/Writer: Grid ///"

    printlog ("'/// - save states ///")

    ToolsOptions
    Call hToolsOptions ( "WRITER" , "GRID" )

    printlog ("'///TabRaster all UnCheck ! ///")

    option_value(1) = FangrasterBenutzen.IsChecked
    option_value(2) = RasterSichtbar.IsChecked
    option_value(3) = AchsenSynchronisieren.IsChecked
    
    sMetricField(1) = RasterAufloesungXAchse.GetText
    sMetricField(2) = RasterAufloesungYAchse.GetText
    sMetricField(3) = RasterUnterteilungXAchse.GetText
    sMetricField(4) = RasterUnterteilungYAchse.GetText

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel

    printlog ("'/// Function Test: Grid Resolution ///'")
    '///  ( gTesttoolPath + "writer\optional\input\options\gridtst.sxw" ) ///'

    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\options\gridtst.sxw", False )
    

    ' select grafik element
    Call wNavigatorAuswahl(4,1)

    ' check if @ (0,0) Pineapples ;-)
    wait 500
    ViewNavigator
    wait 500
    FormatGraphics
    wait 500
    Kontext
    Active.SetPage TabType
    Kontext "TabType"
    wait 500
    if ( StrToDouble (HorizontalBy.GetText) <> 0 ) then WarnLog "HorizontalBy wrong presupposition != 0"
    if ( StrToDouble (VerticalBy.GetText) <> 0 ) then WarnLog   "VerticalBy   wrong presupposition != 0"
    TabType.Cancel
    sleep (3)

    ' change Grid Resolution
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "GRID" )
    AchsenSynchronisieren.Uncheck
    RasterAufloesungXAchse.SetText("2")
    RasterAufloesungYAchse.SetText("3")
    RasterUnterteilungXAchse.SetText("2")
    RasterUnterteilungYAchse.SetText("3")
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    '/// move grafik with <Alt> + <up/down> ///'
    '/// resulting step = 1 Step = 0.10cm ///'
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Mod2 Down>", 45
    sleep (3)
    DocumentWriter.TypeKeys "<Mod2 Right>", 45

    ' check if @ (1,1) Current-Measure-Unit :-)
    Call wNavigatorAuswahl(4,1)
    ViewNavigator
    sleep (3)
    try
        FormatGraphics
    catch
        printlog "why is formatgraphics not enabled ????!!! wrong focus??"
    endcatch
    Kontext "TabType"
    ' This value depends on the current screensize, so I check only if value have changed!!
    if ( 0 = StrToDouble (HorizontalBy.GetText)) then WarnLog "RasterAufloesungXAchse has not been changed !"
    if ( 0 = StrToDouble (VerticalBy.GetText)) then WarnLog   "RasterAufloesungYAchse has not been changed!"
    TabType.Cancel

    Call hCloseDocument
    Call hNewDocument

    '/// restore default values 
    printlog "- set defaultsvalues in options"
    Call wOptionsUndo( "Grid" )

    '/// close document
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------
