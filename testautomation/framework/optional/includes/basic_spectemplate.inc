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
'*  short description : Load the spec template via http and enable macros
'*
'\******************************************************************************

testcase tBasicSpecTemplate

    const cFile = "http://specs.openoffice.org/collaterals/template/2.0/OpenOffice-org-Specification-Template.ott"
    dim cWorkFile as string : cWorkFile = hGetWorkPath() & "SpecTemplate" & hGetSuffix( "current" )

    printlog( "FileOpen" )
    hFileOpen( cFile )
    
    kontext "Active"
    if ( Active.exists( 2 ) ) then
        warnlog( "Unexpected errormessage: " & active.getText()
        active.ok()
        
        kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( 1 ) ) then
            printlog( "Recovering - closing File Open dialog" )
            hCloseDialog( OeffnenDlg, "cancel" )
        endif
        
        goto endsub
    endif
    
    printlog( "Accept to execute macros" )
    if ( not hAllowMacroExecution() ) then
        warnlog( "Missing Macro execution dialog. Aborting test" )
        goto endsub
    endif

    kontext "Active"
    if ( Active.exists( 10 ) ) then
        warnlog( "Fatal: Unexpected active: " & Active.getText() )
        active.ok
        
        kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( 2 ) ) then
            hCloseDialog( OeffnenDlg, "close" )
            goto endsub
        endif
    endif
    
    printlog( "Change the document")
    kontext "DocumentWriter"
    DocumentWriter.typeKeys( "<MOD1 END>" )
    DocumentWriter.typeKeys( "Changing the document" )
    
    printlog( "Disable Help Entry from Spec-Template Menu")
    hUseMenu()
    hMenuSelectNr( 10 )
    hMenuSelectNr( 2 )

    ' This one is required as the document is rendering for a while and we cannot 
    ' tell when it is finished (WaitSlot does not work here)
    wait( 5000 )

    printlog( "Save the file, overwriting" )
    hFileSaveAsKill( cWorkFile )

    printlog( "Close the document" )
    hDestroyDocument()

    printlog( "Load the file again" )
    hFileOpen( cWorkFile )
    hAllowMacroExecution()

    ' This sleep here is needed after loading the document otherwise the document is closed too early during execution of the macros, which results in basic runtime error messagebox, that can not be handled.
    SLEEP(1)
    
    printlog( "Cleanup: Close the document and delete the file" )
    hDestroyDocument()
    hDeleteFile( cWorkFile )
endcase

