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


