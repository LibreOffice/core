'encoding UTF-8  Do not remove or change this line!
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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : Tools->Options: OpenOffice.org Print
'*
'\******************************************************************************


testcase tOOoPrint

  Dim lbSave ( 3 ) as Boolean
  Dim lsSavePrinter ( 3 ) as String
  Dim lbSavePrinter ( 13 ) as Boolean
  Dim lsSaveFile ( 3 ) as String
  Dim lbSaveFile ( 13 ) as Boolean

   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )

 printlog " - save old settings"
 printlog "    - save settings for 'printer'"
  Printer.Check
  Sleep (2)
   lbSavePrinter (1)  = ReduceTransparency.IsChecked
   ReduceTransparency.Uncheck
   lbSave(3) = Transparency.IsChecked
   ReduceTransparency.Check
   lbSavePrinter (2)  = Automatically.IsChecked
   lbSavePrinter (3)  = NoTransparency.IsChecked

   lbSavePrinter (4)  = ReduceGradients.IsChecked
   ReduceGradients.Check
   lbSavePrinter (5)  = GradientStripes.IsChecked
   lbSavePrinter (6)  = IntermediateColor.IsChecked
   GradientStripes.Check
   lsSavePrinter (1)  = Stripes.GetText

   lbSavePrinter (7)  = ReduceBitmaps.IsChecked
   ReduceBitmaps.Check
   lbSavePrinter (8)  = GoodPrintQuality.IsChecked
   lbSavePrinter (9)  = NormalPrintQuality.IsChecked
   lbSavePrinter (10) = Resolution.IsChecked
   lbSavePrinter (11) = IncludeTransparentObjects.IsChecked
   Resolution.Check
   lsSavePrinter (2)  = DPI.GetSelText

   lbSavePrinter (12) = ConvertColorsToGrayscale.IsChecked

 printlog "    - save settings for 'print to file'"
  PrintToFile.Check
  Sleep (2)
   lbSaveFile (1)  = ReduceTransparency.IsChecked
   ReduceTransparency.Check
   lbSaveFile (2)  = Automatically.IsChecked
   lbSaveFile (3)  = NoTransparency.IsChecked

   lbSaveFile (4)  = ReduceGradients.IsChecked
   ReduceGradients.Check
   lbSaveFile (5)  = GradientStripes.IsChecked
   lbSaveFile (6)  = IntermediateColor.IsChecked
   GradientStripes.Check
   lsSaveFile (1)  = Stripes.GetText

   lbSaveFile (7)  = ReduceBitmaps.IsChecked
   ReduceBitmaps.Check
   lbSaveFile (8)  = GoodPrintQuality.IsChecked
   lbSaveFile (9)  = NormalPrintQuality.IsChecked
   lbSaveFile (10) = Resolution.IsChecked
   lbSaveFile (11) = IncludeTransparentObjects.IsChecked
   Resolution.Check
   lsSaveFile (2)  = DPI.GetSelText

   lbSaveFile (12) = ConvertColorsToGrayscale.IsChecked

 printlog "    - save general settings "
   lbSave(1) = PaperOrientation.IsChecked
   lbSave(2) = PaperSize.IsChecked

 printlog " - invert/change settings"
 printlog "    - changes for 'printer'"
  Printer.Check
  Sleep (2)
   if lbSavePrinter (1) = FALSE then ReduceTransparency.Check else ReduceTransparency.UnCheck
   if ReduceTransparency.IsChecked then
      NoTransparency.Check
   else
      if lbSave(3)         = FALSE then Transparency.Check else Transparency.UnCheck
   end if

   if lbSavePrinter (4) = FALSE then ReduceGradients.Check else ReduceGradients.UnCheck
   if ReduceGradients.IsChecked then
      IntermediateColor.Check
   end if

   if lbSavePrinter (7)  = FALSE then ReduceBitmaps.Check else ReduceBitmaps.UnCheck
   if ReduceBitmaps.IsChecked then
      Resolution.Check
      DPI.select 1
      if lbSavePrinter (11) = FALSE then IncludeTransparentObjects.Check else IncludeTransparentObjects.UnCheck
   end if

   if lbSavePrinter (12) = FALSE then ConvertColorsToGrayscale.Check else ConvertColorsToGrayscale.UnCheck

 printlog "    - changes for 'print to file'"
  PrintToFile.Check
  Sleep (2)
   if lbSaveFile (1) = FALSE then ReduceTransparency.Check else ReduceTransparency.UnCheck
   if ReduceTransparency.IsChecked then
      Automatically.Check
   end if

   if lbSaveFile (4) = FALSE then ReduceGradients.Check else ReduceGradients.UnCheck
   if ReduceGradients.IsChecked then
      GradientStripes.Check
      Stripes.SetText "30"
   end if

   if lbSaveFile (7)  = FALSE then ReduceBitmaps.Check else ReduceBitmaps.UnCheck
   if ReduceBitmaps.IsChecked then
      GoodPrintQuality.Check
      if lbSaveFile (11) = FALSE then IncludeTransparentObjects.Check else IncludeTransparentObjects.UnCheck
   end if

   if lbSaveFile (12) = FALSE then ConvertColorsToGrayscale.Check else ConvertColorsToGrayscale.UnCheck

 printlog "    - changes for general settings"
   if lbSave(1) = FALSE then PaperOrientation.Check else PaperOrientation.UnCheck
   if lbSave(2) = FALSE then PaperSize.Check else PaperSize.UnCheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

 printlog " - check invertation/changes"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )

 printlog "    - check for 'printer'"
  Printer.Check
  Sleep (2)
   if ReduceTransparency.IsChecked = lbSavePrinter (1) then Warnlog "Reduce transparency => changes not saved! => BugID 87446"
   if ReduceTransparency.IsChecked then
      if NoTransparency.IsChecked <> TRUE              then Warnlog "No transparency => changes not saved!"
   else
      if Transparency.IsChecked = lbSave(2)            then Warnlog "Transparency => changes not saved!"
   end if


   if ReduceGradients.IsChecked = lbSavePrinter (4)    then Warnlog "ReduceGradients => changes not saved!"
   if ReduceGradients.IsChecked then
      if IntermediateColor.IsChecked <> TRUE           then Warnlog "Intermediate color => changes not saved!"
   end if

   if ReduceBitmaps.IsChecked = lbSavePrinter (7)      then Warnlog "Reduce bitmaps => changes not saved!"
   if ReduceBitmaps.IsChecked then
      if Resolution.IsChecked <> TRUE then
         Warnlog "Resolution => changes not saved!"
      else
         if DPI.GetSelIndex <> 1                       then Warnlog "DPI => changes not saved!"
      end if
      if IncludeTransparentObjects.IsChecked = lbSavePrinter (11) then Warnlog "Include Transparent Objects => changes not saved!"
   end if

   if ConvertColorsToGrayscale.IsChecked = lbSavePrinter (12) then Warnlog "Convert Colors To Grayscale => changes not saved!"

 printlog "    - check for 'print to file'"
  PrintToFile.Check
  Sleep (2)
   if ReduceTransparency.IsChecked = lbSaveFile (1)    then Warnlog "Reduce transparency => changes not saved!"
   if ReduceTransparency.IsChecked then
      if Automatically.IsChecked  <> TRUE              then Warnlog "Automatically => changes not saved!"
   end if

   if ReduceGradients.IsChecked = lbSaveFile (4)       then Warnlog "Reduce gradients => changes not saved!"
   if ReduceGradients.IsChecked then
      if GradientStripes.IsChecked <> TRUE             then Warnlog "GradientStripes => changes not saved!"
      if GradientStripes.IsChecked then
         if Stripes.GetText <> "30"                    then Warnlog "Stripes => changes not saved!"
      end if
   end if

   if ReduceBitmaps.IsChecked = lbSaveFile (7)         then Warnlog "Reduce bitmaps => changes not saved!"

   if ReduceBitmaps.IsChecked then
      if GoodPrintQuality.IsChecked <> TRUE            then Warnlog "Good print quality => changes not saved!"
      if IncludeTransparentObjects.IsChecked = lbSaveFile (11) then Warnlog "Include transparent objects = changes not saved!"
   end if

   if ConvertColorsToGrayscale.IsChecked = lbSaveFile (12) then warnlog "Convert colors to grayscale => changes not saved!"

 printlog "    - check for general settings"
   if PaperOrientation.IsChecked = lbSave(1)           then warnlog "Paper orientation => changes not saved! #i29960"
   if PaperSize.IsChecked        = lbSave(2)           then Warnlog "Paper size => changes not saved! #i29960"

 printlog " - 2. changes"
 printlog "    - 'printer'"
  Printer.Check
  Sleep (2)
   ReduceTransparency.UnCheck
   ReduceGradients.Check
    GradientStripes.Check
    Stripes.SetText "120"
   ReduceBitmaps.Check
    GoodPrintQuality.Check
    IncludeTransparentObjects.Check
   ConvertColorsToGrayscale.UnCheck

 printlog "    - 'print to file'"
  PrintToFile.Check
   ReduceTransparency.Check
    NoTransparency.Check
   ReduceGradients.Uncheck
   ReduceBitmaps.UnCheck
   ConvertColorsToGrayscale.Check

 printlog "    - general settings"
   PaperOrientation.UnCheck
   PaperSize.Check

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check 2. changes"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )

 printlog "    - check for 'printer'"
  Printer.Check
  Sleep (2)
   if ReduceTransparency.IsChecked <> FALSE then Warnlog "Reduce transparency => changes not saved!"
   if ReduceGradients.IsChecked    <> TRUE  then
      Warnlog "ReduceGradients => changes not saved!"
   else
      if GradientStripes.IsChecked <> TRUE  then
         Warnlog "GradientStripes => changes not saved!"
      else
         if Stripes.GetText <> "120"        then Warnlog "Stripes => changes not saved!"
      end if
   end if
   if ReduceBitmaps.IsChecked <> TRUE then
      Warnlog "Reduce bitmaps => changes not saved!"
   else
      if GoodPrintQuality.IsChecked <> TRUE          then Warnlog "Good print quality => changes not saved!"
      if IncludeTransparentObjects.IsChecked <> TRUE then Warnlog "Include Transparent Objects => changes not saved!"
   end if
   if ConvertColorsToGrayscale.IsChecked <> FALSE    then Warnlog "Convert Colors To Grayscale => changes not saved!"

 printlog "    - check 'print to file'"
  PrintToFile.Check
  Sleep (2)
   if ReduceTransparency.IsChecked <> TRUE then
      Warnlog "Reduce transparency => changes not saved!"
   else
      if NoTransparency.IsChecked <> TRUE then Warnlog "No transpanency => changes not saved!"
   end if
   if ReduceGradients.IsChecked <> FALSE then Warnlog "Reduce gradients => changes not saved!"
   if ReduceBitmaps.IsChecked <> FALSE then Warnlog "Reduce bitmaps => changes not saved!"
   if ConvertColorsToGrayscale.IsChecked <> TRUE then Warnlog "Convert colors to grayscale => changes not saved!"

 printlog "    - check general settings"
   if PaperOrientation.IsChecked <> FALSE then warnlog "Paper orientation => changes not saved!"
   if PaperSize.IsChecked        <> TRUE  then Warnlog "Paper size => changes not saved! #i29960"

 printlog " - reset to saved settings"
 printlog "    - 'printer'"
  Printer.Check
  Sleep (2)
   ReduceTransparency.Check
   if lbSavePrinter (2)  = TRUE then Automatically.Check
   if lbSavePrinter (3)  = TRUE then NoTransparency.Check
   ReduceTransparency.UnCheck
   if lbSave(3)          = TRUE then Transparency.Check       else Transparency.Uncheck
   if lbSavePrinter (1)  = TRUE then ReduceTransparency.check else ReduceTransparency.Uncheck

   ReduceGradients.Check
   GradientStripes.Check
   Stripes.SetText lsSavePrinter (1)
   if lbSavePrinter (5)  = TRUE then GradientStripes.Check
   if lbSavePrinter (6)  = TRUE then IntermediateColor.Check
   if lbSavePrinter (4)  = TRUE then ReduceGradients.Check else ReduceGradients.UnCheck
   if lbSavePrinter (4)  = TRUE then ReduceGradients.Check else ReduceGradients.UnCheck

   ReduceBitmaps.Check
   Resolution.Check
   DPI.Select lsSavePrinter (2)
   if lbSavePrinter (8)  = TRUE then GoodPrintQuality.Check
   if lbSavePrinter (9)  = TRUE then NormalPrintQuality.Check
   if lbSavePrinter (10) = TRUE then Resolution.Check
   if lbSavePrinter (11) = TRUE then IncludeTransparentObjects.Check else IncludeTransparentObjects.UnCheck
   if lbSavePrinter (7)  = TRUE then ReduceBitmaps.Check else ReduceBitmaps.UnCheck

   if lbSavePrinter (12) = TRUE then ConvertColorsToGrayscale.Check else ConvertColorsToGrayscale.UnCheck

 printlog "    - 'print to file'"
  PrintToFile.Check
  Sleep (2)
   ReduceTransparency.Check
   if lbSaveFile (2)  = TRUE then Automatically.Check
   if lbSaveFile (3)  = TRUE then NoTransparency.Check
   if lbSaveFile (1)  = TRUE then ReduceTransparency.Check else ReduceTransparency.UnCheck

   ReduceGradients.Check
   GradientStripes.Check
   Stripes.SetText lsSaveFile (1)
   if lbSaveFile (5)  = TRUE then GradientStripes.Check
   if lbSaveFile (6)  = TRUE then IntermediateColor.Check
   if lbSaveFile (4)  = TRUE then ReduceGradients.Check else ReduceGradients.UnCheck

   ReduceBitmaps.Check
   Resolution.Check
   DPI.Select lsSaveFile (2)
   if lbSaveFile (8)  = TRUE then GoodPrintQuality.Check
   if lbSaveFile (9)  = TRUE then NormalPrintQuality.Check
   if lbSaveFile (10) = TRUE then Resolution.Check
   if lbSaveFile (11) = TRUE then IncludeTransparentObjects.Check else IncludeTransparentObjects.UnCheck
   if lbSaveFile (7)  = TRUE then ReduceBitmaps.Check else ReduceBitmaps.Uncheck

   if lbSaveFile (12) = TRUE then ConvertColorsToGrayscale.Check else ConvertColorsToGrayscale.UnCheck

 printlog "    - general settings"
   if lbSave(1) = TRUE then PaperOrientation.check else PaperOrientation.Uncheck
   if lbSave(2) = TRUE then PaperSize.Check else PaperSize.UnCheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check changes"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Print" )

 printlog "    - check for 'printer'"
  Printer.Check
  Sleep (2)
   if ReduceTransparency.IsChecked        <> lbSavePrinter (1)  then Warnlog "Reduce transparency => changes not saved!"
   ReduceTransparency.Check
   if Automatically.IsChecked             <> lbSavePrinter (2)  then Warnlog "Automatically => changes not saved!"
   if NoTransparency.IsChecked            <> lbSavePrinter (3)  then Warnlog "No transparency => changes not saved!"
   ReduceTransparency.UnCheck
   if Transparency.IsChecked              <> lbSave(3)          then Warnlog "Transparency => changes not saved!"
   if lbSavePrinter (1) = TRUE then ReduceTransparency.Check else ReduceTransparency.UnCheck

   if ReduceGradients.IsChecked           <> lbSavePrinter (4) then Warnlog "Reduce Gradients => changes not saved!"
   ReduceGradients.Check
   if GradientStripes.IsChecked           <> lbSavePrinter (5)  then Warnlog "GradientStripes => changes not saved!"
   if IntermediateColor.IsChecked         <> lbSavePrinter (6)  then Warnlog "Intermediate color => changes not saved!"
   GradientStripes.Check
   if Stripes.GetText <> lsSavePrinter (1)                      then Warnlog "Stripes => changes not saved!"
   if lbSavePrinter (5) = TRUE then GradientStripes.Check else IntermediateColor.Check
   if lbSavePrinter (4)  = TRUE then ReduceGradients.Check else ReduceGradients.UnCheck

   if ReduceBitmaps.IsChecked <> lbSavePrinter (7) then Warnlog "Reduce Bitmaps => changes not saved!"
   ReduceBitmaps.Check
   if GoodPrintQuality.IsChecked          <> lbSavePrinter (8)  then Warnlog "Good print quality => changes not saved!"
   if NormalPrintQuality.IsChecked        <> lbSavePrinter (9)  then Warnlog "Normal print quality => changes not saved!"
   if Resolution.IsChecked                <> lbSavePrinter (10) then Warnlog "Resolution => changes not saved!"
   if IncludeTransparentObjects.IsChecked <> lbSavePrinter (11) then Warnlog "Include transparent objects => changes not saved!"
   Resolution.Check
   if DPI.GetSelText <> lsSavePrinter (2)                       then Warnlog "DPI => changes not saved!"
   if lbSavePrinter (8)  = TRUE then GoodPrintQuality.Check
   if lbSavePrinter (9)  = TRUE then NormalPrintQuality.Check
   if lbSavePrinter (10) = TRUE then Resolution.Check
   if lbSavePrinter (7)  = TRUE then ReduceBitmaps.Check else ReduceBitmaps.Uncheck

   if ConvertColorsToGrayscale.IsChecked  <> lbSavePrinter (12) then Warnlog "Convert colors to grayscale => changes not saved!"

 printlog "    - 'print to file'"
  PrintToFile.Check
  Sleep (2)
   if ReduceTransparency.IsChecked        <> lbSaveFile (1)  then Warnlog "Reduce transparency => changes not saved!"
   ReduceTransparency.Check
   if Automatically.IsChecked             <> lbSaveFile (2)  then Warnlog "Automatically => changes not saved!"
   if NoTransparency.IsChecked            <> lbSaveFile (3)  then Warnlog "No transparency => changes not saved!"
   if lbSaveFile (1) = TRUE then ReduceTransparency.Check else ReduceTransparency.UnCheck

   if ReduceGradients.IsChecked           <> lbSaveFile (4) then Warnlog "Reduce Gradients => changes not saved!"
   ReduceGradients.Check
   if GradientStripes.IsChecked           <> lbSaveFile (5)  then Warnlog "GradientStripes => changes not saved!"
   if IntermediateColor.IsChecked         <> lbSaveFile (6)  then Warnlog "Intermediate color => changes not saved!"
   GradientStripes.Check
   if Stripes.GetText <> lsSaveFile (1)                      then Warnlog "Stripes => changes not saved!"
   if lbSaveFile (5) = TRUE then GradientStripes.Check else IntermediateColor.Check
   if lbSaveFile (4)  = TRUE then ReduceGradients.Check else ReduceGradients.UnCheck

   if ReduceBitmaps.IsChecked <> lbSaveFile (7) then Warnlog "Reduce Bitmaps => changes not saved!"
   ReduceBitmaps.Check
   if GoodPrintQuality.IsChecked          <> lbSaveFile (8)  then Warnlog "Good print quality => changes not saved!"
   if NormalPrintQuality.IsChecked        <> lbSaveFile (9)  then Warnlog "Normal print quality => changes not saved!"
   if Resolution.IsChecked                <> lbSaveFile (10) then Warnlog "Resolution => changes not saved!"
   if IncludeTransparentObjects.IsChecked <> lbSaveFile (11) then Warnlog "Include transparent objects => changes not saved!"
   Resolution.Check
   if DPI.GetSelText <> lsSavePrinter (2)                       then Warnlog "DPI => changes not saved!"
   if lbSaveFile (8)  = TRUE then GoodPrintQuality.Check
   if lbSaveFile (9)  = TRUE then NormalPrintQuality.Check
   if lbSaveFile (10) = TRUE then Resolution.Check
   if lbSaveFile (7)  = TRUE then ReduceBitmaps.Check else ReduceBitmaps.Uncheck

   if ConvertColorsToGrayscale.IsChecked  <> lbSaveFile (12) then Warnlog "Convert colors to grayscale => changes not saved!"

 printlog "    - general settings"
   if PaperOrientation.Ischecked          <> lbSave(1)       then Warnlog "Paper orientation => changes not saved!"
   if PaperSize.IsChecked                 <> lbSave(2)       then Warnlog "Paper size. => changes not saved!"

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)
endcase

