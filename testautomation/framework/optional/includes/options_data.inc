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




