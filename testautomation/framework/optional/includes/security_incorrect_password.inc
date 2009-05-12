'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: security_incorrect_password.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:15 $
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
'*  short description : Password protect files
'*
'\******************************************************************************

testcase tIncorrectPassword( filetypeID as string )

    '///<H1>Save and load files with incorrect password</H1>
    '///This test saves a file from each application with a password. Then the 
    '///document is closed and reloaded. When prompted an icorrect password is
    '///entered. The document will not be loaded then, an error is displayed.
 
    const WORKFILE = "password"
    dim suffix as string 
    dim workpath as string

    dim rc as integer
    dim brc as boolean
    dim iCurrentFilter as integer
    dim cCurrentFilter as string
    
    const PASSWORD_VALID = "MyPasswd09#+1"
    const PASSWORD_FALSE = "MyPasswd00#+1"
    const DOC_ID = "f_sec_incorrect_password.bas testdocument"
    
    '///<ul>
    '///+<li>Get the workpath from configuration</li>
    workpath = hGetWorkPath()
    
    '///+<li>Get the suffix for the current filetype</li>
    suffix = hGetSuffix( filetypeID )
    
    '///+<li>Delete the workfile (just in case it still exists from prior run)</li>
    printlog( "Delete the workfile - if it was left over by a former testrun" )
    hDeleteFile( workpath & WORKFILE & suffix )    

    '///+<li>Ensure that we have exactly one open document</li>
    printlog( "Make sure we have exactly one single document open" )
    do while( getDocumentCount > 0 )
        hDestroyDocument()
    loop
    hCreateDocument()

    '///+<li>Change the document so the 'save' button becomes active</li>
    printlog( "Change the document" )
    hChangeDoc()

    '///+<li>Click on FileSave</li>
    printlog( "Click on FileSave" )
    FileSave

    Kontext "SpeichernDlg"
    '///+<li>Check the password-checkbox, name the file and click 'Save'</li>
    printlog( "Check the password checkbox" )
    Passwort.check()
    
    '///+<li>Enter a name for the file, click on &quot;Save&quot;</li>
    printlog( "Name the file as " & workpath & WORKFILE )
    Dateiname.settext( workpath & WORKFILE )
    
    '///+<li>Adjust the filter if it differs from &quot;curren&quot;</li>
    if ( filetypeID <> "current" ) then
        for iCurrentFilter = 1 to DateiTyp.getItemCount()
        
            DateiTyp.select( iCurrentFilter )
            cCurrentFilter = DateiTyp.getSelText()
            if ( instr( cCurrentFilter , suffix ) > 0 ) then
                printlog( "Using custom filter: " & cCurrentFilter )
                exit for
            endif
        next iCurrentFilter
    else
        printlog( "Using default filter: " & DateiTyp.getSelText() )
    endif
    
    printlog( "Click on the Save-Button" )
    Speichern.click()

    '///+<li>On the password-dialog enter a password and confirm it (confirm with 
    '///+ incorrect password on first attempt), close warning, reenter correct pair 
    '///+ of passwords.</li>
    brc = hSecurityEnterPasswordOnSave( PASSWORD_VALID )

    '///+<li>Close the document, verify that only one document remains open</li>
    if ( brc ) then

        kontext "AlienWarning"
        if ( AlienWarning.exists( 3 ) ) then
            AlienWarning.ok()
            if ( filetypeID <> "current" ) then
                printlog( "Accepted to save in alien format" )
            else
                warnlog( "Alien warning not expected for default file format" )
            endif
        endif

        printlog( "Close the document" )
        brc = hDestroyDocument()
        if ( not brc ) then
            warnlog( "Something went wrong while closing the document. please check" )
        endif
        
    else
        warnlog( "Something went wrong setting the password for the docuemnt" )
    endif

    '///+<li>Load the file again, when prompted enter incorrect password</li>
    printlog( "Load the file again" )
    hFileOpen( workpath & WORKFILE & suffix )
    brc = hSecurityEnterPasswordOnLoad( PASSWORD_FALSE , false )
    if ( not brc ) then
        warnlog( "Something went wrong while using the password dialog" )
    endif
    
    '///+<li>Close the current document</li>
    hDestroyDocument()

    '///+<li>Verify that we have no open documents</li>
    printlog( "Verify that we have no open documents" )
    if ( getDocumentCount <> 0 ) then
        warnlog( "No documents should be open at this point" )
    endif

    '///+<li>Delete the workfile</li>
    printlog( "Delete the workfile" )
    hDeleteFile( workpath & WORKFILE & suffix )

    '///</ul>

endcase

