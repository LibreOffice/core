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
'* short description : Chart resource test - Menu INSERT
'*
'************************************************************************
'*
' #1 tInsertTitle
' #1 tInsertAxes
' #1 tInsertGrids
' #1 tInsertLegend
' #1 tInsertDataLabels
' #1 tInsertTrendLines
' #1 tInsertMeanValueLines
' #1 tInsertYErrorBars
' #1 tInsertSpecialCharacter
'*
'\************************************************************************

testcase tInsertTitle
'///<u><b>Dialog test after invoking Insert::Title</b></u>
   '/// Load simple chart document
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '/// Select chart using navigator
   call fSelectFirstOLE
   '/// Invoke Edit::Object::Edit to enter Inplace Mode
   EditObjectEdit
   '/// Convert chart to 3D (this will enable all controls in Titles dialog)
   if NOT fConvertChartTo3D() then
       warnlog "Conversion of chart to 3D failed -> Quit testcase"
       Call hCloseDocument
       goto endsub
   endif
   '/// Invoke Insert::Titles
   if fInvokeTitlesDialog() > 0 then
       warnlog "Something went wrong trying to invoke Titles dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Check existence of dialog
   Kontext "InsertTitleDialog"
   Call DialogTest ( InsertTitleDialog )
   '/// Quit dialog using Cancel button
   InsertTitleDialog.Cancel
   '/// Close document
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tInsertAxes
'///<u><b>Dialog test after invocation of Insert::Axes</b></u>
   '/// Load simple test document
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '/// Select chart using navigator
   call fSelectFirstOLE
   '/// Invoke Edit::Object::Edit to enter Inplace Mode
   EditObjectEdit
   '/// Invoke Insert::Axes
   if fInvokeAxesDialog() > 0 then
       warnlog "Something went wrong trying to invoke Axes dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Check existence of dialog
   Kontext "InsertAxesDialog"
   Call DialogTest ( InsertAxesDialog )
   '/// Quit dialog using Cancel button
   InsertAxesDialog.Cancel
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   '/// Close document
   Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tInsertGrids
'///<u><b>Dialog test after invocation of Insert::Grids</b></u>
   '/// Load simple test document
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '/// Select chart using navigator
   call fSelectFirstOLE
   '/// Invoke Edit::Object::Edit to enter Inplace Mode
   EditObjectEdit
   '/// Invoke Insert::Grids
   if fInvokeGridsDialog() > 0 then
       warnlog "Something went wrong trying to invoke Grids dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Check existence of dialog
   Kontext "InsertGridsDialog"
   Call DialogTest ( InsertGridsDialog )
   '/// Quit dialog using Cancel button
   InsertGridsDialog.Cancel
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   '/// Close document
   Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tInsertLegend
'///<u><b>Dialog test after invocation of Insert::Legend</b></u>
  '/// Load simple test document
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '/// Select chart using navigator
   call fSelectFirstOLE
   '/// Invoke Edit::Object::Edit to enter Inplace Mode
   EditObjectEdit
   '/// Invoke Insert::Legend
   if fInvokeLegendDialog() > 0 then
       warnlog "Something went wrong trying to invoke Legend dialog -> Check this out! (Previous log may help you)"
       Call hCloseDocument
       goto endsub
   endif
   '/// Check existence of dialog
   Kontext "InsertLegendDialog"
   Call DialogTest ( InsertLegendDialog )
   '/// Quit dialog using Cancel button
   InsertLegendDialog.Cancel
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   '/// Close document
   Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tInsertDataLabels
'///<u><b>Dialog test after invocation of Insert::Data Labels</b></u>
  '/// Load simple test document
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '/// Select chart using navigator
   call fSelectFirstOLE
   '/// Invoke Edit::Object::Edit to enter Inplace Mode
   EditObjectEdit
   '/// Invoke Insert::Data Labels
   if fInvokeDataLabelsDialog() > 0 then
       warnlog "Something went wrong trying to invoke Labels dialog -> Check this out!"
       Call hCloseDocument
       goto endsub
   endif
   '/// Check 'Show Value' to enable all controls
   Kontext "InsertLabelsDialog"
   if fSetShowValue ( TRUE ) > 0 then
       warnlog "Something went wrong trying to check 'Show Value' -> Check this out!"
   endif
   '/// Check existence of dialog
   Call DialogTest ( InsertLabelsDialog )
   '/// Quit dialog using Cancel button
   InsertLabelsDialog.Cancel
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   '/// Close document
   Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tInsertTrendLines
'///<u><b>Dialog test after invocation of Insert::TrendLines</b></u>
   '/// Load simple test document
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '/// Select chart using navigator
   call fSelectFirstOLE
   '/// Invoke Edit::Object::Edit to enter Inplace Mode
   EditObjectEdit
   Kontext "DocumentChart"
   printlog "Insert TrendLines"
   InsertTrendlines
   '/// Check existence of dialog
   Kontext "InsertTrendlinesDialog"
   Call DialogTest ( InsertTrendlinesDialog )
   '/// Quit dialog using Cancel button
   InsertTrendlinesDialog.Cancel
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   '/// Close document
   Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tInsertMeanValueLines
'///<u><b>Testing Insert::MeanValueLines</b></u>
   '/// Load simple test document
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif   
   '/// Select chart using navigator
   call fSelectFirstOLE
   '/// Invoke Edit::Object::Edit to enter Inplace Mode
   EditObjectEdit
   '/// Insert::MeanValueLines
   if fInsertMeanValueLines() > 0 then
       warnlog "Something went wrong trying to insert mean value lines -> Check this out!"
       Call hCloseDocument
       goto endsub
   endif
   Kontext "DocumentChart"   
   '/// Close document
   Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tInsertYErrorBars
'///<u><b>Dialog test after invocation of Insert::YErrorBars</b></u>
   '/// Load simple test document
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '/// Select chart using navigator
   call fSelectFirstOLE
   '/// Invoke Edit::Object::Edit to enter Inplace Mode
   EditObjectEdit
   '/// Invoke Insert::TrendLines
   if fInvokeInsertYErrorBarsDialog() > 0 then
       warnlog "Something went wrong trying to invoke YErrorBars Lines dialog -> Check this out!"
       Call hCloseDocument
       goto endsub
   endif
   '/// Check existence of dialog
   Kontext "InsertYErrorBarsDLG"
   Call DialogTest ( InsertYErrorBarsDLG )
   '/// Quit dialog using Cancel button
   InsertYErrorBarsDLG.Cancel
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   '/// Close document
   Call hCloseDocument
endcase
'
'-------------------------------------------------------------
'
testcase tInsertSpecialCharacter   
    printlog "Load simple chart document"
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif   
    printlog "Select chart using navigator"
    call fSelectFirstOLE   
    printlog "Invoke Edit::Object::Edit to enter Inplace Mode"
    EditObjectEdit   
    printlog "Invoke Insert::Titles"
    if fInvokeTitlesDialog() > 0 then
        warnlog "Something went wrong trying to invoke Titles dialog -> Check this out! (Previous log may help you)"
        Call hCloseDocument
        goto endsub
    endif    
    Kontext "InsertTitleDialog"
    if fSetTitle(MainTitle , "Test title") = 0 then
        Kontext "InsertTitleDialog"
        InsertTitleDialog.OK
        Kontext "DocumentChart"
        printlog "Type <TAB> to select the title"
        DocumentChart.TypeKeys "<TAB>" , TRUE
        DocumentChart.UseMenu
        printlog "Select menu 'Insert'"
        hMenuSelectNr(4) 'INSERT
        printlog "Select menu item 'Special Character'."
        hMenuSelectNr(9) 'SPECIAL CHARACTER
        printlog "Close the 'Insert Special Character'-dialog."
        Kontext "Sonderzeichen"
        Call Dialogtest( Sonderzeichen )
        Sonderzeichen.CANCEL
    else
        warnlog "It was not possible to set the title. Test aborted!"
        Kontext "InsertTitleDialog"
        InsertTitleDialog.Cancel
    endif
    Call hCloseDocument   
endcase

