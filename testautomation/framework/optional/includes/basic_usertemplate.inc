'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
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
'*  short description : My Macros/standard execution with user defined template
'*
'\******************************************************************************

testcase tMacroUsertemplate


    ' requires: Default template path, default security level (medium)
    
    ' Details:
    ' This test checks for some really weird behavior. A macro is created 
    ' for the standard lib in My Macros - this means the macro is stored with
    ' the application, not the document. 
    ' When creating a document based on a user-created template the macro
    ' will not execute, instead a com.sun.star... errormessage is displayed

    dim irc as integer  ' returncode

    dim cMacroName as string ' Temp variable to store the name of current macro
    
    const DOC_IDENTIFIER = "A test document for tMacroUserTemplate"

    dim sPathOut as string ' output path for workfile
    const FILEOUT = "tMacroUserTemplate"
    const FILTER = "writer8_template" 
        
    dim iCurrentNode as integer

    sPathOut = convertpath( gOfficePath & "user\template\" )
    hDeleteFile( sPathOut & FILEOUT & ".ott" )
    
    hInitSingleDoc()
    kontext "DocumentWriter"
    DocumentWriter.typeKeys( DOC_IDENTIFIER )

    printlog( "Create a new document" )
    hCreateDocument()

    printlog( "Open the macro organizer" )
    ToolsMacro_uno

    printlog( "Search for the module" )
    kontext "Makro"
    iCurrentNode = hSelectNodeByName( MakroAus, "Module1" )
    if ( iCurrentNode = 0 ) then    
        warnlog( "#i73521# - The expected node could not be found. Aborting test" )
    	kontext "Makro"
    	hCloseDialog( Makro, "close" )
    	hDestroyDocument()
    	goto endsub
    endif        
    
    printlog( "Find the macro we are going to replace" )
    if ( MakroListe.getItemCount() > 0 ) then
    	cMacroName = MakroListe.getSelText()
	    if ( lcase( cMacroName ) <> "main" ) then
    	    warnlog( "Test abort: Incorrect macro is selected: " & cMacroName )
        	printlog( "This should have been the <Main> Macro" )
        	kontext "Makro"
        	hCloseDialog( Makro, "cancel" )
        	hDestroyDocument()
        	goto endsub
    	endif
    else
    	warnlog( "There is no macro listed for the current module, the test cannot continue" )
    	kontext "Makro"
    	hCloseDialog( Makro, "close" )
    	hDestroyDocument()
    	goto endsub
    endif

    printlog( "Edit" )
    Bearbeiten.click()

    printlog( "Insert test macro" )
    hInsertMacro( 3 )

    printlog( "Close BASIC IDE" )
    hCloseBasicIde()

    printlog( "Close Document" )
    hDestroyDocument()

    printlog( "Open new document" )
    hCreateDocument

    printlog( "Open Macro Organizer" )
    ToolsMacrosRunMacro

    printlog( "Find the Macro again" )
    cMacroName = hExecScript_tMacroUserTemplate() ' local function, see below
    if ( lcase( cMacroName ) <> "main" ) then
        warnlog( "Test abort: Incorrect macro is selected: " & cMacroName )
        printlog( "This should have been the <Main> Macro" )
        ScriptSelector.cancel() 
        hDestroyDocument()
        goto endsub
    endif

    printlog( "Run the macro" )
    ScriptSelector.ok()

    printlog( "Check for macro" )
    kontext "Active"
    if ( active.exists( 3 ) ) then
        printlog( "Messagebox: " & active.getText() )
        active.ok()
    else
        warnlog( "The Macro has not been executed" )
    endif

    printlog( "Close the document" )
    hDestroyDocument()

    printlog( "Open a new document" )
    hCreateDocument() 
    
    printlog( "" )
    printlog( "Save as template" )
    hFileSaveAsWithFilterKill( sPathOut & FILEOUT, FILTER )

    printlog( "Close the document" )
    hDestroyDocument()
    
    printlog( "" )
    printlog( "File New from Template" )
    FileNewFromTemplate

    printlog( "Find the template" )
    irc = hFindTemplate( FILEOUT )
    hSelectDocumentObject( irc, 1 )
    
    printlog( "" )
    printlog( "Open the <Run Macro> dialog" )
    ToolsMacrosRunMacro

    printlog( "Try to find the macro" )
    cMacroName = hExecScript_tMacroUserTemplate() ' local function, see below
    if ( lcase( cMacroName ) <> "main" ) then
        warnlog( "Test abort: Incorrect macro is selected: " & cMacroName )
        printlog( "This should have been the <Main> Macro" )
        ScriptSelector.cancel() 
        hDestroyDocument()
        goto endsub
    endif

    printlog( "Run" )
    ScriptSelector.ok()

    printlog( "Verify macro execution" )
    kontext "Active"
    if ( active.exists( 3 ) ) then
    
        if ( instr( active.getText() , "com.sun" ) <> 0 ) then
            warnlog( "#i58527# - unable to run macro with user-template loaded" )
        endif
        
        if ( active.getText() = "TTMacro3" ) then
            printlog( "The macro was executed" )
        else
            printlog( "Unknown dialog: " & hRemoveLineBreaks( active.getText() ) )
        endif
        
        active.ok()
    else
        warnlog( "Macro not executed / no warning" )
    endif

    printlog( "Close all open documents" )
    hFileCloseAll()

    printlog( "Delete test template" )
    hDeleteFile( sPathOut & FILEOUT & ".ott" )


endcase

'*******************************************************************************

function hExecScript_tMacroUserTemplate() as string

    Kontext "ScriptSelector"
    hSelectTopNode( LibraryTreeList )
    hExpandNode( LibraryTreeList , 1 )
    hSelectNode( LibraryTreeList , 2 )
    hExpandNode( LibraryTreeList , 2 )
    hSelectNode( LibraryTreeList , 3 )
    
    hExecScript_tMacroUserTemplate() = ScriptList.getSelText()

end function
