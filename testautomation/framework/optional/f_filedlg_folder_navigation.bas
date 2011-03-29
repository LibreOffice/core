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
' **
' ** owner : gregor.hartmann@oracle.com
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


