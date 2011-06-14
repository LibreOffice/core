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

testcase tBasicVBACompatImportDisabled()

    printlog( "Test VBA compatibility switch / executable Microsoft(R) Excel(R) Macros" )
    printlog( "Test case 2: Import macros but do not set the executable mode" )
    
    ' This test case is based on the use cases provided in issue #i88690
    ' Spec: http://specs.openoffice.org/appwide/options_settings/Option_Dialog.odt

    const NODE_COUNT = 78

    const DOCUMENT_POSITION_OFFSET = -7

    const IMPORT_EXCEL_MACROS = TRUE
    const EXEC_EXCEL_MACROS   = FALSE

    const DOCUMENT_NAME = "vba-test.xls"
    const MATCH_NONE    = 0
    const MATCH_EXACT   = 1
    const MATCH_PARTLY  = 2
    
    dim cTestFile as string
        cTestFile = gTesttoolPath & "framework/optional/input/vba-compat/" & DOCUMENT_NAME
        
    dim cNodeCount as integer
    
    dim caNodeData( 7 ) as string
        caNodeData( 1 ) = "Standard"
        caNodeData( 2 ) = "DieseArbeitsmappe"
        caNodeData( 3 ) = "Modul1"
        caNodeData( 4 ) = "Modul2"
        caNodeData( 5 ) = "Tabelle1"
        caNodeData( 6 ) = "Tabelle2"
        caNodeData( 7 ) = "Tabelle3"    
        
    dim iCurrentModule as integer  
    dim cCurrentModule as string
    dim bFound as boolean  
 
    printlog( "Set macro security to low" )
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_LOW )
    
    printlog( "Open Tools/Options" )

    hSetExcelBasicImportMode( IMPORT_EXCEL_MACROS, EXEC_EXCEL_MACROS )
    
    printlog( "Load the test file" )
    hFileOpen( cTestFile )
    
    printlog( "Open the Basic organizer" )
    ToolsMacro_uno
    
    printlog( "Expand all nodes" )
    kontext "Makro"
    cNodeCount = hExpandAllNodes( MakroAus )
    
    printlog( "Verify that we have the correct node count for the current mode" )
    if ( gOOo ) then	
        if ( cNodeCount <> NODE_COUNT_OOO ) then  warnlog( "The number of nodes is incorrect: " & cNodeCount )
    else
        if ( cNodeCount <> NODE_COUNT ) then  warnlog( "The number of nodes is incorrect: " & cNodeCount )
    endif
    
    printlog( "Verify position of the document node" )
    MakroAus.select( cNodeCount + DOCUMENT_POSITION_OFFSET )
    select case( hCompareSubStrings( MakroAus.getSelText(), DOCUMENT_NAME  ) )
    case MATCH_NONE   : warnlog ( "The document is not listed at the expected position" )
    case MATCH_EXACT  : printlog( "The document is at the expected position and writable" )
    case MATCH_PARTLY : printlog( "The document is at the expected position and write protected" )
    end select
    
    for iCurrentModule = 2 to 7
    
        printlog( "Look for: " & caNodeData( iCurrentModule ) )
        
        bFound = false
    
        MakroAus.select( cNodeCount + DOCUMENT_POSITION_OFFSET + iCurrentModule )
        cCurrentModule = MakroAus.getSelText()
        
        for iCurrentModule = 2 to 7
        
            if ( cCurrentModule = caNodeData( iCurrentModule ) ) then 
                bFound = TRUE
                if ( MakroListe.getSelText() <> caNodeData( iCurrentModule ) ) then
                    warnlog( "Module has incorrect script: " & cCurrentModule )
                    bFound = false
                endif
                exit for
            endif
            
        next iCurrentModule
        
        if ( not bFound ) then
            warnlog( "The node was not found: " & cCurrentModule )
        else
            printlog( "Module found, script found, good" )
        endif
        
    next iCurrentModule
    
    printlog( "Close Macro Organizer" )
    Kontext "Makro"
    Makro.close()
    WaitSlot()
    
    hCloseDocument()
    hSetExcelImportModeDefault()    
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )

endcase


