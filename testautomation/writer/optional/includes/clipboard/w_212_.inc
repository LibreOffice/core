'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
'*
'* short description : CROSS-APPLICATIONS CLIPBOARD TEST (Writer)
'*
'\***********************************************************************

sub w_212_

    Call wDisableImpressAutopilot
    Call WriterToImpressText
    Call WriterToImpressField
    Call WriterToImpressTable
    Call WriterToImpressFrame
    Call WriterToImpressDrawingObject
    Call WriterToImpressGraphicLinked
    Call WriterToImpressGraphicEmbedded
    Call WriterToImpressOLEObject
    Call WriterToImpressControl

end sub

' ---------------------------------------------------------------------------------

testcase WriterToImpressText
    gApplication = "WRITER"
    printlog " Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " Jump to beginning of document "
    printlog " Select first paragraph "
    printlog " Copy selected text "
    if wSetClipboardtestDefaults("text") = True then
        gApplication = "IMPRESS"
        printlog " Open new document "
        Call hNewDocument
        printlog " Edit / Paste Special "
        Sleep 2
        try
            EditPasteSpecial
        catch
            Warnlog "Unable to execute 'Edit / Paste / Special' -> Disabled !"
            Call hCloseDocument
            wait 500
            Call hCloseDocument
            goto endsub
        endcatch
        Wait 500
        printlog " Paste 'Text' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"text")
            printlog " Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_impress_text.odp", "impress8")
            printlog " Save document as ..\user\work\writer_to_impress_text.odp"
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
            wait 500
            hFileOpen( gOfficepath + "user\work\writer_to_impress_text.odp",false)
            printlog " Reopen saved document "
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Impress -> 'Text' failed !"
    end if
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToImpressField
    gApplication = "WRITER"
    printlog " Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " Jump to beginning of document "
    printlog " Select paragraph with 'Date Field' "
    printlog " Copy selected text "
    if wSetClipboardtestDefaults("field") = True then
        gApplication = "IMPRESS"
        printlog " Open new document "
        Call hNewDocument
        printlog " Edit / Paste Special "
        Sleep 2
        try
            EditPasteSpecial
        catch
            Warnlog "Unable to execute 'Edit / Paste / Special' -> Disabled !"
            Call hCloseDocument
            wait 500
            Call hCloseDocument
            goto endsub
        endcatch
        Wait 500
        printlog " Paste 'Field' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"field")
            printlog " Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_impress_field.odp", "impress8")
            printlog " Save document as ..\user\work\writer_to_impress_field.odp"
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
            wait 500
            hFileOpen( gOfficepath + "user\work\writer_to_impress_field.odp",false)
            printlog " Reopen saved document "
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Impress -> 'Field' failed !"
    end if
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToImpressTable
    gApplication = "WRITER"
    printlog " Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " Jump to beginning of document "
    printlog " Select 'Table' "
    printlog " Copy selected table "
    if wSetClipboardtestDefaults("table") = True then
        gApplication = "IMPRESS"
        printlog " Open new document "
        Call hNewDocument
        printlog " Edit / Paste Special "
        Sleep 2
        try
            EditPasteSpecial
        catch
            Warnlog "Unable to execute 'Edit / Paste / Special' -> Disabled !"
            Call hCloseDocument
            wait 500
            Call hCloseDocument
            goto endsub
        endcatch
        Wait 500
        printlog " Paste 'Table' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"table")
            printlog " Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_impress_table.odp", "impress8")
            printlog " Save document as ..\user\work\writer_to_impress_table.odp"
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
            wait 500
            hFileOpen( gOfficepath + "user\work\writer_to_impress_table.odp",false)
            printlog " Reopen saved document "
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Impress -> 'Table' failed !"
    end if
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToImpressFrame
    gApplication = "WRITER"
    printlog " Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " Jump to beginning of document "
    printlog " Select 'Frame' "
    printlog " Copy selected frame "
    if wSetClipboardtestDefaults("frame") = True then
        gApplication = "IMPRESS"
        printlog " Open new document "
        Call hNewDocument
        printlog " Edit / Paste Special "
        Sleep 2
        try
            EditPasteSpecial
        catch
            Warnlog "Unable to execute 'Edit / Paste / Special' -> Disabled !"
            Call hCloseDocument
            wait 500
            Call hCloseDocument
            goto endsub
        endcatch
        Wait 500
        printlog " Paste 'Table' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"frame")
            printlog " Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_impress_frame.odp", "impress8")
            printlog " Save document as ..\user\work\writer_to_impress_frame.odp"
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
            wait 500
            hFileOpen( gOfficepath + "user\work\writer_to_impress_frame.odp",false)
            printlog " Reopen saved document "
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Impress -> 'Frame' failed !"
    end if
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToImpressDrawingObject
    gApplication = "WRITER"
    printlog " Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " Jump to beginning of document "
    printlog " Select 'Drawing Object' "
    printlog " Copy selected Drawing Object "
    if wSetClipboardtestDefaults("DRAW") = True then
        gApplication = "IMPRESS"
        printlog " Open new document "
        Call hNewDocument
        printlog " Edit / Paste Special "
        Sleep 2
        try
            EditPasteSpecial
        catch
            Warnlog "Unable to execute 'Edit / Paste / Special' -> Disabled !"
            Call hCloseDocument
            wait 500
            Call hCloseDocument
            goto endsub
        endcatch
        Wait 500
        printlog " Paste 'Table' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"DRAW")
            printlog " Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_impress_draw.odp", "impress8")
            printlog " Save document as ..\user\work\writer_to_impress_draw.odp"
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
            wait 500
            hFileOpen( gOfficepath + "user\work\writer_to_impress_draw.odp",false)
            printlog " Reopen saved document "
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Impress -> 'Drawing Object' failed !"
    end if
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToImpressGraphicLinked
    gApplication = "WRITER"
    printlog " Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " Jump to beginning of document "
    printlog " Select 'Linked Graphic' "
    printlog " Copy selected Linked Graphic "
    if wSetClipboardtestDefaults("graphicL") = True then
        gApplication = "IMPRESS"
        printlog " Open new document "
        Call hNewDocument
        printlog " Edit / Paste Special "
        Sleep 2
        try
            EditPasteSpecial
        catch
            Warnlog "Unable to execute 'Edit / Paste / Special' -> Disabled !"
            Call hCloseDocument
            wait 500
            Call hCloseDocument
            goto endsub
        endcatch
        Wait 500
        printlog " Paste 'Linked Graphic' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"graphicL")
            printlog " Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_impress_graphic1.odp", "impress8")
            printlog " Save document as ..\user\work\writer_to_impress_graphic1.odp"
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
            wait 500
            hFileOpen( gOfficepath + "user\work\writer_to_impress_graphic1.odp",false)
            printlog " Reopen saved document "
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Impress -> 'Linked Graphic' failed !"
    end if
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToImpressGraphicEmbedded
    gApplication = "WRITER"
    printlog " Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " Jump to beginning of document "
    printlog " Select 'Embedded Graphic' "
    printlog " Copy selected Embedded Graphic "
    if wSetClipboardtestDefaults("graphicE") = True then
        gApplication = "IMPRESS"
        printlog " Open new document "
        Call hNewDocument
        printlog " Edit / Paste Special "
        Sleep 2
        try
            EditPasteSpecial
        catch
            Warnlog "Unable to execute 'Edit / Paste / Special' -> Disabled !"
            Call hCloseDocument
            wait 500
            Call hCloseDocument
            goto endsub
        endcatch
        Wait 500
        printlog " Paste 'Embedded Graphic' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"graphicE")
            printlog " Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_impress_graphic2.odp", "impress8")
            printlog " Save document as ..\user\work\writer_to_impress_graphic2.odp"
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
            wait 500
            hFileOpen( gOfficepath + "user\work\writer_to_impress_graphic2.odp",false)
            printlog " Reopen saved document "
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Impress -> 'Embedded Graphic' failed !"
    end if
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToImpressOLEObject
    gApplication = "WRITER"
    printlog " Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " Jump to beginning of document "
    printlog " Select 'OLE Object' "
    printlog " Copy selected OLE Object "
    if wSetClipboardtestDefaults("ole") = True then
        gApplication = "IMPRESS"
        printlog " Open new document "
        Call hNewDocument
        printlog " Edit / Paste Special "
        Sleep 2
        try
            EditPasteSpecial
        catch
            Warnlog "Unable to execute 'Edit / Paste / Special' -> Disabled !"
            Call hCloseDocument
            wait 500
            Call hCloseDocument
            goto endsub
        endcatch
        Wait 500
        printlog " Paste 'OLE object' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"ole")
            printlog " Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_impress_ole.odp", "impress8")
            printlog " Save document as ..\user\work\writer_to_impress_ole.odp"
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
            wait 500
            hFileOpen( gOfficepath + "user\work\writer_to_impress_ole.odp",false)
            printlog " Reopen saved document "
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Impress -> 'OLE object' failed !"
    end if
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToImpressControl
    gApplication = "WRITER"
    printlog " Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " Jump to beginning of document "
    printlog " Select 'Control' "
    printlog " Copy selected Control "
    if wSetClipboardtestDefaults("control") = True then
        gApplication = "IMPRESS"
        printlog " Open new document "
        Call hNewDocument
        printlog " Edit / Paste Special "
        Sleep 2
        try
            EditPasteSpecial
        catch
            Warnlog "Unable to execute 'Edit / Paste / Special' -> Disabled !"
            Call hCloseDocument
            wait 500
            Call hCloseDocument
            goto endsub
        endcatch
        Wait 500
        printlog " Paste 'Control' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"control")
            printlog " Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_impress_control.odp", "impress8")
            printlog " Save document as ..\user\work\writer_to_impress_control.odp"
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
            wait 500
            hFileOpen( gOfficepath + "user\work\writer_to_impress_control.odp",false)
            printlog " Reopen saved document "
            Wait 500
            Call hCloseDocument
            printlog " Close saved document "
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Impress -> 'Control' failed !"
    end if
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase
