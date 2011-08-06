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
    printlog "to avoid the anoying overwrite warnlog :-)"
  if dir(Datei$) <> "" then app.Kill(Datei$)
  printlog "new document"
Call hNewDocument            
  ViewZoom
  Kontext "Massstab"
  Vergroesserung100.Check       
  Massstab.OK
  printlog "create a number of rectangles with different color properties"
   Kontext "SD_Farbleiste"
   sleep 1
    if SD_Farbleiste.Exists then
       Printlog "- Color toolbar will be disabled now"
       ViewToolbarsColorBar
    end if
   printlog "All rectangles are created with same dimensions but using different zoom settings"
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
    printlog "save document"
    hFileSaveAs (Datei$)
  sleep 3
  printlog "changing zoom settings and checking functionality with controlling the color of the selected rectangle"
   for Zaehler = 1 to 6	          
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
                 
      End Select
      Massstab.OK
       sleep 1
      Kontext "DocumentImpress"
      printlog "Mouseclick should hit a rectangle. Color tells us then if it was zoomed correctly."
       gMouseClick 15,15        
                                    
      FormatArea                    
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
  printlog "close document"
 Call hCloseDocument         
endcase
