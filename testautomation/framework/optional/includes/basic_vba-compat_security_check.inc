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

testcase tBasicVBACompatSecurityCheck()

    printlog( "Test VBA Security feature" )
    printlog( "VBA macros may not ignore global macro security setting" )
    
    ' This test case is based on the use cases provided in issue #i109699
    
    const IMPORT_EXCEL_MACROS = TRUE
    const EXEC_EXCEL_MACROS   = TRUE

    const MACRO_IDENTIFICATION_MESSAGE = "Clicked"
    const MACRO_NAME = "CommandButton1_Click"

    const WORK_FILE = "framework/optional/input/vba-compat/Book1.xls"

    dim cTestFile as string : cTestFile = gTesttoolPath & WORK_FILE
    dim iSecurityLevel as integer


    printlog( "Set VBA macro handling to import and execute" )
    hSetExcelBasicImportMode( IMPORT_EXCEL_MACROS, EXEC_EXCEL_MACROS )

    for iSecurityLevel = GC_MACRO_SECURITY_LEVEL_MEDIUM to GC_MACRO_SECURITY_LEVEL_VERYHIGH

        printlog( "Adjust security level" )
        hSetMacroSecurityAPI( iSecurityLevel )

        printlog( "Load the test file" )
        hFileOpen( cTestFile )

        printlog( "Deny macro execution or close macros blocked message" )
        if ( iSecurityLevel = GC_MACRO_SECURITY_LEVEL_MEDIUM ) then
            hDenyMacroExecution()
        else
            kontext "Active"
            Active.ok()
        endif

        printlog( "Try to execute macro via macro organizer" )
        hMacroOrganizerRunMacro( MACRO_NAME )

        printlog( "Probe for warning message (or macro)" )
        kontext "Active"
        if ( Active.exists( 2 ) ) then
            if ( Active.getText() = MACRO_IDENTIFICATION_MESSAGE ) then
                warnlog( "Macro was executed though macro execution has been denied" )
                Active.ok()
            else
                printlog( "Macro execution blocked - warning is displayed." )
                Active.ok()
            endif
        else
            warnlog( "No warning message for blocked macro execution" )
        endif

        ' recover silently, the warnings above should say it all.
        kontext "Makro"
        if ( Makro.exists() ) then Makro.cancel()

        printlog( "Close file" )
        hDestroyDocument()

    next iSecurityLevel

    printlog( "Reset VBA import options to default" )
    hSetExcelImportModeDefault()

    printlog( "Reset macro security level to default" )
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )

endcase

