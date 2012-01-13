'encoding UTF-8  Do not remove or change this line!
'**************************************************************
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
'**************************************************************
*  
'*  short description : Test VBA compatibility switches
'*
'\******************************************************************************

testcase tBasicVBACompatXLSM_XLSB()

    printlog( "Functionality of macros in XLSM/XLSB files" )
    ' Based on issue #i111007

    const IMPORT_EXCEL_MACROS  = TRUE
    const EXEC_EXCEL_MACROS    = TRUE

    const FILE_COUNT = 2
    const MODULE_COUNT = 7

    const MODULE_NOT_FOUND = 0
    const MAX_DELAY = 3
    const RC_TIMEOUT = -1

    const THE_MODULE_THAT_BEHAVES_DIFFERENTLY = 6
    
    const RELATIVE_PATH = "framework/optional/input/vba-compat/"

    ' We have two files to load, on e .xlsm and one .xlsb
    dim cFileList( FILE_COUNT ) as string
        cFileList( 1 ) = "vba-project.xlsm"
        cFileList( 2 ) = "vba-project.xlsb"

    ' both documents import the same modules
    dim cTestModuleList( MODULE_COUNT )
        cTestModuleList( 1 ) = "MyChartModule"
        cTestModuleList( 2 ) = "MyCodeModule"
        cTestModuleList( 3 ) = "MyPrivateClassModule"
        cTestModuleList( 4 ) = "MyPublicClassModule"
        cTestModuleList( 5 ) = "MySheetModule"
        cTestModuleList( 6 ) = "MyUserForm"
        cTestModuleList( 7 ) = "MyWorkbookModule"

    dim cTestFile as string
    dim iCurrentFile as integer
    dim iCurrentModule as integer

    printlog( "Set macro security to low" )
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_LOW )
    
    printlog( "Open Tools/Options" )
    hSetExcelBasicImportMode( IMPORT_EXCEL_MACROS, EXEC_EXCEL_MACROS )
    
    for iCurrentFile = 1 to FILE_COUNT

        cTestFile = gTesttoolPath & RELATIVE_PATH & cFileList( iCurrentFile )

        printlog( "Loading document: " & cTestFile )
        hFileOpen( cTestFile )

        for iCurrentModule = 1 to MODULE_COUNT

            printlog( "" )
            printlog( "Tryng to execute macro: " & cTestModuleList( iCurrentModule ) )
            ToolsMacro_uno

            kontext "Makro"
            if ( Makro.exists( MAX_DELAY ) ) then

                if ( hSelectNodeByName( MakroAus, cTestModuleList( iCurrentModule ) ) <> MODULE_NOT_FOUND ) then

                    printlog( "Execute the default macro <Test> for the current module" )

                    if ( hClickButton( Ausfuehren ) <> RC_TIMEOUT ) then
                        if ( iCurrentModule = THE_MODULE_THAT_BEHAVES_DIFFERENTLY ) then
                            ' Fuzzy matching message string
                            hTestMacroType2( cTestModuleList( iCurrentModule ) )
                        else
                            ' Exact matching message string
                            hTestMacroType1( cTestModuleList( iCurrentModule ) )
                        endif
                    else
                        warnlog( "No executable macro found for the current module / <Run> button is disabled" )
                    endif
                else
                    warnlog( "the expected macro module could not be found" )
                endif
            else
                warnlog( "BASIC Macro organizer did not open" )
            endif

        next iCurrentModule

        printlog( "Close the document" )
        hFileCloseAll()

    next iCurrentFile

    hSetExcelImportModeDefault()    
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )

endcase

'*******************************************************************************

function hTestMacroType1( cMessage as string ) as boolean

    ' exact matching

    kontext "Active"
    if ( Active.exists( 2 ) ) then
        if ( Active.getText() = cMessage ) then
            printlog( "Correct macro has been executed" )
            hTestMacroType1() = true
        else
            warnlog( "Incorrect macro executed: " )
            printlog( "Expected: " & cMessage )
            printlog( "Found...: " & Active.getText() )
            hTestMacroType1() = false
        endif

        Active.ok()
        WaitSlot()

    else
        warnlog( "Messagebox missing, macro was not executed" )
        hTestMacroType1() = false
    endif

end function

'*******************************************************************************

function hTestMacroType2( cMessage as string ) as boolean

    ' fuzzy matching

    dim iCurrentMessage as integer

    for iCurrentMessage = 1 to 2

        kontext "Active"
        if ( Active.exists( 2 ) ) then
            if ( instr( Active.getText() , cMessage ) > 0 ) then
                printlog( "Correct macro has been executed" )
                hTestMacroType2() = true
            else
                warnlog( "Incorrect macro executed: " )
                printlog( "Expected: " & cMessage )
                printlog( "Found...: " & Active.getText() )
                hTestMacroType2() = false
            endif

            Active.ok()
            WaitSlot()

        else
            warnlog( "Messagebox missing, macro not executed" )
            hTestMacroType2() = false
        endif
    next iCurrentMessage

end function