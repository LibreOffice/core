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
'*  short description : Hit update button for an extension
'*
'\******************************************************************************

testcase tExtensionUpdate

    'warnlog( "#i81543# Office crashes when Testtool executes .exists() method on some objects" )
    'goto endsub

    '///<h3>Hit update button for an extension</h3>
    '///<ul>
    
    const EXTENSION_NAME = "ShortLicense.oxt"
    
    dim cSampleExtension as string
        cSampleExtension = gTesttoolPath & "extensions\optional\input\simpleLicense\" 
        cSampleExtension = cSampleExtension & EXTENSION_NAME
        cSampleExtension = convertpath( cSampleExtension )
        
    dim iDocumentsBefore as integer
    dim iDocumentsAfter as integer
    
    printlog( "" )
    
    if ( dir( cSampleExtension ) <> "" ) then
        printlog( "Using extension: " & cSampleExtension )
    else
        warnlog( "Fatal: Sample extension could not be found: " & cSampleExtension )
        goto endsub
    endif
    printlog( "" )
    
    
    '///+<li>Create a new writer document</li>
    hNewDocument()
    
    '///+<li>get the number of open documents (should be 2)</li>
    iDocumentsBefore = getDocumentCount()
    
    '///+<li>Add sample extension &quot;ShortLicense&quot;</li>
    hExtensionAddGUI( cSampleExtension , "InstallForUser,AcceptLicense,Verbose" )
    
    '///+<li>Reopen the Extension Manager</li>
    printlog( "Reopen Extension Manager" )
    ToolsPackageManager
    
    '///+<li>Click the &quot;Update&quot;-button</li>
    printlog( "Click the update-button" )
    kontext "PackageManager"
    updates.click()
    
    '///+<li>verif y that the update-Dialog is present</li>
    kontext "ExtensionUpdate"
    if ( ExtensionUpdate.exists() ) then
        printlog( "Close update-dialog. Good." )
        ExtensionUpdate.close()
    else
        warnlog( "Extension update dialog is missing" )
    endif
    
    '///+<li>Close the Extension Manager"
    kontext "PackageManager"
    if ( PackageManager.exists() ) then
        printlog( "Close Extension Manager" )
        PackageManager.close()
    endif

    '///+<li>verify that we still have the correct number of documents open</li>
    iDocumentsAfter = getDocumentCount()
    if ( iDocumentsAfter <> iDocumentsBefore ) then
        warnlog( "Incorrect number of documents open. Please verify" )
    else
        printlog( "Correct number of documents are open. Good" )
    endif
    
    '///+<li>Close the second document</li>
    hCloseDocument()
    
    '///+<li>Remove the extension</li>
    hExtensionRemoveGUI( EXTENSION_NAME )
    
    '///</ul>

endcase

