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
'*  short description : Test the extension locator service
'*
'\******************************************************************************

testcase tExtensionLocation

    printlog( "Test the extension locator service" )

     ' The location of the sample extension and helper document
    dim cWorkPath as string
        cWorkPath = gTesttoolPath & "extensions\optional\input\path_to_extension\"
        cWorkPath = convertpath( cWorkPath )

    const EXTENSION_NAME = "locationtest.oxt"
    const DOCUMENT_NAME  = "LocationTest.odt"
    const MACRO_NAME     = "tExtensionLocation"

    dim brc as boolean
    dim irc as integer
    dim cMsg as string

    printlog( "Using extension: " & cWorkPath & EXTENSION_NAME )
    printlog( "Using document.: " & cWorkPath & DOCUMENT_NAME )

    brc = hFileOpen( cWorkPath & DOCUMENT_NAME )
    if ( not brc ) then
        brc = hAllowMacroExecution()
        if ( not brc ) then
            warnlog( "This is not the macro execution warning" )
        endif
    else
        warnlog( "Macro execution warning is missing" )
    endif

    hMacroOrganizerRunMacro( MACRO_NAME )

    kontext "Makro"
    if ( Makro.exists( 1 ) ) then
        warnlog( "Macro was not executed")
        Makro.close()
        goto endsub
    else
        printlog( "Macro was executed")
    endif

    kontext "active"
    if ( active.exists( 2 ) ) then
        cMsg = active.getText()
        if ( cMsg <> "" ) then
            warnlog( "The messagebox is not empty: " & cMsg )
        else
            printlog( "Messagebox is empty, good." )
        endif
        active.ok()
    else
        warnlog( "No messagebox displayed, please check that the macro is executed" )
    endif

    hExtensionAddGUI( cWorkPath & EXTENSION_NAME , "verbose,InstallForUser" )
    hMacroOrganizerRunMacro( MACRO_NAME )

    kontext "active"
    if ( active.exists( 2 ) ) then
        cMsg = active.getText()

        if ( cMsg = "" ) then
            warnlog( "The messagebox is empty, please check." )
        else

            printlog( "Content of messagebox: " & cMsg )

            if ( instr( cMsg , "file:///" ) > 0 ) then
                printlog( "Found File-URL" )
            else
                warnlog( "This does not appear to be a File-URL, please check!" )
            endif

            if ( instr( cMsg , EXTENSION_NAME ) > 0 ) then
                printlog( "Extension string is present" )
            else
                warnlog( "Extension name seems to be missing" )
            endif

            if ( dir( cMsg ) = "" ) then
                warnlog( "The extension could not be found at the expected location" )
            else
                printlog( "Extension found. Good." )
            endif

        endif
        active.ok()
    else
        warnlog( "No messagebox displayed, please check that the macro is executed" )
    endif

    irc = hExtensionRemoveGUI( EXTENSION_NAME )
    if ( irc <> 0 ) then
        warnlog( "the extension was NOT cleanly removed" )
    endif

    hDestroyDocument()

endcase

