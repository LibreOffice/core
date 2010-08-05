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
