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
function GetFileNameListLocal ( sPath$, sMatch$ ,lsFile() as String  ) as integer

    ' DUPLICATE FUNCTION but used in ./global/tools/compressstatus.bas

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
