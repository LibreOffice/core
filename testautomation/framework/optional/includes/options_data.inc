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
'* short description : general option test ( datasource - group )
'*
'\******************************************************************************

testcase tDatasourceConnections
  Dim lsTimeout ( 20 ) as String
  Dim lbPool ( 20 ) as Boolean
  Dim i as Integer, iCounter as Integer
  Dim bSave as Boolean
  Dim sDiv as String
  Dim iDummy as Single

   iCounter = 9          ' sorry only hard coded the number of entries, it gives no way to get the real count ( GetText crashes )
qaerrorLog "GetTextCrasjhes??? TBO"


   hNewDocument
   ToolsOptions
   hToolsOptions ( "Datasources", "Connections" )

 printlog " - save raw data"
   bSave = ConnectionPoolingEnabled.IsChecked
   ConnectionPoolingEnabled.Check
   DriverList.TypeKeys "<Up>", 20
   for i=1 to iCounter
      if i <> 1 then DriverList.TypeKeys "<Down>"
      lbPool (i) = EnablePoolingForThisDriver.IsChecked
      if lbPool (i) = TRUE then
         lsTimeout (i) = Timeout.GetText
      else
         EnablePoolingForThisDriver.Check
         lsTimeout (i) = Timeout.GetText
      end if
   next i

 printlog " - change all settings"
   DriverList.TypeKeys "<Up>", 20
   for i=1 to iCounter
      if i <> 1 then DriverList.TypeKeys "<Down>"
      sDiv = str ( i/2 )
      if Instr ( sDiv, "." ) <> 0 or Instr ( sDiv, "," ) <> 0  then
         EnablePoolingForThisDriver.Check
         Timeout.SetText "4" + i
      else
         EnablePoolingForThisDriver.Uncheck
      end if
      Sleep (1)
   next i
   if bSave = TRUE then ConnectionPoolingEnabled.UnCheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)
   hCloseDocument

 printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

 printlog " - check all changes"
   ToolsOptions
   hToolsOptions ( "Datasources", "Connections" )

   if ConnectionPoolingEnabled.IsChecked = bSave then Warnlog "Connection pooling enabled => changes not saved!"
   ConnectionPoolingEnabled.Check

   DriverList.TypeKeys "<Up>", 20
   for i=1 to iCounter
      if i <> 1 then DriverList.TypeKeys "<Down>"
      sDiv = str ( i/2 )
      if Instr ( sDiv, "." ) <> 0 or Instr ( sDiv, "," ) <> 0  then
         if EnablePoolingForThisDriver.IsChecked <> TRUE then Warnlog "Entry " + i + " : enable pooling for this driver => changes not saved!"
         EnablePoolingForThisDriver.Check
         if Timeout.GetText <> "4" + i then Warnlog "Entry " + i + " : timeout => changes not saved!"
      else
         if EnablePoolingForThisDriver.Ischecked = TRUE then Warnlog "Entry " + i + " : enable pooling for this driver => changes not saved!"
      end if
      Sleep (1)
   next i

 printlog " - make second changes"
   DriverList.TypeKeys "<Up>", 20
   for i=1 to iCounter
      if i <> 1 then DriverList.TypeKeys "<Down>"
      sDiv = str ( i/2 )
      if Instr ( sDiv, "." ) <> 0 or Instr ( sDiv, "," ) <> 0  then
         EnablePoolingForThisDriver.Uncheck
      else
         EnablePoolingForThisDriver.Check
         if i<>10 then
            Timeout.SetText "6" + i
         else
            Timeout.SetText "540"
         end if
      end if
      Sleep (1)
   next i

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check all changes"
   ToolsOptions
   hToolsOptions ( "Datasources", "Connections" )

   DriverList.TypeKeys "<Up>", 20
   for i=1 to iCounter
      if i <> 1 then DriverList.TypeKeys "<Down>"
      sDiv = str ( i/2 )
      if Instr ( sDiv, "." ) <> 0 or Instr ( sDiv, "," ) <> 0  then
         if EnablePoolingForThisDriver.Ischecked = TRUE then Warnlog "Entry " + i + " : enable pooling for this driver => changes not saved!"
      else
         if EnablePoolingForThisDriver.IsChecked <> TRUE then Warnlog "Entry " + i + " : enable pooling for this driver => changes not saved!"
         EnablePoolingForThisDriver.Check
         if i<>10 then
            if Timeout.GetText <> "6" + i then Warnlog "Entry " + i + " : timeout => changes not saved!"
         else
            if Timeout.GetText <> "540" then Warnlog "Entry " + i + " : timeout => changes not saved!"
         end if

      end if
      Sleep (1)
   next i

 printlog " - reset all settings"
   DriverList.TypeKeys "<Up>", 20
   for i=1 to iCounter
      if i <> 1 then DriverList.TypeKeys "<Down>"
      EnablePoolingForThisDriver.Check
      Timeout.SetText lsTimeout (i)
      if lbPool (i) = TRUE then EnablePoolingForThisDriver.Check else EnablePoolingForThisDriver.UnCheck
   next i
   if bSave = TRUE then ConnectionPoolingEnabled.Check else ConnectionPoolingEnabled.UnCheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check the raw data"
   ToolsOptions
   hToolsOptions ( "Datasources", "Connections" )

   ConnectionPoolingEnabled.check
   DriverList.TypeKeys "<Up>", 20
   for i=1 to iCounter
      if i <> 1 then DriverList.TypeKeys "<Down>"
      if EnablePoolingForThisDriver.IsChecked <> lbPool (i) then Warnlog "Entry " + i + " : enable pooling for this driver => changes not saved!"
      EnablePoolingForThisDriver.Check
      if Timeout.GetText <> lsTimeout (i) then Warnlog "Entry " + i + " : timeout => changes not saved!"
      if lbPool (i) = TRUE then EnablePoolingForThisDriver.Check else EnablePoolingForThisDriver.UnCheck
      Sleep (1)
   next i
   ConnectionPoolingEnabled.unCheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

endcase




