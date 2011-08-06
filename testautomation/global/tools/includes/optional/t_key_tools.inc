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
'* short description : tools for evaluation of key=value datalists
'*
'*******************************************************************************
' **
' #1 hGetDataPairAsString     ' retrieve key=value from a list
' #1 hGetValueForPairAsString ' retrieve value from a key=value as string
' #1 hGetValueForPairAsLong   ' retrieve value from a key=value as long integer
' #1 hGetValueForKeyAsInt     ' retrieve value for a specified key as integer
' #1 hGetValueForKeyAsLong    ' retrieve value for a specified key as long integer
' #1 hGetKeyForPairAsString   ' retrieve key from key=value as string
' #1 hGetKeyForValueAsString  ' retrieve key for a specified value as string
' #1 hGetKeyForValueAsInt     ' retrieve key for a specified value as int
' #1 hGetIndexForKeyAsInt     ' retrieve the index for a key in a list
' #1 KeyCompare               ' compare two keys
' **
'\******************************************************************************

' Some variables explained:
'
' cComp is the KEY belonging to the VALUE above for reverse test
'
' NOTE: Other related functions can be found in global/tools/inc/t_filters.inc

'*******************************************************************************

function hGetDataPairAsString( sKey as string, sVal as long ) as string
    
    '///<h3>Concatenate two strings and insert an = sign</h3>
    '///<i>About &quot;KEY&quot;-functions: <br>
    '///These functions are used whenever
    '///+ data is to be processed that is stored in a key=value format.<br>
    '///+ They handle a single string or a list of strings
    '///+ to e.g. isolate a key, a value or to just retrieve the pair.</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    
    '///+<li>KEY (string)</li>
    '///<ul>
    '///+<li>Anything but should not contain # or '</li>
    '///</ul>
    
    '///+<li>VALUE (string)</li>
    '///<ul>
    '///+<li>Anything but should not contain # or '</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>KEY=VALUE pair (string)</li>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    '///+<li>Returns string1=string2</li>
    hGetDataPairAsString() = sKey & "=" & sVal
    
    '///</ul>
    
end function

'*******************************************************************************

function hGetValueForPairAsString( cLine as string ) as string
    
    '///<h3>Retrieve value from a key=value string</h3>
    '///<i>About &quot;KEY&quot;-functions: <br>
    '///These functions are used whenever
    '///+ data is to be processed that is stored in a key=value format.<br>
    '///+ They handle a single string or a list of strings
    '///+ to e.g. isolate a key, a value or to just retrieve the pair.</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    
    '///+<li>KEY=VALUE pair (string)</li>
    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>VALUE (string)</li>
    '///<ul>
    '///+<li>Anything after the = character</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    dim iCharPos as integer
    
    '///+<li>Locate the equal sign</li>
    iCharPos = instr( cLine , "=" )
    iCharPos = len( cLine ) - iCharPos
    
    '///+<li>Isolate everything to the right of the equal sign</li>
    '///+<li>Return the value as string</li>
    hGetValueForPairAsString() = right( cLine , iCharPos )
    
    '///</ul>
    
end function

'*******************************************************************************

function hGetValueForPairAsLong( cLine as string ) as long
    
    '///<h3>Retrieve the value from a key=value pair as long</h3>
    '///<i>About &quot;KEY&quot;-functions: <br>
    '///These functions are used whenever
    '///+ data is to be processed that is stored in a key=value format.<br>
    '///+ They handle a single string or a list of strings
    '///+ to e.g. isolate a key, a value or to just retrieve the pair.</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    
    '///+<li>KEY=VALUE pair (string)</li>
    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>VALUE (long)</li>
    '///<ul>
    '///+<li>Anything after the = character as long integer value</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    dim iCharPos as integer
    
    '///+<li>Locate the equal sign</li>
    iCharPos = instr( cLine , "=" )
    iCharPos = len( cLine ) - iCharPos
    
    '///+<li>Isolate and return the Value as long datatype</li>
    hGetValueForPairAsLong = val( right( cLine , iCharPos ))
    
    '///</ul>
    
end function

'*******************************************************************************

function hGetValueForKeyAsInt( lsList() as string, sKey as string ) as integer
    
    '///<h3>Retrieve the value of a key=value pair as integer</h3>
    '///<i>About &quot;KEY&quot;-functions: <br>
    '///These functions are used whenever
    '///+ data is to be processed that is stored in a key=value format.<br>
    '///+ They handle a single string or a list of strings
    '///+ to e.g. isolate a key, a value or to just retrieve the pair.</i><br><br>
    '///<i>Prerequisite: Array compatible to listfuncs, known, valid key</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    
    '///+<li>List of KEY=VALUE pairs (string)</li>
    '///<ul>
    '///+<li>List must be compatible to listfunctions (see t_listfuncs.inc)</li>
    '///</ul>
    '///+<li>Key to be searched for within the list (string)</li>
    '///<ul>
    '///+<li>Key must be valid</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>VALUE (integer)</li>
    '///<ul>
    '///+<li>The value for a given key</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    dim iItem as integer
    dim cString as string
    dim cComp as string
    
    hGetValueForKeyAsInt() = 0
    
    ' Scan through the list and look for sKey. If found, return the Value
    ' (everything to the right of the '=')
    '///+<li>Search the list for the key</li>
    for iItem = 1 to listcount( lsList() )
        if( instr( lsList( iItem ) , sKey ) <> 0 ) then
            
            '///+<li>Verify that this is really the correct key by reverse-checking</li>
            cComp = hGetKeyforPairAsString( lsList( iItem ) )
            
            '///+<li>Retrieve the value for the key as string</li>
            if( sKey = cComp ) then
                cString = hGetValueForPairAsString( lsList( iItem ) )
                iItem = listcount( lsList() ) + 1
            endif
            
        endif
    next iItem
    
    '///+<li>Convert the stringvalue to integer and return it</li>
    hGetValueForKeyAsInt() = val( cString )
    
    '///</ul>
    
end function

'*******************************************************************************

function hGetValueForKeyAsLong( lsList() as string, sKey as string ) as long
    
    '///<h3>Retrieve the value of a key=value pair as integer</h3>
    '///<i>About &quot;KEY&quot;-functions: <br>
    '///These functions are used whenever
    '///+ data is to be processed that is stored in a key=value format.<br>
    '///+ They handle a single string or a list of strings
    '///+ to e.g. isolate a key, a value or to just retrieve the pair.</i><br><br>
    '///<i>Prerequisite: Array compatible to listfuncs, known, valid key</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    
    '///+<li>List of KEY=VALUE pairs (string)</li>
    '///<ul>
    '///+<li>List must be compatible to listfunctions (see t_listfuncs.inc)</li>
    '///</ul>
    '///+<li>Key to be searched for within the list (string)</li>
    '///<ul>
    '///+<li>Key must be valid</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>VALUE (long integer)</li>
    '///<ul>
    '///+<li>The value for a given key</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    dim iItem as integer
    dim cString as string
    dim cComp as string
    
    hGetValueForKeyAsInt() = 0
    
    ' Scan through the list and look for sKey. If found, return the Value
    ' (everything to the right of the '=')
    '///+<li>Search the list for the key</li>
    for iItem = 1 to listcount( lsList() )
        if( instr( lsList( iItem ) , sKey ) <> 0 ) then
            
            '///+<li>Verify that this is really the correct key by reverse-checking</li>
            cComp = hGetKeyforPairAsString( lsList( iItem ) )
            
            '///+<li>Retrieve the value for the key as string</li>
            if( sKey = cComp ) then
                cString = hGetValueForPairAsString( lsList( iItem ) )
                iItem = listcount( lsList() ) + 1
            endif
            
        endif
    next iItem
    
    '///+<li>Convert the stringvalue to long integer and return it</li>
    hGetValueForKeyAsInt() = val( cString )
    
    '///</ul>
    
end function

'*******************************************************************************

function hGetKeyForPairAsString( cLine as string ) as string
    
    '///<h3>Retrieve the KEY from a key=value pair</h3>
    '///<i>About &quot;KEY&quot;-functions: <br>
    '///These functions are used whenever
    '///+ data is to be processed that is stored in a key=value format.<br>
    '///+ They handle a single string or a list of strings
    '///+ to e.g. isolate a key, a value or to just retrieve the pair.</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    
    '///+<li>KEY=VALUE pair (string)</li>
    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>KEY (string)</li>
    '///<ul>
    '///+<li>Anything before the = character</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    dim iCharPos as integer
    
    iCharPos = instr( cLine , "=" )
    
    '///+<li>get the string to the left of the = char</li>
    if ( iCharPos > 0 ) then
        hGetKeyForPairAsString() = left( cLine , iCharPos -1 )
    else
        warnlog( "Invalid string passed to hGetKeyForPairAsString" )
        printlog( "It was: " & cLine )
    endif
    '///+<li>Return the key</li>
    '///</ul>
    
end function

'*******************************************************************************

function hGetKeyForValueAsString( lsList() as string, sVal as string ) as string
    
    '///<h3>Retrieve the KEY for a known VALUE from a key=value pair</h3>
    '///<i>About &quot;KEY&quot;-functions: <br>
    '///These functions are used whenever
    '///+ data is to be processed that is stored in a key=value format.<br>
    '///+ They handle a single string or a list of strings
    '///+ to e.g. isolate a key, a value or to just retrieve the pair.</i><br><br>
    '///<i>Prerequisite: List compatible to listfuncs, known VALUE for pair</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    
    '///+<li>List of KEY=VALUE pairs (string)</li>
    '///<ul>
    '///+<li></li>
    '///</ul>
    
    '///+<li>VALUE (string)</li>
    '///<ul>
    '///+<li>Absolute match required.</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>KEY (string)</li>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    ' This function returns the value of a key as string.
    ' The form of the input strings is 'key=value', the list is parsed
    ' The key for the first occurrence of sVal is returned
    
    dim iCurrentValue as integer
    dim iListItems as integer
    iListItems = listcount( lsList() )
    dim cLine as string
    
    ' preset a default return string
    hGetKeyForValueAsString() = "Error: No matching KEY found for VALUE"
    
    '///+<li>scan through a list to find an entry that matches sKey. Return the Key.</li>
    for iCurrentValue = 1 to iListItems
        
        cLine = lsList( iCurrentValue )
        
        if( instr( cLine , sVal ) <> 0 ) then
            hGetKeyForValueAsString() = hGetKeyForPairAsString( cLine )
            iCurrentValue = iListItems + 1
        end if
        
    next iCurrentValue
    '///+<li>Return the key from the first matching value</li>
    '///</ul>
    
end function

'*******************************************************************************

function hGetKeyForValueAsInt( lsList() as string, sVal as string ) as integer
    
    '///<h3>Retrieve the KEY for a known VALUE from a key=value pair</h3>
    '///<i>About &quot;KEY&quot;-functions: <br>
    '///These functions are used whenever
    '///+ data is to be processed that is stored in a key=value format.<br>
    '///+ They handle a single string or a list of strings
    '///+ to e.g. isolate a key, a value or to just retrieve the pair.</i><br><br>
    '///<i>Prerequisite: List compatible to listfuncs, known VALUE for pair</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    
    '///+<li>List of KEY=VALUE pairs (string)</li>
    
    '///+<li>VALUE (string)</li>
    '///<ul>
    '///+<li>Absolute match required.</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>KEY (integer)</li>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    '///+<li>Use hgetKeyForValueAsString, convert result to integer, return it.</li>
    hGetKeyForValueAsInt() = val( hGetKeyForValueAsString( lsList() , sVal ) )
    '///</ul>
    
end function

'*******************************************************************************

function hGetIndexForKeyAsInt( lsList() as string, sKey as string ) as integer
    
    '///<h3>Search a list of key=value pairs for a key and return its index</h3>
    '///<i>About &quot;KEY&quot;-functions: <br>
    '///These functions are used whenever
    '///+ data is to be processed that is stored in a key=value format.<br>
    '///+ They handle a single string or a list of strings
    '///+ to e.g. isolate a key, a value or to just retrieve the pair.</i><br><br>
    '///<i>Prerequisite: List compatible to listfuncs, known VALUE for pair</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    
    '///+<li>List of KEY=VALUE pairs (string)</li>
    
    '///+<li>KEY to search for (string)</li>
    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>Index of the KEY in the list (integer)</li>
    '///<ul>
    '///+<li>0 if not found</li>
    '///+<li>Any number &gt; 0 and &le; listcount( List() )</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    dim iItem as integer
    dim cComp as string
    
    hGetIndexForNamedKeyAsInt() = 0
    
    ' Scan through the list and look for sKey. If found, return the Value
    ' (everything to the right of the '=')
    '///+<li>Scan through the list for the KEY</li>
    for iItem = 1 to listcount( lsList() )
        if( instr( lsList( iItem ) , sKey ) <> 0 ) then
            
            '///+<li>if found, perform reverse checking</li>
            cComp = hGetKeyforValueAsString( lsList( iItem ) )
            
            '///+<li>get the index of the item</li>
            if( sKey = cComp ) then
                hGetIndexForKeyAsInt() = iItem
                iItem = listcount( lsList() ) + 1
            endif
            
        endif
    next iItem
    '///+<li>Return the index</li>
    '///</ul>
    
end function

'*******************************************************************************

function keycompare( found as string, expected as string, sKey as string, optional cBugID as string) as boolean
    
    '///<h3>Function to compare two keys with each other</h3>
    '///<u>Deprecated, do not use</u>
    
    if ( isMissing( cBugID ) ) then
        cBugID = ""
    endif
    
    if ( lcase(found) = lcase(expected) ) then
        printlog( "     * '" & sKey & "': ok" )
        keycompare = true
    else
        warnlog( "#" + cBugID + "# Control has incorrect value: '" & sKey & "'")
        printlog( " > Found...: '" & found  & "'" )
        printlog( " > Expected: '" & expected & "'" )
        keycompare = false
    endif
    
end function



