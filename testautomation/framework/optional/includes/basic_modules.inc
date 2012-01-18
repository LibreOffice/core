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
'*  short description : Edit several modules and dialogs in basic ide
'*
'\******************************************************************************


testcase tMore_Modules_Dialogs


    dim brc as boolean
    
    dim iCurrentTab as integer

    dim cFile as string 
        cFile = gOfficePath & "user\work\basic.odt"
        
    const FILTER = "writer8"

    'const iTABCOUNT is the number of dialogs and modules.
    'If iTABCOUNT = 10 then 20 tabs (+ the first one!) = 21 should be created.

    const iTABCOUNT as Integer = 10
    const CMODULE = "TTMODULE"
    
    hDeleteFile( cFile )
    
    printlog "open a new writer-doc"
    gApplication = "WRITER"
    hCreateDocument()
    
    brc = hInitBasicIDE( CMODULE )
    
    printlog "insert 10 modules"

    for iCurrentTab = 1 to iTABCOUNT
        printlog "   - insert new module : " & iCurrentTab

        kontext "basicide"
        Tabbar.OpenContextMenu
        hMenuSelectNr(1)
        hMenuSelectNr(1)
        brc = hDeleteMacro()
        if ( brc ) then
            EditWindow.TypeKeys "'# " & ( iCurrentTab + 1 ) & ". module in this document"
        endif
    next iCurrentTab

    printlog "insert 10 dialogs"

    for iCurrentTab = 1 to iTABCOUNT
        printlog "   - insert new dialog : " & iCurrentTab
        Tabbar.OpenContextMenu
        hMenuSelectNr(1)
        hMenuSelectNr (2)

        if DialogWindow.Exists (2) <> TRUE then
            warnlog "No dialog window is shown!"
        end if

    next iCurrentTab

    printlog Chr(13) + "- close the BasicIDE"
    hCloseBasicIDE()
    
    printlog Chr(13) + "- save the document"
    Call hFileSaveAsWithFilterKill( cFile , FILTER )
    
    printlog Chr(13) + "- close the document"
    hDestroyDocument()
    
    printlog Chr(13) + "- open the saved document"
    hFileOpen( cFile )

    ToolsMacro_uno
    kontext "Makro"
    hSelectNodeByName( MakroAus , CMODULE )

    Bearbeiten.Click()
    
    printlog "delete all modules and dialogs"

    for iCurrentTab = 1 to ( 2 * iTABCOUNT + 1 )

        try
            printlog "delete  - " & iCurrentTab & "/" & ( 2 * iTABCOUNT + 1 )
            UseBindings

            kontext "basicide"
            Tabbar.OpenContextMenu
            
            hMenuSelectNr( 2 )

            kontext "active"

            if Active.Exists then
                Active.Yes
            else
                warnlog "No warning after deleting a dialog!"
            end if

        catch
            QAErrorLog "Deleting  - " & iCurrentTab & "/" & ((2 * iTABCOUNT)+1) & " failed."
        endcatch

    next iCurrentTab

    hDestroyDocument()
    hDestroyDocument()


endcase

