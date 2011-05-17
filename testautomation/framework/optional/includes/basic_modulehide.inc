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
'*  short description : Verify that hiding modules works
'*
'\******************************************************************************

testcase tBasicIdeModuleHide

    
    const CFN = "tBasicIdeModuleHide::"

    const TESTMACRO = "TTMacro1"
    const ERR_NO_LINES_INSERTED = 0

    const RC_SUCCESS = 0
    const RC_TIMEOUT = -1

    const TAB_MODULES = 1

    dim rc as integer
    dim brc as boolean
    dim cDefaultTabName as string

    gApplication = "WRITER"
    call hNewDocument()
    
    ToolsMacro_uno
    hCreateModuleForDoc()
    
    printlog( "Insert macro" )
    if ( hInsertMacroFromFile( TESTMACRO ) > ERR_NO_LINES_INSERTED ) then
       printlog( CFN & "Macro has been written successfully" )
    else
       warnlog( CFN & "Failed to insert macro" )
    endif

    printlog( "Hide the module" )
    if ( hHideModule() ) then 

        ' hTestMacro is expected to fail, so we jump to the catch statement
        if ( hInsertMacroFromFile( TESTMACRO ) <> ERR_NO_LINES_INSERTED ) then
            warnlog( "For some reason the original module is still visible" )
        else
            printlog( "The edit window is not accessible, good" )
        endif

        if ( hOpenBasicObjectOrganizer( TAB_MODULES ) ) then

            modulliste.typekeys( "<END><RIGHT><DOWN><RIGHT><DOWN>" )
            if ( hClickButton( Bearbeiten ) = RC_TIMEOUT ) then
                warnlog( "#i35097# Crash when editing last module" )
                goto endsub
            endif

            if ( not hBasicIDERunMacro( TESTMACRO ) ) then
                warnlog( "Incorrect macro executed/macro missing" )
            endif

            hCloseBasicIDE()
            hFileCloseAll()

        else
            warnlog( "restarting the office to recover from errors" )
            call exitRestartTheOffice()
        endif

    else
        warnlog( "Some unexpected error occurred while trying to hide the module" )
    endif
    
endcase
