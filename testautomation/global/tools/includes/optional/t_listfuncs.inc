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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : Replacements for routines in t_lists.inc adds some
'*
'\******************************************************************************

function hListDelete( aList() as string, iItemToDelete as integer ) as boolean

    '///<h3>Delete one item from a list specified by index</h3>
    '///<i>Prerequisite: Array compatible with those from t_lists.inc</i><br>
    '///<i>About listfunctions: All listfunctions rely on a special type of
    '///+ array. This can be string arrays and - in some cases - numeric
    '///+ arrays. What makes the arrays unique is that the first item which
    '///+ has the index 0 contains the number of items in the list to be used,
    '///+ anything that is stored beyond this number is ignored. This has three 
    '///+ consequences: 1) all listfunctions that alter an array must update
    '///+ the index stored in array(0) and 2) it is possible that the index
    '///+ point beyond ubound of the array which will most likely cause a
    '///+ runtime error. 3) Means that arrays may only have an upper boundary
    '///+ declared, all loops must start with index array(1) and must end with
    '///+ index array(val( array(0))</i><br>    

    const CFN = "hListDelete::"
    const INDEX_CORRECTION = 1
    dim iCurrentItem as integer ' Increment-Variable
    
    if ( GVERBOSE ) then 
        printlog( CFN & "Removing: " & aList( iItemToDelete ) & " at pos " & iItemToDelete )
    endif
    
    ' Move all items down by one in the list beginning with the item after
    ' iItemToDelete
    for iCurrentItem = ( iItemToDelete + INDEX_CORRECTION ) to ListCount( aList() )
        aList( iCurrentItem - INDEX_CORRECTION ) = aList( iCurrentItem )
    next iCurrentItem
    
    ' Delete the last entry, it is no longer used and it is duplicate to the item
    ' at iListSizeOld-1 (iListSizeNew)
    aList( iCurrentItem ) = ""
    aList( 0 ) = iCurrentItem - INDEX_CORRECTION
    
end function

'*******************************************************************************

function hListAppend( sNewString as string, aTargetList() as string ) as integer

    '///<h3>Append an item to an existing list</h3>
    '///<i>Prerequisite: Array compatible with those from t_lists.inc</i>
    '///<i>About listfunctions: All listfunctions rely on a special type of
    '///+ array. This can be string arrays and - in some cases - numeric
    '///+ arrays. What makes the arrays unique is that the first item which
    '///+ has the index 0 contains the number of items in the list to be used,
    '///+ anything that is stored beyond this number is ignored. This has three 
    '///+ consequences: 1) all listfunctions that alter an array must update
    '///+ the index stored in array(0) and 2) it is possible that the index
    '///+ point beyond ubound of the array which will most likely cause a
    '///+ runtime error. 3) Means that arrays may only have an upper boundary
    '///+ declared, all loops must start with index array(1) and must end with
    '///+ index array(val( array(0))</i><br>    

    const CFN = "hListAppend::"
    const RC_ARRAY_TOO_SMALL = -1

    dim iCurrentListSize as integer
    dim iNewListSize as integer
    dim iArraySize as integer
    dim irc as integer

    iCurrentListSize = val( aTargetList( 0 ) )
    iNewListSize = iCurrentListSize + 1
    iArraySize = ubound( aTargetList() )
    
    if ( iNewListSize > iArraySize ) then
        warnlog ( CFN & "Cannot append, array too small" )
        printlog( CFN & "Array-Size.....: " & iArraySize )
        printlog( CFN & "Requested index: " & iNewListSize )
        hListAppend() = RC_ARRAY_TOO_SMALL
    else
        aTargetList( iNewListSize ) = sNewString
        aTargetList( 0 ) = iNewListSize
        hListAppend() = iNewListSize
    endif
    
end function

'*******************************************************************************

function hManageComparisionList( sFileIn as string, sFileOut as string, sListOut() as string ) as integer

    '///<h3>Function to create or compare a list to a reference</h3>
    '///<i>Prerequisite: List of items to compare, input- and outputfilename</i><br>
    '///<i>About listfunctions: All listfunctions rely on a special type of
    '///+ array. This can be string arrays and - in some cases - numeric
    '///+ arrays. What makes the arrays unique is that the first item which
    '///+ has the index 0 contains the number of items in the list to be used,
    '///+ anything that is stored beyond this number is ignored. This has three 
    '///+ consequences: 1) all listfunctions that alter an array must update
    '///+ the index stored in array(0) and 2) it is possible that the index
    '///+ point beyond ubound of the array which will most likely cause a
    '///+ runtime error. 3) Means that arrays may only have an upper boundary
    '///+ declared, all loops must start with index array(1) and must end with
    '///+ index array(val( array(0))</i><br>    
    '///<u>BEWARE: This is a core function and used by many tests!<br>
    '///Please read the inline documentation for further reference</u><br>
    '///Function parameters:
    '///<ol>
    '///+<li>sFileIn    = The file that contains the reference data</li>
    '///+<li>sFileOut   = The file new lists are written to in case of an error</li>
    '///+<li>sListOut() = The list containing the newly retrieved data.</li>
    '///</ol>
    '///Description:
    '///<ul>

    const CFN = "hManageComparisionList::"

    ' maximum lines per file. Currently this limit is determined by the help
    ' tests which have up to 22000 entries + reseve.
    const FILESIZE = 25000
    const COMPARE_SUCCESS = 0
    const ENCODING_UTF8 = "UTF8"

    dim irc as integer
    dim aReferenceList( FILESIZE ) as string

    if ( GVERBOSE ) then printlog( CFN & "Reading: " & sFileIn )

    ' Do not use hGetDataFileSection() as strings in some lists may begin with a
    ' "#" which is interpreted as a comment by hGetDataFileSection()
    listread( aReferenceList(), sFileIn, ENCODING_UTF8 )

    ' Word of caution: If the number of new items equals the number of removed items
    ' this function returns 0 -> success. This case is highly unlikely to ever happen
    ' unless someone renames scripts.
    irc = hListCompare( sListOut() , aReferenceList() )
        
    '///+<li>In case the lists are not identical, write the new one to the local work directory</li>
    if ( irc = COMPARE_SUCCESS ) then
        printlog( CFN & "Comparision succeeded" )
        hManageComparisionList() = COMPARE_SUCCESS
    else
        printlog( CFN & "The two compared lists differ. There are a number of possible reasons:" )
        printlog( CFN & "- Installation requirements are not met (setup /a?, missing packages?)" )
        printlog( CFN & "- Reference and actual UI-Content do not match: File an issue." )
        printlog( CFN & "- The reference file does not exist: Follow steps below." )
        printlog( CFN & "Verify and copy the file: " & sFileOut )
        printlog( CFN & "to this location........: " & sFileIn  )
        printlog( CFN & "Check this file into the SCM or attach it to an issue" )
        listwrite( sListOut(), sFileOut, ENCODING_UTF8 )
        hManageComparisionList() = irc
    endif

    '///+<li>Return number of differences between the lists</li>
    '///</ul>

end function

'*******************************************************************************

function hListCompare( aListOne() as String, aListTwo() as String ) as integer

    const CFN = "hListcompare::"

    '///<h3>Compare two lists with each other, where <b>list TWO</b> is the reference</h3>
    '///<i>Prerequisites: Two lists compatible with listfunctions</i><br>
    '///<i>About listfunctions: All listfunctions rely on a special type of
    '///+ array. This can be string arrays and - in some cases - numeric
    '///+ arrays. What makes the arrays unique is that the first item which
    '///+ has the index 0 contains the number of items in the list to be used,
    '///+ anything that is stored beyond this number is ignored. This has three 
    '///+ consequences: 1) all listfunctions that alter an array must update
    '///+ the index stored in array(0) and 2) it is possible that the index
    '///+ point beyond ubound of the array which will most likely cause a
    '///+ runtime error. 3) Means that arrays may only have an upper boundary
    '///+ declared, all loops must start with index array(1) and must end with
    '///+ index array(val( array(0))</i><br>    
    '///<u>Duplicates gCompare2Lists but does not print warnlogs, evaluate returncode instead</u>
    '///<ul>

    dim aOneOnlyList( ubound( aListOne() ) ) as string
    dim aTwoOnlyList( ubound( aListTwo() ) ) as string
    
    dim iListOneIndex as integer
    dim iListTwoIndex as integer
    
    dim iTwoOnlyListSize as integer
    dim iListOneSize as integer
    
    dim bFound as boolean
    
    '///+<li>Create a copy of list two so we do not change the original list</li>
    ListCopy( aListTwo() , aTwoOnlyList() )

    iTwoOnlyListSize = ListCount( aTwoOnlyList() )
    iListOneSize     = ListCount( aListOne() )

    
    '///+<li>Step through each item in list one</li>
    for iListOneIndex = 1 to iListOneSize
    
        bFound = false
    
        '///+<li>Compare it to each item in list two</li>
        for iListTwoIndex = 1 to iTwoOnlyListSize
        
            '///+<li>If the entries match, delete it from the TwoOnly list</li>
            if ( aListOne( iListOneIndex ) = aTwoOnlyList( iListTwoIndex ) ) then
            
                bFound = true
                aTwoOnlyList( iListTwoIndex ) = aTwoOnlyList( iTwoOnlyListSize )
                ' this breaks compatibility to listfunctions because the actual
                ' number of items is out of sync with listcount
                iTwoOnlyListSize  = iTwoOnlyListSize -1                 
                exit for
                
            end if
            
        next iListTwoIndex
        
        '///+<li>If there is no match, the item exists in list one only -> copy</li>
        if ( not bFound ) then hListAppend( aListOne( iListOneIndex ), aOneOnlyList() )
        
    next iListOneIndex
    
    ' restore compatibility to listfunctions so hListPrint() will not fail
    aTwoOnlyList( 0 ) = iTwoOnlyListSize
    
    '///+<li>List all items that exist in List One only</li>
    if ( ListCount( aOneOnlyList() ) > 0 ) then
        printlog( CFN & "Objects have been added to the list" )
        hListPrint( aOneOnlyList() , "Items found in list ONE only (NEW)" )
        hListCompare() = ListCount( aOneOnlyList() )
    end if
        
    '///+<li>List all items that exist in List Two only</li>
    if ( ListCount( aTwoOnlyList() ) > 0 ) then
        printlog( CFN & "Objects have been removed from the list" )
        hListPrint( aTwoOnlyList() , "Items found in list TWO only (MISSING)" )
        hListCompare() = ListCount( aTwoOnlyList() ) * -1
    end if

    '///</ul>
    
end function

'*******************************************************************************

function hListPrependString( aList() as string, cString as string ) as boolean

    '///<h3>Insert a string infront of each item in a list</h3>
    '///<i>Prerequisites: A list compatible with listfunctions</i><br>
    '///<i>About listfunctions: All listfunctions rely on a special type of
    '///+ array. This can be string arrays and - in some cases - numeric
    '///+ arrays. What makes the arrays unique is that the first item which
    '///+ has the index 0 contains the number of items in the list to be used,
    '///+ anything that is stored beyond this number is ignored. This has three 
    '///+ consequences: 1) all listfunctions that alter an array must update
    '///+ the index stored in array(0) and 2) it is possible that the index
    '///+ point beyond ubound of the array which will most likely cause a
    '///+ runtime error. 3) Means that arrays may only have an upper boundary
    '///+ declared, all loops must start with index array(1) and must end with
    '///+ index array(val( array(0))</i><br><br>
    '///<i>Note that the function alters the input list. If the list contains 
    '///+ strings of the type &quot;MyString&quot; the items will be changed to
    '///+ read &quot;Some Text : MyString&quot;</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>List (string)</li>
    '///+<li>A text to be inserted infront of every item in the list</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>The returnvalue is currently undefined</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hListPrependString::"
    dim iCurrentItem as integer
    
    '///+<li>Cycle through the list and insert a text infront of each item</li>
    for iCurrentItem = 1 to listcount( aList() )
        aList( iCurrentItem ) = cString & " : " & aList( iCurrentItem )
    next iCurrentItem
    
    hListPrependString() = true
    '///</ul>
    
end function

'*******************************************************************************

function hListAppendList( aBaseList() as string, aListToAppend() as string ) as integer

    '///<h3>Append one list to another</h3>
    '///<i>Prerequisites: A list compatible with listfunctions</i><br>
    '///<i>About listfunctions: All listfunctions rely on a special type of
    '///+ array. This can be string arrays and - in some cases - numeric
    '///+ arrays. What makes the arrays unique is that the first item which
    '///+ has the index 0 contains the number of items in the list to be used,
    '///+ anything that is stored beyond this number is ignored. This has three 
    '///+ consequences: 1) all listfunctions that alter an array must update
    '///+ the index stored in array(0) and 2) it is possible that the index
    '///+ point beyond ubound of the array which will most likely cause a
    '///+ runtime error. 3) Means that arrays may only have an upper boundary
    '///+ declared, all loops must start with index array(1) and must end with
    '///+ index array(val( array(0))</i><br><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Target list (string)</li>
    '///+<li>Source list (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Listsize (integer)</li>
    '///<ul>
    '///+<li>The size of the sum of both lists</li>
    '///+<li>0 in case of error</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hListAppendList::"
    
    dim iCurrentItem as integer
    dim iNewSize as integer
    
    '///+<li>Do some basic boundary checking</li>
    if ( ubound( aBaseList() ) < _
        ( listcount( aBaseList ) + listcount( aListToAppend() ) ) ) then
        warnlog( CFN & "Base Array too small" )
        iNewSize = 0
    else
    
        '///+<li>Append the list</li>
        for iCurrentItem = 1 to listcount( aListToAppend() ) 
  
            hListAppend( aBaseList() , aListToAppend( iCurrentItem ) )
            
        next iCurrentItem
        
        iNewSize = listcount( aBaseList() ) 
        
    endif
    '///</ul>

end function


'*******************************************************************************

function hCountMatchesInList( acItemList() as string, cSearchTerm as string ) as integer


    '///<h3>Find out how many times a string is found in a list</h3>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>List to be searched (String)</li>
    '///<ul>
    '///+<li>The list may not be empty</li>
    '///+<li>Search begins at index 1</li>
    '///</ul>

    '///+<li>Expression to search for (String)</li>
    '///<ul>
    '///+<li>Only exact matches are counted</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Number of hits (Integer)</li>
    '///<ul>
    '///+<li>0: if no matches were found</li>
    '///+<li>-1: Any error</li>
    '///</ul>
    '///</ol>

    const CFN = "hCountMatchesInList::"
    dim iHitCount as integer
    dim iCurrentItem as integer

    if ( GVERBOSE ) then printlog( CFN & "Begin with term: " & cSearchTerm )

    for iCurrentItem = 1 to ListCount( acItemList() )
        if ( GVERBOSE ) then printlog( acItemList( iCurrentItem ) )

        if ( instr( acItemList( iCurrentItem ), cSearchTerm ) > 0 ) then
            iHitCount = iHitCount + 1
        endif
    next iCurrentItem

    if ( GVERBOSE ) then printlog( CFN & "Exit with result: " & iHitCount )
    hCountMatchesInList() = iHitCount

end function

'*******************************************************************************

function hListResultEvaluation( i_diffcount as integer, i_allowed_delta as integer ) as boolean

    ' This function evaluates the outcome of hManageComaprisionList() or
    ' hListCompare(). This extra step is done because in some cases the
    ' program installations might differ slightly - in some CWS (when using the
    ' archive) we can end up having a different set of import/export filters.
    ' So the evaluation must allow for a specific number of mismatches which is
    ' specified in i_allowed_delta.

    hListResultEvaluation() = true

    ' If lists are identical we return directly.
    if ( i_diffcount = 0 ) then
        printlog( "The lists are identical. Good" )
        exit function
    endif

    ' if we have differences we need to have a closer look.
    ' Note that the difference is optional.
    if ( i_allowed_delta <> 0 ) then
        if ( i_diffcount = i_allowed_delta ) then
            printlog( "The lists have the allowed delta of " & i_allowed_delta )
            exit function
        endif
    endif

    warnlog( "The list check failed, please review the test." )
    hListResultEvaluation() = false

end function
