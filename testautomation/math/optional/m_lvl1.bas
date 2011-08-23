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
'* short description : Math Function Test
'*
'\******************************************************************

global cFilterCountLoad as integer

sub main
   use "math\optional\includes\m_101_.inc"
   use "math\optional\includes\m_105.inc"
   use "math\optional\includes\m_106_.inc"

   printlog Chr(13) + "******* Math - Function - Test ******* "

   cFilterCountLoad = 7

   Call hStatusIn ( "Math","m_lvl1.bas" )

   Call m_101_          'Filemenue
   Call m_105           'Options
   Call m_106_          'Tools Menue

   Call hStatusOut
end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   use "global\tools\includes\optional\t_ui_filters.inc"
   Call GetUseFiles
   gApplication = "MATH"
end sub
