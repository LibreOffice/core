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
'*  short description : Password settings in Tools/Options
'*
'\******************************************************************************

testcase tRecommendPassword( filetypeID as string )


    dim workfile as string
        workfile = "password" & hGetSuffix( filetypeID )
        printlog( " * Name of workfile: " & workfile )
                                                                                                                               
    dim workpath as string
        workpath = convertpath( gOfficePath & "user\work\" )
        printlog( " * Save file to....: " & workpath )

    dim rc as integer

    printlog( "Open a new document" )
    call hNewDocument()

    printlog( "Change the document" )
    call hChangeDoc()

    printlog( "Open filesave" )
    FileSave
    kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then
	printlog( "File Open dialog is open. Good" )
    else
	warnlog( "File Open dialog is not open." )
        goto endsub
    endif

    printlog( "Verify that the password-checkbox is enabled" )
    if ( passwort.isenabled() ) then
        printlog( " * Password checkbox is enabled. Good." )

        if ( passwort.ischecked() ) then
            printlog( " * Password checkbox is checked. Good." )
            rc = 0
        else
            warnlog( "Password checkbox is not checked, the option " & _
                     "has not been transported from tools/options " & _
                     "to the dialog -> bug" )
            rc = 1
        endif
    else
        warnlog( "#i36663# Password checkbox is disabled" )
        rc = 2
    endif

    if ( rc = 0 ) then

        AutomatischeDateinamenserweiterung.Uncheck
                                                                                                                              
        printlog( "Name the file" )
        Dateiname.settext( workpath & workfile )
  
        printlog( "Uncheck password-protection" )
        passwort.uncheck()

        printlog( "Save the file" )
        Speichern.click()
                       
        Kontext "active"
        if ( active.exists( 2 ) ) then
            printlog( " - handle overwrite warning (if any)" )
            active.yes()
        endif
  
        Kontext "passwordDlg"
        if ( passwordDlg.exists( 2 ) ) then
            warnlog( "Password dialog comes up, this is not expected at this " & _
                     "point. Trying to handle the error gracefully" )
            Password.settext( workfile )
            PasswordConfirm.settext( workfile )
            PasswordDlg.ok()
        else
            printlog( " * No password dialog is displayed. Good." )
        endif

        hDeleteFile( workpath & workfile )

    endif

    printlog( "Close the current document" )
    call hCloseDocument()

endcase
