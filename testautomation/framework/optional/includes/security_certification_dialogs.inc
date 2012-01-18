'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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

