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
