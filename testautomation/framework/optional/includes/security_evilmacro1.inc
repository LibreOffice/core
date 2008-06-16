'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: security_evilmacro1.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:15 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Load documents containing hidden BASIC scripts
'*
'\******************************************************************************

testcase tSecurityEvilMacro1()

    '///<h1>Level 1 test: Allow macro execution/Security set to medium</h1>
    '///<ul>
    
    dim acFile( 2 ) as string

        acFile( 1 ) = gTesttoolPath & "framework\optional\input\security\test_macro.html"
        acFile( 1 ) = convertpath( acFile( 1 ) )

        acFile( 2 ) = gTesttoolPath & "framework\optional\input\security\test_macro.odt"
        acFile( 2 ) = convertpath( acFile( 2 ) )

    dim iCurrentFile as integer ' increment variable
    dim cMsg as string          ' string from messagebox
    dim brc as boolean          ' some returnvalue
    

    '///+<li>Do following for each document to be tested:</li>
    '///<ul>

    for iCurrentFile = 1 to ubound( acFile() )

        printlog( "" )
        printlog( "File: " & acFile( iCurrentFile ) )

        '///+<li>Click FileOpen or go to the dialog via menu</li>
	FileOpen

        '///+<li>Enter the name of the file</li>
	kontext "OeffnenDlg"
	Dateiname.setText( acFile( iCurrentFile ) )

        '///+<li>Click &quot;Open&quot;</li>
        Oeffnen.click()

        '///+<li>Verify that the security warning comes up, confirm to run macro.<br>
        '///+ There should be no warning for security levels other than medium!</li>
        brc = hAllowMacroExecution() 
        if ( not brc ) then
            warnlog( "Macro security dialog not triggered by document with macro" )
        endif

        '///+<li>Wait a moment for the document to get loaded</li>

        '///+<li>Check for a messagebox to come up on load. This might only<br>
        '///+ after a security warning has been displayed!</li>
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

        '///+<li>Close the document</li>
        call ExitRestartTheOffice()

    next iCurrentFile

    '///</ul>
    '///</ul>

endcase

