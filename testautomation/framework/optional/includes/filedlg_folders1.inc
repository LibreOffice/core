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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : check the internal file dialog ( 1. part )
'*
'\******************************************************************************

testcase tFolder1

   Dim VerList(50) as String
   dim FULLPATH as string
       FULLPATH = gOfficePath + "user\work\"
   
   printlog " - cleanup the output-dir, kill all directories."
    
   GetDirList ( ConvertPath ( FULLPATH ), "*" , VerList() )
   KillDirList ( VerList() )

   FileOpen
   
   'This is a hack to prevent trouble when opening 'FileOpen' and the path
   'is not accessible. 
   Kontext "Active"
   if Active.Exists( 1 ) then
      Warnlog "The preset path is invalid, a messagebox has been shown."
      Active.OK()
      endif
      
   Kontext "OeffnenDlg"
   Dateiname.SetText( ConvertPath ( FULLPATH ) )
   Oeffnen.Click()
   
   
   printlog " - foldername with only one character"
   CreateValidDirectory( "z" )


   printlog " - foldername only with numbers"
   CreateValidDirectory( "1234" )
  

   printlog " - foldername with 8 characters"
   CreateValidDirectory( "xaxaxaxa" )

  
   printlog " - foldername with 8.3 characters"
   CreateValidDirectory( "yxyxyxyx.aaa" )   

  
   printlog " - foldername with more than 8.3 characters"
   CreateValidDirectory( "yxyxyxyxyxyx.aaabbb" )

  
   printlog " - foldername with special charaters ( umlauts )"
   CreateValidDirectory( "hälölüle" )
  
   
   printlog " - foldername with one whitespace in the middle"
   CreateValidDirectory( "aa bb" )
  

   printlog " - foldername with leading spaces"
   CreateValidDirectoryCrop( " lead" , "lead" )

   
   printlog " - foldername with trailing spaces"
   CreateValidDirectoryCrop( "trail " , "trail" )
   

   printlog " - foldername with more spaces"
   CreateValidDirectory( "here is a dir with spaces" )   

   
   printlog " - foldername with forbidden signes ( ?+! ) - only on windows they are forbidden"
   CreateInvalidDirectory( "??++!!" )   

   Kontext "OeffnenDlg"
   OeffnenDlg.Cancel()
   
endcase
