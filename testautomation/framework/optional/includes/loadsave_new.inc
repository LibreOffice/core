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
'* short description : global functionality - load/save documents
'*
'\***************************************************************************

testcase NewCloseDok
   PrintLog "    open and close all documenttypes"
   Call hNewCloseDocument ("WRITER")
   Call hNewCloseDocument ("CALC")
   Call hNewCloseDocument ("IMPRESS")
   Call hNewCloseDocument ("DRAW")
   Call hNewCloseDocument ("MATH")
   Call hNewCloseDocument ("HTML")
   Call hNewCloseDocument ("MASTERDOCUMENT")
endcase

testcase NewDok
  Dim i%
   PrintLog "   open all applications and close all one by one"
   Call NewDocument ("WRITER")
   Call NewDocument ("CALC")
   Call NewDocument ("IMPRESS")
   Call NewDocument ("DRAW")
   Call NewDocument ("MATH")
   Call NewDocument ("HTML")
   Call NewDocument ("MASTERDOCUMENT")

   PrintLog "   Close all documents ( file/close )"
   for i%=1 to 7
      Kontext "Navigator"
      if Navigator.Exists then Navigator.Close
      Sleep (1)
      FileClose
      kontext "Active"
      if Active.Exists(10) then
         try
         printlog "" + i% + " " +active.gettext
            Active.No
         catch
            try
                 Active.Click ( 202 )
            catch
                 warnlog "Error on hitting 'No' button"
                 sleep 5
            endcatch
         endcatch
      end if
   next i%
endcase

sub hNewCloseDocument ( sApplikation as String )
  PrintLog "- " + sApplikation
   gApplication = sApplikation

   try
      hNewDocument
      if gApplication = "DRAW" OR gApplication = "IMPRESS" then
         hTextrahmenErstellen ( "Dummy text", 20, 20, 50, 50 )
      else
         hTypeKeys "Dummy text<Return>"
      end if
      Sleep 2
      Kontext "Navigator"
      if Navigator.Exists then Navigator.Close
      hCloseDocument
   catch
      Warnlog gApplication + ": a error is occurred."
      Exceptlog
   endcatch
end sub

sub NewDocument ( sApplikation as String )
 PrintLog "- " + sApplikation
   gApplication = sApplikation
   try
      hNewDocument
      if gApplication = "DRAW" OR gApplication = "IMPRESS" then
         hTextrahmenerstellen ( "Dummy text",20,20,50,50 )
      else
         Call hTypeKeys "Dummy text<Return>"
      end if
      Sleep 2
   catch
      Warnlog gApplication + ": a error is occurred."
      Exceptlog
   endcatch
end sub

