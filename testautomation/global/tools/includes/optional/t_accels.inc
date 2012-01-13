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

*****
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
