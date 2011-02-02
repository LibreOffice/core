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
'*  short description : Walk through a number of dialogs related to digital signatures
'*
'\******************************************************************************

testcase tCertificationDialogs
    dim brc as boolean
    const CFN = "tCertificationDialogs::"
    const SVERSION = "680"

    dim sFile as string
        sFile = gOfficePath & "user\work\certdlg" & hGetSuffix( SVERSION )
        sFile = convertpath( sFile )

    hDeleteFile( sFile )

    hNewDocument() 

    hChangeDoc()

    hOpenDigitalSignaturesDialog()

    kontext "active"
    if ( active.exists( 1 ) ) then
        printlog( "MSGBOX: " & active.getText() )
        try
           active.yes()
        catch
            warnlog( CFN & "This is not the expected messagebox, aborting test" )
            active.ok()
            hCloseDocument()
            goto endsub
        endcatch
        printlog( CFN & "Allow to save the document" )
    else
        if ( gApplication <> "MASTERDOCUMENT" ) then
            warnlog( CFN & "No message that the file must be saved to be signed" )
        else
            printlog( CFN & "OK - No save-message for Masterdoc." )
        endif
    endif

    kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 1 ) ) then
        DateiName.setText( sFile )
        if AutomatischeDateinamenserweiterung.Exists then
            AutomatischeDateinamenserweiterung.Uncheck
        endif
        Speichern.click()
    else
        if ( gApplication <> "MASTERDOCUMENT" ) then
            warnlog( CFN & "Save As Dialog is not open" )
        else
            printlog( CFN & "OK - No filesave for Masterdoc." )
        endif
    endif

    kontext "DigitalSignature"
    if ( DigitalSignature.exists( 2 ) ) then
        printlog( CFN & "Digital signatures is open" )
        DigitalSignature.close()
    else
        if ( gApplication <> "MASTERDOCUMENT" ) then
            warnlog( CFN & "Digital Signatures Dialog is not open" )
        else
            printlog( CFN & "OK - No signature dialog for Masterdoc." )
        endif
    endif

    call hCloseDocument()
endcase

