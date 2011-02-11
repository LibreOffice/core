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
'* owner : helge.delfs@oracle.com
'*
'* short description :  Required-Testcases for the File-menu.
'*
'\***********************************************************************

sub w_001b_

    printLog Chr(13) + "--------- File Menu (w_001b_.inc) ----------"
    gApplication = "WRITER"

    Call tToolsXMLFilterSettings
    Call tFileExportAsPDF
    Call tExportAsPDFButton
    Call tFileSendCreateMasterDocument
    Call tFileSendCreateHTMLDocument
    Call tFileSendOutlineToPresentation
    Call tFileSendOutlineToClipboard
    Call tFileSendCreateAutoabstract
    Call tFileSendAutoabstractToPresentation

    'Writer Master Document
    gApplication = "MASTERDOCUMENT"
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

'-------------------------------------------------------------------------------

testcase tFileSendCreateMasterDocument
    PrintLog "- File / Send / Create Masterdocument"
    if gApplication <> "WRITER" then
        printlog "Not in : " + gApplication
        goto endsub
    endif

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
    Case "WRITER", "MASTERDOCUMENT" : hNewDocument()
    Case else
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
    Case "MASTERDOCUMENT"
        sFile$ = "htmltest.sxg"
    Case "HTML"
        printlog "Not in Writer/Web!"
        goto endsub
    end select
    PrintLog "Open file ( gTesttoolPath + writer\required\input\ + htmltest.sxw )"
    hFileOpenLocally(  gTesttoolPath + "writer\required\input\" + sFile$ )
    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    Sleep 1
    Kontext "DocumentWriter"
    PrintLog "File / Send / Outline to presentation"
    try
        FileSendOutlineToPresentation
    catch
        Warnlog "Unable to execute 'File -> Send -> Outline to Presentation'"
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
    endif

    Kontext "Navigator"
    hCloseDialog( Navigator, "Close,optional" )
    hFileCloseAll()
endcase

'-----------------------------------------------------------

testcase tFileSendOutlineToClipboard
    Dim sFile$
    PrintLog "- File / Send / Outline to Clipboard"
    Select Case gApplication
    Case "WRITER"
        sFile$ = "htmltest.sxw"
    Case "MASTERDOCUMENT"
        sFile$ = "htmltest.sxg"
    Case "HTML"
        printlog "Not in Writer/Web!"
        goto endsub
    end select

    PrintLog "Open file ( gTesttoolPath + writer\required\input\ + htmltest.sxw )"
    hFileOpenLocally(  gTesttoolPath + "writer\required\input\" + sFile$ )
    Sleep 1
    PrintLog "File / Send / Outline to Clipboard"
    try
        FileSendOutlineToClipboard
    catch
        Warnlog "Unable to execute 'File -> Send -> Outline to Clipboard'"
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
    hUseAsyncSlot( "EditPaste" )
    WaitSlot(1000)
    Call wTypeKeys("<Up>", 6)
    Call wTypeKeys("<Home><Shift End>")
    hUseAsyncSlot( "EditCopy" )
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
    Case "MASTERDOCUMENT"
        sFile$ = "htmltest.sxg"
    Case "HTML"
        printlog "Not in Writer/Web!"
        goto endsub
    end select

    PrintLog "Open file ( gTesttoolPath + writer\required\input\ + htmltest.sxw )"
    hFileOpenLocally(  gTesttoolPath + "writer\required\input\" + sFile$ )
    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    Sleep 1
    Kontext "DocumentWriter"
    PrintLog "File / Send / Create Autoabstract"
    try
        FileSendCreateAutoabstract
    catch
        Warnlog "Unable to execute 'File -> Send -> Create Autoabstract'"
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
    hCloseDialog( Navigator, "Close,optional" )
    hFileCloseAll()

endcase

'-----------------------------------------------------------

testcase tFileSendAutoabstractToPresentation
    Dim sFile$
    PrintLog "- File / Send / Autoabstract to Presentation"
    Select Case gApplication
    Case "WRITER"
        sFile$ = "htmltest.sxw"
    Case "MASTERDOCUMENT"
        sFile$ = "htmltest.sxg"
    Case "HTML"
        printlog "Not in Writer/Web!"
        goto endsub
    end select

    PrintLog "Open file ( gTesttoolPath + writer\required\input\ + htmltest.sxw )"
    hFileOpenLocally(  gTesttoolPath + "writer\required\input\" + sFile$ )
    Kontext "Navigator"
    if Navigator.Exists then Navigator.Close
    Sleep 1
    Kontext "DocumentWriter"
    PrintLog "File / Send / Create Autoabstract To Presentation"
    try
        FileSendAutoabstractToPresentation
    catch
        Warnlog "Unable to execute 'File -> Send -> Autoabstract to Presentation'"
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
    endif
    Kontext "Navigator"
    hCloseDialog( Navigator, "Close,optional" )
    Call hCloseDocument
    PrintLog "Close active document"

    hSetDocumentContext()
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

    gApplication = "MASTERDOCUMENT"
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
    endif

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
    endif

    Kontext "Active"
    Sleep 1
    PrintLog "A messagebox has to come up, warns of wrong confirmation"
    if Active.Exists(1) then
        Active.OK
    else
        Warnlog "Able to save document with wrong password"
    endif

    Kontext "PasswordFileSave"
    if PasswordFileSave.Exists then
        PrintLog "Enter new password, and re-enter the same password"
        Password.SetText "12345"
        PasswordConfirm.SetText "12345"
        PasswordFileSave.OK
    else
        Warnlog "- Password-Dialog not up !"
    endif

    Sleep 1
    PrintLog "Close the document"
    FileClose
    WaitSlot(2000)
    PrintLog "Open previous saved document"
    Call hFileOpen( Datei$)

    Kontext "PasswordFileOpen"
    PrintLog "Enter a wrong password."
    if PasswordFileOpen.Exists then
        PasswortName.SetText "34567"
        PasswordFileOpen.OK
    else
        Warnlog "- Password-Dialog not up or Document couldn't be loaded!"
    endif

    PrintLog "A messagebox has to come up, warns of wrong password"
    Kontext "Active"
    Sleep 1
    if Not Active.Exists(1) then
        Warnlog "Able to load document with wrong password"
    else
        Active.OK
    endif

    Kontext "PasswordFileOpen"
    PrintLog "Close password dialog"
    if PasswordFileOpen.Exists then
        PasswordFileOpen.Cancel
    endif

    Sleep 1
    PrintLog "File / Open again your previous saved document"
    Call hFileOpen( Datei$)
    PrintLog "Enter your password in password dialog"
    Kontext "PasswordFileOpen"
    if PasswordFileOpen.Exists then
        PasswortName.SetText "12345"
        PasswordFileOpen.OK
    else
        Warnlog "- Password-Dialog not up!"
    endif
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
        endif
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
            hFileOpen( Datei$)
            Sleep 1
            Kontext "PasswordFileOpen"
            PrintLog "Now the passwort dialog should come up"
            if Not PasswordFileOpen.Exists(2) then
                Warnlog "Passworddialog is not coming up on reload"
            else
                PasswortName.SetText "54321"
                PasswordFileOpen.OK
            endif
        endif
    endif
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tMasterDocFileReload

    Dim sDocumentPath as string
    Dim sFile as string
    Dim WriterFilterExtension(10) as string
    Dim MasterDocFilterExtension(10) as string

    WriterFilterExtension() = hGetFilternameExtension ( "writer8" )
    MasterDocFilterExtension() = hGetFilternameExtension ( "writerglobal8" )

    PrintLog "- File / Reload"
    sFile = "reload." & MasterDocFilterExtension(0)
    sDocumentPath = gOfficePath + "user\work\" + sFile

    gApplication = "MASTERDOCUMENT"
    
    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Enter some text, save and close document"
    Call wTypeKeys("File -> Reload!")
    Call hFileSaveAsKill ( sDocumentPath )
    hUseAsyncSlot( "FileClose" )
    
    Kontext
    if ( MessageBox.Exists( 1 ) ) then
        WarnLog "Messagebox after saving and closing : " + MessageBox.GetText
        MessageBox.Yes
    endif

    PrintLog "Open previous saved document"
    Call hFileOpen(  sDocumentPath )
    
    PrintLog "Insert a table in document"
    Call hTabelleEinfuegen
    
    PrintLog "File / Reload"
    hUseAsyncSlot( "FileReload" )

    PrintLog "Quit messagebox with 'No'"
    Kontext "Active"
    if ( Active.exists( 1 ) ) then
        Active.No
    else
        warnlog( "Messagebox missing" )
    endif

    PrintLog "File / Reload again and quit messagebox with yes"
    hUseAsyncSlot( "FileReload" )

    Kontext "Active"
    if ( Active.exists( 2 ) ) then
        Active.Yes
    else
        warnlog( "Messagebox missing" )
    endif
    
    PrintLog "Close active document"
    Call hCloseDocument
    hDeleteFile( sDocumentPath )
    
endcase

'-----------------------------------------------------------

testcase tMasterDocFileVersions

    Dim sFile as string
    Dim WriterFilterExtension(10) as string
    Dim MasterDocFilterExtension(10) as string

    WriterFilterExtension() = hGetFilternameExtension ( "writer8" )
    MasterDocFilterExtension() = hGetFilternameExtension ( "writerglobal8" )

    gApplication = "MASTERDOCUMENT"
    
    PrintLog "- File / Versions"
    sFile = "kann_weg." & MasterDocFilterExtension(0)

    PrintLog "Open new document"
    Call hNewDocument
    Call wTypeKeys ("File version")
    
    PrintLog "enter some text in documet and save it"
    Call hFileSaveAsKill(gOfficePath + "user\work\" + sFile )

    try
        PrintLog "Choose File / Versions"
        FileVersions
    catch
        Warnlog "- File / Versions not accessible!"
        goto endsub
    endcatch

    Kontext "Versionen"
    if ( Versionen.exists( 2 ) ) then
    
        Call DialogTest ( Versionen )
        Speichern.Click
        
        PrintLog "In Version-dialog click 'Save New Version'"
        Kontext "VersionskommentarEingeben"
        if ( VersionskommentarEingeben.exists( 1 ) ) then
            Call DialogTest ( VersionskommentarEingeben )
            
            PrintLog "Cancel dialog 'Insert version comment'"
            VersionskommentarEingeben.Cancel
        else
            warnlog( "Dialog <VersionskommentarEingeben> is not available" )
        endif

        Kontext "Versionen"
        PrintLog "Close dialog"
        Versionen.Close()
    else
        warnlog( "Dialog <Versionen> is not available" )
    endif
    
    PrintLog "Close active document"
    Call hCloseDocument
    
endcase

'-----------------------------------------------------------

testcase tMasterDocFileSendCreateHTMLDocument

    PrintLog "- File / Send / Create HTML-Dokument"
    
    gApplication = "MASTERDOCUMENT"
    
    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Choose File / Send / Create HTML-Document"
    FileSendCreateHTMLDocument

    PrintLog "Cancel Filedialog"
    Kontext "SendHTMLDocument"
    if ( SendHTMLDocument.exists( 2 ) ) then
        SendHTMLDocument.Cancel
        
        Kontext "Active"
        if Active.Exists( 1 ) then 
            warnlog( "Unexpected messgebox" & Active.getText() )
            Active.OK
        endif
    else
        warnlog( "Dialog <SendHTMLDocument> is not available" )
    endif
        
    PrintLog "Close active document"
    Call hCloseDocument
    
endcase

'-----------------------------------------------------------

testcase tMasterDocFileSendOutlineToPresentation

    PrintLog "- File / Send / Outline to Presentation"

    Dim sFile as string : sFile = "htmltest.sxg"

    gApplication = "MASTERDOCUMENT"

    hFileOpenLocally(  gTesttoolPath + "writer\required\input\" + sFile )
    
    Kontext "Navigator"
    if ( Navigator.Exists ) then 
        Navigator.Close
        WaitSlot()
    endif
    
    Kontext "DocumentWriter"
    PrintLog "File / Send / Outline to presentation"
    try
        FileSendOutlineToPresentation
    catch
        Warnlog "Unable to execute 'File -> Send -> Outline to Presentation'"
        Call hCloseDocument
        goto endsub
    endcatch

    Kontext "DocumentImpressOutlineView"
    if ( DocumentImpressOutlineView.Exists( 2 ) ) then
        DocumentImpressOutlineView.TypeKeys "<Mod1 Home>"
        DocumentImpressOutlineView.TypeKeys "<Shift End>"
        hUseAsyncSlot( "EditCopy" )
        if ( GetClipboardText <> "Seite 1." ) then 
            QAErrorlog "Wrong Outline in Impress Outlinemode"
        endif
    else
        WarnLog "Outlinemode comes not up!"
    endif

    Kontext "Navigator"
    hCloseDialog( Navigator, "Close,optional" )
    
    PrintLog "Close documents"
    hFileCloseAll()
    
endcase

'-----------------------------------------------------------

testcase tMasterDocFileSendOutlineToClipboard

    gApplication = "MASTERDOCUMENT"
    Dim sFile as string : sFile = "htmltest.sxg"

    PrintLog "- File / Send / Outline to Clipboard"
    hFileOpenLocally(  gTesttoolPath + "writer\required\input\" + sFile )
    
    PrintLog "File / Send / Outline to Clipboard"
    try
        FileSendOutlineToClipboard
    catch
        Warnlog "Unable to execute 'File -> Send -> Outline to Clipboard'"
        Call hCloseDocument
        goto endsub
    endcatch

    PrintLog "Close active document"
    Call hCloseDocument
    
    PrintLog "Open new document"
    Call hNewDocument
    
    PrintLog "Paste outline in new document"
    hUseAsyncSlot( "EditPaste" )
    Call wTypeKeys("<Up>", 6)
    Call wTypeKeys("<Home><Shift End>")
    hUseAsyncSlot( "EditCopy" )
    if ( GetClipboardText <> "Seite 1." ) then 
        QAErrorlog "Wrong Outline in Impress Outlinemode"
    endif

    PrintLog "Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
    
endcase

'-----------------------------------------------------------

testcase tMasterDocFileSendCreateAutoabstract

    gApplication = "MASTERDOCUMENT"
    Dim sFile as string : sFile = "htmltest.sxg"

    PrintLog "- File / Send / Create Autoabstract"
    hFileOpenLocally(  gTesttoolPath + "writer\required\input\" + sFile )
    
    Kontext "Navigator"
    hCloseDialog( Navigator, "Close,optional" )

    Kontext "DocumentWriter"
    PrintLog "File / Send / Create Autoabstract"
    try
        FileSendCreateAutoabstract
    catch
        Warnlog "Unable to execute 'File -> Send -> Create Autoabstract'"
        Call hCloseDocument
        goto endsub
    endcatch

    PrintLog "Close upcoming dialog with 'Ok"
    Kontext "AutoAbstractErzeugen"
    if ( AutoAbstractErzeugen.exists( 2 ) ) then
    
        AutoAbstractErzeugen.OK()

        Call wTypeKeys("<Home><Up>", 10)
        Call wTypeKeys("<Shift End>")
        
        hUseAsyncSlot( "EditCopy" )
        if GetClipboardText <> "Seite 1." then 
            QAErrorlog "Wrong Outline in Impress Outlinemode"
        endif
    else
        warnlog( "Dialog <AutoAbstractErzeugen> is not available" )
    endif    

    Kontext "Navigator"
    hCloseDialog( Navigator, "Close,optional" )

    PrintLog "Close documents"
    hFileCloseAll()    
    gApplication = "WRITER"
    
endcase

'-----------------------------------------------------------

testcase tMasterDocFileSendAutoabstractToPresentation

    gApplication = "MASTERDOCUMENT"
    Dim sFile as string : sFile = "htmltest.sxg"
    
    PrintLog "- File / Send / Autoabstract to Presentation"
    PrintLog "Open file ( gTesttoolPath + writer\required\input\ + htmltest.sxw )"
    hFileOpenLocally(  gTesttoolPath + "writer\required\input\" + sFile )
    
    Kontext "Navigator"
    hCloseDialog( Navigator, "Close,optional" )
    
    Kontext "DocumentWriter"
    PrintLog "File / Send / Create Autoabstract To Presentation"
    try
        FileSendAutoabstractToPresentation
    catch
        Warnlog "Unable to execute 'File -> Send -> Autoabstract to Presentation'"
        Call hCloseDocument
        goto endsub
    endcatch

    Kontext "AutoAbstractErzeugen"
    if ( AutoAbstractErzeugen.exists( 1 ) ) then
        PrintLog "Close upcoming dialog with 'Ok'"
        AutoAbstractErzeugen.OK
    else
        warnlog( "Dialog <AutoAbstractErzeugen> is not available" )
    endif

    Kontext "DocumentImpressOutlineView"
    if ( DocumentImpressOutlineView.Exists( 2 ) ) then
    
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
    endif
    
    Kontext "Navigator"
    hCloseDialog( Navigator, "Close,optional" )

    Call hCloseDocument
    PrintLog "Close active document"

    hSetDocumentContext()
    FileClose
    gApplication = "WRITER"
    
endcase
'-------------------------------------------------------------------------------
