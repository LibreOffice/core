'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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
