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

function gCompare2Lists( aListOne() as String, aListTwo() as String ) as boolean

    const CFN = "global::tools::inc::t_list.inc::gCompare2Lists: "

    '///<h3>Compare two lists with each other, where <b>list TWO</b> is the reference</h3>
    '///<ul>

    dim aOneOnlyList( ubound( aListOne() ) ) as string
    dim aTwoOnlyList( ubound( aListTwo() ) ) as string

    dim iListOneIndex as integer
    dim iListTwoIndex as integer

    dim bFound as boolean
    dim brc as boolean     ' returncode: true if lists are identical
    brc = true

    '///+<li>Create a copy of list two so we do not change the original list</li>
    ListCopy( aListTwo() , aTwoOnlyList() )

    '///+<li>Step through each item in list one</li>
    for iListOneIndex = 1 to ListCount( aListOne() )

        bFound = false

        '///+<li>Compare it to each item in list two</li>
        for iListTwoIndex = 1 to ListCount( aTwoOnlyList() )

            '///+<li>If the entries match, delete it from the TwoOnly list</li>
            if ( aListOne( iListOneIndex ) = aTwoOnlyList( iListTwoIndex ) ) then

                bFound = true
                ListDelete( aTwoOnlyList() , iListTwoIndex )
                exit for

            end if

        next iListTwoIndex

        '///+<li>If there is no match, the item exists in list one only -> copy</li>
        if ( not bFound ) then
            ListAppend( aOneOnlyList() , aListOne( iListOneIndex ) )
        end if

    next iListOneIndex

    '///+<li>List all items that exist in List One only</li>
    if ( ListCount( aOneOnlyList() ) > 0 ) then
        warnlog( CFN & "Objects have been added to the list" )
        hListPrint( aOneOnlyList() , "Items found in list ONE only (NEW)" )
        brc = false
    end if

    '///+<li>List all items that exist in List Two only</li>
    if ( ListCount( aTwoOnlyList() ) > 0 ) then
        warnlog( CFN & "Objects have been removed from the list" )
        hListPrint( aTwoOnlyList() , "Items found in list TWO only (MISSING)" )
        brc = false
    end if

    gCompare2Lists() = brc
    '///</ul>

end function

'*******************************************************************************

function hListPrint( lsList() as string , optional cComment as string ) as integer

    const CFN = "global::tools::inc::t_list.inc::hListPrint: "

    '///<h3>Print the content of a list to the log with a heading comment</h3>
    '///<ul>

    dim iListItem as integer

    '///+<li>If no comment is provided we print a qaerrorlog</li>
    if ( ismissing( cComment ) ) then
        qaerrorlog( CFN & "Please provide any string as second parameter." )
        cComment = ""
    end if

    '///+<li>Print a comment if desired</li>
    if ( cComment <> "" ) then
        printlog( "" )
        printlog( CFN & cComment )
        printlog( "" )
    end if

    '///+<li>Print all items in the list to the log</li>
    for iListItem = 1 to listcount( lsList() )
        printlog( "(" & iListItem & ") :  " & lsList( iListItem ) )
    next iListItem

    '///+<li>Return the number of listitems to the calling function</li>
    hListPrint() = listcount( lsList() )

    '///</ul>

end function

'*******************************************************************************

function hListClearPattern( lsList() as string, cPattern as string ) as integer

    '///<h3>Search a list for the occurrence of a special pattern.</h3>
    '///+ If the pattern is found, the entries are deleted, the new size of the
    '///+ array is returned.

    dim iCurItem as integer
    iCurItem = 1

    do while ( iCurItem <= listcount( lsList() ) )

        if ( instr( lsList( iCurItem ) , cPattern ) <> 0 ) then
            listdelete( lsList() , iCurItem )
        else
            iCurItem = iCurItem + 1
        end if

    loop

    hListClearPattern() = listcount( lsList() )

end function

'*******************************************************************************

function hListClearBlank( lsList() as string ) as integer

    '///<h3>Search a list for blank lines and remove them.</h3>

    dim iCurItem as integer
    iCurItem = 1

    do while ( iCurItem <= listcount( lsList() ) )

        if ( len( lsList( iCurItem ) )  = 0 ) then
            listdelete( lsList() , iCurItem )
        else
            iCurItem = iCurItem + 1
        end if

    loop

    hListClearBlank() = listcount( lsList() )

end function



