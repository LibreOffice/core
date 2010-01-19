'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_vba-compat_import_enabled.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:13 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Test VBA compatibility switches
'*
'\******************************************************************************

testcase tBasicVBACompatImportEnabled()

    printlog( "Test VBA compatibility switch / executable Microsoft(R) Excel(R) Macros" )
    printlog( "Test case 3: Import macros and set them executable" )
    
    
    ' This test case is based on the use cases provided in issue #i88690
    ' Spec: http://specs.openoffice.org/appwide/options_settings/Option_Dialog.odt
    
    dim cFile as string : cFile = "vba-test.xls"
    
    dim cTestFile as string
        cTestFile = gTesttoolPath & "framework/optional/input/vba-compat/" & cFile
        
    dim cNodeCount as integer
    
    ' note that index 0 and 1 are ommitted intentionally
    dim caNodeData( 3 ) as string
        caNodeData( 2 ) = "Modul1"
        caNodeData( 3 ) = "Modul2"
        
    dim caScripts( 3 ) as string
        caScripts( 2 ) = "ConcatFct Ende"
        caScripts( 3 ) = "WriteIt"
        
    dim iCurrentModule as integer  
    dim iCurrentScript as integer
    dim cCurrentModule as string
    dim cTempString as string
    dim bFound as boolean  
    
    ' Depending on the mode of macro import we have differtent basic libraries listed
    const NODE_COUNT = 74
 
    const DOCUMENT_POSITION_OFFSET = -3
    
    const IMPORT_EXCEL_MACROS = TRUE
    const EXEC_EXCEL_MACROS   = TRUE
 
    printlog( "Set macro security to low" )
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_LOW )
    
    printlog( "Open Tools/Options" )

    hSetExcelBasicImportMode( IMPORT_EXCEL_MACROS, EXEC_EXCEL_MACROS )
    
    printlog( "Load the test file" )
    hFileOpen( cTestFile )
    
    printlog( "Open the Basic organizer" )
    hOpenBasicOrganizerFromDoc()
    
    printlog( "Expand all nodes" )
    cNodeCount = hExpandAllNodes( MakroAus )
    
    printlog( "Verify that we have the correct node count for the current mode." )
    if ( cNodeCount <> NODE_COUNT ) then
        warnlog( "The number of nodes is incorrect: " & cNodeCount )
        printlog( "Expected were...................: " & NODE_COUNT )
    endif
    
    printlog( "Verify position of the document node." )
    MakroAus.select( cNodeCount + DOCUMENT_POSITION_OFFSET )
    cTempString = MakroAus.getSelText()
    if ( cTempString <> cFile ) then
        qaerrorlog( "The filename does not match. File may be read-only." )
        if ( instr( cTempString, cFile ) > 1 ) then
            printlog( "Filename is contained in the UI string." )
            select case ( getAttr( cTestFile ) )
            case 0 : warnlog( "The file appears to be read-only but it is " & _
                              "not write-protected on filesystem level." )
            case 1 : printlog( "The file is read-only on filesystem level. Ok." )
            case else
                warnlog( "getAttr() returned unexpected value. 1 or 2 are allowed" )
                printlog( "RC from getAttr() is: " &  getAttr( cTestFile ) )
            end select
        else
            qaerrorlog( "The document node is not at the expected position." )
            qaerrorlog( "Object at this position is: " & MakroAus.getSelText() )
        endif
    endif
    
    for iCurrentModule = 2 to 3
    
        printlog( "Look for: " & caNodeData( iCurrentModule ) )
        
        bFound = FALSE
    
        MakroAus.select( cNodeCount + DOCUMENT_POSITION_OFFSET + iCurrentModule )
        cCurrentModule = MakroAus.getSelText()
        
        if ( cCurrentModule = caNodeData( iCurrentModule ) ) then 
            bFound = TRUE
            for iCurrentScript = 1 to MakroListe.getItemCount()
                MakroListe.select( iCurrentScript )
                if ( instr( caScripts( iCurrentModule ), MakroListe.getSelText() ) = 0 ) then
                    warnlog( "Script for the current module not found" )
                    bFound = FALSE
                else
                    printlog( "Script found at pos" & iCurrentScript )
                endif
            next iCurrentScript
        endif
            
        if ( not bFound ) then
            warnlog( "The node was not found: " & cCurrentModule )
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

