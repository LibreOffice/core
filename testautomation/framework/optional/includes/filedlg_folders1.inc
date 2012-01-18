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
