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
'* short description: includefile for Ole-testing
'*
'\*************************************************************************************

testcase tOLE_Copy
    qaerrorlog "not running due to reconstruction"
    goto endsub
    printlog "Test to see if three different OLEs can be copied and identified correctly."
    
    dim writerfilename1 as string
    writerfilename1 = ConvertPath (gTesttoolPath + "graphics\required\input\oletest.odg")
    
    printlog "Open draw-file" & writerfilename1
    FileOpen
    Kontext "OeffnenDlg"
    sleep 1
    Dateiname.SetText (writerfilename1)
    Oeffnen.Click
    Sleep 3
    
    printlog "When messagebox about Update all links shows up - press yes."
    kontext "Messagebox"
    if Messagebox.exists then
    Messagebox.Yes
    endif
    
    sleep 3
    gApplication = "DRAW"
    printlog "Select first object. Use EditCopy to copy it to the clipboard"
    sleep 1
    kontext "DocumentDraw"
    DocumentDraw.mousedown (10,10)
    DocumentDraw.mouseup (10,10)
    sleep 1
    DocumentDraw.Typekeys "<TAB>"
    EditCopy
    
    printlog "Close document. Open new Impress-Document. Paste the OLE-Object."
    Call hCloseDocument
    gApplication = "IMPRESS"
    Call hNewDocument
    EditPaste
    
    printlog "Select object and check via the Context-menu what it has been opened as."
    sleep 1
    Kontext "DocumentImpress"
    hTypekeys "<MOD1> A"
    hTypekeys "<RETURN>"
    hOpenContextMenu
    
    printlog "Count the number of entries. If 5 (including a menuseparator), then its a spreadsheet"
    if MenuGetItemCount <> 5 then
        Warnlog "This doesn't seem to be recognized as a Metafile. It has " + MenuGetItemCount + " entries."
    end if

    printlog "Close Context menu. Then check if a Edit-line for the Cells is visible."
    sleep 3
    MenuSelect (1)  'Default
    
    Kontext "RechenleisteCalc"
    if RechenleisteCalc.Exists then
        Printlog "This is a spreadsheet-Document. Good"
    endif
    
    Kontext "DocumentImpress"
    printlog "Click outside the OLE to deactivate it. Then close the Document "
    DocumentImpress.mousedown (10,10)
    DocumentImpress.mouseup (10,10)
    sleep 3
    Call hCloseDocument
    
    printlog "Open Draw-file "
    FileOpen
    Kontext "OeffnenDlg"
    sleep 1
    Dateiname.SetText (writerfilename1)
    Oeffnen.Click
    Sleep 3
    
    printlog " When messagebox about Update all links shows up - press yes."
    kontext "Messagebox"
    if Messagebox.exists then
        Messagebox.Yes
    endif
    sleep 3
    
    printlog "Select second object. Use EditCopy to copy it to the clipboard."
    gApplication = "DRAW"
    sleep 1
    kontext "DocumentDraw"
    DocumentDraw.mousedown (10,10)
    DocumentDraw.mouseup (10,10)
    sleep 1
    DocumentDraw.Typekeys "<TAB><TAB>"
    EditCopy
    
    printlog "Close document. Open new Impress-Document. Paste the OLE-Object."
    sleep 1
    DocumentDraw.mousedown (10,10)
    DocumentDraw.mouseup (10,10)
    sleep 1
    Call hCloseDocument
    
    gApplication = "IMPRESS"
    Call hNewDocument
    EditPaste
    
    printlog "Select the object, and check via the Context-menu what it has been opened as."
    sleep 1
    Kontext "DocumentImpress"
    hTypekeys "<MOD1> A"
    hTypekeys "<RETURN>"
    hOpenContextMenu

    printlog "Count the number of entries. If 5 (including a menuseparator), then its a spreadsheet"
    if MenuGetItemCount <> 5 then
        Warnlog "This doesn't seem to be recognized as a Metafile. It has " + MenuGetItemCount + " entries."
    end if
    
    printlog "Close Context menu. Then check if a Edit-line for the Cells is visible."
    sleep 3
    MenuSelect (1)  'Default
    
    Kontext "RechenleisteCalc"
    if RechenleisteCalc.Exists then
        Printlog "This is a spreadsheet-Document. Good"
    endif
    
    kontext "DocumentImpress"
    sleep 1
    DocumentImpress.mousedown (10,10)
    DocumentImpress.mouseup (10,10)
    sleep 1
    
    printlog "Close document"
    Call hCloseDocument
    
    printlog "Open draw-file"
    FileOpen
    Kontext "OeffnenDlg"
    sleep 1
    Dateiname.SetText (writerfilename1)
    Oeffnen.Click
    Sleep 3
    kontext "Messagebox"
    printlog "When messagebox about Update all links shows up - press yes."
    if Messagebox.exists then
        Messagebox.Yes
    endif
    sleep 3
    
    printlog "Select third object. Use EditCopy to copy it to the clipboard."
    gApplication = "DRAW"
    sleep 1
    kontext "DocumentDraw"
    DocumentDraw.mousedown (10,10)
    DocumentDraw.mouseup (10,10)
    sleep 1
    DocumentDraw.Typekeys "<TAB><TAB><TAB>"
    EditCopy
    
    printlog "Close the Document. Open new Impress-Document. Paste the OLE-Object."
    sleep 1
    DocumentDraw.mousedown (10,10)
    DocumentDraw.mouseup (10,10)
    sleep 1
    Call hCloseDocument

    gApplication = "IMPRESS"
    Call hNewDocument
    EditPaste
    
    printlog "Select the object, and check via the Context-menu what it has been opened as."
    Kontext "DocumentImpress"
    hTypekeys "<MOD1> A"
    hTypekeys "<RETURN>"
    hOpenContextMenu
    
    printlog "Count the number of entries. If 5 (including a menuseparator), then its a spreadsheet"
    if MenuGetItemCount <> 5 then
        Warnlog "This doesn't seem to be recognized as a Metafile. It has " + MenuGetItemCount + " entries."
    end if
    
    printlog "Close Context menu. Then check if a Edit-line for the Cells is visible."
    sleep 3
    MenuSelect (1)  'Default
    
    Kontext "RechenleisteCalc"
    if RechenleisteCalc.Exists then
        Printlog "This is a spreadsheet-Document. Good"
    endif
    
    printlog "Close Context menu. Then close the Document"
    Call hCloseDocument
    
endcase 'tOLE_Copy
'-----------------------------------------------------------------------------------------------------------'
testcase tOLE_SaveLoad

    printlog "Test to see if an OLE can be inserted, saved and loaded correctly."
    dim filename as string
    dim filetype as string
    dim filetype2 as string
    
    if (gApplication = "IMPRESS") then 
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if
    setClipboard(" ")
    filename = ConvertPath (gOfficePath + "user\work\oletest." & ExtensionString)
    printlog "Create a new document."
    Call hNewDocument
    Call sSelectEmptyLayout
    printlog "Insert->Object->OLE Object..."
    InsertObjectOLEObject
    Kontext "OLEObjektInsert"
    
    printlog "Choose create new. Select Writer."
    Objekttyp.Select 5
    
    printlog "Click OK to close dialog and insert OLE."
    OLEObjektInsert.OK
    
    printlog "Deselect object."
    sleep 1
    kontext "DocumentImpress"
    DocumentImpress.mousedown (10,10)
    DocumentImpress.mouseup (10,10)
    sleep 1
    printlog "Select object."
    hTypekeys "<MOD1 A>"
    hOpenContextMenu
    printlog "Count number of context menu entries. If 20 (including menuseparators), then its a OLE."
    if MenuGetItemCount <> 20 then
        Warnlog "This doesnt seem to be a Writer-OLE. It has " + MenuGetItemCount + " entries."
    else
        printlog "Right number of context menu entries for OLE."
    end if
    printlog "Closing Context Menu"
    MenuSelect (0)
    printlog "Save the File."
    FileSaveAs
    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 1 ) ) then
        Dateiname.Settext (filename)
        printlog "Saving as: " & filename
        Speichern.Click
    else
        warnlog( "<FileSaveAs> dialog did not open" )
    endif
    
    printlog "If the file already exists - press yes to overwrite it."
    kontext "Messagebox"
    if ( Messagebox.exists( 3 ) ) then
        Messagebox.Yes
    endif
    Sleep 3
   
    printlog "Getting type of OLE from save as dialog."
    hOpenContextMenu
    hMenuSelectNr(-1)
    Kontext "SpeichernDlg"
    filetype = Dateityp.GetSelText (1)
    printlog "Filetype is " & Dateityp.GetSelText (1)
    SpeichernDlg.Cancel

    printlog "Close Document."
    Call hCloseDocument
    Call hNewDocument

    Kontext "DocumentImpress"
    printlog "Open saved file."
    Call hFileOpen (filename)
    sleep 1
    hTypekeys "<MOD1 A>"
    hOpenContextMenu                               
    hMenuSelectNr(-1)
    Kontext "SpeichernDlg"
    filetype2 = Dateityp.GetSelText
    SpeichernDlg.Cancel
    printlog "Check if it has the same number of context menu entries."
    Printlog "The OLE created was a: " + filetype + " and the one which was loaded was a: " + filetype2
    if ((left(right(filetype,4),3))) <> ((left(right(filetype2,4),3))) then
        warnlog "The first was a: " + ((left(right(filetype,4),3))) + " and second was a: " + ((left(right(filetype2,4),3)))
    else
        printlog "Good: the first one was a: " + ((left(right(filetype,4),3))) + "-file, and second one was a: " + ((left(right(filetype2,4),3))) + "-file."
    endif
    printlog "Closing document."
    Call hCloseDocument

    'FHA TODO
    'testcase tOLE_DragDrop
    printlog " Create OLE, drag n drop within the document. "
    printlog " If Impress, drag and drop from document to another slide. "
    'endcase 'tOLE_DragDrop

endcase ' tOLE_Save
'-------------------------------------------------------------------------------
testcase tiOpenOLECrash

    dim sFileodp as string
    dim oFile as string
    dim i as integer
    printlog "Testing for bug nr: i70019"
    
    printlog "Open Application"
    Call hNewDocument
    sleep 1
    printlog "1. Open file: generic_de.ppt"
    FileOpen
    sleep 1
    Kontext "OeffnenDlg"
    printlog " Use file: graphics\optional\input\generic_de.ppt "
    oFile = ConvertPath (gTesttoolPath + "graphics\required\input\generic_de.ppt")
    Printlog "file:    '" + oFile + "'"
    Dateiname.SetText (oFile)
    printlog "2. Save this document as an *.odp -file."
    Oeffnen.Click
    kontext "DocumentImpress"
    i = 0
    do while (i < 20)
        try
            ApplicationBusy
        catch
            sleep (1)
            i = i + 1
        endcatch
    loop
    if DocumentImpress.StatusIsProgress AND i > 19 then
        warnlog "   Took over 20 seconds to load the document Document. Stalled?"
    endif
    
    FileSaveAs
    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then
        sFileodp = (ConvertPath(gOfficePath + "user\work\generic_de"))
        Dateiname.SetText (sFileodp)
        Dateityp.Select (1)
        Speichern.Click
        Kontext "Messagebox"
        if Messagebox.Exists(2) then
            try
                Messagebox.Yes
            catch
                warnlog( "#i110938 - Failure to save certain impress documents" )
                Messagebox.ok()
                goto endsub
            endcatch
            Kontext "AlienWarning"
        if AlienWarning.Exists(2) then
            Warnlog "Should not be any alienwarning when saving in our own format!"
            AlienWarning.OK
        end if
        printlog "Saved as: " + sFileodp
        end if
    else
        warnlog( "<FileSaveAs> dislog did not open" )
        goto endsub
    endif
    Sleep 3
    kontext "DocumentImpress"
    printlog "3. Close Impress."
    Call hCloseDocument
    
    printlog "4.Reopen the *.odp file again"
    FileOpen
    Kontext "OeffnenDlg"
    Dateiname.SetText (sFileodp + ".odp")
    Oeffnen.Click
    kontext
    if Active.exists (5) then
        Printlog "ACTIVE: "+active.gettext
        Active.ok
        Warnlog "failed to open file? :-("
    end if
    
    i = 0
    do while (i < 20)
        try
            ApplicationBusy
        catch
            sleep (1)
            i = i + 1
        endcatch
    loop
    if DocumentImpress.StatusIsProgress AND i > 19 then
    warnlog "Took over 20 seconds to load the document Document. Stalled?"
    endif
    
    printlog "5. Right-click on the second slide and click on New slide. "
    Kontext "Slides"
    SlidesControl.TypeKeys "<PAGEDOWN>" 'Goto second slide. (and get the focus right)
    SlidesControl.TypeKeys "<SHIFT F10>"  'OpenContextMenu (True)  'Open Context-menu
    hMenuSelectNr (1) 'New Slide

    printlog "6. Change the title-(the text was F�r den TCM-Test, but this should irrelevant... ;) )."
    hTypeKeys "<TAB>" 'To select the title.
    hTypeKeys "Fuer den TCM-Test" 'To enter text
    printlog " 7. Clicked two times on the Click twice to insert object "
    hTypeKeys "<ESCAPE>"
    gMouseClick 1,1
    hTypeKeys "<TAB><TAB>" '(to select the second object)
    hTypeKeys "<RETURN>"
    sleep (2)
    
    printlog "8. Choose OpenOffice.org 2.0 Formular"
    Kontext "OLEObjektInsert"
    Objekttyp.Select (4)
    sleep (2)
    PrintLog "    Object: " + Objekttyp.GetSelText + "  will be inserted."
    printlog "Leave dialog 'Insert OLE Object' with OK"
    OLEObjektInsert.OK
    sleep (2)
    
    kontext "CommandsMath"
    Commands.TypeKeys "A over B + FACT 6 = x"
    
    'printlog " Leave edit mode by typing key [Escape] "
    'Comment: If I do this, the bug wont occur...
    'hTypeKeys "<ESCAPE>"
    
    printlog "10. Click on the Save button."
    kontext "Standardbar"
    Speichern.Click
    
    printlog "11. Click on the X to close the window while the OLE-object still in Edit-mode."
    Call hCloseDocument
endcase 'tiOpenOLECrash
