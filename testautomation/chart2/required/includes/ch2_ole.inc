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

