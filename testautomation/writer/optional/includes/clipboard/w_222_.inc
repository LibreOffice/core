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

sub w_222_

    Call DrawToWriterScrollingText
    Call DrawToWriterGroupedObject
    Call DrawToWriter3D
    Call DrawToWriterFontwork
    Call DrawToWriterBullets
    Call DrawToWriterDimensionLines
    Call DrawToWriterConnectors

end sub

'------------------------------------------------------------------------------

testcase DrawToWriterScrollingText

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog "  Jump to beginning of document "
  printlog "  Select Scrolling text "
  printlog "  Copy selected text "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<Tab>"
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
  catch
     Warnlog "Unable to execute 'Edit / Paste Special' ! Test failed!"
     Call hCloseDocument ' Close Source-File
     Call hCloseDocument ' Close Target-File
     goto endsub
  endcatch
  Wait 500
  printlog "  Paste 'Scrolling Text' in all available clipboard formats "
  Kontext "InhaltEinfuegen"
  if InhaltEinfuegen.Exists then
    Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"DRAW")
    printlog "  Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_writer_scrolling_text.odt", "writer8")
    printlog "  Save document as ..\user\work\draw_to_writer_scrolling_text.odt"
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_writer_scrolling_text.odt",false)
    printlog "  Reopen saved document "
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
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

' ---------------------------------------------------------------------------------

testcase DrawToWriterGroupedObject

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog "  Jump to beginning of document "
  printlog "  Select Grouped Object "
  printlog "  Copy selected Object "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<Tab>",2
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
  catch
     Warnlog "Unable to execute 'Edit / Paste Special' ! Test failed!"
     Call hCloseDocument ' Close Source-File
     Call hCloseDocument ' Close Target-File
     goto endsub
  endcatch
  Wait 500
  printlog "  Paste 'Text' in all available clipboard formats "
  Kontext "InhaltEinfuegen"
  if InhaltEinfuegen.Exists then
    Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"DRAW")
    printlog "  Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_writer_grouped_object.odt", "writer8")
    printlog "  Save document as ..\user\work\draw_to_writer_grouped_object.odt"
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_writer_grouped_object.odt",false)
    printlog "  Reopen saved document "
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
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

' ---------------------------------------------------------------------------------

testcase DrawToWriter3D

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog "  Jump to beginning of document "
  printlog "  Select cell <A5:C5> "
  printlog "  Copy selected text "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<Tab>",3
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
  catch
     Warnlog "Unable to execute 'Edit / Paste Special' ! Test failed!"
     Call hCloseDocument ' Close Source-File
     Call hCloseDocument ' Close Target-File
     goto endsub
  endcatch
  Wait 500
  printlog "  Paste 'Text' in all available clipboard formats "
  Kontext "InhaltEinfuegen"
  if InhaltEinfuegen.Exists then
    Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"DRAW")
    printlog "  Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_writer_3d.odt", "writer8")
    printlog "  Save document as ..\user\work\draw_to_writer_3d.odt"
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_writer_3d.odt",false)
    printlog "  Reopen saved document "
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
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

' ---------------------------------------------------------------------------------

testcase DrawToWriterFontwork

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog "  Jump to beginning of document "
  printlog "  Select Fontwork "
  printlog "  Copy selected Fontwork "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<Tab>",4
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
  catch
     Warnlog "Unable to execute 'Edit / Paste Special' ! Test failed!"
     Call hCloseDocument ' Close Source-File
     Call hCloseDocument ' Close Target-File
     goto endsub
  endcatch
  Wait 500
  printlog "  Paste 'Text' in all available clipboard formats "
  Kontext "InhaltEinfuegen"
  if InhaltEinfuegen.Exists then
    Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"DRAW")
    printlog "  Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_writer_fontwork.odt", "writer8")
    printlog "  Save document as ..\user\work\draw_to_writer_fontwork.odt"
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_writer_fontwork.odt",false)
    printlog "  Reopen saved document "
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
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

' ---------------------------------------------------------------------------------

testcase DrawToWriterBullets

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog "  Jump to beginning of document "
  printlog "  Select Bullets "
  printlog "  Copy selected Bullets "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<Tab>",5
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
  catch
     Warnlog "Unable to execute 'Edit / Paste Special' ! Test failed!"
     Call hCloseDocument ' Close Source-File
     Call hCloseDocument ' Close Target-File
     goto endsub
  endcatch
  Wait 500
  printlog "  Paste 'Text' in all available clipboard formats "
  Kontext "InhaltEinfuegen"
  if InhaltEinfuegen.Exists then
    Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"DRAW")
    printlog "  Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_writer_bullets.odt", "writer8")
    printlog "  Save document as ..\user\work\draw_to_writer_bullets.odt"
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_writer_bullets.odt",false)
    printlog "  Reopen saved document "
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
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

' ---------------------------------------------------------------------------------

testcase DrawToWriterDimensionLines

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog "  Jump to beginning of document "
  printlog "  Select Dimension Lines "
  printlog "  Copy selected Dimension Lines "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<PageDown>"
  DocumentDraw.TypeKeys "<Tab>"
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
  catch
     Warnlog "Unable to execute 'Edit / Paste Special' ! Test failed!"
     Call hCloseDocument ' Close Source-File
     Call hCloseDocument ' Close Target-File
     goto endsub
  endcatch
  Wait 500
  printlog "  Paste 'Dimension Lines' in all available clipboard formats "
  Kontext "InhaltEinfuegen"
  if InhaltEinfuegen.Exists then
    Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"DRAW")
    printlog "  Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_writer_dim_lines.odt", "writer8")
    printlog "  Save document as ..\user\work\draw_to_writer_dim_lines.odt"
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_writer_dim_lines.odt",false)
    printlog "  Reopen saved document "
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
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

' ---------------------------------------------------------------------------------

testcase DrawToWriterConnectors

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog "  Jump to beginning of document "
  printlog "  Select Connectors "
  printlog "  Copy selected Connectors "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<PageDown>"
  DocumentDraw.TypeKeys "<Tab>"
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
  catch
     Warnlog "Unable to execute 'Edit / Paste Special' ! Test failed!"
     Call hCloseDocument ' Close Source-File
     Call hCloseDocument ' Close Target-File
     goto endsub
  endcatch
  Wait 500
  printlog "  Paste 'Connectors' in all available clipboard formats "
  Kontext "InhaltEinfuegen"
  if InhaltEinfuegen.Exists then
    Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"DRAW")
    printlog "  Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_writer_connectors.odt", "writer8")
    printlog "  Save document as ..\user\work\draw_to_writer_connectors.odt"
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_writer_connectors.odt",false)
    printlog "  Reopen saved document "
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
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

'------------------------------------------------------------------------------
