'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_001_.inc,v $
'*
'* $Revision: 1.5 $
'*
'* last change: $Author: rt $ $Date: 2008-09-04 09:26:17 $
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
'* owner : fredrik.haegg@sun.com
'*
'* short description :  Required-Testcases for the File-menu.
'*
'\***********************************************************************

sub w_001printLog Chr(13) + "--------- File Menu (w_001_.inc) ----------"
    gApplication = "WRITER"

    Call tFileNewLabels
    Call tFileNewBusinessCards
    Call tFileOpen

    Call tFileClose
    Call tFileSave
    Call tFileSaveAll
    Call tFileSaveAs
    Call tFileSaveAsExport
    Call tFileSaveWithPassword
    Call tFileReload
    Call tFileVersions

    if gApplication <> "HTMLDOKUMENT" then
        Call tToolsXMLFilterSettings
    end if
    Call tFileExportAsPDF
    Call tExportAsPDFButton
    Call tFileSendCreateMasterDocument
    Call tFileSendCreateHTMLDocument
    Call tFileSendOutlineToPresentation
    Call tFileSendOutlineToClipboard
    Call tFileSendCreateAutoabstract
    Call tFileSendAutoabstractToPresentation

    'Writer Master Document
    gApplication = "MASTERDOC"
    Call tMasterDocFileSaveWithPassword
    Call tMasterDocFileReload
    Call tMasterDocFileVersions
    Call tMasterDocFileSendCreateHTMLDocument
    Call tMasterDocFileSendOutlineToPresentation
    Call tMasterDocFileSendOutlineToClipboard
    Call tMasterDocFileSendCreateAutoabstract
    Call tMasterDocFileSendAutoabstractToPresentation
    gApplication = "WRITER"

end sub

'-----------------------------------------------------------

testcase tFileNewLabels
    Dim i as integer
    Dim x as integer
    Dim y as integer
    Dim sTypes as integer
    Dim vTypes as integer

    PrintLog "- File / New / Labels"
    if gApplication <> "WRITER" then
        printlog "Not in : " + gApplication
        goto endsub
    end if

    PrintLog "Open Labels-dialog with File / New / Labels"
    if hCreateLabels = False then goto endsub
    Sleep()
    Call DialogTest (TabEtiketten)
    PrintLog "Switch to Tabpage 'Labels'"
    Sleep()
    PrintLog "Check number of brands and types (see #i44801# for details)"
    For i = 1 to 2
        Select Case i
        Case 1
            Endlos.Check
            printlog "Continous is checked"
        Case 2
            Bogen.Check
            printlog "Sheet is checked"
        end select
        PrintLog "There should be 10 brands in list"
        For x = 1 to Marke.GetItemCount
            Marke.Select x
            printlog "- Brand: " & Marke.GetSelText
            sTypes = Typ.GetItemCount
            Select Case x
            Case 1:
                PrintLog "Avery/A4 should have 295 types in list if 'Sheet' is checked"
                PrintLog "Avery/A4 should have 1 type in list if 'Continous' is checked"
                if i = 1 then vTypes = 1 else vTypes = 295
            Case 2:
                PrintLog "Avery A4/Asia should have 88 types in list if 'Sheet' is checked"
                PrintLog "Avery A4/Asia should have 1 type in list if 'Continous' is checked"
                if i = 1 then vTypes = 1 else vTypes = 88
            Case 3:
                PrintLog "Avery Lettersize should have 251 types in list if 'Sheet' is checked"
                PrintLog "Avery Lettersize should have 45 types in list if 'Continous' is checked"
                if i = 1 then vTypes = 45 else vTypes = 251
            Case 4:
                PrintLog "Avery Zweckform should have 333 types in list if 'Sheet' is checked"
                PrintLog "Avery Zweckform should have 67 types in list if 'Continous' is checked"
                if i = 1 then vTypes = 67 else vTypes = 333
            Case 5:
                PrintLog "Herlitz should have 26 types in list if 'Sheet' is checked"
                PrintLog "Herlitz should have 6 types in list if 'Continous' is checked"
                if i = 1 then vTypes = 6 else vTypes = 26
            Case 6:
                PrintLog "Herma A4 should have 491 types in list if 'Sheet' is checked"
                PrintLog "Herma A4 should have 1 type in list if 'Continous' is checked"
                if i = 1 then vTypes = 1 else vTypes = 491
            Case 7:
                PrintLog "Herma A5 should have 18 types in list if 'Sheet' is checked"
                PrintLog "Herma A5 should have 1 type in list if 'Continous' is checked"
                if i = 1 then vTypes = 1 else vTypes = 18
            Case 8:
                PrintLog "Herma endlos should have 1 type in list if 'Sheet' is checked"
                PrintLog "Herma endlos should have 119 types in list if 'Continous' is checked"
                if i = 1 then vTypes = 119 else vTypes = 1
            Case 9:
                PrintLog "Leitz should have 13 types in list if 'Sheet' is checked"
                PrintLog "Leitz should have 1 type in list if 'Continous' is checked"
                if i = 1 then vTypes = 1 else vTypes = 13
            Case 10:
                PrintLog "Sigel should have 20 types in list if 'Sheet' is checked"
                PrintLog "Sigel should have 1 type in list if 'Continous' is checked"
                if i = 1 then vTypes = 1 else vTypes = 20
            Case else: Warnlog "- There are more than 10 brands in list"
            end select
            if sTypes <> vTypes then Warnlog "-  Not " & sTypes & " types in list but " & vTypes
            For y = 1 to Typ.GetItemCount
                Typ.Select y
                printlog "-  Type: " & Typ.GetSelText
            next y
        next x
    next i
    Sleep()

    PrintLog "Switch to Tabpage 'Format'"
    Kontext
    active.SetPage TabFormatEtiketten

    wait 250
    Kontext "TabFormatEtiketten"
    Call DialogTest (TabFormatEtiketten)
    wait 250

    PrintLog "Click 'Save'"
    FormatSpeichern.Click

    PrintLog "Close dialog 'Save Label Format'"
    Kontext "SaveLabelFormat"
    Call DialogTest (SaveLabelFormat)
    SaveLabelFormat.Cancel
    Kontext "TabFormatEtiketten"
    Sleep()

    PrintLog "Switch to Tabpage 'Options'"
    Kontext
    active.SetPage TabDruckerEtiketten
    wait 250
    Kontext "TabDruckerEtiketten"
    Call DialogTest (TabDruckerEtiketten)

    PrintLog "Click pushbutton 'Setup'"
    if gPlatgroup = "w95" or gPlatgroup = "wnt" then
        Einrichten.Click

        PrintLog "Close printer-setup dialog"
        Kontext "DruckerEinrichten"
        if DruckerEinrichten.Exists( 2 ) then
            DruckerEinrichten.Cancel
        else
            Warnlog "No Default Printer!"
        end if
        Kontext "TabDruckerEtiketten"
    end if

    Sleep()
    PrintLog "Check checkbox 'Synchronize contents'"
    InhaltSynchronisieren.Check
    wait 250
    PrintLog "Click 'New document'"
    NeuesDok.Click
    Sleep()

    Kontext "Active"
    if Active.Exists then
        try
            Active.Ok
        catch
            Active.Cancel
            Warnlog "Once Document has been opened, office asks to save it! BugID 89702"
        endcatch
    end if

    wait 250
    PrintLog "Check if dialog 'Synchronize Labels' is up"
    Kontext "EtikettenSynchronisieren"
    if EtikettenSynchronisieren.Exists then
        Call DialogTest ( EtikettenSynchronisieren )
        Synchronisieren.Click
        Sleep()
    else
        Warnlog " Dialog to synchronize labels not up!"
    end if

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileNewBusinessCards
    Dim i as integer
    Dim x as integer
    Dim y as integer
    Dim sTypes as integer
    Dim vTypes as integer

    Printlog "- File / New / Business Cards"
    if gApplication <> "WRITER" then
        printlog "Not in : " + gApplication
        goto endsub
    end if

    PrintLog "Open Business Cards dialog with File / New / Business Cards"
    if hCreateBusinessCards = False then goto endsub
    PrintLog "Switch to Tabpage 'Medium'"
    Kontext
    Active.Setpage TabEtikettenMedium
    Kontext "TabEtikettenMedium"
    Call DialogTest (TabEtikettenMedium)
    Sleep()
    PrintLog "Check number of brands and types (see #i44801# for details)"
    For i = 1 to 2
        Select Case i
        Case 1
            Endlos.Check
            printlog "Continous is checked"
        Case 2
            Bogen.Check
            printlog "Sheet is checked"
        end select
        PrintLog "There should be 10 brands in list"
        For x = 1 to Marke.GetItemCount
            Marke.Select x
            printlog "- Brand: " & Marke.GetSelText
            sTypes = Typ.GetItemCount
            Select Case x
            Case 1:
                PrintLog "Avery/A4 should have 295 types in list if 'Sheet' is checked"
                PrintLog "Avery/A4 should have 1 type in list if 'Continous' is checked"
                if i = 1 then vTypes = 1 else vTypes = 295
            Case 2:
                PrintLog "Avery A4/Asia should have 88 types in list if 'Sheet' is checked"
                PrintLog "Avery A4/Asia should have 1 type in list if 'Continous' is checked"
                if i = 1 then vTypes = 1 else vTypes = 88
            Case 3:
                PrintLog "Avery Lettersize should have 251 types in list if 'Sheet' is checked"
                PrintLog "Avery Lettersize should have 45 types in list if 'Continous' is checked"
                if i = 1 then vTypes = 45 else vTypes = 251
            Case 4:
                PrintLog "Avery Zweckform should have 333 types in list if 'Sheet' is checked"
                PrintLog "Avery Zweckform should have 67 types in list if 'Continous' is checked"
                if i = 1 then vTypes = 67 else vTypes = 333
            Case 5:
                PrintLog "Herlitz should have 26 types in list if 'Sheet' is checked"
                PrintLog "Herlitz should have 6 types in list if 'Continous' is checked"
                if i = 1 then vTypes = 6 else vTypes = 26
            Case 6:
                PrintLog "Herma A4 should have 491 types in list if 'Sheet' is checked"
                PrintLog "Herma A4 should have 1 type in list if 'Continous' is checked"
                if i = 1 then vTypes = 1 else vTypes = 491
            Case 7:
                PrintLog "Herma A5 should have 18 types in list if 'Sheet' is checked"
                PrintLog "Herma A5 should have 1 type in list if 'Continous' is checked"
                if i = 1 then vTypes = 1 else vTypes = 18
            Case 8:
                PrintLog "Herma endlos should have 1 type in list if 'Sheet' is checked"
                PrintLog "Herma endlos should have 119 types in list if 'Continous' is checked"
                if i = 1 then vTypes = 119 else vTypes = 1
            Case 9:
                PrintLog "Leitz should have 13 types in list if 'Sheet' is checked"
                PrintLog "Leitz should have 1 type in list if 'Continous' is checked"
                if i = 1 then vTypes = 1 else vTypes = 13
            Case 10:
                PrintLog "Sigel should have 20 types in list if 'Sheet' is checked"
                PrintLog "Sigel should have 1 type in list if 'Continous' is checked"
                if i = 1 then vTypes = 1 else vTypes = 20
            Case else: Warnlog "- There are more than 10 brands in list"
            end select
            if sTypes <> vTypes then Warnlog "-  Not " & sTypes & " types in list but " & vTypes
            For y = 1 to Typ.GetItemCount
                Typ.Select y
                printlog "-  Type: " & Typ.GetSelText
            next y
        next x
    next i
    Sleep()
    PrintLog "Switch to Tabpage 'Business Cards'"
    Kontext
    Active.Setpage TabVisitenkarten
    Sleep()

    Kontext "Active"
    if Active.GetRT = 304 then
        Warnlog Active.Gettext
        Active.Ok
    end if

    Kontext "TabVisitenkarten"
    for i = 1 to 20
        if Inhalt.IsVisible then
            Call DialogTest (TabVisitenkarten)
            i = 21
        else
            Sleep 1
        end if
    next i

    PrintLog "Switch to Tabpage 'Private'"
    Kontext
    Active.Setpage TabPrivat
    wait 250
    Kontext "TabPrivat"
    Call DialogTest (TabPrivat)

    PrintLog "Switch to Tabpage 'Business'"
    Kontext
    Active.Setpage TabGeschaeft
    wait 250
    Kontext "TabGeschaeft"
    Call DialogTest (TabGeschaeft)

    Kontext
    Active.Setpage TabFormatEtiketten
    wait 250
    Kontext "TabFormatEtiketten"
    Call DialogTest (TabFormatEtiketten)
    PrintLog "Switch to Tabpage 'Format'"
    wait 250
    PrintLog "Click pushbutton 'Save'"
    FormatSpeichern.Click

    PrintLog "Close upcoming dialog 'Save Label Format'"
    Kontext "SaveLabelFormat"
    wait 250
    Call DialogTest (SaveLabelFormat)
    SaveLabelFormat.Cancel
    Kontext "TabFormatEtiketten"
    Sleep 2

    Kontext
    Active.Setpage TabZusaetzeEtiketten
    wait 250
    Kontext "TabZusaetzeEtiketten"
    Call DialogTest (TabZusaetzeEtiketten)
    Einrichten.Click
    Sleep 1
    Kontext "DruckerEinrichten"
    if DruckerEinrichten.Exists then
        DruckerEinrichten.Cancel
    else
        Warnlog "No Default Printer!"
    end if

    Kontext "TabZusaetzeEtiketten"
    PrintLog "Switch to Tabpage 'Options'"
    PrintLog "Check Checkbox 'Syncronize contents"
    wait 250
    InhaltSynchronisieren.Check

    PrintLog "Click 'new Document'"
    NeuesDok.Click
    Sleep 2
    Kontext "Active"
    if Active.Exists then
        try
            Active.Ok
        catch
            Active.Cancel
            Warnlog "Once Document has been opened, office asks to save it! BugID 89702"
        endcatch
    end if

    wait 250
    Kontext "EtikettenSynchronisieren"
    PrintLog "Check i dialog 'Syncronize Labels' is up"
    if EtikettenSynchronisieren.Exists then
        Call DialogTest ( EtikettenSynchronisieren )
        Synchronisieren.Click
        Sleep 1
    else
        Warnlog "Dialog to synchronize Business Cards not up!"
    end if

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileOpen
    printLog "- File / Open"

    FileOpen
    WaitSlot(2000)
    if gUseSysDlg = False then
        Kontext "OeffnenDlg"
        Call DialogTest ( OeffnenDlg )
        UebergeordneterOrdner.Click
        PrintLog "Click 'Up one level'"
        Sleep 1
        try
            Standard.Click
            PrintLog "Click button 'Default directory'"
        catch
            ExceptLog
        endcatch
        Sleep 1
        PrintLog "Click Button 'New folder'"
        if NeuerOrdner.IsVisible and NeuerOrdner.IsEnabled then
            NeuerOrdner.Click
            Sleep 1
            Kontext "NeuerOrdner"
            OrdnerName.SetText "TT_Test"
            PrintLog "Enter a folder name in upcoming dialog and click 'Cancel'"
            NeuerOrdner.Cancel
        else
            Warnlog "Unable to create new folder! (Button disabled)"
        end if
        Kontext "OeffnenDlg"
        OeffnenDlg.Cancel
        PrintLog "Close File-Open-dialog with 'Cancel'"
    end if
endcase

'-----------------------------------------------------------

testcase tFileClose
    PrintLog "- File/Close"

    PrintLog "Open new document"
    Call hNewDocument
    PrintLog "Insert a table"
    Call hTabelleEinfuegen
    wait 250

    PrintLog "File / Close"
    FileClose
    Kontext
    WaitSlot(500)
    PrintLog "Cancel upcoming messagebox"
    Active.Cancel
    Sleep 1

    PrintLog "again File / Close"
    FileClose
    Kontext "Active"
    wait 250
    Active.Yes

    Sleep 1
    if gUSeSysDlg = False then
        PrintLog "Click 'Cancel' in File-Save-Dialog"
        Kontext "SpeichernDlg"
        if SpeichernDlg.Exists then
            SpeichernDlg.Cancel
        else
            Kontext "Active"
            if Active.Exists then
                Warnlog Active.Gettext
                Active.Ok
            else
                Warnlog "Dialog to Save document isn't up!"
            end if
        end if
    end if

    Sleep 1
    PrintLog "Choose File / Close again"
    FileClose
    PrintLog "On upcoming messagebox click 'No'"
    Kontext
    WaitSlot(500)
    Active.No
    Sleep 1
endcase

'-----------------------------------------------------------

testcase tFileSave
    PrintLog "- File/Save"

    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Insert a table"
    Call hTabelleEinfuegen

    if gUseSysDlg = False then
        PrintLog "Choose 'File / Save'"
        FileSave
        Kontext "SpeichernDlg"
        Call DialogTest ( SpeichernDlg )
        PrintLog "In File-Dialog click 'Up one level'"
        UebergeordneterOrdner.Click
        Sleep 1
        try
            PrintLog "Click 'Default directory'"
            Standard.Click
        catch
            ExceptLog
        endcatch
        Sleep 1
        PrintLog "Click 'Create New Folder '"
        if NeuerOrdner.IsVisible and NeuerOrdner.Exists then
            if NeuerOrdner.IsEnabled then
                NeuerOrdner.Click
                Kontext "NeuerOrdner"
                PrintLog "Enter a foldername"
                OrdnerName.SetText "TT_Test"
                PrintLog "Close 'Create New Folder-dialog' with cancel"
                NeuerOrdner.Cancel
            else
                Warnlog "Unable to creater new folder! (Button disabled)"
            end if
        else
            Warnlog "Unable to create new folder! (Button not found)"
        end if
        Kontext "SpeichernDlg"
        PrintLog "Close File-Save-Dialog with Cancel"
        SpeichernDlg.Cancel
    end if

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileSaveAll
    PrintLog "- File / Save All"

    PrintLog "Open new document"
    Call hNewDocument
    PrintLog "Insert a table in document"
    Call hTabelleEinfuegen
    PrintLog "Open a second document"
    Call hNewDocument
    PrintLog "Insert a table in second document"
    Call hTabelleEinfuegen


    if gUSeSysDlg = False then
        PrintLog "Choose File / Save all"
        FileSaveAll
        Kontext "SpeichernDlg"
        WaitSlot(5000)
        PrintLog "Cancel 1. upcoming Filedialog"
        SpeichernDlg.Cancel

        Kontext "SpeichernDlg"
        PrintLog "Cancel 2. upcoming Filedialog"
        Sleep 1
        SpeichernDlg.Cancel

        Kontext "SpeichernDlg"
        Sleep 1
        if SpeichernDlg.Exists then SpeichernDlg.Cancel
    end if

    Sleep 1

    PrintLog "Close documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-----------------------------------------------------------

testcase tFileSaveAs
    PrintLog "- File / Save As"

    PrintLog "Open new document"
    Call hNewDocument

    if gUseSysDlg = False then
        PrintLog "Choose 'Fiole / Save as'"
        FileSaveAs
        Kontext "SpeichernDlg"
        Call DialogTest ( SpeichernDlg )
        PrintLog "Click button 'Up one level'"
        UebergeordneterOrdner.Click
        Sleep 1
        try
            PrintLog "Click button 'Default Directory'"
            Standard.Click
        catch
            ExceptLog
        endcatch
        Sleep 1
        PrintLog "Click button 'Create Folder'"
        if NeuerOrdner.IsVisible and NeuerOrdner.Exists then
            if NeuerOrdner.IsEnabled then
                NeuerOrdner.Click
                Kontext "NeuerOrdner"
                PrintLog "Enter a foldername in upcoming dialog"
                OrdnerName.SetText "TT_Test"
                PrintLog "Close dialog with 'Cancel'"
                NeuerOrdner.Cancel
            else
                Warnlog "Unable to create new folder! (Button disabled)"
            end if
        else
            Warnlog "Unable to create new folder! (Button not found)"
        end if
        Kontext "SpeichernDlg"
        SpeichernDlg.Cancel
    end if

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileSaveAsExport
    QaErrorLog "#i93340# - Testcase outcommented due to bug with filter."
    goto endsub

    Dim xExport as string
    Dim xExportFile as string
    Dim CountDocu as integer
    PrintLog "- File / Save As / Export with MS-filter"

    Select Case gApplication
    Case "WRITER"
        xExport = Convertpath( gTesttoolpath + "writer\required\input\st1_1.sxw" )
    Case else
        goto endsub
    end select
    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Open document '..\writer\required\input\st1_1.sxw"
    Call hFileOpen(xExport)

    Sleep 1
    Call sMakeReadOnlyDocumentEditable

    PrintLog "Select Save as .. and choose 'Microsoft Word 97/2000/XP' as export-filter"
    xExportFile = gOfficepath + "user\work\ecksport.doc"
    Call hFileSaveAsWithFilterKill (xExportFile, "MS Word 2003 XML")

    CountDocu = 0
    Do Until GetDocumentCount = 1
        Call hCloseDocument
        CountDocu = CountDocu + 1
    Loop
    if CountDocu > 1 then
        QAErrorlog "More than 1 document had to be closed!"
    end if

    PrintLog "Close document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileSaveWithPassword
    Dim DokumentPfad$
    Dim Datei$
    Dim sFile$
    Dim sFile2$

    Printlog "- File / Save with Password"
    Select Case gApplication
    Case "WRITER"
        sFile$ = "password.odt"
        sFile2$ = "password2.odt"
    Case "MASTERDOC"
        sFile$ = "password.odm"
        sFile2$ = "password2.odm"
    Case "HTMLDOKUMENT"
        printlog "Not in Writer/Web!"
        goto endsub
    end select

    PrintLog "Open new document"
    Call hNewDocument
    PrintLog "Enter some text into document"
    Call wTypeKeys("This document will be saved with a password!")

    if gUseSysDlg = True then goto endsub
    Datei$ = ConvertPath ( gOfficePath + "user\work\" + sFile$ )
    if Dir (Datei$) <> "" then kill Datei$

    PrintLog "Choose File / Save as."
    FileSaveAs
    PrintLog "enter destination and check 'Password'"
    Kontext "SpeichernDlg"
    if Passwort.IsEnabled then
        Passwort.Check
    else
        Warnlog "- Not able to save with Password -> Checkbox is disabled! Maybe default-filter wrong!?"
        SpeichernDlg.Cancel
        PrintLog "if Checkbox is disabled the test stops here!"
        Call hCloseDocument
        goto endsub
    end if

    PrintLog "Save the document"
    Dateiname.SetText Datei$
    Speichern.Click
    Sleep 1
    Kontext "Active"
    if Active.Exists (1) then Active.Yes

    Sleep 1
    PrintLog "Password dialog has to come up"
    Kontext "PasswordFileSave"
    if PasswordFileSave.Exists then
        PrintLog "Enter your password"
        Password.SetText "12345"
        PrintLog "Re-Enter a wrong passwort"
        PasswordConfirm.Settext "54321"
        PasswordFileSave.OK
        Sleep 2
    else
        Warnlog "- Password-Dialog not up !"
    end if

    Kontext "Active"
    Sleep 1
    PrintLog "A messagebox has to come up, warns of wrong confirmation"
    if Active.Exists(1) then
        Active.OK
    else
        Warnlog "Able to save document with wrong password"
    end if

    Kontext "PasswordFileSave"
    if PasswordFileSave.Exists then
        PrintLog "Enter new password, and re-enter the same password"
        Password.SetText "12345"
        PasswordConfirm.SetText "12345"
        PasswordFileSave.OK
    else
        Warnlog "- Password-Dialog not up !"
    end if

    Sleep 1
    PrintLog "Close the document"
    FileClose
    WaitSlot(2000)
    PrintLog "Open previous saved document"
    Call hFileOpen(Datei$)

    Kontext "PasswordFileOpen"
    PrintLog "Enter a wrong password."
    if PasswordFileOpen.Exists then
        PasswortName.SetText "34567"
        PasswordFileOpen.OK
    else
        Warnlog "- Password-Dialog not up or Document couldn't be loaded!"
    end if

    PrintLog "A messagebox has to come up, warns of wrong password"
    Kontext "Active"
    Sleep 1
    if Not Active.Exists(1) then
        Warnlog "Able to load document with wrong password"
    else
        Active.OK
    end if

    Kontext "PasswordFileOpen"
    PrintLog "Close password dialog"
    if PasswordFileOpen.Exists then
        PasswordFileOpen.Cancel
    end if

    Sleep 1
    PrintLog "File / Open again your previous saved document"
    Call hFileOpen(Datei$)
    PrintLog "Enter your password in password dialog"
    Kontext "PasswordFileOpen"
    if PasswordFileOpen.Exists then
        PasswortName.SetText "12345"
        PasswordFileOpen.OK
    else
        Warnlog "- Password-Dialog not up!"
    end if
    Sleep 1

    Datei$ = ConvertPath ( gOfficePath + "user\work\" + sFile2$ )
    if Dir (Datei$) <> "" then kill Datei$
    PrintLog "File / Save as ..."
    FileSaveAs
    Kontext "SpeichernDlg"
    if Not Passwort.IsChecked = True then
        if gPlatgroup = "unx" then
            QAErrorlog "#i73968#Opening a password protected file looses focus after loading"
        else
            Warnlog "#i36015#Checkbox 'Save with password' is not checked in file-dialog !"
        end if
        Passwort.Check
        SpeichernDlg.Cancel
    else
        Dateiname.SetText Datei$
        Speichern.Click
        Kontext "Active"
        if Active.Exists(2) then Active.Yes
        PrintLog "Click on 'Save' in file open dialog"
        Kontext "PasswordFileSave"
        if Not PasswordFileSave.Exists(1) then
            Warnlog "Passworddialog is not coming up on saving document"
        else
            Password.SetText "54321"
            PasswordConfirm.SetText "54321"
            PasswordFileSave.OK
            PrintLog "The password dialog should come up"
            Sleep 1
            FileClose
            PrintLog "File / Close and open your document again"
            Call hFileOpen(Datei$)
            Sleep 1
            Kontext "PasswordFileOpen"
            PrintLog "Now the passwort dialog should come up"
            if Not PasswordFileOpen.Exists(2) then
                Warnlog "Passworddialog is not coming up on reload"
            else
                PasswortName.SetText "54321"
                PasswordFileOpen.OK
            end if
        end if
    end if
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileReload
    Dim DokumentPfad$
    Dim Datei$
    Dim sFile$
    Dim WriterFilterExtension(10) as string
    Dim MasterDocFilterExtension(10) as string

    WriterFilterExtension() = hGetFilternameExtension ( "writer8" )
    MasterDocFilterExtension() = hGetFilternameExtension ( "writerglobal8" )

    PrintLog "- File / Reload"
    Select Case gApplication
    Case "WRITER"
        sFile$ = "reload." & WriterFilterExtension(0)
    Case "MASTERDOC"
        sFile$ = "reload." & MasterDocFilterExtension(0)
    Case "HTMLDOKUMENT"
        printlog "Not in Writer/Web!"
        goto endsub
    end select

    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Enter some text, save and close document"
    Call wTypeKeys("File -> Reload!")
    Call hFileSaveAsKill ( gOfficePath + "user\work\" + sFile$ )
    FileClose
    Kontext
    if MessageBox.Exists (1) then
        WarnLog "Messagebox after saving and closing : " + MessageBox.GetText
        MessageBox.Yes
    end if
    Wait (500)

    PrintLog "Open previous saved document"
    Call hFileOpen ( gOfficePath + "user\work\" + sFile$ )
    PrintLog "Insert a table in document"
    Call hTabelleEinfuegen
    PrintLog "File / Reload"
    FileReload
    WaitSlot(1000)
    PrintLog "Quit messagebox with 'No'"
    Kontext "Active"
    Active.No
    Wait (500)

    PrintLog "File / Reload again and quit messagebox with yes"
    FileReload

    WaitSlot(2000)
    Kontext "Active"
    Active.Yes
    Wait (500)
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileVersions
    Dim sFile$
    Dim WriterFilterExtension(10) as string
    Dim MasterDocFilterExtension(10) as string

    WriterFilterExtension() = hGetFilternameExtension ( "writer8" )
    MasterDocFilterExtension() = hGetFilternameExtension ( "writerglobal8" )

    PrintLog "- File / Versions"
    Select Case gApplication
    Case "WRITER"
        sFile$ = "kann_weg." & WriterFilterExtension(0)
    Case "MASTERDOC"
        sFile$ = "kann_weg." & MasterDocFilterExtension(0)
    Case "HTMLDOKUMENT"
        printlog "Not in Writer/Web!"
        goto endsub
    end select

    PrintLog "Open new document"
    Call hNewDocument
    Call wTypeKeys ("File version")
    PrintLog "enter some text in documet and save it"
    Call hFileSaveAsKill(gOfficePath + "user\work\" + sFile$)
    Sleep 1

    try
        PrintLog "Choose File / Versions"
        FileVersions
    catch
        Warnlog "- File / Versions not accessible!"
        goto endsub
    endcatch

    PrintLog "In Version-dialog click 'Save New Version'"
    Kontext "Versionen"
    Call DialogTest ( Versionen )
    Speichern.Click
    PrintLog "Cancel dialog 'Insert version comment'"
    Kontext "VersionskommentarEingeben"
    Call DialogTest ( VersionskommentarEingeben )
    VersionskommentarEingeben.Cancel

    PrintLog "Close dialog"
    Kontext "Versionen"
    Versionen.Close

    PrintLog "Close documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-----------------------------------------------------------

testcase tFileSendCreateMasterDocument
    PrintLog "- File / Send / Create Masterdocument"
    if gApplication <> "WRITER" then
        printlog "Not in : " + gApplication
        goto endsub
    end if

    PrintLog "Open new document"
    Call hNewDocument
    Call wTypeKeys("File -> Send -> Create Master Document")
    PrintLog "Choose File / Send /Create Master Document"
    FileSendCreateMasterdocument
    Kontext "SendMasterDocument"

    PrintLog "Cancel File-Save-dialog"
    SendMasterdocument.Cancel
    Kontext "Active"
    if Active.Exists then Active.OK
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileSendCreateHTMLDocument
    PrintLog "- File / Send / Create HTML-Dokument"
    PrintLog "Open new document"
    Select Case gApplication
    Case "WRITER"
        Call hNewDocument
    Case "MASTERDOC"
        Call hNewDocument
    Case "HTMLDOKUMENT"
        printlog "Not in WriterWeb!"
        goto endsub
    end select
    PrintLog "Choose File / Send / Create HTML-Document"
    FileSendCreateHTMLDocument

    PrintLog "Cancel Filedialog"
    Kontext "SendHTMLDocument"
    SendHTMLDocument.Cancel
    Kontext "Active"
    if Active.Exists then Active.OK

    PrintLog "Close documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-----------------------------------------------------------

testcase tFileSendOutlineToPresentation
    Dim sFile$
    PrintLog "- File / Send / Outline to Presentation"
    Select Case gApplication
    Case "WRITER"
        sFile$ = "htmltest.sxw"
    Case "MASTERDOC"
        sFile$ = "htmltest.sxg"
    Case "HTMLDOKUMENT"
        printlog "Not in Writer/Web!"
        goto endsub
    end select
    PrintLog "Open file ( gTesttoolPath + writer\required\input\ + htmltest.sxw )"
    Call hFileOpen ( gTesttoolPath + "writer\required\input\" + sFile$ )
    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    Sleep 1
    Kontext "DocumentWriter"
    PrintLog "File / Send / Outline to presentation"
    try
        FileSendOutlineToPresentation
    catch
        Warnlog "#i38723#Unable to execute 'File -> Send -> Outline to Presentation'"
        Call hCloseDocument
        goto endsub
    endcatch
    Wait(500)

    Kontext "DocumentImpressOutlineView"
    if DocumentImpressOutlineView.Exists then
        DocumentImpressOutlineView.TypeKeys "<Mod1 Home>"
        DocumentImpressOutlineView.TypeKeys "<Shift End>"
        Wait(500)
        try
            EditCopy
            if GetClipboardText <> "Seite 1." then QAErrorlog "Wrong Outline in Impress Outlinemode"
        catch
            QAErrorlog "Send outline to presentation doesn't work correct."
        endcatch
    else
        WarnLog "Outlinemode comes not up!"
    end if

    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    PrintLog "Close documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-----------------------------------------------------------

testcase tFileSendOutlineToClipboard
    Dim sFile$
    PrintLog "- File / Send / Outline to Clipboard"
    Select Case gApplication
    Case "WRITER"
        sFile$ = "htmltest.sxw"
    Case "MASTERDOC"
        sFile$ = "htmltest.sxg"
    Case "HTMLDOKUMENT"
        printlog "Not in Writer/Web!"
        goto endsub
    end select

    PrintLog "Open file ( gTesttoolPath + writer\required\input\ + htmltest.sxw )"
    Call hFileOpen ( gTesttoolPath + "writer\required\input\" + sFile$ )
    Sleep 1
    PrintLog "File / Send / Outline to Clipboard"
    try
        FileSendOutlineToClipboard
    catch
        Warnlog "#i38723#Unable to execute 'File -> Send -> Outline to Clipboard'"
        Call hCloseDocument
        goto endsub
    endcatch

    Sleep 1
    PrintLog "Close active document"
    Call hCloseDocument
    WaitSlot(1000)
    Call hNewDocument
    PrintLog "Open new document"

    WaitSlot(1000)

    PrintLog "Paste outline in new document"
    EditPaste
    Call wTypeKeys("<Up>", 6)
    Call wTypeKeys("<Home><Shift End>")
    EditCopy
    if GetClipboardText <> "Seite 1." then QAErrorlog "Wrong Outline in Impress Outlinemode"
    WaitSlot(1000)
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileSendCreateAutoabstract
    Dim sFile$
    PrintLog "- File / Send / Create Autoabstract"
    Select Case gApplication
    Case "WRITER"
        sFile$ = "htmltest.sxw"
    Case "MASTERDOC"
        sFile$ = "htmltest.sxg"
    Case "HTMLDOKUMENT"
        printlog "Not in Writer/Web!"
        goto endsub
    end select

    PrintLog "Open file ( gTesttoolPath + writer\required\input\ + htmltest.sxw )"
    Call hFileOpen ( gTesttoolPath + "writer\required\input\" + sFile$ )
    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    Sleep 1
    Kontext "DocumentWriter"
    PrintLog "File / Send / Create Autoabstract"
    try
        FileSendCreateAutoabstract
    catch
        Warnlog "#i38723#Unable to execute 'File -> Send -> Create Autoabstract'"
        Call hCloseDocument
        goto endsub
    endcatch
    WaitSlot(2000)

    PrintLog "Close upcoming dialog with 'Ok"
    Kontext "AutoAbstractErzeugen"
    AutoAbstractErzeugen.OK
    Sleep 1
    Call wTypeKeys("<Home><Up>", 10)
    Call wTypeKeys("<Shift End>")
    try
        EditCopy
        WaitSlot(1000)
        if GetClipboardText <> "Seite 1." then QAErrorlog "Wrong Outline in Impress Outlinemode"
    catch
        QAErrorlog "Empty autoabstract created"
    endcatch

    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    Sleep 1

    PrintLog "Close active documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-----------------------------------------------------------

testcase tFileSendAutoabstractToPresentation
    Dim sFile$
    PrintLog "- File / Send / Autoabstract to Presentation"
    Select Case gApplication
    Case "WRITER"
        sFile$ = "htmltest.sxw"
    Case "MASTERDOC"
        sFile$ = "htmltest.sxg"
    Case "HTMLDOKUMENT"
        printlog "Not in Writer/Web!"
        goto endsub
    end select

    PrintLog "Open file ( gTesttoolPath + writer\required\input\ + htmltest.sxw )"
    Call hFileOpen ( gTesttoolPath + "writer\required\input\" + sFile$ )
    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    Sleep 1
    Kontext "DocumentWriter"
    PrintLog "File / Send / Create Autoabstract To Presentation"
    try
        FileSendAutoabstractToPresentation
    catch
        Warnlog "#i38723#Unable to execute 'File -> Send -> Autoabstract to Presentation'"
        Call hCloseDocument
        goto endsub
    endcatch

    Sleep 1
    Kontext "AutoAbstractErzeugen"
    PrintLog "Close upcoming dialog with 'Ok'"
    AutoAbstractErzeugen.OK

    Sleep 2
    Kontext "DocumentImpressOutlineView"
    if DocumentImpressOutlineView.Exists then
        PrintLog "Point cursor to beginning of document"
        DocumentImpressOutlineView.TypeKeys "<Mod1 Home>"
        PrintLog "Mark text"
        DocumentImpressOutlineView.TypeKeys "<Shift End>"
        try
            EditCopy
            WaitSlot(1000)
            if GetClipboardText <> "Seite 1." then QAErrorlog "Wrong Outline in Impress Outlinemode"
        catch
            QAErrorlog "Edit / copy disabled. Maybe data could not be pasted correctly!"
        endcatch
    else
        Warnlog "- Outline in Impress not created!"
    end if
    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    Sleep 1
    Call hCloseDocument
    PrintLog "Close active document"

    Call wDocSetContext
    FileClose
endcase
'--------------------------------------------------------------------------------------------------------------------------------------------

testcase tMasterDocFileSaveWithPassword
    Dim DokumentPfad$
    Dim Datei$
    Dim sFile$
    Dim sFile2$

    Printlog "- File / Save with Password"
    sFile$ = "password.odm"
    sFile2$ = "password2.odm"

    gApplication = "MASTERDOC"
    PrintLog "Open new document"
    Call hNewDocument
    PrintLog "Enter some text into document"
    Call wTypeKeys("This document will be saved with a password!")

    if gUseSysDlg = True then goto endsub
    Datei$ = ConvertPath ( gOfficePath + "user\work\" + sFile$ )
    if Dir (Datei$) <> "" then kill Datei$

    PrintLog "Choose File / Save as."
    FileSaveAs
    PrintLog "enter destination and check 'Password'"
    Kontext "SpeichernDlg"
    if Passwort.IsEnabled then
        Passwort.Check
    else
        Warnlog "- Not able to save with Password -> Checkbox is disabled! Maybe default-filter wrong!?"
        SpeichernDlg.Cancel
        PrintLog "if Checkbox is disabled the test stops here!"
        Call hCloseDocument
        goto endsub
    end if

    Dateiname.SetText Datei$
    PrintLog "Save your document"
    Speichern.Click
    Sleep 1
    Kontext "Active"
    if Active.Exists (1) then Active.Yes

    Sleep 1
    PrintLog "Password dialog has to come up"
    Kontext "PasswordFileSave"
    if PasswordFileSave.Exists then
        PrintLog "Enter your password"
        Password.SetText "12345"
        PrintLog "Re-Enter a wrong passwort"
        PasswordConfirm.Settext "54321"
        PasswordFileSave.OK
        Sleep 2
    else
        Warnlog "- Password-Dialog not up !"
    end if

    Kontext "Active"
    Sleep 1
    PrintLog "A messagebox has to come up, warns of wrong confirmation"
    if Active.Exists(1) then
        Active.OK
    else
        Warnlog "Able to save document with wrong password"
    end if

    Kontext "PasswordFileSave"
    if PasswordFileSave.Exists then
        PrintLog "Enter new password, and re-enter the same password"
        Password.SetText "12345"
        PasswordConfirm.SetText "12345"
        PasswordFileSave.OK
    else
        Warnlog "- Password-Dialog not up !"
    end if

    Sleep 1
    PrintLog "Close the document"
    FileClose
    WaitSlot(2000)
    PrintLog "Open previous saved document"
    Call hFileOpen(Datei$)

    Kontext "PasswordFileOpen"
    PrintLog "Enter a wrong password."
    if PasswordFileOpen.Exists then
        PasswortName.SetText "34567"
        PasswordFileOpen.OK
    else
        Warnlog "- Password-Dialog not up or Document couldn't be loaded!"
    end if

    PrintLog "A messagebox has to come up, warns of wrong password"
    Kontext "Active"
    Sleep 1
    if Not Active.Exists(1) then
        Warnlog "Able to load document with wrong password"
    else
        Active.OK
    end if

    Kontext "PasswordFileOpen"
    PrintLog "Close password dialog"
    if PasswordFileOpen.Exists then
        PasswordFileOpen.Cancel
    end if

    Sleep 1
    PrintLog "File / Open again your previous saved document"
    Call hFileOpen(Datei$)
    PrintLog "Enter your password in password dialog"
    Kontext "PasswordFileOpen"
    if PasswordFileOpen.Exists then
        PasswortName.SetText "12345"
        PasswordFileOpen.OK
    else
        Warnlog "- Password-Dialog not up!"
    end if
    Sleep 1

    Datei$ = ConvertPath ( gOfficePath + "user\work\" + sFile2$ )
    if Dir (Datei$) <> "" then kill Datei$
    PrintLog "File / Save as ..."
    FileSaveAs
    Kontext "SpeichernDlg"
    if Not Passwort.IsChecked = True then
        if gPlatgroup = "unx" then
            QAErrorlog "#i73968#Opening a password protected file looses focus after loading"
        else
            Warnlog "#i36015#Checkbox 'Save with password' is not checked in file-dialog !"
        end if
        Passwort.Check
        SpeichernDlg.Cancel
    else
        Dateiname.SetText Datei$
        Speichern.Click
        Kontext "Active"
        if Active.Exists(2) then Active.Yes
        PrintLog "Click on 'Save' in file open dialog"
        Kontext "PasswordFileSave"
        if Not PasswordFileSave.Exists(1) then
            Warnlog "Passworddialog is not coming up on saving document"
        else
            Password.SetText "54321"
            PasswordConfirm.SetText "54321"
            PasswordFileSave.OK
            PrintLog "The password dialog should come up"
            Sleep 1
            FileClose
            PrintLog "File / Close and open your document again"
            Call hFileOpen(Datei$)
            Sleep 1
            Kontext "PasswordFileOpen"
            PrintLog "Now the passwort dialog should come up"
            if Not PasswordFileOpen.Exists(2) then
                Warnlog "Passworddialog is not coming up on reload"
            else
                PasswortName.SetText "54321"
                PasswordFileOpen.OK
            end if
        end if
    end if
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tMasterDocFileReload
    Dim DokumentPfad$
    Dim Datei$
    Dim sFile$
    Dim WriterFilterExtension(10) as string
    Dim MasterDocFilterExtension(10) as string

    WriterFilterExtension() = hGetFilternameExtension ( "writer8" )
    MasterDocFilterExtension() = hGetFilternameExtension ( "writerglobal8" )

    PrintLog "- File / Reload"
    sFile$ = "reload." & MasterDocFilterExtension(0)

    gApplication = "MASTERDOC"
    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Enter some text, save and close document"
    Call wTypeKeys("File -> Reload!")
    Call hFileSaveAsKill ( gOfficePath + "user\work\" + sFile$ )
    FileClose
    Kontext
    if MessageBox.Exists (1) then
        WarnLog "Messagebox after saving and closing : " + MessageBox.GetText
        MessageBox.Yes
    end if
    Wait (500)

    PrintLog "Open previous saved document"
    Call hFileOpen ( gOfficePath + "user\work\" + sFile$ )
    PrintLog "Insert a table in document"
    Call hTabelleEinfuegen
    PrintLog "File / Reload"
    FileReload
    WaitSlot(1000)
    PrintLog "Quit messagebox with 'No'"
    Kontext "Active"
    Active.No
    Wait (500)

    PrintLog "File / Reload again and quit messagebox with yes"
    FileReload

    WaitSlot(2000)
    Kontext "Active"
    Active.Yes
    Wait (500)
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tMasterDocFileVersions
    Dim sFile$
    Dim WriterFilterExtension(10) as string
    Dim MasterDocFilterExtension(10) as string

    WriterFilterExtension() = hGetFilternameExtension ( "writer8" )
    MasterDocFilterExtension() = hGetFilternameExtension ( "writerglobal8" )

    gApplication = "MASTERDOC"
    PrintLog "- File / Versions"
    sFile$ = "kann_weg." & MasterDocFilterExtension(0)

    PrintLog "Open new document"
    Call hNewDocument
    Call wTypeKeys ("File version")
    PrintLog "enter some text in documet and save it"
    Call hFileSaveAsKill(gOfficePath + "user\work\" + sFile$)

    try
        PrintLog "Choose File / Versions"
        FileVersions
    catch
        Warnlog "- File / Versions not accessible!"
        goto endsub
    endcatch

    Kontext "Versionen"
    Call DialogTest ( Versionen )
    Speichern.Click
    PrintLog "In Version-dialog click 'Save New Version'"
    Kontext "VersionskommentarEingeben"
    Call DialogTest ( VersionskommentarEingeben )
    VersionskommentarEingeben.Cancel
    PrintLog "Cancel dialog 'Insert version comment'"

    Kontext "Versionen"
    PrintLog "Close dialog"
    Versionen.Close
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tMasterDocFileSendCreateHTMLDocument
    PrintLog "- File / Send / Create HTML-Dokument"
    gApplication = "MASTERDOC"
    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Choose File / Send / Create HTML-Document"
    FileSendCreateHTMLDocument

    PrintLog "Cancel Filedialog"
    Kontext "SendHTMLDocument"
    SendHTMLDocument.Cancel
    Kontext "Active"
    if Active.Exists then Active.OK
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tMasterDocFileSendOutlineToPresentation
    Dim sFile$
    PrintLog "- File / Send / Outline to Presentation"
    gApplication = "MASTERDOC"
    sFile$ = "htmltest.sxg"

    PrintLog "Open file ( gTesttoolPath + writer\required\input\ + htmltest.sxw )"
    Call hFileOpen ( gTesttoolPath + "writer\required\input\" + sFile$ )
    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    Sleep 1
    Kontext "DocumentWriter"
    PrintLog "File / Send / Outline to presentation"
    try
        FileSendOutlineToPresentation
    catch
        Warnlog "#i38723#Unable to execute 'File -> Send -> Outline to Presentation'"
        Call hCloseDocument
        goto endsub
    endcatch
    Wait(500)

    Kontext "DocumentImpressOutlineView"
    if DocumentImpressOutlineView.Exists then
        DocumentImpressOutlineView.TypeKeys "<Mod1 Home>"
        DocumentImpressOutlineView.TypeKeys "<Shift End>"
        Wait(500)
        try
            EditCopy
            if GetClipboardText <> "Seite 1." then QAErrorlog "Wrong Outline in Impress Outlinemode"
        catch
            QAErrorlog "Send outline to presentation doesn't work correct."
        endcatch
    else
        WarnLog "Outlinemode comes not up!"
    end if

    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    PrintLog "Close documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-----------------------------------------------------------

testcase tMasterDocFileSendOutlineToClipboard
    gApplication = "MASTERDOC"
    Dim sFile$
    PrintLog "- File / Send / Outline to Clipboard"
    sFile$ = "htmltest.sxg"

    PrintLog "Open file ( gTesttoolPath + writer\required\input\ + htmltest.sxw )"
    Call hFileOpen ( gTesttoolPath + "writer\required\input\" + sFile$ )
    Sleep 1
    PrintLog "File / Send / Outline to Clipboard"
    try
        FileSendOutlineToClipboard
    catch
        Warnlog "#i38723#Unable to execute 'File -> Send -> Outline to Clipboard'"
        Call hCloseDocument
        goto endsub
    endcatch

    Sleep 1
    PrintLog "Close active document"
    Call hCloseDocument
    WaitSlot(1000)
    PrintLog "Open new document"
    Call hNewDocument
    WaitSlot(1000)
    PrintLog "Paste outline in new document"
    EditPaste
    Call wTypeKeys("<Up>", 6)
    Call wTypeKeys("<Home><Shift End>")
    EditCopy
    if GetClipboardText <> "Seite 1." then QAErrorlog "Wrong Outline in Impress Outlinemode"
    WaitSlot(1000)
    PrintLog "Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tMasterDocFileSendCreateAutoabstract
    gApplication = "MASTERDOC"
    Dim sFile$
    PrintLog "- File / Send / Create Autoabstract"
    sFile$ = "htmltest.sxg"

    PrintLog "Open file ( gTesttoolPath + writer\required\input\ + htmltest.sxw )"
    Call hFileOpen ( gTesttoolPath + "writer\required\input\" + sFile$ )
    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    Sleep 1
    Kontext "DocumentWriter"
    PrintLog "File / Send / Create Autoabstract"
    try
        FileSendCreateAutoabstract
    catch
        Warnlog "#i38723#Unable to execute 'File -> Send -> Create Autoabstract'"
        Call hCloseDocument
        goto endsub
    endcatch
    WaitSlot(2000)

    PrintLog "Close upcoming dialog with 'Ok"
    Kontext "AutoAbstractErzeugen"
    AutoAbstractErzeugen.OK
    Sleep 1
    Call wTypeKeys("<Home><Up>", 10)
    Call wTypeKeys("<Shift End>")
    try
        EditCopy
        WaitSlot(1000)
        if GetClipboardText <> "Seite 1." then QAErrorlog "Wrong Outline in Impress Outlinemode"
    catch
        QAErrorlog "Empty autoabstract created"
    endcatch

    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    Sleep 1

    PrintLog "Close documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tMasterDocFileSendAutoabstractToPresentation
    gApplication = "MASTERDOC"
    Dim sFile$
    PrintLog "- File / Send / Autoabstract to Presentation"
    sFile$ = "htmltest.sxg"

    PrintLog "Open file ( gTesttoolPath + writer\required\input\ + htmltest.sxw )"
    Call hFileOpen ( gTesttoolPath + "writer\required\input\" + sFile$ )
    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    Sleep 1
    Kontext "DocumentWriter"
    PrintLog "File / Send / Create Autoabstract To Presentation"
    try
        FileSendAutoabstractToPresentation
    catch
        Warnlog "#i38723#Unable to execute 'File -> Send -> Autoabstract to Presentation'"
        Call hCloseDocument
        goto endsub
    endcatch

    Sleep 1
    Kontext "AutoAbstractErzeugen"
    PrintLog "Close upcoming dialog with 'Ok'"
    AutoAbstractErzeugen.OK

    Sleep 2
    Kontext "DocumentImpressOutlineView"
    if DocumentImpressOutlineView.Exists then
        PrintLog "Point cursor to beginning of document"
        DocumentImpressOutlineView.TypeKeys "<Mod1 Home>"
        PrintLog "Mark text"
        DocumentImpressOutlineView.TypeKeys "<Shift End>"
        try
            EditCopy
            WaitSlot(1000)
            if GetClipboardText <> "Seite 1." then QAErrorlog "Wrong Outline in Impress Outlinemode"
        catch
            QAErrorlog "Edit / copy disabled. Maybe data could not be pasted correctly!"
        endcatch
    else
        Warnlog "- Outline in Impress not created!"
    end if
    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    Sleep 1
    Call hCloseDocument
    PrintLog "Close active document"

    Call wDocSetContext
    FileClose
    gApplication = "WRITER"
endcase
'--------------------------------------------------------------------------------------------------------------------------------------------
