'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_listfuncs.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-20 07:58:28 $
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
'/******************************************************************************
'*
'* owner : joerg.skottke@sun.com
'*
'* short description : Replacements for routines in t_lists.inc adds some
'*
'*******************************************************************************
'*
' #1 hListTestUpperBoundary ' Tests upper boundary of arrays
' #1 hListTestLowerBoundary ' Tests lower boundary of arrays
' #1 hListDelete            ' Deletes one item from a list by index
' #1 hListAppend            ' Append an item to a list
' #1 hManageComparisionList ' quick way to compare/create reference lists
' #1 hListFileGetSize       ' find out how big an array has to be to hold the file
' #1 hListCompare           ' compare two lists
' #1 hListPrependString     ' Insert a string infront of each item in a list
' #1 hListAppendList        ' Append one list to another
' #1 hCountMatchesInList    ' Return count of occurrences of a string within a list
'*
'\******************************************************************************

' Note: These functions fix some minor bugs and introduce strict boundary
'       checking for the arrays we work with. The arrays must be compatible
'       to those from the "standard" list-functions. 
' Why:  Two reasons:
'
'       1) When working with listboxes it might happen that they are empty (bug)
'          or contain more items than expected. In this case the tests would 
'          usually break. This is not desired as many testcases do not rely 
'          on the content of the listboxes. 
'          
'       2) This way eases the trouble of debugging huge amounts of arrays
'          like those in the installation test or anywhere else where we work 
'          with reference lists. This is a coding help.

'*******************************************************************************

function hListTestUpperBoundary( aList() as string ) as boolean

    '///<h3>Verify that ListCount does not exceed upper boundary</h3>
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
    '///<u>Input</u>:
    '///<ol>
    '///+<li>List (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE: Array is ok</li>
    '///+<li>FALSE: Array logic has errors</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    
    const CFN = "hListTestUpperBoundary::"
    
    dim iUpperBoundary as integer ' size according to UBOUND
    dim iListSize as integer  ' size according to ListCount
    
    dim brc as boolean
        brc = true
    
    '///+<li>Determine the size of the array</li>
    iUpperBoundary = UBOUND( aList() )
    
    '///+<li>Determine the <i>claimed</i> size of the array</li>
    iListSize      = ListCount( aList() )
    
    '///+<li>Verify that val(array(0)) <= array-size</li>
    if ( iListSize > iUpperBoundary ) then
        warnlog ( CFN & "List points beyound upper array boundary:" )
        printlog( CFN & "ListCount: " & iListSize  )
        printlog( CFN & "UBOUND...: " & iUpperBoundary )
        brc = false
    endif
    
    hListTestUpperBoundary() = brc
    '///</ul>

end function

'*******************************************************************************

function hListTestLowerBoundary( aList() as string ) as boolean

    '///<h3>Verify that the lower boundaries of an array are ok</h3>
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
    '///<u>Input</u>:
    '///<ol>
    '///+<li>List (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE: Array is ok</li>
    '///+<li>FALSE: Array logic has errors</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>    
    
    const CFN = "hListTestLowerBoundary::"
    
    dim iLowerBoundary as integer ' size according to LBOUND
    dim iListSize as integer  ' size according to ListCount
    
    dim brc as boolean
        brc = true
        
    iLowerBoundary = LBOUND( aList() )
    iListSize      = ListCount( aList() )   
    
    '///+<li>Verify that ubound for the array returns 0 (lower boundary)</li>
    if ( iLowerBoundary <> 0 ) then
        warnlog ( CFN & "Boundary of the array must be 0." )
        printlog( CFN & "Lower boundary is: " & iLowerBoundary )
        brc = false
    endif
    
    '///+<li>Verify that val(array(0)) &gt; 0</li>
    if ( iListSize < 0 ) then
        warnlog ( CFN & "Defined Listsize (ListCount) may never be negative" )
        printlog( CFN & "ListCount is: " & iListSize )
        brc = false
    endif
   
    hListTestLowerBoundary() = brc
    '///</ul>
    
end function

'*******************************************************************************

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
    '///<u>Review the code, it has many unused variables</u>
    '///<ul>

    const CFN = "hListDelete::"

    dim iArraySize as integer   ' The size of the array, must be large enough
    dim iListSizeOld as integer ' The size of the list before deletion
    dim iListSizeNew as integer ' The size of the list after deletion
    dim iCurrentItem as integer ' Increment-Variable
    dim iOffset as integer      ' First item to be "moved down" by index
    dim sItemToDelete as string ' The string that will be deleted
    dim brc as boolean          ' preliminary return value
    
    '///+<li>test array integrity: upper boundary</li>
    brc = hListTestUpperBoundary( alist() )
    if ( not brc ) then
        hListDelete() = brc
        exit function
    endif
    
    '///+<li>test array integrity: lower boundary</li>
    brc = hListTestLowerBoundary( alist() )
    if ( not brc ) then
        hListDelete() = brc
        exit function
    endif    
    
    '///+<li>Get some data from the arrays to work with.</li>
    iArraySize    = ubound( aList() )
    iListSizeOld  = ListCount( aList() )
    iListSizeNew  = iListSizeOld - 1
    sItemToDelete = aList( iItemToDelete )
    iOffset       = iItemToDelete + 1
    
    ' some output (may be removed as soon the function is thoroughly tested)
    'printlog( CFN & "Removing: " & sItemToDelete & " at pos " & iItemToDelete )
    
    ' Move all items down by one in the list beginning with the item after
    ' iItemToDelete
    '///+<li>Move all items one up</li>
    for iCurrentItem = iOffset to iListSizeOld
        aList( iCurrentItem - 1 ) = aList( iCurrentItem )
    next iCurrentItem
    
    ' Delete the last entry, it is no longer used and it is duplicate to the item
    ' at iListSizeOld-1 (iListSizeNew)
    '///+<li>Delete the last item from the list</li>
    aList( iListSizeOld ) = ""
    
    '///+<li>Set the new listsize (one smaller than the original list)</li>
    aList( 0 ) = iListSizeNew
    '///</ul>
    
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
        irc = -1
    else
        aTargetList( iNewListSize ) = sNewString
        aTargetList( 0 ) = iNewListSize
        irc = iNewListSize
    endif
    
    hListAppend() = irc
    
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
    
    '///+<li>The name of the input file may not be empty</li>
    if ( sFileIn = "" ) then
        warnlog( CFN & "Invalid parameter: Input filename is empty string" )
        hManageComparisionList() = 2
        exit function
    endif
    
    '///+<li>The name of the output-file may not be empty</li>
    if ( sFileOut = "" ) then
        warnlog( CFN & "Invalid parameter: Output filename is empty string" )
        hManageComparisionList() = 3
        exit function
    endif    
    
    '///+<li>the list should not claim to be empty / be empty</li>
    if ( listcount( sListOut() ) = 0 ) then
        qaerrorlog( CFN & "Invalid parameter: Array claims to be empty" )
        hManageComparisionList() = 4
        exit function
    endif   
    
    ' hListFileGetSize will return -1 if the list does not exist or the number
    ' of lines in the reference file plus additional 10 lines.
    '///+<li>Verify that the reference file exists and is non-empty</li>
    dim iFileSize as integer
        iFileSize = hListFileGetSize( sFileIn ) 
    dim brc as boolean
        brc = false
    
    '///+<li>Read the reference list and compare</li>
    if ( iFileSize > -1 ) then
    
        dim aReferenceList( iFileSize ) as string
        printlog( CFN & "Reading: " & sFileIn )
        ' disabled hGetDataFileSection because some lists contain a #
        ' (hash) as value which is identified as comment by the function.
        ' Fixing this for hGetDataFileSection() would break compatibility
        ' to other functions both in framework and global module
        'hgetDataFileSection( sFileIn , aReferenceList() , "" , "" , "" )
        listread( aReferenceList(), sFileIn, "utf8" )
        brc = hListCompare( sListOut() , aReferenceList() )
        
    endif
    
    ' A this point there are three possible states:
    ' a) the reference list does not exist
    ' b) the comparision failed
    ' c) the comparision succeeded
    ' only if hListCompare() returns TRUE the testrun is successful.
    ' This means that on any error, the ref-list will be written so it can be
    ' directly reviewed/compared to the "faulty" list without having to run this
    ' test again (after deleting the ref-file)
    '///+<li>In case the lists are not identical, write the new one to the local work directory</li>
    if ( brc ) then
        printlog( CFN & "Comparision succeeded" )
        hManageComparisionList() = 0    
    else
        printlog( CFN & "Writing: " & sFileOut )
        listwrite( sListOut(), sFileOut, "UTF8" )
        qaerrorlog ( CFN & "Reference file review required:" )
        printlog( "" )
        printlog( CFN & "Two possible reasons:" )
        printlog( CFN & "1) The reference file does not exist at all" )
        printlog( CFN & "2) Reference and actual UI-Content do not match." )
        printlog( CFN & "Location: " & sFileOut )
        printlog( CFN & "Target..: " & sFileIn  )
        printlog( CFn & "Verify the content and copy the file to <Target>" )
        printlog( "" )
        printlog( CFN & "Comparision failed" )
        hManageComparisionList() = 1
    endif
    
    '///+<li>Return 0 if the lists are identical, 1 if not and 2-4 on any other error</li>
    '///</ul>

end function

'*******************************************************************************

function hListFileGetSize( sFileIn as string ) as integer

    '///<h3>Get the number of lines from a file</h3>
    '///<i>Prerequisites: Path to an existing plain text file</i>
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
    '///<ul>

    const CFN = "hListFileGetSize::"
    const I_EXTRA_SIZE = 10
    
    '///+<li>Verify that the filename is non-empty</li>
    if ( sFileIn = "" ) then
        warnlog( CFN & "Invalid parameter: Filename is empty string" )
        hListFileGetSize() = -1
        exit function
    endif
    
    '///+<li>Verify that the file exists</li>
    if ( dir( sFileIn ) = "" ) then
        warnlog( CFN & "File not found: " & sFileIn ) 
        hListFileGetSize() = -1
        exit function
    endif
    
    dim iFile as integer
    dim sLine as string
    dim iLineCount as integer
        iLineCount = 0
    
    '///+<li>Open the file (standard BASIC calls)</li>
    iFile = freefile
    open sFileIn for input as iFile
    
    '///+<li>Read the number of lines from the file</li>
    while( not eof( iFile ) ) 
    
        line input #iFile, sLine
        iLineCount = iLineCount + 1
        
    wend
    
    '///+<li>Close the file</li>
    close #iFile 
    
    '///+<li>Return the number of lines read or -1 on error</li>
    hListFileGetSize() = iLineCount + I_EXTRA_SIZE
    '///</ul>
    
end function

'*******************************************************************************

function hListCompare( aListOne() as String, aListTwo() as String ) as boolean

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
    dim brc as boolean     ' returncode: true only if lists are identical
        brc = true
    
    '///+<li>Create a copy of list two so we do not change the original list</li>
    ListCopy( aListTwo() , aTwoOnlyList() )

    iTwoOnlyListSize = ListCount( aTwoOnlyList() )
    iListOneSize = ListCount( aListOne() )

    
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
        if ( not bFound ) then
            hListAppend( aListOne( iListOneIndex ), aOneOnlyList() )
        end if
        
    next iListOneIndex
    
    ' restore compatibility to listfunctions so hListPrint() will not fail
    aTwoOnlyList( 0 ) = iTwoOnlyListSize
    
    '///+<li>List all items that exist in List One only</li>
    if ( ListCount( aOneOnlyList() ) > 0 ) then
        printlog( CFN & "Objects have been added to the list" )
        hListPrint( aOneOnlyList() , "Items found in list ONE only (NEW)" )
        brc = false
    end if
        
    '///+<li>List all items that exist in List Two only</li>
    if ( ListCount( aTwoOnlyList() ) > 0 ) then
        printlog( CFN & "Objects have been removed from the list" )
        hListPrint( aTwoOnlyList() , "Items found in list TWO only (MISSING)" )
        brc = false
    end if

    hListCompare() = brc
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
    printlog( CFN & "Enter" )
    
    dim iHitCount as integer
    dim iItemCount as integer
    dim iCurrentItem as integer

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Retrieve the number of items in the list</li>
    iItemCount = ListCount( acItemList() )
    
    '///+<li>Walk through the list and count the hits</li>
    printlog( CFN & "Begin with term: " & cSearchTerm )
    for iCurrentItem = 1 to iItemCount
    
        printlog( acItemList( iCurrentItem ) )
    
        if ( instr( acItemList( iCurrentItem ), cSearchTerm ) > 0 ) then
            iHitCount = iHitCount + 1
        endif
        
    next iCurrentItem
    printlog( CFN & "End" )
   '///</ul>

    printlog( CFN & "Exit with result: " & iHitCount )

    hCountMatchesInList() = iHitCount

end function
