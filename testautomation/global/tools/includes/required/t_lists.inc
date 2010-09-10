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
'* owner : helge.delfs@oracle.com
'*
'* short description : general routines to work with lists (arrays)
'*
'\******************************************************************

function ListCount ( lsList() as String ) as Integer
    'Author: tz
    '///Returns the number of list entries.
    '///+<u>Input</u>: The list (only string lists are possible)
    '///+<u>Return</u>: The number of entries
    ListCount =  Val(lsList(0))
end function

'-------------------------------------------------------------------------

function ListCopy ( lsList1() as String, lsList2() as String ) as Boolean
    'Author: tz
    '///Copies all entries out of one list into another list.
    '///+<u>Input</u>:<ol><lo>list which should be copied</li><li>An empty list</li></ol>After this function the 2nd list is a copy of the 1st list.
    '///+<u>Return</u>: If copy of the list is correct this function returns TRUE otherweise FALSE

    Dim ii as Integer

    ListAllDelete ( lsList2() )
    for ii=1 to ListCount ( lsList1() )
        ListAppend ( lsList2(), lsList1(ii) )
    next ii

    if ListCount ( lsList1() ) = ListCount ( lsList2 () ) then
        ListCopy = TRUE
    else
        ListCopy = FALSE
    end if
end function

'-------------------------------------------------------------------------

sub ListAllDelete ( lsList() as String )
    'Author: tz
    '///Deletes a complete list.
    '///+<u>Input</u>: The list (only string lists are possible)
    lsList(0) = "0"
end sub

'-------------------------------------------------------------------------

sub ListAppend ( lsList() as String, sNewEntry as String )
    'Author: tz
    '///Appends a new entry at the end of the list.
    '///+<u>Input</u>: <ol><li>the list (only string lists are possible)</li><li>The new entry</li></ol>
    lsList(0) = Val(lsList(0)) + 1
    lsList( lsList(0) ) = sNewEntry
end sub

'-------------------------------------------------------------------------

function ListDelete ( lsList() as String, iNr as Integer ) as Boolean
    'Author: tz
    '///Deletes an entry out of the list on a defined position (iNr).
    '///+<u>Input</u>: <ol><li>The list (only string lists are possible)</li><li>The position of the entry</li></ol>
    '///+<u>Return</u>: TRUE if the entry was deleted otherweise FALSE


    Dim i%, ListenAnzahl as Integer

    ListenAnzahl = listcount( lsList() )

    if iNr > ListenAnzahl then
        ListDelete = FALSE
        Exit Function
    end if

    for i% = iNr to ListenAnzahl
        lsList( i% ) = lsList( i% + 1 )
    next i%

    lsList(0) = ListenAnzahl - 1

    ListDelete = TRUE
end function

'-------------------------------------------------------------------------

function ListDeleteString ( lsList() as String, sText as String ) as Boolean
    'Author: tz
    '///Deletes the 1st string in the list which is equal to the input string.
    '///+<u>Input</u>: <ol><li>The list (only string lists are possible)</li><li>The string</li></ol>
    '///+<u>Return</u>: TRUE if the entry was deleted otherwise FALSE
    Dim i as Integer : Dim EintragsNr as Integer : Dim ListenAnzahl as Integer

    ListenAnzahl = Val(lsList(0))
    EintragsNr = 0
    for i = 1 to ListenAnzahl
        if lsList(i) = sText then
            EintragsNr = i
            i = ListenAnzahl + 1
        end if
    next i
    if EintragsNr = 0 then
        ListDeleteString = FALSE
    else
        ListDeleteString = ListDelete ( lsList(), EintragsNr )
    end if
end function

'-------------------------------------------------------------------------

function ListInsert ( lsList() as String, ZeileNr%, sWert$ ) as Boolean
    'Author: tz
    '///Inserts a string at a defined position in the list.
    '///+<u>Input</u>: <ol><li>The list (only string lists are possible)</li><li>The position</li><li>The string</li></ol>
    '///+<u>Return</u>: TRUE if the entry was inserted otherwise FALSE
    Dim i% : Dim ListenAnzahl as Integer

    ListenAnzahl = Val(lsList(0))
    if ZeileNr% > ListenAnzahl then
        ListInsert = FALSE
        Exit Function
    end if

    ' Nach hinten verschieben, hinten beginnend
    for i% = ListenAnzahl to ZeileNr% step -1
        lsList( i%+1 ) = lsList( i% )
    next i%

    ' Einfuegen
    lsList( ZeileNr% ) = sWert$
    lsFile(0) = ListenAnzahl + 1
    ListInsert = TRUE

end function

'-------------------------------------------------------------------------

function ListRead ( lsList() as String, Datei$, optional sEncode as String ) as Boolean
    'Author: tz
    '///+Opens a file and insert all rows into a list (row for row).
    '///+<u>Input</u>: <ol><li>The list (old list entries will be deleted)</li><li>The file</li><li><b>optional</b>: The encoding &quot;UTF8&quot;</li></ol>
    '///+<u>Return</u>: TRUE or FALSE if this routine can read the file.
    Dim bUTF8 as Boolean
    Dim i%
    Dim CompareList(15000) as String

    if Dir( Datei$ ) = "" then
        Warnlog "ListRead: " + Datei$ + " is missing!"
        ListRead = FALSE
        exit function
    end if

    if IsMissing ( sEncode ) = TRUE then
        bUTF8 = FALSE
    else
        if UCASE ( sEncode ) = "UFT8" OR UCASE ( sEncode ) = "UTF8" then
            bUTF8 = TRUE
        else
            Warnlog "ListRead :" +  sEncode + " - Encoding is unkown!"
            bUTF8 = FALSE
        end if
    end if

    ListAllDelete ( lsList() )                             ' clean up the list

    if bUTF8 =  TRUE then
        Dim textin as object, sfa as object, xInput as object                   ' for UTF-8-input-routines
        Dim iC as Integer

        textin = createUnoService( "com.sun.star.io.TextInputStream" )         ' uno-handling to input an UFT-8-File
        textin.setEncoding("utf8")                                             '
        sfa = createUnoService( "com.sun.star.ucb.SimpleFileAccess" )          '
        xInput = sfa.openFileRead( Datei$ )                                    '
        textin.setInputStream( xInput )                                        '

        do until textin.isEOF()                                                '
            i% = Val(lsList(0)) + 1
            lsList(0) = i%
            lsList( i% ) = textin.readLine()                                    '
        loop
        xInput.closeInput                                                      ' uno-file-close

        'INFO: (TZ) Only to workaround a problem with UNIX-Files...
        if Right ( lsList(i%), 1 ) = Chr(10) then
            lsList(i%) = Left ( lsList(i%), Len ( lsList(i%) ) - 1 )
        end if
        'INFO: (TBO) Remove the BOM http://www.unicode.org/versions/Unicode4.0.0/ch15.pdf
        if (left(lsList(1), 1) = chr(&HFEFF)) then
            lsList(1) = right(lsList(1), Len(lsList(1)) - 1)
        end if
    else
        Dim FileNum%

        FileNum% = FreeFile
        Open Datei$ for input  as #FileNum%

        do until EOF(#FileNum%) ' all from LIS-file
            i% = Val(lsList(0)) + 1
            lsList(0) = i%
            Line Input #FileNum%, lsList( i% )
        loop
        Close #FileNum%
    end if
    ListRead = TRUE
end function

'-------------------------------------------------------------------------

function ListWrite ( lsList() as String, Datei$, optional sEncode as String) as Boolean
    'Author: tz
    '///+Writes a list into a file (an existing file will be deleted before)
    '///+<u>Input</u>: <ol><li>The list</li><li>The file</li><li><b>optional</b>: The encoding &quot;UTF8&quot;</li></ol>
    '///+<u>return</u>: TRUE or FALSE if this routine can read the file.

    Dim bUTF8 as Boolean
    Dim i%

    if Dir (Datei$) <> "" then
        Kill(Datei$)  ' the file must be deleted if you use 'UTF8'
    endif

    if IsMissing ( sEncode ) = TRUE then
        bUTF8 = FALSE
    else
        if UCASE ( sEncode ) = "UTF8" then
            bUTF8 = TRUE
        else
            Warnlog "ListWrite :" +  sEncode + " - Encoding is unkown!"
            bUTF8 = FALSE
        end if
    end if

    if bUTF8 =  TRUE then
        Dim textout as object, sfa as object, xOutput as object                  ' for UTF-8-output-routines

        textout = createUnoService( "com.sun.star.io.TextOutputStream" )        ' uno-handling to output an UFT-8-File
        textout.setEncoding("utf8")                                             '
        sfa = createUnoService( "com.sun.star.ucb.SimpleFileAccess" )           '
        xOutput = sfa.openFileWrite( Datei$ )                                   '
        textout.setOutputStream( xOutput )                                      '

        for i%=1 to ListCount ( lsList() )
            textout.writeString( lsList( i% ) + Chr(13) + Chr(10) )              '
        next i%
        xOutput.closeOutput                                                     ' uno-file-close
    else
        Dim FileNum% : Dim iLast%

        FileNum% = FreeFile
        Open Datei$ for Output  as #FileNum%
        iLast% = Val(lsList(0))
        i%=1
        do while i% <= iLast%
            Print #FileNum%, lsList(i%)
            i% = i% +1
        loop
        Close #FileNum%
    endif

    ListWrite = TRUE
end function

'-------------------------------------------------------------------------

function ListReadAppend( lsList() as String , Datei$, optional sEncode as String ) as Boolean
    'Author: tz
    '///+Appends a list into a file (If the file exists the file will be deleted before!).
    '///+<u>Input</u>: <ol><li>The list</li><li>The file</li><li><b>optional</b>: The encoding &quot;UTF8&quot;</li></ol>
    '///+<u>return</u>: TRUE or FALSE if this routine can read the file.

    Dim bUTF8 as Boolean
    Dim i%
    Dim CompareList() as String
    Dim isCounter as Integer
    Dim FileNum%

    if Dir( Datei$ ) = "" then
        Warnlog "ListReadAppend : " + Datei$ + " is missing!"
        ListReadAppend = FALSE
        exit function
    end if

    isCounter = ListCount ( lsList() )

    if IsMissing ( sEncode ) = TRUE then
        bUTF8 = FALSE
    else
        if UCASE ( sEncode ) = "UFT8" OR UCASE ( sEncode ) = "UTF8" then
            bUTF8 = TRUE
        else
            Warnlog "ListRead : " + sEncode + " - Encoding is unkown!"
            bUTF8 = FALSE
        end if
    end if

    if bUTF8 =  TRUE then
        Dim textin as object, sfa as object, xInput as object                  ' for UTF-8-input-routines

        textin = createUnoService( "com.sun.star.io.TextInputStream" )         ' uno-handling to input an UFT-8-File
        textin.setEncoding("utf8")                                             '
        sfa = createUnoService( "com.sun.star.ucb.SimpleFileAccess" )          '
        xInput = sfa.openFileRead( Datei$ )                                    '
        textin.setInputStream( xInput )                                        '

        do until textin.isEOF()                                                '
            i% = Val(lsList(0)) + 1
            lsList(0) = i%
            lsList( i% ) = textin.readLine()                                    '
        loop
        xInput.closeInput                                                      ' uno-file-close

        'INFO: (TZ) Only to workaround a problem with UNIX-Files...
        if Right ( lsList(i%), 1 ) = Chr(10) then
            lsList(i%) = Left ( lsList(i%), Len ( lsList(i%) ) - 1 )
        end if
        '...
    else
        FileNum% = FreeFile
        Open Datei$ for input  as #FileNum%

        do until EOF(FileNum%)                            ' All from LIST-file
            i% =  Val(lsList(0)) + 1
            lsList(0) = i%
            Line Input #FileNum%, lsList( i% )
        loop
        Close #FileNum%
    end if

    ListReadAppend = TRUE

end function

'-------------------------------------------------------------------------

function ListWriteAppend( lsList() as String, Datei$, optional sEncode as String ) as Boolean
    'Author: tz
    '///+Writes a list into a file (If the files exist all entries will be appended).
    '///+<u>Input</u>: <ol><li>The list</li><li>The file</li><li><b>optional</b>: The encoding &quot;UTF8&quot;</li></ol>
    '///+<u>return</u>: TRUE or FALSE if this routine can read the file.

    Dim bUTF8 as Boolean
    Dim i%
    Dim DummyList ( 15000 ) as String

    if IsMissing ( sEncode ) = TRUE then
        bUTF8 = FALSE
    else
        if UCASE ( sEncode ) = "UTF8" then
            bUTF8 = TRUE
        else
            Warnlog "ListRead :" +  sEncode + " - Encoding is unkown!"
            bUTF8 = FALSE
        end if
    end if

    if bUTF8 =  TRUE then
        Dim sfa as object, xOutput as object, textout as object                 ' for UTF-8-output-routines

        ListRead ( DummyList(), Datei$, "utf8" )                               ' read old file in another list
        for i% = 1 to ListCount ( lsList() )
            ListAppend ( DummyList(), lsList(i%) )                              ' add the new list at the old list
        next i%

        textout = createUnoService( "com.sun.star.io.TextOutputStream" )       ' uno-handling to output an UFT-8-File
        textout.setEncoding("utf8")                                            '
        sfa = createUnoService( "com.sun.star.ucb.SimpleFileAccess" )          '
        xOutput = sfa.openFileWrite( Datei$ )                                  '
        textout.setOutputStream( xOutput )                                     '

        for i%=1 to ListCount (DummyList())
            textout.writeString( DummyList( i% ) + Chr(13) + Chr(10 )           '
        next i%
        xOutput.closeOutput                                                    ' uno-file-close
    else
        Dim FileNum%

        FileNum% = FreeFile
        Open Datei$ for Append  as #FileNum%

        for i% = 1 to Val(lsList(0))
            Print #FileNum%, lsList(i%)
        next i%

        Close #FileNum%
    end if
    ListWriteAppend = TRUE

end function

'-------------------------------------------------------------------------

sub ListSort ( lsList() as String, optional UpDown as Boolean )
    'Author: tz
    '///+Sorts a list upward per default or downward if optional parameter is FALSE with quicksort method.
    '///+<u>Input</u>: Unsorted list

    Dim Listenanzahl as Integer, i as Integer, j as Integer
    Dim Zwischenspeicher as String

    ListenAnzahl = Val(lsList(0))
    for i=ListenAnzahl-1 to 1 step -1
        for j=1 to i
            if UpDown = FALSE then
                ' upward sorting
                if uCase ( lsList(j) ) < uCase ( lsList(j+1) ) then
                    Zwischenspeicher = lsList (j)                               ' invert value (i) with value (i+1)
                    lsList (j) = lsList(j+1)
                    lsList (j+1) = Zwischenspeicher
                end if
            else
                ' Downward sorting
                if uCase ( lsList(j) ) > uCase ( lsList(j+1) ) then
                    Zwischenspeicher = lsList (j)                               ' invert value (i) with value (i+1)
                    lsList (j) = lsList(j+1)
                    lsList (j+1) = Zwischenspeicher
                end if
            end if
        next j
    next i
end sub

'*******************************************************************************

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



