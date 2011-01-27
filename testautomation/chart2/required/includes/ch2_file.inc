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
'* short description : Chart resource test - Menu FILE
'*
'************************************************************************
'*
' #1 tFileClose
' #1 tFileSaveAsOTS
' #1 tFileSaveAsSXC
' #1 tFileSaveAsSTC
' #1 tFileSaveAsXLS
' #1 tFileSaveAsXLT
' #1 tFileReload
'*
'\************************************************************************

testcase tFileClose
'///<u><b>Dialog test after invocation of File::Close</b></u
   '/// Load new document
   Call hNewDocument
   '/// Insert default chart
   InsertChart
   sleep(2)
   Kontext "ChartWizard"
   '/// Leave wizard with OK 
   ChartWizard.OK
   printlog "Left Chart wizard with OK"
   sleep(2)
   '/// File / Close
   FileClose
   printlog "File / Close executed"
   Kontext
   '/// MsgBox: Cancel
   Active.Cancel
   '/// File / Close
   FileClose
   '/// MsgBox: Save
   Kontext
   Active.Yes
   '/// Check for File Save dialo
   Kontext "SpeichernDlg"
   '/// Cancel File Save dialog
   SpeichernDlg.Cancel
   '/// File / Close
   FileClose
   Kontext
   '/// MsgBox: Discard
   Active.No
endcase
'
'-----------------------------------------------------------
'
testcase tFileSaveAsOTS
'///<u><b>Save the test file as OpenDocument Spreadsheet Template (.ots)</b></u>
    Dim sOutputFile as string
    
    sOutputFile = convertpath( gOfficepath & "user/work/verySimpleChart.ots" )

    '/// Load simple chart document   
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document failed!"
        goto endsub
    else
        printlog "Test document successfully been loaded."
    endif
    '/// Select first OLE using the navigator 
    call fSelectFirstOLE
    printlog "First OLE selected via navigator"
    '/// Edit / Object / Edit to enter Inplace Mode 
    EditObjectEdit
    printlog "Edit / Object / Edit"
    '/// Convert chart to 3D (just to edit document) 
    if NOT fConvertChartTo3D() then
        warnlog "Conversion of chart to 3D failed -> Quit testcase"
        Call hCloseDocument
        goto endsub
    else
        printlog "Conversation of chart to 3D was successful."
    endif
    '/// Unselect chart using Escape key 
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"        
    if NOT hFileSaveAsWithFilterKill ( sOutputFile , "calc8_template" ) then
        warnlog "Saving test document as '" & sOutputFile & "' failed -> Aborting"
        call hCloseDocument
        goto endsub
    endif
    '/// Clsoing the Navigator
    Kontext "NavigatorCalc"
    if NavigatorCalc.Exists(1) then
    	NavigatorCalc.Close
        QAErrorLog "Why did fSelectFirstOLE not close this dialog?"
    endif
    '/// Close document 
    Call hCloseDocument 
endcase
'
'-------------------------------------------------------------
'
testcase tFileSaveAsSXC
'///<u><b>Save the test file as StarOffice 6.0/7 Spreadsheet (.sxc)</b></u>
    Dim sOutputFile as string
        
    sOutputFile = convertpath( gOfficepath & "user/work/verySimpleChart.sxc" )

    '/// Load simple chart document 
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document failed!"
        goto endsub
    else
        printlog "Loading test document was successful!"
    endif
    '/// Select chart using navigator 
    call fSelectFirstOLE
    printlog "First OLE selected via Navigator"
    '/// Edit / Object / Edit to enter Inplace Mode 
    EditObjectEdit
    printlog "Edit / Object / Edit"
    '/// Convert chart to 3D (just to edit document) 
    if NOT fConvertChartTo3D() then
        warnlog "Conversion of chart to 3D failed -> Quit testcase"
        Call hCloseDocument
        goto endsub
    endif
    '/// Unselect chart using Escape key 
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"        
    '/// Save document as StarOffice 6.0/7 Spreadsheet (.sxc)
    if NOT hFileSaveAsWithFilterKill ( sOutputFile , "StarOffice XML (Calc)") then
        warnlog "Saving test document as '" & sOutputFile & "' failed -> Aborting"
        call hCloseDocument
        goto endsub
    endif
    '/// Close document 
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------
'
testcase tFileSaveAsSTC
'///<u><b>Save the test file as StarOffice 6.0/7 Spreadsheet Template (.stc)</b></u>
    Dim sOutputFile as string
    
    sOutputFile = convertpath( gOfficepath & "user/work/verySimpleChart.stc" )

    '/// Load simple chart document 
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    else
        printlog "Simple chart document successfully been loaded."   
    endif
    '/// Select chart using navigator 
    call fSelectFirstOLE
    '/// Edit / Object / Edit 
    EditObjectEdit
    '/// Convert chart to 3D (just to edit document) 
    if NOT fConvertChartTo3D() then
        warnlog "Conversion of chart to 3D failed -> Quit testcase"
        Call hCloseDocument
        goto endsub
    endif
    '/// Unselect chart using Escape key 
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"
    '/// Save document as StarOffice 6.0/7 Spreadsheet Template (.stc)
    if NOT hFileSaveAsWithFilterKill ( sOutputFile , "calc_StarOffice_XML_Calc_Template" ) then
        warnlog "Saving test document as '" & sOutputFile & "' failed -> Aborting"
        call hCloseDocument
        goto endsub
    endif
    '/// Close document         
    Call hCloseDocument
endcase
'
'-------------------------------------------------------------
'
testcase tFileSaveAsXLS
'///<u><b>Save the test file as Microsoft Excel 97/2000/XP (.xls)</b></u>
    Dim sOutputFile as string
       
    sOutputFile = convertpath( gOfficepath & "user/work/verySimpleChart.xls" )

    '/// Load simple chart document <i>Testtoolpath</i>/chart2/required/input/verySimpleChart.ods
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document failed!"
        goto endsub
    endif
    '/// Edit / Navigator
    '/// Select the first OLE object in the tree view of the Navigator
    '/// Press <RETURN>
    call fSelectFirstOLE
    printlog "First OLE selected." 
    '/// Edit / Object / Edit to enter Inplace Mode   
    EditObjectEdit
    printlog "Edit / Object / Edit executed."
    '/// Format / Chart Type
    '/// Check 3D checkbox
    '/// Close Chart Type dialog
    if NOT fConvertChartTo3D() then
        warnlog "Conversion of chart to 3D failed -> Quit testcase"
        Call hCloseDocument
        goto endsub   
    else
        printlog "Chart type converted to 3D."
    endif
    '/// Unselect chart using Escape key 
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"        
   '/// Save document as Microsoft Excel 97/2000/XP (.xls)
    if NOT hFileSaveAsWithFilterKill ( sOutputFile , "MS Excel 97" ) then
        warnlog "Saving test document as '" & sOutputFile & "' failed -> Aborting"
        call hCloseDocument
        goto endsub
    endif
   '/// Close document
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
endcase
'
'-------------------------------------------------------------------------------
'
testcase tFileSaveAsXLT
'///<u><b>Save the test file as Microsoft Excel 97/2000/XP Template(.xlt)</b></u>
    Dim sOutputFile as string
        
    sOutputFile = convertpath( gOfficepath & "user/work/verySimpleChart.xlt" )

    '/// Load simple chart document <i>Testtoolpath</i>/chart2/required/input/verySimpleChart.ods
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document failed!"
        goto endsub
    endif
    '/// Edit / Navigator
    '/// Select the first OLE object in the tree view of the Navigator
    '/// Press <RETURN>
    call fSelectFirstOLE
    printlog "First OLE selected." 
    '/// Edit / Object / Edit to enter Inplace Mode   
    EditObjectEdit
    printlog "Edit / Object / Edit executed."
    '/// Format / Chart Type
    '/// Check 3D checkbox
    '/// Close Chart Type dialog
    if NOT fConvertChartTo3D() then
        warnlog "Conversion of chart to 3D failed -> Quit testcase"
        Call hCloseDocument
        goto endsub   
    else
        printlog "Chart type converted to 3D."
    endif
    '/// Unselect chart using Escape key 
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Escape>"        
    '/// Save document as Microsoft Excel 97/2000/XP Template(.xlt)    
    if NOT hFileSaveAsWithFilterKill ( sOutputFile , "MS Excel 97 Vorlage/Template" ) then
        warnlog "Saving test document as '" & sOutputFile & "' failed -> Aborting"
        call hCloseDocument
        goto endsub
    endif
   '/// Close document
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
endcase
'
'-------------------------------------------------------------

'
testcase tFileReload
    printlog "Load simple chart document" 
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif   
   printlog "Select chart using navigator" 
   call fSelectFirstOLE   
   printlog "Invoke Edit / Object / Edit to enter Inplace Mode" 
   EditObjectEdit   
   printlog "Convert chart to 3D (just to edit document)" 
   if NOT fConvertChartTo3D() then
       warnlog "Conversion of chart to 3D failed -> Quit testcase"
       Call hCloseDocument
       goto endsub
   endif   
   printlog "Unselect chart using Escape key" 
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   sleep (2)   
   printlog "File / Reload" 
   FileReload   
   printlog "MsgBox 'Cancel all changes?' should be visible -> Commit OK" 
   Kontext
   Active.Yes
   sleep (2)   
   printlog "Close document ///"
   Call hCloseDocument
endcase

