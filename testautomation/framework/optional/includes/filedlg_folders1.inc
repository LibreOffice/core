'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_folders1.inc,v $
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

testcase tFolder1

   '///<h1>Level 1 test: Create new folders from the File Open dialog</h1>
   Dim VerList(50) as String
   dim FULLPATH as string
       FULLPATH = gOfficePath + "user\work\"
   
   '///<ul>    
   printlog " - cleanup the output-dir, kill all directories."
    
   '///+<li>Delete all directories in [home/Office-dir/user/work]</li>
   GetDirList ( ConvertPath ( FULLPATH ), "*" , VerList() )
   KillDirList ( VerList() )

   '///+<li>Either click on &quot;FileOpen&quot; or use the menu to get there</li>
   FileOpen
   
   'This is a hack to prevent trouble when opening 'FileOpen' and the path
   'is not accessible. 
   '///+<li>Handle possible errormessages (path not accessible)</li>
   Kontext "Active"
   if Active.Exists( 1 ) then
      Warnlog "The preset path is invalid, a messagebox has been shown."
      Active.OK()
      endif
      
   '///+<li>Insert the pathname into the filename field</li>
   Kontext "OeffnenDlg"
   Dateiname.SetText( ConvertPath ( FULLPATH ) )
   Oeffnen.Click()
   
   
   '///+<li>click on 'new folder', insert as name 'z' and click OK<br>
   '///+ Verify that the folder was created ( if yes -> kill it )</li>
   printlog " - foldername with only one character"
   CreateValidDirectory( "z" )


   '///<li>click on 'new folder', insert as name '1234' and click OK<br>
   '///+ Verify that the folder was created ( if yes -> kill it )</li>
   printlog " - foldername only with numbers"
   CreateValidDirectory( "1234" )
  

   '///+<li>click on 'new folder', insert as name 'xaxaxaxa' ( 8 characters ) and click OK<br>
   '///+ Verify that the folder was created ( if yes -> kill it )</li>
   printlog " - foldername with 8 characters"
   CreateValidDirectory( "xaxaxaxa" )

  
   '///+<li>click on 'new folder', insert as name 'yxyxyxyx.aaa' ( 8.3 characters ) and click OK<br>
   '///+ Verify that the folder was created ( if yes -> kill it )</li>
   printlog " - foldername with 8.3 characters"
   CreateValidDirectory( "yxyxyxyx.aaa" )   

  
   '///+<li>click on 'new folder', insert as name 'yxyxyxyxyxyx.aaabbb' ( more than 8.3 characters ) and click OK<br>
   '///+ Verify that the folder was created ( if yes -> kill it )</li>
   printlog " - foldername with more than 8.3 characters"
   CreateValidDirectory( "yxyxyxyxyxyx.aaabbb" )

  
   '///+<li>click on 'new folder', insert as name 'hälölüle' ( with special characters - umlauts ) and click OK<br>
   '///+ Verify that the folder was created ( if yes -> kill it )</li>
   printlog " - foldername with special charaters ( umlauts )"
   CreateValidDirectory( "hälölüle" )
  
   
   '///+<li>click on 'new folder', insert as name 'aa bb' ( with spaces in the middle ) and click OK<br>
   '///+ Verify that the folder was created ( if yes -> kill it )</li>
   printlog " - foldername with one whitespace in the middle"
   CreateValidDirectory( "aa bb" )
  

   '///+<li>click on 'new folder', insert as name ' ccdd' ( with leading spaces ) and click OK<br>
   '///+ Verify that the folder was created ( if yes -> kill it )</li>
   printlog " - foldername with leading spaces"
   CreateValidDirectoryCrop( " lead" , "lead" )

   
   '///+<li>click on 'new folder', insert as name 'ddee ' ( with spaces at the end ) and click OK<br>
   '///+ Verify that the folder was created ( if yes -> kill it )</li>
   printlog " - foldername with trailing spaces"
   CreateValidDirectoryCrop( "trail " , "trail" )
   

   '///+<li>click on 'new folder', insert as name 'Here is a dir with spaces' ( with more spaces ) and click OK<br>
   '///+ Verify that the folder was created ( if yes -> kill it )</li>
   printlog " - foldername with more spaces"
   CreateValidDirectory( "here is a dir with spaces" )   

   
   '///+<li>click on 'new folder', insert as name '??++!!' ( with forbidden signes on windows ) and click OK<br>
   '///+ Verify that the folder was created ( if yes -> kill it )</li>
   printlog " - foldername with forbidden signes ( ?+! ) - only on windows they are forbidden"
   CreateInvalidDirectory( "??++!!" )   

	'///+<li>Cancel the File Open dialog</li>
   Kontext "OeffnenDlg"
   OeffnenDlg.Cancel()
   '///</ul>
   
endcase
