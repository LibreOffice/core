'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_folders3.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:14 $
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
'* owner : joerg.skottke@sun.com
'*
'* short description : check the internal file dialog ( 1. part )
'*
'\******************************************************************************

testcase tFolder3

   '///<h1>Level 1 test: Create folders with lower and upper cases in the local file system</h1>
   
   dim FULLPATH as string
       FULLPATH = gOfficePath + "user\work\"
       
   dim FOLDERNAME_U as string
       FOLDERNAME_U = "AAABBB"
       
   dim FOLDERNAME_M as string
       FOLDERNAME_M = "AAaabbBB"
       
   '///<ul>

   '///+<li>Click &quot;FileOpen&quot;</li>
   FileOpen
   
   '///+<li>Make sure we are in the local working directory, go there if necessary</li>
   printlog( " - change to working directory" )
   Kontext "OeffnenDlg"
   Dateiname.SetText( ConvertPath ( FULLPATH ))
   Oeffnen.Click()
   
   '///+<li>Click on the &quot;New Folder&quot; icon, insert an uppercase foldername<br>
   '///+ Verify on filesystem level that the name is indeed in uppercase</li>
   printlog( " - foldername with upper case characters only" )
   call CreateValidDirectory( FOLDERNAME_U )
   
   '///+<li>Click on the &quot;New Folder&quot; icon, insert a mixed case foldername<br>
   '///+ Verify on filesystem level that the name is indeed in mixed case</li>
   printlog( " - foldername with mixed case characters" )
   call CreateValidDirectory( FOLDERNAME_M )
   
   '///+<li>Cancel FileOpen dialog</li>
   kontext "OeffnenDlg"
   OeffnenDlg.Cancel()
   '///</ul>
   
endcase


