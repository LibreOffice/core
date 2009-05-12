'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_folders2.inc,v $
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

testcase tFolder2

   '///<h1>Level 1 test: Create folders with duplicate names</h1>

   dim FULLPATH as string
       FULLPATH = gOfficePath + "user\work\"

   Dim VerList(50) as String
   
   '///<ul>

   '///+<li>Delete all directories in [home/Office-dir/user/work]</li>
   GetDirList ( ConvertPath ( FULLPATH ), "*" , VerList() )
   KillDirList ( VerList() )

   '///+<li>Either click on &quot;FileOpen&quot; or use the menu to get there</li>
   FileOpen

	'///+<li>Change to the local workdirectory (if necessary)</li>
   Kontext "OeffnenDlg"
   Dateiname.SetText( ConvertPath ( FULLPATH ))
   Oeffnen.Click()

   '///+<li>Click on &quot;New Folder&quot; name it &quot;aaa&quot;, repeat the two steps<br>
   '///+ There should be a warning that the folder exist and it should not be possible to create it</li>
   printlog( CHR$(13) + " - Create a folder named 'aaa' twice" )
   CheckDirectoryName( "aaa" ) 

	'///+<li>Click on &quot;New Folder&quot; name it &quot;123&quot;, repeat the two steps<br>
   '///+ There should be a warning that the folder exist and it should not be possible to create it</li>
   printlog( CHR$(13) + " - Create a folder named '123' twice" )
   CheckDirectoryName( "123" )     

	'///+<li>Click on &quot;New Folder&quot; name it &quot;yayayaya&quot;, repeat the two steps<br>
   '///+ There should be a warning that the folder exist and it should not be possible to create it</li>
   printlog( CHR$(13) + " - Create a folder named 'yayayaya' twice" )
   CheckDirectoryName( "yayayaya" )

	'///+<li>Click on &quot;New Folder&quot; name it &quot;ycycycyc.aaa&quot;, repeat the two steps<br>
   '///+ There should be a warning that the folder exist and it should not be possible to create it</li>
   printlog( CHR$(13) + " - Create a folder named 'ycycycyc.aaa' twice" )
   CheckDirectoryName( "ycycycyc.aaa" )
   
   '///+<li>If we hang on the &quot;New Folder&quot; dialog: Close it</li>
   Kontext "NeuerOrdner"
   if NeuerOrdner.Exists() then
      NeuerOrdner.Cancel()
   endif

   '///+<li>Cancel the FileOpen dialog</li>
   Kontext "OeffnenDlg"
   OeffnenDlg.Cancel()
   '///</ul>

endcase
