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
'* owner : oliver.craemer@Sun.COM
'*
'* short description : calc named range level 2 test
'*
'\************************************************************************

global sDefaultCalcFilter as STRING
global sDefaultExtension as STRING

sub main
dim startedAt
startedAt = now()
PrintLog "---------------------------------------------------------------------"

   use "spreadsheet\optional\includes\namedrange\c_namedrange.inc"
   use "spreadsheet\tools\includes\c_select_tools.inc"
   
   call hStatusIn("spreadsheet", "c_namedrange.bas")               

    sDefaultCalcFilter = "calc8"
    sDefaultExtension = "ods"
   
   call cNamedRange

PrintLog "-------------------------------------------------------------------"
Printlog "  End: " + Date + " at: " + Time + " Duration: " + wielange(startedAt)
PrintLog "-------------------------------------------------------------------"

   call hStatusOut                                 

end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   gApplication   = "CALC"
   Call GetUseFiles

end sub
