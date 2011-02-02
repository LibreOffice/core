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
'*  short description : Execute macros from trusted path
'*
'\******************************************************************************

testcase tSecTrustedPath( cWorkFile as string, iSecLevel as integer )


    dim irc as integer
    dim cMsg as string

    cWorkFile = convertToUrl( convertpath( cWorkFile ) )
    FileOpen( "URL", cWorkFile, "FrameName", "_default"
    irc = hFileWait( false )
    
    ' We have quite a bunch of possible combinations here. Some trigger a security warning
    ' others do not. Files within a trusted path should never prompt.
    
    '         |---------------------------------------------------------|
    '         | Trusted |     Security Level / Warning displayed        |
    '         |  Path   |   Low     |  Medium   |   High    | Very High |
    '         |---------------------------------------------------------|
    '         |  Yes    |     No    |    No     |     No    |    No     |
    '         |---------------------------------------------------------|
    '         |   No*)  |     No    |    Yes    |  Blocked  |  Blocked  |
    '         |---------------------------------------------------------|
    ' *) Covered by test "f_sec_macrosecurity.bas"
       

    if ( irc = -2 ) then
        kontext "SecurityWarning"
        if ( SecurityWarning.exists() ) then
            warnlog( "Macro Security Warning was not expected at this point" )
            SecurityWarning.ok()
        endif
    endif

    Sleep (1)
    printlog( "Verify that the macro is executed" )
    kontext "Active"
    if ( active.exists( 2 ) ) then
        cMsg = active.getText()
        cMsg = hRemoveLineBreaks( cMsg )
        if ( cMsg = "TTMacro3" ) then
            printlog( "Macro was executed" )
        else
            warnlog( "Invalid messagebox is open, please check" )
        endif

        active.ok()

    endif

    hDestroyDocument()



endcase

