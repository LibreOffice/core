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
'* short description : To get Help ID's from the office
'*
'\*************************************************************************************
sub main
   Ich_Moechte_Help_IDs_haben
'   Ich_Moechte_Die_Position_Und_Groesse_Eines_Dialoges_Haben ( ChaosDokument )

' ! set the proper Kontext for teh Application in the subroutine !
'   Ich_Moechte_Die_Mausposition_Herausbekommen 
end sub


sub Ich_Moechte_Help_IDs_haben
'/// Get Id's ///'
   DisplayHid true
end sub

sub Ich_Moechte_Die_Position_Und_Groesse_Eines_Dialoges_Haben ( window )
'/// Get Size an Position from dialogs ///'
   Dim i
   Dim Px : Dim Py : Dim Sx : Dim Sy

   for i = 1 to 20
      Px = window.GetPosX
      Py = window.GetPosY
      Sx = window.GetSizeX
      Sy = window.GetSizeY
      print "x: " + Px + "  y: " + Py + "  x-: " + Sx + "  y-: " + Sy
   next i
end sub

sub Ich_Moechte_Die_Mausposition_Herausbekommen
'/// Get Mouse Position ///'
 '  Writer
 '   Kontext "DocumentWriter"
 '   DocumentWriter.DisplayPercent

 '  Calc
 '   Kontext "DocumentCalc"
 '   DocumentCalc.DisplayPercent

 '  Draw
 '   Kontext "DocumentDraw"
 '   DocumentDraw.DisplayPercent

 '  Chart
 '    Kontext "DocumentChart"
 '    DocumentChart.DisplayPercent
end sub

sub LoadIncludeFiles
   use "global\system\includes\declare.inc"
   use "global\system\includes\gvariabl.inc"
   Call GetUseFiles()
end sub

