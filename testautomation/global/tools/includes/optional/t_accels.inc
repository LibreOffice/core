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
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : handle accelerators
'*
'*******************************************************************************
'**
' #1 hGetAccel    ' function to retrieve a language specific accelerator 
'**
'\******************************************************************************

function hGetAccel( cCommand as string ) as string

    '///<h3>Retrieve a keyboard accelerator for a specific function</h3>
    '///<i>Uses: global/input/accelerators.txt</i><br>
    '///<i>NOTE: Accelerator is language dependent</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the action to be executed (string). Valid options are:</li>
    '///<ul>
    '///+<li>&quot;FileOpen&quot;</li>
    '///+<li>&quot;FileSave&quot;</li>
    '///+<li>&quot;Print&quot;</li>
    '///+<li>&quot;SelectAll&quot;</li>
    '///+<li>&quot;Copy&quot;</li>
    '///+<li>&quot;DocumentConverter_ShowLog&quot;</li>
    '///+<li>&quot;IDE_SWITCH_TAB+&quot;</li>
    '///+<li>&quot;IDE_SWITCH_TAB-&quot;</li>    
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Accelerator (string)</li>
    '///<ul>
    '///+<li>A string ready to use by .typeKeys(...) method</li>
    '///+<li>&quot;Error&quot; if the requested Accelerator is unknown</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    const CFN = "hGetAccel::"
    const DEFAULT_LANGUAGE = "en-us"

    dim cAccel as string
    dim lsAccelerators( 1000 ) as string
    dim cFile as string
        cFile = gTesttoolpath & "global/input/accelerators.txt"
        cFile = convertpath( cFile )
        
    dim cProximityLocale as string
        
    dim iLang as integer

    printlog( CFN & "Enter with option: " & cCommand )
    'printlog( CFN & "Current Language.: <" & gISOLang & ">" )

    '///+<li>Get the section from the accelerators file</li>
    hGetDatafileSection( cFile , lsAccelerators() , cCommand , "" , "" )

    '///+<li>Find the matching string for the current language</li>
    cAccel = hGetValueForKeyAsString( lsAccelerators() , gISOLang )

    '///+<li>In case of a miss we retry with a modified string</li> 
    '///<ul>
    if ( instr( cAccel , "Error" ) <> 0 ) then
    
        iLang = len( gISOLang )
        
        select case iLang
        case 2 :
            '///+<li>Try xx-XX</li>
            cProximityLocale = gISOLang & "-" & ucase( gISOLang )
            printlog( CFN & "Trying alternative locale: " & cProximityLocale )
            cAccel = hGetValueForKeyAsString( lsAccelerators() , cProximityLocale )
        case 5 :
            '///+<li>Try xx</li>
            cProximityLocale = mid( cUpperCaseLocale , 1, 2 )
            printlog( CFN & "Trying alternative locale: " & cProximityLocale )
            cAccel = hGetValueForKeyAsString( lsAccelerators() , cProximityLocale )
        case else :        
            '///+<li>Try en-US</li>
            cProximityLocale = "en-US"
            printlog( CFN & "Trying default locale: " & cProximityLocale )
            cAccel = hGetValueForKeyAsString( lsAccelerators() , DEFAULT_LANGUAGE )
        end select            
            
    endif
    '///</ul>

    '///+<li>Build the complete accelerator-string so it can be used by "TypeKeys"</li>
    '///+<li>Print it to the log and return the string to the calling function</li>
    
    cAccel = "<" & cAccel & ">"
    printlog( CFN & "Requested accelerator: " & cAccel & " for language: " & gISOLang  )
    hGetAccel() = cAccel
    '///</ul>

end function
