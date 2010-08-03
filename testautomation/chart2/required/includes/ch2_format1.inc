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
'* short description : Continue Chart resource test - Menu FORMAT
'*
'************************************************************************
'*
' #1 tFormatXAxisMainGrid
' #1 tFormatYAxisMainGrid
' #1 tFormatZAxisMainGrid
' #1 tFormatXAxisMinorGrid
' #1 tFormatYAxisMinorGrid
' #1 tFormatZAxisMinorGrid
' #1 tFormatAllAxesGrids
' #1 tFormatLegend
' #1 tFormatChartWall
' #1 tFormatChartFloor
' #1 tFormatChartArea
' #1 tFormatChartType
' #1 tFormatDataRanges
' #1 tFormat3DView
'*
'\************************************************************************

testcase tFormatXAxisMainGrid
'///<u><b>Dialog test after invoking Format::Grid::X Axis Main Grid</b></u>

'///<ul>
   '///+<li>Load simple chart document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   sleep(1)
   '///+<li>Invoke Format::Chart Type</li>
   if fInvokeChartTypeDialog() > 0 then
       warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Change Chart type to STOCK</li>
   if fSetChartType( "stock" ) > 0 then
       warnlog "Something went wrong setting chart type to STOCK -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Close Chart Type dialog</li>
   if fCloseChartTypeDialogOK() > 0 then
       warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Invoke Insert::Grids</li>
   if fInvokeGridsDialog() > 0 then
       warnlog "Something went wrong trying to invoke the Grids dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   Kontext "InsertGridsDialog"
   '///+<li>Check Main Grid X Axis</li>
   if fSetGrid( MainGridXAxis , TRUE ) > 0 then
       warnlog "Something went wrong enabling Main Grid X Axis  -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Close Grids dialog</li>
   if fCloseGridsDialogOK() > 0 then
       warnlog "Something went wrong trying to close the Grids dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Invoke Format::Grid::X Axis Main Grid</li>
   FormatGridXAxisMainGrid
   '///+<li>Check if tab-page Line exists</li>
   Kontext "TabFormatChartLine"
   call DialogTest ( TabFormatChartLine )
   '///+<li>Close dialog</li>
   TabFormatChartLine.Cancel
   '///+<li>Close document</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
'///</ul>

endcase
'-------------------------------------------------------------
testcase tFormatYAxisMainGrid
'///<u><b>Dialog test after invoking Format::Grid::Y Axis Main Grid</b></u>

'///<ul>
   '///+<li>Load simple chart document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   sleep(1)
   '///+<li>Invoke Format::Chart Type</li>
   if fInvokeChartTypeDialog() > 0 then
       warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Change Chart type to AREA</li>
   if fSetChartType( "area" ) > 0 then
       warnlog "Something went wrong setting chart type to AREA -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Close Chart Type dialog</li>
   if fCloseChartTypeDialogOK() > 0 then
       warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Invoke Format::Grid::Y Axis Main Grid</li>
   FormatGridYAxisMainGrid
   '///+<li>Check if tab-page Line exists</li>
   Kontext "TabFormatChartLine"
   call DialogTest ( TabFormatChartLine )
   '///+<li>Close dialog</li>
   TabFormatChartLine.Cancel
   '///+<li>Close document</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
'///</ul>

endcase
'-------------------------------------------------------------
testcase tFormatZAxisMainGrid
'///<u><b>Dialog test after invoking Format::Grid::Z Axis Main Grid</b></u>

'///<ul>
   '///+<li>Load simple chart document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   sleep(1)
   '///+<li>Invoke Format::Chart Type</li>
   if fInvokeChartTypeDialog() > 0 then
       warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Change Chart type to LINE</li>
   if fSetChartType( "line" ) > 0 then
       warnlog "Something went wrong setting chart type to LINE -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Change Chart type Variant to #4 (3D Variant)</li>
   if fSetChartTypeVariant( 4 ) > 0 then
       warnlog "Something went wrong selecting 4th chart type variant -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Close Chart Type dialog</li>
   if fCloseChartTypeDialogOK() > 0 then
       warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Invoke Insert::Grids</li>
   if fInvokeGridsDialog() > 0 then
       warnlog "Something went wrong trying to invoke the Grids dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   Kontext "InsertGridsDialog"
   '///+<li>Check Main Grid Z Axis</li>
   if fSetGrid( MainGridZAxis , TRUE ) > 0 then
       warnlog "Something went wrong enabling Main Grid X Axis  -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Close Grids dialog</li>
   if fCloseGridsDialogOK() > 0 then
       warnlog "Something went wrong trying to close the Grids dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Invoke Format::Grid::Z Axis Main Grid</li>
   FormatGridZAxisMainGrid
   '///+<li>Check if tab-page Line exists</li>
   Kontext "TabFormatChartLine"
   call DialogTest ( TabFormatChartLine )
   '///+<li>Close dialog</li>
   TabFormatChartLine.Cancel
   '///+<li>Close document</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
'///</ul>

endcase
'
'-------------------------------------------------------------------------------
'
testcase tFormatXAxisMinorGrid
'///<u><b>Dialog test after invoking Format::Grid::X Axis Minor Grid</b></u>
   '/// Load simple chart document
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '/// Select chart using navigator
   call fSelectFirstOLE
   '/// Invoke Edit::Object::Edit to enter Inplace Mode
   EditObjectEdit
   sleep(1)
   '/// Invoke Format::Chart Type
   if fInvokeChartTypeDialog() > 0 then
       warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Change Chart type to xy chart
   if fSetChartType( "xy" ) > 0 then
       warnlog "Something went wrong setting chart type to xy chart -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Close Chart Type dialog
   if fCloseChartTypeDialogOK() > 0 then
       warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Invoke Insert::Grids
   if fInvokeGridsDialog() > 0 then
       warnlog "Something went wrong trying to invoke the Grids dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   Kontext "InsertGridsDialog"
   '/// Check Minor Grid X Axis
   if fSetGrid( MinorGridXAxis , TRUE ) > 0 then
       warnlog "Something went wrong enabling Minor Grid Y Axis  -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Close Grids dialog
   if fCloseGridsDialogOK() > 0 then
       warnlog "Something went wrong trying to close the Grids dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Invoke Format::Grid::X Axis Minor Grid
   FormatGridXAxisMinorGrid
   '/// Check if tab-page Line exists
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
'-------------------------------------------------------------------------------
'
testcase tFormatYAxisMinorGrid
'///<u><b>Dialog test after invoking Format::Grid::Y Axis Minor Grid</b></u>

'///<ul>
   '///+<li>Load simple chart document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   sleep(1)
   '///+<li>Invoke Format::Chart Type</li>
   if fInvokeChartTypeDialog() > 0 then
       warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Change Chart type to NET</li>
   if fSetChartType( "net" ) > 0 then
       warnlog "Something went wrong setting chart type to NET -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Close Chart Type dialog</li>
   if fCloseChartTypeDialogOK() > 0 then
       warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Invoke Insert::Grids</li>
   if fInvokeGridsDialog() > 0 then
       warnlog "Something went wrong trying to invoke the Grids dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   Kontext "InsertGridsDialog"
   '///+<li>Check Minor Grid Y Axis</li>
   if fSetGrid( MinorGridYAxis , TRUE ) > 0 then
       warnlog "Something went wrong enabling Minor Grid Y Axis  -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Close Grids dialog</li>
   if fCloseGridsDialogOK() > 0 then
       warnlog "Something went wrong trying to close the Grids dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Invoke Format::Grid::Y Axis Minor Grid</li>
   FormatGridYAxisMinorGrid
   '///+<li>Check if tab-page Line exists</li>
   Kontext "TabFormatChartLine"
   call DialogTest ( TabFormatChartLine )
   '///+<li>Close dialog</li>
   TabFormatChartLine.Cancel
   '///+<li>Close document</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
'///</ul>

endcase
'
'-------------------------------------------------------------------------------
'
testcase tFormatAllAxesGrids
'///<u><b>Dialog test after invoking Format::Grid::All Grids</b></u>

'///<ul>
   '///+<li>Load simple chart document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   sleep(1)
   '///+<li>Invoke Format::Chart Type</li>
   if fInvokeChartTypeDialog() > 0 then
       warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Change Chart type to XY</li>
   if fSetChartType( "xy" ) > 0 then
       warnlog "Something went wrong setting chart type to XY -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Change Chart type Variant to #4 (3D Variant)</li>
   if fSetChartTypeVariant( 4 ) > 0 then
       warnlog "Something went wrong selecting 4th chart type variant -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Close Chart Type dialog</li>
   if fCloseChartTypeDialogOK() > 0 then
       warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Invoke Format::Grid::All Grids</li>
   FormatGridAllAxisGrids
   '///+<li>Check if tab-page Line exists</li>
   Kontext "TabFormatChartLine"
   call DialogTest ( TabFormatChartLine )
   '///+<li>Close dialog</li>
   TabFormatChartLine.Cancel
   '///+<li>Close document</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
'///</ul>

endcase
'-------------------------------------------------------------
testcase tFormatLegend
    '///<u><b>Dialog test after invoking Format::Legend</b></u>

    '///<ul>
    '///+<li>Load simple chart document</li>
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    '///+<li>Select chart using navigator</li>
    call fSelectFirstOLE
    '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
    EditObjectEdit
    sleep(1)
    '///+<li>Invoke Format::Chart Type</li>
    if fInvokeChartTypeDialog() > 0 then
        warnlog "Something went wrong trying to invoke the Chart Types dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '///+<li>Change Chart type to PIE</li>
    if fSetChartType( "pie" ) > 0 then
        warnlog "Something went wrong setting chart type to PIE -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '///+<li>Close Chart Type dialog</li>
    if fCloseChartTypeDialogOK() > 0 then
        warnlog "Something went wrong trying to close the Chart Type dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif
    '///+<li>Invoke Format::Legend</li>
    FormatLegend
    '///+<li>Goto tab-page Borders</li>
    Kontext
    active.SetPage TabFormatChartBorders
    '///+<li>Check if tab-page Borders exists</li>
    Kontext "TabFormatChartBorders"
    call DialogTest ( TabFormatChartBorders )
   
    '///+<li>Goto tab-page Area</li>
    Kontext
    active.SetPage TabArea
    '///+<li>Check if tab-page Area exists</li>
    Kontext "TabArea"
    call DialogTest ( TabArea )
   
    '///+<li>Goto tab-page Transparency</li>
    Kontext
    active.SetPage TabTransparency
    '///+<li>Check if tab-page Transparency exists</li>
    Kontext "TabTransparency"
    call DialogTest ( TabTransparency )
   
    '///+<li>Goto tab-page Characters</li>
    Kontext
    active.SetPage TabFont
    '///+<li>Check if tab-page Characters exists</li>
    Kontext "TabFont"
    call DialogTest ( TabFont )
   
    '///+<li>Goto tab-page Font Effects</li>
    Kontext
    active.SetPage TabFontEffects
    '///+<li>Check if tab-page Font Effects exists</li>
    Kontext "TabFontEffects"
    call DialogTest ( TabFontEffects )
   
    '///+<li>Goto tab-page Position</li>
    Kontext
    active.SetPage TabLegendPosition
    '///+<li>Check if tab-page Alignment exists</li>
    Kontext "TabLegendPosition"
    call DialogTest ( TabLegendPosition )
   
    if gAsianSup = true then
        Kontext
        Active.SetPage TabAsianTypography    
        Kontext "TabAsianTypography"
        call DialogTest ( TabAsianTypography )        
        TabAsianTypography.Cancel
    else   
        '///+<li>Close dialog</li>
        TabLegendPosition.Cancel
    endif
    '///+<li>Close document</li>
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"
    Call hCloseDocument
    '///</ul>

endcase
'-------------------------------------------------------------
testcase tFormatChartWall
'///<u><b>Dialog test after invoking Format::Chart Wall</b></u>

'///<ul>
   '///+<li>Load simple chart document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   sleep(1)
   '///+<li>Invoke Format::Chart Wall</li>
   FormatChartWall
   '///+<li>Goto tab-page Borders</li>
   Kontext
   active.SetPage TabFormatChartBorders
   '///+<li>Check if tab-page Borders exists</li>
   Kontext "TabFormatChartBorders"
   call DialogTest ( TabFormatChartBorders )
   
   '///+<li>Goto tab-page Area</li>
   Kontext
   active.SetPage TabArea
   '///+<li>Check if tab-page Area exists</li>
   Kontext "TabArea"
   call DialogTest ( TabArea )
   
   '///+<li>Goto tab-page Transparency</li>
   Kontext
   active.SetPage TabTransparency
   '///+<li>Check if tab-page Transparency exists</li>
   Kontext "TabTransparency"
   call DialogTest ( TabTransparency )
   
   '///+<li>Close dialog</li>
   TabTransparency.Cancel
   '///+<li>Close document</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
'///</ul>

endcase
'-------------------------------------------------------------
testcase tFormatChartFloor
'///<u><b>Dialog test after invoking Format::Chart Floor</b></u>

'///<ul>
   '///+<li>Load simple chart document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   sleep(1)
   '///+<li>Convert chart to 3D (this will enable the Chart Floor)</li>
   if NOT fConvertChartTo3D() then
       warnlog "Conversion of chart to 3D failed -> Quit testcase"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Invoke Format::Chart Floor</li>
   FormatChartFloor
   '///+<li>Goto tab-page Area</li>
   Kontext
   active.SetPage TabArea
   '///+<li>Check if tab-page Area exists</li>
   Kontext "TabArea"
   call DialogTest ( TabArea )
   
   '///+<li>Goto tab-page Transparency</li>
   Kontext
   active.SetPage TabTransparency
   '///+<li>Check if tab-page Transparency exists</li>
   Kontext "TabTransparency"
   call DialogTest ( TabTransparency )
   
   '///+<li>Goto tab-page Borders</li>
   Kontext
   active.SetPage TabFormatChartBorders
   '///+<li>Check if tab-page Borders exists</li>
   Kontext "TabFormatChartBorders"
   call DialogTest ( TabFormatChartBorders )
   
   '///+<li>Close dialog</li>
   TabFormatChartBorders.Cancel
   '///+<li>Close document</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
'///</ul>

endcase
'-------------------------------------------------------------
testcase tFormatChartArea
'///<u><b>Dialog test after invoking Format::Chart Area</b></u>

'///<ul>
   '///+<li>Load simple chart document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   sleep(1)
   '///+<li>Invoke Format::Chart Area</li>
   FormatChartArea
   '///+<li>Goto tab-page Transparency</li>
   Kontext
   active.SetPage TabTransparency
   '///+<li>Check if tab-page Transparency exists</li>
   Kontext "TabTransparency"
   call DialogTest ( TabTransparency )
   
   '///+<li>Goto tab-page Borders</li>
   Kontext
   active.SetPage TabFormatChartBorders
   '///+<li>Check if tab-page Borders exists</li>
   Kontext "TabFormatChartBorders"
   call DialogTest ( TabFormatChartBorders )
   
   '///+<li>Goto tab-page Area</li>
   Kontext
   active.SetPage TabArea
   '///+<li>Check if tab-page Area exists</li>
   Kontext "TabArea"
   call DialogTest ( TabArea )
   
   '///+<li>Close dialog</li>
   TabArea.Cancel
   '///+<li>Close document</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
'///</ul>

endcase
'-------------------------------------------------------------
testcase tFormatChartType
'///<u><b>Dialog test after invoking Format::Chart Type</b></u>

'///<ul>
   '///+<li>Load simple chart document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   sleep(1)
   '///+<li>Invoke Format::Chart Type</li>
   FormatChartType
   '///+<li>Check if Chart Type dialog exists</li>
   Kontext "ChartType"
   call DialogTest ( ChartType )
   '///+<li>Change Chart type to LINE</li>
   if fSetChartType( "line" ) > 0 then
       warnlog "Something went wrong setting chart type to LINE -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Invoke Smooth Lines Properties</li>
   if fInvokeSmoothLineProperties() > 0 then
       warnlog "Something went wrong invoking Smooth Lines Properties -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Check if the Smooth Line Properties dialog exists</li>
   Kontext "SmoothLineProperties"
   call DialogTest ( SmoothLineProperties )
   '///+<li>Close Smooth Lines Properties</li>
   if fCloseSmoothLinePropertiesOK() > 0 then
       warnlog "Something went wrong closing Smooth Lines Properties -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Close dialog</li>
   Kontext "ChartType"
   ChartType.Cancel
   '///+<li>Close document</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
'///</ul>

endcase
'-------------------------------------------------------------
testcase tFormatDataRanges
'///<u><b>Dialog test after invoking Format::Data Ranges</b></u>

'///<ul>
   '///+<li>Load simple chart document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   sleep(1)
   '///+<li>Invoke Format::Data Ranges</li>
   FormatDataRanges
   '///+<li>Goto tab-page Data Range</li>
   Kontext
   active.SetPage TabFormatDataRangesDataRange
   '///+<li>Check if tab-page Data Range exists</li>
   Kontext "TabFormatDataRangesDataRange"
   call DialogTest ( TabFormatDataRangesDataRange )
   qaErrorLog "Implode dialog using range select button when feature is implemented"
   
   '///+<li>Goto tab-page Data Series</li>
   Kontext
   active.SetPage TabFormatDataRangesDataSeries
   '///+<li>Check if tab-page Data Series exists</li>
   Kontext "TabFormatDataRangesDataSeries"
   call DialogTest ( TabFormatDataRangesDataSeries )
   MoveDown.Click
   qaErrorLog "Implode dialog using range select button when feature is implemented"
   
    '///+<li>Close dialog</li>
   TabFormatDataRangesDataSeries.Cancel
   '///+<li>Close document</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
'///</ul>

endcase
'-------------------------------------------------------------
testcase tFormat3DView
'///<u><b>Dialog test after invoking Format::3D View</b></u>

'///<ul>
   '///+<li>Load simple chart document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   sleep(1)
   '///+<li>Convert chart to 3D</li>
   if NOT fConvertChartTo3D() then
       warnlog "Conversion of chart to 3D failed -> Quit testcase"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Invoke Format::3D View</li>
   Format3DView
   '///+<li>Goto tab-page Perspective</li>
   Kontext
   active.SetPage TabFormat3DViewGeometry
   '///+<li>Check if tab-page Perspective exists</li>
   Kontext "TabFormat3DViewGeometry"
   call DialogTest ( TabFormat3DViewGeometry )
   
   '///+<li>Goto tab-page Appearance</li>
   Kontext
   active.SetPage TabFormat3DViewAppearance
   '///+<li>Check if tab-page Appearance exists</li>
   Kontext "TabFormat3DViewAppearance"
   call DialogTest ( TabFormat3DViewAppearance )
   
   '///+<li>Goto tab-page Illumination</li>
   Kontext
   active.SetPage TabFormat3DViewIllumination
   '///+<li>Check if tab-page Illumination exists</li>
   Kontext "TabFormat3DViewIllumination"
   call DialogTest ( TabFormat3DViewIllumination )
   '///+<li>Click Color select button</li>
   ColorSelect.Click
   '///+<li>Check if Color dialog exists</li>
   Kontext "FarbeDLG"
   call DialogTest ( FarbeDLG )
   '///+<li>Close dialog</li>
   FarbeDLG.Cancel
   Kontext "TabFormat3DViewIllumination"
   
   '///+<li>Click Ambient Light select button</li>
   AmbientLightSelect.Click
   '///+<li>Check if Color dialog exists</li>
   Kontext "FarbeDLG"
   call DialogTest ( FarbeDLG )
   '///+<li>Close dialog</li>
   FarbeDLG.Cancel
   
   '///+<li>Close dialog</li>
   Kontext "TabFormat3DViewIllumination"
   TabFormat3DViewIllumination.Cancel
   '///+<li>Close document</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
'///</ul>

endcase
'-------------------------------------------------------------
'

