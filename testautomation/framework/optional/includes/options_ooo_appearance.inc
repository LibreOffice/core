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
'* short description : Tools->Options: OpenOffice.org Appearance
'*
'\******************************************************************************

testcase tOOoAppearance
  Dim iSchemes as Integer
  Dim sSchemeNames (10) as String
  Dim sCurScheme as String, sCurScheme2 as String
  Dim bIsError as Boolean
  dim i as integer


 printlog " - save settings"
'tools / options / staroffice / appearance
   ToolsOptions
   hToolsOptions ( "StarOffice", "Appearance" )

'get the number of schemes
   iSchemes = Scheme.GetItemCount
   sCurScheme = Scheme.GetSelText
   if iSchemes > 10 then Warnlog "There are more than 10 schemes, please update the test for it!"
'get the name of schemes
   for i=1 to iSchemes
      ListAppend ( sSchemeNames(), Scheme.GetItemText (i) )
   next i

 printlog " - change settings"
'insert a new scheme ( press Save, insert TT-Scheme as name, and close all dialogs with OK )
   Save.Click
   Kontext "SaveScheme"
   SchemeName.SetText "TT-Scheme"
   try
      SaveScheme.OK()
   catch
      warnlog( "The newly created scheme cannot be saved (ok is disabled) -> #i26913")
      bIsError=true
      SaveScheme.Cancel()
   endcatch

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

 printlog " - check settings"
'tools / options / staroffice / appearance
   ToolsOptions
   hToolsOptions ( "StarOffice", "Appearance" )

'try to select the TT-Scheme
   try
      Scheme.Select "TT-Scheme"
      bIsError = FALSE
   catch
      if Scheme.GetItemCount <> iSchemes + 1 then
         Warnlog "The new scheme 'TT-Scheme' is not saved!"
      else
         Warnlog "The new scheme 'TT-Scheme' has another name!"
      end if
      bIsError = TRUE
   endcatch

   printlog " - delete the new scheme"

   'select 'TT-Scheme'
   if bIsError = FALSE then
      try
         Scheme.Select "TT-Scheme"
         'press delete and click 'no'
         Delete.Click()
         Kontext "Active"
         Active.No()
      catch
         warnlog( "The scheme does not exist in the scheme-selector listbox")
      endcatch

      Kontext "TabAppearance"
      try
         'press delete and click 'yes'
         Scheme.Select "TT-Scheme"
         Delete.Click

         Kontext "Active"
         Active.Yes
      catch
         Warnlog "Perhaps the Scheme is deleted! But the messagebox was left with 'NO'!"
      endcatch

'select the default-entry
      Sleep (1)
      Kontext "TabAppearance"
      sCurScheme2 = Scheme.GetSelText

 printlog " - press OK for options dialog"
'press OK for options dialog
      Kontext "ExtrasOptionenDlg"
      ExtrasOptionenDlg.OK
      Sleep (3)

 printlog " - check if the scheme is deleted and the saved base state is current"
      ToolsOptions
      hToolsOptions ( "StarOffice", "Appearance" )

      if Scheme.GetSelText <> sCurScheme2 then
         Warnlog "The last selected entry after deleting is not selected after restart the options dialog!"
      end if
      if iSchemes <> Scheme.GetItemCount then Warnlog "There are not " + iSchemes + " items in the scheme list! There are " + Scheme.GetItemCount
      for i = 1 to Scheme.GetItemCount
         if Scheme.GetItemText(i) <> sSchemeNames(i) then
            Warnlog "The " + i + " entry is not the same -> should : '" + sSchemeNames(i) + "' is '" + Scheme.GetItemText(i) + "'"
         end if
      next i

 printlog " - set the selected scheme to default"
      Scheme.Select sCurScheme

 printlog " - press OK at the options dialog"
      Kontext "ExtrasOptionenDlg"
      ExtrasOptionenDlg.OK
      Sleep (3)

 printlog " - check if the default setting is active, after deleting a scheme and a restart of the options dialog"
      ToolsOptions
      hToolsOptions ( "StarOffice", "Appearance" )

      if Scheme.GetSelText <> sCurScheme then
         Warnlog "The default scheme is not selected!"
      end if

   end if

   printlog " - press OK at the options dialog"
      Kontext "ExtrasOptionenDlg"
      ExtrasOptionenDlg.OK
      Sleep (3)
endcase


