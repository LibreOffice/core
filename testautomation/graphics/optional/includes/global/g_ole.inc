'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_ole.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:40 $
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
'* short description: includefile for Ole-testing
'*
'**************************************************************************************
' #1 tOLE_Copy
' #1 tOLE_SaveLoad
' #1 tiOpenOLECrash
'\*************************************************************************************

testcase tOLE_Copy
    qaerrorlog "not running due to reconstruction"
    goto endsub

    dim writerfilename1 as string
    writerfilename1 = ConvertPath (gTesttoolPath + "graphics\required\input\oletest.odg")

    FileOpen
    Kontext "OeffnenDlg"
    sleep 1
    Dateiname.SetText (writerfilename1)
    Oeffnen.Click
    Sleep 3

    kontext "Messagebox"
    if Messagebox.exists then
        Messagebox.Yes
    endif

    sleep 3
    gApplication = "DRAW"
    sleep 1
    kontext "DocumentDraw"
    DocumentDraw.mousedown (10,10)
    DocumentDraw.mouseup (10,10)
    sleep 1
    DocumentDraw.Typekeys "<TAB>"
    EditCopy

    Call hCloseDocument
    gApplication = "IMPRESS"
    Call hNewDocument
    EditPaste

    sleep 1
    Kontext "DocumentImpress"
    hTypekeys "<MOD1> A"
    hTypekeys "<RETURN>"
    hOpenContextMenu

    if MenuGetItemCount <> 5 then
        Warnlog "This doesn't seem to be recognized as a Metafile. It has " + MenuGetItemCount + " entries."
    end if

    sleep 3
    MenuSelect (1)  'Default

    Kontext "RechenleisteCalc"
    if RechenleisteCalc.Exists then
        Printlog "This is a spreadsheet-Document. Good"
    endif

    Kontext "DocumentImpress"
    DocumentImpress.mousedown (10,10)
    DocumentImpress.mouseup (10,10)
    sleep 3
    Call hCloseDocument

    FileOpen
    Kontext "OeffnenDlg"
    sleep 1
    Dateiname.SetText (writerfilename1)
    Oeffnen.Click
    Sleep 3

    kontext "Messagebox"
    if Messagebox.exists then
        Messagebox.Yes
    endif
    sleep 3

    gApplication = "DRAW"
    sleep 1
    kontext "DocumentDraw"
    DocumentDraw.mousedown (10,10)
    DocumentDraw.mouseup (10,10)
    sleep 1
    DocumentDraw.Typekeys "<TAB><TAB>"
    EditCopy

    sleep 1
    DocumentDraw.mousedown (10,10)
    DocumentDraw.mouseup (10,10)
    sleep 1
    Call hCloseDocument

    gApplication = "IMPRESS"
    Call hNewDocument
    EditPaste

    sleep 1
    Kontext "DocumentImpress"
    hTypekeys "<MOD1> A"
    hTypekeys "<RETURN>"
    hOpenContextMenu

    if MenuGetItemCount <> 5 then
        Warnlog "This doesn't seem to be recognized as a Metafile. It has " + MenuGetItemCount + " entries."
    end if

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

    Call hCloseDocument

    FileOpen
    Kontext "OeffnenDlg"
    sleep 1
    Dateiname.SetText (writerfilename1)
    Oeffnen.Click
    Sleep 3
    kontext "Messagebox"
    if Messagebox.exists then
        Messagebox.Yes
    endif
    sleep 3

    gApplication = "DRAW"
    sleep 1
    kontext "DocumentDraw"
    DocumentDraw.mousedown (10,10)
    DocumentDraw.mouseup (10,10)
    sleep 1
    DocumentDraw.Typekeys "<TAB><TAB><TAB>"
    EditCopy

    sleep 1
    DocumentDraw.mousedown (10,10)
    DocumentDraw.mouseup (10,10)
    sleep 1
    Call hCloseDocument

    gApplication = "IMPRESS"
    Call hNewDocument
    EditPaste

    Kontext "DocumentImpress"
    hTypekeys "<MOD1> A"
    hTypekeys "<RETURN>"
    hOpenContextMenu

    if MenuGetItemCount <> 5 then
        Warnlog "This doesn't seem to be recognized as a Metafile. It has " + MenuGetItemCount + " entries."
    end if

    sleep 3
    MenuSelect (1)  'Default

    Kontext "RechenleisteCalc"
    if RechenleisteCalc.Exists then
        Printlog "This is a spreadsheet-Document. Good"
    endif

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
    sleep 1
    Kontext "SpeichernDlg"
    Dateiname.Settext (filename)
    printlog "Saving as: " & filename
    Speichern.Click

    printlog "If the file already exists - press yes to overwrite it."
    Sleep 3
    kontext "Messagebox"
    if Messagebox.exists then
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
    'endcase 'tOLE_DragDrop

endcase ' tOLE_Save

'-------------------------------------------------------------------------------
testcase tiOpenOLECrash

    dim sFileodp as string
    dim oFile as string
    dim i as integer

    Call hNewDocument
    sleep 1
    FileOpen
    sleep 1
    Kontext "OeffnenDlg"
    oFile = ConvertPath (gTesttoolPath + "graphics\required\input\generic_de.ppt")
    Printlog "file:    '" + oFile + "'"
    Dateiname.SetText (oFile)
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
    sFileodp = (ConvertPath(gOfficePath + "user\work\generic_de"))
    Dateiname.SetText (sFileodp)
    Dateityp.Select (1)
    Speichern.Click
    Kontext "Messagebox"
    if Messagebox.Exists(2) then
        Messagebox.Yes
        Kontext "AlienWarning"
        if AlienWarning.Exists(2) then
            Warnlog "Should not be any alienwarning when saving in our own format!"
            AlienWarning.OK
        end if
        printlog "Saved as: " + sFileodp
    end if
    Sleep 3
    kontext "DocumentImpress"
    Call hCloseDocument

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
        warnlog "   Took over 20 seconds to load the document Document. Stalled?"
    endif

    Kontext "Slides"
    SlidesControl.TypeKeys "<PAGEDOWN>" 'Goto second slide. (and get the focus right)
    SlidesControl.TypeKeys "<SHIFT F10>"  'OpenContextMenu (True)  'Open Context-menu
    hMenuSelectNr (1) 'New Slide

    hTypeKeys "<TAB>" 'To select the title.
    hTypeKeys "F�r den TCM-Test" 'To enter text
    hTypeKeys "<ESCAPE>"
    gMouseClick 1,1
    hTypeKeys "<TAB><TAB>" '(to select the second object)
    hTypeKeys "<RETURN>"
    sleep (2)

    Kontext "OLEObjektInsert"
    Objekttyp.Select (4)
    sleep (2)
    PrintLog "    Objeckt: " + Objekttyp.GetSelText + "  will be inserted."
    OLEObjektInsert.OK
    sleep (2)

    kontext "CommandsMath"
    Commands.TypeKeys "A over B + FACT 6 = x"

    'Comment: If I do this, the bug wont occur...
    'hTypeKeys "<ESCAPE>"

    kontext "Standardbar"
    Speichern.Click

    Call hCloseDocument
endcase

'-----------------oooo-----------------oooo-----------------oooo-----------------

