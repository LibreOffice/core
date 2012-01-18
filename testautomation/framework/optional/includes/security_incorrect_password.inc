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

