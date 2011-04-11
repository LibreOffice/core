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
'* short description : CROSS-APPLICATIONS CLIPBOARD TEST (Writer)
'*
'\***********************************************************************

sub w_210_

    Call WriterToWriterText
    Call WriterToWriterField
    Call WriterToWriterTable
    Call WriterToWriterFrame
    Call WriterToWriterDrawingObject
    Call WriterToWriterGraphicLinked
    Call WriterToWriterGraphicEmbedded
    Call WriterToWriterOLEObject
    Call WriterToWriterControl

end sub

' ---------------------------------------------------------------------------------

testcase WriterToWriterText
    gApplication = "WRITER"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " + Jump to beginning of document "
    printlog " + Select first paragraph "
    printlog " + Copy selected text "
    if wSetClipboardtestDefaults("text") = True then
        printlog " + Open new document "
        Call hNewDocument
        printlog " + Edit / Paste Special "
        EditPasteSpecialWriter
        Wait 500
        printlog "  Paste 'Text' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            printlog " + Write Clipboard format under pasted content "
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"text")
            printlog " + Save document as ..\user\work\writer_to_writer_text.odt"
            Call hFileSaveAsWithFilterKill( gOfficepath + "user\work\writer_to_writer_text.odt" , "writer8" )
            Wait 500
            printlog " + Close saved document "
            Call hCloseDocument
            printlog " + Reopen saved document "
            if hFileOpen (gOfficepath + "user\work\writer_to_writer_text.odt",true) = false then
                Kontext "Active"
                if Active.Exists then
                    try
                        Active.Yes
                    catch
                        Warnlog "Unable to remove checkbox: " + Active.Gettext
                        Active.ok
                    endcatch
                end if
            end if
            printlog " + Close saved document "
            Call hCloseDocument
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Writer -> 'Text' failed !"
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToWriterField
    gApplication = "WRITER"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " + Jump to beginning of document "
    printlog " + Select paragraph with 'Date Field' "
    printlog " + Copy selected text "
    printlog " + Open new document "
    if wSetClipboardtestDefaults("field") = True then
        Call hNewDocument
        printlog " + Edit / Paste Special "
        EditPasteSpecialWriter
        Wait 500
        printlog "  Paste 'Field' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            printlog " + Write Clipboard format under pasted content "
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"field")
            printlog " + Save document as ..\user\work\writer_to_writer_field.odt"
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_field.odt", "writer8")
            sleep (2) 'Wait 500
            printlog " + Close saved document "
            Call hCloseDocument
            printlog " + Reopen saved document "
            if hFileOpen (gOfficepath + "user\work\writer_to_writer_field.odt",true) = false then
                Kontext "Active"
                if Active.Exists then
                    try
                        Active.Yes
                    catch
                        Warnlog "Unable to remove checkbox: " + Active.Gettext
                        Active.ok
                    endcatch
                end if
            end if
            printlog " + Close saved document "
            Call hCloseDocument
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Writer -> 'Field' failed !"
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToWriterTable
    gApplication = "WRITER"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Table' "
    printlog " + Copy selected table "
    printlog " + Open new document "
    if wSetClipboardtestDefaults("table") = True then
        Call hNewDocument
        printlog " + Edit / Paste Special "
        EditPasteSpecialWriter
        sleep (1) 'Wait 500
        printlog "  Paste 'Table' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            printlog " + Write Clipboard format under pasted content "
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"table")
            printlog " + Save document as ..\user\work\writer_to_writer_table.odt"
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_table.odt", "writer8")
            Wait 500
            printlog " + Close saved document "
            Call hCloseDocument
            printlog " + Reopen saved document "
            if hFileOpen (gOfficepath + "user\work\writer_to_writer_table.odt",true) = false then
                Kontext "Active"
                if Active.Exists then
                    try
                        Active.Yes
                    catch
                        Warnlog "Unable to remove checkbox: " + Active.Gettext
                        Active.ok
                    endcatch
                end if
            end if
            printlog " + Close saved document "
            Call hCloseDocument
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Writer -> 'Table' failed !"
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToWriterFrame
    gApplication = "WRITER"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Frame' "
    printlog " + Copy selected frame "
    printlog " + Open new document "
    if wSetClipboardtestDefaults("frame") = True then
        Call hNewDocument
        printlog " + Edit / Paste Special "
        EditPasteSpecialWriter
        sleep (1) 'Wait 500
        printlog "  Paste 'Table' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            printlog " + Write Clipboard format under pasted content "
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"frame")
            printlog " + Save document as ..\user\work\writer_to_writer_frame.odt"
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_frame.odt", "writer8")
            WaitSlot (2000)
            printlog " + Close saved document "
            Call hCloseDocument
            printlog " + Reopen saved document "
            hFileOpen( gOfficepath + "user\work\writer_to_writer_frame.odt",false)
            printlog " + Close saved document "
            Call hCloseDocument
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Writer -> 'Frame' failed !"
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToWriterDrawingObject
    gApplication = "WRITER"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Drawing Object' "
    printlog " + Copy selected Drawing Object "
    printlog " + Open new document "
    if wSetClipboardtestDefaults("DRAW") = True then
        Call hNewDocument
        printlog " + Edit / Paste Special "
        EditPasteSpecialWriter
        Wait 500
        printlog "  Paste 'Table' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            printlog " + Write Clipboard format under pasted content "
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"DRAW")
            printlog " + Save document as ..\user\work\writer_to_writer_draw.odt"
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_draw.odt", "writer8")
            WaitSlot (2000)
            printlog " + Close saved document "
            Call hCloseDocument
            printlog " + Reopen saved document "
            hFileOpen( gOfficepath + "user\work\writer_to_writer_draw.odt",false)
            printlog " + Close saved document "
            Call hCloseDocument
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
       Warnlog "Writer To Writer -> 'Drawing Object' failed !"
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToWriterGraphicLinked
    gApplication = "WRITER"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Linked Graphic' "
    printlog " + Copy selected Linked Graphic "
    printlog " + Open new document "
    if wSetClipboardtestDefaults("graphicL") = True then
        Call hNewDocument
        printlog " + Edit / Paste Special "
        EditPasteSpecialWriter
        WaitSlot (2000)
        printlog "  Paste 'Linked Graphic' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            printlog " + Write Clipboard format under pasted content "
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"graphicL")
            printlog " + Save document as ..\user\work\writer_to_writer_graphic1.odt"
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_graphic1.odt", "writer8")
            Wait 500
            printlog " + Close saved document "
            Call hCloseDocument
            printlog " + Reopen saved document "
            hFileOpen( gOfficepath + "user\work\writer_to_writer_graphic1.odt",false)
            printlog " + Close saved document "
            Call hCloseDocument
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Writer -> 'Linked Graphic' failed !"
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToWriterGraphicEmbedded
    gApplication = "WRITER"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Embedded Graphic' "
    printlog " + Copy selected Embedded Graphic "
    printlog " + Open new document "
    if wSetClipboardtestDefaults("graphicE") = True then
        Call hNewDocument
        printlog " + Edit / Paste Special "
        EditPasteSpecialWriter
        Wait 500
        printlog "  Paste 'Embedded Graphic' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"graphicE")
            printlog " + Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_graphic2.odt", "writer8")
            printlog " + Save document as ..\user\work\writer_to_writer_graphic2.odt"
            WaitSlot (2000)
            printlog " + Close saved document "
            Call hCloseDocument
            printlog " + Reopen saved document "
            hFileOpen( gOfficepath + "user\work\writer_to_writer_graphic2.odt",false)
            printlog " + Close saved document "
            Call hCloseDocument
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Writer -> 'Embedded Graphic' failed !"
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToWriterOLEObject
    gApplication = "WRITER"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'OLE Object' "
    printlog " + Copy selected OLE Object "
    printlog " + Open new document "
    if wSetClipboardtestDefaults("ole") = True then
        Call hNewDocument
        printlog " + Edit / Paste Special "
        EditPasteSpecialWriter
        Wait 500
        printlog "  Paste 'OLE object' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            printlog " + Write Clipboard format under pasted content "
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"ole")
            printlog " + Save document as ..\user\work\writer_to_writer_ole.odt"
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_ole.odt", "writer8")
            WaitSlot (2000)
            printlog " + Close saved document "
            Call hCloseDocument
            printlog " + Reopen saved document "
            hFileOpen( gOfficepath + "user\work\writer_to_writer_ole.odt",false)
            printlog " + Close saved document "
            Call hCloseDocument
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Writer -> 'OLE object' failed !"
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToWriterControl
    gApplication = "WRITER"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Control' "
    printlog " + Copy selected Control "
    printlog " + Open new document "
    if wSetClipboardtestDefaults("control") = True then
        Call hNewDocument
        printlog " + Edit / Paste Special "
        EditPasteSpecialWriter
        Wait 500
        printlog "  Paste 'Control' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            printlog " + Write Clipboard format under pasted content "
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"control")
            printlog " + Save document as ..\user\work\writer_to_writer_control.odt"
            Call hFileSaveAsWithFilterKill(gOfficepath & "user\work\writer_to_writer_control.odt", "writer8")
            WaitSlot (2000)
            printlog " + Close saved document "
            Call hCloseDocument
            printlog " + Reopen saved document "
            hFileOpen( gOfficepath + "user\work\writer_to_writer_control.odt",false)
            printlog " + Close saved document "
            Call hCloseDocument
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Writer -> 'Control' failed !"
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------
