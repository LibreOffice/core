'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: options_so_5.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:15 $
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
'* owner : thorsten.bosbach@sun.com
'*
'* short description : general option test 
'*                                           
'\******************************************************************************


testcase func_StarOfficeView_3

   Dim bSave as Boolean
   Dim iSave as Integer
   Dim xPos, yPos as Integer

   '///StarOffice / View => 'Buttons' ( flat/3D-buttons, large/normal )
   printlog " - view"
   printlog "   - buttons"

   '///+open a new writer doc
   hNewDocument

   '///+save the sizes of the writer doc
   Kontext "DocumentWriter"
   xPos = DocumentWriter.getSizeX
   yPos = DocumentWriter.getSizeY

   '///+open tools/options/staroffice/view
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   
   '///+save the default settings for 'large buttons' and 'flat buttons'
   iSave = IconScale.getSelIndex()
   
   '///+select 'large buttons'
   printlog "     - set buttonsize to 'large'"
   IconScale.Select( 3 )
   
   '///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   '///+check if the buttons in the toolbars are bigger ( testtool : comparison between the sizes before and after the changes )
   Kontext "DocumentWriter"
   if DocumentWriter.getSizeX = xPos AND DocumentWriter.getSizeY = yPos then 
      warnlog "Perhaps the buttons are not large!"
      endif

   '///+open tools/options/staroffice/view
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   
   '///+select buttonsize 'small'
   printlog "     - set buttonsize to 'small'"
   IconScale.Select( 2 )
   
   '///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   '///+check if the buttons in the toolbars are normal ( testtool : comparison between the saved sizes before the changes )
   Kontext "DocumentWriter"
   if DocumentWriter.getSizeX <> xPos OR DocumentWriter.getSizeY <> yPos then 
      warnlog "Perhaps the buttons are not small!"
      endif

   '///+open tools/options/staroffice/view
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   
   '///+set 'Icon Size' back to the default
   printlog "     - large buttons => default"
   if iSave <> 1 then 
      IconScale.Select( iSave ) 
      endif
   
   '///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   '///+close the writer doc
   hCloseDocument
endcase

'*******************************************************************************

testcase func_StarOfficeView_4
  Dim bSave ( 10 ) as Boolean
  Dim i as Integer, iCount as Integer

'///StarOffice / View => 'Options'
 printlog " - view"
 printlog "   - options"

'///open a new writer doc
   hNewDocument

'///+open tools / options / StarOffice / View
'///+save settings for all 'options'
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   bSave(4) = VorschauInSchriftlisten.IsChecked
   bSave(6) = SchriftenHistorie.IsChecked

'///<b>inactive menu items</b>
 printlog "     - inactive menu items : => check"
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open the context menu on a writer doc and save the number of menu items
   Kontext "DocumentWriter"
   DocumentWriter.OpenContextMenu
   iCount = hMenuItemGetCount
   Sleep (2)
   hMenuClose

'///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
 printlog "     - inactive menu items : => uncheck"
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open the context menu on a writer doc and compare the number of menu items with the numbers before
'///+ the behaviour changed: the checkbox in the options does not affect the context menu! ///'
   Kontext "DocumentWriter"
   DocumentWriter.OpenContextMenu
   if hMenuItemGetCount <> iCount then
      Warnlog "The number of entries at the context menu on a writer doc is NOT the same at 'show inactive' is checked or not => BUG!"
   end if
   Sleep (2)
   hMenuClose

'///<b>single line tab headings</b>
'///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
 printlog "     - preview in fontlist : => Uncheck"
'///check 'preview in fontlist'
   VorschauInSchriftlisten.Check
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+select all Fonts in the fontlist in object bar => testtool cannot check if the preview is shown
   Kontext "textobjectbar"
   if textobjectbar.exists then
   for i=1 to Schriftart.GetItemCount
      Schriftart.Select i
   next i
   else
   warnlog "No Textobjectbar?"
   endif

'///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
 printlog "     - preview in fontlist : => check"
'///uncheck 'preview in fontlist'
   VorschauInSchriftlisten.UnCheck
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+select all Fonts in the fontlist in object bar => testtool cannot check if the preview is not shown
   Kontext "textobjectbar"
   for i=1 to Schriftart.GetItemCount
      Schriftart.Select i
   next i

'///<b>font history</b>
'///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   if bSave(4) = TRUE then VorschauInSchriftlisten.Check else VorschauInSchriftlisten.UnCheck
'///reset 'preview in fontlist' to default
 printlog "     - font history : => Uncheck"
'///check 'font history'
   SchriftenHistorie.Check
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+select all Fonts in the fontlist in object bar => testtool cannot check if font history is not shown
   Kontext "textobjectbar"
   for i=1 to Schriftart.GetItemCount
      Schriftart.Select i
   next i

'///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
 printlog "     - font history : => check"
'///uncheck 'font history'
   SchriftenHistorie.UnCheck
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+select all Fonts in the fontlist in object bar => testtool cannot check if font history is not shown
   Kontext "textobjectbar"
   for i=1 to Schriftart.GetItemCount
      Schriftart.Select i
   next i

'///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///reset 'font history' to default
   if bSave(6) = TRUE then SchriftenHistorie.Check else SchriftenHistorie.UnCheck
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///close the writer document
   hCloseDocument

endcase

'*******************************************************************************

testcase func_StarOfficeView_5

   '///StarOffice / View => '3D'
   Dim bSave ( 5 ) as Boolean
   Dim i as Integer

   printlog " - view"
   printlog "   - 3D-view"

   '///<b>disable OpenGL</b>
   '///+open tools / options / StarOffice / View
   '///+save all settings for OpenGL
   printlog "     - use OpenGL => Uncheck"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   bSave (1) = OpenGL.IsChecked
   bSave (2) = OptimierteAusgabe.IsChecked
   
   '///+uncheck OpenGL
   OpenGL.Uncheck
   '///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   '///+open a impress-doc, create cube and close the document => testtool can only test if this feature does not crash
   Call Make3D

   printlog "     - use OpenGL => Check   + optimized output => UnCheck"
   '///<b>enable OpenGL and disable optimized output</b>
   '///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   '///+check 'OpenGL' and uncheck 'optimized output'
   OpenGL.Check
   OptimierteAusgabe.Uncheck
   '///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   '///+open a impress-doc, create cube and close the document => testtool can only test if this feature does not crash
   Call Make3D

   printlog "     - use OpenGL => Check   + optimized output => Check"
   '///<b>enable OpenGL and enable optimized output</b>
   '///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   '///+check 'OpenGL' and check 'optimized output'
   OpenGL.Check
   OptimierteAusgabe.Check
   '///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open a impress-doc, create cube and close the document => testtool can only test if this feature does not crash
   Call Make3D

 printlog "     - use dithering => uncheck"
'///<b>disable 'use dithering'</b>
'///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+set 'OpenGL' and 'optimized output' to default and uncheck 'dithering'
   if bSave(2) = TRUE then OptimierteAusgabe.Check else OptimierteAusgabe.Check
   if bSave(1) = TRUE then OpenGL.Check else OpenGL.UnCheck
   bSave (3) = Dithering.IsChecked
   Dithering.Uncheck
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open a impress-doc, create cube and close the document => testtool can only test if this feature does not crash
   Call Make3D

'///<b>enable 'use dithering'</b>
 printlog "     - use dithering => Check"
'///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+check 'dithering'
   Dithering.Check
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open a impress-doc, create cube and close the document => testtool can only test if this feature does not crash
   Call Make3D

 printlog "     - use OpenGL => UnCheck  + Dithering => UnCheck!"
'///<b>disable 'OpenGL' and 'use dithering'</b>
'///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+uncheck 'OpenGL' and 'dithering'
   OpenGL.Uncheck
   Dithering.UnCheck
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open a impress-doc, create cube and close the document => testtool can only test if this feature does not crash
   Call Make3D

 printlog "     - Object refresh after interaction => check!"
'///<b>enable 'Object refresh after interaction'</b>
'///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+set 'OpenGL' and 'Dithering' to default
   if bSave(1) = TRUE then OpenGL.check else OpenGL.UnCheck
   if bSave(3) = TRUE then Dithering.Check else Dithering.UnCheck
   bSave (4) = Volldarstellung.IsChecked
'///+check 'Object refresh after interaction'
   Volldarstellung.Check
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open a impress-doc, create cube and close the document => testtool can only test if this feature does not crash
   Call Make3D

 printlog "     - use OpenGL => UnCheck  + Object refresh after interaction => check!"
'///<b>disable OpenGL and enable 'Object refresh after interaction'</b>
'///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+uncheck 'OpenGL' and check 'Object refresh after interaction'
   OpenGL.Uncheck
   Volldarstellung.Check
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open a impress-doc, create cube and close the document => testtool can only test if this feature does not crash
   Call Make3D

 printlog "     - Object refresh after interaction => UnCheck!"
'///<b>disable 'Object refresh after interaction'</b>
'///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   if bSave(1) = TRUE then OpenGL.check else OpenGL.UnCheck
'///+set 'OpenGL' to default and uncheck 'Object refresh after interaction'
   Volldarstellung.UnCheck
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open a impress-doc, create cube and close the document => testtool can only test if this feature does not crash
   Call Make3D

 printlog "     - reset to default"
'///<b>reset all 3D-Options to default</b>
'///+open tools / options / StarOffice / View
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+set all settings back to default
   if bSave(4) = TRUE then Volldarstellung.check else Volldarstellung.UnCheck
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   gApplication = "WRITER"
endcase

' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase func_StarOfficeView_6
'///StarOffice / View => Anti Aliasing - only on UNIX
  Dim bSave as Boolean
  Dim sSave as String
  Dim i as Integer

  if gPlatgroup <> "unx" then
     printlog "No test for 'anti aliasing' fonts on windows platforms!"
     goto endsub
  end if

'///open a new writer-document
 printlog "open a new writer-document"
   gApplication = "WRITER"
   hNewDocument

 printlog " - view"
 printlog "   - anti aliasing"

'///+open tools / options / StarOffice / view
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )

'///+save the raw data for 'Font anti aliasing'
   bSave = FontAntiAliasing.IsChecked
   FontAntiAliasing.Check
   sSave = AAPixel.GetText

'///+activate 'font anti aliasing' and change pixelsize to 5
 printlog " change pixelsize to 5"
   AAPixel.SetText "5"

'///+close options dialog with OK
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

'///+open all menus for the writer doc
   for i=1 to 8
      Kontext "DocumentWriter"
      DocumentWriter.UseMenu
      hMenuSelectNr i
      Sleep (2)
      hMenuClose
   next i

'///open tools / options / StarOffice / view
 printlog " change pixelsize to 15"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+activate 'font anti aliasing' and change pixelsize to 15
   AAPixel.SetText "15"

'///+close options dialog with OK
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

'///+open all menus for the writer doc
   for i=1 to 8
      Kontext "DocumentWriter"
      DocumentWriter.UseMenu
      hMenuSelectNr i
      Sleep (2)
      hMenuClose
   next i

'///open tools / options / StarOffice / view
 printlog " change pixelsize to 25"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+activate 'font anti aliasing' and change pixelsize to 25
   AAPixel.SetText "25"

'///+close options dialog with OK
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

'///+open all menus for the writer doc
   for i=1 to 8
      Kontext "DocumentWriter"
      DocumentWriter.UseMenu
      hMenuSelectNr i
      Sleep (2)
      hMenuClose
   next i

'///open tools / options / StarOffice / view
 printlog " reset all changes to raw data"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+reset all changes
   AAPixel.SetText sSave
   if bSave = TRUE then FontAntiAliasing.Check else FontAntiAliasing.UnCheck

'///+close options dialog with OK
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

'///+close the writer doc
   hCloseDocument

endcase

' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase func_StarOfficePrint_1
  Dim bSave as Boolean, bSave2 as Boolean
  Dim i as Integer

   gApplication = "DRAW"

'///StarOffice / Print => Printer Warnings
 printlog "   - printer warnings"

'///<b>check 'printer size'</b>
 printlog "     - Paper size => active"

'///+open tools / options / StarOffice / print
   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )

'///+save settings for 'paper size'
   bSave = PaperSize.IsChecked
'///+check 'paper size'
   PaperSize.Check

'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open a new drawing document
   hNewDocument
'///+open format / page
   FormatSlideDraw
'///+select the page tabpage and select the first entry out of the Format list ( A0 )
   Kontext
   Active.Setpage TabSeite
   Kontext "TabSeite"
   Papierformat.Select 1            ' select the format A0
'///+click OK
   TabSeite.OK

'///+file / print
   FilePrint
   Kontext "DruckenDlg"
'///+click OK in the print-dialog
   if DruckenDlg.Exists then
      DruckenDlg.OK
      Sleep 5
'///+wait a little bit => then you have to get a warning about the size of your document
      Kontext "WarningPrintOptions"
      if WarningPrintOptions.Exists(3) then
         WarningPrintOptions.Cancel
      else
         Warnlog "The warning-message for papersize does not turn up!"
      end if
   else
      Kontext "Active"
      if Active.Exists then Warnlog Active.GetText
      Active.OK
   end if
'///+click cancel => the document shouldn't be printed

'///close the document
   hCloseDocument

'///<b>uncheck 'printer size'</b>
 printlog "     - Paper size => disabled"
'///+open tools / options / StarOffice / print
   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )

'///+uncheck 'paper size'
   PaperSize.uncheck

'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open a new drawing document
   hNewDocument
'///+open format / page
   FormatSlideDraw
'///+select the page tabpage and select the first entry out of the Format list ( A0 )
   Kontext
   Active.Setpage TabSeite
   Kontext "TabSeite"
   Papierformat.Select 1            ' select the format A0
'///+click OK
   TabSeite.OK

'///+file / print
   FilePrint
   Kontext "DruckenDlg"
'///+click OK in the print-dialog
   if DruckenDlg.Exists then
      DruckenDlg.OK
      Sleep 5
'///+wait a little bit => the document should be printed without a warning
      Kontext "WarningPrintOptions"
      if WarningPrintOptions.Exists(3) then
         WarningPrintOptions.Cancel
         Printlog "The warning-message for papersize does exist => not realy a BUG"
      end if
   else
      Kontext "Active"
      if Active.Exists then Warnlog Active.GetText
      Active.OK
   end if

'///close the document
   hCloseDocument

'///<b>check 'transparency'</b>
 printlog "     - Transparency => active"
'///+open tools / options / StarOffice / print
   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )

'///+save settings for 'trancparency'
   bSave2 = Transparency.IsChecked
'///+uncheck 'paper size'
   PaperSize.Uncheck
'///+check 'tranceparency'
   Transparency.Check

'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open a new drawing document
   hNewDocument
'///+create a rectangle
   hRechteckErstellen ( 30, 30, 60, 60 )

'///+format/area => transparency => check transparency
   FormatArea
   Kontext
   Active.Setpage TabTransparenz
   Kontext "TabTransparenz"
   LineareTransparenz.Check
'///+click OK
   TabTransparenz.OK

'///+file / print
   FilePrint
   Kontext "DruckenDlg"
'///+click OK in the print-dialog
   if DruckenDlg.Exists then
      DruckenDlg.OK
      Sleep 5
'///+wait a little bit => then you have to get a warning about the printing trancparency
      Kontext "WarningPrintOptions"
      if WarningPrintOptions.Exists(3) then
         WarningPrintOptions.Cancel
      else
         Warnlog "The warning-message for papersize does not turn up!"
      end if
   else
      Kontext "Active"
      if Active.Exists then Warnlog Active.GetText
      Active.OK
   end if
'///+click cancel => the document shouldn't be printed

'///close the document
   hCloseDocument

'///<b>uncheck 'transparency'</b>
 printlog "     - Transparency => disabled"
'///+open tools / options / StarOffice / print
   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )

'///+uncheck 'tranceparency'
   Transparency.Uncheck

'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open a new drawing document
   hNewDocument
'///+create a rectangle
   hRechteckErstellen ( 30, 30, 60, 60 )

'///+format/area => transparency => check transparency
   FormatArea
   Kontext
   Active.Setpage TabTransparenz
   Kontext "TabTransparenz"
   LineareTransparenz.Check
'///+click OK
   TabTransparenz.OK

'///+file / print
   FilePrint
   Kontext "DruckenDlg"
'///+click OK in the print-dialog
   if DruckenDlg.Exists then
      DruckenDlg.OK
      Sleep 5
'///+wait a little bit => then you have to get a warning about the printing trancparency
      Kontext "WarningPrintTransparency"
      if WarningPrintTransparency.Exists(3) then
         WarningPrintTransparency.Cancel
         Warnlog "The warning-message for papersize does exist => BUG!"
      end if
   else
      Kontext "Active"
      if Active.Exists then Warnlog Active.GetText
      Active.OK
   end if
'///+click cancel => the document shouldn't be printed

'///close the document => perhaps it take some time, before you can close the document
   for i=1 to 100
      FileClose
      Sleep 2
      Kontext "Active"
      if Active.Exists then
         try
            Active.OK
            Sleep ( 5 )
         catch
            Active.No
            i=101
         endcatch
      end if
   next i

'///reset the printing options
 printlog "     - reset the printing options"

'///+open tools / options / StarOffice / print
   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )
'///+reset 'paper size' and 'trancparency' to default
   if bSave  = TRUE then PaperSize.Check else PaperSize.Uncheck
   if bSave2 = TRUE then Transparency.Check  else Transparency.Uncheck

'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

endcase

