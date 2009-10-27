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

   printlog " - view"
   printlog "   - buttons"

   hNewDocument

   Kontext "DocumentWriter"
   xPos = DocumentWriter.getSizeX
   yPos = DocumentWriter.getSizeY

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   
   iSave = IconScale.getSelIndex()
   
   printlog "     - set buttonsize to 'large'"
   IconScale.Select( 3 )
   
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "DocumentWriter"
   if DocumentWriter.getSizeX = xPos AND DocumentWriter.getSizeY = yPos then 
      warnlog "Perhaps the buttons are not large!"
      endif

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   
   printlog "     - set buttonsize to 'small'"
   IconScale.Select( 2 )
   
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "DocumentWriter"
   if DocumentWriter.getSizeX <> xPos OR DocumentWriter.getSizeY <> yPos then 
      warnlog "Perhaps the buttons are not small!"
      endif

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   
   printlog "     - large buttons => default"
   if iSave <> 1 then 
      IconScale.Select( iSave ) 
      endif
   
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   hCloseDocument
endcase

'*******************************************************************************

testcase func_StarOfficeView_4
  Dim bSave ( 10 ) as Boolean
  Dim i as Integer, iCount as Integer

 printlog " - view"
 printlog "   - options"

   hNewDocument

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   bSave(4) = VorschauInSchriftlisten.IsChecked
   bSave(6) = SchriftenHistorie.IsChecked

 printlog "     - inactive menu items : => check"
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "DocumentWriter"
   DocumentWriter.OpenContextMenu
   iCount = hMenuItemGetCount
   Sleep (2)
   hMenuClose

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
 printlog "     - inactive menu items : => uncheck"
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "DocumentWriter"
   DocumentWriter.OpenContextMenu
   if hMenuItemGetCount <> iCount then
      Warnlog "The number of entries at the context menu on a writer doc is NOT the same at 'show inactive' is checked or not => BUG!"
   end if
   Sleep (2)
   hMenuClose

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
 printlog "     - preview in fontlist : => Uncheck"
   VorschauInSchriftlisten.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "textobjectbar"
   if textobjectbar.exists then
   for i=1 to Schriftart.GetItemCount
      Schriftart.Select i
   next i
   else
   warnlog "No Textobjectbar?"
   endif

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
 printlog "     - preview in fontlist : => check"
   VorschauInSchriftlisten.UnCheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "textobjectbar"
   for i=1 to Schriftart.GetItemCount
      Schriftart.Select i
   next i

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   if bSave(4) = TRUE then VorschauInSchriftlisten.Check else VorschauInSchriftlisten.UnCheck
 printlog "     - font history : => Uncheck"
   SchriftenHistorie.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "textobjectbar"
   for i=1 to Schriftart.GetItemCount
      Schriftart.Select i
   next i

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
 printlog "     - font history : => check"
   SchriftenHistorie.UnCheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "textobjectbar"
   for i=1 to Schriftart.GetItemCount
      Schriftart.Select i
   next i

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   if bSave(6) = TRUE then SchriftenHistorie.Check else SchriftenHistorie.UnCheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   hCloseDocument

endcase

'*******************************************************************************

testcase func_StarOfficeView_5

   Dim bSave ( 5 ) as Boolean
   Dim i as Integer

   printlog " - view"
   printlog "   - 3D-view"

   printlog "     - use OpenGL => Uncheck"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   bSave (1) = OpenGL.IsChecked
   bSave (2) = OptimierteAusgabe.IsChecked
   
   OpenGL.Uncheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Call Make3D

   printlog "     - use OpenGL => Check   + optimized output => UnCheck"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   OpenGL.Check
   OptimierteAusgabe.Uncheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Call Make3D

   printlog "     - use OpenGL => Check   + optimized output => Check"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   OpenGL.Check
   OptimierteAusgabe.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Call Make3D

 printlog "     - use dithering => uncheck"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   if bSave(2) = TRUE then OptimierteAusgabe.Check else OptimierteAusgabe.Check
   if bSave(1) = TRUE then OpenGL.Check else OpenGL.UnCheck
   bSave (3) = Dithering.IsChecked
   Dithering.Uncheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Call Make3D

 printlog "     - use dithering => Check"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   Dithering.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Call Make3D

 printlog "     - use OpenGL => UnCheck  + Dithering => UnCheck!"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   OpenGL.Uncheck
   Dithering.UnCheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Call Make3D

 printlog "     - Object refresh after interaction => check!"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   if bSave(1) = TRUE then OpenGL.check else OpenGL.UnCheck
   if bSave(3) = TRUE then Dithering.Check else Dithering.UnCheck
   bSave (4) = Volldarstellung.IsChecked
   Volldarstellung.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Call Make3D

 printlog "     - use OpenGL => UnCheck  + Object refresh after interaction => check!"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   OpenGL.Uncheck
   Volldarstellung.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Call Make3D

 printlog "     - Object refresh after interaction => UnCheck!"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   if bSave(1) = TRUE then OpenGL.check else OpenGL.UnCheck
   Volldarstellung.UnCheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Call Make3D

 printlog "     - reset to default"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   if bSave(4) = TRUE then Volldarstellung.check else Volldarstellung.UnCheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   gApplication = "WRITER"
endcase

' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase func_StarOfficeView_6
  Dim bSave as Boolean
  Dim sSave as String
  Dim i as Integer

  if gPlatgroup <> "unx" then
     printlog "No test for 'anti aliasing' fonts on windows platforms!"
     goto endsub
  end if

 printlog "open a new writer-document"
   gApplication = "WRITER"
   hNewDocument

 printlog " - view"
 printlog "   - anti aliasing"

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )

   bSave = FontAntiAliasing.IsChecked
   FontAntiAliasing.Check
   sSave = AAPixel.GetText

 printlog " change pixelsize to 5"
   AAPixel.SetText "5"

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

   for i=1 to 8
      Kontext "DocumentWriter"
      DocumentWriter.UseMenu
      hMenuSelectNr i
      Sleep (2)
      hMenuClose
   next i

 printlog " change pixelsize to 15"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   AAPixel.SetText "15"

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

   for i=1 to 8
      Kontext "DocumentWriter"
      DocumentWriter.UseMenu
      hMenuSelectNr i
      Sleep (2)
      hMenuClose
   next i

 printlog " change pixelsize to 25"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   AAPixel.SetText "25"

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

   for i=1 to 8
      Kontext "DocumentWriter"
      DocumentWriter.UseMenu
      hMenuSelectNr i
      Sleep (2)
      hMenuClose
   next i

 printlog " reset all changes to raw data"
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   AAPixel.SetText sSave
   if bSave = TRUE then FontAntiAliasing.Check else FontAntiAliasing.UnCheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

   hCloseDocument

endcase

' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase func_StarOfficePrint_1
  Dim bSave as Boolean, bSave2 as Boolean
  Dim i as Integer

   gApplication = "DRAW"

 printlog "   - printer warnings"

 printlog "     - Paper size => active"

   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )

   bSave = PaperSize.IsChecked
   PaperSize.Check

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   hNewDocument
   FormatSlideDraw
   Kontext
   Active.Setpage TabSeite
   Kontext "TabSeite"
   Papierformat.Select 1            ' select the format A0
   TabSeite.OK

   FilePrint
   Kontext "DruckenDlg"
   if DruckenDlg.Exists then
      DruckenDlg.OK
      Sleep 5
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

   hCloseDocument

 printlog "     - Paper size => disabled"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )

   PaperSize.uncheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   hNewDocument
   FormatSlideDraw
   Kontext
   Active.Setpage TabSeite
   Kontext "TabSeite"
   Papierformat.Select 1            ' select the format A0
   TabSeite.OK

   FilePrint
   Kontext "DruckenDlg"
   if DruckenDlg.Exists then
      DruckenDlg.OK
      Sleep 5
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

   hCloseDocument

 printlog "     - Transparency => active"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )

   bSave2 = Transparency.IsChecked
   PaperSize.Uncheck
   Transparency.Check

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   hNewDocument
   hRechteckErstellen ( 30, 30, 60, 60 )

   FormatArea
   Kontext
   Active.Setpage TabTransparenz
   Kontext "TabTransparenz"
   LineareTransparenz.Check
   TabTransparenz.OK

   FilePrint
   Kontext "DruckenDlg"
   if DruckenDlg.Exists then
      DruckenDlg.OK
      Sleep 5
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

   hCloseDocument

 printlog "     - Transparency => disabled"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )

   Transparency.Uncheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   hNewDocument
   hRechteckErstellen ( 30, 30, 60, 60 )

   FormatArea
   Kontext
   Active.Setpage TabTransparenz
   Kontext "TabTransparenz"
   LineareTransparenz.Check
   TabTransparenz.OK

   FilePrint
   Kontext "DruckenDlg"
   if DruckenDlg.Exists then
      DruckenDlg.OK
      Sleep 5
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

 printlog "     - reset the printing options"

   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )
   if bSave  = TRUE then PaperSize.Check else PaperSize.Uncheck
   if bSave2 = TRUE then Transparency.Check  else Transparency.Uncheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

endcase

