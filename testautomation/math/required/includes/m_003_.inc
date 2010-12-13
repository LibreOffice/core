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
'* Owner : thorsten.bosbach@oracle.com
'*
'* short description :
'*
'\*****************************************************************

sub M_003_
   printlog Chr(13)+"--  File_View  m_003_ --"

   Call tViewZoom
   Call tViewUpdate
   Call tViewAutomaticRepaint
   Call tViewBars
   ' ^ View->Status bar
   Call tViewOperators
   Call tViewEntireDesktop
end sub

testcase tViewZoom
'/// open application ///'
   Call hNewDocument
   '/// type something into the document ///'
   Call SchreibenInMathdok "What's the matter?"
   sleep 2
   UseBindings
  '/// View -> Zoom ///'
   ViewZoom
   Kontext "Massstab"
      DialogTest ( Massstab )
      '/// check checkbox 'Entire Page' ///'
      GanzeSeite.check
      '/// check checkbox 'Page Width' ///'
      Seitenbreite.check
      '/// check checkbox 'Optmal' ///'
      Optimal.check
      '/// check checkbox 'Variable' ///'
      VergroesserungStufenlos.check
   '/// cancel dialog 'Zoom' ///'
   Massstab.Cancel
  '///View -> Zoom In ///
   ViewZoomIn
   sleep (2)
  '///View -> Zoom Out ///
   ViewZoomAll
   sleep (2)
  '///View -> Show All ///
   ViewShowAll
   sleep (2)
   '/// close application ///'
   Call  hCloseDocument
endcase

testcase tViewUpdate
'/// open application ///'
   Call hNewDocument
   '/// type something into the document ///'
   Call SchreibenInMathdok "a sup b"
   '/// Edit->Paste ///'
   EditPaste
   '/// View->Update ///'
   ViewUpdate
   sleep (2)
   '/// close application ///'
   Call  hCloseDocument
endcase

testcase tViewAutomaticRepaint
'/// open application ///'
   Call hNewDocument
   '/// type something into the document ///'
   Call SchreibenInMathdok "a sup b"
   '/// View->AutoUpdate Display ///'
   ViewAutoUpdateDisplay
   '/// Edit->Paste ///'
   EditPaste
   sleep (2)
   '/// View->AutoUpdate Display ///'
   ViewAutoUpdateDisplay
   sleep (2)
   '/// close application ///'
   Call  hCloseDocument
endcase

testcase tViewBars
'/// open application ///'
   Call hNewDocument
   '/// type something into the document ///'
   SchreibenInMathdok "a sup b"
   try
   '/// View->Toolbars->Function bar ///'
   ViewToolbarsStandard
   sleep (2)
   '/// View->Toolbars->Function bar ///'
   ViewToolbarsStandard
   sleep (1)
   '/// View->Toolbars->Main Toolbar ///'
   ViewToolbarsTools
   sleep (2)
   '/// View->Toolbars->Main Toolbar ///'
   ViewToolbarsTools
   sleep (1)
   '/// View->Status bar ///'
   ViewToolbarsStatusbar
   sleep (1)
   '/// View->Status bar ///'
   ViewToolbarsStatusbar
   sleep (1)
   catch
   		warnlog "toolbar slots not accessible"
   endcatch
   '/// close application ///'
   Call  hCloseDocument
endcase

testcase tViewOperators
'/// open application ///'
   Call hNewDocument
   '/// type something into the document ///'
   Call SchreibenInMathdok "a over b"
   '/// View->Selection ///'
   ViewSelection
   sleep (2)
   '/// View->Selection ///'
   ViewSelection
   Kontext "OperatorenMath"
   if (NOT OperatorenMath.Exists (2)) then 
      ViewSelection
      printlog "View Selection wasn't visible :-( now is!"
   endif
   Call DialogTest ( OperatorenMath )
   '/// close application ///'
   Call hCloseDocument
endcase

testcase tViewEntireDesktop
'/// open application ///'
   Call hNewDocument
   '/// type something into the document ///'
   Call SchreibenInMathdok "(a over ba) over (a + 1/2 * b )"
   '/// View->Full Screen ///'
   ViewFullScreen
   sleep 2
   '/// View->Full Screen ///'
   ViewFullScreen
   sleep 1
   '/// close application ///'
   Call  hCloseDocument
endcase


