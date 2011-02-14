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
'*  short description : Load documents containing hidden BASIC scripts
'*
'\******************************************************************************

testcase tSecurityEvilMacro1()

    
    dim acFile( 2 ) as string

        acFile( 1 ) = gTesttoolPath & "framework\optional\input\security\test_macro.html"
        acFile( 1 ) = convertpath( acFile( 1 ) )

        acFile( 2 ) = gTesttoolPath & "framework\optional\input\security\test_macro.odt"
        acFile( 2 ) = convertpath( acFile( 2 ) )

    dim iCurrentFile as integer ' increment variable
    dim cMsg as string          ' string from messagebox
    dim brc as boolean          ' some returnvalue
    


    for iCurrentFile = 1 to ubound( acFile() )

        printlog( "" )
        printlog( "File: " & acFile( iCurrentFile ) )

	FileOpen

	kontext "OeffnenDlg"
	Dateiname.setText( acFile( iCurrentFile ) )

        Oeffnen.click()

        brc = hAllowMacroExecution() 
        if ( not brc ) then
            warnlog( "Macro security dialog not triggered by document with macro" )
        endif


        kontext "Active"
        if ( active.exists( 5 ) ) then

            ' warn if macro runs without security warning
            if ( not brc ) then
                cMsg = active.getText()
                cMsg = hRemoveLineBreaks( cMsg )
                if ( cMsg = "Hello from StarBasic" ) then
                    warnlog( "The macro was executed without asking for permission" )
                else
                    printlog( "Messagebox: " & cMsg )
                endif
            endif

            ' get the string on the messagebox
            cMsg = active.getText()
            cMsg = hRemoveLineBreaks( cMsg )
            if ( cMsg = "Hello from StarBasic!" ) then
                printlog( "Macro was executed: " & cMsg )
            else
                warnlog( "Unknown dialog is open. This is not the macro" )
            endif
            active.ok()
        endif

        call ExitRestartTheOffice()

    next iCurrentFile


endcase

