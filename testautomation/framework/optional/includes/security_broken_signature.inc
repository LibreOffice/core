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

testcase tBrokenSignature()

    printlog( "Load documents containing hidden BASIC scripts" )

    dim aFile as string
        aFile = "framework\optional\input\security\test_macro (signed).odt"
        aFile = convertpath( gTesttoolPath & aFile )

    printlog( "" )
    printlog( "File: " & aFile )

    FileOpen

    kontext "OeffnenDlg"
    Dateiname.setText( aFile )

    Oeffnen.click()
    hAllowMacroExecution()
    
    kontext "Active"
    if ( active.exists( 2 ) ) then
        printlog( active.getText() )
        
        if ( Active.getButtonCount() > 1 ) then
            warnlog( "This is not the expected dialog" )
            printlog( "The warning about the not recognized signature is missing" )
        else
            printlog( "This appears to be the correct dialog" )
        endif
        active.ok()
    else
        warnlog( "A messagebox was expected informing about the invalid signature" )
    endif

    call hCloseDocument()

endcase

