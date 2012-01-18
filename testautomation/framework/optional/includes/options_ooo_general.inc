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
'* short description : 1. test for general group userdata
'*
'\******************************************************************************

testcase tOOoGeneral

    dim lsSave (4) as String
    dim lbSave (3) as Boolean

    hNewDocument

    ToolsOptions
    hToolsOptions ( "StarOffice", "General" )

    printlog " - save old data"
    lsSave ( 1 ) = Zweistellig.GetText
    lbSave ( 1 ) = Aktivieren.IsChecked
    Aktivieren.Check

    if gPlatgroup = "w95" then
        lbSave (2) = StarOfficeDialogeBenutzen.IsChecked
    endif

    lbSave ( 3 ) = DruckenStatus.IsChecked
    lsSave ( 3 ) = StyleSheet.GetSelText

    printlog " - invert/change data"
    Zweistellig.SetText "1950"
    Zuruecksetzen.Click

    if gPlatgroup = "w95" then
        if lbSave (2) then
            StarOfficeDialogeBenutzen.Uncheck
        else
            StarOfficeDialogeBenutzen.Check
        endif
    endif

    If lbSave ( 3 ) then
        DruckenStatus.Uncheck
    else
        DruckenStatus.Check
    endif
    
    StyleSheet.Select 3

    Kontext "ExtrasOptionenDlg"
    hCloseDialog( ExtrasOptionenDlg, "ok" )
    hCloseDocument

    printlog " - exit/restart StarOffice"
    ExitRestartTheOffice

    printlog " - check data"
    ToolsOptions
    hToolsOptions ( "StarOffice", "General" )

    if Zweistellig.GetText <> "1950" then
        '#i29810# 
        Warnlog "Year ( 2 digits ) => changes not saved: " + Zweistellig.GetText
    endif

    if not Aktivieren.IsChecked then
        Warnlog "Enable HelpAgent => changes not saved"
    endif

    Zuruecksetzen.Click

    if gPlatgroup = "w95" then
        if StarOfficeDialogeBenutzen.IsChecked = lbSave (2) then
            Warnlog "Use StarOffice dialogs => changes not saved!"
        endif
    endif

    If DruckenStatus.IsChecked = lbSave ( 3 ) then
        Warnlog "Printing sets 'Document modified' status => changes not saved!"
    endif

    printlog " - 2. change data"
    Zweistellig.SetText "1800"
    DruckenStatus.UnCheck

    Kontext "ExtrasOptionenDlg"
    hCloseDialog( ExtrasOptionenDlg, "ok" )

    printlog " - check data"
    ToolsOptions
    hToolsOptions ( "StarOffice", "General" )

    if ( Zweistellig.GetText <> "1800" ) then
        '#i29810# 
        Warnlog( "Year ( 2 digits ) => changes not saved : " & Zweistellig.GetText() )
    endif

    If DruckenStatus.IsChecked  then
        Warnlog "Printing sets 'Document modified' status => changes not saved!"
    endif

    if StyleSheet.GetSelIndex <> 3 then
        Warnlog "Style sheet for StarOffice Help => changes not saved!"
    endif

    printlog " - reset to saved data"
    Zweistellig.SetText lsSave ( 1 )
    Aktivieren.Check
    Zuruecksetzen.Click

    if lbSave (1) then
        Aktivieren.Check
    else
        Aktivieren.Uncheck
    endif

    if gPlatgroup = "w95" then
        if lbSave (2) then
            StarOfficeDialogeBenutzen.Check
        else
            StarOfficeDialogeBenutzen.UnCheck
        endif
    endif

    If lbSave(3) then
        DruckenStatus.Check
    else
        DruckenStatus.UnCheck
    endif

    StyleSheet.Select lsSave(3)

    Kontext "ExtrasOptionenDlg"
    hCloseDialog( ExtrasOptionenDlg, "ok" )

    ToolsOptions
    hToolsOptions ( "StarOffice", "General" )

    printlog " - check the reset data"
    if Zweistellig.GetText <> lsSave ( 1 ) then
        Warnlog "Year ( 2 digits ) => changes not saved"
    endif

    if Aktivieren.IsChecked <> lbSave (1) then
        Warnlog "Enable HelpAgent => changes not saved"
    endif

    Aktivieren.Check
    Zuruecksetzen.Click

    if lbSave (1) then
        Aktivieren.Check else Aktivieren.Uncheck
    endif

    if gPlatgroup = "w95" then
        if StarOfficeDialogeBenutzen.IsChecked <> lbSave (2) then
            Warnlog "Use StarOffice dialogs => changes not saved!"
        endif
    endif

    If DruckenStatus.IsChecked <> lbSave(3) then
        Warnlog "Printing sets 'Document modified' status => changes not saved!"
    endif

    if StyleSheet.GetSelText <> lsSave (3) then
        Warnlog "Style sheet for StarOffice Help => changes not saved!"
    endif

    Kontext "ExtrasOptionenDlg"
    hCloseDialog( ExtrasOptionenDlg, "ok" )

endcase

