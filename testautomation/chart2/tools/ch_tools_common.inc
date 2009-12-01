'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: ch_tools_common.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: obo $ $Date: 2008-07-21 12:08:07 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* owner : helge.delfs@sun.com
'*
'* short description : Chart2 Tool library
'*
'**************************************************************************************************
'*
' #1 fInsertOLE                             ' User Insert / Object / OLE to insert a chart
' #1 fLoadVerySimpleChart                   ' Load file with a very simple chart
' #1 fLoadVerySimpleChartAndSaveLocal       ' Load file with a very simple chart and save it loacally
' #1 fConvertChartTo3D                      ' Function to invoke the 'Insert::Titles...' dialog
' #1 fOpenTestDocumentAndSelectDataRange    ' Opens a test document and executes FormatDataRange
' #1 fOpenTestDocumentAndInsertChart        ' Opens a test document and selects a data range at a specific position
'*
'\************************************************************************************************

sub fInsertOLE as BOOLEAN
'Insert Chart OLE in Document
    fInsertOLE = FALSE
    '/// Insert / Object / OLE
    InsertObjectOLEObject
    sleep(1)
    Kontext "OLEObjektEinfuegen"
    '/// Check if list box is not empty
    if Objekttyp.getItemcount < 1 then
        warnlog "There are no OLE-Objects in the listbox"
    endif
    '/// Select 3rd entry in listbox (which should be the chart)
    Objekttyp.TypeKeys "<HOME>"
	Objekttyp.TypeKeys "<Down>", 2
    '/// Quit dialog using OK
    OLEObjektEinfuegen.OK
    sleep(2)
    fInsertOLE = TRUE
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
    call hFileOpen(sInputFile)
    sleep(2)
    Call sMakeReadOnlyDocumentEditable()    
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
    call hFileOpen(sInputFile)
    sleep(2)
    Call sMakeReadOnlyDocumentEditable()    
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
