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
'*  short description : Test Load/Save MS-Office settings page
'*
'\******************************************************************************

testcase tLoadSaveMSOffice

   Dim lbSave1 ( 9 ) as Boolean
   Dim lbSave2 ( 9 ) as Boolean
   Dim i as Integer
   printlog "If you get errors in this test, please check the settings."
   printlog "This test can only run without an error, when all checkboxes are checked or all are unchecked."


   hNewDocument
   ToolsOptions
   hToolsOptions ( "LoadSave", "MicrosoftOffice" )

 printlog " - save old settings"
   Auswahl.TypeKeys "<PageUp>"
   for i=1 to 4
      lbSave1(i) = Auswahl.IsChecked (1)
      lbSave2(i) = Auswahl.IsChecked (2)
      Auswahl.TypeKeys "<Down>"
   next i

 
   Auswahl.TypeKeys "<PageUp>"
   for i=1 to 4
   
      printlog( " - invert setting: " + i )
   
      if lbSave1(i) = TRUE then
         if lbSave2(i) = FALSE then Auswahl.TypeKeys ("<Space>")               ' 2. checkboxes must be checked
         if lbSave2(i) = TRUE  then Auswahl.TypeKeys ("<Space><Space><Space>") ' 2. checkboxes must be unchecked
      else
         if lbSave2(i) = FALSE then Auswahl.TypeKeys ("<Space>")               ' 2. checkboxes must be checked
         if lbSave2(i) = TRUE  then Auswahl.TypeKeys ("<Space><Space><Space>") ' 2. checkboxes must be unchecked
      end if
      
      Auswahl.TypeKeys ("<Down>")
      
   next i

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   hCloseDocument

 printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

 printlog " - check inverting"
   ToolsOptions
   hToolsOptions ( "LoadSave", "MicrosoftOffice" )

   Auswahl.TypeKeys( "<PageUp>" )
   for i=1 to 4
      if Auswahl.IsChecked(1) = lbSave1(i) then warnlog "Entry " + i + ": state of 1. checkbox isn't saved"
      if Auswahl.IsChecked(2) = lbSave2(i) then warnlog "Entry " + i + ": state of 2. checkbox isn't saved"
      Auswahl.TypeKeys( "<Down>" )
   next i

 printlog " - reset to saved settings"
   Auswahl.TypeKeys "<PageUp>"
   for i=1 to 4

      if lbSave1(i) = TRUE then
         if lbSave2(i) = FALSE then Auswahl.TypeKeys ("<Space><Space><Space>") ' 2. checkboxes must be unchecked
         if lbSave2(i) = TRUE  then Auswahl.TypeKeys ("<Space>")               ' 2. checkboxes must be checked
      else
         if lbSave2(i) = FALSE then Auswahl.TypeKeys ("<Space><Space><Space>") ' 2. checkboxes must be unchecked
         if lbSave2(i) = TRUE  then Auswahl.TypeKeys ("<Space>")               ' 2. checkboxes must be checked
      end if
      
      Auswahl.TypeKeys ("<Down>")
      
   next i


   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check default settings"
   ToolsOptions
   hToolsOptions ( "LoadSave", "MicrosoftOffice" )

   for i=1 to 4
      if Auswahl.IsChecked(1) <> lbSave1(i) then warnlog "Entry " + i + ": state of 1. checkbox isn't saved"
      if Auswahl.IsChecked(2) <> lbSave2(i) then warnlog "Entry " + i + ": state of 2. checkbox isn't saved"
      Auswahl.TypeKeys "<Down>"
   next i

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   
endcase
