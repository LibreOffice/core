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
'*  short description : save and load passworded files
'*
'\******************************************************************************

testcase tFiledlgPasswords()
    
    dim workfile as string
    dim workpath as string
        workpath = hGetWorkPath()

    dim filetype as string
    dim filetypeID as string
    dim cMsg as string
        
    dim iCurrentDocumentType as integer
    dim iCurrentFileFormat as integer
        

    for iCurrentDocumentType = 1 to 6
    
        printlog( "Document Type: " & hNumericDocType( iCurrentDocumentType ) )
    
        for iCurrentFileFormat = 1 to 2
        
            select case iCurrentFileFormat
            case 1 : filetypeID = "645"
            case 2 : filetypeID = "current"
            end select
            
            hCreateDocument()
            
            kontext "Navigator"
            hCloseDialog( Navigator, "close,optional" )

            call hChangeDoc()

            printlog( "Open file-save dialog" )
            if ( gApplication = "IMPRESS" ) then
                printlog( "   IMPRESS is slower than other apps, delaying for 2 sec." )
                sleep( 2 )
            endif
            
            FileSave

            Kontext "SpeichernDlg"
            if ( SpeichernDlg.exists( 2 ) ) then

                filetype = hGetFilter( filetypeID )
                hSelectUIFilter( filetype )
                
                workfile = "password" & hGetSuffix( filetypeID )
                DateiName.setText( workpath & workfile )
                
                printlog( "Using fileformat: " & filetype )
                printlog( "Using filename..: " & workfile )

                printlog( "Check password" )
                if ( passwort.isEnabled() ) then
                    Passwort.check()
                else
                    warnlog( "#i36663# Password checkbox is disabled" )
                endif

                printlog( "Save the file" )
                Speichern.click()

                kontext "active"
                if ( active.exists( 2 ) ) then
                   printlog( "Handle overwrite warning (if any)" )
                   active.yes()
                endif
                

                Kontext "PasswordFileSave"
                printlog( "Enter the name of the file as password and confirm" )
                if ( passwordFileSave.exists() ) then
                    Password.settext( workfile )
                    PasswordConfirm.settext( workfile )
                    PasswordFileSave.ok()
                else
                    warnlog( "No password dialog is displayed" )
                endif
                
                kontext "AlienWarning"
                if ( AlienWarning.exists( 2 ) ) then
                    if ( FileTypeID <> "current" ) then
                        printlog( "Handle alienwarning (expected)" )
                        AlienWarning.ok()
                    else
                        warnlog( "Handle alienwarning (unexpected)" )
                        AlienWarning.ok()
                     endif         
                endif    

                hDestroyDocument()

                printlog( "Open the file via file-open" )
                FileOpen

                Kontext "OeffnenDlg"
                if ( OeffnenDlg.exists( 2 ) ) then
                    Dateiname.settext( workfile )

                    Oeffnen.click()

                    Kontext "PasswordFileOpen"
                    printlog( "Enter the filename as password" )
                    if ( PasswordFileOpen.exists( 4 ) ) then
                        PasswortName.settext( workfile )
                        PasswordFileOpen.ok()
                    else
                        kontext "Active"
                        if ( Active.exists( 2 ) ) then
                            cMsg = active.getText()
                            cMsg = hRemoveLineBreaks( cMsg )
                            printlog( cMsg )
	                        warnlog( "#i65889# - CWS Warnings01: Cannot read password protected files" )
                            active.ok()
                            goto endsub
                        else
                            warnlog( "No password dialog is displayed" )
                        endif
                    endif

                    printlog( "Verify that the document is loaded" )
                    hFileWait( FALSE )

                    Kontext "Navigator"
                    if ( Navigator.exists( 1 ) ) then
            	        qaerrorlog( "Navigator should not be visible - setting not saved?" )
            	    endif
	            else
	                warnlog( "Second file open failed" )
                endif
            else
                warnlog( "File Open dislog did not come up" )
            endif

            kontext "Navigator"
            hCloseDialog( Navigator, "close,optional" )

            hDestroyDocument()

            hDeleteFile( workpath & workfile )

            kontext "Navigator"
            hCloseDialog( Navigator, "close,optional" )
            
        next iCurrentFileFormat
        
    next iCurrentDocumentType



endcase
