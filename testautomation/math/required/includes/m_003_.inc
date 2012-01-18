'encoding UTF-8  Do not remove or change this line!
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


