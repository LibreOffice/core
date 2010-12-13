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
'* owner : thorsten.bosbach@oracle.com
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

