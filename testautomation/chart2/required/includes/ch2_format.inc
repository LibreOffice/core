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
'* owner : oliver.craemer@oracle.com
'*
'* short description : Chart resource test - Menu FORMAT
'*
'************************************************************************
'*
' #1 tFormatObjectProperties
' #1 tFormatObjectPropertiesPieChart
' #1 tFormatPositonAndSize
' #1 tFormatArrangement
' #1 tFormatTitleMaintitle
' #1 tFormatTitleSubtitle
' #1 tFormatTitleXAxistitle
' #1 tFormatTitleYAxistitle
' #1 tFormatTitleZAxistitle
' #1 tFormatTitleAllTitles
' #1 tFormatAxisXAxis
' #1 tFormatAxisYAxis
' #1 tFormatAxisZAxis
' #1 tFormatAxisSecondaryXAxis
' #1 tFormatAxisSecondaryYAxis
' #1 tFormatAxisAllAxes
' #1 tFormatDateAxis
'*
'\************************************************************************

testcase tFormatObjectProperties
    Dim iCountTabPg as integer
    
    iCountTabPg = 0
        
    printlog "Load simple chart document"
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif    
    printlog "Select chart using navigator"
    if fSelectFirstOLE = 0 then        
        printlog "Edit / Object / Edit"
        EditObjectEdit        
		printlog "Select DataSeries A by toolbar selectorbox"
		call fChartSelectElement (8)        
		printlog "Open the properties dialog by using the FormatSelection button in the toolbar"
		FormatSelection.Click        
        printlog "Select tab page Options"
        Kontext
        iCountTabPg = Active.GetPageCount
        if iCountTabPg <> 4 then
            warnlog "Edit::Properties on a data series should have 4 tab pages but has " & iCountTabPg & "."            
        else             
            printlog "Edit::Properties on a data series has 4 tab pages."
        endif
        Active.SetPage TabDataSeriesOptions
        '/// Check if tab page data series options exists
        Kontext "TabDataSeriesOptions"
        call DialogTest ( TabDataSeriesOptions )
        '/// Check if tab page area exists
        Kontext
		Active.SetPage TabArea
        Kontext "TabArea"
        call DialogTest ( TabArea )
		'/// Check if tab page transparency exists
        Kontext
		Active.SetPage TabTransparency
        Kontext "TabTransparency"
        call DialogTest ( TabTransparency )
		'/// Check if tab page borders exists
        Kontext
		Active.SetPage TabFormatChartBorders
        Kontext "TabFormatChartBorders"
        call DialogTest ( TabFormatChartBorders )
        '/// Close dialog with Cancel-button
        TabFormatChartBorders.Cancel
        '/// Close document
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys "<Escape>"
    else
        warnlog "It was not possible to select the chart object!"
    endif
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tFormatObjectPropertiesPieChart
    Dim iCountTabPg as integer
    
    iCountTabPg = 0
    
'///<u><b>Dialog test after invoking Format::Object Properties... on a pie chart</b></u>
    '/// Load simple chart document    
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    '/// Select chart using navigator
    if fSelectFirstOLE = 0 then
        '/// Edit / Object / Edit
        EditObjectEdit
        '/// Move to third object in Chart using TAB key (data series)        
        Kontext "DocumentChart"
        '/// Format / Chart Type
        if fInvokeChartTypeDialog() > 0 then
            warnlog "Something went wrong trying to invoke the Chart Types dialog"
            Call hCloseDocument
            goto endsub
        endif
        if fSetChartType("pie") > 0 then
            warnlog "Something went wrong trying to set the Chart Type to 'Pie'"
            Call hCloseDocument
            goto endsub
        endif        
        if fCloseChartTypeDialogOK() > 0 then
            warnlog "Something went wrong trying to leave the dialog with OK"
            Call hCloseDocument
            goto endsub
        endif        
        '/// Move to data series in Chart
		call fChartSelectElement (5)
		printlog "Open the properties dialog by using the FormatSelection button in the toolbar"
		FormatSelection.Click   
        '/// Select tab page Aligment
        Kontext
        iCountTabPg = Active.GetPageCount
        if iCountTabPg <> 4 then
            warnlog "Edit::Properties on a data series of a pie chart should have 4 tab pages but has " & iCountTabPg & "."
        else
            printlog "Edit::Properties on a data series on a pie chart has 4 tab pages."
        endif
        Active.SetPage TabChartPieOptions
        '/// Check if tab page Options exists
        Kontext "TabChartPieOptions"
        call DialogTest ( TabChartPieOptions )        
        '/// Close dialog with Cancel-button
        TabChartPieOptions.Cancel
        '/// Close document
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys "<ESCAPE>"
    else
        warnlog "It was not possible to select the chart object!"
    endif
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------------
'
testcase tFormatPositonAndSize
'///<u><b>Dialog test after invoking Format::PositionAndSize</b></u>
    '/// Load simple chart document
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    '/// Select chart using navigator
    call fSelectFirstOLE
    '/// Edit / Object / Edit
    EditObjectEdit
    '/// Move to first object in Chart using TAB key
    Kontext "DocumentChart"
    DocumentChart.TypeKeys "<TAB>"
    '/// Invoke Format::PositionAndSize
    FormatPositionAndSizeChart
    '/// Check if Position and Size dialog exists (single tab page)
    Kontext "TabPositionAndSize"
    call DialogTest ( TabPositionAndSize )
    '/// Close dialog
    TabPositionAndSize.Cancel
    '/// Close document
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------------
'
testcase tFormatArrangement
    dim sInputFile as string            
    dim iLoop as integer
    sInputFile = convertpath(gTestToolPath & "chart2/required/input/ch_flexrange01.ods")
    printlog "Open test document gtesttoolpath/chart2/required/input/ch_flexrange01.ods"            
    if fOpenTestDocumentAndInsertChart(sInputFile) then
        Kontext "ChartType"    
        printlog "Choose the Chart type 'bar' (should be the second in the top-down list)"
        if fSetChartType( "bar" ) > 0 then
            warnlog "Something went wrong setting chart type to BAR. Testcase aborted"            
            Kontext "ChartWizard"            
            ChartWizard.Cancel
            call hCloseDocument
            goto endsub
        endif            
        Kontext "ChartWizard"
        printlog "Press OK button"
        ChartWizard.OK                    
        Kontext "DocumentChart"
        printlog "Type twice <TAB> to select a data series to enable the menu entry."
        DocumentChart.TypeKeys "<TAB>", 2, TRUE                        
        Kontext "DocumentCalc"
        DocumentCalc.UseMenu
        printlog "Select 'Format' entry in main menu"
        hMenuSelectNr(5) ' FORMAT
        printlog "Select 'Arrangement' entry in 'Format'-menu"
        hMenuSelectNr(3) ' ARRANGEMENT                                  
        printlog "Select enabled 'Send Backward' entry."
        hMenuSelectNr(2)
        Kontext "DocumentCalc"
        DocumentCalc.UseMenu
        printlog "Select 'Format' entry in main menu again"
        hMenuSelectNr(5) ' FORMAT
        printlog "Select 'Arrangement' entry in 'Format'-menu"
        hMenuSelectNr(3) ' ARRANGEMENT                                  
        printlog "Select now enabled 'Bring Forward' entry."
        hMenuSelectNr(1)                            
        printlog "Close document"
        Call hCloseDocument
    else
        warnlog "It was not possible to open the test document."
    endif    
endcase
'
'-------------------------------------------------------------------------------
'
testcase tFormatTitleMaintitle
'///<u><b>Dialog test after invoking Format::Title::MainTitle</b></u>
    '/// Load simple chart document
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    '/// Select chart using navigator
    call fSelectFirstOLE
    '/// Edit / Object / Edit
    EditObjectEdit
    sleep(1)
    '/// Format / Title / Main title
    FormatTitleMainTitle
    '/// Goto tab page Borders
    Kontext
    Active.SetPage TabFormatChartBorders
    '/// Check if tab page Borders exists
    Kontext "TabFormatChartBorders"
    call DialogTest ( TabFormatChartBorders )  
    '/// Goto tab page Area
    Kontext
    Active.SetPage TabArea
    '/// Check if tab page Area exists
    Kontext "TabArea"
    call DialogTest ( TabArea )   
    '/// Goto tab page Transparency
    Kontext
    Active.SetPage TabTransparency
    '/// Check if tab page Transparency exists
    Kontext "TabTransparency"
    call DialogTest ( TabTransparency )   
    '/// Goto tab page Characters
    Kontext
    Active.SetPage TabFont
    '/// Check if tab page Characters exists
    Kontext "TabFont"
    call DialogTest ( TabFont )   
    '/// Goto tab page Font Effects
    Kontext
    Active.SetPage TabFontEffects
    '/// Check if tab page Font Effects exists
    Kontext "TabFontEffects"
    call DialogTest ( TabFontEffects )   
    '/// Goto tab page Alignment
    Kontext
    Active.SetPage TabFormatChartTitleAlignment
    '/// Check if tab page Alignment exists
    Kontext "TabFormatChartTitleAlignment"
    call DialogTest ( TabFormatChartTitleAlignment )
    '/// Close dialog
    TabFormatChartTitleAlignment.Cancel
    '/// Close document
     Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------------
'
testcase tFormatTitleSubtitle
'///<u><b>Dialog test after invoking Format::Title::Subtitle</b></u>
    '/// Load simple chart document
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    '/// Select chart using navigator
    call fSelectFirstOLE
    '/// Edit / Object / Edit
    EditObjectEdit
    sleep(1)
    '/// Insert / Title
    if fInvokeTitlesDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Title dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    Kontext "InsertTitleDialog"
    '/// Set Subtitle
    if fSetTitle( SubTitle , "Custom Subtitle" ) > 0 then
        warnlog "Something went wrong setting the subtitle -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Close Title dialog
    if fCloseTitlesDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Title dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Format / Title / Subtitle
    FormatTitleSubTitle
    '/// Goto tab page Area
    Kontext
    Active.SetPage TabArea
    '/// Check if tab page Area exists
    Kontext "TabArea"
    call DialogTest ( TabArea )   
    '/// Goto tab page Characters
    Kontext
    Active.SetPage TabFont
    '/// Check if tab page Characters exists
    Kontext "TabFont"
    call DialogTest ( TabFont )   
    '/// Goto tab page Borders
    Kontext
    Active.SetPage TabFormatChartBorders
    '/// Check if tab page Borders exists
    Kontext "TabFormatChartBorders"
    call DialogTest ( TabFormatChartBorders )
    '/// Goto tab page Alignment
    Kontext
    Active.SetPage TabFormatChartTitleAlignment
    '/// Check if tab page Alignment exists
    Kontext "TabFormatChartTitleAlignment"
    call DialogTest ( TabFormatChartTitleAlignment )
    '/// Goto tab page Transparency
    Kontext
    Active.SetPage TabTransparency
    '/// Check if tab page Transparency exists
    Kontext "TabTransparency"
    call DialogTest ( TabTransparency )
    '/// Goto tab page Font Effects
    Kontext
    Active.SetPage TabFontEffects
    '/// Check if tab page Font Effects exists
    Kontext "TabFontEffects"
    call DialogTest ( TabFontEffects )   
    '/// Close dialog with Cancel
    TabFontEffects.Cancel
    '/// Close document
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------------
'
testcase tFormatTitleXAxistitle
'///<u><b>Dialog test after invoking Format::Title::X Axis Title</b></u>
    '/// Load simple chart document
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    '/// Select chart using navigator
    call fSelectFirstOLE
    '/// Edit / Object / Edit
    EditObjectEdit
    sleep(1)
    '/// Insert / Title
    if fInvokeTitlesDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Title dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    Kontext "InsertTitleDialog"
    '/// Set X Axis Title
    if fSetTitle( TitleXaxis , "Custom X Axis Title" ) > 0 then
        warnlog "Something went wrong setting the X Axis Title -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Close Title dialog
    if fCloseTitlesDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Title dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Format / Chart Type
    if fInvokeChartTypeDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Change Chart type to Column + Line
    if fSetChartType( "column_line" ) > 0 then
        warnlog "Something went wrong setting chart type to Column + Line -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Close Chart Type dialog
    if fCloseChartTypeDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif   
    '/// Format / Title / X Axis Title
    FormatTitleX_Axis
    '/// Goto tab page Font Effects
    Kontext
    Active.SetPage TabFontEffects
    '/// Check if tab page Font Effects exists
    Kontext "TabFontEffects"
    call DialogTest ( TabFontEffects )   
    '/// Goto tab page Transparency
    Kontext
    Active.SetPage TabTransparency
    '/// Check if tab page Transparency exists
    Kontext "TabTransparency"
    call DialogTest ( TabTransparency )
    '/// Goto tab page Borders
    Kontext
    Active.SetPage TabFormatChartBorders
    '/// Check if tab page Borders exists
    Kontext "TabFormatChartBorders"
    call DialogTest ( TabFormatChartBorders )
    '/// Goto tab page Alignment
    Kontext
    Active.SetPage TabFormatChartTitleAlignment
    '/// Check if tab page Alignment exists
    Kontext "TabFormatChartTitleAlignment"
    call DialogTest ( TabFormatChartTitleAlignment )
    '/// Goto tab page Characters
    Kontext
    Active.SetPage TabFont
    '/// Check if tab page Characters exists
    Kontext "TabFont"
    call DialogTest ( TabFont )   
    '/// Goto tab page Area
    Kontext
    Active.SetPage TabArea
    '/// Check if tab page Area exists
    Kontext "TabArea"
    call DialogTest ( TabArea )   
    '/// Close dialog
    TabArea.Cancel
    '/// Close document    
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------------
'
testcase tFormatTitleYAxistitle
'///<u><b>Dialog test after invoking Format::Title::Y Axis Title</b></u>
    '/// Load simple chart document
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    '/// Select chart using navigator
    call fSelectFirstOLE
    '/// Edit / Object / Edit
    EditObjectEdit
    sleep(1)
    '/// Insert / Title
    if fInvokeTitlesDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Title dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    Kontext "InsertTitleDialog"
    '/// Set Y Axis Title
    if fSetTitle( TitleYaxis , "Custom Y Axis Title" ) > 0 then
        warnlog "Something went wrong setting the Y Axis Title -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Close Title dialog
    if fCloseTitlesDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Title dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Format / Chart Type
    if fInvokeChartTypeDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Change Chart type to STOCK
    if fSetChartType( "stock" ) > 0 then
        warnlog "Something went wrong setting chart type to STOCK -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Close Chart Type dialog
    if fCloseChartTypeDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif   
    '/// Format / Title / Y Axis Title
    FormatTitleY_Axis
   '/// Goto tab page Transparency
    Kontext
    Active.SetPage TabTransparency
    '/// Check if tab page Transparency exists
    Kontext "TabTransparency"
    call DialogTest ( TabTransparency )
    '/// Goto tab page Alignment
    Kontext
    Active.SetPage TabFormatChartTitleAlignment
    '/// Check if tab page Alignment exists
    Kontext "TabFormatChartTitleAlignment"
    call DialogTest ( TabFormatChartTitleAlignment )
    '/// Goto tab page Area
    Kontext
    Active.SetPage TabArea
    '/// Check if tab page Area exists
    Kontext "TabArea"
    call DialogTest ( TabArea )  
    '/// Goto tab page Font Effects
    Kontext
    Active.SetPage TabFontEffects
    '/// Check if tab page Font Effects exists
    Kontext "TabFontEffects"
    call DialogTest ( TabFontEffects )   
    '/// Goto tab page Borders
    Kontext
    Active.SetPage TabFormatChartBorders
    '/// Check if tab page Borders exists
    Kontext "TabFormatChartBorders"
    call DialogTest ( TabFormatChartBorders )
    '/// Goto tab page Characters
    Kontext
    Active.SetPage TabFont
    '/// Check if tab page Characters exists
    Kontext "TabFont"
    call DialogTest ( TabFont )   
    '/// Close dialog
    TabFont.Cancel
    '/// Close document
   Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------------
'
testcase tFormatTitleZAxistitle
'///<u><b>Dialog test after invoking Format::Title::Z Axis Title</b></u>
    '/// Load simple chart document
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    '/// Select chart using navigator
    call fSelectFirstOLE
    '/// Edit / Object / Edit
    EditObjectEdit
    sleep(1)
    '/// Convert chart to 3D (this will enable all controls in Title dialog)
    if NOT fConvertChartTo3D() then
        warnlog "Conversion of chart to 3D failed -> Quit testcase"
        Call hCloseDocument
        goto endsub
    endif
    '/// Insert / Title
    if fInvokeTitlesDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Title dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    Kontext "InsertTitleDialog"
    '/// Set Z Axis Title
    if fSetTitle( TitleZaxis , "Custom Z Axis Title" ) > 0 then
        warnlog "Something went wrong setting the Z Axis Title -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Close Title dialog
    if fCloseTitlesDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Title dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Format / Chart Type
    if fInvokeChartTypeDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Change Chart type to BAR
    if fSetChartType( "bar" ) > 0 then
        warnlog "Something went wrong setting chart type to BAR -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Close Chart Type dialog
    if fCloseChartTypeDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif   
    '/// Invoke Format::Title::Z Axis Title
    FormatTitleZ_Axis
    '/// Goto tab page Alignment
    Kontext
    Active.SetPage TabFormatChartTitleAlignment
    '/// Check if tab page Alignment exists
    Kontext "TabFormatChartTitleAlignment"
    call DialogTest ( TabFormatChartTitleAlignment )   
    '/// Goto tab page Transparency
    Kontext
    Active.SetPage TabTransparency
    '/// Check if tab page Transparency exists
    Kontext "TabTransparency"
    call DialogTest ( TabTransparency )
    '/// Goto tab page Font Effects
    Kontext
    Active.SetPage TabFontEffects
    '/// Check if tab page Font Effects exists
    Kontext "TabFontEffects"
    call DialogTest ( TabFontEffects )   
    '/// Goto tab page Area
    Kontext
    Active.SetPage TabArea
    '/// Check if tab page Area exists
    Kontext "TabArea"
    call DialogTest ( TabArea )
    '/// Goto tab page Characters
    Kontext
    Active.SetPage TabFont
    '/// Check if tab page Characters exists
    Kontext "TabFont"
    call DialogTest ( TabFont )   
    '/// Goto tab page Borders
    Kontext
    Active.SetPage TabFormatChartBorders
    '/// Check if tab page Borders exists
    Kontext "TabFormatChartBorders"
    call DialogTest ( TabFormatChartBorders )
    '/// Close dialog
    TabFormatChartBorders.Cancel
    '/// Close document
   Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------------
'
testcase tFormatTitleAllTitles
'///<u><b>Dialog test after invoking Format::Title::All Titles</b></u>
    '/// Load simple chart document
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    '/// Select chart using navigator
    call fSelectFirstOLE
    '/// Edit / Object / Edit
    EditObjectEdit
    sleep(1)
    '/// Convert chart to 3D (this will enable all controls in Title dialog)
    if NOT fConvertChartTo3D() then
        warnlog "Conversion of chart to 3D failed -> Quit testcase"
        Call hCloseDocument
        goto endsub
    endif
    '/// Insert / Title
    if fInvokeTitlesDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Title dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    Kontext "InsertTitleDialog"
    '/// Set All Axis Title
    if fSetTitle( SubTitle , "Custom Subtitle" ) > 0 then
        warnlog "Something went wrong setting the subtitle -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    if fSetTitle( TitleXaxis , "Custom X Axis Title" ) > 0 then
        warnlog "Something went wrong setting the X Axis Title -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    if fSetTitle( TitleYaxis , "Custom Y Axis Title" ) > 0 then
        warnlog "Something went wrong setting the Y Axis Title -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    if fSetTitle( TitleZaxis , "Custom Z Axis Title" ) > 0 then
        warnlog "Something went wrong setting the Y Axis Title -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Close Title dialog
    if fCloseTitlesDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Title dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Format / Chart Type
    if fInvokeChartTypeDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Change Chart type to AREA
    if fSetChartType( "area" ) > 0 then
        warnlog "Something went wrong setting chart type to AREA -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '/// Close Chart Type dialog
    if fCloseChartTypeDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif   
    '/// Format / Title / All Titles
    FormatTitleAllTitles
    '/// Goto tab page Characters
    Kontext
    Active.SetPage TabFont
    '/// Check if tab page Characters exists
    Kontext "TabFont"
    call DialogTest ( TabFont )   
    '/// Goto tab page Alignment
    Kontext
    Active.SetPage TabFormatChartTitleAlignment
    '/// Check if tab page Alignment exists
    Kontext "TabFormatChartTitleAlignment"
    call DialogTest ( TabFormatChartTitleAlignment )   
    '/// Goto tab page Font Effects
    Kontext
    Active.SetPage TabFontEffects
    '/// Check if tab page Font Effects exists
    Kontext "TabFontEffects"
    call DialogTest ( TabFontEffects )   
    '/// Goto tab page Area
    Kontext
    Active.SetPage TabArea
    '/// Check if tab page Area exists
    Kontext "TabArea"
    call DialogTest ( TabArea ) 
    '/// Goto tab page Borders
    Kontext
    Active.SetPage TabFormatChartBorders
    '/// Check if tab page Borders exists
    Kontext "TabFormatChartBorders"
    call DialogTest ( TabFormatChartBorders )
    '/// Goto tab page Transparency
    Kontext
    Active.SetPage TabTransparency
    '/// Check if tab page Transparency exists
    Kontext "TabTransparency"
    call DialogTest ( TabTransparency )
    '/// Close dialog
    TabTransparency.Cancel
    '/// Close document
    Kontext "DocumentCalc"    
    DocumentCalc.TypeKeys "<Escape>"   
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------------
'
testcase tFormatAxisXAxis

	Dim bAsianLanguage as boolean
    Dim iCountTabPg as integer
    iCountTabPg = 0
    
    printlog "Load simple chart document"
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    printlog " Enable language support for CJK (Tools-Options-Language Support)"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    printlog "Select chart using navigator"
    call fSelectFirstOLE   
    printlog "Edit / Object / Edit"
    EditObjectEdit
    sleep(1)   
    printlog "Format / Chart Type"
    if fInvokeChartTypeDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif    
    printlog "Change Chart type to XY (to obtain more tabpages)"
    if fSetChartType( "xy" ) > 0 then
        warnlog "Something went wrong setting chart type to XY -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif    
    printlog "Close Chart Type dialog"
    if fCloseChartTypeDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif       
    printlog "Invoke Format::Axis::X Axis"
    FormatAxisXAxis    
    printlog "Goto tab page Line"
    Kontext
    iCountTabPg = Active.GetPageCount
    if iCountTabPg <> 8 then 
        warnlog "Format::Axis::XAxis should have 8 tab pages but has " & iCountTabPg & "."            
    else             
        printlog "Format::Axis::XAxis has 8 tab pages."
    endif    
    Active.SetPage TabFormatChartLine    
    printlog "Check if tab page Line exists"
    Kontext "TabFormatChartLine"
    call DialogTest ( TabFormatChartLine )       
    printlog "Goto tab page Characters"
    Kontext
    Active.SetPage TabFont   
    printlog "Check if tab page Characters exists"
    Kontext "TabFont"
    call DialogTest ( TabFont )     
    printlog "Goto tab page Font Effects"
    Kontext
    Active.SetPage TabFontEffects   
    printlog "Check if tab page Font Effects exists"
    Kontext "TabFontEffects"
    call DialogTest ( TabFontEffects )       
    printlog "Goto tab page Label"
    Kontext
    Active.SetPage TabChartAxisLabel   
    printlog "Check if tab page Label exists"
    Kontext "TabChartAxisLabel"
    call DialogTest ( TabChartAxisLabel )      
    printlog "Goto tab page Scale"
    Kontext
    Active.SetPage TabChartAxisScale    
    printlog "Check if tab page Scale exists"
    Kontext "TabChartAxisScale"
    call DialogTest ( TabChartAxisScale )   
    Kontext    
    Active.SetPage TabChartAxisPositioning
    printlog "Check if tab page Positiong exists"
    Kontext "TabChartAxisPositioning"
    call DialogTest ( TabChartAxisPositioning )      
    printlog "Goto tab page Numbers"
    Kontext
    Active.SetPage TabChartAxisNumbers   
    printlog "Check if tab page Numbers exists"
    Kontext "TabChartAxisNumbers"
    call DialogTest ( TabChartAxisNumbers )   
    printlog "Close dialog with cancel"
    TabChartAxisNumbers.Cancel
    printlog " Set support for CJK back to default (Tools – Options – Language Support)"
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if
    printlog "File / Close"
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------
'
testcase tFormatAxisYAxis

	Dim bAsianLanguage as boolean
    Dim iCountTabPg as integer
    Dim iCrossingEntries as integer
    Dim iCategoryEntries as integer
    Dim iPlaceLabels as integer
    Dim iPlaceMarks as integer
    
    iCountTabPg = 0
    iCrossingEntries = 0
    iCategoryEntries = 0   
    iPlaceLabels = 0 
    iPlaceMarks = 0

    printlog "Load simple chart document"
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    printlog " Enable language support for CJK (Tools-Options-Language Support)"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    printlog "Select chart using navigator"
    call fSelectFirstOLE   
    printlog "Edit / Object / Edit"
    EditObjectEdit
    sleep(1)   
    printlog "Format / Chart Type"
    if fInvokeChartTypeDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif   
    printlog "Change Chart type to BAR (to obtain more tabpages)"
    if fSetChartType( "bar" ) > 0 then
        warnlog "Something went wrong setting chart type to BAR -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif   
    printlog "Close Chart Type dialog"
    if fCloseChartTypeDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif      
    printlog "Invoke Format::Axis::Y Axis"
    FormatAxisYAxis   
    Kontext
    iCountTabPg = Active.GetPageCount
    if iCountTabPg <> 8 then 
        warnlog "Format::Axis::YAxis should have 8 tab pages but has " & iCountTabPg & "."            
    else             
        printlog "Format::Axis::YAxis has 8 tab pages."
    endif          
    printlog "Goto tab page Characters"
    Active.SetPage TabFont    
    printlog "Check if tab page Characters exists"
    Kontext "TabFont"
    call DialogTest ( TabFont )       
    printlog "Goto tab page Line"
    Kontext
    Active.SetPage TabFormatChartLine    
    printlog "Check if tab page Line exists"
    Kontext "TabFormatChartLine"
    call DialogTest ( TabFormatChartLine )       
    printlog "Goto tab page Label"
    Kontext
    Active.SetPage TabChartAxisLabel   
    printlog "Check if tab page Label exists"
    Kontext "TabChartAxisLabel"
    call DialogTest ( TabChartAxisLabel )       
    printlog "Goto tab page Font Effects"
    Kontext
    Active.SetPage TabFontEffects    
    printlog "Check if tab page Font Effects exists"
    Kontext "TabFontEffects"
    call DialogTest ( TabFontEffects )       
    printlog "Goto tab page Numbers"
    Kontext
    Active.SetPage TabChartAxisNumbers    
    printlog "Check if tab page Numbers exists"
    Kontext "TabChartAxisNumbers"
    call DialogTest ( TabChartAxisNumbers )       
    printlog "Goto tab page Scale"
    Kontext
    Active.SetPage TabChartAxisScale    
    printlog "Check if tab page Scale exists"
    Kontext "TabChartAxisScale"
    call DialogTest ( TabChartAxisScale )
    printlog "Goto tab page Positioning"    
    Kontext
    Active.SetPage TabChartAxisPositioning
    printlog "Check if tab page Positioning exists"
    Kontext "TabChartAxisPositioning"
    call DialogTest ( TabChartAxisPositioning )
    iCrossingEntries = CrossOtherAxisAt.GetItemCount
    if iCrossingEntries <> 3 then
        warnlog "'Cross other axis at'-listbox sould have three entries but has " & iCrossingEntries & "."
    else    
        printlog "'Cross other axis at'-listbox has three entries."
    endif    
    printlog "Check that the first entry in 'Cross other axis at'-listbox is selected."
    if CrossOtherAxisAt.GetSelIndex <> 1 OR AxisAtCategory.IsVisible then
        warnlog "The first entry in 'Cross other axis at'-listbox should be selected and the 'Category'-combobox should not be visible."
    else
        printlog "Select the third entry (Category) in 'Cross other axis at'-listbox"
        CrossOtherAxisAt.Select(3)
        if AxisAtCategory.IsVisible then            
            printlog "The 'Category'-combobox is visible."
            iCategoryEntries =  AxisAtCategory.GetItemCount
            if iCategoryEntries = 2 then
                printlog "The 'Category'-combobox should have two entries."
            else
                warnlog "The 'Category'-combobox should have two entries but has " & iCategoryEntries & "."
            endif
        endif
    endif
    iPlaceLabels = PlaceLabels.GetItemCount
    if iPlaceLabels <> 4 then
        warnlog "'Place labels'-listbox should have four entries but has " & iPlaceLabels & "."
    else
        printlog "'Place labels'-listbox has four entries."
        if PlaceMarks.IsEnabled then
            warnlog "'Place marks'-listbox should only enabled if 'Place labels'-listbox has been set to 4th entry."
        else
            printlog "Select fourth entry in 'Place labels'-listbox."
            PlaceLabels.Select(4)
            if PlaceMarks.IsEnabled then
                printlog "'Place marks'-listbox is enabled if fourth entry of 'Place labels'-listbox has been selected."
                iPlaceMarks = PlaceMarks.GetItemCount
                if iPlaceMarks = 3 then
                    printlog "'Place marks'-listbox has three entries."
                else
                    warnlog "'Place marks'-listbox has not three entries. It has " & iPlaceMarks & " entries."
                endif
            else
                warnlog "'Place marks'-listbox is not enabled if fourth entry of 'Place labels'-listbox has been selected." 
            endif
        endif        
    endif
    Kontext "TabChartAxisPositioning"
    printlog "Leave dialog with Cancel."
    TabChartAxisPositioning.Cancel
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if
    printlog "File / Close"   
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------
'
testcase tFormatAxisZAxis
'///<u><b>Dialog test after invoking Format::Axis::Z Axis</b></u>


   '/// Load simple chart document
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '/// Select chart using navigator
   call fSelectFirstOLE
   '/// Edit / Object / Edit
   EditObjectEdit
   sleep(1)
   '/// Format / Chart Type
   if fInvokeChartTypeDialog() > 0 then
       warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Change Chart type to AREA (to obtain more tabpages)
   if fSetChartType( "area" ) > 0 then
       warnlog "Something went wrong setting chart type to AREA -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Close Chart Type dialog
   if fCloseChartTypeDialogOK() > 0 then
       warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Convert chart to 3D (this will enable the Z Axis)
   if NOT fConvertChartTo3D() then
       warnlog "Conversion of chart to 3D failed -> Quit testcase"
       Call hCloseDocument
       goto endsub
   endif
   
   '/// Invoke Format::Axis::Z Axis
   FormatAxisZAxis
   '/// Goto tab page Font Effects
   Kontext
   Active.SetPage TabFontEffects
   '/// Check if tab page Font Effects exists
   Kontext "TabFontEffects"
   call DialogTest ( TabFontEffects )
   
   '/// Goto tab page Characters
   Kontext
   Active.SetPage TabFont
   '/// Check if tab page Characters exists
   Kontext "TabFont"
   call DialogTest ( TabFont )
   
   '/// Goto tab page Label
   Kontext
   Active.SetPage TabChartAxisLabel
   '/// Check if tab page Label exists
   Kontext "TabChartAxisLabel"
   call DialogTest ( TabChartAxisLabel )
   
   '/// Goto tab page Line
   Kontext
   Active.SetPage TabFormatChartLine
   '/// Check if tab page Line exists
   Kontext "TabFormatChartLine"
   call DialogTest ( TabFormatChartLine )
   
   '/// Close dialog
   TabFormatChartLine.Cancel
   '/// Close document
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument


endcase
'
'-------------------------------------------------------------
'
testcase tFormatAxisSecondaryXAxis
    
	Dim bAsianLanguage as boolean
	Dim iCountTabPg as integer
    iCountTabPg = 0

    printlog "Load simple chart document"
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
	printlog " Enable language support for CJK (Tools-Options-Language Support)"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    printlog "Select chart using navigator"
    call fSelectFirstOLE   
    printlog "Edit / Object / Edit"
    EditObjectEdit
    sleep(1)    
    printlog "Format / Chart Type"
    if fInvokeChartTypeDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif    
    printlog "Change Chart type to XY (to obtain more tabpages)"
    if fSetChartType( "xy" ) > 0 then
        warnlog "Something went wrong setting chart type to XY -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif    
    printlog "Close Chart Type dialog"
    if fCloseChartTypeDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif    
    printlog "Invoke Insert::Axes"
    if fInvokeAxesDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Axes dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    Kontext "InsertAxesDialog"    
    printlog "Check Secondary X Axis Title"
    if fSetAxis( SecondaryXAxis , TRUE ) > 0 then
        warnlog "Something went wrong enabling the Secondary X Axis  -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif    
    printlog "Close Axes dialog"
    if fCloseAxesDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Axes dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif       
    printlog "Invoke Format::Axis::Secondary X Axis"
    FormatAxisSecondaryXAxis        
    printlog "Goto tab page Line"
    Kontext
    iCountTabPg = Active.GetPageCount
    if iCountTabPg <> 8 then 
        warnlog "Format::Axis::XAxis should have 8 tab pages but has " & iCountTabPg & "."            
    else             
        printlog "Format::Axis::XAxis has 8 tab pages."
    endif    
    Active.SetPage TabFormatChartLine    
    printlog "Check if tab page Line exists"
    Kontext "TabFormatChartLine"
    call DialogTest ( TabFormatChartLine )       
    printlog "Goto tab page Characters"
    Kontext
    Active.SetPage TabFont   
    printlog "Check if tab page Characters exists"
    Kontext "TabFont"
    call DialogTest ( TabFont )     
    printlog "Goto tab page Font Effects"
    Kontext
    Active.SetPage TabFontEffects   
    printlog "Check if tab page Font Effects exists"
    Kontext "TabFontEffects"
    call DialogTest ( TabFontEffects )       
    printlog "Goto tab page Label"
    Kontext
    Active.SetPage TabChartAxisLabel   
    printlog "Check if tab page Label exists"
    Kontext "TabChartAxisLabel"
    call DialogTest ( TabChartAxisLabel )      
    printlog "Goto tab page Scale"
    Kontext
    Active.SetPage TabChartAxisScale    
    printlog "Check if tab page Scale exists"
    Kontext "TabChartAxisScale"
    call DialogTest ( TabChartAxisScale )   
    Kontext    
    Active.SetPage TabChartAxisPositioning
    printlog "Check if tab page Positiong exists"
    Kontext "TabChartAxisPositioning"
    call DialogTest ( TabChartAxisPositioning )      
    printlog "Goto tab page Numbers"
    Kontext
    Active.SetPage TabChartAxisNumbers   
    printlog "Check if tab page Numbers exists"
    Kontext "TabChartAxisNumbers"
    call DialogTest ( TabChartAxisNumbers )   
    printlog "Close dialog with cancel"
    TabChartAxisNumbers.Cancel
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if
    printlog "File / Close"
    Call hCloseDocument   
endcase
'
'-------------------------------------------------------------
'
testcase tFormatAxisSecondaryYAxis
    
	Dim bAsianLanguage as boolean
	Dim iCountTabPg as integer
    iCountTabPg = 0

    printlog "Load simple chart document"
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    printlog " Enable language support for CJK (Tools-Options-Language Support)"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    printlog "Select chart using navigator"
    call fSelectFirstOLE    
    printlog "Edit / Object / Edit"
    EditObjectEdit
    sleep(1)    
    printlog "Invoke Insert::Axes"
    if fInvokeAxesDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Axes dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    Kontext "InsertAxesDialog"    
    printlog "Check Secondary Y Axis"
    if fSetAxis( SecondaryYAxis , TRUE ) > 0 then
        warnlog "Something went wrong enabling the Secondary Y Axis  -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif    
    printlog "Leave Axes dialog with OK"
    if fCloseAxesDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Axes dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif       
    printlog "Invoke Format::Axis::Secondary Y Axis"
    FormatAxisSecondaryYAxis
    printlog "Goto tab page Line"
    Kontext
    iCountTabPg = Active.GetPageCount
    if iCountTabPg <> 8 then 
        warnlog "Format::Axis::XAxis should have 8 tab pages but has " & iCountTabPg & "."            
    else             
        printlog "Format::Axis::XAxis has 8 tab pages."
    endif    
    Active.SetPage TabFormatChartLine    
    printlog "Check if tab page Line exists"
    Kontext "TabFormatChartLine"
    call DialogTest ( TabFormatChartLine )       
    printlog "Goto tab page Characters"
    Kontext
    Active.SetPage TabFont   
    printlog "Check if tab page Characters exists"
    Kontext "TabFont"
    call DialogTest ( TabFont )     
    printlog "Goto tab page Font Effects"
    Kontext
    Active.SetPage TabFontEffects   
    printlog "Check if tab page Font Effects exists"
    Kontext "TabFontEffects"
    call DialogTest ( TabFontEffects )       
    printlog "Goto tab page Label"
    Kontext
    Active.SetPage TabChartAxisLabel   
    printlog "Check if tab page Label exists"
    Kontext "TabChartAxisLabel"
    call DialogTest ( TabChartAxisLabel )      
    printlog "Goto tab page Scale"
    Kontext
    Active.SetPage TabChartAxisScale    
    printlog "Check if tab page Scale exists"
    Kontext "TabChartAxisScale"
    call DialogTest ( TabChartAxisScale )   
    Kontext    
    Active.SetPage TabChartAxisPositioning
    printlog "Check if tab page Positiong exists"
    Kontext "TabChartAxisPositioning"
    call DialogTest ( TabChartAxisPositioning )      
    printlog "Goto tab page Numbers"
    Kontext
    Active.SetPage TabChartAxisNumbers   
    printlog "Check if tab page Numbers exists"
    Kontext "TabChartAxisNumbers"
    call DialogTest ( TabChartAxisNumbers )   
    printlog "Close dialog with cancel"
    TabChartAxisNumbers.Cancel
    printlog " Set support for CJK back to default (Tools – Options – Language Support)"
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if
    printlog "File / Close"
    Call hCloseDocument   
endcase

'-------------------------------------------------------------
testcase tFormatAxisAllAxes
'///<u><b>Dialog test after invoking Format::Axis::All Axes</b></u>


   '/// Load simple chart document
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '/// Select chart using navigator
   call fSelectFirstOLE
   '/// Edit / Object / Edit
   EditObjectEdit
   sleep(1)
   '/// Format / Chart Type
   if fInvokeChartTypeDialog() > 0 then
       warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Change Chart type to NET
   if fSetChartType( "net" ) > 0 then
       warnlog "Something went wrong setting chart type to NET -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Close Chart Type dialog
   if fCloseChartTypeDialogOK() > 0 then
       warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Invoke Format::Axis::All Axes
   FormatAxisAllAxes
   '/// Goto tab page Characters
   Kontext
   Active.SetPage TabFont
   '/// Check if tab page Characters exists
   Kontext "TabFont"
   call DialogTest ( TabFont )
   
   '/// Goto tab page Font Effects
   Kontext
   Active.SetPage TabFontEffects
   '/// Check if tab page Font Effects exists
   Kontext "TabFontEffects"
   call DialogTest ( TabFontEffects )
   
   '/// Goto tab page Line
   Kontext
   Active.SetPage TabFormatChartLine
   '/// Check if tab page Line exists
   Kontext "TabFormatChartLine"
   call DialogTest ( TabFormatChartLine )
   
   '/// Goto tab page Label
   Kontext
   Active.SetPage TabChartAxisLabel
   '/// Check if tab page Label exists
   Kontext "TabChartAxisLabel"
   call DialogTest ( TabChartAxisLabel )
   
   '/// Close dialog
   TabChartAxisLabel.Cancel
   '/// Close document
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument


endcase
'
'-------------------------------------------------------------
'
testcase tFormatDateAxis

    printlog "Load simple chart document"
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    printlog " Select chart using navigator"
    call fSelectFirstOLE
    printlog "Edit / Object / Edit"
    EditObjectEdit
    sleep(1)
    printlog "Invoke Format::Axis::X Axis"
    FormatAxisXAxis
    printlog "Goto tab page Scale"
    Kontext
    Active.SetPage TabChartAxisScale
    Kontext "TabChartAxisScale"
    printlog " Switch axis type to Date"
    AxisType.Select (3)
    printlog " Verify that the date controls are available"
    if MinorIntervalUnit.exists then
        call DialogTest ( TabChartAxisScale,2 )
        else
            warnlog "Datecontrols are not available"
    end if
    printlog " Close dialog"
    TabChartAxisScale.Cancel
    printlog "Close document"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"
    Call hCloseDocument

endcase
