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
'* Owner : wolfram.garten@sun.com
'*
'* short description : Impress Required Test Library (5)
'*
'\*****************************************************************

testcase tiFormatModifyLayout

'/// open application ///'
   Call  hNewDocument
'/// Impress: Format->Modify Layout ///'
'/// Draw   : kontext menu: Slide-> Modify Slide (always disabled :-( ?///'
   FormatPage ' 27046 SID_MODIFYPAGE
      Kontext "SeitenLayout"
      DialogTest ( SeitenLayout )
      sleep 1
'/// cancel dialog 'Page Setup' ///'
      SeitenLayout.Cancel
'/// close application ///'
   Call hCloseDocument
endcase


