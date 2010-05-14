'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_dirloc.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-13 10:27:10 $
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
'/************************************************************************
'*
'* owner : thorsten.bosbach@sun.com
'*
'* short description : functions for directories and files; execution happens in the testtool
'*
'\************************************************************************

function hFileExistsLocal ( Dat as String ) as Boolean
    '/// Checks if a file exists
    '/// <u>Input</u>: Filename with complete path
    '/// <u>Return</u>: TRUE or FALSE if the file exists.
    if Dir ( Dat ) = "" then
        hFileExistsLocal = FALSE
    else
        hFileExistsLocal = TRUE
    end if
end function
'
'-------------------------------------------------------------------------------
'
function hDirectoryExistsLocal ( Verz as String ) as Boolean
    '/// Checks if a directory exists
    '/// <u>Input</u>: Directory with complete path
    '/// <u>Return</u>: TRUE or FALSE if the directory exists.
    ' at the end of the string has to be teh path seperator, else the dir-command doesn't work
    if right ( Verz, 1 ) <> gPathSigne then Verz = Verz + gPathSigne
    if Dir ( Verz, 16 ) = "" then
        hDirectoryExistsLocal = FALSE
    else
        hDirectoryExistsLocal = TRUE
    end if
end function
'
'-------------------------------------------------------------------------------
'
function hKillFileLocal ( Dat as String ) as Boolean
    '/// Delete a file
    '/// <u>Input</u>: File with complete path
    '/// <u>Return</u>: TRUE or FALSE success on deleting?
    if Dir ( Dat ) <> "" then
        try
            kill ( Dat )
        catch
        endcatch
        if Dir ( Dat ) <> "" then
            hKillFileLocal = FALSE
        else
            hKillFileLocal = TRUE
        end if
    else
        hKillFileLocal = TRUE
    end if
end function
'
'-------------------------------------------------------------------------------
'
function DirNameListLocal (ByVal sPfad$ , lsDirName() as String ) as Integer
    '/// seperate a path in its parts
    '/// <u>Input</u>: Path to seperate; Empty list, because it get's reset in this function!;
    '/// <u>Return</u>: Number on entries in the list; list with entries
    Dim i%
    Dim Pos%

    lsDirName(0) = 0
    do
        Pos% = InStr(1, sPfad$, "\")  ' got a path
        i% = Val(lsDirName(0) ) + 1
        lsDirName(0) = i%
        lsDirName( i%  ) = Left( sPfad$, Pos%  )    ' .. put in list
        sPfad = Mid( sPfad$, Pos% + 1 )         ' ...cut off
    loop while Pos%>0
    lsDirName( i%  ) = sPfad$
    DirNameListLocal = i%    ' count of
end function
'
'-------------------------------------------------------------------------------
'
function GetFileNameListLocal ( sPath$, sMatch$ ,lsFile() as String  ) as integer
    '/// Get files from a directory that match the pattern and append them to a list (without path)
    '/// <u>Input</u>: Directory with complete path; Search Pattern, e.g *.*; List
    '/// <u>Return</u>: count of appended entries; updated list
    Dim Count%
    Dim Datname as String

    Count% = 0

    if right ( sPath$, 1 ) <> gPathSigne then sPath$ = sPath$ + gPathSigne
    ' at the end of the string has to be teh path seperator, else the dir-command doesn't work
    Datname = Dir( sPath$ + sMatch$ , 0)

    do until Len(Datname) = 0
        Count% = Count% + 1
        lsFile(Count%) = Datname ' append
        lsFile(0) = Count%
        Datname = Dir
    loop

    GetFileNameListLocal = Count%    ' all files
end function
'
'-------------------------------------------------------------------------------
'
function GetFileListLocal ( sPath$, sMatch$ ,lsFile() as String  ) as integer
    '/// Get files from a directory that match the pattern and append them to a list (<b>with</b> path)
    '/// <u>Input</u>: Directory with complete path; Search Pattern, e.g *.*; List
    '/// <u>Return</u>: count of appended entries; updated list
    Dim Count%
    Dim Datname as String

    Count% = 0

    if right ( sPath$, 1 ) <> gPathSigne then sPath$ = sPath$ + gPathSigne
    ' at the end of the string has to be teh path seperator, else the dir-command doesn't work
    Datname = Dir( sPath$ + sMatch$ , 0)

    do until Len(Datname) = 0
        lsFile(0) = Val(lsFile(0)) + 1
        lsFile( lsFile(0) ) =sPath$ + Datname
        Count% = Count% + 1
        Datname = Dir
    loop
    GetFileListLocal = Count%
end function
'
'-------------------------------------------------------------------------------
'
function GetDirListLocal ( sPath$, sMatch$ ,lsFile() as String ) as integer
    '/// Get Subdirectories from a directory and append them to a list (<b>with</b> path)
    '/// <u>Input</u>: Directory with complete path; Search Pattern, e.g *; List
    '/// <u>Return</u>: count of appended entries; updated list
    Dim Count%
    Dim Verzeichnis as String

    if right ( sPath$, 1 ) <> gPathSigne then sPath$ = sPath$ + gPathSigne
    ' at the end of the string has to be teh path seperator, else the dir-command doesn't work
    Verzeichnis = Dir( sPath$ + sMatch$ , 16)
    Count% = 0

    do until Len(Verzeichnis) = 0
        if Verzeichnis <>"." AND Verzeichnis <> ".." then
            lsFile(0) = Val(lsFile(0)) + 1
            lsFile( lsFile(0) ) = sPath$  + Verzeichnis + gPathSigne
            Count% = Count% + 1
        end if
        Verzeichnis = Dir
    loop

    GetDirListLocal = Count%
end function
'
'-------------------------------------------------------------------------------
'
function GetAllDirListLocal ( byVal sPath$, byVal sMatch$ ,lsFile() as String ) as integer
    '/// Get all directorys recursiv that match the pattern and append them to a list
    '/// <u>Input</u>: Directory with complete path; Search Pattern, e.g *; Empty list, because it get's reset in this function!;
    '/// <u>Return</u>: Count of appended entries (1. entry is the whole path); updated list
    Dim Count% : Dim DirCount%

    DirCount% = 1   ' dummy
    Count% = 1
    lsFile(0) = 1           'new list
    lsFile(1) = sPath$      'first path is the calling path

    do until Count%>Val(lsFile(0))  ' get first generation
        DirCount% = GetDirListLocal ( lsFile(Count%) , sMatch$, lsFile() )        ' append all subdirectories
        Count% = Count% +1
    loop

    GetAllDirListLocal = Count% - 1  ' count of listelements
end function
'
'-------------------------------------------------------------------------------
'
function GetAllFileListLocal ( byVal sPath$, byVal sMatch$ ,lsFile() as String ) as integer
    '/// Get all Files recursiv (including in subdirectories) that match the pattern and append them to a list
    '/// <u>Input</u>: Directory with complete path; Search Pattern, e.g *.*; Empty list, because it get's reset in this function!;
    '/// <u>Return</u>: Count of appended entries (1. entry is the whole path); updated list
    Dim DirCount% : Dim FileCount% : Dim Count%
    Dim lsDir(1000) as String

    DirCount% = GetAllDirListLocal ( sPath$, "*", lsDir() ) ' just all directories
    FileCount% = 0
    lsFile(0) = 1
    lsFile(1) = sPath$

    For Count% = 1 to Val( lsDir(0) )
        FileCount% = FileCount% + GetFileListLocal ( lsDir( Count% ), sMatch$, lsFile() )
    next Count%

    GetAllFileListLocal = FileCount% ' count of files
end function
'
'-------------------------------------------------------------------------------
'
function KillFileListLocal ( lsList() as String ) as Boolean
    '/// Delete all files in the list
    '/// <u>Input</u>: List with files
    '/// <u>Return</u>: TRUE or FALSE if files are killed; modified list with not deleted files.
    Dim i as Integer
    Dim FehlerListe ( 1000 ) as String

    FehlerListe ( 0 ) = 0
    for i=1 to ListCount ( lsList() )
        try
            kill ( lsList(i) )
        catch
            ListAppend ( FehlerListe (), lsList(i) )
        endcatch
    next i

    lsList(0) = 0               ' delete old list
    KillFileListLocal = TRUE
    for i=1 to ListCount ( FehlerListe () )
        KillFileListLocal = FALSE
        ListAppend ( lsList(), FehlerListe (i) )
    next i
end function
'
'-------------------------------------------------------------------------------
'
function KillDirListLocal ( lsList() as String ) as Boolean
    '/// Delete all directories in the list
    '/// <u>Input</u>: List with directories
    '/// <u>Return</u>: TRUE or FALSE if directories are killed; modified list with not deleted directories.
    Dim i as Integer
    Dim FehlerListe ( 1000 ) as String

    FehlerListe ( 0 ) = 0
    for i=1 to ListCount ( lsList() )
        try
            rmDir ( lsList(i) )
        catch
            ListAppend ( FehlerListe (), lsList(i) )
        endcatch
    next i

    lsList(0) = 0               ' delete old list
    KillDirListLocal = TRUE
    for i=1 to ListCount ( FehlerListe () )
        KillDirListLocal = FALSE
        ListAppend (  lsList(), FehlerListe (i) )
    next i
end function
'
'-------------------------------------------------------------------------------
'
function GetFileSizesLocal ( lsList() as String ) as long
    '/// Computes the total Filesize of the files in the list
    '/// <u>Input</u>: List with files
    '/// <u>Return</u>: Filesize in bytes
    Dim iSum
    Dim i as Integer

    iSum = 0
    for i=1 to ListCount ( lsList() )
        try
            iSum = iSum + FileLen ( lsList(i) )
        catch
        endcatch
    next i

    GetFileSizesLocal = iSum
end function
