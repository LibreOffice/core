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
'* owner : oliver.craemer@oracle.com
'*
'* short description : Chart resource test - Menu WINDOW
'*
'************************************************************************
'*
' #1 tWindowNewWindowAndClose
'*
'\************************************************************************

testcase tWindowNewWindowAndClose
'///<u><b>Test Window::New Window and Window::Close Window</b></u>

'///<ul>
   '///+<li>Load simple test document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   sleep(2)
   '///+<li>Invoke Window::New Window</li>
   WindowNewWindow
   '///+<li>Invoke Window::Close Window</li>
   WindowCloseWindow

   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   '///+<li>Close document</li>///
   Call hCloseDocument
'///</ul>
endcase

