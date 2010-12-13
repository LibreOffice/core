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

sub w_215_

    Call CalcToWriterText1
    Call CalcToWriterText2
    Call CalcToWriterCalculation
    Call CalcToWriterHyperlink1
    Call CalcToWriterHyperlink2
    Call CalcToWriterDrawObject
    Call CalcToWriterOLE

end sub

'--------------------------------------------------------

testcase CalcToWriterText1
  gApplication = "CALC"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\calc.sxc' "
  printlog "  Jump to beginning of document "
  printlog "  Select cell <A2> "
  printlog "  Copy selected text "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\calc.sxc")
  
  Kontext "DocumentCalc"
  printlog "  Jump to beginning of document "
  DocumentCalc.TypeKeys "<Mod1 Home>"
  DocumentCalc.TypeKeys "<Down>"
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
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
    Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"ctext")
    printlog "  Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_textformat_wholecell.odt", "writer8")
    printlog "  Save document as ..\user\work\writer_to_writer_textformat_wholecell.odt"
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
    wait 500
    printlog "  Reopen saved document "
    if hFileOpen (gOfficepath + "user\work\writer_to_writer_textformat_wholecell.odt",true) = false then
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
    printlog "  Close saved document "
    Call hCloseDocument
  else
    Warnlog "Dialog 'Paste Special' is not up!"
    Call hCloseDocument
    goto endsub
  end if
  printlog "  Close active document "
  Call hCloseDocument
endcase

' ---------------------------------------------------------------------------------

testcase CalcToWriterText2
  gApplication = "CALC"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\calc.sxc' "
  printlog "  Jump to beginning of document "
  printlog "  Select cell <A1>, press F2 and select all "
  printlog "  Copy selected text "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\calc.sxc")
  
  Kontext "DocumentCalc"
  printlog "  Jump to beginning of document "
  DocumentCalc.TypeKeys "<Mod1 Home>"
  DocumentCalc.TypeKeys "<Down>"
  DocumentCalc.TypeKeys "<F2>"
  DocumentCalc.TypeKeys "<Mod1 A>"
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
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
    printlog "  Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_textformat_cellcontent.odt", "writer8")
    printlog "  Save document as ..\user\work\writer_to_writer_textformat_cellcontent.odt"
    Wait 500
    printlog "  Close saved document "
    Call hCloseDocument
    wait 500
    printlog "  Reopen saved document "
    hFileOpen( gOfficepath + "user\work\writer_to_writer_textformat_cellcontent.odt",false)
    Wait 500
    printlog "  Close saved document "
    Call hCloseDocument
  else
    Warnlog "Dialog 'Paste Special' is not up!"
    Call hCloseDocument
    goto endsub
  end if
  printlog "  Close active document "
  Call hCloseDocument
endcase

' ---------------------------------------------------------------------------------

testcase CalcToWriterCalculation
  gApplication = "CALC"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\calc.sxc' "
  printlog "  Jump to beginning of document "
  printlog "  Select cell <A5:C5> "
  printlog "  Copy selected text "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\calc.sxc")
  
  Kontext "DocumentCalc"
  printlog "  Jump to beginning of document "
  DocumentCalc.TypeKeys "<Mod1 Home>"
  DocumentCalc.TypeKeys "<Down>",4
  DocumentCalc.TypeKeys "<Shift Right>", 2
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
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
    Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"ctext")
    printlog "  Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_calculation_cellformat.odt", "writer8")
    printlog "  Save document as ..\user\work\writer_to_writer_calculation_cellformat.odt"
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
    wait 500
    printlog "  Reopen saved document "
    if hFileOpen (gOfficepath + "user\work\writer_to_writer_calculation_cellformat.odt",true) = false then
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
    printlog "  Close saved document "
    Call hCloseDocument
  else
    Warnlog "Dialog 'Paste Special' is not up!"
    Call hCloseDocument
    goto endsub
  end if
  printlog "  Close active document "
  Call hCloseDocument
endcase

' ---------------------------------------------------------------------------------

testcase CalcToWriterHyperlink1
  gApplication = "CALC"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\calc.sxc' "
  printlog "  Jump to beginning of document "
  printlog "  Select cell <A8> "
  printlog "  Copy selected text "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\calc.sxc")
  
  Kontext "DocumentCalc"
  printlog "  Jump to beginning of document "
  DocumentCalc.TypeKeys "<Mod1 Home>"
  DocumentCalc.TypeKeys "<Down>",7
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
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
    Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"ctext")
    printlog "  Write Clipboard format under pasted content "
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_hyperlink_wholecell.odt", "writer8")
    printlog "  Save document as ..\user\work\writer_to_writer_hyperlink_wholecell.odt"
    Wait 500
    Call hCloseDocument
    printlog "  Close saved document "
    wait 500
    printlog "  Reopen saved document "
    if hFileOpen (gOfficepath + "user\work\writer_to_writer_hyperlink_wholecell.odt",true) = false then
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
    printlog "  Close saved document "
    Call hCloseDocument
  else
    Warnlog "Dialog 'Paste Special' is not up!"
    Call hCloseDocument
    goto endsub
  end if
  printlog "  Close active document "
  Call hCloseDocument
endcase

' ---------------------------------------------------------------------------------

testcase CalcToWriterHyperlink2
  gApplication = "CALC"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\calc.sxc' "
  printlog "  Jump to beginning of document "
  printlog "  Select cell <A8> "
  printlog "  Copy selected text "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\calc.sxc")
  
  Kontext "DocumentCalc"
  printlog "  Jump to beginning of document "
  DocumentCalc.TypeKeys "<Mod1 Home>"
  DocumentCalc.TypeKeys "<Down>",7
  DocumentCalc.TypeKeys "<F2>"
  DocumentCalc.TypeKeys "<Mod1 A>"
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
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
    printlog "  Write Clipboard format under pasted content "
    Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"text")
    printlog "  Save document as ..\user\work\writer_to_writer_hyperlink_cellcontent.odt"
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_hyperlink_cellcontent.odt", "writer8")
    Wait 500
    printlog "  Close saved document "
    Call hCloseDocument
    wait 500
    printlog "  Reopen saved document "
    hFileOpen( gOfficepath + "user\work\writer_to_writer_hyperlink_cellcontent.odt",false)
    Wait 500
    printlog "  Close saved document "
    Call hCloseDocument
  else
    Warnlog "Dialog 'Paste Special' is not up!"
    Call hCloseDocument
    goto endsub
  end if
  printlog "  Close active document "
  Call hCloseDocument
endcase

' ---------------------------------------------------------------------------------

testcase CalcToWriterDrawObject
  gApplication = "CALC"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\calc.sxc' "
  printlog "  Jump to beginning of document "
  printlog "  Select Drawing object "
  printlog "  Copy selected object "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\calc.sxc")
  
  Kontext "DocumentCalc"
  printlog "  Jump to beginning of document "
  Call wNavigatorAuswahl(8,1)
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
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
    printlog "  Write Clipboard format under pasted content "
    Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"ctext")
    printlog "  Save document as ..\user\work\writer_to_writer_draw.odt"
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_draw.odt", "writer8")
    Wait 500
    printlog "  Close saved document "
    Call hCloseDocument
    wait 500
    printlog "  Reopen saved document "
    hFileOpen( gOfficepath + "user\work\writer_to_writer_draw.odt",false)
    Wait 500
    printlog "  Close saved document "
    Call hCloseDocument
  else
    Warnlog "Dialog 'Paste Special' is not up!"
    Call hCloseDocument
    goto endsub
  end if
  printlog "  Close active document "
  Call hCloseDocument
endcase

' ---------------------------------------------------------------------------------

testcase CalcToWriterOLE
  gApplication = "CALC"
  printlog "  Open File '..\\writer\\optional\\input\\clipboard\\calc.sxc' "
  printlog "  Jump to beginning of document "
  printlog "  Select OLE-Object "
  printlog "  Copy selected object "
  hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\calc.sxc")
  
  Kontext "DocumentCalc"
  printlog "  Jump to beginning of document "
  Call wNavigatorAuswahl(6,1)
  EditCopy
  gApplication = "WRITER"
  printlog "  Open a new writerdocument "
  Call hNewDocument
  Sleep 2
  try
     EditPasteSpecialWriter
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
    printlog "  Write Clipboard format under pasted content "
    Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"ctext")
    printlog "  Save document as ..\user\work\writer_to_writer_ole.odt"
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\writer_to_writer_ole.odt", "writer8")
    Wait 500
    printlog "  Close saved document "
    Call hCloseDocument
    wait 500
    printlog "  Reopen saved document "
    hFileOpen( gOfficepath + "user\work\writer_to_writer_ole.odt",false)
    Wait 500
    printlog "  Close saved document "
    Call hCloseDocument
  else
    Warnlog "Dialog 'Paste Special' is not up!"
    Call hCloseDocument
    goto endsub
  end if
  printlog "  Close active document "
  Call hCloseDocument
endcase

