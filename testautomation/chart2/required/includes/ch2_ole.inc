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
'* short description : Chart resource test - section FILE
'*
'************************************************************************
'*
' #1 tChartOLE
'*
'\************************************************************************

testcase tChartOLE ( sCurrentApplication as STRING )
'///<u><b>Chart as OLE in all applicable applications</b></u>
    dim sOutputFile as STRING    
    dim bCommitDialog as boolean
    dim bStatusOfAgent as boolean
    
    bStatusOfAgent = FALSE
                
    select case sCurrentApplication
        case ("CALC") : bCommitDialog = TRUE
        case else     : bCommitDialog = FALSE
    end select
    printlog("Chart as OLE in " & sCurrentApplication)
    gApplication = sCurrentApplication

    '/// Open new document
    Call hNewDocument
    '/// Disabling the Help Agent if enabled.
    ToolsOptions
    hToolsOptions ( "StarOffice", "General" )
    Kontext "TabSonstigesAllgemein"
    if Aktivieren.IsChecked then
        bStatusOfAgent = TRUE
        printlog "HelpAgent was enabled. Disabling"
        Aktivieren.Uncheck
    else
        printlog "HelpAgent wasn't enabled."
    end if    
    Kontext "ToolsOptionsDlg"
    ToolsOptionsDlg.OK
    '/// Insert default chart
    InsertChart
    sleep(2)
    if bCommitDialog then
        Kontext "ChartWizard"
        ChartWizard.OK
        sleep(2)
    endif
    '/// Save file
    select case ucase(sCurrentApplication)
        case ("CALC")           :   sOutputFile = ( gOfficePath & ConvertPath("user\work\") & "tChartOLE_" & sCurrentApplication & ".ods" )
        case ("IMPRESS")        :   sOutputFile = ( gOfficePath & ConvertPath("user\work\") & "tChartOLE_" & sCurrentApplication & ".odp" )
        case ("DRAW")           :   sOutputFile = ( gOfficePath & ConvertPath("user\work\") & "tChartOLE_" & sCurrentApplication & ".odg" )
        case ("WRITER")         :   sOutputFile = ( gOfficePath & ConvertPath("user\work\") & "tChartOLE_" & sCurrentApplication & ".odt" )
        case ("MASTERDOCUMENT")      :   sOutputFile = ( gOfficePath & ConvertPath("user\work\") & "tChartOLE_" & sCurrentApplication & ".odm" )        
        case ("HTML")   :   sOutputFile = ( gOfficePath & ConvertPath("user\work\") & "tChartOLE_" & sCurrentApplication & ".html" )
        case else               :   warnlog "The modul " & sCurrentApplication & " is not supported in this test case!"
                                    Call hCloseDocument
                                    goto endsub               
    end select  
    if hFileSaveAsKill(sOutputFile) then
        printlog "OK, successfully saved the test document!"
    else
        warnlog "Saving the test document failed!"
    endif
    sleep(2)
    '/// Leave implace mode in Spreadsheet, Draw and Impress ...
    '/// ... and then select Chart OLE (Green handles)
    select case ucase(sCurrentApplication)
        case ("CALC")           :   Kontext "DocumentCalc"
                                    DocumentCalc.TypeKeys "<Escape>"        
                                    Call fSelectFirstOLE
        case ("IMPRESS")        :   call gMouseclick (99,99) 
                                    call gMouseclick (50,50)
        case ("DRAW")           :   call gMouseclick (99,99) 
                                    call gMouseclick (50,50)
        case ("WRITER")         :   call gMouseclick (99,99) 
                                    call gMouseclick (50,50)
                                    Call fSelectFirstOLE
        case ("MASTERDOCUMENT")      :   call gMouseclick (99,99)
        case ("HTML")   :   call gMouseclick (99,99) 
                                    call gMouseclick (50,50)
                                    Call fSelectFirstOLE
        case else               :   warnlog "The modul " & sCurrentApplication & " is not supported in this test case!"
                                    Call hCloseDocument
                                    goto endsub               
    end select
    '/// Try if Edit::Object:Edit works for chart OLE
    try
        EditObjectEdit
        sleep(2)
        printlog "Edit::Object::Edit seems to work"
    catch
        warnlog "Edit::Object::Edit seems to fail."
    endcatch

    if bStatusOfAgent then    
        '/// Enabling the HelpAgent if it was enabled at the beginning.    
        ToolsOptions
        hToolsOptions ( "StarOffice", "General" )
        Kontext "TabSonstigesAllgemein"
        Aktivieren.Check
        Kontext "ToolsOptionsDlg"
        ToolsOptionsDlg.OK        
    end if            
    '/// Close document
    Call hCloseDocument
    gApplication = "CALC"    
endcase

