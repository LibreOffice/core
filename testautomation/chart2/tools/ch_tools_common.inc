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
'* short description : Chart2 Tool library
'*
'\************************************************************************************************

sub fInsertOLE() as boolean

    ' Insert an OLE object (the name is localized)
    call GetOleDefaultNames()

    '/// Insert / Object / OLE
    InsertObjectOLEObject

    Kontext "OLEObjektEinfuegen"
    if ( OLEObjektEinfuegen.exists( 1 ) ) then

        if ( Objekttyp.getItemcount() = 0 ) then

            warnlog( "fInsertOLE(): There are no OLE-Objects in the <Objekttyp> listbox" )
            OLEObjektEinfuegen.cancel()
            fInsertOLE() = false

        else

            try
                Objekttyp.Select gOLEChart
                OLEObjektEinfuegen.ok()
                sleep( 2 )
                fInsertOLE() = true
            catch
                warnlog( "fInsertOLE(): Invalid OLE Object selected: " & gOLEChart )
                OLEObjektEinfuegen.cancel()
                fInsertOLE() = false
            endcatch

        endif

    else
        warnlog( "fInsertOLE(): Failed to open <Insert OLE object> dialog" )
        fInsertOLE() = false
    endif
    
end sub
'
'--------------------------------------------------------------------
'
function fLoadVerySimpleChart() as INTEGER
    fLoadVerySimpleChart = 99
    'Load file with a very simple chart
    'No Input
    'Returns error-code:
    '+ 0 := Sucess
    '+ 1 := Serious problem trying to load the document
    '+99 := Unexpected error

    dim sInputFile as string
    sInputFile = convertpath( gtesttoolpath & "chart2/required/input/verySimpleChart.ods" )
    '/// Load file <i>gTesttoolpath</i>/chart2/required/input/verySimpleChart.ods
    try
        call hFileOpen ( sInputFile )
        printlog "File with very simple chart loaded"
        fLoadVerySimpleChart = 0
    catch
        qaErrorLog "Loading file with very simple chart seems to fail"
        fLoadVerySimpleChart = 1
    endcatch
end function
'
'--------------------------------------------------------------------
'
function fLoadVerySimpleChartAndSaveLocal() as INTEGER
    fLoadVerySimpleChartAndSaveLocal = 99
    'Load file with a very simple chart and save it loacally
    'No Input
    'Returns error-code:
    '+ 0 := Sucess
    '+ 1 := Serious problem trying to load the document
    '+ 2 := Serious problem trying to save the document

    dim iCheckIsFileLoaded as INTEGER
    dim sOutputFile as STRING
    sOutputFile = convertpath( gOfficepath & "user/work/verySimpleChart.ods" )
    iCheckIsFileLoaded = fLoadVerySimpleChart()
    if iCheckIsFileLoaded > 0 then
        'When loading fails pass error code to current function
        fLoadVerySimpleChartAndSaveLocal = iCheckIsFileLoaded
        exit function
    else
        printlog "File with very simple chart loaded successfully."
    endif
    '/// Save document local
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<ESCAPE>"
    DocumentCalc.TypeKeys "<MOD1 HOME>"
    if NOT hFileSaveAsKill (sOutputFile) then
        fLoadVerySimpleChartAndSaveLocal = 2
        qaErrorLog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        exit function
    else
        printlog "File with very simple chart saved successfully."
    endif
    fLoadVerySimpleChartAndSaveLocal = 0
end function
'
'--------------------------------------------------------------------
'
function fConvertChartTo3D() as BOOLEAN
    fConvertChartTo3D = FALSE
    'Function to invoke the 'Insert::Titles...' dialog
    'No Input
    'Returns TRUE on success

    '/// Invoke Format::Chart Type...
    if fInvokeChartTypeDialog() > 0 then
        warnlog "Invoking 'Format::Chart Type...' seems to cause an Error"
        exit function
    endif
    '/// Check 3D box
    if fSetChartType3D ( TRUE ) = 0 then
        fConvertChartTo3D = TRUE
    else
        warnlog "Converting chart to 3D seems to cause an Error"
        exit function
    endif
    '/// Close Chart Type dialog with OK
    if fCloseChartTypeDialogOK() > 0 then
        warnlog "Closing Chart Type dialog seems to cause an Error"
        fConvertChartTo3D = FALSE
    endif
end function
'
'-------------------------------------------------------------------------
'
function fOpenTestDocumentAndSelectDataRange(sInputFile as string) as BOOLEAN
    fOpenTestDocumentAndSelectDataRange = FALSE
    hFileOpenLocally(sInputFile)
    '/// Select chart using navigator
    printlog "Select chart using navigator"
    call fSelectFirstOLE
    '/// Edit / Object / Edit
    printlog "Edit / Object / Edit"
    EditObjectEdit
    '///Open FORMAT / DATA RANGES
    printlog "Open FORMAT / DATA RANGES"
    FormatDataRanges
    WaitSlot()
    '///Switch to tabpage Data Range
    printlog "Switch to tabpage Data Range"
    Kontext
    'Switching tabpages as workaround for #i77237#
    Active.SetPage TabChartTypeDataSeries
    QAErrorLog "#i77237#-Testtool can't access OK after switching tabpage"
    Active.SetPage TabChartTypeDataRange
    Kontext "TabChartTypeDataRange"
    call DialogTest ( TabChartTypeDataRange )
    fOpenTestDocumentAndSelectDataRange = TRUE
end function
'
'-------------------------------------------------------------------------
'
function fOpenTestDocumentAndInsertChart(sInputFile as string) as BOOLEAN
    fOpenTestDocumentAndInsertChart = FALSE
    hFileOpenLocally(sInputFile)
    Kontext "DocumentCalc"
    printlog "Type <STRG+HOME>"
    DocumentCalc.TypeKeys "<MOD1 HOME>"
    printlog "Type twice <RIGHT> cursor key"
    DocumentCalc.TypeKeys "<RIGHT>" , 2
    printlog "Type thrice <DOWN> cursor key"
    DocumentCalc.TypeKeys "<DOWN>" , 3
    printlog "Menu Insert chart to open chart wizard"
    InsertChart
    try
        WaitSlot()
        fOpenTestDocumentAndInsertChart = TRUE
    catch
        fOpenTestDocumentAndInsertChart = FALSE
    endcatch
end function
