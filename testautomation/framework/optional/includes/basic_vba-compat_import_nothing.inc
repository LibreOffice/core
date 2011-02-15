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

testcase tBasicVBACompatImportNothing()

    printlog( "Test VBA compatibility switch / executable Microsoft(R) Excel(R) Macros" )
    printlog( "Test case 1: Do not import macros at all" )
    
    ' This test case is based on the use cases provided in issue #i88690
    ' Spec: http://specs.openoffice.org/appwide/options_settings/Option_Dialog.odt
' Depending on the mode of macro import we have differtent basic libraries listed
    const NODE_COUNT = 72 ' Do not import Microsoft(R) Excel(R) macros at all
    const MACRO_LIST = 0  ' The document library should have no scripts listed
    const DOCUMENT_POSITION_OFFSET = -1

    const IMPORT_EXCEL_MACROS = FALSE
    const EXEC_EXCEL_MACROS   = FALSE

    const DOCUMENT_NAME = "vba-test.xls"
    const MATCH_NONE    = 0
    const MATCH_EXACT   = 1
    const MATCH_PARTLY  = 2

    dim cTestFile as string
        cTestFile = gTesttoolPath & "framework/optional/input/vba-compat/" & DOCUMENT_NAME
        
    dim cNodeCount as integer
     
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
        if ( cNodeCount <> NODE_COUNT_OOO ) then warnlog( "The number of nodes is incorrect: " & cNodeCount )
    else
        if ( cNodeCount <> NODE_COUNT ) then warnlog( "The number of nodes is incorrect: " & cNodeCount )
    endif
    
    printlog( "Verify position of the document node" )
    MakroAus.select( cNodeCount + DOCUMENT_POSITION_OFFSET )
    select case( hCompareSubStrings( MakroAus.getSelText(), DOCUMENT_NAME  ) )
    case MATCH_NONE   : warnlog ( "The document is not listed at the expected position" )
    case MATCH_EXACT  : printlog( "The document is at the expected position and writable" )
    case MATCH_PARTLY : printlog( "The document is at the expected position and write protected" )
    end select
    
    printlog( "Select the last node, this should be the standard Library for the document" )
    MakroAus.select( cNodeCount ) 
    
    printlog( "Verify that the Standard library for the document has no scripts" )
    if ( MakroListe.getItemCount <> MACRO_LIST ) then
        warnlog( "There should be no macros listed for the current library"
    endif
    
    printlog( "Close Macro Organizer" )
    Kontext "Makro"
    Makro.close()
    WaitSlot()
            
    hCloseDocument()
    hSetExcelImportModeDefault()    
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )

endcase


