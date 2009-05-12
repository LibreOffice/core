'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: m_020_.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 11:51:23 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* Owner : thorsten.bosbach@sun.com
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
      hTBOtypeInDoc
      kontext "Toolbar"
      '/// click buttons on Main Toolbar ///'
      '/// click 'Zoom in' ///'
      Vergroessern.Click
      '/// click 'Zoom out' ///'
      Verkleinern.Click
      '/// click 'Zoom 100%' ///'
      ZoomHundertProzent.Click
      '/// click 'Zoom inEntire Formula' ///'
      GanzeFormel.Click
      '/// click 'Refresh' ///'
      FormelAktualisieren.Click
      '/// click 'Formula Cursor' ///'
      FormulaCursor.Click
      '/// click 'Symbols' ///'
      FormelSymbole.Click
      kontext "SymboleMath"
      if (SymboleMath.exists <> TRUE) then
         warnlog "Symbols Dialog didn't came up :-("
      else
         '/// close dialog 'Symbols' ///'
         SymboleMath.close
      endif
      kontext "Toolbar"
      '/// click 'Formula Cursor' ///'
      FormulaCursor.Click
   '/// close application ///'
   Call hCloseDocument
endcase
