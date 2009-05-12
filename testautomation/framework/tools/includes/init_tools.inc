'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: init_tools.inc,v $
'*
'* $Revision: 1.3 $
'*
'* last change: $Author: obo $ $Date: 2008-07-25 07:44:24 $
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
'*  short description : Tools to put the office into a defined state 
'*
'\******************************************************************************

function hInitSingleDoc() as boolean

    '///<h3>Make sure exactly one single writer document is open</h3>
    '///<i>The wizards cannot be triggered correctly from the backing window. 
    '///+ As a workaround this function checks the amount of open documents and
    '///+ creates exactly one unchanged Writer document</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>
    '///+<li>No input parameters</li>
    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Errorcondition (Boolean)</li>
    '///<ul>
    '///+<li>TRUE: Exactly one Writer document is open</li>
    '///+<li>FALSE: Any error</li>
    '///</ul>
    '///</ol>

    const CFN = "hInitSingleDoc::"
    dim cOldApplication as string

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Close all documents until we are on the backing window</li>
    do while ( getDocumentCount > 0 ) 
        call hCloseDocument()
    loop
    
    '///+<li>Save the previous gApplication</li>
    cOldApplication = gApplication
    
    '///+<li>Set gApplication to WRITER</li>
    gApplication = "WRITER"
    
    '///+<li>Open one new Writer document</li>
    call hNewDocument()
    
    '///+<li>Verify that exactly one document is open</li>
    if ( getDocumentCount = 1 ) then 
        printlog( CFN & "A single unchanged writer document is open" )
        hInitSingleDoc() = true
    else
        printlog( CFN & "Failed to open just one single writer document" )
        hInitSingleDoc() = false
    endif
    
    '///+<li>Restore gApplication</li>
    gApplication = cOldApplication
    '///</ul>

end function

'*******************************************************************************

function hInitBackingMode() as boolean

    use "global\tools\includes\optional\t_docfuncs.inc"

    '///<h3>Make that we are on the backing window (no open documents)</h3>
    '///<i>Close all open documents</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>
    '///+<li>No input parameters</li>
    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Errorcondition (Boolean)</li>
    '///<ul>
    '///+<li>TRUE: No open documents are present</li>
    '///+<li>FALSE: Any error</li>
    '///</ul>
    '///</ol>

    const CFN = "hInitBackingMode::"

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Close all documents until we are on the backing window</li>
    do while ( getDocumentCount > 0 ) 
        hCloseDocument()
    loop
    
    '///+<li>verify that we do not have any open documents left (redundant check)</li>
    if ( getDocumentCount = 0 ) then
        printlog( CFN & "Office is in backing mode." )
        hInitBackingMode() = true
    else
        printlog( CFN & "Office is in undefined state." )
        hInitBackingMode() = false
    endif
    '///</ul>
    
end function

'*******************************************************************************

function hInitWriteDocIdentifier( cString as string ) as boolean


    '///<h3>Write a specific string to an open writer document</h3>
    '///<i>This function verifies that exactly one document is open, that this is a 
    '///+ Writer document and writes the string to the document</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>A free form string (String) which serves as identifier for the document</li>
    '///<ul>
    '///+<li>The first character should be uppercase</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Errorcondition (Boolean)</li>
    '///<ul>
    '///+<li>TRUE: The string was written correctly</li>
    '///+<li>FALSE: Too many open documents</li>
    '///+<li>FALSE: Not a Writer document</li>
    '///+<li>FALSE: Any other error</li>
    '///</ul>
    '///</ol>

    const CFN = "hInitWriteDocIdentifier::"

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Verify number of open documents</li>
    if ( getDocumentCount <> 1 ) then
        printlog( CFN & "Incorrect document count" )
        hInitWriteDocIdentifier() = false
        exit function
    endif
    
    '///+<li>Verify that it is a writer document</li>
    kontext "DocumentWriter"
    if ( not DocumentWriter.exists() ) then
        printlog( CFN & "Open document is not a text document" )
        hInitWriteDocIdentifier() = false
        exit function
    endif
    
    '///+<li>Write the string</li>
    kontext "DocumentWriter"
    DocumentWriter.typeKeys( "<MOD1 END>" )
    DocumentWriter.typeKeys( "<MOD1 SHIFT HOME>" )
    DocumentWriter.typeKeys( "<DELETE>" )
    DocumentWriter.typekeys( cString )
    
    '///+<li>Verify the string</li>
    DocumentWriter.typeKeys( "<MOD1 END>" )
    DocumentWriter.typeKeys( "<MOD1 SHIFT HOME>" )
    EditCopy
    if ( getClipboardText = cString ) then
        printlog( CFN & "Document has been successfully modified." )
        hInitWriteDocIdentifier() = true
    else
        printlog( CFN & "Could not verify document identification string" )
        hInitWriteDocIdentifier() = false
    endif    
    '///</ul>

end function
