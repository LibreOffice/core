'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_modulenames.inc,v $
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
'/************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Test modulenames on the tabbar
'*
'\******************************************************************************

testcase tBasicIdeModuleNames

    '///<h1>Naming conventions in BASIC-IDE module-/dialog-tabs</h1>

    dim rc as integer
    dim brc as boolean
    dim i as integer
    dim iCurrentName as integer
    const CMODULE = "TModuleNames"
    dim cTabName as string

    gApplication = "WRITER"
    hCreateDocument()

    ' ------ prerequisites ---------
    
    '///<ul>
    '///+<li>Create a new module for the current (writer) document</li>
    brc = hInitBasicIde( CMODULE )

   
    brc = hInsertMacro( 1 )
    if ( rc <> 0 ) then
       warnlog( "Failed to insert macro" )
    endif
   
    '///+<li>Verify that the name makes it to the BASIC macro organizer</li>
    cTabName = hGetTabNameFromOrganizer()

    '///+<li>Try to name the tab something containing invalid characters (should fail)</li>
    Randomize
    iCurrentName = Int( 24 * RND ) + 1 ' Range from 1 to 24
    
    cTabName = hCreateInvalidModuleName( iCurrentName )
    rc = hRenameTab( cTabName )
    if ( rc = 1 ) then
        hHandleInvalidNameWarning( cTabname )
    else
        warnlog( "Warning missing" )
    endif
        
    '///+<li>Try to name the tab to the current name (should work)</li>
    cTabname = CMODULE
    rc = hRenameTab( cTabname )
    if ( rc <> 0 ) then
        brc = hHandleInvalidNameWarning( cTabname ) 
        if ( brc ) then
            warnlog( "Failed to set valid name" )
        endif      
    endif

    
    '///+<li>Finally try to give the tab a valid name (should work)</li>
    cTabname = hCreateInvalidModuleName( 0 )
    rc = hRenameTab( cTabName )
    if ( rc <> 0 ) then
        brc = hHandleInvalidNameWarning( cTabname ) 
        if ( brc ) then
            warnlog( "Failed to set valid name" )
        endif      
    endif
    
    
    '///+<li>Verify that we are still in the same IDE-Window</li>
    rc = hTestMacro( 1 )
    if ( rc <> 1 ) then
        warnlog( "The open macro-module is not the one that was expected" )
    endif
    
    '///+<li>Close IDE and document</li>
    hCloseBasicIDE()
    
    hDestroyDocument()
    '///</ul>

endcase

'*******************************************************************************

testcase tInvalidModuleNames

    '///<h1>Module naming conventions: Forbidden characters in module names</h1>

    dim brc as boolean
    dim cMsg as string
    dim iCurrentName as Integer
    
    dim sSeparator as String 
    dim cModuleName as string

    '///<ul>
    '///+<li>Open a new writer document</li>
    '///+<li>Open the BASIC Macro Organizer</li>
    '///+<li>Select the last module (Document BASIC)</li>
    '///+<li>Click the &quot;New&quot; button</li>
    hBasicModuleCreateNew()

    '///+<li>Try a couple of names including '-', '+', '!', '(' ... <br>
    '///+ -&gt; Invalid name warnings are expected for all of these names</li>

    Randomize
    iCurrentName = Int( 24 * RND ) + 1 ' Range from 1 to 24

    cModuleName = hCreateInvalidModuleName( iCurrentName )
    printlog( "" )
    printlog( "Trying module name: " & cModuleName )

    brc = hNameBasicModule( cModuleName )
    
    '///+<li>Check if the Basic-Ide is open (should not be)</li>
    if ( brc ) then
        warnlog( "Basic-Ide should not open for invalid module names" )
        
        ' try to recover and continue for other names
        brc = hCloseBasicIde()
        brc = hDestroyDocument
        brc = hBasicModuleCreateNew()
        
        ' if we cannot recover, exit the test
        if ( not brc ) then
            goto endsub
        endif
    endif


    '///+<li>Close the naming dialog</li>
    printlog( "Close the naming dialog (cancel)" )
    kontext "neuesmodul"
    if ( NeuesModul.Exists() ) then
        NeuesModul.Cancel()
    endif

    '///+<li>Close the macro dialog</li>
    printlog( "Clsoe macro dialog (if it exists)" )
    kontext "makro"
    if ( Makro.Exists() ) then
        Makro.Close()
    endif
    
    '///+<li>Close the last document</li>
    printlog( "Close the document" )
    brc = hDestroyDocument()
    '///</ul>
    
endcase

'*******************************************************************************

testcase tValidModuleNames

    '///<h1>Module naming conventions: Basic keywords, underscores, long names</h1>

    dim brc as boolean
    dim cMsg as string
    dim iCurrentName as Integer
    
    dim sKeyword as String

    '///<ul>
    '///+<li>Open a new writer document</li>
    '///+<li>Open the BASIC Macro Organizer</li>
    '///+<li>Select the last module (Document BASIC)</li>
    '///+<li>Click the &quot;New&quot; button</li>
    hBasicModuleCreateNew()

    '///+<li>Try a couple of basic keywords as module names such as 
    '///+ open, print, select, function etc.</li>
    Randomize
    iCurrentName = Int( 8 * RND ) + 1 ' Range from 1 to 8

    select case iCurrentName
        case 1  : sKeyword = "option"
        case 2  : sKeyword = "sub"
        case 3  : sKeyword = "function"
        case 4  : sKeyword = "end"
        case 5  : sKeyword = "exit"
        case 6  : sKeyword = "_underscore1"
        case 7  : sKeyword = "underscore_2"
        case 8  : sKeyWord = "ThisIsQuiteALongNameForAModuleDontYouThink"
    end select

    printlog( "" )
    printlog( "Trying module name: " & sKeyword )

    brc = hNameBasicModule( sKeyword )
    
    '///+<li>Check if the Basic-Ide is open (should not be)</li>
    if ( brc ) then
        
        ' try to recover and continue for other names
        brc = hCloseBasicIde()
        brc = hDestroyDocument
        brc = hBasicModuleCreateNew()
        
        ' if we cannot recover, exit the test
        if ( not brc ) then
            goto endsub
        endif
    endif

    '///+<li>Close the naming dialog</li>
    printlog( "Close the naming dialog (cancel)" )
    kontext "neuesmodul"
    if ( NeuesModul.Exists() ) then
        NeuesModul.Cancel()
    endif

    '///+<li>Close the macro dialog</li>
    printlog( "Clsoe macro dialog (if it exists)" )
    kontext "makro"
    if ( Makro.Exists() ) then
        Makro.Close()
    endif
    
    '///+<li>Close the last document</li>
    printlog( "Close the document" )
    brc = hDestroyDocument()
    '///</ul>
    
endcase

'*******************************************************************************

function hBasicModuleCreateNew() as boolean

    '///<h3>Create a new doc, open Basic organizer, select last module, click New...</h3>
    '///<i>Starting point: The first doc!</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcode (boolean) </li>
    '///<ul>
    '///+<li>True if module naming dialog is open</li>
    '///+<li>False on any other condition</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    dim brc as boolean
        brc = false
        
    dim iPos as integer
        
    const CFN = "hBasicModuleCreateNew::"

    '///+<li>Open a new writer document</li>
    gApplication = "WRITER"
    brc = hCreateDocument()
    
    '///+<li>Open the BASIC Macro Organizer</li>
    brc = hOpenBasicOrganizerFromDoc()
    if ( not brc ) then
        warnlog( CFN & "Could not open the BASIC Macro Organizer, aborting" )
        hDestroyDocument()
    endif     
    
    '///+<li>Select the last module (Document BASIC)</li>
    if ( brc ) then
        iPos = hSelectTheLastNode( MakroAus )
        if ( not Neu.isEnabled() ) then
            warnlog( CFN & "New button is disabled for the current module, aborting" )
            kontext "Makro"
            Makro.cancel()
            brc = hDestroyDocument()
        endif
    endif 
    
    '///+<li>Click the &quot;New&quot; button</li>
    '///+<li>Verify that the renaming dialog is open</li>
    if ( brc ) then
    
        printlog( "Click 'New'" )
        Neu.Click()
        
        kontext "NeuesModul"
        if ( Neuesmodul.exists() ) then
            brc = true
            printlog( CFN & "Naming dialog is open"
        endif
    endif 
    
    hBasicModuleCreateNew() = brc
    '///</ul>
    
end function

'*******************************************************************************

function hHandleInvalidNameWarning( cTabName as string ) as boolean

    '///<h3>Handle the &quot;Invalid name&quot; dialog when renaming a module</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the tab</li>
    '///<ul>
    '///+<li>Used for debugging purpose (printlogs only)</li>
    '///</ul>    
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE if warning was displayed and closed correctly</li>
    '///+<li>FALSE on any other condition</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>        

    dim brc as boolean : brc = false
    dim iTry as integer
    
    const CFN = "hHandleInvalidNameWarning::"

    '///+<li>Test for a messagebox (Invalid name warning), warn if missing</li>
    kontext "Active"
    for iTry = 1 to 5
    
    	if ( active.exists( 1 ) ) then
    	
    	    if ( brc ) then
    	    	warnlog( CFN & "Too many invalid name warnings" )
    	    endif
    	
        	'///+<li>Retrieve the text from the messagebox, printlog</li>
    	    printlog( CFN & "Message: " & active.getText() )
    	    
        	'///+<li>Click OK on the messagebox</li>
	        active.ok()
    	    brc = true
    	   
    	else
    	
    		if ( not brc ) then
    			warnlog( CFN & "Invalid name warning missing" )
    	    endif
    	
        endif
    
    next iTry
    
    '///+<li>Return to Basic Ide, take focus from Tabbar</li>
    kontext "basicide"
    tabbar.typekeys( "<ESCAPE>" , true )       
    hHandleInvalidNameWarning() = brc
    '///</ul>

end function

'*******************************************************************************

function hCreateInvalidModuleName( iName as integer ) as string

    '///<h3>Retrieve an invalid module name for Basic IDE by index</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Index (integer)</li>
    '///<ul>
    '///+<li>0 = insert underscore which creates a valid name</li>
    '///+<li>1 ... 23 = insert characters like &, +, i, ;, etc. (invalid)</li>
    '///+<li>&gt; 23 = no character will be definde</li>
    '///</ul>    
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Name for a module (string)</li>
    '///<ul>
    '///+<li>Name of type &quot;ttModule&lt;Char&gt;X&quot;</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>    

    dim sSeparator as string
    
    ' Note: We cannot test for "<" and ">" because these are the characters
    '       that identify keystrokes for the .typeKeys() method.

    '///+<li>Assign a character by index</li>
    select case iName
        case 0  : sSeparator = "_"
        case 1  : sSeparator = "-"
        case 2  : sSeparator = "."
        case 3  : sSeparator = ","
        case 4  : sSeparator = "+"
        case 5  : sSeparator = ":"
        case 6  : sSeparator = "!"
        case 7  : sSeparator = "$"
        case 8  : sSeparator = "("
        case 9  : sSeparator = ")"
        case 10 : sSeparator = "="
        case 11 : sSeparator = "?"
        case 12 : sSeparator = "*"
        case 13 : sSeparator = " "
        case 14 : sSeparator = "&"
        case 15 : sSeparator = "\"
        case 16 : sSeparator = "/"
        case 17 : sSeparator = "§"
        case 18 : sSeparator = """"
        case 19 : sSeparator = "'"
        case 20 : sSeparator = "@"
        case 21 : sSeparator = "["
        case 22 : sSeparator = "]"
        case 23 : sSeparator = "%"
        case 24 : sSeparator = CHR$( 387 )
        case else : sSeparator = ""
    end select

    '///+<li>Build a string containing the character</li>
    hCreateInvalidModuleName() = "ttModule" & sSeparator & "X"
    '///</ul>
    
end function
