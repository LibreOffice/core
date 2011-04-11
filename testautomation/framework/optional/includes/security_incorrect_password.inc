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
'/************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Password protect files
'*
'\******************************************************************************

testcase tIncorrectPassword( filetypeID as string )
 
    dim iCurrentFilter as integer
    dim cCurrentFilter as string
    dim filepath as string
    
    const PASSWORD_VALID = "MyPasswd09#+1"
    const PASSWORD_FALSE = "MyPasswd00#+1"
    const DOC_ID = "f_sec_incorrect_password.bas testdocument"

    printlog( "Build a valid filename with full path and suffix" )
    filepath = hGetWorkPath() & "password" & hGetSuffix( filetypeID )
    printlog( "The workfile is: " & filepath )
    
    printlog( "Delete the workfile - if it was left over by a former testrun" )
    hDeleteFile( filepath )

    printlog( "Make sure we have exactly one single document open" )
    hFileCloseAll()
    hCreateDocument() ' Do not use hInitSingleDoc() here as it creates a Writer document

    printlog( "Change the document" )
    hChangeDoc()

    printlog( "Click on FileSave" )
    hUseAsyncSlot( "FileSave" )

    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 1 ) ) then
        printlog( "Automatic filename extension" )
        AutomatischeDateinamenserweiterung.check()

        printlog( "Check the password checkbox" )
        Passwort.check()

        printlog( "Name the file as " & filepath )
        Dateiname.settext( filepath )

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

        if ( hSecurityEnterPasswordOnSave( PASSWORD_VALID ) ) then

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
            if ( not hDestroyDocument() ) then
                warnlog( "Something went wrong while closing the document. please check" )
            endif

        else
            warnlog( "Something went wrong setting the password for the docuemnt" )
        endif

        printlog( "Load the file again" )
        hFileOpen( filepath )
        if ( not hSecurityEnterPasswordOnLoad( PASSWORD_FALSE , false ) ) then
            warnlog( "Something went wrong while using the password dialog" )
        endif

        hDestroyDocument()

        printlog( "Verify that we have no open documents" )
        if ( getDocumentCount <> 0 ) then
            warnlog( "No documents should be open at this point" )
        endif

        printlog( "Delete the workfile" )
        hDeleteFile( filepath )
    else
        warnlog( "Unable to open file save dialog" )
    endif

endcase

