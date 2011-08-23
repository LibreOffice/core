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
'* Owner : wolfram.garten@oracle.com
'*
'* short description :
'*
'*********************************************************************
' #1 tViewZoom
'\********************************************************************

testcase tViewZoom
 dim Zaehler as integer
 dim Position1 as integer
 dim Position2 as integer
 Dim Datei$
   Printlog "- view Zoom"

    if (gApplication = "IMPRESS") then 
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

    Datei$ = ConvertPath (gOfficePath + "user\work\test." & ExtensionString)

    Printlog "- View-Zoom testing"
  if dir(Datei$) <> "" then app.Kill(Datei$) ' to avoid the anoying overwrite warnlog :-)
  Call hNewDocument            '/// new document ///'
  ViewZoom
  Kontext "Massstab"
  Vergroesserung100.Check       '/// set zoom to 50% ///'
  Massstab.OK
  '/// create a number of rectangles with different color properties ///'
   Kontext "SD_Farbleiste"
   sleep 1
    if SD_Farbleiste.Exists then
       Printlog "- Color toolbar will be disabled now"
       ViewToolbarsColorBar
    end if
   '/// All rectangles are created with same dimensions but using different zoom settings ///'
   For Zaehler = 1 to 9
      Position1 = 5 * Zaehler - 3
      Position2 = 105 - 5 * Zaehler
      Call hRechteckErstellen (Position1, Position1, Position2, Position2)
      FormatArea
      kontext
      Active.SetPage TabFarben
      Kontext "TabFarben"
      Farbe.Select 5*Zaehler
      TabFarben.OK
       sleep 1
       gMouseClick 50,0
   Next Zaehler
   sleep 2
  hFileSaveAs (Datei$)        '/// save document ///'
  sleep 3
   for Zaehler = 1 to 6	          '/// changing zoom settings and checking functionality with controlling the color of the selected rectangle ///'
      ViewZoom
      Kontext "Massstab"
      VergroesserungStufenlos.check
      Stufenlos.Settext "45"
      Massstab.OK
       sleep 2
       gMouseClick 50,1
      ViewZoom
      Kontext "Massstab"
      Select Case Zaehler
         Case 1: VergroesserungStufenlos.check
                 Stufenlos.Settext "40"
         Case 2: VergroesserungStufenlos.check
                 Stufenlos.Settext "50"
         Case 3: VergroesserungStufenlos.check
                 Stufenlos.Settext "75"
         Case 4: VergroesserungStufenlos.check
                 Stufenlos.Settext "100"
         Case 5: VergroesserungStufenlos.check
                 Stufenlos.Settext "150"
         Case 6: VergroesserungStufenlos.check
                 Stufenlos.Settext "200"
                 
         'Case 2: Vergroesserung50.check
         'Case 3: Vergroesserung75.check
         'Case 4: Vergroesserung100.check
         'Case 5: Vergroesserung150.check
         'Case 6: Vergroesserung200.check
      End Select
      Massstab.OK
       sleep 1
      Kontext "DocumentImpress"
       gMouseClick 15,15            'hier soll die Maus ein Rechteck treffen;
                                    'die Farbe des Rechtecks sagt uns dann, ob
      FormatArea                    'richtig vergroessert oder verkleinert wurde.
       sleep 1
      Kontext
      Active.SetPage TabFarben
      Kontext "TabFarben"
      printlog "Color index ", Farbe.GetSelIndex
      TabFarben.Cancel
      sleep 1
   next Zaehler

  'GanzeSeite.push
  'Optimal.push
  'Seitenbreite.push
  sleep 2

 Call hCloseDocument         '/// close document ///'
endcase
