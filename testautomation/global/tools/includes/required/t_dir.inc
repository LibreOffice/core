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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : functions for directories and files; execution happens in the office
'*
'\************************************************************************

function DirNameList (ByVal sPfad$ , lsDirName() as String ) as Integer
    '/// seperate a path in its parts
    '/// <u>Input</u>: Path to seperate; Empty list, because it get's reset in this function!;
    '/// <u>Return</u>: Number on entries in the list; list with entries
    Dim i% : Dim Pos%
    lsDirName(0) = 0
    do
        Pos% = InStr(1, sPfad$, gPathsigne )  ' got a part of teh path
        i% = Val(lsDirName(0) ) + 1
        lsDirName(0) = i%
        lsDirName( i%  ) = Left( sPfad$, Pos%  )    ' .. put into list
        sPfad = Mid( sPfad$, Pos% + 1 )         ' ...cut off
    loop while Pos%>0
    lsDirName( i%  ) = sPfad$
    DirNameList = i%    ' count of
end function
'
'-------------------------------------------------------------------------------
'
function GetFileNameList ( sPath$, sMatch$ ,lsFile() as String  ) as integer
    '/// Get files from a directory that match the pattern and append them to a list (without path)
    '/// <u>Input</u>: Directory with complete path; Search Pattern, e.g *.*; List
    '/// <u>Return</u>: count of appended entries; updated list
    Dim Count% : Dim Datname as String
    Dim i  as Integer
    Count% = 0
    ' at the end of the string has to be teh path seperator, else the dir-command doesn't work
    if right ( sPath$, 1 ) <> gPathSigne then sPath$ = sPath$ + gPathSigne
    Datname = app.Dir( sPath$ + sMatch$ , 0) ' 0: normal files
    for i=1 to 5
        if Right ( Datname, 1 ) = "." then
            Datname = app.Dir
        else
            i=10
        end if
    next i

    do until Len(Datname) = 0
        Count% = Count% + 1
        lsFile(Count%) = Datname    ' append
        lsFile(0) = Count%
        Datname = app.Dir
    loop

    GetFileNameList = Count%    ' All files
end function
'
'-------------------------------------------------------------------------------
'
function GetFileList ( sPath$, sMatch$ ,lsFile() as String  ) as integer
    '/// Get files from a directory that match the pattern and append them to a list (<b>with</b> path)
    '/// <u>Input</u>: Directory with complete path; Search Pattern, e.g *.*; List
    '/// <u>Return</u>: count of appended entries; updated list
    Dim Count% : Dim Datname as String
    Dim i as Integer
    Count% = 0
    ' at the end of the string has to be teh path seperator, else the dir-command doesn't work
    if right ( sPath$, 1 ) <> gPathSigne then sPath$ = sPath$ + gPathSigne
    Datname = app.Dir( sPath$ + sMatch$ , 0)
    for i=1 to 5
        if Right ( Datname, 1 ) = "." then
            Datname = app.Dir
        else
            i=10
        end if
    next i

    do until Len(Datname) = 0
        lsFile(0) = Val(lsFile(0)) + 1
        lsFile( lsFile(0) ) =sPath$ + Datname
        Count% = Count% + 1

        ' if the number of files in the directory exceeds the arraysize do not
        ' crash but try to handle the situation gracefully. Of course this
        ' makes the testresults worthless...
        if ( Count% = ubound( lsFile() ) ) then
            warnlog ( "List of files exceeds bounds of array." )
            printlog( "Processing of this directory will be discontinued." )
            printlog( "Last processed file was: " & Datname )
            printlog( "Arraysize is: " & ubound( lsFile() ) )
            Datname = ""
        else
            Datname = app.Dir
        endif
    loop
    GetFileList = Count%
end function
'
'-------------------------------------------------------------------------------
'
function GetDirList ( sPath$, sMatch$ ,lsFile() as String ) as integer
    '/// Get Subdirectories from a directory and append them to a list (<b>with</b> path)
    '/// <u>Input</u>: Directory with complete path; Search Pattern, e.g *; List
    '/// <u>Return</u>: count of appended entries; updated list
    Dim iFolderCount as integer
    Dim Folder as String
    ' at the end of the string has to be teh path seperator, else the dir-command doesn't work
    if right ( sPath$, 1 ) <> gPathSigne then sPath$ = sPath$ + gPathSigne
    Folder = app.Dir( sPath$ + sMatch$ , 16)
    iFolderCount = 0

    do until Len( Folder ) = 0
        select case ( lcase( Folder ) )
        case "."
        case ".."
        case ".svn"
        case ".hg"
        case else
            lsFile(0) = Val(lsFile(0)) + 1
            lsFile( lsFile(0) ) = sPath$  + Folder + gPathSigne
            iFolderCount = iFolderCount + 1
        end select
        Folder = app.Dir
    loop
    GetDirList = iFolderCount
end function
'
'-------------------------------------------------------------------------------
'
function GetAllDirList ( byVal sPath$, byVal sMatch$ ,lsFile() as String ) as integer
    '/// Get all directorys recursiv that match the pattern and append them to a list
    '/// <u>Input</u>: Directory with complete path; Search Pattern, e.g *; Empty list, because it get's reset in this function!;
    '/// <u>Return</u>: Count of appended entries (1. entry is the whole path); updated list
    Dim Count% : Dim DirCount%

    DirCount% = 1   ' dummy
    Count% = 1
    lsFile(0) = 1           'new list
    lsFile(1) = sPath$      'first path is the called path

    do until Count%>Val(lsFile(0))  ' get count of 1.generation
        DirCount% = GetDirList( lsFile(Count%) , sMatch$, lsFile() )        ' append all subdirectories
        Count% = Count% +1
    loop

    GetAllDirList = Count% - 1  ' count of...
end function
'
'-------------------------------------------------------------------------------
'
function GetAllFileList ( byVal sPath$, byVal sMatch$ ,lsFile() as String ) as integer
    '/// Get all Files recursiv (including in subdirectories) that match the pattern and append them to a list
    '/// <u>Input</u>: Directory with complete path; Search Pattern, e.g *.*; Empty list, because it get's reset in this function!;
    '/// <u>Return</u>: Count of appended entries (1. entry is the whole path); updated list
    Dim DirCount% : Dim FileCount% : Dim Count%
    Dim lsDir(1000) as String

    DirCount% = GetAllDirList( sPath$, "*", lsDir() ) ' erstmal _alle_ Verzeichnisse
    FileCount% = 0
    lsFile(0) = 1
    lsFile(1) = sPath$

    For Count% = 1 to Val( lsDir(0) )
        FileCount% = FileCount% + GetFileList( lsDir( Count% ), sMatch$, lsFile() )
    next Count%

    GetAllFileList = FileCount% ' Anzahl aller Dateien
end function
'
'-------------------------------------------------------------------------------
'
function KillFileList ( lsList() as String ) as Boolean
    '/// Delete all files in the list
    '/// <u>Input</u>: List with files
    '/// <u>Return</u>: TRUE or FALSE if files are killed; modified list with not deleted files.
    Dim i as Integer
    Dim FehlerListe ( 1000 ) as String

    FehlerListe ( 0 ) = 0
    for i=1 to ListCount ( lsList() )
        try
            app.kill ( lsList(i) )
        catch
            ListAppend ( FehlerListe (), lsList(i) )
        endcatch
    next i

    lsList(0) = 0               ' delete old list
    KillFileList = TRUE
    for i=1 to ListCount ( FehlerListe () )
        KillFileList = FALSE
        ListAppend (  lsList(), FehlerListe (i) )
    next i
end function
'
'-------------------------------------------------------------------------------
'
function KillDirList ( lsList() as String ) as Boolean
    '/// Delete all directories in the list
    '/// <u>Input</u>: List with directories
    '/// <u>Return</u>: TRUE or FALSE if directories are killed; modified list with not deleted directories.
    Dim i as Integer
    Dim FehlerListe ( 1000 ) as String

    FehlerListe ( 0 ) = 0
    for i=1 to ListCount ( lsList() )
        try
            app.rmDir ( lsList(i) )
        catch
            ListAppend ( FehlerListe (), lsList(i) )
        endcatch
    next i

    lsList(0) = 0               ' delete old list
    KillDirList = TRUE
    for i=1 to ListCount ( FehlerListe () )
        KillDirList = FALSE
        ListAppend (  lsList(), FehlerListe (i) )
    next i
end function
'
'-------------------------------------------------------------------------------
'
function DateiExtract ( sFileDat$ )
   '/// Get the filename from a path
    '/// <u>Input</u>: path with file
    '/// <u>Return</u>: filename without the path
    Dim i%
    dim ls(20) as String

    i% = DirNameList( sFileDat$, ls() )
    DateiExtract = ls(i%)
end function
'
'-------------------------------------------------------------------------------
'
function DateiOhneExt (Datei$) as String
    '/// Get the filename without the extension
    '/// <u>Input</u>: filename
    '/// <u>Return</u>: filename without the extension
    Dim wh as Integer
    Dim dummy as String

    dummy = Datei$
    for wh = 1 to len(dummy)
        if mid(dummy,wh,1) = "." then
            dummy = left(dummy,wh - 1)
            wh = len(dummy) + 1
        else
            dummy = dummy
        end if
    next wh
    DateiOhneExt = dummy
end function
'
'-------------------------------------------------------------------------------
'
function GetExtention ( Datei as String ) as string
    '/// Get the extension from a file
    '/// <u>Input</u>: filename
    '/// <u>Return</u>: extension of the file
    Dim i%
    for i% = 1 to len ( Datei )
        if mid(Datei,i%,1) = "." then Datei = right( Datei, len(Datei)-i%)
    next i%
    GetExtention = Datei
end function
'
'-------------------------------------------------------------------------------
'
function hSplitString( sString as string, sSeparator as string, iIndex as integer ) as string

    ' This function wraps around the "split" command and returns one single
    ' item by index. Index = 0 means the *LAST* item is returned as this is
    ' probably the most commonly used item. If the index is invalid (out of
    ' bounds) we print a warning and return an error string.

    const CFN = "global::tools::includes::required::t_dir.inc:hSplitString(): "
    const ERROR_MESSAGE = "Array out of bounds for the requested index in string "
    const ARRAY_INDEX_CORRECTION = 1 ' The array lower boundary is zero but
                                     ' function starts to count with one.

    ' Split the string into its fragments into an array with dynamic boundaries
    dim sArray() as string
    dim sReturnString as string : sReturnString = ""

    if ( GVERBOSE ) then
        printlog( CFN & "Separator is: " & sSeparator )
        printlog( CFN & "Original string is: " & sString )
    endif

    sArray() = split( sString, sSeparator )

    if ( GVERBOSE ) then
        printlog( CFN & "Number of items found: " & ubound( sArray() ) )
    endif

	' Special case: Index out of bounds
	if ( iIndex > ( ubound( sArray() ) + ARRAY_INDEX_CORRECTION ) or iIndex < 0 ) then
        warnlog( CFN   & ERROR_MESSAGE & sString )
		hSplitString() = ERROR_MESSAGE & sString
		exit function
    endif

    ' Special case: Last item requested (this usually is the filename from a path)
    if ( iIndex = 0 ) then
        sReturnString = sArray( ubound( sArray() )
        if ( GVERBOSE ) then printlog( CFN & sReturnString )
    	hSplitString() = sReturnString
    	exit function
    endif

    ' Default is to return the requested item.
    sReturnString = sArray( iIndex - ARRAY_INDEX_CORRECTION )
    if ( GVERBOSE ) then printlog( CFN & sReturnString )
    hSplitString() = sReturnString

end function

