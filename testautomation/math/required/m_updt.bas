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
