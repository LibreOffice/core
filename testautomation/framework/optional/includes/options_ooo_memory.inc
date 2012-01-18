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
'* short description : 1. test for general group userdata ... view)
'*
'\******************************************************************************

testcase tOOoMemory
    
    Dim lsSave (7) as String
    Dim lbSave as Boolean
    Dim sValue as String
    
    dim bHasQuickstarter as boolean
    
    hNewDocument
    ToolsOptions
    hToolsOptions ( "StarOffice", "Memory" )
    
    printlog " - check platform dependences "
    
    if ( LoadQuickstarter.exists() and LoadQuickstarter.isVisible() and LoadQuickstarter.isEnabled() ) then
        bHasQuickstarter = TRUE
        printlog( "Quickstarter is available" )
    else
        bHasQuickstarter = FALSE
        printlog( "Quickstarter is NOT available on this platform" )
    endif
    
    printlog " - save old data"
    lsSave(1) = UndoSteps.GetText
    lsSave(2) = StarOffice.GetText
    lsSave(3) = MemoryPerObject.GetText
    lsSave(4) = RemoveFromMemoryAfter.GetText
    lsSave(5) = NumberOfObjects.GetText
    if ( bHasQuickstarter ) then
        lbSave = LoadQuickstarter.IsChecked
    endif
    
    printlog " - invert/change data"
    UndoSteps.SetText "80"
    StarOffice.SetText "50"
    if Instr ( lsSave(3), "," ) <> 0 then
        sValue = "4,1"
    else
        sValue = "4.1"
    end if
    MemoryPerObject.SetText sValue
    RemoveFromMemoryAfter.SetText "00:41"
    NumberOfObjects.SetText "81"
    if ( bHasQuickstarter ) then
        LoadQuickstarter.UnCheck
    endif
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
   if ( ExtrasOptionenDlg.notExists( 3 ) ) then
        printlog( "Options closed" )
    endif

    
    hCloseDocument
    
    printlog " - exit/restart StarOffice"
    ExitRestartTheOffice
    
    printlog " - check data"
    ToolsOptions
    hToolsOptions ( "StarOffice", "Memory" )
    
    if UndoSteps.GetText             <> "80" then
        Warnlog "Undo steps => changes not saved!"
    endif
    if StarOffice.GetText            <> "50" then
        Warnlog "Use For StarOffice => changes not saved!"
    endif
    if MemoryPerObject.GetText       <> sValue then
        Warnlog "Memory per object => changes not saved! => " + MemoryPerObject.GetText
    endif
    if RemoveFromMemoryAfter.GetText <> "00:41" then
        Warnlog "Remove from memory after => changes not saved! =>" + RemoveFromMemoryAfter.GetText
    endif
    if NumberOfObjects.GetText       <> "81" then
        Warnlog "Number of objects => changes not saved!"
    endif
    if ( bHasQuickstarter ) then
        if LoadQuickstarter.IsEnabled then
            if LoadQuickstarter.IsChecked = TRUE then
                Warnlog "Load StarOffice during system-startup => changes not saved!"
            end if
        end if
    endif
    
    printlog " - 2. change data"
    UndoSteps.SetText "5"
    StarOffice.SetText "11"
    if Instr ( lsSave(3), "," ) <> 0 then
        sValue = "1,9"
    else
        sValue = "1.9"
    end if
    MemoryPerObject.SetText sValue
    RemoveFromMemoryAfter.SetText "01:32"
    NumberOfObjects.SetText "2"
    if ( bHasQuickstarter ) then    
        LoadQuickstarter.Check
    endif
        
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
   if ( ExtrasOptionenDlg.notExists( 3 ) ) then
        printlog( "Options closed" )
    endif
    
    printlog " - check data"
    ToolsOptions
    hToolsOptions ( "StarOffice", "Memory" )
    
    if UndoSteps.GetText             <> "5" then
        Warnlog "Undo steps => changes not saved!"
    endif
    if StarOffice.GetText            <> "11" then
        Warnlog "Use For StarOffice => changes not saved!"
    endif
    if MemoryPerObject.GetText       <> sValue then
        Warnlog "Memory per object => changes not saved! => " + MemoryPerObject.GetText
    endif
    if RemoveFromMemoryAfter.GetText <> "01:32" then
        Warnlog "Remove from memory after => changes not saved! => " + RemoveFromMemoryAfter.GetText
    endif
    if NumberOfObjects.GetText       <> "2" then
        Warnlog "Number of objects => changes not saved!"
    endif
    if ( bHasQuickstarter ) then
        if LoadQuickstarter.IsChecked <> TRUE then
            Warnlog "Load StarOffice during system-startup => changes not saved!"
        end if
    endif
    
    printlog " - reset to saved data"
    UndoSteps.SetText lsSave(1)
    StarOffice.SetText lsSave(2)
    MemoryPerObject.SetText lsSave(3)
    RemoveFromMemoryAfter.SetText lsSave(4)
    NumberOfObjects.SetText lsSave(5)

    if ( bHasQuickstarter ) then
        if lbSave = TRUE then
            LoadQuickstarter.Check else LoadQuickstarter.UnCheck
        endif
    endif
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
   if ( ExtrasOptionenDlg.notExists( 3 ) ) then
        printlog( "Options closed" )
    endif
    
    ToolsOptions
    hToolsOptions ( "StarOffice", "Memory" )
    
    printlog " - check the reset data"
    if UndoSteps.GetText             <> lsSave(1) then
        Warnlog "Undo steps => changes not saved!"
    endif
    if StarOffice.GetText            <> lsSave(2) then
        Warnlog "Use For StarOffice => changes not saved!"
    endif
    if MemoryPerObject.GetText       <> lsSave(3) then
        Warnlog "Memory per object => changes not saved! => " + MemoryPerObject.GetText
    endif
    if RemoveFromMemoryAfter.GetText <> lsSave(4) then
        Warnlog "Remove from memory after => changes not saved! =>" + RemoveFromMemoryAfter.GetText
    endif
    if NumberOfObjects.GetText       <> lsSave(5) then
        Warnlog "Number of objects => changes not saved!"
    endif
    if ( bHasQuickstarter ) then
        if LoadQuickstarter.IsChecked <> lbSave then
            Warnlog "Load StarOffice during system-startup => changes not saved!"
        endif
    endif
    
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
   if ( ExtrasOptionenDlg.notExists( 3 ) ) then
        printlog( "Options closed" )
    endif

endcase



