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
    gApplication = "HTML"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog " + Jump to beginning of document "
    printlog " + Select first paragraph "
    printlog " + Copy selected text "
    hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\html.html")
    
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
        printlog " + Write Clipboard format under pasted content "
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"text")
        printlog " + Save document as ..\user\work\html_to_draw_text.odg"
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_draw_text.odg", "draw8")
        Wait 500
        printlog " + Close saved document "
        Call hCloseDocument
        wait 500
        printlog " + Reopen saved document "
        if hFileOpen (gOfficepath + "user\work\html_to_draw_text.odg",false) = false then
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
        Wait 500
        printlog " + Close saved document "
        Call hCloseDocument
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
    gApplication = "HTML"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog " + Jump to beginning of document "
    printlog " + Select paragraph with 'Date Field' "
    printlog " + Copy selected text "
    hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\html.html")
    
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
        printlog " + Write Clipboard format under pasted content "
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"field")
        printlog " + Save document as ..\user\work\html_to_draw_field.odg"
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_draw_field.odg", "draw8")
        Wait 500
        printlog " + Close saved document "
        Call hCloseDocument
        sleep (2)
        printlog " + Reopen saved document "
        hFileOpen( gOfficepath + "user\work\html_to_draw_field.odg",false)
        Wait 500
        printlog " + Close saved document "
        Call hCloseDocument
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
  gApplication = "HTML"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
  printlog " + Jump to beginning of document "
  printlog " + Select 'Table' "
  printlog " + Copy selected table "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\html.html")
  
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
     printlog " + Write Clipboard format under pasted content "
     Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"table")
     printlog " + Save document as ..\user\work\html_to_draw_table.odg"
     Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_draw_table.odg", "draw8")
     Wait 500
     printlog " + Close saved document "
     Call hCloseDocument
     wait 500
     printlog " + Reopen saved document "
     hFileOpen( gOfficepath + "user\work\html_to_draw_table.odg",false)
     Wait 500
     printlog " + Close saved document "
     Call hCloseDocument
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
    gApplication = "HTML"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Linked Graphic' "
    printlog " + Copy selected Linked Graphic "
    hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\html.html")
    
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
        printlog " + Write Clipboard format under pasted content "
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"graphicL")
        printlog " + Save document as ..\user\work\html_to_draw_graphic1.odg"
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_draw_graphic1.odg", "draw8")
        Wait 500
        printlog " + Close saved document "
        Call hCloseDocument
        wait 500
        printlog " + Reopen saved document "
        hFileOpen( gOfficepath + "user\work\html_to_draw_graphic1.odg",false)
        Wait 500
        printlog " + Close saved document "
        Call hCloseDocument
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
    gApplication = "HTML"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Control' "
    printlog " + Copy selected Control "
    hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\html.html")
    
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
        printlog " + Write Clipboard format under pasted content "
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"control")
        printlog " + Save document as ..\user\work\html_to_draw_control.odg"
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_draw_control.odg", "draw8")
        Wait 500
        printlog " + Close saved document "
        Call hCloseDocument
        wait 500
        printlog " + Reopen saved document "
        hFileOpen( gOfficepath + "user\work\html_to_draw_control.odg",false)
        Wait 500
        printlog " + Close saved document "
        Call hCloseDocument
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
    gApplication = "HTML"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog " + Jump to beginning of document "
    printlog " + Select 'Floating Frame' "
    printlog " + Copy selected Floating Frame "
    hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\html.html")
    
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
        printlog " + Write Clipboard format under pasted content "
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"control")
        printlog " + Save document as ..\user\work\html_to_draw_float.odg"
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_draw_float.odg", "draw8")
        Wait 500
        printlog " + Close saved document "
        Call hCloseDocument
        wait 500
        printlog " + Reopen saved document "
        hFileOpen( gOfficepath + "user\work\html_to_draw_float.odg",false)
        Wait 500
        printlog " + Close saved document "
        Call hCloseDocument
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
