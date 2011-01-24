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
'*  owner : gregor.hartmann@oracle.com
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
