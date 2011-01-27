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

sub w_213_

    Call WriterToDrawText
    Call WriterToDrawField
    Call WriterToDrawTable
    Call WriterToDrawFrame
    Call WriterToDrawDrawingObject
    Call WriterToDrawGraphicLinked
    Call WriterToDrawGraphicEmbedded
    Call WriterToDrawOLEObject
    Call WriterToDrawControl

end sub

' ---------------------------------------------------------------------------------

testcase WriterToDrawText
  gApplication = "WRITER"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
  printlog " + Jump to beginning of document "
  printlog " + Select first paragraph "
  printlog " + Copy selected text "
  if wSetClipboardtestDefaults("text") = True then
    gApplication = "DRAW"
    printlog " + Open new document "
    Call hNewDocument
    printlog " + Edit / Paste Special "
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
    printlog "  Paste 'Text' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"text")
        printlog " + Write Clipboard format under pasted content "
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_draw_text.odg", "draw8")
        printlog " + Save document as ..\user\work\writer_to_draw_text.odg"
        Wait 500
        Call hCloseDocument
        printlog " + Close saved document "
        wait 500
        hFileOpen( gOfficepath + "user\work\writer_to_draw_text.odg",false)
        printlog " + Reopen saved document "
        Wait 500
        Call hCloseDocument
        printlog " + Close saved document "
    else
        Warnlog "Dialog 'Paste Special' is not up!"
        Call hCloseDocument
        goto endsub
    end if
  else
      Warnlog "Writer To Draw -> 'Text' failed !"
  end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToDrawField
  gApplication = "WRITER"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
  printlog " + Jump to beginning of document "
  printlog " + Select paragraph with 'Date Field' "
  printlog " + Copy selected text "
  if wSetClipboardtestDefaults("field") = True then
    gApplication = "DRAW"
    printlog " + Open new document "
    Call hNewDocument
    printlog " + Edit / Paste Special "
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
    printlog "  Paste 'Field' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"field")
            printlog " + Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_draw_field.odg", "draw8")
            printlog " + Save document as ..\user\work\writer_to_draw_field.odg"
            Wait 500
            Call hCloseDocument
            printlog " + Close saved document "
            wait 500
            hFileOpen( gOfficepath + "user\work\writer_to_draw_field.odg",false)
            printlog " + Reopen saved document "
            Wait 500
            Call hCloseDocument
            printlog " + Close saved document "
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Draw -> 'Field' failed !"
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToDrawTable
    gApplication = "WRITER"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Table' "
    printlog " + Copy selected table "
    if wSetClipboardtestDefaults("table") = True then
        gApplication = "DRAW"
        printlog " + Open new document "
        Call hNewDocument
        printlog " + Edit / Paste Special "
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
        printlog "  Paste 'Table' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"table")
            printlog " + Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_draw_table.odg", "draw8")
            printlog " + Save document as ..\user\work\writer_to_draw_table.odg"
            Wait 500
            Call hCloseDocument
            printlog " + Close saved document "
            wait 500
            hFileOpen( gOfficepath + "user\work\writer_to_draw_table.odg",false)
            printlog " + Reopen saved document "
            Wait 500
            Call hCloseDocument
            printlog " + Close saved document "
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Draw -> 'Table' failed !"
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToDrawFrame
    gApplication = "WRITER"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Frame' "
    printlog " + Copy selected frame "
    if wSetClipboardtestDefaults("frame") = True then
        gApplication = "DRAW"
        printlog " + Open new document "
        Call hNewDocument
        printlog " + Edit / Paste Special "
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
        printlog "  Paste 'Table' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"frame")
            printlog " + Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_draw_frame.odg", "draw8")
            printlog " + Save document as ..\user\work\writer_to_draw_frame.odg"
            Wait 500
            Call hCloseDocument
            printlog " + Close saved document "
            wait 500
            hFileOpen( gOfficepath + "user\work\writer_to_draw_frame.odg",false)
            printlog " + Reopen saved document "
            Wait 500
            Call hCloseDocument
            printlog " + Close saved document "
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Draw -> 'Frame' failed !"
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToDrawDrawingObject
    gApplication = "WRITER"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Drawing Object' "
    printlog " + Copy selected Drawing Object "
    if wSetClipboardtestDefaults("DRAW") = True then
        gApplication = "DRAW"
        printlog " + Open new document "
        Call hNewDocument
        printlog " + Edit / Paste Special "
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
        printlog "  Paste 'Table' in all available clipboard formats "
        Kontext "InhaltEinfuegen"
        if InhaltEinfuegen.Exists then
            Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"DRAW")
            printlog " + Write Clipboard format under pasted content "
            Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_draw_draw.odg", "draw8")
            printlog " + Save document as ..\user\work\writer_to_draw_draw.odg"
            Wait 500
            Call hCloseDocument
            printlog " + Close saved document "
            wait 500
            hFileOpen( gOfficepath + "user\work\writer_to_draw_draw.odg",false)
            printlog " + Reopen saved document "
            Wait 500
            Call hCloseDocument
            printlog " + Close saved document "
        else
            Warnlog "Dialog 'Paste Special' is not up!"
            Call hCloseDocument
            goto endsub
        end if
    else
        Warnlog "Writer To Draw -> 'Drawing Object' failed !"
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToDrawGraphicLinked
   EnableQaErrors = true
   gApplication = "WRITER"
   printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
   printlog " + Jump to beginning of document "
   printlog " + Select 'Linked Graphic' "
   printlog " + Copy selected Linked Graphic "
   if wSetClipboardtestDefaults("graphicL") = True then
      gApplication = "DRAW"
      printlog " + Open new document "
      Call hNewDocument
      printlog " + Edit / Paste Special "
      Sleep 2
      try
         EditPasteSpecial
      catch
         warnlog "Check why linked graphic fails !"
         Call hCloseDocument
         wait 500
         Call hCloseDocument
         goto endsub
      endcatch
      Wait 500
      printlog "  Paste 'Linked Graphic' in all available clipboard formats "
      Kontext "InhaltEinfuegen"
      if InhaltEinfuegen.Exists then
         Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"graphicL")
         printlog " + Write Clipboard format under pasted content "
         Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_draw_graphic1.odg", "draw8")
         printlog " + Save document as ..\user\work\writer_to_draw_graphic1.odg"
         Wait 500
         Call hCloseDocument
         printlog " + Close saved document "
         wait 500
         hFileOpen( gOfficepath + "user\work\writer_to_draw_graphic1.odg",false)
         printlog " + Reopen saved document "
         Wait 500
         Call hCloseDocument
         printlog " + Close saved document "
      else
         Warnlog "Dialog 'Paste Special' is not up!"
         Call hCloseDocument
         goto endsub
      end if
   else
      Warnlog "Writer To Draw -> 'Linked Graphic' failed !"
   end if
   printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
    EnableQaErrors = false
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToDrawGraphicEmbedded
  gApplication = "WRITER"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
  printlog " + Jump to beginning of document "
  printlog " + Select 'Embedded Graphic' "
  printlog " + Copy selected Embedded Graphic "
  if wSetClipboardtestDefaults("graphicE") = True then
    gApplication = "DRAW"
    printlog " + Open new document "
    Call hNewDocument
    printlog " + Edit / Paste Special "
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
    printlog "  Paste 'Embedded Graphic' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
     Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"graphicE")
     printlog " + Write Clipboard format under pasted content "
     Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_draw_graphic2.odg", "draw8")
     printlog " + Save document as ..\user\work\writer_to_draw_graphic2.odg"
     Wait 500
     Call hCloseDocument
     printlog " + Close saved document "
     wait 500
     hFileOpen( gOfficepath + "user\work\writer_to_draw_graphic2.odg",false)
     printlog " + Reopen saved document "
     Wait 500
     Call hCloseDocument
     printlog " + Close saved document "
    else
     Warnlog "Dialog 'Paste Special' is not up!"
     Call hCloseDocument
     goto endsub
    end if
  else
   Warnlog "Writer To Draw -> 'Embedded Graphic' failed !"
  end if
  printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToDrawOLEObject
  gApplication = "WRITER"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
  printlog " + Jump to beginning of document "
  printlog " + Select 'OLE Object' "
  printlog " + Copy selected OLE Object "
  if wSetClipboardtestDefaults("ole") = True then
    gApplication = "DRAW"
    printlog " + Open new document "
    Call hNewDocument
    printlog " + Edit / Paste Special "
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
    printlog "  Paste 'OLE object' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
     Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"ole")
     printlog " + Write Clipboard format under pasted content "
     Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_draw_ole.odg", "draw8")
     printlog " + Save document as ..\user\work\writer_to_draw_ole.odg"
     Wait 500
     Call hCloseDocument
     printlog " + Close saved document "
     wait 500
     hFileOpen( gOfficepath + "user\work\writer_to_draw_ole.odg",false)
     printlog " + Reopen saved document "
     Wait 500
     Call hCloseDocument
     printlog " + Close saved document "
    else
     Warnlog "Dialog 'Paste Special' is not up!"
     Call hCloseDocument
     goto endsub
    end if
  else
   Warnlog "Writer To Draw -> 'OLE object' failed !"
  end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase WriterToDrawControl
  gApplication = "WRITER"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\writer.sxw' "
  printlog " + Jump to beginning of document "
  printlog " + Select 'Control' "
  printlog " + Copy selected Control "
  if wSetClipboardtestDefaults("control") = True then
    gApplication = "DRAW"
    printlog " + Open new document "
    Call hNewDocument
    printlog " + Edit / Paste Special "
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
    printlog "  Paste 'Control' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
     Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"control")
     printlog " + Write Clipboard format under pasted content "
     Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_draw_control.odg", "draw8")
     printlog " + Save document as ..\user\work\writer_to_draw_control.odg"
     Wait 500
     Call hCloseDocument
     printlog " + Close saved document "
     wait 500
     hFileOpen( gOfficepath + "user\work\writer_to_draw_control.odg",false)
     printlog " + Reopen saved document "
     Wait 500
     Call hCloseDocument
     printlog " + Close saved document "
    else
     Warnlog "Dialog 'Paste Special' is not up!"
     Call hCloseDocument
     goto endsub
    end if
  else
    Warnlog "Writer To Draw -> 'Control' failed !"
  end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase
