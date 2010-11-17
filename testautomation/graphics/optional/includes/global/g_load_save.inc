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
'**************************************************************************************
' #1 tFileOpenSaveEtc
' #1 tFileProperties
' #1 tFilePassword52
' #1 tFileOpenImport
' #1 tTestDXF255CharBug
' #1 tDeletedTemplate
' #1 tSaveLoadLayoutOLE
' #1 tSaveLoadLayoutEmpty
' #1 tSaveLoadLayoutText
' #1 tSaveLoadLayoutOutline
' #1 tSaveLoadLayoutPicture
' #1 tSaveLoadLayoutChart
' #1 tSaveLoadLayoutSpreadsheet
'\*************************************************************************************
testcase tFileOpenSaveEtc
    Dim Dokument$
    Dim Dokument2$

      Dokument$ = ConvertPath ( gOfficePath + "user\work\test1")
      Dokument2$ = ConvertPath ( gOfficePath + "user\work\test1_2")

    if (gApplication = "IMPRESS") then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

   printlog "delete previous created documents: 'user\\work\\test1*.od?'"
   if Dir (Dokument$ + ".odp") <> "" then kill (Dokument$ + ".odp")
   if Dir (Dokument2$ + ".odg") <> "" then kill (Dokument2$ + ".odg")

   printlog "open application"
   hNewDocument
   gMouseclick 90,90
   printlog "create a textframe with content"
   Call hTextrahmenErstellen ( "This is a test",20,20,40,30 )
   printlog "save file as 'test1'"
   hFileSaveAsKill (Dokument$ + "." + ExtensionString)
   Sleep 10
   printlog "close Document"
   hCloseDocument
   printlog "Document Saved and Closed"

   printlog "load document 'test1'"
   hFileOpen (Dokument$ + "." + ExtensionString)

    Sleep 10
    printlog "close Document"
    hCloseDocument
    printlog "- document Loaded and closed"
    Sleep 10
    printlog "load document 'test1'"
    hFileOpen (Dokument$ + "." + ExtensionString)
    printlog "check if the document is writable"
    if fIsDocumentWritable = false then
        printlog "make the document writable and check if it's succesfull"
        if fMakeDocumentWritable = false then
            warnlog "The document can't be make writeable. Test stopped."
            goto endsub
        endif
    endif
    Sleep 10
    gMouseclick 90,90
    printlog "create another textbox with content"
    Call hTextrahmenerstellen ( "Change",60,60,80,80 )
    printlog "File->Close needn't work, just say CANCEL to the question about saving changes!"
    FileClose
    Kontext
    printlog "message about 'something changed should i [overwrite discard cancel]?'"
    if Messagebox.exists (5) then
      printlog "2:"+messagebox.gettext
      Messagebox.Cancel
    else
      Warnlog "Mesagebox is missing :-("
    endif
    printlog "Document loaded, changed and closed discarding all changes"

    printlog "Last version"
    try
        printlog "File Reload"
        FileReload
    catch
        kontext "DocumentImpress"
    try
        printlog "File Reload"
        FileReload
   catch
           warnlog "BREAKPOINT! File reload doesn't work. Create new object and try again :-("
       endcatch
   endcatch
   Kontext
   Messagebox.Yes
   Sleep 10
   gMouseDoubleClick 70,70
   hTypeKeys "<Down>", 3
   hTypeKeys "<SHIFT LEFT>",10
   hTypeKeys "<MOD1 C>"
   Sleep 1
   if GetClipboardText = "Change" then Warnlog "- Last version not loaded. Changes in the document still in."
   printlog "- Change document, save it under different name"
   Kontext "DocumentImpress"
   printlog "new textbox"
   Call hTextrahmenErstellen ("Change2",10,10,50,20 )		
   gMouseclick 90,90
   sleep 1
    FileSaveAs
    Kontext "SpeichernDlg"
    Dateiname.SetText Dokument2$
    Speichern.Click
    kontext "AlienWarning"
    if AlienWarning.exists(5) then
     warnlog "#i41983# Alien Warning on export not allowed"
     AlienWarning.OK
    endif
    Kontext "Messagebox"
    if Messagebox.Exists(2) then Messagebox.Yes
   Sleep 3
   printlog "Close document"
   FileClose								
   sleep 10
      Kontext "Messagebox"
      if Messagebox.Exists Then
          Warnlog "- Document saved, but Msgbox appeared, even no changes were made: " + Messagebox.GetText
          Messagebox.Yes
      end if
      sleep 2
      printlog "- Loading write protected file"
      FileOpen
      printlog "Open write protected file"
      Kontext "OeffnenDlg"
      Dateiname.SetText (Dokument2$ + "." + ExtensionString)
      NurLesen.Check
      Oeffnen.Click
    Sleep 10
    printlog "check if the document is writable"
    try
        printlog "attempt to create a textbox"
        Call hTextrahmenErstellen ( "Hello",85,85,95,95 )
        hTypeKeys "<SHIFT LEFT>",5
        hTypeKeys "<MOD1 C>"
        sleep 1
        if GetClipboardText = "Hello" then Warnlog "Write protected document could be edited"
   catch
        Printlog "- Write protected document could not be edited"
   endcatch
   sleep 5
   printlog "Close document"
   FileClose								
   
endcase 'tFileOpenSaveEtc
'-------------------------------------------------------------------------------
testcase tFileProperties

    qaerrorlog "#i88163#, File/properties/user defined fields get mixed up."
    goto endsub
    Call hNewDocument
    Call hTextrahmenErstellen ("This is a document to test the Menupoint in File/Properties",30,30,60,40)
    if (gApplication = "IMPRESS") then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if
    FileProperties
    Kontext
    active.SetPage TabDokumentinfo
    Kontext "TabDokumentinfo"
    Titel.SetText "Properties"
    Thema.SetText "Documentproperties"
    Schluesselworte.SetText "None"

    qaerrorlog "#i95523#"
    'Kontext
    'active.SetPage TabBenutzer
    'Kontext "TabBenutzer"
    'Info2.SetText "Thorsten Peters"

    'Infofelder.Click
    'Kontext "InfonamenBearbeiten"
    'Namen2.SetText "Name"
    'InfonamenBearbeiten.OK
    'sleep 2
    Kontext
    active.SetPage TabInternet
    Kontext "TabInternet"
    RefreshThisDocument.Check

    TabInternet.OK
    hFileSaveAsKill ConvertPath (gOfficePath + "user\work\eigen." & ExtensionString)
    FileClose
    Kontext "Active"
    if Active.Exists(2) then Active.Yes

        PrintLog "- File / Properties  compare"
        Sleep 2
        hFileOpen ConvertPath (gOfficePath + "user\work\eigen." + ExtensionString)

        printlog "check if the document is writable"
        if fIsDocumentWritable = false then
            printlog "make the document writable and check if it's succesfull"
            if fMakeDocumentWritable = false then
                warnlog "The document can't be make writeable. Test stopped."
                goto endsub
            endif
        endif
    FileProperties
    Kontext
    active.SetPage TabDokumentinfo
    Kontext "TabDokumentinfo"
    if Titel.GetText <> "Properties" then Warnlog "Description / Titel is wrong"
    if Thema.GetText <> "Documentproperties" then Warnlog "Description / Thema is wrong"
    if Schluesselworte.GetText <> "None" then Warnlog "Description / Keywords is wrong"

    qaerrorlog "#i95523#"
    'Kontext
    'active.SetPage TabBenutzer
    'Kontext "TabBenutzer"
    'if Info2.GetText <> "Thorsten Peters" then Warnlog "User Defined / Info2 is wrong"

    'Infofelder.Click
    'sleep 2
    'Kontext "InfonamenBearbeiten"
    'if Namen2.GetText <> "Name" then Warnlog "Edit Field Names / Info2 is wrong"
    'InfonamenBearbeiten.OK

    Kontext
    active.SetPage TabInternet
    Kontext "TabInternet"
    if RefreshThisDocument.IsChecked = FALSE then Warnlog "Internet / 'Refresh this document' is not checked"

    TabInternet.Close

    Call hCloseDocument
endcase 'tFileProperties

'-------------------------------------------------------------------------------

testcase tFilePassword52
    Dim Datei$
    dim i as integer
    dim x as integer
    dim y as integer
    dim sExt() as string

    sExt = array("d","a")
    for i = 0 to 1
        Datei$ = ConvertPath ( gTesttoolPath + "graphics\required\input\passwo52.sd"+sExt(i) )
        hFileOpen(Datei$)
        sleep 2
        Kontext "passwordFileOpen"
        if passwordFileOpen.exists(5) then
            PasswortName.SetText "12345"
            passwordFileOpen.OK
            Printlog "Ok, password dialog came up :-) for file : '" + datei$ + "'"
        else
            Warnlog "No password dialog came up :-( for file : '" + datei$ + "'"
        endif
        Kontext "Messagebox"
        if Messagebox.Exists(2) then
            Warnlog "messagebox ?: '" + Messagebox.GetText + "'"
        endif
        hCloseDocument
    next i
endcase 'tFilePassword52

'-------------------------------------------------------------------------------

testcase tFileOpenImport
    dim q as integer
    call hNewDocument
    FileOpen
    Kontext "OeffnenDlg"
    Dateiname.SetText (ConvertPath (gTesttoolpath + "graphics\required\input\imsusmap.cgm"))
    Oeffnen.Click
    Sleep 2
    
    call fMakeDocumentWritable
    
    kontext "DocumentImpress"
    if DocumentImpress.exists(5) then
        DocumentImpress.TypeKeys ("<Tab>")
    else
        warnlog "Not loaded in impress"
    endif
    try
        ContextPositionAndSize
        kontext
        messagebox.setpage TabPositionAndSize
        sleep 1
        TabPositionAndSize.Close
    catch
        warnlog "Documents of Type 'ComputerGraphicsMetafile' can't get loaded :-("
    endcatch
    Call hCloseDocument
    if (gApplication="DRAW") then
        Call hCloseDocument
    endif
endcase 'tFileOpenImport

'-------------------------------------------------------------------------------
testcase tTestDXF255CharBug

    printlog "tries to load a DXF-document which has a string-leght of 255 characters."
    dim q as integer
    call hNewDocument
    printlog "insert graphic "
    InsertGraphicsFromFile
    sleep 1
    kontext "Active"
    sleep 1
    if Active.Exists Then
        Active.OK
    end if
    sleep 1
    Kontext "GrafikEinfuegenDlg"
    sleep 2
    Dateiname.SetText (ConvertPath (gTesttoolpath + "graphics\required\input\255chars.dxf"))
    Oeffnen.Click
    Sleep 2

   if gApplication = "DRAW" then
      Kontext "Documentdraw"
      if Documentdraw.exists(5) then
         Documentdraw.TypeKeys ("<Tab>")
      else
         warnlog "Not loaded in draw"
      endif
   else '(Impress)
      Kontext "DocumentImpress"
      if DocumentImpress.exists(5) then
         DocumentImpress.TypeKeys ("<Tab>")
      else
         warnlog "Not loaded in impress"
      endif
   endif

   try
      ContextPositionAndSize
      kontext
      messagebox.setpage TabPositionAndSize
      sleep 1
      TabPositionAndSize.Close
   catch
      warnlog "DXF-Document could not be loaded :-("
   endcatch
   Call hCloseDocument
   
endcase 'tTestDXF255CharBug
'-------------------------------------------------------------------------------
testcase tDeletedTemplate

    dim i as integer
    dim Dokument1$
    dim Dokument2$
    dim SavedActiveMessage as string
    dim found as boolean

    if (gApplication = "IMPRESS") then 
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if
    
    Dokument1$ = ConvertPath ( gOfficePath + "user\work\test1." & ExtensionString)
    Dokument2$ = ConvertPath ( gOfficePath + "user\work\test2." & ExtensionString)

    

   printlog "delete previous created documents: 'user\\work\\test*.odp'"
   if Dir (Dokument1$) <> "" then kill Dokument1$
   if Dir (Dokument2$) <> "" then kill Dokument2$

   printlog "open application"
   hNewDocument
   gMouseclick 90,90
   printlog "create a textframe with content"
   Call hTextrahmenErstellen ( "This is the first file",20,20,40,30 )
   printlog "save file as 'test1.odp'"
   hFileSaveAsKill (Dokument1$)
   Sleep 10
   printlog "close Document"
   hCloseDocument
   printlog "-  Document 'test1.odp' Saved and Closed"

   printlog "open application again"
   hNewDocument
   gMouseclick 90,90
   printlog "create another textframe with content"
   Call hTextrahmenErstellen ( "This is the second file",20,20,40,30 )
   printlog "save file as 'test2.odp'"
   hFileSaveAsKill (Dokument2$)
   Sleep 10
   printlog "close Document"
   hCloseDocument
   printlog "-  Document 'test2.odp' Saved and Closed"

   printlog "Delete the first file"
   try
       kill Dokument1$
       Printlog "   Correctly deleted the file: " + Dokument1$
   catch
       warnlog "Could not delete the file: " + Dokument1$
   endcatch

    printlog "File => New => Presentation"
    printlog "new document-autopilot"
    FileAutopilotPresentation
    Kontext "AutopilotPraesentation1"
    sleep 5
    Existingpresentation.Check
    sleep 5
    kontext "active"
    if active.exists then
    SavedActiveMessage = Active.GetText
    QaErrorLog "Message: " + SavedActiveMessage
    active.ok
    Kontext "AutopilotPraesentation1"
    if Existinglist.GetSelText = "test1.odp" then
    warnlog "   The deleted File was found. Messagebox said: " + SavedActiveMessage
    endif
    SavedActiveMessage = ""
    endif
    Kontext "AutopilotPraesentation1"
    Existinglist.TypeKeys "<HOME>"
    sleep 5
    found = false
    for i = 1 to Existinglist.GetItemCount
          kontext "active"
          if active.exists then
             SavedActiveMessage = Active.GetText
             active.ok
          endif
          Kontext "AutopilotPraesentation1"
          if Existinglist.GetSelText = "test1.odp" then
             found = true
             if SavedActiveMessage <> "" then
                warnlog "   The deleted File was found. Messagebox said: " + SavedActiveMessage
             else
                warnlog "   The deleted File was found. However without any messagebox."
             endif
          endif
          Existinglist.TypeKeys "<DOWN>"
          sleep 5
      next i
      if found = true then warnlog "   An error-message showed up. Eventually a result of '138498'"
      AutopilotPraesentation1.Cancel
   Sleep 2
   printlog "Delete the second file"
    try
        kill Dokument2$
        Printlog "   Correctly deleted the file: " + Dokument2$
    catch
        warnlog "Could not delete the file: " + Dokument2$
    endcatch

endcase 'tDeletedTemplate
'-------------------------------------------------------------------------------

testcase tSaveLoadLayoutEmpty
    Dim NewFileDir as String

    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    printlog "   Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    gApplication = "IMPRESS"
    Call hNewDocument

    printlog "Choose and Insert an empty Layout."
    FormatModifyPage
    sleep 1
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<HOME>"
    kontext "Pagelayout_UndoDeleteWarning"
    if Pagelayout_UndoDeleteWarning.exists then
        Pagelayout_UndoDeleteWarning.ok
    endif
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<RETURN>"
    sleep (5)

    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.
    printlog "Will try to delete the directory: " + NewFileDir
    app.rmDir NewFileDir

endcase 'tSaveLoadLayoutEmpty

'-------------------------------------------------------------------------------

testcase tSaveLoadLayoutOLE
    qaerrorlog "not yet ready"
    goto endsub
    Dim NewFileDir as String

    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    printlog "Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir


    gApplication = "IMPRESS"
    Call hNewDocument

    printlog "Choose and Insert an Layout with an OLE."
    FormatModifyPage
    sleep 1
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<HOME>"
    kontext "Pagelayout_UndoDeleteWarning"
    if Pagelayout_UndoDeleteWarning.exists then
        Pagelayout_UndoDeleteWarning.ok
    endif
    kontext "Tasks"
    LayoutsPreview.TypeKeys ("<RIGHT>",12)
    sleep (1)
    LayoutsPreview.TypeKeys ("<Return>")
    sleep (1)
    hTypeKeys ("<TAB><TAB><TAB>")
    hTypeKeys ("<Return>")
    try
        sleep (2)
        kontext "OLEObjektInsert"
        sleep (1)
        OLEObjektInsert.Cancel
        Printlog "Inserted a layout with an 'Insert OLE-Object'-frame. Correct."
    catch
        warnlog "Ole wasn't selected :-("
        CALL hCloseDocument
        Goto Endsub
    endcatch
    sleep (5)

    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    printlog "Will try to delete the directory: " + NewFileDir
    app.rmDir ConvertPath (gOfficePath + "user\work\LayoutTest\")' NewFileDir

endcase 'tSaveLoadLayoutOLE

'-------------------------------------------------------------------------------

testcase tSaveLoadLayoutText
    qaerrorlog "not yet ready"
    goto endsub
    Dim NewFileDir as String

    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    printlog "   Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    gApplication = "IMPRESS"
    Call hNewDocument

    printlog "Choose and Insert an Layout with a Text-field."
    FormatModifyPage
    sleep 1
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<HOME>"
    kontext "Pagelayout_UndoDeleteWarning"
    if Pagelayout_UndoDeleteWarning.exists then
        Pagelayout_UndoDeleteWarning.ok
    endif
    kontext "Tasks"
    LayoutsPreview.TypeKeys ("<RIGHT>")
    sleep (1)
    LayoutsPreview.TypeKeys ("<Return>")
    sleep (1)
    hTypeKeys ("<TAB><TAB><TAB>")
    hTypeKeys ("<Return>")
    hTypeKeys ("<ESCAPE>")
    sleep (5)

    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    printlog "Will try to delete the directory: " + NewFileDir
    app.rmDir ConvertPath (gOfficePath + "user\work\LayoutTest\")' NewFileDir

endcase 'tSaveLoadLayoutText

'-------------------------------------------------------------------------------

testcase tSaveLoadLayoutOutline
    qaerrorlog "not yet ready"
    goto endsub
    Dim NewFileDir as String

    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    printlog "   Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    gApplication = "IMPRESS"
    Call hNewDocument

    printlog "Choose and Insert an Layout with a Outline-field."
    FormatModifyPage
    sleep 1
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<HOME>"
    kontext "Pagelayout_UndoDeleteWarning"
    if Pagelayout_UndoDeleteWarning.exists then
        Pagelayout_UndoDeleteWarning.ok
    endif
    kontext "Tasks"
    LayoutsPreview.TypeKeys ("<RIGHT>",3)
    sleep (1)
    LayoutsPreview.TypeKeys ("<Return>")
    sleep (1)
    hTypeKeys ("<TAB><TAB><TAB>")
    hTypeKeys ("<Return>")
    hTypeKeys ("<ESCAPE>")
    sleep (5)

    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    printlog "   Will try to delete the directory: " + NewFileDir
    app.rmDir ConvertPath (gOfficePath + "user\work\LayoutTest\")' NewFileDir

endcase 'tSaveLoadLayoutOutline

'-------------------------------------------------------------------------------

testcase tSaveLoadLayoutPicture
    qaerrorlog "not yet ready"
    goto endsub
    Dim NewFileDir as String

    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    printlog "   Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    gApplication = "IMPRESS"
    Call hNewDocument

    printlog "Choose and Insert an Layout with a Graphic-field."
    FormatModifyPage
    sleep 1
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<HOME>"
    kontext "Pagelayout_UndoDeleteWarning"
    if Pagelayout_UndoDeleteWarning.exists then
        Pagelayout_UndoDeleteWarning.ok
    endif
    kontext "Tasks"
    LayoutsPreview.TypeKeys ("<RIGHT>",8)
    sleep (1)
    LayoutsPreview.TypeKeys ("<Return>")
    sleep (1)
    hTypeKeys ("<TAB><TAB><TAB>")
    hTypeKeys ("<Return>")
    hTypeKeys ("<ESCAPE>")
    sleep (5)

    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    printlog "   Will try to delete the directory: " + NewFileDir
    app.rmDir ConvertPath (gOfficePath + "user\work\LayoutTest\")' NewFileDir

endcase 'tSaveLoadLayoutPicture

'-------------------------------------------------------------------------------

testcase tSaveLoadLayoutChart
    qaerrorlog "not yet ready"
    goto endsub
    Dim NewFileDir as String

    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    printlog "   Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    gApplication = "IMPRESS"
    Call hNewDocument

    printlog "Choose and Insert an Layout with a Chart-field."
    FormatModifyPage
    sleep 1
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<HOME>"
    kontext "Pagelayout_UndoDeleteWarning"
    if Pagelayout_UndoDeleteWarning.exists then
        Pagelayout_UndoDeleteWarning.ok
    endif
    kontext "Tasks"
    LayoutsPreview.TypeKeys ("<RIGHT>",6)
    sleep (1)
    LayoutsPreview.TypeKeys ("<Return>")
    sleep (1)
    hTypeKeys ("<TAB><TAB><TAB>")
    hTypeKeys ("<Return>")
    hTypeKeys ("<ESCAPE>")
    sleep (5)

    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    printlog "   Will try to delete the directory: " + NewFileDir
    app.rmDir ConvertPath (gOfficePath + "user\work\LayoutTest\")' NewFileDir

endcase 'tSaveLoadLayoutChart

'-------------------------------------------------------------------------------

testcase tSaveLoadLayoutSpreadsheet
    qaerrorlog "not yet ready"
    goto endsub
    Dim NewFileDir as String

    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    printlog "   Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    gApplication = "IMPRESS"
    Call hNewDocument

    printlog "Choose and Insert an Layout with a Spreadsheet-field."
    FormatModifyPage
    sleep 1
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<HOME>"
    kontext "Pagelayout_UndoDeleteWarning"
    if Pagelayout_UndoDeleteWarning.exists then
        Pagelayout_UndoDeleteWarning.ok
    endif
    kontext "Tasks"
    LayoutsPreview.TypeKeys ("<RIGHT>",7)
    sleep (1)
    LayoutsPreview.TypeKeys ("<Return>")
    sleep (1)
    hTypeKeys ("<TAB><TAB><TAB>")
    hTypeKeys ("<Return>")
    hTypeKeys ("<ESCAPE>")
    sleep (5)

    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    printlog "Will try to delete the directory: " + NewFileDir
    app.rmDir ConvertPath (gOfficePath + "user\work\LayoutTest\")' NewFileDir

endcase 'tSaveLoadLayoutSpreadsheet

'-------------------------------------------------------------------------------

