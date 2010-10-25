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
'* short description : Chart resource test - CTL features
'*
'************************************************************************
'*
' #1 tTitleTextDirection
' #1 tLegendTextDirection
' #1 tXAxisTextDirection
' #1 tDataSeriesTextDirection
' #1 tEquationTextDirection
' #1 tDataPointTextDirection
' #0 sValidateTextDirection
'*
'\************************************************************************

testcase tTitleTextDirection    
    Dim bCTLSupBefore as boolean
        
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    else
        printlog "Load test document and save it local as ../user/work/verySimpleChart.ods"    
    endif
    printlog "Enable the complex text layout (CTL) support under Tools / Options / Language Settings / Languages / Enhanced language support"
    bCTLSupBefore = ActiveDeactivateCTLSupport(TRUE)     
    printlog "Select the first OLE object in the document."
    call fSelectFirstOLE
    printlog "Edit / Object / Edit"    
    EditObjectEdit
	Kontext "DocumentChart"
	printlog "Select the Main Title by using the ChartElememtSelectorListBox in the toolbar (4th entry)"
    call fChartSelectElement (4)
    printlog "Open the properties dialog by using the FormatSelection button in the toolbar"
    FormatSelection.Click
    Kontext
    printlog "Select the tab page Alignment"    
    Active.SetPage TabFormatChartTitleAlignment
    Kontext "TabFormatChartTitleAlignment"
    call DialogTest(TabFormatChartTitleAlignment)
    sValidateTextDirection(TextDirection)
    TabFormatChartTitleAlignment.Cancel
    printlog "Leave the tab page dialog with Cancel"
    if bCTLSupBefore <> gCTLSup then
        Call ActiveDeactivateCTLSupport(bCTLSupBefore)
        printlog "Reset the state of CTL support to the default before this test case has been executed"
    endif
    printlog "File / Close"
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tLegendTextDirection
    Dim bCTLSupBefore as boolean
    
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    else
        printlog "Load test document and save it local as ../user/work/verySimpleChart.ods"    
    endif
    printlog "Enable the complex text layout (CTL) support under Tools / Options / Language Settings / Languages / Enhanced language support"
    bCTLSupBefore = ActiveDeactivateCTLSupport(TRUE)
    call fSelectFirstOLE
    printlog "Select the first OLE object"
    EditObjectEdit
    printlog "Edit / Object Edit"
    printlog "Format / Legend"
    FormatLegend
    Kontext
    printlog "Select the tab page Position"
    Active.SetPage TabLegendPosition  
    Kontext "TabLegendPosition"    
    call DialogTest(TabLegendPosition)
    sValidateTextDirection(TextDirection)
    TabLegendPosition.Cancel
    if bCTLSupBefore <> gCTLSup then
        Call ActiveDeactivateCTLSupport(bCTLSupBefore)
        printlog "Reset the state of CTL support to the default before this test case has been executed"
    endif
    printlog "File / Close"
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tXAxisTextDirection
    Dim bCTLSupBefore as boolean

    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    else
        printlog "Load test document and save it local as ../user/work/verySimpleChart.ods"    
    endif
    printlog "Enable the complex text layout (CTL) support under Tools / Options / Language Settings / Languages / Enhanced language support"
    bCTLSupBefore = ActiveDeactivateCTLSupport(TRUE)
    printlog "Select the first OLE object"
    call fSelectFirstOLE    
    printlog "Edit / Object Edit"
    EditObjectEdit
    printlog "Format / Axis / X Axis"    
    FormatAxisXAxis   
    Kontext
    printlog "Select the tab page Label"
    Active.SetPage TabChartAxisLabel
    Kontext "TabChartAxisLabel"
    call DialogTest(TabChartAxisLabel)    
    sValidateTextDirection(TextDirection)    
    TabChartAxisLabel.Cancel
    printlog "Leave the tab dialog with Cancel"
    if bCTLSupBefore <> gCTLSup then
        Call ActiveDeactivateCTLSupport(bCTLSupBefore)
        printlog "Reset the state of CTL support to the default before this test case has been executed"
    endif
    printlog "File / Close"
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tDataSeriesTextDirection
    Dim bCTLSupBefore as boolean
    Dim oControlToCheck as object
    Dim oControlToCheck2 as object
    Dim oControlToCheck3 as object
    Dim iCombination as integer
    Dim sText as string

    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    else
        printlog "Load test document and save it local as ../user/work/verySimpleChart.ods"    
    endif
    printlog "Enable the complex text layout (CTL) support under Tools / Options / Language Settings / Languages / Enhanced language support"
    bCTLSupBefore = ActiveDeactivateCTLSupport(TRUE)
    call fSelectFirstOLE
    printlog "Select the first OLE object"
    printlog "Edit / Object Edit"
    EditObjectEdit
    printlog "Select DataSeries A by toolbar selectorbox"
	call fChartSelectElement (8) 
	printlog "Insert DataLabels"
	Kontext "DocumentChart"
	InsertDataLabels
    Kontext
    Active.Setpage TabDataSeriesLabels
    Kontext "TabDataSeriesLabels"    
    for iCombination = 1 to 7
        select case iCombination
            'Check-box combinations with 1, 2 or 3 enabled check-boxes.
            case 1      :   oControlToCheck = ShowValue
                            sText = "Show value as number"
            case 2      :   oControlToCheck = AsPercentage
                            sText = "Show value as percentage"
            case 3      :   oControlToCheck = ShowLabelText
                            sText = "Show category"
            
            case 4      :   oControlToCheck = ShowValue
                            oControlToCheck2 = AsPercentage
                            sText = "Show value as number and Show value as percentage"
            case 5      :   oControlToCheck = ShowValue
                            oControlToCheck2 = ShowLabelText
                            sText = "Show value as number Show category"
            case 6      :   oControlToCheck2 = AsPercentage
                            oControlToCheck2 = ShowLabelText
                            sText = "Show value as percentage and Show category"
            
            case 7      :   oControlToCheck = ShowValue
                            oControlToCheck2 = AsPercentage
                            oControlToCheck3 = ShowLabelText
                            sText = "Show value as number and Show value as percentage and Show category"  
                            
            case else   :   warnlog "Wrong combination entered. Exiting!"
                            TabDataSeriesLabels.Cancel
                            call hCloseDocument
                            goto endsub
        end select
        'One checked check-box
        if iCombination > 0 AND iCombination < 4 then
            if fSetCheckBox(oControlToCheck, TRUE) > 0 then
                warnlog "Setting check-box '" & sText & "' failed. Exiting test case"
                TabDataSeriesLabels.Cancel
                call hCloseDocument
                goto endsub
            else
                printlog "Check '" & sText & "' and check that text direction-list-box contains 3 entries."
                sValidateTextDirection(TextDirection)
            endif
        endif
        'Two checked check-boxes
        if iCombination > 3 AND iCombination < 7 then
            if fSetCheckBox(oControlToCheck, TRUE) > 0 AND fSetCheckBox(oControlToCheck2, TRUE) > 0 then
                warnlog "Setting check-box '" & sText & "' failed. Exiting test case"
                TabDataSeriesLabels.Cancel
                call hCloseDocument
                goto endsub
            else
                printlog "Check '" & sText & "' and check that text direction-list-box contains 3 entries."
                sValidateTextDirection(TextDirection)
            endif
            if fSetCheckBox(oControlToCheck2, FALSE) > 0 then
                warnlog "Unchecking check-box '" & sText & "' failed. Exiting test case"
                TabDataSeriesLabels.Cancel
                call hCloseDocument
                goto endsub
            endif            
        endif
        'Three checked check-boxes
        if iCombination = 7 then
            if fSetCheckBox(oControlToCheck, TRUE) > 0 AND fSetCheckBox(oControlToCheck2, TRUE) > 0 AND fSetCheckBox(oControlToCheck3, TRUE) > 0 then
                warnlog "Setting check-box '" & sText & "' failed. Exiting test case"
                TabDataSeriesLabels.Cancel
                call hCloseDocument
                goto endsub
            else
                printlog "Check '" & sText & "' and check that text direction-list-box contains 3 entries."
                sValidateTextDirection(TextDirection)
            endif
            if fSetCheckBox(oControlToCheck2, FALSE) > 0 then
                warnlog "Unchecking check-box '" & sText & "' failed. Exiting test case"
                TabDataSeriesLabels.Cancel
                call hCloseDocument
                goto endsub
            endif
            if fSetCheckBox(oControlToCheck3, FALSE) > 0 then
                warnlog "Unchecking check-box '" & sText & "' failed. Exiting test case"
                TabDataSeriesLabels.Cancel
                call hCloseDocument
                goto endsub
            endif                                
        end if
        if fSetCheckBox(oControlToCheck, FALSE) > 0 then
            warnlog "Unchecking check-box '" & sText & "' failed. Exiting test case"
            TabDataSeriesLabels.Cancel
            call hCloseDocument
            goto endsub
        endif
    next iCombination            
    TabDataSeriesLabels.Cancel
    if bCTLSupBefore <> gCTLSup then
        Call ActiveDeactivateCTLSupport(bCTLSupBefore)
        printlog "Reset the state of CTL support to the default before this test case has been executed"
    endif
    printlog "File / Close"
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tEquationTextDirection
    Dim bCTLSupBefore as boolean

    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    else
        printlog "Load test document and save it local as ../user/work/verySimpleChart.ods"    
    endif
    printlog "Enable the complex text layout (CTL) support under Tools / Options / Language Settings / Languages / Enhanced language support"
    bCTLSupBefore = ActiveDeactivateCTLSupport(TRUE)
    printlog "Select the first OLE object"
    call fSelectFirstOLE    
    printlog "Edit / Object Edit"
    EditObjectEdit    
    printlog "Select DataSeries A by toolbar selectorbox"
	call fChartSelectElement (8) 
	Kontext "DocumentChart"
    if fInvokeTrendLinesDialog() > 0 then
        warnlog "Something went wrong trying to invoke Trend Lines dialog -> Check this out!"
        Call hCloseDocument
        goto endsub
    endif   
    Kontext "TabTrendLineType"
    Call DialogTest(TabTrendLineType)
    printlog "Select Linear radio-button"
    Linear.Check
    printlog "Check Show Equation check-box"
    ShowEquation.Check
    printlog "Leave the dialog with OK"
    TabTrendLineType.OK
    Kontext "DocumentChart"
    printlog "Select Equation by toolbar selectorbox"
	call fChartSelectElement (10)
    printlog "Format / Object properties"
    FormatSelection.Click
    Kontext 
    printlog "Count the numbers of tab pages. It should be 7 in CTL mode."
    if Active.GetPageCount <> 7 then
        warnlog "The count of tab pages is not 7, it is " & Active.GetPageCount & "!"
    endif
    printlog "Select the Alignment tab page (only available in CTL mode!)"
    Active.SetPage TabFormatChartTitleAlignment
    Kontext "TabFormatChartTitleAlignment"
    Call DialogTest(TabFormatChartTitleAlignment)
    printlog "Check that the text direction list-box contains 3 entries."    
    sValidateTextDirection(TextDirection2)
    printlog "Leave the dialog with cancel"
    TabFormatChartTitleAlignment.Cancel
    if bCTLSupBefore <> gCTLSup then
        Call ActiveDeactivateCTLSupport(bCTLSupBefore)
        printlog "Reset the state of CTL support to the default before this test case has been executed"
    endif
    printlog "File / Close"
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
testcase tDataPointTextDirection
    Dim bCTLSupBefore as boolean
    Dim oControlToCheck as object
    Dim oControlToCheck2 as object
    Dim oControlToCheck3 as object
    Dim iCombination as integer
    Dim sText as string

    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    else
        printlog "Load test document and save it local as ../user/work/verySimpleChart.ods"    
    endif
    printlog "Enable the complex text layout (CTL) support under Tools / Options / Language Settings / Languages / Enhanced language support"
    bCTLSupBefore = ActiveDeactivateCTLSupport(TRUE)
    call fSelectFirstOLE
    printlog "Select the first OLE object"
    EditObjectEdit
    printlog "Edit / Object Edit"
    printlog "Select DataSeries A by toolbar selectorbox"
	call fChartSelectElement (8) 
	Kontext "DocumentChart"
    printlog "Press F3-key to enter the data series."
    DocumentChart.TypeKeys "<F3>"
    printlog "Insert / DataLabels"
    InsertDataLabels
    Kontext
    Active.Setpage TabDataPointLabels
    Kontext "TabDataPointLabels"    
    for iCombination = 1 to 7
        select case iCombination
            'Check-box combinations with 1, 2 or 3 enabled check-boxes.
            case 1      :   oControlToCheck = ShowValue
                            sText = "Show value as number"
            case 2      :   oControlToCheck = AsPercentage
                            sText = "Show value as percentage"
            case 3      :   oControlToCheck = ShowLabelText
                            sText = "Show category"
            
            case 4      :   oControlToCheck = ShowValue
                            oControlToCheck2 = AsPercentage
                            sText = "Show value as number and Show value as percentage"
            case 5      :   oControlToCheck = ShowValue
                            oControlToCheck2 = ShowLabelText
                            sText = "Show value as number Show category"
            case 6      :   oControlToCheck2 = AsPercentage
                            oControlToCheck2 = ShowLabelText
                            sText = "Show value as percentage and Show category"
            
            case 7      :   oControlToCheck = ShowValue
                            oControlToCheck2 = AsPercentage
                            oControlToCheck3 = ShowLabelText
                            sText = "Show value as number and Show value as percentage and Show category"  
                            
            case else   :   warnlog "Wrong combination entered. Exiting!"
                            TabDataPointLabels.Cancel
                            call hCloseDocument
                            goto endsub
        end select
        'One checked check-box
        if iCombination > 0 AND iCombination < 4 then
            if fSetCheckBox(oControlToCheck, TRUE) > 0 then
                warnlog "Setting check-box '" & sText & "' failed. Exiting test case"
                TabDataPointLabels.Cancel
                call hCloseDocument
                goto endsub
            else
                printlog "Check '" & sText & "' and check that text direction-list-box contains 3 entries."
                sValidateTextDirection(TextDirection)
            endif
        endif
        'Two checked check-boxes
        if iCombination > 3 AND iCombination < 7 then
            if fSetCheckBox(oControlToCheck, TRUE) > 0 AND fSetCheckBox(oControlToCheck2, TRUE) > 0 then
                warnlog "Setting check-box '" & sText & "' failed. Exiting test case"
                TabDataPointLabels.Cancel
                call hCloseDocument
                goto endsub
            else
                printlog "Check '" & sText & "' and check that text direction-list-box contains 3 entries."
                sValidateTextDirection(TextDirection)
            endif
            if fSetCheckBox(oControlToCheck2, FALSE) > 0 then
                warnlog "Unchecking check-box '" & sText & "' failed. Exiting test case"
                TabDataPointLabels.Cancel
                call hCloseDocument
                goto endsub
            endif            
        endif
        'Three checked check-boxes
        if iCombination = 7 then
            if fSetCheckBox(oControlToCheck, TRUE) > 0 AND fSetCheckBox(oControlToCheck2, TRUE) > 0 AND fSetCheckBox(oControlToCheck3, TRUE) > 0 then
                warnlog "Setting check-box '" & sText & "' failed. Exiting test case"
                TabDataPointLabels.Cancel
                call hCloseDocument
                goto endsub
            else
                printlog "Check '" & sText & "' and check that text direction-list-box contains 3 entries."
                sValidateTextDirection(TextDirection)
            endif
            if fSetCheckBox(oControlToCheck2, FALSE) > 0 then
                warnlog "Unchecking check-box '" & sText & "' failed. Exiting test case"
                TabDataPointLabels.Cancel
                call hCloseDocument
                goto endsub
            endif
            if fSetCheckBox(oControlToCheck3, FALSE) > 0 then
                warnlog "Unchecking check-box '" & sText & "' failed. Exiting test case"
                TabDataPointLabels.Cancel
                call hCloseDocument
                goto endsub
            endif                                
        end if
        if fSetCheckBox(oControlToCheck, FALSE) > 0 then
            warnlog "Unchecking check-box '" & sText & "' failed. Exiting test case"
            TabDataPointLabels.Cancel
            call hCloseDocument
            goto endsub
        endif
    next iCombination            
    TabDataPointLabels.Cancel
    if bCTLSupBefore <> gCTLSup then
        Call ActiveDeactivateCTLSupport(bCTLSupBefore)
        printlog "Reset the state of CTL support to the default before this test case has been executed"
    endif
    printlog "File / Close"
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------
'
sub sValidateTextDirection (oThisListtBox as OBJECT)
    Dim iEntries as integer
    iEntries = 0

    if oThisListtBox.IsEnabled then
        printlog "Get the count of the list-box entries."
        iEntries = oThisListtBox.GetItemCount
        if iEntries <> 3 then
            warnlog "There are " & iEntries &" and not 3 entries in the text direction list-box!"
        else
            printlog "3 entries in the text direction list-box"
        endif
    else
        warnlog "Text direction is not enabled!"
    endif
end sub

