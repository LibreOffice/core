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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
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

