'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: id_001.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:41 $
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
'* Owner : wolfram.garten@sun.com
'*
'* short description :
'*
'***********************************************************************************
' #1 tiFileSaveAs
' #1 tiFileReload (sFileIn as string)
' #1 tiFileVersion (sFileIn as string)
' #1 tiFilePassword
' #1 tiFileTemplates
' #1 tiFilePrint
' #1 tmFileNewFromTemplate
' #1 tmFileOpen
' #1 tmFileClose
' #1 tmFileSave
' #1 tmFileSaveAs
' #1 tmFileSaveAll
' #1 tmFileProperties
' #1 tmFilePrinterSetting
'\**********************************************************************************


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

    if (gApplication = "IMPRESS") then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

    sFilter (0) = 0 ' initalize ;-)...

    sFileName = "graphics\required\input\tbo_alf_." + ExtensionString    ' this is the file with all features
    sFile = "isas_"                                ' this is the filename of the export files
    sPath = ConvertPath ( gOfficePath + "user/work/graphics/required/"+gApplication+"\"+ gPlatform) ' this is the export path
    if dir (sPath) = "" then
        app.mkdir (sPath)
    end if

    if gSamePC = TRUE then ' delete export directory
        GetFileList ( sPath, sFile+"*", Exlist() )
        if KillFileList ( Exlist() ) <> TRUE then
            Warnlog "Couldn't delete all Files in Output-Export-Directory, the followings are still there:"
            for i=1 to ListCount ( Exlist() )
                printlog "  <> " + Exlist(i)
            next i
        end if
    end if

    Call hFileOpen ( ConvertPath(gTesttoolPath + sFileName) )
    WaitSlot (3000)


    ' to use the helper fileSaveAs functions i get alll available filters...
    FileSaveAs
    WaitSlot (2000)
    Kontext "SpeichernDlg"
    for i = 0 to 5
        if i=0 then x = Dateityp.GetItemCount
            if (i) then  ' set border, whenm start from beginning/end
                y = i     ' set filter from beginning
            else
                y = x-3     ' set filter from end
            end if
            ListAppend(sFilter(), Dateityp.GetItemText (y))
        next i
        SpeichernDlg.Cancel
        y = ListCount(sFilter())

        if gtSYSName = "Linux" then y=y-1
            '#i45961# - last part of filter-list deactivated due to BUG - FHA
            'TODO: JSI->FHA please verify if this special handling needed after the issue has been fixed.

            for i = 1 to y
                sFileName = sPath + sFile + (i)
                hFileSaveAsWithFilter (sFileName, sFilter(i), TRUE )
                kontext
                if messagebox.exists then
                    messagebox.Yes ' to go on ....
                end if
                printlog " saved with filter ("+i+"/"+y+"): "+ sFilter(i)
            next i
            ' TODO TBO: small check, if correct filter was used!
            WaitSlot (3000)
            fileclose
            WaitSlot (3000)
            kontext "Messagebox"
            if Messagebox.exists then
                printlog "Messagebox about informationloss... :-) that's OK: 'Text in the messagebox: "+Messagebox.GetText+"'"
                Messagebox.YES
            end if

            '-----------------------------------------------------------------
            hNewDocument


            sFilter (0) = 0
            sFileName = sPath
            GetFileList ( sFileName, sFile + "*", sFilter() )

            '   call hNewDocument
            x = ListCount ( sFilter() )
            for i = 1 to x
                printlog "("+i+"/"+x+"): "+sFilter(i)
                hFileOpen ( sFilter(i) )
                WaitSlot (3000)
                If hIsNamedDocLoaded (sFilter(i), true) Then
                    printlog "  used filter: " + hGetUsedFilter()
                else
                    sTemp = left(right(sFilter(i),2),1)
                    if ( (sTemp = "t") OR (sTemp = "o") ) then
                        printlog "document is --- TEMPLATE?! --- "
                    else
                        qaErrorlog "#116563# document didn't get loaded "
                    end if
                end if
                hCloseDocument
                WaitSlot (2000)
            next i
            '   Call hCloseDocument
endcase

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
    Call hNewDocument
    WaitSlot (2000)
    Call hFileSaveAsKill (Datei$)
    WaitSlot (2000)
    Call hCloseDocument

    Call hFileOpen (Datei$)
    WaitSlot (2000)
    call hTBOtypeInDoc
    WaitSlot (2000)
    FileReload
    WaitSlot (2000)
    Kontext "Messagebox"
    sleep 2
    Messagebox.No
    WaitSlot (2000)

    FileReload
    WaitSlot (2000)
    Kontext "Messagebox"
    sleep 2
    Messagebox.Yes
    WaitSlot (1000)

    Call hCloseDocument
    if Dir (Datei$) <> "" AND gSamePC = TRUE then kill Datei$
endcase

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
        Call hNewDocument
        WaitSlot (1000)
        Call hFileSaveAsKill (Datei$)
        WaitSlot (1000)
        Call hCloseDocument

        Call hFileOpen (Datei$)
        WaitSlot (1000)
        call hTBOtypeInDoc
        WaitSlot (1000)
        try
            FileVersions
        catch
            Warnlog "- File / Versions not accessible!"
            goto endsub
        endcatch

        Kontext "Versionen"
        Call DialogTest ( Versionen )
        Speichern.Click
        Kontext "VersionskommentarEingeben"
        Call DialogTest ( VersionskommentarEingeben )
        VersionskommentarEingeben.Cancel
        Kontext "Versionen"
        Versionen.Close
        Call hCloseDocument
        '   if Dir (Datei$) <> "" AND gSamePC = TRUE then kill Datei$
endcase

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

    sFile = "isp__"                                                     ' this is the filename of the export files
    sPath = ConvertPath ( gOfficePath + "user/work/graphics/required/"+gApplication+"\"+ gPlatform+"\") ' this is the export path
    mkdir (sPath)
    if dir (sPath) = "" then app.mkdir (sPath)
        sFileIn = (sPath + sFile + "." + ExtensionString)
        if gSamePC = TRUE then ' delete export directory
            GetFileList ( sPath, sFile+"*", Exlist() )
            if KillFileList ( Exlist() ) <> TRUE then
                Warnlog "Couldnt delete all Files in Output-Export-Directory, the followings are still there:"
                for i=1 to ListCount ( Exlist() )
                    printlog "  <> " + Exlist(i)
                next i
            end if
        end if

        Call hNewDocument
        WaitSlot (1000)
        FileSaveAs
        Kontext "SpeichernDlg"
        Datei$ = ConvertPath ( sFileIn )
        printlog "   will use the file: "+Datei$
        if Dir (Datei$) <> "" then
            kill Datei$
        end if
        Passwort.Check
        Dateiname.SetText Datei$
        Speichern.Click
        WaitSlot (2000)
        Kontext "Passwort"
        PasswortName.SetText "12345"
        PasswortBestaetigen.Settext "54321"
        Passwort.OK
        WaitSlot (2000)
        Kontext "Messagebox"
        if Messagebox.Exists(1) then
            Messagebox.OK
        else
            Warnlog "- Wrong password not recognized"
        end if
        Kontext "Passwort"
        PasswortName.SetText "12345"
        PasswortBestaetigen.SetText "12345"
        Passwort.OK
        sleep 2
        FileClose
        sleep 2

        FileOpen
        Kontext "OeffnenDlg"
        WaitSlot (2000)
        Dateiname.SetText Datei$
        Oeffnen.Click
        WaitSlot (2000)
        Kontext "PasswordFileOpen"
        PasswortName.SetText "34567"
        try
            PasswordFileOpen.OK
        catch
            Printlog "- Wrong password not accepted"
        endcatch
        Kontext
        if Not Messagebox.Exists(1) then
            Warnlog "Wrong password while loading not recognized"
        else
            Messagebox.OK
            printlog "Wrong password on loading ok - recogniced"
        end if
        WaitSlot (3000)
        Kontext "PasswordFileOpen"
        PasswortName.SetText "12345"
        PasswordFileOpen.OK
        WaitSlot (5000)
        FileSaveAs
        Kontext "SpeichernDlg"
        Dateiname.SetText Datei$
        if (Passwort.IsChecked <> TRUE) then
            Warnlog "Password has to be checked! :-("
        end if
        Speichern.Click
        WaitSlot (1000)
        Kontext "Messagebox"
        if Messagebox.Exists(5) then
            Messagebox.Yes
        end if
        Kontext "Passwort"
        if (Passwort.Exists(5) = FALSE)then
            Warnlog "- Password dialog Didn't pop up after pressing save"
        else
            Kontext "Passwort"
            WaitSlot (1000)
            PasswortName.SetText "a12345"
            PasswortBestaetigen.SetText "a12345"
            Passwort.OK
            WaitSlot (1000)
            FileClose
            WaitSlot (1000)
        end if
        FileOpen
        WaitSlot (2000)
        Kontext "OeffnenDlg"
        Dateiname.SetText Datei$
        Oeffnen.Click
        Kontext "PasswordFileOpen"
        WaitSlot (1000)
        PasswortName.SetText "a12345"
        PasswordFileOpen.OK
        WaitSlot (5000)
        FileSaveAs
        Kontext "SpeichernDlg"
        Dateiname.SetText Datei$
        if (Passwort.IsChecked <> TRUE) then
            Warnlog "Password has to be checked! :-("
        end if
        Passwort.UnCheck
        Speichern.Click
        Kontext "Messagebox"
        if Messagebox.Exists(5) then Messagebox.Yes
            Kontext "Passwort"
            if (Passwort.Exists(5))then
                Warnlog "- Password dialog didn't pop up after pressing save"
                Kontext "Passwort"
                password.cancel
                FileClose
            else
                hCloseDocument
            end if
endcase

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
endcase

testcase tiFilePrint
    Call hNewDocument
    FilePrint
    WaitSlot (1000)
    kontext
    if active.exists(5) then
        active.ok
        qaerrorlog "There is no printer available - please install one on your system!"
    end if
    WaitSlot (2000)
    Kontext "DruckenDlg"
    Call DialogTest (DruckenDlg)
    WaitSlot (2000)
    Zusaetze.Click

    Kontext "DruckerZusaetzeDraw"
    Call DialogTest (DruckerZusaetzeDraw)
    DruckerZusaetzeDraw.Cancel
    Kontext "DruckenDlg"
    DruckenDlg.Cancel
    WaitSlot (2000)
    Call hCloseDocument
endcase


'-----------------------------------------------------------
'*******************  M A T H dito *************************
'-----------------------------------------------------------


testcase tmFileNewFromTemplate
    Call hNewDocument
    FileNewFromTemplate
    WaitSlot (5000)
    Kontext "TemplateAndDocuments"
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
endcase

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
endcase

testcase tmFileClose
    printlog "- File Close"
    hNewDocument    ' just for the records: I open ONE document
    call hTBOtypeInDoc
    WaitSlot (2000)
    FileClose
    Kontext         ' Expecting "Modified, do you want to close?"
    if active.exists (5) then
        printlog "  Ok, active came up: " + active.gettext
        Active.Cancel   ' No, not this time
    else
        warnlog "active missing (1)"
    end if
    WaitSlot (2000)

    FileClose
    Kontext
    Active.Yes      ' but now - records: this document is closed
    WaitSlot (2000)

    Kontext "SpeichernDlg"
    Call DialogTest ( SpeichernDlg )
    SpeichernDlg.Cancel
    WaitSlot (2000)

    FileClose       ' now the office gets closed! (if there were no modifications!)
    Kontext
    Active.No
    WaitSlot (2000)
endcase
'-----------------------------------------------------------
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
endcase

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
endcase

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
endcase

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

    'Deactivating this part because of #i95523#:
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
endcase

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
endcase

testcase tdFileExport
    goto endsub
    dim x as integer
    Call hNewDocument
    WaitSlot (3000)
    FileExport
    Kontext "ExportierenDlg"
    Call DialogTest ( ExportierenDlg )
    UebergeordneterOrdner.Click
    Kontext "SpeichernDlg"
    x=Dateityp.getitemcount
    if x <> 18 then warnlog "the number of filters is not 18, it is: " + x
        SpeichernDlg.Cancel
        WaitSlot (2000)
        Call hCloseDocument
        WaitSlot (2000)
endcase

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
endcase
