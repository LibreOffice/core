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
' **
' ** short description : check the internal file dialog (Folders/Navigation)
' **
'\******************************************************************************

sub main

   use "framework\optional\includes\filedlg_folders1.inc"
   use "framework\optional\includes\filedlg_folders2.inc"
   use "framework\optional\includes\filedlg_folders3.inc"
   use "framework\optional\includes\filedlg_folders4.inc"
   use "framework\optional\includes\filedlg_folders5.inc"
   use "framework\optional\includes\filedlg_folders6.inc"
   use "framework\optional\includes\filedlg_folders7.inc"   
   
   Call hStatusIn ( "framework", "f_filedlg_folder_navigation.bas" )

   call tFolder1 ' create folders with different names ('new folder'-button)
   call tFolder2 ' create folders with same names ( 'new folder'-button )
   call tFolder3 ' create folders with lower and upper cases in
                 '  local-file-system ('new folder'-button)

   call tUpOneLevel1 ' click 'up one level' 10 times and check the results
   call tUpOneLevel2 ' go up one level with '..' 10 times and check the results
   call tUpOneLevel3 ' jump to the root with '/' or '\' and check the result
   call tUpOneLevel4 ' check the items under 'up one level'-button

   Call hStatusOut

end sub

' ****************************************

sub LoadIncludeFiles

   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"

   use "global\tools\includes\optional\t_stringtools.inc"

   use "framework\optional\includes\filedlg_tools.inc"
   use "framework\tools\includes\fileoperations.inc"
   
   gApplication = "WRITER"
   Call GetUseFiles

end sub


