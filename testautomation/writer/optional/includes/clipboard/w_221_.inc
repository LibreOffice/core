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

sub w_221_

    Call DrawToHTMLScrollingText
    Call DrawToHTMLGroupedObject
    Call DrawToHTML3D
    Call DrawToHTMLFontwork
    Call DrawToHTMLBullets
    Call DrawToHTMLDimensionLines
    Call DrawToHTMLConnectors

end sub


testcase DrawToHTMLScrollingText

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog " + Jump to beginning of document "
  printlog " + Select Scrolling text "
  printlog " + Copy selected text "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<Tab>"
  EditCopy
  gApplication = "HTML"
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
    printlog " + Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_html_scrolling_text.html", "HTML")
    printlog " + Save document as ..\user\work\draw_to_html_scrolling_text.html"
    Wait 500
    Call hCloseDocument
    printlog " + Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_html_scrolling_text.html",false)
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

' ---------------------------------------------------------------------------------

testcase DrawToHTMLGroupedObject

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog " + Jump to beginning of document "
  printlog " + Select Grouped Object "
  printlog " + Copy selected Object "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<Tab>",2
  EditCopy
  gApplication = "HTML"
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
    printlog " + Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_html_grouped_object.html", "HTML")
    printlog " + Save document as ..\user\work\draw_to_html_grouped_object.html"
    Wait 500
    Call hCloseDocument
    printlog " + Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_html_grouped_object.html",false)
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

' ---------------------------------------------------------------------------------

testcase DrawToHTML3D

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog " + Jump to beginning of document "
  printlog " + Select cell <A5:C5> "
  printlog " + Copy selected text "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<Tab>",3
  EditCopy
  gApplication = "HTML"
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
    printlog " + Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_html_3d.html", "HTML")
    printlog " + Save document as ..\user\work\draw_to_html_3d.html"
    Wait 500
    Call hCloseDocument
    printlog " + Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_html_3d.html",false)
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

' ---------------------------------------------------------------------------------

testcase DrawToHTMLFontwork

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog " + Jump to beginning of document "
  printlog " + Select Fontwork "
  printlog " + Copy selected Fontwork "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<Tab>",4
  EditCopy
  gApplication = "HTML"
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
    printlog " + Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_html_fontwork.html", "HTML")
    printlog " + Save document as ..\user\work\draw_to_html_fontwork.html"
    Wait 500
    Call hCloseDocument
    printlog " + Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_html_fontwork.html",false)
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

' ---------------------------------------------------------------------------------

testcase DrawToHTMLBullets

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog " + Jump to beginning of document "
  printlog " + Select Bullets "
  printlog " + Copy selected Bullets "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<Tab>",5
  EditCopy
  gApplication = "HTML"
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
    printlog " + Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_html_bullets.html", "HTML")
    printlog " + Save document as ..\user\work\draw_to_html_bullets.html"
    Wait 500
    Call hCloseDocument
    printlog " + Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_html_bullets.html",false)
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

' ---------------------------------------------------------------------------------

testcase DrawToHTMLDimensionLines

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog " + Jump to beginning of document "
  printlog " + Select Dimension Lines "
  printlog " + Copy selected Dimension Lines "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<PageDown>"
  DocumentDraw.TypeKeys "<Tab>"
  EditCopy
  gApplication = "HTML"
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
    printlog " + Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_html_dim_lines.html", "HTML")
    printlog " + Save document as ..\user\work\draw_to_html_dim_lines.html"
    Wait 500
    Call hCloseDocument
    printlog " + Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_html_dim_lines.html",false)
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

' ---------------------------------------------------------------------------------

testcase DrawToHTMLConnectors

  gApplication = "DRAW"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\draw.sxd' "
  printlog " + Jump to beginning of document "
  printlog " + Select Connectors "
  printlog " + Copy selected Connectors "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\draw.sxd")
  
  Kontext "DocumentDraw"
  printlog "  Jump to beginning of document "
  DocumentDraw.TypeKeys "<PageDown>"
  DocumentDraw.TypeKeys "<Tab>"
  EditCopy
  gApplication = "HTML"
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
    printlog " + Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\draw_to_html_connectors.html", "HTML")
    printlog " + Save document as ..\user\work\draw_to_html_connectors.html"
    Wait 500
    Call hCloseDocument
    printlog " + Close saved document "
    wait 500
    hFileOpen( gOfficepath + "user\work\draw_to_html_connectors.html",false)
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

