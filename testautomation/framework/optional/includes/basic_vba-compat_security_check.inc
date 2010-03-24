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
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Test VBA compatibility switches
'*
'\******************************************************************************

testcase tBasicVBACompatSecurityCheck()

    printlog( "Test VBA Security feature" )
    printlog( "VBA macros may not ignore global macro security setting" )
    
    ' This test case is based on the use cases provided in issue #i109699
    
    dim cFile as string : cFile = "Book1.xls"
    
    dim cTestFile as string
        cTestFile = gTesttoolPath & "framework/optional/input/vba-compat/" & cFile
        
    const IMPORT_EXCEL_MACROS = TRUE
    const EXEC_EXCEL_MACROS   = TRUE

    const MACRO_IDENTIFICATION_MESSAGE = "Clicked"

    dim iSecurityLevel as integer


    printlog( "Set VBA macro handling to import and execute" )
    hSetExcelBasicImportMode( IMPORT_EXCEL_MACROS, EXEC_EXCEL_MACROS )

    printlog( "Macros should execute for security levels LOW and MEDIUM" )
    for iSecurityLevel = GC_MACRO_SECURITY_LEVEL_LOW to GC_MACRO_SECURITY_LEVEL_MEDIUM

        printlog( "Adjust security level" )
        hSetMacroSecurityAPI( iSecurityLevel )

        printlog( "Load the test file" )
        hFileOpen( cTestFile )

        printlog( "Probe for macro execution warning..." )
        if ( hAllowMacroExecution() ) then
            printlog( "Macro execution warning displayed, good" )
        else
            warnlog( "Macro execution warning is missing" )
        endif

        kontext "Active"
        printlog( "Probe for macro..." )
        if ( Active.exists( 3 ) ) then
            if ( Active.getText() = MACRO_IDENTIFICATION_MESSAGE ) then
                printlog( "Macro was executed" )
                Active.ok()
            else
                warnlog( "Macro was not executed" )
            endif
        else
            warnlog( "Expected macro was not executed" )
        endif

        printlog( "Close file" )
        hDestroyDocument()

    next iSecurityLevel

    printlog( "Macros should *not* execute for HIGH and VERY HIGH security levels" )
    for iSecurityLevel = GC_MACRO_SECURITY_LEVEL_HIGH to GC_MACRO_SECURITY_LEVEL_VERYHIGH

        printlog( "Adjust security level" )
        hSetMacroSecurityAPI( iSecurityLevel )

        printlog( "Reload file" )
        hFileOpen( cTestFile )

        kontext "Active"
        printlog( "Probe for macro or security message" )
        if ( Active.exists() ) then
            if ( Active.getText() = MACRO_IDENTIFICATION_MESSAGE ) then
                warnlog( "Macro was executed" )
                Active.ok()
            else
                printlog( "Macro was not executed, message displayed" )
                Active.ok()
            endif
        else
            warnlog( "Message for disabled macro execution was not displayed" )
        endif

        printlog( "Close file" )
        hDestroyDocument()

    next iSecurityLevel

    printlog( "Reset VBA import options to default" )
    hSetExcelImportModeDefault()

    printlog( "Reset macro security level to default" )
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )

endcase

