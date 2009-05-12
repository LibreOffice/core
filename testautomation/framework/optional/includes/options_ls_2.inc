'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: options_ls_2.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:15 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* owner : thorsten.bosbach@sun.com
'*
'* short description : (functionality test for load/save group)
'*
'\******************************************************************************

testcase func_LoadSaveGeneral_1

    warnlog( "#i95523# - cannot access custom tabpage on document info dialog" )
    goto endsub
    
  Dim bSave as boolean, bPromptSave as boolean
  Dim sSaveTime as String
  Dim sFilename, sFilebak as String
  Dim i as Integer

   gApplication = "WRITER"

'///load&save/general : functionality test of all settings in 'Save'-group

   sFilename = ConvertPath ( gOfficePath + "user\work\o_save.sxw" )
   sFilebak  = ConvertPath ( gOfficePath + "user\backup\o_save.bak" )

   if app.Dir ( sFilename ) <> "" then app.kill ( sFilename )
   if app.Dir ( sFilebak  ) <> "" then app.kill ( sFilebak  )

'///save : 'document properties before saving'
'///+open a new document
 printlog " - save"
 printlog "    - document properties before saving"
 printlog "      open a new document"
   hNewDocument

'///+insert a short paragraph
   Kontext "DocumentWriter"
   DocumentWriter.TypeKeys "tools/options/load-save/general: save-> edit document properties before saving<Return>"
'///+tools / options / load&save / general
 printlog "      open tools / options / load & save / general"
   ToolsOptions
   hToolsOptions ( "LoadSave", "General" )

 printlog "      check 'document properties before saving'"
'///+save the default setting for 'document properties before saving' and check it
   bSave = DokumenteigenschaftenBearbeiten.IsChecked
   DokumenteigenschaftenBearbeiten.Check

'///+close options dialog with OK
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)
 printlog "      save the document"

'///+file / save as
   FileSaveAs
'///+insert a file name and press 'save'
   Kontext "SpeichernDlg"
   Dateiname.SetText sFilename
   Sleep (1)
   Speichern.Click
   Sleep (1)
   kontext
   'overwrite warning on rerun
   if active.exists(3) then
   active.yes
   endif

 printlog "      activate all tabpages on properties dialog and cancel it"
'///+=> now the properties dialog must be actived
   try
      Kontext
      active.SetPage( TabDokument)
      active.SetPage TabDokumentinfo
      active.SetPage TabBenutzer
      active.SetPage TabInternet
      active.SetPage TabStatistik
      Kontext "TabStatistik"
'///+cancel the properties dialog
      TabStatistik.Cancel
   catch
      Warnlog "Perhaps the document properties aren't active after saving => the dialog come not up after file save!"
   endcatch

'///save : 'always create backup copy'
 printlog "    - always create backup copy"
   Kontext "DocumentWriter"
'///+insert a short paragraph in the writer doc
   DocumentWriter.TypeKeys "tools/options/load-save/general: save-> always create backup copy<Return>"
'///+tools / options / load&save / general
 printlog "      open tools / options / load & save / general"
   ToolsOptions
   hToolsOptions ( "LoadSave", "General" )

'///+uncheck 'document properties before saving'
   if bSave = TRUE then DokumenteigenschaftenBearbeiten.Check else DokumenteigenschaftenBearbeiten.UnCheck
   bSave = Sicherungskopie.IsChecked
'///+check 'always create backup copy'
   Sicherungskopie.Check
'///+close options dialog with OK
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

'///+check if the backup file exists before saving => BUG
   if app.Dir ( sFilebak  ) <> "" then Warnlog "Before saving the document the backup-file exists!"
'///+file / save
   hFileSave
'///+check if the backup file exists after saving => ( ../user/backup/.. )
   if app.Dir ( sFilebak  ) = "" then Warnlog "After saving the document the backup-file doesn't exists => BUG!"

'///save : 'auto save' after 1 min
 printlog "    - autosave : after 1 min, with prompt"
'///+write another short paragraph into the writer document
   Kontext "DocumentWriter"
   DocumentWriter.TypeKeys "tools/options/load-save/general: save-> autosave after 1 min with prompt<Return>"
'///+tools / options / load&save / general
   ToolsOptions
   hToolsOptions ( "LoadSave", "General" )

'///+set 'always create backup copy' to default
   if bSave = TRUE then Sicherungskopie.Check else Sicherungskopie.UnCheck
   bSave = AutomatischSpeichern.IsChecked
'///+check 'auto save' and set the time to 1 minute
  AutomatischSpeichern.Check
   sSaveTime = 15 'debug

   Minuten.SetText "2"

   '///+close options dialog with OK
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (130)

'///+wait about 2 minutes
'///+two 'save file?" dialogs should pop up, one for each document.
'///+Select 'yes' for both dialogs and cancel both file-save dialogs
   for i=1 to 2
      Kontext "Active"
      if Active.Exists (3) then
         Active.Yes
         Kontext "SpeichernDlg"
         if SpeichernDlg.Exists then SpeichernDlg.Cancel
         Sleep (2)
         printlog "Active came up :-) " + i
      else
         printlog "Active didn't come up :-( " + i
      end if
   next i

   '///check the documents
   '///+close the document, there should be a warning that the file has not been saved
   '///+discard the file
   printlog "    - close the document and check the saved data"
   hCloseDocument ' closes the document with 'discard'
   '///+reopen the file and check if all paragraphs are inserted, t
   hFileOpen ( sFilename )
kontext
if active.exists(5) then
warnlog active.gettext
goto endsub
endif
   ' This part compares the content of the file after reload with text entered
   ' during the entire test. It should contain three lines of text.
   printlog( "   - Compare the content of the file with the text entered during the test." )
   printlog( "     First line..." )
   Kontext "DocumentWriter"
   DocumentWriter.TypeKeys "<Up>", 5
   DocumentWriter.TypeKeys "<Home>"
   DocumentWriter.TypeKeys "<Shift End>"
   EditCopy
   WaitSlot()
   if lcase ( GetClipboardText  ) <> "tools/options/load-save/general: save-> edit document properties before saving" then Warnlog "The first sentence is wrong!"

   printlog( "     Second line..." )
   DocumentWriter.TypeKeys "<Down>"
   DocumentWriter.TypeKeys "<Home>"
   DocumentWriter.TypeKeys "<Shift End>"
   EditCopy
   WaitSlot()
   if lcase ( GetClipboardText  ) <> "tools/options/load-save/general: save-> always create backup copy" then Warnlog "The second sentence is wrong!"

   printlog( "     Third line..." )
   DocumentWriter.TypeKeys "<Down>"
   DocumentWriter.TypeKeys "<Home>"
   DocumentWriter.TypeKeys "<Shift End>"
   EditCopy
   WaitSlot()
   if lcase ( GetClipboardText  ) <> "tools/options/load-save/general: save-> autosave after 1 min with prompt" then Warnlog "The third sentence is wrong!"

   '///+close the document
   hCloseDocument

   '///+open the backup file ( ../user/backup/.. ) and check if there are 2 paragraphs (last paragraph should not have been saved)
   printlog "    - load the backup-file"
   hOpenFile ( sFilebak )

   Kontext "Filterauswahl"
   if Filterauswahl.Exists(2) then
      warnlog "Is bugId 107399 present? => Detection of our own fileformar fails when the extension is not the standard extension!"
      Filterauswahl.Close
   else
      Kontext "DocumentWriter"
      DocumentWriter.TypeKeys "<Up>", 5
      DocumentWriter.TypeKeys "<Home>"
      DocumentWriter.TypeKeys "<Shift End>"
      EditCopy

      '///+close the document
      WaitSlot()

      if GetClipboardText <> "" then
         call hCloseDocument
      else
         warnlog "Perhaps the bak-file was not loaded. => Verify BugID 86607!"
      end if
   end if

   '///reset the options to default settings
   printlog " - reset options"
   ToolsOptions
   hToolsOptions ( "LoadSave", "General" )
   AutomatischSpeichern.Check
   Minuten.SetText sSaveTime
   if bSave = TRUE then AutomatischSpeichern.Check else AutomatischSpeichern.UnCheck
'///+close options dialog with OK
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

endcase

' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase func_LoadSaveGeneral_2
  Dim bSave as Boolean
  Dim sFilename, sFilename2, sCharSet as String
  
  const ICWAIT as integer = 1

'///load&save/general : functionality test of all settings in 'Save URLs relativ to'-group

   sFilename = ConvertPath ( gOfficePath + "user\work\o_save_1.html" )
   sFilename2 = ConvertPath ( gOfficePath + "user\work\o_save_2.html" )

   if app.dir ( sFilename ) <> "" then app.kill ( sFilename )
   if app.dir ( sFilename2 ) <> "" then app.kill ( sFilename2 )

 printlog " - save"
 printlog "    - save URL relative to"

'///if you test a StarSuite ( asian language ) you have to change the encoding for HTML-export to 'UTF8' ( load&save/HTML compatibility )
   if bAsianLan = TRUE then
      sCharSet = GetHTMLCharSet
      if SetHTMLCharSetToUTF8 = FALSE then
         Warnlog "The test cannot find the UTF8 Character Set for HTML-Export. The test can have many errors at saving HTML-Files!"
      else
         printlog "The Character Set for HTML-Export is now Unicode UTF8!"
      end if
   end if

'///open a new HTML document
   gApplication = "HTML"
   hNewDocument

'///+ insert a graphic out of the internal gallery ( [Officepath]/share/gallery/photos/desert.jpg )
   if gNetzInst = TRUE then
      hGrafikEinfuegen ( ConvertPath ( gNetzOfficePath + "share\gallery\photos\desert1.jpg" ) )
   else
      hGrafikEinfuegen ( ConvertPath ( gOfficePath + "share\gallery\photos\desert1.jpg" ) )
   end if

'///+open tools/options/load&save/general
 printlog "    - relative to filesystem"
   ToolsOptions
   hToolsOptions ( "LoadSave", "General" )
   
'///+check 'relativ to filesystem'
   bSave = URLimDateisystem.IsChecked
   URLimDateisystem.Check
   Kontext "ExtrasOptionenDlg"
'///+close options dialog with OK
   ExtrasOptionenDlg.OK
   Sleep (2)

 printlog "      save the HTML file"
'///+save the HTML-doc at ../user/work/..
   hFileSaveAs ( sFilename )

'///+check if the path for the graphic is 'relativ' in the source code
 printlog "      check the relativ path in source code of HTML file"
   Call URLGraphicCheck ( TRUE, sFilename )

'///+open tools/options/load&save/general
 printlog "    - relative to filesystem ( unchecked )"
   ToolsOptions
   hToolsOptions ( "LoadSave", "General" )
   
'///+uncheck 'relativ to filesystem'
   URLimDateisystem.UnCheck
'///+close options dialog with OK
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+save the HTML-doc at ../user/work/.. with another name
 printlog "      save the HTML file"
   hFileSaveAs ( sFilename2 )

'///+check if the path for the graphic is 'not relativ' in the source code
 printlog "      check the hard coded path in source code of HTML file"
   Call URLGraphicCheck ( FALSE, sFilename2 )

'///+close the document
   hCloseDocument

'///+reset 'relativ to file system' to default and close the options dialog with OK
 printlog "    - reset the options"
   ToolsOptions
   hToolsOptions ( "LoadSave", "General" )
   
   if bSave = TRUE then URLimDateisystem.Check else URLimDateisystem.UnCheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   printlog "No test for 'relative to internet'"

'///<FONT COLOR="#ff0000">No test for 'relative to internet'</FONT>
  if bAsianLan = TRUE then
     printlog Chr(13 ) + "Reset the Character Set back to default : " + sCharSet
     SetHTMLCharSet ( sCharSet )
  end if

   gApplication = "WRITER"
endcase


