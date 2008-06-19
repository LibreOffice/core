'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: errors.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-19 09:02:09 $
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
'*  short description : Reproduction of misc errors when working with extensions
'*
'\******************************************************************************

testcase tExtensionError77436

    '///<h3>Registration error when installing extension, Linux/UNIX only</h3>
    '///<ul>
    
    warnlog( "#i87453# - Failure to load sample extension" )
    goto endsub
    
    if ( lcase( gtSysName ) <> "linux" ) then 
        printlog( "Sample extension is not suitable for Windows or Solaris" )
        goto endsub
    endif
    
    dim irc as integer
    dim brc as boolean
    dim cMsg as string
    
    dim cWorkPath as string
        cWorkPath = gTesttoolPath & "extensions\optional\input\errors\"
        cWorkPath = convertpath( cWorkPath )
        
    const EXTENSION_NAME = "i77436-extension.oxt"
    const MACRO_NAME = "i77436"
    
    '///+<li>Create an initial document, change it so it cannot be replaced</li>
    gApplication = "WRITER"
    do while ( getDocumentCount > 0 ) 
        hCloseDoc()
    loop
    hNewDocument()
    kontext "DocumentWriter"
    DocumentWriter.typeKeys( "Testdocument for tExtensionError77436" )
    
    '///+<li>Add the extension (see code for details)</li>
    irc = hExtensionAddGUI( cWorkPath & EXTENSION_NAME , "verbose,InstallForUser" )
    
    '///+<li>Go to the BASIC IDE, create a new module</li>
    brc = hInitBasicIde( MACRO_NAME )
    if ( not brc ) then
        warnlog( "Failed to init BASIC IDE, fatal" )
        call ExitRestartTheOffice()
        goto endsub
    endif
    
    '///+<li>Insert the macro (that comes with the extension</li>
    irc = hInsertMacroFromFile( MACRO_NAME )
    if ( irc <> 9 ) then
        warnlog( "Retrieved/inserted wrong macro, please check" )
    endif
    
    '///+<li>Close the BASIC IDE</li>
    irc = hCloseBasicIDE()
    
    '///+<li>Run the macro from the Run Macro dialog</li>
    irc = hMacroOrganizerRunMacro( MACRO_NAME )
    
    '///+<li>Verify that the macro was indeed executed - messagebox 1</li>
    kontext "Active"
    if ( Active.exists( 2 ) ) then
        cMsg = Active.getText()
        cMsg = hRemoveLineBreaks( cMsg )
        printlog( "Message 1: " & cMsg )
        active.ok()
    endif
    
    '///+<li>Verify that the macro was indeed executed - messagebox 2</li>
    kontext "Active"
    if ( Active.exists( 2 ) ) then
        cMsg = Active.getText()
        cMsg = hRemoveLineBreaks( cMsg )
        printlog( "Message 2: " & cMsg )
        active.ok()
    endif
    
    '///+<li>Remove the extension again</li>
    irc = hExtensionRemoveGUI( EXTENSION_NAME )
    select case irc
    case 0    : printlog( "Extension was removed, ok" )
    case else : warnlog( "Removing extension failed with rc = " & irc )
    end select
    
    '///+<li>Verify that the number of documents is still correct (should be two)</li>
    if ( getDocumentCount() = 2 ) then  
        warnlog( "Document left over by some error. Closing" )
        hCloseDocument()
    endif
    
    '///+<li>Close all documents until we are on backing window</li>
    do while ( getDocumentCount > 0 ) 
        hCloseDocument()
    loop
    '///</ul>

endcase

