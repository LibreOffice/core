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
'* short description : Tools->Options: OpenOffice.org Fonts
'*
'\******************************************************************************

testcase tOOoFontReplacement
   Dim i, il as Integer

   ToolsOptions
   hToolsOptions ( "StarOffice", "Fontreplacement" )

 printlog " - change settings"
   if Anwenden.IsChecked then
      Warnlog "The default for this page is wrong. 'Apply replacement table' has top be unchecked!"
   else
      Anwenden.Check
   end if

   Schriftart.Select 4
   ErsetzenDurch.Select 6
   Uebernehmen.Click
   sleep 1 'gh13
   Schriftart.Select 10
   ErsetzenDurch.Select 5
   Uebernehmen.Click

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

 printlog " - check changes"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Fontreplacement" )

   if Anwenden.IsChecked = FALSE then
      Warnlog "Apply replacement table' => changes not saved!"
   else
      Anwenden.Check
   end if

   il = Liste.GetItemCount
   if il <> 2 then Warnlog "Not all entries are saved!"

   for i=1 to il
      if i=1 then
         Liste.TypeKeys "<Down><Up>"
      else
         Liste.TypeKeys "<Down>"
      end if
      if i=1 then
         if Schriftart.GetSelIndex <> 4 then Warnlog "Font : the first entrie is not correctly saved!"
         if ErsetzenDurch.GetSelIndex <> 6  then Warnlog "Replace with : the first entrie is not correctly saved!"
      end if
      if i=2 then
         if Schriftart.GetSelIndex <> 10 then Warnlog "Font : the first entrie is not correctly saved!"
         if ErsetzenDurch.GetSelIndex <> 5  then Warnlog "Replace with : the first entrie is not correctly saved!"
      end if
   next i

 printlog " - reset to default"
   for i=1 to il
      Liste.TypeKeys "<Down><Up>"
      Loeschen.Click
      sleep 1 'gh13
   next i
   Anwenden.Uncheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check the default"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Fontreplacement" )

   if Anwenden.IsChecked = TRUE then
      Warnlog "Apply replacement table' => changes not saved!"
   else
      Anwenden.Check
   end if
   if Liste.GetItemCount <> 0 then Warnlog "Not all deleted entries are realy deleted!"
   Anwenden.UnCheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)
endcase
