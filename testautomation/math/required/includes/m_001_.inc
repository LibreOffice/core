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
'* Owner : thorsten.bosbach@oracle.com
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
    Call tExportAsPDFButton
    Call tmFileProperties
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
    hFileCloseAll()
    hNewDocument
    '/// type a formula ///'
    SchreibenInMathdok "a over b" : WaitSlot()

    printlog "'/// - save doc as "+cActFilter+" with passwd - ///'"
    hUseAsyncSlot( "FileSaveAs" )

    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then
        Datei$ = convertpath( gOfficePath + "user\work\killme" + cActFilterExt)
        hDeleteFile( Datei$ )

        printlog "  Testfile: "+Datei$
        Passwort.Check
        Dateiname.SetText Datei$
        Speichern.Click
        Kontext "Active"
        if Active.Exists (5) then
            ' TBO: ??? what will be ??!
            Printlog "(1/5) " + Active.GetText
            hCloseDialog( Active, "yes" )
        end if
    else
        warnlog( "File Save As dialog did not open" )
    endif

    printlog "'/// Try wrong passwd confirmation ///'"
    Kontext "PasswordFileSave"
    if ( PasswordFileSave.exists( 2 ) ) then
        Password.SetText "12345"
        PasswordConfirm.Settext "54321"
       hCloseDialog( PasswordFileSave, "ok" )
    else
        warnlog( "Password dialog on File Save is missing" )
    endif

    Kontext
    if ( Active.Exists ( 5 ) ) then
        Printlog "(2/5) " + Active.GetText
        hCloseDialog( Active, "ok" )
    else
        Warnlog "Wrong passwordinput not detected"
    end if

    printlog "'/// Now do it right ///'"
    Kontext "PasswordFileSave"
    if ( PasswordFileSave.exists( 3 ) ) then
        Password.SetText "12345"
        PasswordConfirm.Settext "12345"
        hCloseDialog( PasswordFileSave, "ok" )
    else
        warnlog( "Password dialog on File Save is missing" )
    endif

    printlog "'/// Gotcha & Close file/window ///'"
    hUseAsyncSlot( "FileClose" )

    printlog "'/// load again with wrong passwd ///'"
    Call hFileOpen( Datei$ )

    Kontext "PasswordFileOpen"
    if ( PasswordFileOpen.exists( 3 ) ) then
        Passwortname.SetText "34567"
        hCloseDialog( PasswordFileOpen, "ok" )
    else
        warnlog( "Password dialog on File Open is missing" )
    endif

    Kontext "Active"
    if Active.Exists(5) then
        Printlog "(3/5) " + Active.GetText
        Active.Ok
    else
        Warnlog "Wrong passwordinput not detected by loading the document"
    end if

    printlog "'///load now with right passwd ///'"
    Kontext "PasswordFileOpen"
    if ( PasswordFileOpen.exists( 3 ) ) then
        Passwortname.SetText "12345"
        hCloseDialog( PasswordFileOpen, "ok" )
    else
        warnlog( "Password dialog on File Open is missing" )
    endif

    printlog "'/// - save doc with new name under 6.... (has to be automagical selected by loading this doc!) w/o passwd - ///'"
    Datei$ = convertpath( gOfficePath + "user\work\Erwin2"+cActFilterExt)
    hDeleteFile( Datei$ )

    hUseAsyncSlot( "FileSaveAs" )

    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then
        Dateiname.SetText Datei$
        if (Passwort.IsChecked <> TRUE) then
            '#i36015# 
            Warnlog "Password has to be checked! :-("
            Passwort.Check
            printlog "will be forced checked no!"
        endif
        Speichern.Click
    else
        warnlog( "File Save dialog is missing" )
    endif

    Kontext "Messagebox"
    '/// (if messagebox comes up, say YES) ///'
    if Messagebox.Exists(2) then
        Messagebox.Yes
    endif
    '/// password dialog has to show up! ///'
    Kontext "PasswordFileSave"
    if ( PasswordFileSave.exists( 2 ) ) then
        '/// type password "a12345", confirm password: "a12345" -> RIGHT ///'
        Password.SetText "a12345"
        PasswordConfirm.SetText "a12345"
        hCloseDialog( PasswordFileSave, "ok" )
    else
        warnlog( "Password dialog on FileSave is missing" )
    endif
    '/// close document ///'
    hUseAsyncSlot( "FileClose" )
    '/// open document ///'
    hUseAsyncSlot( "FileOpen" )

    Kontext "OeffnenDlg"
    if ( OeffnenDlg.exists( 2 ) ) then
        Dateiname.SetText Datei$
        Oeffnen.Click
        Kontext "PasswordFileOpen"
        if ( PasswordFileOpen.exists( 2 ) ) then
            '/// type password: "a12345" -> RIGHT ///'
            PasswortName.SetText "a12345"
            hCloseDialog( PasswordFileOpen, "ok" )
        else
            warnlog( "Password dialog on FileOpen is missing" )
        endif
    else
        warnlog( "Failed to open File Open dialog" )
    endif

'/// File->SaveAs ///'
    hUseAsyncSlot( "FileSaveAs" )
    Kontext "SpeichernDlg"
    Dateiname.SetText Datei$
    if (Passwort.IsChecked <> TRUE) then
        '#i36015# 
        Warnlog "Password has to be checked! :-("
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
        hCloseDialog( passwordFileSave, "cancel" )
        '/// close document ///'
        hUseAsyncSlot( "FileClose" )
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
    hDeleteFile( Datei )

    ' make sure saving in latest math filter works, checking global filtername and if necessary change it
    sFilter = hGetUIFiltername("math8")

    '/// open application ///'
    hNewDocument
    '/// type a formula ///'
    call SchreibenInMathdok "a over b"
    '/// File->Save As ///'
    hUseAsyncSlot( "FileSaveAs" )
    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then
        '/// select the actual default file format ///'
        Dateityp.Select (hFindFilterPosition( sFilter ))
        '/// type a file name ///'
        Dateiname.SetText Datei
        '/// click button 'save' ///'
        Speichern.Click
        Kontext "Active"

        if Active.Exists(2) then hCloseDialog( Active, "yes" )
    else
        warnlog( "File Save dialog did not open" )
    endif

    '/// File->Close ///'
    hUseAsyncSlot( "FileClose" )
    Kontext
    if MessageBox.Exists (1) then
        WarnLog "After Saving and Closing the following MessageBox appears: " + MessageBox.GetText
        MessageBox.Yes
    end if

    '/// open just saved file ///'
    Call hFileOpen( Datei )
    '/// type a formula ///'
    sleep( 2 )
    call SchreibenInMathdok "a over b"
    sleep( 2 )
'/// File->Reload ///'
    hUseAsyncSlot( "FileReload" )
    Kontext
    if Active.Exists ( 3 ) then
        PrintLog "Say no to: " + Active.GetText
        '/// say NO to active ///'
        active.No
    else
        WarnLog "Missing smth to say no to! "
    end if


    '/// File->Reload ///'
    hUseAsyncSlot( "FileReload" )
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

'-----------------------------------------------------------
'*******************   I D  dito   *************************
'-----------------------------------------------------------

testcase tmFileOpen
    '/// open application ///'
    call hNewDocument
    '/// File->Open ///'
    FileOpen
    Kontext "OeffnenDlg"
    if ( OeffnenDlg.exists( 2 ) ) then
        '/// klick button 'Up one level' ///'
        UebergeordneterOrdner.Click
        '/// klick button 'Default Directory' ///'
        Standard.Click
        '/// check checkbox 'Read Only' ///'
        NurLesen.check
        Call DialogTest ( OeffnenDlg )
        '/// cancel dialog 'Open' ///'
        hCloseDialog( OeffnenDlg, "cancel" )
    endif
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tmFileClose
    '/// open application ///'
    hNewDocument    ' just for the records: i open ONE document
    '/// type a formula ///'
    sleep( 2 )
    call SchreibenInMathdok "a over b"
    sleep( 2 )
'/// File->Close ///'
    hUseAsyncSlot( "FileClose" )
    Kontext         ' expecting 'modified, do you want to close?'
    if active.exists (5) then
        printlog "  ok, active came up: " + active.gettext
        '/// say NO to active about 'modified stuff :-)' ///'
        Active.Cancel   ' no, not this time
    else
        warnlog "active missing (1)"
    endif

    '/// File->Close ///'
    hUseAsyncSlot( "FileClose" )
    Kontext
    '/// say YES to active about 'modified stuff :-)' ///'
    hCloseDialog( Active, "yes" )

    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then
        Call DialogTest ( SpeichernDlg )
        '/// cancel dialog 'Save As' ///'
        hCloseDialog( SpeichernDlg, "cancel" )
    else
        warnlog( "File Save dialog did not open" )
    endif

    '/// File->Close ///'
    hUseAsyncSlot( "FileClose" )       ' now the office gets closed! (if there were no modifications!)
    Kontext
    if active.exists (5) then hCloseDialog( Active, "No" )

endcase

testcase tmFileSave
    '/// open application ///'
    hNewDocument
    '/// type a formula ///'
    call SchreibenInMathdok "a over b"
    '/// File->Save ///'
    hUseAsyncSlot( "FileSave" )

    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then
        UebergeordneterOrdner.click
        Standard.Click
        '      NeuerOrdner.Click
        '      DateiAuswahl.TypeKeys "Hallo<Return>"
        '      DateiLoeschen
        '      Kontext "Messagebox"
        '      MessageBox.Yes
        Call DialogTest (SpeichernDlg)
        '/// cancel dialog 'Save' ///'
        hCloseDialog( SpeichernDlg, "cancel" )
    else
        warnlog( "File Save dialog did not open" )
    endif
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tmFileSaveAs
    '/// open application ///'
    hNewDocument
    sleep (2)
    '/// type a formula ///'
    call SchreibenInMathdok "a over b"
    '/// File->Save As ///'
    hUseAsyncSlot( "FileSaveAs" )

    Kontext "SpeichernDlg"
    if (SpeichernDlg.exists ( 5 ) ) then

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
        hCloseDialog( SpeichernDlg, "cancel" )
    else
        warnlog( "File Save dialog did not open" )
    endif

    '/// close application ///'
    Call hCloseDocument
endcase

testcase tmFileSaveAll
    '/// open application ///'
    hNewDocument
    '/// type a formula ///'
    call SchreibenInMathdok "a over b"
    Printlog " '/// open 2. window ///"
    hNewDocument
    '/// type a formula ///'
    call SchreibenInMathdok "a over b"

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
    hCloseDialog( ExportierenDlg, "cancel" )

    '/// close application ///'
    Call hCloseDocument
endcase

testcase tmFileProperties
    '/// open application ///'
    Call hNewDocument
    '/// File->Properties... ///'
    hUseAsyncSlot( "FileProperties" )

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
    hCloseDialog( TabInternet, "cancel" )
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
    hCloseDialog( InfoNamenBearbeiten, "cancel" )
endcase

testcase tmFilePrinterSetting
    '/// open application ///'
    Call hNewDocument

'/// File->Printer Settings ///'
    hUseAsyncSlot( "FilePrintersettings" )
    kontext
    if active.exists(2) then
        active.ok
        qaerrorlog "There is no printer available - please install one on your system!"
    endif

    Kontext "DruckerEinrichten"
    if ( DruckerEinrichten.exists( 2 ) ) then
        Call DialogTest (DruckerEinrichten)
    
        '/// cancel dialog 'printer setup' ///'
        hCloseDialog( DruckerEinrichten, "cancel" )
    else
        warnlog( "Printer Configuration dialog did not open" )
    endif

    '/// close application ///'
    Call hCloseDocument
endcase


