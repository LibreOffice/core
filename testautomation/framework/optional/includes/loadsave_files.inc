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
'* short description : File / New-Saving-Loading-Saving-Loading
'*
'\***********************************************************************

sub l1_lasp

   PrintLog Chr(13)+"******  File / New-Saving-Loading-Saving-Loading  ******"
   gApplication = "WRITER"
   Call tFileLoadSave
   gApplication = "CALC"
   Call tFileLoadSave
   gApplication = "IMPRESS"
   Call tFileLoadSave
   gApplication = "DRAW"
   Call tFileLoadSave
   gApplication = "MATH"
   Call tFileLoadSave
   gApplication = "HTML"
   Call tFileLoadSave
   gApplication = "MASTERDOCUMENT"
   Call tFileLoadSave

end sub

' --------------------------------------------------------------------------------

testcase tFileLoadSave

   Dim Datei1$ 
   Dim Datei2$ 
   Dim DokName1$ 
   Dim DokName2$
   Dim DefaultPfad$ 
   Dim AppsFilter$ 
   Dim sApp$ 
   Dim DefFilter$
   Dim vExtension(10) as string

   DefFilter$ = hGetFilter( "current" )
   
    if gApplication = "HTML" then
        'hGetFilternameExtension is able to read more than one extension entry
        vExtension() = hGetFilternameExtension("writer_web_HTML")
        DokName1$ = "ls_test1" & "." & vExtension(0) 
        Dokname2$ = "ls_test2" & "." & vExtension(0)
    else   
        DokName1$ = "ls_test1" & hGetSuffix( "current" )
        DokName2$ = "ls_test2" & hGetSuffix( "current" )
    end if

   PrintLog Chr(13)+"** " + gApplication

   Call hNewDocument

   if gApplication = "WRITER" OR gApplication = "HTML" OR gApplication = "MASTERDOCUMENT" then
      Kontext "DocumentWriter"
      DocumentWriter.TypeKeys gApplication + "   =>   Load - Save - Load - Save<Return>"
      DocumentWriter.TypeKeys "- first saving / loading<Return>"
   end if

   if gApplication = "CALC" then
      Kontext "DocumentCalc"
      DocumentCalc.TypeKeys "Testdokument   =>   Load - Save - Load - Save<Return>"
      DocumentCalc.TypeKeys "- first saving / loading<Return>"
   end if

   if gApplication = "MATH" then
      SchreibenInMathDok "a over 2 = 3 over d"
      Sleep (2)
   end if

   if gApplication = "DRAW" OR gApplication = "IMPRESS" then
      hRechteckErstellen ( 30, 30, 60, 60 )
   end if

   DefaultPfad$ = ConvertPath ( gOfficePath + "user\work\" )
   Datei1$ = ConvertPath ( DefaultPfad$+DokName1 )
   Datei2$ = ConvertPath ( DefaultPfad$+DokName2 )

   if app.Dir (Datei1$) <> "" then app.kill Datei1$
   if app.Dir (Datei2$) <> "" then app.kill Datei2$
   Sleep 2

   PrintLog "- first saving"
   hFileSaveAsKill ( Datei1$ )

   PrintLog "- change and save"
   if gApplication = "WRITER" OR gApplication = "HTML" OR gApplication = "MASTERDOCUMENT" then
      Kontext "DocumentWriter"
      DocumentWriter.TypeKeys "- file/save<Return>"
   end if

   if gApplication = "CALC" then
      Kontext "DocumentCalc"
      DocumentCalc.TypeKeys "- file/save<Return>"
   end if

   if gApplication = "MATH" then
      SchreibenInMathDok "file save : 4 over 5 = 3 over d"
      Sleep (2)
   end if

   if gApplication = "DRAW" OR gApplication = "IMPRESS" then
      hRechteckErstellen ( 10, 10, 20, 20 )
   end if

   hFileSave()

   PrintLog "- close"
   Call hCloseDocument()

   PrintLog "- first loading"
   hFileOpen ( Datei1$ )
   sleep( 2 )

   if gApplication = "WRITER" OR gApplication = "HTML" OR gApplication = "MASTERDOCUMENT" then
      Kontext "DocumentWriter"
      gMouseClick ( 50, 50 )
      Kontext "DocumentWriter"
      DocumentWriter.TypeKeys "<Down>", 4
      DocumentWriter.TypeKeys "<Return>- second saving / loading"
   end if

   if gApplication = "CALC" then
      Kontext "DocumentCalc"
      gMouseClick ( 50, 50 )
      DocumentCalc.TypeKeys "<Down>", 4
      DocumentCalc.TypeKeys "<Return>- second saving / loading"
   end if

   if gApplication = "MATH" then
      SchreibenInMathDok "file save : 4 over 5 = 3 over d"
      Sleep (2)
   end if

   if gApplication = "DRAW" OR gApplication = "IMPRESS" then
      hRechteckErstellen ( 65, 65, 75, 75 )
   end if

   PrintLog "- second saving"
   hFileSaveAsKill ( Datei2$ )
   Call hCloseDocument

   PrintLog "- second loading"
   hFileOpen ( Datei2$ )
   Kontext "Navigator"
   sleep (1)
   if Navigator.Exists(5) then Navigator.Close
   Call hCloseDocument

endcase

