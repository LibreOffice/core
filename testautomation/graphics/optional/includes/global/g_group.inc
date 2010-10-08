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
' #1 tiGruppierung
'\********************************************************************

testcase tiGruppierung
 Dim PosX 		 'Variable fuer PositionX

    Call hNewDocument
    Call sSelectEmptyLayout								   '/// new document  ///'
  sleep 3
  Kontext "DocumentImpress"
  sleep 2
  hRechteckErstellen (20,20,40,40)				   '/// create rectangle  ///'
  sleep 1
  hRechteckErstellen (60,60,70,10)				   '/// create 2nd rectangle ///'
  sleep 1
  hRechteckErstellen (50,90,70,40)				   '/// create 3rd rectangle ///'
  sleep 1
    Printlog "- Created 3 rectangles for group testing"
  gMouseClick (50,50)                              '/// Put the mouse-marker in the middle of the screen ///'
  EditSelectAll 						   '/// select all rectangles ///'
  if (gApplication = "DRAW") then
     hOpenContextMenu
        sleep(2)
        hMenuSelectNr(12) ' Select "Group"
        sleep(2)
  else
     FormatGroupDraw							   '/// open context menue and group rectangles ///'
  endif
  sleep 1
    Printlog "- Get position and dimensions of elements"
  ContextPositionAndSize								'/// get dimensions of group ///'
  sleep 1
  Kontext
  Active.SetPage TabPositionAndSize
  Kontext "TabPositionAndSize"
  sleep 1
  PosX=PositionX.GetText
  TabPositionAndSize.OK
  sleep 1
  Kontext "DocumentImpress"
  gMouseClick 35,35
  sleep 1
  hTypeKeys "<F3>"							'/// entering group using key "F3" ///'
  sleep 3
   try
      EditCut										'/// cut rectangle out of document ///'
      sleep 1
      Warnlog "- Entering the group, no object within the group should have been selected"
   catch
      Printlog "- entered group, nothing selected"
   endcatch
  gMouseClick (35,35)
  gMouseMove (30,30,60,60)
  sleep 1
   Printlog "- Exit group"
'  DocumentImpress.OpenContextMenu							'/// leave group, compare dimensions ///'
'  sleep 1
'  hMenuSelectNr (13)
  hTypeKeys "<mod1 F3>"							'/// exit group using key "strg F3" ///'
  ' would be better to call the slot TBO!
  ContextPositionAndSize
  sleep 1
  Kontext
  Active.SetPage TabPositionAndSize
  Kontext "TabPositionAndSize"
  sleep 1
   if PosX = PositionX.GetText then
      Warnlog "- No change in position for X axis, even we changed position of 1 object within the group"
      TabPositionAndSize.OK
   else
      Printlog "- Moving within the group works"
      TabPositionAndSize.OK
   end if
  Call hCloseDocument									'/// close document  ///'
endcase
