'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: m_001_.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 11:51:23 $
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
'* Owner : thorsten.bosbach@sun.com
'*
'* short description :
'*
'\*****************************************************************

 global cActFilter as string ' used filter for save
 Const cActFilterExt = ".odf" ' used filter for save (extension)

' "StarMath 5.0" ".smf"
'   Datei$ = convertpath( gOfficePath + "user\work\killme" + cActFilterExt)
'   Datei$ = convertpath( gOfficePath + "user\work\Erwin2"+cActFilterExt)
'   Datei = convertpath(gofficepath + "user\work\test"+cActFilterExt)

sub M_001_
   Printlog Chr(13) + "--  File Menu  m_001_ --"

   cActFilter = gMathFilter

   Call tmFileNew
  Call tmFileOpen
'   Call Autopilot               'in Inc\desktop\autopilo.inc
  Call tmFileClose
  Call tmFileSave
  Call tmFileSaveAs
  Call tmFileSaveAll
   Call tmFileReload
   Call tmFileVersions
   Call tmExportAsPDF
   Call tmFileProperties
   Call tmFilePrint
  Call tmFilePrinterSetting
   Call tmFilePassword
end sub

testcase tmFileNew
'/// open application ///'
   Call hNewDocument
   '/// File->New->Templates and Documents ///'
   FileNewFromTemplate
   sleep 3
   Kontext "TemplateAndDocuments"
   if TemplateAndDocuments.exists (5) then
      try
         '/// klick button 'Organize' ///'
         Organize.click
         kontext "DVVerwalten"
         if DVVerwalten.exists (5) then
            Call DialogTest (DVVerwalten)
            sleep 1
            '/// close dialog 'Template Management' ///'
            DVVerwalten.close
         else
            warnlog "DVVerwalten didn't exist :-("
         endif
      catch
          warnlog "error ;-) - 1"
      endcatch
   else
      warnlog "templates and dokuments didn't exist :-("
   endif
   try
     sleep 1
     kontext "TemplateAndDocuments"
     sleep 1
     Call DialogTest ( TemplateAndDocuments )
   catch
     warnlog "no dialogtest possibele :-("
   endcatch

' TBO i don't get in deep here, because it is not math specific
'     and IMHO it doesn't make that sense here!
'     should be enoug in writer...
   '/// close dialog 'Templates and Documents' ///'
   TemplateAndDocuments.Cancel
'/// close application ///'
        Call hCloseDocument
endcase

testcase tmFilePassword
   Dim DokumentPfad$
   Dim Datei$
'/// open application ///'
while (getDocumentCount > 0)
hCloseDocument
wend
   hNewDocument
   '/// type a formula ///'
   SchreibenInMathdok "a over b"

   printlog "'/// - save doc as "+cActFilter+" with passwd - ///'"
   FileSaveAs
   Kontext "SpeichernDlg"
   Datei$ = convertpath( gOfficePath + "user\work\killme" + cActFilterExt)
   if Dir (Datei$) <> "" then kill Datei$
   printlog "  Testfile: "+Datei$
   'Dateityp.Select (cActFilter)
   Passwort.Check
   Dateiname.SetText Datei$
   Speichern.Click
   Kontext "Active"
   if Active.Exists (5) then
   ' TBO: ??? what will be ??!
      Printlog "(1/5) " + Active.GetText
      Active.Yes
   end if

   printlog "'/// Try wrong passwd confirmation ///'"
   Sleep 2
   Kontext "PasswordFileSave"
   sleep (2)
   try
      Password.SetText "12345"
      PasswordConfirm.Settext "54321"
      PasswordFileSave.OK
      Sleep 2
   catch
      warnlog "had to ask for passwd!!!!"
   endcatch
   Kontext
   if (Active.Exists (5) = FALSE ) then
      Warnlog "Wrong passwordinput not detected"
   else
      Printlog "(2/5) " + Active.GetText
      Active.OK
   end if

   printlog "'/// Now do it right ///'"
   Kontext "PasswordFileSave"
   try
      Password.SetText "12345"
      PasswordConfirm.Settext "12345"
      PasswordFileSave.OK
   catch
      warnlog "had to ask for passwd!!!!"
   endcatch

   Sleep 2
   printlog "'/// Gotcha & Close file/window ///'"
   FileClose
   sleep 1

   printlog "'/// load again with wrong passwd ///'"
   Call hFileOpen Datei$
   sleep 1
   Kontext "PasswordFileOpen"
   try
      Passwortname.SetText "34567"
      PasswordFileOpen.OK
   catch
      warnlog "had to ask for passwd!!!!"
   endcatch
   Kontext "Active"
   if Active.Exists(5) then
         Printlog "(3/5) " + Active.GetText
      Active.Ok
   else
      Warnlog "Wrong passwordinput not detected by loading the document"
   end if

   printlog "'///load now with right passwd ///'"
   Sleep 3
   'Call hDateiOeffnen Datei$
   Kontext "PasswordFileOpen"
   try
      Passwortname.SetText "12345"
      PasswordFileOpen.OK
   catch
      warnlog "had to ask for passwd!!!!"
   endcatch
   Sleep 2

   printlog "'/// - save doc with new name under 6.... (has to be automagical selected by loading this doc!) w/o passwd - ///'"
   Datei$ = convertpath( gOfficePath + "user\work\Erwin2"+cActFilterExt)
   if Dir (Datei$) <> "" then kill Datei$
   FileSaveAs
      Kontext "SpeichernDlg"
      Dateiname.SetText Datei$
      if (Passwort.IsChecked <> TRUE) then
         Warnlog "#i36015# Password has to be checked! :-("
         Passwort.Check
         printlog "will be forced checked no!"
      endif
      Speichern.Click
   Kontext "Messagebox"
      '/// (if messagebox comes up, say YES) ///'
      if Messagebox.Exists(2) then
         Messagebox.Yes
      endif
      '/// password dialog has to show up! ///'
   Kontext "PasswordFileSave"
      if (PasswordFileSave.Exists(5) = FALSE)then
         Warnlog "- Password dialog Didn't popped up after pressing save"
      else
         Kontext "PasswordFileSave"
         '/// type password "a12345", confirm password: "a12345" -> RIGHT ///'
         Password.SetText "a12345"
         PasswordConfirm.SetText "a12345"
         PasswordFileSave.OK
         sleep 2
         '/// close document ///'
         FileClose
         sleep 1
      endif
   '/// open document ///'
   FileOpen
      sleep 1
      Kontext "OeffnenDlg"
      Dateiname.SetText Datei$
      Oeffnen.Click
      Kontext "PasswordFileOpen"
      sleep 1
      '/// type password: "a12345" -> RIGHT ///'
      PasswortName.SetText "a12345"
      PasswordFileOpen.OK
      sleep 5
   '/// File->SaveAs ///'
   FileSaveAs
      Kontext "SpeichernDlg"
      Dateiname.SetText Datei$
      if (Passwort.IsChecked <> TRUE) then
         Warnlog "#i36015# Password has to be checked! :-("
         Passwort.Check
         printlog "will be forced checked no!"
      endif
      Passwort.UnCheck
      '/// press 'save', without changing any checkbox! -> file has to be saved WITH password protection ///'
      Speichern.Click
   Kontext "Messagebox"
      '/// (if messagebox comes up, say YES) ///'
      if Messagebox.Exists(2) then Messagebox.Yes
   '/// password dialog needn't to show up! ///'
   Kontext "PasswordFileSave"
      if (PasswordFileSave.Exists(5))then
         Warnlog "- Password dialog neeedn't popped up after pressing save"
         Kontext "PasswordFileSave"
         passwordFileSave.cancel
         '/// close document ///'
         FileClose
      else
         '/// close document ///'
         hCloseDocument
      end if
endcase

testcase tmFileReload
   Dim Datei as String
   dim sFilterName as string
   dim sFilter as string
   dim sFilterExt() as string

   Datei = convertpath(gofficepath + "user\work\test"+cActFilterExt)
   if Dir (Datei) <> "" then kill (Datei)

   ' make sure saving in latest math filter works, checking global filtername and if necessary change it
   sFilter = hGetUIFiltername("math8")

   '/// open application ///'
   hNewDocument
   '/// type a formula ///'
   call hTBOtypeInDoc
   '/// File->Save As ///'
   FileSaveAs
   Kontext "SpeichernDlg"
   '/// select the actual default file format ///'
   Dateityp.Select (hFindFilterPosition( sFilter ))
      '/// type a file name ///'
      Dateiname.SetText Datei
      '/// click button 'save' ///'
      Speichern.Click
      Kontext "Active"
      if Active.Exists(2) then Active.Yes
   sleep 3
   '/// File->Close ///'
   FileClose
   Kontext
   if MessageBox.Exists (1) then
      WarnLog "After Saving and Closing the following MessageBox appears: " + MessageBox.GetText
      MessageBox.Yes
   end if

   '/// open just saved file ///'
   Call hFileOpen Datei
   '/// type a formula ///'
   call hTBOtypeInDoc
   sleep (3)
   '/// File->Reload ///'
   FileReload
   sleep (3)
   Kontext
   if Active.Exists (1) then
      PrintLog "Say no to: " + Active.GetText
      '/// say NO to active ///'
      active.No
   else
      WarnLog "Missing smth to say no to! "
   end if

   Sleep 3
   '/// File->Reload ///'
   FileReload
   Kontext
   if Active.Exists (1) then
      PrintLog "Said yes to: " + Active.GetText
      '/// say YES to active ///'
      active.Yes
   end if
   '/// close application ///'
   Call hCloseDocument
endcase

testcase tmFileVersions
   Dim Datei as String

   Datei = ConvertPath (gOfficePath & "user/work/test"+cActFilterExt)
   if app.Dir (Datei) <> "" then app.kill (Datei)

   '/// open application ///'
   Call hNewDocument
   sleep 2
   '/// type a formula ///'
   SchreibenInMathdok "a over b"
   '/// save file ///'
   Call hFileSaveAs(Datei)
   Sleep 3
   try
      '/// File->Versions ///'
      FileVersions
   catch
      Warnlog "- File / Versions not accessible!"
      goto endsub
   endcatch

   Kontext "Versionen"
   Call DialogTest ( Versionen )
   '/// click button 'Save New Version' ///'
   Speichern.Click
   Kontext "VersionskommentarEingeben"
   Call DialogTest ( VersionskommentarEingeben )
   '/// cancel dialog 'Insert Version Comment' ///'
   VersionskommentarEingeben.Cancel
   Kontext "Versionen"
   '/// close dialog 'Version of ...' ///'
   Versionen.Close
   '/// close application ///'
   Call hCloseDocument
   if app.Dir (Datei) <> "" then app.kill Datei
endcase

testcase tmFilePrint
'/// open application ///'
   Call hNewDocument
   sleep 3
   '/// File->Print ///'
   FilePrint
   kontext
   if active.exists(2) then
       active.ok
       qaerrorlog "There is no printer available - please install one on your system!"
   endif
   sleep 2
   Kontext "DruckenDlg"
   Call DialogTest (DruckenDlg)
   '/// cancel dialog 'Print' ///'
   DruckenDlg.Cancel
   sleep 2
'/// close application ///'
   Call hCloseDocument
endcase

'-----------------------------------------------------------
'*******************   I D  dito   *************************
'-----------------------------------------------------------

testcase tmFileOpen
'/// open application ///'
   call hNewDocument
   '/// File->Open ///'
   FileOpen
      Kontext "OeffnenDlg"
      '/// klick button 'Up one level' ///'
      UebergeordneterOrdner.Click
      '/// klick button 'Default Directory' ///'
      Standard.Click
      '/// check checkbox 'Read Only' ///'
      NurLesen.check
      Call DialogTest ( OeffnenDlg )
      '/// cancel dialog 'Open' ///'
   OeffnenDlg.Cancel
   Sleep 2
   '/// close application ///'
   Call hCloseDocument
endcase

testcase tmFileClose
'/// open application ///'
   hNewDocument    ' just for the records: i open ONE document
   '/// type a formula ///'
   call hTBOtypeInDoc
   sleep (2)
   '/// File->Close ///'
   FileClose
   Kontext         ' expecting 'modified, do you want to close?'
   if active.exists (5) then
      printlog "  ok, active came up: " + active.gettext
      '/// say NO to active about 'modified stuff :-)' ///'
      Active.Cancel   ' no, not this time
   else
      warnlog "active missing (1)"
   endif
   sleep (2)
   '/// File->Close ///'
   FileClose
   Kontext
   '/// say YES to active about 'modified stuff :-)' ///'
   Active.Yes      ' but now - records: this document is closed
   sleep (2)
   Kontext "SpeichernDlg"
   Call DialogTest ( SpeichernDlg )
   '/// cancel dialog 'Save As' ///'
   SpeichernDlg.Cancel
   sleep (2)

   '/// File->Close ///'
   FileClose       ' now the office gets closed! (if there were no modifications!)
   Kontext
   if active.exists (5) then Active.No
   sleep (2)
endcase

testcase tmFileSave
'/// open application ///'
   hNewDocument
   '/// type a formula ///'
   call hTBOtypeInDoc
   '/// File->Save ///'
   FileSave
   sleep 2
   Kontext "SpeichernDlg"
   UebergeordneterOrdner.click
   Standard.Click
'      NeuerOrdner.Click
'      DateiAuswahl.TypeKeys "Hallo<Return>"
'      DateiLoeschen
'      Kontext "Messagebox"
'      MessageBox.Yes
   Call DialogTest (SpeichernDlg)
   '/// cancel dialog 'Save' ///'
   SpeichernDlg.Cancel
    sleep 2
   '/// close application ///'
   Call hCloseDocument
endcase

testcase tmFileSaveAs
'/// open application ///'
   hNewDocument
   sleep (2)
   '/// type a formula ///'
   call hTBOtypeInDoc
   '/// File->Save As ///'
   FileSaveAs
   sleep 1
   Kontext "SpeichernDlg"
   if (SpeichernDlg.exists (5) = FALSE) then
      warnlog "error :-("
   endif
   sleep 2
   '/// check ceckbox 'save with Password' ///'
   Passwort.check
   '/// UNcheck ceckbox 'save with Password' ///'
   Passwort.uncheck
   '/// click button 'up one level' ///'
   UebergeordneterOrdner.click
   '/// click button 'default directory' ///'
   Standard.Click
   '/// click button 'create new directory' ///'
   NeuerOrdner.click
      kontext "NeuerOrdner"
      '/// cancel dialog 'create new folder' ///'
      NeuerOrdner.cancel
   Kontext "SpeichernDlg"
   Call DialogTest (SpeichernDlg)
   '/// cancel dialog 'Save As' ///'
   SpeichernDlg.Cancel
   sleep (2)
   '/// close application ///'
   Call hCloseDocument
endcase

testcase tmFileSaveAll
'/// open application ///'
   hNewDocument
   '/// type a formula ///'
   call hTBOtypeInDoc
   Printlog " '/// open 2. window ///"
   hNewDocument
   '/// type a formula ///'
   call hTBOtypeInDoc

   Printlog "  call save all"
   '/// File->Save All ///'
   FileSaveAll
   Printlog "  cancel 1. save"
   Kontext "SpeichernDlg"
   if SpeichernDlg.exists(5) then
       '/// cancel dialog 'save as' ///'
       SpeichernDlg.Cancel
   else
       qaErrorlog "First File save dialog did not show up."
   endif
   Printlog "  cancel 2. save"
   Kontext "SpeichernDlg"
   if SpeichernDlg.exists(5) then
       '/// cancel dialog 'save as' ///'
       SpeichernDlg.Cancel
   else
       qaErrorlog "Second File save dialog did not show up."
   endif

   try
      Kontext "SpeichernDlg"
      SpeichernDlg.Cancel
      printlog "smth had been typed in the starting window (just a hint ;-) )"
   catch
      printlog "--------- no other window want's to get saved. :-)"
   endcatch

   Sleep 2
   Printlog "  hCloseDocument both"
   '/// close document ///'
   Call hCloseDocument
   sleep 2
   Printlog "  first closed"
   '/// close application ///'
   try
   Call hCloseDocument
   catch
   printlog "any catching?"
   endcatch
   sleep 1
   Printlog "  second closed"
   sleep (5)
endcase

testcase tmExportAsPDF
   dim sPDF as string
   dim sTemp as string

   sPDF = "PDF - Portable Document Format (.pdf)"

   '/// open application ///'
   Call hNewDocument

   '/// click the button 'Export Directly as PDF' on the Functionbar ///'
   kontext "Standardbar"
   Sleep 5
   ExportAsPDF.click
      ' the 'Export as PDF' dialog has to come up, with the only 'File type' 'PDF - Portable Document Format (.pdf)'
      kontext "ExportierenDlg"
      sTemp = Dateityp.GetSelText
      if (sTemp <> sPDF) then
         Warnlog "filter for PDF export is missing :-( should: '" + sPDF + "'; is: '" +  + "'"
      endif
      '///+ - set Textbox 'File name' to "abc" ///'
      Dateiname.SetText "abc"
      '///+ - use the cursor keys in the filebrowser ///'
      DateiAuswahl.TypeKeys "<home><down>"
      '///+ - click on the button 'Up one level' ///'
      UebergeordneterOrdner.Click
      '///+ - click on the button 'Create New Directory' ///'
      NeuerOrdner.Click
         kontext "NeuerOrdner"
         '///+ - - in the dialog $Foldername set textfield 'New' to 'abc' ///'
         OrdnerName.SetText "abc"
         '///+ - - cancel dialog $Foldername ///'
         NeuerOrdner.cancel
      kontext "ExportierenDlg"
      '///+ - click on the button 'Default Directory' ///'
      Standard.Click
      '///+ - cancel dialog ///'
      Kontext "ExportierenDlg"
      ExportierenDlg.Cancel
      
   '/// close application ///'
   Call hCloseDocument
endcase

testcase tmFileProperties
'/// open application ///'
   Call hNewDocument
   '/// File->Properties... ///'
   FileProperties

   Kontext
   '/// select tabpage 'General' ///'
   active.SetPage TabDokument
   Kontext "TabDokument"
   Call DialogTest ( TabDokument )

   Kontext
   '/// select tabpage 'Description' ///'
   active.SetPage TabDokumentInfo
   Kontext "TabDokumentInfo"
   Call DialogTest ( TabDokumentInfo )

   Kontext
   '/// select tabpage 'Internet' ///'
   active.SetPage TabInternet
   Kontext "TabInternet"
   Call DialogTest (TabInternet)
   TabInternet.Cancel
   '/// close application ///'
   Call hCloseDocument

   qaerrorlog "#i95523# Custom Properties is not controlable by VCL TestTool Application"
   goto endsub

   Kontext
   '/// select tabpage 'User defined' ///'
   active.SetPage TabBenutzer
   Kontext "TabBenutzer"
   Call DialogTest ( TabBenutzer )
   '/// click button 'info fields' ///'
   Infofelder.Click
      Kontext "InfonamenBearbeiten"
      Call DialogTest (InfonamenBearbeiten)
   '/// close dialog 'edit field names' ///'
   InfoNamenBearbeiten.Cancel
endcase

testcase tmFilePrinterSetting
'/// open application ///'
   Call hNewDocument
   sleep 3
   '/// File->Printer Settings ///'
   FilePrintersettings
   kontext
   if active.exists(2) then
       active.ok
       qaerrorlog "There is no printer available - please install one on your system!"
   endif
   Kontext "DruckerEinrichten"
   Call DialogTest (DruckerEinrichten)
   sleep 2
   '/// cancel dialog 'printer setup' ///'
   DruckerEinrichten.Cancel
   sleep 2
   '/// close application ///'
   Call hCloseDocument
endcase

sub hTBOtypeInDoc
   SchreibenInMathdok "a over b"
end sub

