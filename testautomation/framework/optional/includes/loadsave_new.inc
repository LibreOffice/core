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
'* owner : tborsten.bosbach@oracle.com
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

