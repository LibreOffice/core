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
'* short description : Math Update Test
'*
'\*****************************************************************

sub main
   use "math\required\includes\m_001_.inc"
   use "math\required\includes\m_002_.inc"
   use "math\required\includes\m_003_.inc"
   use "math\required\includes\m_004_.inc"
   use "math\required\includes\m_005_.inc"
   use "math\required\includes\m_006_.inc"
   use "math\required\includes\m_007_.inc"   ' help
   use "math\required\includes\m_010_.inc"
   use "math\required\includes\m_020_.inc"   ' toolbars
   use "global\required\includes\g_009.inc"

   Call hStatusIn ( "Math","m_updt.bas" )

   PrintLog Chr(13) + " Math_Update Test "

'/// Duration ca. 30 minutes ///'
   Call M_002_       ' EditMenu
   Call M_003_       ' View-Menu
   Call M_004_       ' Format-Menu
   Call M_005_       ' Tools Menu
   Call M_006_       ' Window-Menu
   Call M_007_       ' Help-Menu
   Call tHelpCheckForUpdates
   Call m_020_       ' toolbars
   Call M_010_       ' Operators
   Call M_001_       ' File Menu
   Call g_printing()
   Call tFileExportAsPDF

   Call hStatusOut
end sub

'----------------------------------------------
sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   use "global\required\includes\g_option.inc"
   use "global\required\includes\g_customize.inc"
   use "global\required\includes\g_001.inc"
   use "global\required\includes\g_009.inc"
   use "global\required\includes\g_printing.inc"
   use "global\tools\includes\optional\t_ui_filters.inc" ' for gMathFilter in m_001_.inc
   use "global\tools\includes\optional\t_docfuncs.inc"
   use "global\tools\includes\optional\t_control_objects.inc"
   gApplication = "MATH"
   Call GetUseFiles
end sub
