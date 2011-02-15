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


