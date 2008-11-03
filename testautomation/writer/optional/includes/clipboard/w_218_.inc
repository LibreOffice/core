'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_218_.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:25:03 $
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
'* short description : CROSS-APPLICATIONS CLIPBOARD TEST (Writer)
'*
'\***********************************************************************

sub w_218_

    Call HTMLToDRAWText
    Call HTMLToDRAWField
    Call HTMLToDRAWTable
    Call HTMLToDRAWGraphicLinked
    Call HTMLToDRAWControl
    Call HTMLToDRAWFloatingFrame

end sub

' ---------------------------------------------------------------------------------

testcase HTMLToDRAWText
  gApplication = "HTMLDOKUMENT"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
  printlog " + Jump to beginning of document "
  printlog " + Select first paragraph "
  printlog " + Copy selected text "
  Call hFileOpen (gtesttoolpath & "writer\optional\input\clipboard\html.html")
  Call sMakeReadOnlyDocumentEditable
  printlog "  Jump to beginning of document "
  Call wTypeKeys "<Mod1 Home>"
  Call wTypeKeys "<Shift Down><Shift End>"
  EditCopy

  gApplication = "DRAW"
  printlog " + Open new document "
  Call hNewDocument
  printlog " + Edit / Paste Special "
    Sleep 2
    try
        EditPasteSpecial
    catch
        Warnlog "Unable to execute 'Edit / Paste Special'! Test failed!"
        Call hCloseDocument
        Call hCloseDocument
        goto endsub
    endcatch
  Wait 500
  printlog "  Paste 'Text' in all available clipboard formats "
  Kontext "InhaltEinfuegen"
  if InhaltEinfuegen.Exists then
   Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"text")
   printlog " + Write Clipboard format under pasted content "
   Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_draw_text.odg", "draw8")
   printlog " + Save document as ..\user\work\html_to_draw_text.odg"
   Wait 500
   Call hCloseDocument
   printlog " + Close saved document "
   wait 500
   Call hFileOpen (gOfficepath + "user\work\html_to_draw_text.odg",false)
   printlog " + Reopen saved document "
   Wait 500
   Call hCloseDocument
   printlog " + Close saved document "
  else
   Warnlog "Dialog 'Paste Special' is not up!"
   Call hCloseDocument
   goto endsub
  end if
  printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase HTMLToDRAWField
    gApplication = "HTMLDOKUMENT"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog " + Jump to beginning of document "
    printlog " + Select paragraph with 'Date Field' "
    printlog " + Copy selected text "
    Call hFileOpen (gtesttoolpath & "writer\optional\input\clipboard\html.html")
    Call sMakeReadOnlyDocumentEditable
    printlog "  Jump to beginning of document "
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<Down>",2
    Call wTypeKeys "<Home><Shift End>"
    try
        EditCopy
    catch
        Call wTypeKeys "<Down>",1
        Call wTypeKeys "<Home><Shift End>"
        EditCopy
    endcatch
    gApplication = "DRAW"
    printlog " + Open new document "
    Call hNewDocument
    printlog " + Edit / Paste Special "
    Sleep 2
    try
        EditPasteSpecial
    catch
        Warnlog "Unable to execute 'Edit / Paste Special'! Test failed!"
        Call hCloseDocument
        Call hCloseDocument
        goto endsub
    endcatch
    Wait 500
    printlog "  Paste 'Field' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"field")
        printlog " + Write Clipboard format under pasted content "
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_draw_field.odg", "draw8")
        printlog " + Save document as ..\user\work\html_to_draw_field.odg"
        Wait 500
        Call hCloseDocument
        printlog " + Close saved document "
        sleep (2)
        Call hFileOpen (gOfficepath + "user\work\html_to_draw_field.odg",false)
        printlog " + Reopen saved document "
        Wait 500
        Call hCloseDocument
        printlog " + Close saved document "
    else
        Warnlog "Dialog 'Paste Special' is not up!"
        Call hCloseDocument
        goto endsub
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase HTMLToDRAWTable
  gApplication = "HTMLDOKUMENT"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
  printlog " + Jump to beginning of document "
  printlog " + Select 'Table' "
  printlog " + Copy selected table "
  Call hFileOpen (gtesttoolpath & "writer\optional\input\clipboard\html.html")
  Call sMakeReadOnlyDocumentEditable
  printlog "  Jump to beginning of document "
  Call wTypeKeys "<Mod1 Home>"
  Call wTypeKeys "<Down>",5
  Call wTypeKeys "<Mod1 A>",2
  EditCopy

  gApplication = "DRAW"
  printlog " + Open new document "
  Call hNewDocument
  printlog " + Edit / Paste Special "
    Sleep 2
    try
        EditPasteSpecial
    catch
        Warnlog "Unable to execute 'Edit / Paste Special'! Test failed!"
        Call hCloseDocument
        Call hCloseDocument
        goto endsub
    endcatch
  Wait 500
  printlog "  Paste 'Table' in all available clipboard formats "
  Kontext "InhaltEinfuegen"
   if InhaltEinfuegen.Exists then
     Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"table")
     printlog " + Write Clipboard format under pasted content "
     Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_draw_table.odg", "draw8")
     printlog " + Save document as ..\user\work\html_to_draw_table.odg"
     Wait 500
     Call hCloseDocument
     printlog " + Close saved document "
     wait 500
     Call hFileOpen (gOfficepath + "user\work\html_to_draw_table.odg",false)
     printlog " + Reopen saved document "
     Wait 500
     Call hCloseDocument
     printlog " + Close saved document "
   else
     Warnlog "Dialog 'Paste Special' is not up!"
     Call hCloseDocument
     goto endsub
   end if
  printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase HTMLToDRAWGraphicLinked
    gApplication = "HTMLDOKUMENT"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Linked Graphic' "
    printlog " + Copy selected Linked Graphic "
    Call hFileOpen (gtesttoolpath & "writer\optional\input\clipboard\html.html")
    Call sMakeReadOnlyDocumentEditable
    printlog "  Jump to beginning of document "
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys ( "<Shift F4>" )
    EditCopy
    
    gApplication = "DRAW"
    printlog " + Open new document "
    Call hNewDocument
    printlog " + Edit / Paste Special "
    Sleep 2
    try
        EditPasteSpecial
    catch
        Warnlog "Unable to execute 'Edit / Paste Special'! Test failed!"
        Call hCloseDocument
        Call hCloseDocument
        goto endsub
    endcatch
    Wait 500
    printlog "  Paste 'Linked Graphic' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"graphicL")
        printlog " + Write Clipboard format under pasted content "
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_draw_graphic1.odg", "draw8")
        printlog " + Save document as ..\user\work\html_to_draw_graphic1.odg"
        Wait 500
        Call hCloseDocument
        printlog " + Close saved document "
        wait 500
        Call hFileOpen (gOfficepath + "user\work\html_to_draw_graphic1.odg",false)
        printlog " + Reopen saved document "
        Wait 500
        Call hCloseDocument
        printlog " + Close saved document "
    else
        Warnlog "Dialog 'Paste Special' is not up!"
        Call hCloseDocument
        goto endsub
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase HTMLToDRAWControl
    gApplication = "HTMLDOKUMENT"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Control' "
    printlog " + Copy selected Control "
    Call hFileOpen (gtesttoolpath & "writer\optional\input\clipboard\html.html")
    Call sMakeReadOnlyDocumentEditable
    printlog "  Jump to beginning of document "
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys ( "<Shift F4>" )
    Call wTypeKeys "<Tab>"
    EditCopy
    
    gApplication = "DRAW"
    printlog " + Open new document "
    Call hNewDocument
    printlog " + Edit / Paste Special "
    Sleep 2
    try
        EditPasteSpecial
    catch
        Warnlog "Unable to execute 'Edit / Paste Special'! Test failed!"
        Call hCloseDocument
        Call hCloseDocument
        goto endsub
    endcatch
    Wait 500
    printlog "  Paste 'Control' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"control")
        printlog " + Write Clipboard format under pasted content "
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_draw_control.odg", "draw8")
        printlog " + Save document as ..\user\work\html_to_draw_control.odg"
        Wait 500
        Call hCloseDocument
        printlog " + Close saved document "
        wait 500
        Call hFileOpen (gOfficepath + "user\work\html_to_draw_control.odg",false)
        printlog " + Reopen saved document "
        Wait 500
        Call hCloseDocument
        printlog " + Close saved document "
    else
        Warnlog "Dialog 'Paste Special' is not up!"
        Call hCloseDocument
        goto endsub
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase HTMLToDRAWFloatingFrame
    gApplication = "HTMLDOKUMENT"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Floating Frame' "
    printlog " + Copy selected Floating Frame "
    Call hFileOpen (gtesttoolpath & "writer\optional\input\clipboard\html.html")
    Call sMakeReadOnlyDocumentEditable
    printlog "  Jump to beginning of document "
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys ( "<Shift F4>" )
    Call wTypeKeys "<Tab>",2
    try
        EditCopy
    catch
        Warnlog "Edit / Copy is disabled ! Maybe object not selected!"
        Call hCloseDocument
        goto endsub
    endcatch
    gApplication = "DRAW"
    printlog " + Open new document "
    Call hNewDocument
    printlog " + Edit / Paste Special "
    Sleep 2
    try
        EditPasteSpecial
    catch
        Warnlog "Unable to execute 'Edit / Paste Special'! Test failed!"
        Call hCloseDocument
        Call hCloseDocument
        goto endsub
    endcatch
    Wait 500
    printlog "  Paste 'Floating Frame' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"control")
        printlog " + Write Clipboard format under pasted content "
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_draw_float.odg", "draw8")
        printlog " + Save document as ..\user\work\html_to_draw_float.odg"
        Wait 500
        Call hCloseDocument
        printlog " + Close saved document "
        wait 500
        Call hFileOpen (gOfficepath + "user\work\html_to_draw_float.odg",false)
        printlog " + Reopen saved document "
        Wait 500
        Call hCloseDocument
        printlog " + Close saved document "
    else
        Warnlog "Dialog 'Paste Special' is not up!"
        Call hCloseDocument
        goto endsub
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase
