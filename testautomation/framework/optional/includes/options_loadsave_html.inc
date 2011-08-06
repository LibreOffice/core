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
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Test the Load/Save HTML compatibility page
'*
'\******************************************************************************

testcase tLoadSaveHTML

   Dim lbSave ( 10 ) as Boolean
   Dim lsSave2 ( 8 ) as String
   Dim lsSave ( 10 ) as String
   Dim i as Integer


   hNewDocument
   ToolsOptions
   hToolsOptions ( "LoadSave", "HTMLCompatibility" )

 printlog " - save old settings"
   lsSave ( 1 ) = Groesse1.GetText
   lsSave ( 2 ) = Groesse2.GetText
   lsSave ( 3 ) = Groesse3.GetText
   lsSave ( 4 ) = Groesse4.GetText
   lsSave ( 5 ) = Groesse5.GetText
   lsSave ( 6 ) = Groesse6.GetText
   lsSave ( 7 ) = Groesse7.GetText
   lbSave ( 1 ) = UnbekannteHTML.IsChecked
   lbSave ( 2 ) = FontEinstellungen.IsChecked
   lbSave ( 3 ) = StarBasic.IsChecked
   lbSave ( 4 ) = WarnungAnzeigen.IsChecked
   lbSave ( 5 ) = Drucklayout.IsChecked
   lbSave ( 6 ) = Grafikenkopieren.IsChecked
   lsSave ( 8 ) = Export.GetSelText
   lsSave ( 9 ) = Zeichensatz.GetSelText
   lbSave ( 7 ) = UseEnglishlocaleForNumbers.isChecked

 printlog " - change/invert settings"
   Groesse1.More 1 : lsSave2 (1) = Groesse1.GetText
   Groesse2.More 2 : lsSave2 (2) = Groesse2.GetText
   Groesse3.More 3 : lsSave2 (3) = Groesse3.GetText
   Groesse4.More 4 : lsSave2 (4) = Groesse4.GetText
   Groesse5.More 5 : lsSave2 (5) = Groesse5.GetText
   Groesse6.More 6 : lsSave2 (6) = Groesse6.GetText
   Groesse7.More 7 : lsSave2 (7) = Groesse7.GetText
   if lbSave ( 1 ) = TRUE then UnbekannteHTML.Uncheck else UnbekannteHTML.Check
   if lbSave ( 2 ) = TRUE then FontEinstellungen.Uncheck else FontEinstellungen.Check
   if FontEinstellungen.IsChecked <> TRUE then
      if lbSave ( 3 ) = TRUE then StarBasic.Uncheck else StarBasic.Check
   end if
   if lbSave ( 4 ) = TRUE then WarnungAnzeigen.Uncheck else WarnungAnzeigen.Check
   if lbSave ( 5 ) = TRUE then Drucklayout.Uncheck else Drucklayout.Check
   if lbSave ( 6 ) = TRUE then Grafikenkopieren.Uncheck else Grafikenkopieren.Check
   if lbSave ( 7 ) = TRUE then UseEnglishlocaleForNumbers.Uncheck else UseEnglishlocaleForNumbers.Check
   Export.Select 1
   Zeichensatz.Select 10

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   hCloseDocument

 printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

 printlog " - check inverting / changes"
   ToolsOptions
   hToolsOptions ( "LoadSave", "HTMLCompatibility" )

   if Groesse1.GetText <> lsSave2 (1) then Warnlog "Size 1 => changes  not saved!"
   if Groesse2.GetText <> lsSave2 (2) then Warnlog "Size 2 => changes  not saved!"
   if Groesse3.GetText <> lsSave2 (3) then Warnlog "Size 3 => changes  not saved!"
   if Groesse4.GetText <> lsSave2 (4) then Warnlog "Size 4 => changes  not saved!"
   if Groesse5.GetText <> lsSave2 (5) then Warnlog "Size 5 => changes  not saved!"
   if Groesse6.GetText <> lsSave2 (6) then Warnlog "Size 6 => changes  not saved!"
   if Groesse7.GetText <> lsSave2 (7) then Warnlog "Size 7 => changes  not saved!"
   if UnbekannteHTML.IsChecked    = lbSave ( 1 ) then Warnlog "Unknown HTML => changes not saved!"
   if FontEinstellungen.IsChecked = lbSave ( 2 ) then Warnlog "Font settings => changes not saved!"
   if FontEinstellungen.IsChecked <> TRUE then
      if StarBasic.IsChecked      = lbSave ( 3 ) then Warnlog "StarBasic => changes not saved!"
   end if
   if WarnungAnzeigen.IsChecked   = lbSave ( 4 ) then Warnlog "Show Warnings => changes not saved!"
   if Drucklayout.IsEnabled then
      if Drucklayout.IsChecked    = lbSave ( 5 ) then Warnlog "Printlayout => changes not saved!"
   end if
   if Grafikenkopieren.IsChecked  = lbSave ( 6 ) then Warnlog "Copy graphics => changes not saved!"
   if UseEnglishlocaleForNumbers.IsChecked  = lbSave ( 7 ) then Warnlog "UseEnglishlocaleForNumbers => changes not saved!"
   if Export.GetSelIndex      <> 1  then Warnlog "Export => changes not saved!"
   if Zeichensatz.GetSelIndex <> 10 then Warnlog "Font => changes not saved!"

 printlog " - 2. changes"
   Groesse1.SetText "6"
   Groesse2.SetText "9"
   Groesse3.SetText "10"
   Groesse4.SetText "11"
   Groesse5.SetText "20"
   Groesse6.SetText "38"
   Groesse7.SetText "50"
   UnbekannteHTML.Uncheck
   FontEinstellungen.Check
   Export.Select 2
   StarBasic.UnCheck
   WarnungAnzeigen.UnCheck
   Drucklayout.Check
   Grafikenkopieren.Check
   UseEnglishlocaleForNumbers.Check
   Zeichensatz.Select 2

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check 2. changes"
   ToolsOptions
   hToolsOptions ( "LoadSave", "HTMLCompatibility" )

   if Groesse1.GetText <> "6"  then Warnlog "Size 1 => changes not saved!"
   if Groesse2.GetText <> "9"  then Warnlog "Size 2 => changes not saved!"
   if Groesse3.GetText <> "10" then Warnlog "Size 3 => changes not saved!"
   if Groesse4.GetText <> "11" then Warnlog "Size 4 => changes not saved!"
   if Groesse5.GetText <> "20" then Warnlog "Size 5 => changes not saved!"
   if Groesse6.GetText <> "38" then Warnlog "Size 6 => changes not saved!"
   if Groesse7.GetText <> "50" then Warnlog "Size 7 => changes not saved!"
   if UnbekannteHTML.IsChecked    <> FALSE  then Warnlog "Unknown HTML => changes not saved!"
   if FontEinstellungen.IsChecked <> TRUE   then Warnlog "Font settings => changes not saved!"
   if StarBasic.IsChecked <> FALSE then
      Warnlog "StarBasic => changes not saved!"
   else
      if WarnungAnzeigen.IsChecked <> FALSE   then Warnlog "Show Warnings => changes not saved!"
   end if
   if Drucklayout.IsEnabled then
      if Drucklayout.IsChecked     <> TRUE   then Warnlog "Printlayout => changes not saved!"
   end if
   if Grafikenkopieren.IsChecked   <> TRUE   then Warnlog "Copy graphics => changes not saved!"
   if UseEnglishlocaleForNumbers.IsChecked   <> TRUE   then Warnlog "UseEnglishlocaleForNumbers => changes not saved!"
   if Export.GetSelIndex      <> 2 then Warnlog "Export => changes not saved!"
   if Zeichensatz.GetSelIndex <> 2 then Warnlog "Font => changes not saved!"

 printlog " - reset to saved settings"
   Groesse1.SetText lsSave (1)
   Groesse2.SetText lsSave (2)
   Groesse3.SetText lsSave (3)
   Groesse4.SetText lsSave (4)
   Groesse5.SetText lsSave (5)
   Groesse6.SetText lsSave (6)
   Groesse7.SetText lsSave (7)
   if lbSave(1) = TRUE then UnbekannteHTML.Check else UnbekannteHTML.UnCheck
   if lbSave(2) = TRUE then FontEinstellungen.Check else FontEinstellungen.UnCheck
   StarBasic.Uncheck
   if lbSave(4) = TRUE then WarnungAnzeigen.Check else WarnungAnzeigen.UnCheck

   if lbSave(3) = TRUE then StarBasic.Check else StarBasic.UnCheck
   Export.Select      lsSave ( 8 )
   if lbSave(5) = TRUE then Drucklayout.Check else Drucklayout.UnCheck
   if lbSave(6) = TRUE then Grafikenkopieren.Check else Grafikenkopieren.UnCheck
   if lbSave(7) = TRUE then UseEnglishlocaleForNumbers.Check else UseEnglishlocaleForNumbers.UnCheck
   Export.Select      lsSave ( 8 )
   Zeichensatz.Select lsSave ( 9 )

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check default settings"
   ToolsOptions
   hToolsOptions ( "LoadSave", "HTMLCompatibility" )

   if Groesse1.GetText <> lsSave (1) then Warnlog "Size 1 => changes not saved!"
   if Groesse2.GetText <> lsSave (2) then Warnlog "Size 2 => changes not saved!"
   if Groesse3.GetText <> lsSave (3) then Warnlog "Size 3 => changes not saved!"
   if Groesse4.GetText <> lsSave (4) then Warnlog "Size 4 => changes not saved!"
   if Groesse5.GetText <> lsSave (5) then Warnlog "Size 5 => changes not saved!"
   if Groesse6.GetText <> lsSave (6) then Warnlog "Size 6 => changes not saved!"
   if Groesse7.GetText <> lsSave (7) then Warnlog "Size 7 => changes not saved!"
   if UnbekannteHTML.IsChecked    <> lbSave(1) then Warnlog "Unknown HTML => changes not saved!"
   if FontEinstellungen.IsChecked <> lbSave(2) then Warnlog "Font settings => changes not saved!"
   if StarBasic.IsChecked         <> lbSave(3) then Warnlog "StarBasic => changes not saved!"
   StarBasic.Uncheck
   if WarnungAnzeigen.IsChecked   <> lbSave(4) then Warnlog "Show Warnings => changes not saved!"

   if lbSave(3) = TRUE then StarBasic.Check else StarBasic.UnCheck
   if Drucklayout.IsEnabled then
      if Drucklayout.IsChecked       <> lbSave(5) then Warnlog "Printlayout => changes not saved!"
   end if
   if Grafikenkopieren.IsChecked  <> lbSave(6) then Warnlog "Copy graphics => changes not saved!"
   if UseEnglishlocaleForNumbers.IsChecked  <> lbSave(7) then Warnlog "UseEnglishlocaleForNumbers => changes not saved!"
   if Export.GetSeltext      <> lsSave (8) then Warnlog "Export => changes not saved!"
   if Zeichensatz.GetSeltext <> lsSave (9) then Warnlog "Font => changes not saved!"

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

endcase
