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
'* short description : Test bug fixes made for StarOffice 7 PP1 - srx645_m22s1-5  
'*
'\***********************************************************************

sub main

PrintLog "---------------------------------------------------------------------"
Printlog "-----                    Calc SO7 PP1 Bug Fix Check             -----"
PrintLog "---------------------------------------------------------------------"

   use "spreadsheet\optional\includes\so7pp1\c_so7_pp1_iz.inc"
   use "spreadsheet\optional\includes\so7pp1\c_so7_pp1_ibis.inc"
   use "spreadsheet\tools\includes\c_select_tools.inc"
   
   Call hStatusIn("spreadsheet", "c_so7_pp1.bas", "Test bug fixes made for StarOffice 7 PP1")
 
        Printlog Chr(13) + "--------- Izzuezilla Bugs fixed? ---------"

        call tIZ19381
        call tIZ21036
          
        Printlog Chr(13) + "--------- IBIS Bugs fixed? ---------"

        call tIBIS111099
        call tIBIS111158
        call tIBIS111711

    call hStatusOut       

end sub

sub LoadIncludeFiles

   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   gApplication   = "CALC"
   Call GetUseFiles

End Sub

