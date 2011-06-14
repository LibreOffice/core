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
'*  short description : Tools to ease working with language dependent strings/values
'*
'\******************************************************************************

function hTestLocale() as boolean


    '///<h3>Test whether we are running on an utf8 locale (Linux only)</h3><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>
    '///+<li>No input parameters</li>
    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE: Yes, this is a UTF-8 locale</li>
    '///+<li>FALSE: Anything else</li>
    '///</ul>
    '///</ol>

    const CFN = "hTestLocale::"
    dim brc as boolean 'a multi purpose boolean returnvalue
    dim irc as integer
        irc = 0
    
    dim lc_all as string
    dim lang as string

    '///<u>Description:</u>
    '///<ul>
    
    if ( gtSysName = "Linux" ) then
    
        '///+<li>Retrieve LANG</li>
        lang = environ( "LANG" )
        lang = ucase( lang )
        if ( instr( lang , "UTF8" ) or instr( lang , "UTF-8" ) ) then
            irc = 1
        endif
    
        '///+<li>Retrieve LC_ALL (Note that this variable is mostly set but has no value assigned)</li>
        lc_all = environ( "LC_ALL" )
        lc_all = ucase( lc_all )
        if ( instr( lc_all , "UTF8" ) or instr( lc_all , "UTF-8" ) ) then
            irc = irc + 1
        else
            if ( lc_all = "" ) then
                printlog( CFN & "No value set for LC_ALL, assuming UTF-8" )
                irc = irc + 1
            else
                printlog( CFN & "LC_ALL is set but has no UTF-8 locale" )
            endif
        endif

    else
	
	irc = 2 ' this is Windows and Solaris - they always can handle weird content
        
    endif
    
    if ( irc = 2 ) then
        printlog( CFN & "UTF-8 locale found" )
        hTestLocale() = TRUE
    else
        printlog( CFN & "Please verify that you are running on UTF-8 locale" )
        hTestLocale() = FALSE
    endif
        
    '///</ul>


end function
