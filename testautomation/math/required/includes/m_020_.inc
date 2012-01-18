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

sub m_020_
   printlog Chr(13)+ "--  Toolbar m_020_  --"
   call tMainToolbar
end sub

testcase tMainToolbar
   '/// open application ///'
   Call hNewDocument
      '/// type something into document ///'
      SchreibenInMathdok( "a over b" )
      kontext "Toolbar"
      '/// click buttons on Main Toolbar ///'
      '/// click 'Zoom in' ///'
      hClickButton( Vergroessern )
      '/// click 'Zoom out' ///'
      hClickButton( Verkleinern )
      '/// click 'Zoom 100%' ///'
      hClickButton( ZoomHundertProzent )
      '/// click 'Zoom inEntire Formula' ///'
      hClickButton( GanzeFormel )
      '/// click 'Refresh' ///'
      hClickButton( FormelAktualisieren )
      '/// click 'Formula Cursor' ///'
      hClickButton( FormulaCursor )
      '/// click 'Symbols' ///'
      hClickButton( FormelSymbole )
      kontext "SymboleMath"
      if ( SymboleMath.exists( 1 ) ) then
         '/// close dialog 'Symbols' ///'
         hCloseDialog( SymboleMath, "close" )
      else
         warnlog( "Dialog <SymboleMath> did not open" )
      endif
      kontext "Toolbar"
      '/// click 'Formula Cursor' ///'
      hClickButton( FormulaCursor )
   '/// close application ///'
   Call hCloseDocument
endcase
