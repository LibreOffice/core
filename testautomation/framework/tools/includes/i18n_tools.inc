'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Tools to ease working with language dependent strings/values
'*
'\******************************************************************************

function hGetI18nData( cSection as string, cLanguage as string ) as string

    '///<h3>Retrieve various information about i18n</h3>
    '///<i>Uses datafile: framework/tools/input/i18ndata.txt</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Section from which to retrieve the data (string)</li>
    '///<ul>
    '///+<li>Any name of a section existing in the datafile</li>
    '///</ul>
    '///+<li>Language code as string</li>
    '///<ul>
    '///+<li>Use hGetTwoDigitLangCode(...) to ensure proper string formatting</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Language identifier (string)</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    '///+<li>Create the path to the datafile</li>
    dim cPath as string
        cPath = gTesttoolPath & "framework\tools\input\i18ndata.txt"
        cPath = convertpath( cPath )
        
    '///+<li>Find out the required size of the array to hold the entire file</li>
    dim iFileSize as integer
        iFileSize = hListFileGetSize( cPath )
        
    '///+<li>Define an array to hold the datafile</li>
    dim aFileContent( iFileSize ) as string
    
    '///+<li>Retrieve the requested section from the datafile</li>
    hGetDatafileSection( cPath, aFileContent(), cSection, "", "" )
    
    '///+<li>Isolate the requested language item</li>
    hGetI18nData() = hGetValueForKeyAsString( aFileContent(), cLanguage )
    
    '///</ul>
    
end function

'*******************************************************************************

function hGetTwoDigitLangCode( iLanguage as integer ) as string

    '///<h3>Retrieve a two digit language code from integer</h3>
    '///<i>Replaces and enhances deprecated sub &quot;siSpracheSetzen&quot;</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Language Code (integer)</li>
    '///<ul>
    '///+<li>Any number between (and including) 1 and 99</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Language Code (string)</li>
    '///<ul>
    '///+<li>1 - 9 -&gt; &quot;01&quot; - &quot;09&quot;</li>
    '///+<li>10 - 99 -&gt; &quot;10&quot; - &quot;99&quot;</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    dim cLanguage as string
    
    '///+<li>Convert single digit language code to two digit language string</li>
    if ( ( iLanguage > 0 ) and ( iLanguage < 10 ) ) then
        cLanguage = "0" & iLanguage
    else
        cLanguage = iLanguage
    endif
    
    hGetTwoDigitLangCode() = cLanguage
    '///</ul>

end function


'*******************************************************************************

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
