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