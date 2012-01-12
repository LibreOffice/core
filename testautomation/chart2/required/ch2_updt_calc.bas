'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
'*
'* short description : Resource Test for all chart specific menues
'*
'\************************************************************************************************

sub main
    use "chart2\tools\ch_tools_common.inc"
    use "chart2\tools\ch_tools_chart_type.inc"
    use "chart2\tools\ch_tools_titles.inc"
    use "chart2\tools\ch_tools_axes.inc"
    use "chart2\tools\ch_tools_grids.inc"
    use "chart2\tools\ch_tools_legend.inc"
    use "chart2\tools\ch_tools_data_labels.inc"
    use "chart2\tools\ch_tools_statistics.inc"
    use "chart2\tools\ch_tools_tab_pages.inc"
    use "chart2\tools\ch_tools_select.inc"
    use "chart2\required\includes\ch2_wizard.inc"
    use "chart2\required\includes\ch2_file.inc"
    use "chart2\required\includes\ch2_view.inc"
    use "chart2\required\includes\ch2_insert_ctl.inc"
    use "chart2\required\includes\ch2_insert.inc"
    use "chart2\required\includes\ch2_format.inc"
    use "chart2\required\includes\ch2_format1.inc"
    use "chart2\required\includes\ch2_window.inc"
    use "chart2\required\includes\ch2_toolbars.inc"
    use "chart2\required\includes\ch2_menu.inc"
    use "chart2\required\includes\ch2_ole.inc"

    Call hStatusIn("Chart2", "ch2_updt_calc.bas")

    printlog "----------------------------------------"
    printlog "C h a r t 2   i n  S p r e a d s h e e t"
    printlog "           U p d a t e t e s t"
    printlog "----------------------------------------"
    printLog Chr(13) + "-----------  Chart2 Wizard  ------------"
    Call tCreateNewChart
    printLog Chr(13) + "-----------  Chart2 Menu  ------------"
    call tMainMenu        
    printLog Chr(13) + "----------  Chart2 File Menu  ----------"
    call tFileClose
    call tFileSaveAsOTS
    call tFileSaveAsSXC
    call tFileSaveAsSTC
    call tFileSaveAsXLS
    call tFileSaveAsXLT
    call tFileReload                
    printLog Chr(13) + "----------  Chart2 View Menu -----------"
    Call tViewToolbars
    Call tViewToolbarsCustomize
    Call tViewFullScreen              
    printLog Chr(13) + "--------  Chart2 Insert Menu  ----------"
    call tInsertTitle
    call tInsertAxes
    call tInsertGrids
    call tInsertLegend
    call tInsertDataLabels
    call tInsertTrendLines
    call tInsertMeanValueLines
    call tInsertYErrorBars
    call tInsertSpecialCharacter
    printLog Chr(13) + "-------- Chart2 CTL Features  ----------"
    call tTitleTextDirection
    call tLegendTextDirection
    call tXAxisTextDirection
    call tDataSeriesTextDirection
    call tEquationTextDirection
    call tDataPointTextDirection
    printlog Chr(13) + "---------  Chart2 Format Menu  ---------"
    call tFormatObjectProperties
    call tFormatObjectPropertiesPieChart
    call tFormatPositonAndSize
    call tFormatArrangement
    Call tFormatTitleMaintitle
    Call tFormatTitleSubtitle
    Call tFormatTitleXAxistitle
    Call tFormatTitleYAxistitle
    Call tFormatTitleZAxistitle
    Call tFormatTitleAllTitles
    Call tFormatAxisXAxis
    Call tFormatAxisYAxis
    Call tFormatAxisZAxis
    Call tFormatAxisSecondaryXAxis
    Call tFormatAxisSecondaryYAxis
    Call tFormatAxisAllAxes
    Call tFormatDateAxis
    Call tFormatXAxisMainGrid
    Call tFormatYAxisMainGrid
    Call tFormatZAxisMainGrid
    call tFormatXAxisMinorGrid
    Call tFormatYAxisMinorGrid
    Call tFormatAllAxesGrids
    Call tFormatLegend
    Call tFormatChartWall
    Call tFormatChartFloor
    Call tFormatChartArea
    Call tFormatChartType
    call tFormatDataRanges
    Call tFormat3DView                
    printlog Chr(13) + "---------  Chart2 Window Menu  ---------"
    Call tWindowNewWindowAndClose
    printLog Chr(13) + "----------  Chart2 Toolbars  -----------"      
    Call tDrawBar
    printLog Chr(13) + "--------- Chart2 as OLE object ---------"
    Call tChartOLE("WRITER")
    Call tChartOLE("CALC")
    Call tChartOLE("IMPRESS")
    Call tChartOLE("DRAW")
    Call tChartOLE("HTML")
    Call tChartOLE("MASTERDOCUMENT")       
    Call hStatusOut
    
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles             
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_set_standard_controls.inc"
    use "global\tools\includes\optional\t_ole.inc"
    Call GetUseFiles 
    gApplication = "CALC"
end sub
