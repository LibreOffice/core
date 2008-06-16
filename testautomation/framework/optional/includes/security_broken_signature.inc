'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: security_broken_signature.inc,v $
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

testcase tBrokenSignature()

    '///<h1>Allow macro execution/Security set to medium</h1>
    '///<ul>

    dim acFile( 1 ) as string

        acFile( 1 ) = "framework\optional\input\security\test_macro (signed).odt"
        acFile( 1 ) = gTesttoolPath & acFile( 1 )
        acFile( 1 ) = convertpath( acFile( 1 ) )

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
        
        '///+<li>Test for Broken Signature Messagebox:</li>
        '///<ul>
        Kontext "Active"
        if ( active.exists( 3 ) ) then
            cMsg = active.getText()
            cMsg = hRemoveLineBreaks( cMsg )
            
            '///+<li>Make sure that we do not have the macro</li>
            if ( cMsg = "Hello from StarBasic!" ) then
                warnlog( "Macro was executed without any warning" )            
            else
                '///+<li>Guess the dialog by Ressource Type</li>
                if ( active.getRT() = 304 ) then
                    printlog( "Broken signature message: " & cMsg )
                else
                    warnlog( "Unknown messagebox: " & cMsg )
                endif
            endif
            
            '///+<li>Close dialog with OK</li>
            active.ok()
            
        endif
        '///</ul>
        
        '///+<li>Test for the macro again</li>        
        kontext "Active"
        if ( active.exists() ) then
            cMsg = active.getText()
            cMsg = hRemoveLineBreaks( cMsg )
            warnlog( "Unexpected messagebox: " & cMsg )
            active.ok()
        endif

        '///+<li>Close the document</li>
        call hCloseDocument()

    next iCurrentFile

    '///</ul>
    '///</ul>

endcase

