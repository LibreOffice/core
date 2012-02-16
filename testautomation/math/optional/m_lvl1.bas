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
