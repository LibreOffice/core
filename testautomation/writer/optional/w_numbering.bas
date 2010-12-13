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
'* short description : Writer numbering test
'*
'\***********************************************************************

global TBOstringLocale(30) as string
global WriterFilterExtension(10) as string

sub main
    Dim StartZeit
    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool5.inc"
    use "writer\tools\includes\w_tool6.inc"
	use "writer\tools\includes\w_tool7.inc"
    use "writer\optional\includes\number\w_205_.inc"
    use "writer\optional\includes\number\w_205a_.inc"
	use "writer\optional\includes\number\w_206_.inc"
	use "writer\optional\includes\number\w_207_.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )

    WriterFilterExtension() = hGetFilternameExtension ( "writer8" )

    Call hStatusIn("writer","w_numbering.bas","Writer Numbering test")
    Printlog ""
    Printlog "----------------------------------------------------------"
    Printlog "|                  Writer Numbering Test                 |"
    Printlog "----------------------------------------------------------"
    Printlog ""
    Call w_205_
    Call w_205a_
	Call w_206_
	Call w_207_
    Call hStatusOut

    Printlog "Duration: "+ WieLange ( StartZeit )
    Printlog "Date: " +  Date + "    Time: " + Time
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_ui_filters.inc"
    use "global/tools/includes/optional/t_listfuncs.inc"
    use "global/tools/includes/optional/t_security_tools.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end sub
