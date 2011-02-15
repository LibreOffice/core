'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
'*  
'*  short description : Test VBA compatibility switches
'*
'\******************************************************************************

testcase tBasicVBACompatThisWorkBook()

    printlog( "Functionality of .thisWorkbook" )
    ' Based on issue #i110672

    const IMPORT_EXCEL_MACROS  = TRUE
    const EXEC_EXCEL_MACROS    = TRUE
    const DOCUMENT_NAME        = "vba-thisworkbook.xls"
    const LIBRARY_NAME         = "Standard"
    const MODULE_NAME          = "Module1"
    const EXPECTED_MACRO_COUNT = 2
    const MAX_REPEAT_COUNT     = 2
    const MAX_WAIT             = 2

    dim macro_identifier( EXPECTED_MACRO_COUNT )
        macro_identifier( 1 ) = "Test 1 passed."
        macro_identifier( 2 ) = "Test 2 passed."

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

    printlog( "Execute the procedure twice to trigger issue 111553" )
    for iRepeat = 1 to MAX_REPEAT_COUNT

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
            iMacroPosition = hClickButton( Ausfuehren )

            printlog( "Check for two expected messageboxes" )
            for iCurrentMacro = 1 to EXPECTED_MACRO_COUNT

                kontext "Active"
                printlog( "Check for messagebox: " & macro_identifier( iCurrentMacro ) )
                if ( Active.exists( MAX_WAIT ) ) then

                    if ( Active.getText() = macro_identifier( iCurrentMacro ) ) then
                        printlog( "Correct macro executed" )
                    else
                        printlog( "Macro execution error, trying to identify..." )
                        if ( instr( Active.getText(), "BASIC runtime error." ) > 0 ) then
                            if ( iRepeat = MAX_REPEAT_COUNT ) then
                                warnlog( "#i111533 - Cannot execute ThisWorkbook macro twice" )
                                Active.ok()
                                hFileCloseAll()
                                goto endsub
                            else
                                warnlog( "Executing macro failed" )
                                goto endsub
                            endif
                        else
                            warnlog( "Unknown messagebox. Aborting test" )
                            goto endsub
                        endif
                    endif

                    Active.ok()

                else
                    warnlog( "Expected messagebox not displayed within reasonable timeframe" )
                    goto endsub
                endif

            next iCurrentMacro
        else
            warnlog( "A macro with the name <" & MODULE_NAME & "> could not be found" )
            goto endsub
        endif

    next iRepeat

    printlog( "Cleanup" )
    hCloseDocument()
    hSetExcelImportModeDefault()    
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )

endcase


