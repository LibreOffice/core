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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : check the internal file dialog ( extended tests )
'*
'\******************************************************************************

testcase tSaveReservedNamesUNIX()

    printlog( "Names that are reserved on WINDOWS but are OK on UNIX" ) 
    
    if ( gPlatGroup = "w95" ) then
        printlog( "Test not relevant for Windows/DOS family of OSes" )
        goto endsub
    endif
    
    dim cStrangeName as string
    dim irc as integer
    dim iCurrentDocType as integer
    
    for iCurrentDocType = 1 to 2 
    
        printlog( "" )
        printlog( "Document Type is: " & hNumericDocType( iCurrentDocType ) )
        
        irc = hSaveLoadDelSuccess( "NUL" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        irc = hSaveLoadDelSuccess( "CLOCK$" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        irc = hSaveLoadDelSuccess( "CON" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        irc = hSaveLoadDelSuccess( "PRN" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        irc = hSaveLoadDelSuccess( "AUX" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        irc = hSaveLoadDelSuccess( "LPT1" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        irc = hSaveLoadDelSuccess( "COM1" , true )
        if ( irc <> 0 ) then warnlog( "failed" )
        
    next iCurrentDocType

endcase

