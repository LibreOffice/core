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
'*  short description : Test VBA compatibility switches
'*
'\******************************************************************************

testcase tBasicVBACompatApplicationUnion()

    printlog( "Functionality of Application.Union" )
    ' Based on issue #i110724

    const IMPORT_EXCEL_MACROS  = TRUE
    const EXEC_EXCEL_MACROS    = TRUE
    const DOCUMENT_NAME        = "vba-application-union.xls"
    const LIBRARY_NAME         = "Standard"
    const MODULE_NAME          = "Module1"
    const EXPECTED_MACRO_COUNT = 6
    const MAX_WAIT             = 2
    const RC_FAILURE           = -1

    dim macro_identifier( EXPECTED_MACRO_COUNT )

    ' Find out whether we can write to the program directory or not.
    ' If we can we get different values returned from the test functions
    ' This is required for userland installations.
    if( hCheckForAdministratorPermissions() ) then
        macro_identifier( 1 ) = "1: False"
        macro_identifier( 2 ) = "2: False"
        macro_identifier( 3 ) = "3: False"
        macro_identifier( 4 ) = "4: True"
        macro_identifier( 5 ) = "5: False"
        macro_identifier( 6 ) = "6: True"
    else
        macro_identifier( 1 ) = "1: False"
        macro_identifier( 2 ) = "2: False"
        macro_identifier( 3 ) = "3: True"
        macro_identifier( 1 ) = "4: False"
        macro_identifier( 2 ) = "5: False"
        macro_identifier( 6 ) = "6: True"
    endif

    dim cTestFile as string
        cTestFile = gTesttoolPath & "framework/optional/input/vba-compat/" & DOCUMENT_NAME

    dim iMacroPosition as integer
    dim iCurrentMacro as integer
    dim iRepeat as integer

    printlog( "Set macro security to low" )
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_LOW )
    
    printlog( "Open Tools/Options" )
    hSetExcelBasicImportMode( IMPORT_EXCEL_MACROS, EXEC_EXCEL_MACROS )
    
    printlog( "Load the test file" )
    hFileOpen( cTestFile )

    ToolsMacro_uno

    kontext "Makro"
    if ( Makro.exists( MAX_WAIT ) ) then

        iMacroPosition = hSelectNodeByName( MakroAus, DOCUMENT_NAME )
        hExpandNode( MakroAus, iMacroPosition )

        printlog( "Move to node <" & LIBRARY_NAME & ">" )
        iMacroPosition = hSelectNextNode( MakroAus )
        if ( not hVerifyNodeName( MakroAus, LIBRARY_NAME ) ) then
            warnlog( "This is not the expected library" )
        endif

        printlog( "Move to node <" & MODULE_NAME & ">" )
        hSelectNextNode( MakroAus )
        if ( not hVerifyNodeName( MakroAus, MODULE_NAME ) ) then
            warnlog( "This is not the expected module" )
        endif

        printlog( "Click <Ausfuehren> to execute the macro" )
        if ( hClickButton( Ausfuehren ) <> RC_FAILURE ) then

            printlog( "Check for the " & EXPECTED_MACRO_COUNT & " expected messageboxes" )
            for iCurrentMacro = 1 to EXPECTED_MACRO_COUNT

                printlog( "" )
                kontext "Active"
                printlog( iCurrentMacro & ": Check for messagebox: " & macro_identifier( iCurrentMacro ) )
                if ( Active.exists( MAX_WAIT ) ) then

                    if ( Active.getText() = macro_identifier( iCurrentMacro ) ) then
                        printlog( "The expected macro was executed" )
                    else
                        warnlog( "Unexpected macro executed: " & Active.getText() )
                    endif
                    Active.ok()

                else
                    warnlog( "Expected messagebox not displayed within reasonable timeframe" )
                    goto endsub
                endif

            next iCurrentMacro
        else
            warnlog( "Unable to execute macro, the <Run Macro> button is disabled" )
            kontext "Makro"
            Makro.close()
        endif
    else
        warnlog( "Failed to open BASIC organizer" )
    endif

    printlog( "Cleanup" )
    hCloseDocument()
    hSetExcelImportModeDefault()    
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )

endcase


