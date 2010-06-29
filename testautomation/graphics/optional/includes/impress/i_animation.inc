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
'* Owner : wolfram.garten@sun.com
'*
'* short description :
'*
'*********************************************************************
' #1 tExtrasAnimation
'\********************************************************************

'    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
' Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\grafix3.ras")

testcase tExtrasAnimation
    Printlog "- Tools/Animation"
 Dim i
 Call hNewDocument
    Call sSelectEmptyLayout									'/// New impress document ///'
 sleep 2
 InsertGraphicsFromFile									'/// insert graphic file (sample.bmp) ///'
 sleep 1
 Kontext "GrafikEinfuegenDlg"
 sleep 2

    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
    sleep 2
    Oeffnen.Click
    sleep 2
 Kontext "DocumentImpress"
 DocumentImpress.MouseDoubleClick 90,90							'/// Deselect graphic ///'
 sleep 1
 InsertGraphicsFromFile
 sleep 2
 Kontext "GrafikEinfuegenDlg"
 Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\grafix3.ras")
 sleep 2
 Oeffnen.Click
 sleep 2
 Opl_SD_EffekteZulassen
 Kontext "Animation"
 
 for i=1 to 10
     BildAufnehmen.Click								'/// add selected picture 10 times into the animation ///'
 next i
 sleep 1
  if AnzahlBilder.GetText <> "10" Then 							'/// compare frame number in animation dialog ///'
     WarnLog "  - Adding graphics did not work"
  else
     PrintLog "  Pictures added"
  end if

 if AlleAufnehmen.IsEnabled Then WarnLog "  - Add all should not be enabled, there is only 1 graphic selected"	'/// test if Add all is eneabled (shouldn't be because of only 1 object selected) ///'
 Kontext "DocumentImpress"
 EditSelectAll							'/// Select both pictures ///'

 Kontext "Animation"

 for i=1 to 5
     AlleAufnehmen.Click								'/// Add all (5 times) ///'
 next i
 sleep 1
  if AnzahlBilder.GetText <> "20" Then 							'/// check if number of frames is now 20 ///'
     WarnLog "  - Adding pics did not work"
  else
     PrintLog "  All pics added"
  end if

 ErstesBild.Click									'/// Click First Picture  ///'

  if AnzahlBilder.GetText <> "1" Then 							'/// Control if we are at frame 1 ///'
     WarnLog "  - Jump back to start did not work"
  else
     PrintLog "  Jumped back to first picture"
  end if

 LetztesBild.Click									'/// Go to last picture ///'

  if AnzahlBilder.GetText <> "20" Then 							'/// Control frame number ///'
     WarnLog "  - Jump to end did not work"
  else
     PrintLog "  Jump to end did work"
  end if

 Abspielen.Click									'/// Play animation ///'
 sleep 3
 Kontext "DocumentImpress"
 EditSelectAll										'/// Select all in document ///'
 DocumentImpress.TypeKeys "<DELETE>"							'/// Delete content ///'

 Kontext "Animation"
 Erstellen.Click									'/// /Create animation //'
 sleep 10

 Kontext "DocumentImpress"
  try
     EditCopy										'/// Try to copy ccreated animation into clipboard ///'
     PrintLog "  animation created"
  catch
     WarnLog "  - Animation not created"
  endcatch

 Kontext "Animation"

  if AnimationsgruppeGruppenobjekt.IsChecked=True Then
     try
        AnzeigedauerProBild.SetText "1"						'/// Try setting duration per frame ///'
        WarnLog "  - Edit field should be disabled"
     catch
        PrintLog "  Edit field not enabled because groupobject status is = " + AnimationsgruppeGruppenobjekt.IsChecked
     endcatch
  end if


 AnimationsgruppeBitmapobjekt.Check							'/// check animation group object ///'

  AnzeigedauerProBild.SetText "3"							'/// Set duration per frame to 3///'
  SetClipboard AnzeigedauerProBild.GetText
  AnzeigedauerProBild.More 								'/// Raise value for duration ///'
   if AnzeigedauerProBild.GetText <> GetClipboardText Then
      PrintLog "  Time per pic could be edited"
   else
      WarnLog "  - Time per pic could be edited"
   end if

  Dim Zaehler
  Zaehler = AnzahlDurchlaeufe.GetItemCount
   for i=1 to Zaehler									'/// Change number of plays ///'
       AnzahlDurchlaeufe.Select i
   next i
     PrintLog "  Number of loops checked"


  Dim ZaehlerAnpassung
  ZaehlerAnpassung=Anpassung.GetItemCount						'/// change ZaehlerAnpassung ///'
   for i=1 to ZaehlerAnpassung
       Anpassung.Select i
   next i
    PrintLog "  Loop count changed"

 BildLoeschen.Click									'/// Delete 1 frame ///'
  if AnzahlBilder = "20" Then
     WarnLog "  - Delete pictures from animation did not work"
  else
     PrintLog "  Picture No20 deleted"
  end if

 AlleLoeschen.Click									'/// Delete all frames ///'
 Kontext "Active"
 Active.Yes
 sleep 3

 Kontext "Animation"
  if Abspielen.IsEnabled=true Then
     WarnLog "  - Not all pics could be deleted"
  else
     PrintLog "  all pics deleted"
  end if
 sleep 2
 Animation.Close
 sleep 2
 Call hCloseDocument									'/// close document///'
endcase
