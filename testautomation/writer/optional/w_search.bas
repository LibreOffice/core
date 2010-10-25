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
'* short description : Search & Replace and testing of regular expressions
'*
'\***********************************************************************

global Such_Text as string
global Vergleichstext as string
global Meldungs_Text as string
global dummy_Text as string

sub main
    use "writer\optional\includes\regexp\regular.inc"
    use "writer\optional\includes\regexp\search.inc"

    Dim Start : Start = Now()

    Call hStatusIn("writer","w_search.bas","Search & Replace")
    printlog "** Search and Replace **"
    Call search
    printlog "** Regular expressions **"
    Call regular
    Call hStatusOut

    printlog "Duration : " +  Wielange ( Start )
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    gApplication = "WRITER"
    Call GetUseFiles
end sub
