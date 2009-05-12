'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_reserved_names_unix.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:14 $
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
'/************************************************************************
'*
'* owner : joerg.skottke@sun.com
'*
'* short description : check the internal file dialog ( extended tests )
'*
'\******************************************************************************

testcase tSaveReservedNamesUNIX()

    '///<h1>Check if we accidentially disallow some special names on UNIX</h1>
    '///<ul>
    
    if ( gPlatGroup = "w95" ) then
        printlog( "Test not relevant for Unix(like)"
        goto endsub
    endif
    
    dim cStrangeName as string
    dim irc as integer
    dim iCurrentDocType as integer
    
    for iCurrentDocType = 1 to 2 
    
        printlog( "" )
        printlog( "Document Type is: " & hNumericDocType( iCurrentDocType ) )
        printlog( "Names that are reserved on WINDOWS but are OK on UNIX" )
        
        '///+<li>Try &quot;NUL&quot as filename -> should save ok</li>
        irc = hSaveLoadDelSuccess( "NUL" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        '///+<li>Try &quot;CLOCK$&quot as filename -> should save ok</li>
        irc = hSaveLoadDelSuccess( "CLOCK$" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        '///+<li>Try &quot;CON&quot as filename -> should save ok</li>
        irc = hSaveLoadDelSuccess( "CON" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        '///+<li>Try &quot;PRN&quot as filename -> should save ok</li>
        irc = hSaveLoadDelSuccess( "PRN" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        '///+<li>Try &quot;AUX&quot as filename -> should save ok</li>
        irc = hSaveLoadDelSuccess( "AUX" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        '///+<li>Try &quot;LPT1&quot as filename -> should save ok</li>
        irc = hSaveLoadDelSuccess( "LPT1" , true )
        if ( irc <> 0 ) then warnlog( "failed" )

        '///+<li>Try &quot;COM1&quot as filename -> should save ok</li>
        irc = hSaveLoadDelSuccess( "COM1" , true )
        if ( irc <> 0 ) then warnlog( "failed" )
        
    next iCurrentDocType

    '///</ul>    
endcase

