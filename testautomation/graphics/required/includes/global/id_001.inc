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
'* Owner : wolfram.garten@oracle.com
'*
'* short description :
'*
'\******************************************************************************
sub id_001

    printLog Chr(13) + "---------    id_001    ----------"

    qaerrorlog "#74988# tiFilePassword outcommented due to bug. -FHA"
    call tiFilePassword
    call tiFileSaveAs
    call tiFileReload
    call tiFileVersion
    printlog " File->Send not possible to test, because extrnal prg get's called!"
    call tiFileTemplates
    '   Call tiFileNew instead i call:
    call tmFileNewFromTemplate
    call tmFileOpen
    call tmFileClose
    call tmFileSave
    call tmFileSaveAs
    call tmFileExit

    call tmFileSaveAll
    call tmFileProperties
    call tdFileExport
    call tmFilePrinterSetting
    ' special cases
    '   Call AutoPilot      'inc\desktop\autopilo.inc
    call tmFileExit ' don't test because unpredictable behaviour
end sub
'------------------------------------------------------------------------------
testcase tiFileSaveAs

    dim sFileName as string ' test document & new created doc
'   dim sFormula as string  ' container for formula to create document with
    dim sTemp as string
    dim sFilter (50) as string
    dim i as integer
    dim x as integer
    dim y as integer
    dim Exlist(100) as string ' files to be deleted
    dim sFile as string ' filename to export
    dim sPath as string ' filename to export
    dim sFilterName as string
    dim sFileNameLoaded as string 'gets name of fiel from Dateiname that will be loaded


    if (gApplication = "IMPRESS") then 
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

    sFilter (0) = 0 ' initalize ;-)...
    printlog " used document: graphics\required\input\tbo_alf_.od?"
    sFileName = "graphics\required\input\tbo_alf_." + ExtensionString
    printlog "FileName is " + sFileName
    sFile = "isas_"
    sPath = ConvertPath ( gOfficePath + "user/work/graphics/required/"+gApplication+"\"+ gPlatform) ' this is the export path
    printlog "sPath is " + sPath
    if dir (sPath) = "" then
        app.mkdir (sPath)
    end if

    printlog " deleting all files in output directory: \\user\\work\\graphics\\required\\gApplication gPlatform isas_*"
    if gSamePC = TRUE then 
        GetFileList ( sPath, sFile+"*", Exlist() )
        if KillFileList ( Exlist() ) <> TRUE then
            Warnlog "Couldn't delete all Files in Output-Export-Directory, the followings are still there:"
            for i=1 to ListCount ( Exlist() )
                printlog "  <> " + Exlist(i)
            next i
        end if
    end if

    printlog "load a sample presentation file"
    Call hFileOpen ( ConvertPath(gTesttoolPath + sFileName) )
    WaitSlot (3000)

    printlog "Select between Impress and Draw:"

    if (gApplication = "DRAW") then
        printlog "CASE DRAW: running 4 times save as with different Filters"
        for i = 1 to 4
            printlog "calling Save as Dialog, " & i
            FileSaveAs
            WaitSlot (2000)
            Kontext "SpeichernDlg"
            printlog "Selecting filter according to number of run"
            Dateityp.Select i
            sFilterName = Dateityp.GetSelText
            sFileName = sPath + sFile + (i)
            printlog "Filename is " & sFileName
            printlog "Filtername for this file is " & sFilterName
            Dateiname.SetText (sFileName)
            Speichern.Click
            kontext "AlienWarning"
            if AlienWarning.exists(5) then
                AlienWarning.OK
            end if
            WaitSlot (5000)
        next i
        printlog "closing the sample file"
        fileclose
        WaitSlot (3000)
        kontext "Messagebox"
        if Messagebox.exists then
            printlog "Messagebox about informationloss... :-) that's OK: 'Text in the messagebox: "+Messagebox.GetText+"'"
            Messagebox.YES
        end if

        printlog "Loading the former saved files:"
        for i = 1 to 4 'run it 4 times

            FileOpen
            Kontext "OeffnenDlg"
            WaitSlot (2000)
            DateiAuswahl.TypeKeys "<END>"
            DateiAuswahl.TypeKeys "<HOME>"
            if i <> 1 then
                DateiAuswahl.TypeKeys "<DOWN>", (i-1)
            endif
            sFileNameLoaded = Dateiname.GetSelText
            printlog i & ":File will be loaded: " & sFileNameLoaded
            Oeffnen.Click
            printlog "Closing doc if it gets loaded: " & sFileNameLoaded
            Kontext "DocumentDraw"
            if DocumentDraw.exists(5) then
                Call hCloseDocument
            else
                warnlog "File did not get loaded."
            endif
            WaitSlot (2000)
        next i
    else
        printlog "CASE IMPRESS: running 9 times save as with different Filters"
        for i = 1 to 9
            printlog "calling Save as Dialog, " & i
            FileSaveAs
            WaitSlot (2000)
            Kontext "SpeichernDlg"
            printlog "Selecting filter according to number of run"
            Dateityp.Select i
            sFilterName = Dateityp.GetSelText
            sFileName = sPath + sFile + (i)
            printlog "Filename is " & sFileName
            printlog "Filtername for this file is " & sFilterName
            Dateiname.SetText (sFileName)
            Speichern.Click
            kontext "AlienWarning"
            if AlienWarning.exists(5) then
                AlienWarning.OK
            end if
            WaitSlot (5000)
        next i
        printlog "closing the sample file"
        fileclose
        WaitSlot (3000)
        kontext "Messagebox"
        if Messagebox.exists then
            printlog "Messagebox about informationloss... :-) that's OK: 'Text in the messagebox: "+Messagebox.GetText+"'"
            Messagebox.YES
        end if

        printlog "Loading the former saved files:"
        for i = 1 to 9 'run it 9 times
            FileOpen
            Kontext "OeffnenDlg"
            WaitSlot (2000)
            DateiAuswahl.TypeKeys "<END>"
            DateiAuswahl.TypeKeys "<HOME>"
            if i <> 1 then
                DateiAuswahl.TypeKeys "<DOWN>", (i-1)
            endif
            sFileNameLoaded = Dateiname.GetSelText
            printlog i & ":File will be loaded: " & sFileNameLoaded
            Oeffnen.Click
            printlog "Closing doc if it gets loaded: " & sFileNameLoaded
            printlog "For checking if its open and closing the doc we need to differ Impress and Draw:"
            Kontext "DocumentImpress"
            if DocumentImpress.exists(5) then
                Call hCloseDocument
            else
                Kontext "DocumentDraw"
                if DocumentDraw.exists(5) then
                    Call hCloseDocument
                else
                    warnlog "File did not get loaded."
                endif
            endif

            WaitSlot (2000)
        next i
    endif

endcase 'tiFileSaveAs

'-------------------------------------------------------------------------------

testcase tiFileReload

  Dim DokumentPfad$
  Dim Datei$

    if (gApplication = "IMPRESS") then 
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

  Datei$= (ConvertPath (gOfficePath + "user/work/graphics/required/version1." + ExtensionString))

  if app.dir(ConvertPath (gOfficePath + "user/work/graphics/required/")) = "" then
    app.mkdir (ConvertPath (gOfficePath + "user/work/graphics/required/"))
  end if
printlog " open application"
   Call hNewDocument
   WaitSlot (2000)
printlog " save document ..."
   Call hFileSaveAsKill (Datei$)
   WaitSlot (2000)
printlog " close document "
   Call hCloseDocument

printlog " load document again "
   Call hFileOpen (Datei$)
   WaitSlot (2000)
printlog " type something in document "
   call hTBOtypeInDoc
   WaitSlot (2000)
printlog " File->Reload "
   FileReload
   WaitSlot (2000)
printlog " Messagebox has to come up; say NO"
   Kontext "Messagebox"
   sleep 2
   Messagebox.No
   WaitSlot (2000)

printlog " File->Reload "
   FileReload
   WaitSlot (2000)
   Kontext "Messagebox"
   sleep 2
printlog " Messagebox has to come up; say YES"
   Messagebox.Yes
   WaitSlot (1000)

printlog " close document"
   Call hCloseDocument
printlog " delete document"
   if Dir (Datei$) <> "" AND gSamePC = TRUE then kill Datei$
   
endcase 'tiFileReload

'-------------------------------------------------------------------------------

testcase tiFileVersion

  Dim DokumentPfad$
  Dim Datei$

    if (gApplication = "IMPRESS") then 
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

  Datei$= (ConvertPath (gOfficePath + "user/work/graphics/required/erwin." + ExtensionString))
  'if dir (ConvertPath (gOfficePath + "user/work/graphics/required/")) = "" then
  if dir (Datei$) = "" then app.mkdir (ConvertPath (gOfficePath + "user/work/graphics/required/"))
printlog " open application"
   Call hNewDocument
   WaitSlot (1000)
printlog " save document as ..."
   Call hFileSaveAsKill (Datei$)
   WaitSlot (1000)
printlog " close document"
   Call hCloseDocument

printlog " open document again"
   Call hFileOpen (Datei$)
   WaitSlot (1000)
printlog " type something in document"
   call hTBOtypeInDoc
   WaitSlot (1000)
printlog " File->Versions "
   try
      FileVersions
   catch
      Warnlog "- File / Versions not accessible!"
      goto endsub
   endcatch

   Kontext "Versionen"
   Call DialogTest ( Versionen )
printlog " in Versions dialog press save "
   Speichern.Click
printlog " type a comment"
   Kontext "VersionskommentarEingeben"
   Call DialogTest ( VersionskommentarEingeben )
printlog " exit comment dialog with cancel"
   VersionskommentarEingeben.Cancel
   Kontext "Versionen"
printlog " close version dialog"
   Versionen.Close
printlog " close document"
   Call hCloseDocument
'   if Dir (Datei$) <> "" AND gSamePC = TRUE then kill Datei$

endcase 'tiFileVersion

'-------------------------------------------------------------------------------

testcase tiFilePassword

    goto endsub
   Dim DokumentPfad$
   Dim Datei$
   dim sFileName as string ' test document & new created doc
'   dim sFormula as string  ' container for formula to create document with
   dim e as string
   dim sTemp as string
   dim sFilter (50) as string
   dim i as integer
   dim x as integer
   dim y as integer
   dim Exlist(100) as string ' files to be deleted
   dim sFileIn as string ' filename to import
   dim sFile as string ' filename to export
   dim sPath as string ' and path to export

    if (gApplication = "IMPRESS") then 
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

printlog "  save file to:  graphics\required\output\" + gApplication + "\" + gPlatform + "\"
   sFile = "isp__"                                                     ' this is the filename of the export files
   sPath = ConvertPath ( gOfficePath + "user/work/graphics/required/"+gApplication+"\"+ gPlatform+"\") ' this is the export path
   mkdir (sPath)
   if dir (sPath) = "" then app.mkdir (sPath)
   sFileIn = (sPath + sFile + "." + ExtensionString)
   printlog " deleting all files in output directory: \\graphics\required\output\\impress\\ gPlatform isp_* "
   if gSamePC = TRUE then ' delete export directory
      GetFileList ( sPath, sFile+"*", Exlist() )
      if KillFileList ( Exlist() ) <> TRUE then
         Warnlog "Couldnt delete all Files in Output-Export-Directory, the followings are still there:"
         for i=1 to ListCount ( Exlist() )
            printlog "  <> " + Exlist(i)
         next i
      end if
   end if

printlog " open application "
   Call hNewDocument
   WaitSlot (1000)
printlog " File->SaveAs "
   FileSaveAs
      Kontext "SpeichernDlg"
      Datei$ = ConvertPath ( sFileIn )
      printlog "   will use the file: "+Datei$
      if Dir (Datei$) <> "" then
         kill Datei$
      end if
      printlog " check 'password'"
      Passwort.Check
      printlog " set 'filename'"
      Dateiname.SetText Datei$
      printlog " press 'save'"
      Speichern.Click
   WaitSlot (2000)
   Kontext "Passwort"
      printlog " type password 12345, confirm password: 54321 -> WRONG "
      PasswortName.SetText "12345"
      PasswortBestaetigen.Settext "54321"
      Passwort.OK
   WaitSlot (2000)
   Kontext "Messagebox"
      printlog " messagebox has to show up, say OK, if messagebox doesnt come up -> ERROR "
      if Messagebox.Exists(1) then
         Messagebox.OK
      else
         Warnlog "- Wrong password not recognized"
      end if
   Kontext "Passwort"
      printlog " type password 12345, confirm password: 12345 -> RIGHT "
      PasswortName.SetText "12345"
      PasswortBestaetigen.SetText "12345"
      Passwort.OK
      printlog " close document"
      sleep 2
   FileClose
   sleep 2

   printlog " open document"
   FileOpen
      Kontext "OeffnenDlg"
      WaitSlot (2000)
      Dateiname.SetText Datei$
      Oeffnen.Click
   WaitSlot (2000)
   Kontext "PasswordFileOpen"
      printlog " type password: 34567 -> WRONG "
      PasswortName.SetText "34567"
      try
         PasswordFileOpen.OK
      catch
         Printlog "- Wrong password not accepted"
      endcatch
   Kontext
   printlog " messagebox has to show up, say OK, if messagebox doesn't come up -> ERROR "
      if Not Messagebox.Exists(1) then
         Warnlog "Wrong password while loading not recognized"
      else
         Messagebox.OK
         printlog "Wrong password on loading ok - recogniced"
      end if
   WaitSlot (3000)
   Kontext "PasswordFileOpen"
      printlog " type password: 12345 -> RIGHT"
      PasswortName.SetText "12345"
      PasswordFileOpen.OK
   WaitSlot (5000)
   printlog " File->SaveAs "
   FileSaveAs
      Kontext "SpeichernDlg"
      Dateiname.SetText Datei$
      if (Passwort.IsChecked <> TRUE) then
         Warnlog "Password has to be checked! :-("
      end if
      printlog " press 'save', without changing any checkbox! -> file has to be saved WITH password protection "
      Speichern.Click
      WaitSlot (1000)
   Kontext "Messagebox"
      printlog " (if messagebox comes up, say YES)"
      if Messagebox.Exists(5) then
         Messagebox.Yes
      end if
      printlog " password dialog has to show up!"
   Kontext "Passwort"
      if (Passwort.Exists(5) = FALSE)then
         Warnlog "- Password dialog Didn't pop up after pressing save"
      else
         Kontext "Passwort"
         printlog " type password a12345, confirm password: a12345 -> RIGHT"
         WaitSlot (1000)
         PasswortName.SetText "a12345"
         PasswortBestaetigen.SetText "a12345"
	 Passwort.OK
         WaitSlot (1000)
         printlog " close document"
         FileClose
         WaitSlot (1000)
      end if
   printlog " open document"
   FileOpen
      WaitSlot (2000)
      Kontext "OeffnenDlg"
      Dateiname.SetText Datei$
      Oeffnen.Click
      Kontext "PasswordFileOpen"
      WaitSlot (1000)
      printlog " type password: a12345 -> RIGHT"
      PasswortName.SetText "a12345"
      PasswordFileOpen.OK
      WaitSlot (5000)
   printlog " File->SaveAs "
   FileSaveAs
      Kontext "SpeichernDlg"
      Dateiname.SetText Datei$
      if (Passwort.IsChecked <> TRUE) then
         Warnlog "Password has to be checked! :-("
      end if
      Passwort.UnCheck
      printlog " press 'save', without changing any checkbox! -> file has to be saved WITH password protection "
      Speichern.Click
   Kontext "Messagebox"
      printlog " (if messagebox comes up, say YES)"
      if Messagebox.Exists(5) then Messagebox.Yes
   printlog " password dialog needn't to show up!"
   Kontext "Passwort"
      if (Passwort.Exists(5))then
         Warnlog "- Password dialog didn't pop up after pressing save"
         Kontext "Passwort"
         password.cancel
         printlog " close document "
         FileClose
      else
         printlog " close document"
         hCloseDocument
      end if
endcase 'tiFilePassword

'-------------------------------------------------------------------------------

testcase tiFileTemplates
   Call hNewDocument

   FileTemplatesOrganize
      Kontext "DVVerwalten"
      WaitSlot (1000)
      Call DialogTest (DVVerwalten)
      sleep 1
      PopuplisteLinks.Select 2
      WaitSlot (1000)
      PopuplisteRechts.Select 2
      WaitSlot (1000)
      WelcheDatei.Click
      WaitSlot (1000)
         Kontext "Oeffnendlg"
         Call DialogTest (OeffnenDlg)
         sleep 1
      OeffnenDlg.Cancel
      WaitSlot (2000)
      Kontext "DVVerwalten"
   DVVerwalten.Close

if gtSYSName = "Solaris x86" then
   qaErrorLog "#i62423# - FileTemplatesAddressBookSource outcommented under x86. - FHA"
else
   FileTemplatesAddressBookSource
     Printlog "- AddressBookAssignment"
      kontext "AddressBookSource"
      Call DialogTest (AddressBookSource)
      Administrate.Click
         kontext "AddressSourceAutopilot"
         AddressSourceAutopilot.Cancel
      kontext "AddressBookSource"
      AddressBookSource.Cancel
end if

   FileTemplatesSave
     Printlog "- Save template"
      WaitSlot (1000)
      Kontext "Dokumentvorlagen"
      sleep 1
      Call DialogTest (Dokumentvorlagen)
      WaitSlot (1000)
      Verwalten.Click
         WaitSlot (2000)
         Kontext "DVVerwalten"
         Call DialogTest (DVVerwalten)
         WaitSlot (3000)
      DVVerwalten.Close
      WaitSlot (1000)
      Kontext "Dokumentvorlagen"
      sleep 1
   Dokumentvorlagen.Cancel
   WaitSlot (1000)

   try
      FileTemplatesEdit
         Printlog "- Edit template"
         WaitSlot (1000)
         Kontext "OeffnenDlg"
         sleep 1
         Call DialogTest (OeffnenDlg)
         WaitSlot (1000)
      OeffnenDlg.Cancel
   catch
      Warnlog "- There are problems with File-Template-Save"
   endcatch
   WaitSlot (1000)
   Call hCloseDocument
endcase 'tiFileTemplates

'-------------------------------------------------------------------------------

testcase tmFileNewFromTemplate
   Call hNewDocument
   FileNewFromTemplate
  printlog " Open dialog with File / New / From Template"
  WaitSlot (5000)
  Kontext "TemplateAndDocuments"
  printlog " Check if dialog has been opened"
  if TemplateAndDocuments.NotExists then
     Warnlog "Dialog Templates and Documents are not up!"
     goto endsub
  end if
   Call DialogTest (TemplateAndDocuments)
   WaitSlot (5000)
   try
      TemplateAndDocuments.Cancel
   catch
   endcatch
   WaitSlot (5000)
   Call hCloseDocument
endcase 'tmFileNewFromTemplate

'-------------------------------------------------------------------------------

testcase tmFileOpen
   call hNewDocument
   FileOpen
      Kontext "OeffnenDlg"
      UebergeordneterOrdner.Click
      Standard.Click
      NurLesen.check
      Call DialogTest ( OeffnenDlg )
   OeffnenDlg.Cancel
   WaitSlot (2000)
   Call hCloseDocument
endcase 'tmFileOpen

'-------------------------------------------------------------------------------

testcase tmFileClose
   printlog "- File Close"
   hNewDocument 
   call hTBOtypeInDoc
   WaitSlot (2000)
   FileClose
   Kontext   
   if active.exists (5) then
      printlog "  Ok, active came up: " + active.gettext
      Active.Cancel 
   else
      warnlog "active missing (1)"
   end if
       WaitSlot (2000)

   FileClose
   Kontext
   Active.Yes 
   WaitSlot (2000)

   Kontext "SpeichernDlg"
   Call DialogTest ( SpeichernDlg )
   SpeichernDlg.Cancel
   WaitSlot (2000)

   FileClose
   Kontext
   Active.No
   WaitSlot (2000)
endcase 'tmFileClose

'-------------------------------------------------------------------------------

testcase tmFileSave
   hNewDocument
   call hTBOtypeInDoc

   FileSave
   WaitSlot (2000)
   Kontext "SpeichernDlg"
   UebergeordneterOrdner.click
   Standard.Click
   Call DialogTest (SpeichernDlg)
   Kontext "SpeichernDlg"
   Standard.Click
   SpeichernDlg.Cancel
   WaitSlot (2000)
   Call hCloseDocument
endcase 'tmFileSave

'-------------------------------------------------------------------------------

testcase tmFileSaveAs

   hNewDocument
   WaitSlot (2000)
   call hTBOtypeInDoc

   FileSaveAs
   WaitSlot (2000)
   Kontext "SpeichernDlg"
   if (SpeichernDlg.exists (5) = FALSE) then
      warnlog "FileSaveAs dialog is not visible"
   end if
   WaitSlot (2000)
   Passwort.check
   Passwort.uncheck
   UebergeordneterOrdner.click
   Standard.Click

   NeuerOrdner.click
      kontext "NeuerOrdner"
      neuerordner.cancel
   Kontext "SpeichernDlg"

   Call DialogTest (SpeichernDlg)

   Kontext "SpeichernDlg"
   SpeichernDlg.Cancel
   WaitSlot (2000)
   Call hCloseDocument
endcase 'tmFileSaveAs

'-------------------------------------------------------------------------------

testcase tmFileSaveAll
   printlog "- File SaveAll"

   hNewDocument
   call hTBOtypeInDoc

   Printlog "  open 2. window"
   hNewDocument
   call hTBOtypeInDoc

   Printlog "  call save all"
   FileSaveAll
   Printlog "  cancel 1. save"
   Kontext "SpeichernDlg"
   SpeichernDlg.Cancel

   Printlog "  cancel 2. save"
   WaitSlot (2000)
   Kontext "SpeichernDlg"
   SpeichernDlg.Cancel
   WaitSlot (2000)

   try
      Kontext "SpeichernDlg"
      SpeichernDlg.Cancel
      printlog "smth had been typed in the starting window (just a hint ;-) )"
   catch
      printlog "--------- no other window wants to get saved. :-)"
   endcatch

   WaitSlot (2000)
   Printlog "  hCloseDocument both"
   Call hCloseDocument
   sleep 2
   Printlog "  first closed"
   Call hCloseDocument
   WaitSlot (2000)
   Printlog "  second closed"
   WaitSlot (5000)
endcase 'tmFileSaveAll

'-------------------------------------------------------------------------------

testcase tmFileProperties
   printlog "- File Properties"

   Call hNewDocument
   FileProperties

   Kontext
   active.SetPage TabDokument
   Kontext "TabDokument"
   Call DialogTest ( TabDokument )

   Kontext
   active.SetPage TabDokumentInfo
   Kontext "TabDokumentInfo"
   Call DialogTest ( TabDokumentInfo )

   qaerrorlog "#i95523#"
   'Kontext
   'active.SetPage TabBenutzer
   'Kontext "TabBenutzer"
   'Call DialogTest ( TabBenutzer )
   'Infofelder.Click
   '   Kontext "InfonamenBearbeiten"
   '   Call DialogTest (InfonamenBearbeiten)
   'InfonamenBearbeiten.Cancel

   Kontext
   active.SetPage TabInternet
   Kontext "TabInternet"
   Call DialogTest (TabInternet)
   TabInternet.Cancel

   Call hCloseDocument
endcase 'tmFileProperties

'-------------------------------------------------------------------------------

testcase tmFilePrinterSetting
   printlog "- File Printersettings"
   Call hNewDocument
   WaitSlot (3000)
   FilePrintersettings
   kontext
   if active.exists(5) then
       active.ok
       qaerrorlog "There is no printer available - please install one on your system!"
   end if
   WaitSlot (2000)
   Kontext "DruckerEinrichten"
       Call DialogTest (DruckerEinrichten)
       sleep 2
       DruckerEinrichten.Cancel
       WaitSlot (2000)
   Call hCloseDocument
endcase 'tmFilePrinterSetting

'-------------------------------------------------------------------------------

testcase tdFileExport
    goto endsub
   dim x as integer
printlog " just call File->Export and exit "
   Call hNewDocument
   WaitSlot (3000)
   FileExport
      Kontext "ExportierenDlg"
      Call DialogTest ( ExportierenDlg )
      UebergeordneterOrdner.Click
      Kontext "SpeichernDlg"
      printlog " check number of available export filters. it has to be 17 "
      printlog "+ this is also done in the 'first test', I provide it just as an additional info here"
      printlog "+ testcase: ExFilterCountCompare"
      x=Dateityp.getitemcount
      if x <> 18 then warnlog "the number of filters is not 18, it is: " + x
   SpeichernDlg.Cancel
   WaitSlot (2000)
   Call hCloseDocument
   WaitSlot (2000)
endcase 'tdFileExport

'-------------------------------------------------------------------------------

testcase tmFileExit
   goto endsub
   printlog "- File Close"
   Call hNewDocument
   WaitSlot (2000)
   try
      FileExit "SynchronMode", TRUE
      WaitSlot (2000)
      Kontext
      WaitSlot (2000)
      Kontext "MessageBox"
      if MessageBox.Exists(1) then ' this is the messagebox from the first window!
         Printlog MessageBox.GetText
         try
            MessageBox.OK
         catch
            MessageBox.No
         endcatch
      end if
      Kontext "MessageBox"
      if MessageBox.Exists(1) then
         Warnlog "MsgBox popped up and there were no changes in the document"
         Printlog MessageBox.GetText
         try
            MessageBox.OK
         catch
            MessageBox.No
         endcatch
      end if
   catch
      printlog "this exit is wanted :-)"
   endcatch
   try
      WaitSlot (20000)
      call hStartTheOffice ' from master.inc
   catch
      printlog "catch AGAIN"
   endcatch
   Kontext "SD_Praesentation"
   if SD_Praesentation.exists (2) then
      ViewToolbarsPresentation
   end if
endcase 'tmFileExit
