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

