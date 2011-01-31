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
'*  short description : Functions for manipulation of strings
'*
'\******************************************************************************

function hRemoveLineBreaks( cString as string ) as string

    dim myString as string : myString = cString
    myString = hStringReplaceChar( myString, CHR$(09), " " )
    myString = hStringReplaceChar( myString, CHR$(13), " " )
    myString = hStringReplaceChar( myString, CHR$(10), ""  )
    hRemoveLineBreaks() = myString

end function

'*******************************************************************************

function hCompareSubStrings( cRef as string, cSub as string ) as integer

    '///<h3>Find substring in a reference string</h3>
    '///<i>Used to determine that we are on &quot;The first doc!&quot;</i><br><br>
    '///<u>Parameter(s):</u>
    '///<ol>
    '///+<li>Term to search for (string)</li>
    '///+<li>Term to be searched (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcode (integer)</li>
    '///<ul>
    '///+<li>-1: Invalid parameter(s)</li>
    '///+<li>0: Strings do not match</li>
    '///+<li>1: Term is exact match</li>
    '///+<li>2: Term is a substring</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hCompareSubStrings::"
    const RETVAL_INVALID_PARAMETER = -1
    const RETVAL_MATCH_NONE        =  0
    const RETVAL_MATCH_EXACT       =  1
    const RETVAL_MATCH_SUBSTRING   =  2

    '///+<li>Test function parameters</li>
    if ( ( cRef = "" ) or ( cSub = "" ) ) then
        warnlog( CFN & "invalid parameter(s): Empty string passed." )
        hCompareSubStrings() = RETVAL_INVALID_PARAMETER
        exit function
    endif
    
    dim irc as integer

    '///+<li>Test if we have a substring</li>
    if ( instr( cRef, cSub ) ) then
        irc = RETVAL_MATCH_SUBSTRING
    else
        irc = RETVAL_MATCH_NONE
    endif

    '///+<li>Test if we have an exact match</li>
    if ( irc = RETVAL_MATCH_SUBSTRING ) then
        if ( ( cRef = cSub ) and ( len( cRef ) = len( cSub ) ) ) then
            irc = RETVAL_MATCH_EXACT
        endif
    endif

    if ( GVERBOSE ) then
        select case irc
        case RETVAL_MATCH_NONE      : printlog( CFN & "No matching substring found" )
        case RETVAL_MATCH_EXACT     : printlog( CFN & "Strings are identical" )
        case RETVAL_MATCH_SUBSTRING : printlog( CFN & "String is substring" )
        end select
    endif
    
    hCompareSubStrings() = irc
    '///</ul>
    
end function

'******************************************************************************

function hGetDirTreeLevel( cFullPath as string ) as integer

    '///<h3>Count the numbers of pathseparators in a path-string</h3>
    '///<i>Used to find the level of current directory within the directory tree.<br>
    '///+The function prints a warning when no pathseparators were found</i><br><br>
    '///<u>Parameter(s):</u>
    '///<ol>
    '///+<li>Path (string) with <b>no trailing pathseparator</b></li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Number of Pathseparators within the string (integer)</li>
    '///<ul>
    '///+<li>0 = failure</li>
    '///+<li>&gt; 0 = level</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hGetDirTreeLevel::"
    
    dim iCurrentChar as integer
    dim cCurrentChar as string
    dim iSeparatorCount as integer : iSeparatorCount = 0
    
    '///+<li>Walk through the string</li>
    '///<ul>
    for iCurrentChar = 1 to len( cFullPath )
    
        '///+<li>Get the next character</li>
        cCurrentChar = mid( cFullPath , iCurrentChar , 1 )
        
        '///+<li>If it is a separtator, increase the counter</li>
        if ( cCurrentChar = gPathSigne ) then iSeparatorCount = iSeparatorCount + 1
        
    next iCurrentChar
    '///</ul>
    
    '///+<li>Print a warning if no separators were found</li>
    if ( iSeparatorCount = 0 ) then
        warnlog( CFN & "Did not find any separators in given string: " & cFullPath )
    endif
    '///</ul>
    
    hGetDirTreeLevel() = iSeparatorCount

end function


'*******************************************************************************

function hGetStringFromStaticTextField( oControl as object ) as string

    use "global\tools\includes\optional\t_accels.inc"

    '///<h3>Get the string from a static textfield</h3>

    '///<i>Static textfields like those in the document properties dialog are
    '///+ in certain places designed in a way that the string can be selected
    '///+ and copied to the clipboard. This has been introduced with SRC680m212
    '///+ (9156). This function uses keyboard shortcuts for &quot;Select All&quot;
    '///+ and &quot;Copy&quot; to get the string into the clipboard as .uno.Copy
    '///+ is not enabled.</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>Name of the control (Object)</li>
    '///<ul>
    '///+<li>The object must exist in the current context</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Content of the field (String)</li>
    '///<ul>
    '///+<li>Blank if string is empty</li>
    '///</ul>
    '///</ol>

    const CFN = "hGetStringFromStaticTextField::"
    dim brc as boolean 'a multi purpose boolean returnvalue

    dim cSelectAll as string
    dim cCopy as string
    dim cText as string

    if ( GVERBOSE ) then printlog( CFN & "Enter" )

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Verify that the object exists</li>
    '///+<li>Get the accelerator for SelectAll and Copy</li>
    '///+<li>Execute SelectAll and Copy on control</li>
    '///+<li>Get the string from the clipboard</li>
    if ( oControl.exists() ) then
        if ( oControl.isVisible() ) then
            cSelectAll = hGetAccel( "SelectAll" )
            cCopy      = hGetAccel( "Copy"      )

            oControl.typeKeys( "<right>" ) 
            oControl.typeKeys( cSelectAll )
            oControl.typeKeys(   cCopy    )
    
            cText = getClipboardText()
            if ( GVERBOSE ) then printlog( CFN & "Exit with result: " & cText )
        else
            ctext = ""
            warnlog( CFN & "Exit: Control exists but is not visible" )
        endif
    else
        cText = ""
        warnlog( CFN & "Exit: Control does not exist in this context" )
    endif

    '///</ul>

    hGetStringFromStaticTextField() = cText

end function


'*******************************************************************************

function hConvertStringToLong( cValue as string ) as long


    '///<h3>Convert a stringvalue to long int</h3>

    '///<i>The purpose of this function is to isolate the filesize from a string
    '///+ of the type &quot;1.345 Bytes&quot; by removing the thousands-separator
    '///+ and the trailing unit. The result is then a filesize as long integer
    '///+ which then can be compared to the result from the BASIC function 
    '///+ FileLen( FileSpec )</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>String containing a long integer value (String)</li>
    '///<ul>
    '///+<li>No floating point values allowed</li>
    '///+<li>Numeric value must be at the beginning of the string (no leading characters/spaces)</li>
    '///+<li>Negative values are allowed</li>
    '///+<li>Leading &quot;+&quot; is not allowed</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Value of the string as long integer (Long)</li>
    '///<ul>
    '///+<li>Thousands separator (. or ,) have been removed</li>
    '///+<li>Decimal separators (though not allowed) have been removed</li>
    '///</ul>
    '///</ol>

    const CFN = "hConvertStringToLong::"
    const ONE_CHARACTER = 1

    if ( GVERBOSE ) then printlog( CFN & "Enter with option: " & cValue )

    dim iLen as integer : iLen = len( cValue )

    dim iChar as integer
    dim cChar as string

    dim cStringValue as string : cStringValue = ""

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Walk through the single chars of the string</li>
    '///<ul>
    for iChar = 1 to iLen

        '///+<li>Get the current character</li>
        cChar = mid( cValue , iChar , ONE_CHARACTER )

        '///+<li>Copy valid characters to temporary string, drop invalid, exit on first space or other character</li>
        select case cChar
        case "-" : cStringValue = cStringValue & cChar
        case "1" : cStringValue = cStringValue & cChar
        case "2" : cStringValue = cStringValue & cChar
        case "3" : cStringValue = cStringValue & cChar
        case "4" : cStringValue = cStringValue & cChar
        case "5" : cStringValue = cStringValue & cChar
        case "6" : cStringValue = cStringValue & cChar
        case "7" : cStringValue = cStringValue & cChar
        case "8" : cStringValue = cStringValue & cChar
        case "9" : cStringValue = cStringValue & cChar
        case "0" : cStringValue = cStringValue & cChar
        case else: ' do nothing
        end select

    next iChar
    '///</ul>

    if ( GVERBOSE ) then printlog( CFN & "Exit with value: " & cStringValue )

    '///+<li>Convert string to long integer and return to calling function</li>
    hConvertStringToLong() = val( cStringValue )
    '///</ul>

end function

'*******************************************************************************

function hStringReplace( cString as string, search_string as string, replace_with as string ) as string

    const CFN = "hStringReplace(): "

    dim search_string_position as string
    dim search_string_found as boolean   : search_string_found = true
    dim len_search_string as integer     : len_search_string = len( search_string )
    dim len_replace_with  as integer     : len_replace_with  = len( replace_with  )
    dim myString as string               : myString = cString

    if ( GVERBOSE ) then printlog( CFN & "Replace all <" & search_string & "> with <" & replace_with & "> in <" & myString & ">" )

    if ( not instr( replace_with, search_string ) and len_search_string >= len_replace_with ) then
        do while( search_string_found )
            search_string_position = instr( myString, search_string )
            if ( search_string_position > 0 ) then
                mid( myString, search_string_position, len( search_string ), replace_with )
            else
                search_string_found = false
            endif
        loop
    else
        warnlog( CFN & "Function used incorrectly" )
        warnlog( CFN & "Replace all <" & search_string & "> with <" & replace_with & "> in <" & myString & ">" )
        warnlog( CFN & "The new string must be of equal length or shorter than the string to be replaced" )
        warnlog( CFN & "The new string may not contain the string to be replaced (e.g. replace 'a' with 'ha' is not allowed)" )
    endif

    if ( GVERBOSE ) then printlog( CFN & "Return string is <" & myString & ">" )
    hStringReplace() = myString

end function
