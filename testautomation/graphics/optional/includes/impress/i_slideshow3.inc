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
'*******************************************************************
' #1 tExtrasInteraktion
' #1 tExtrasEffekt
' #1 tExtrasPraesentationseinstellungen
' #1 tExtrasIndividuellePraesentation
' #1 tExtrasInteraktion
' #1 tSlideshowContextMenuOneSlide
' #1 tSlideshowContextMenuMoreSlides
' #1 tiMousePointerHides
'\******************************************************************

testcase tExtrasInteraktion
    Printlog " -  SlideShow/Interaction"
 Call hNewDocument
 Call sSelectEmptyLayout
 sleep 1
 Call hTextrahmenErstellen ("Seite 1",10,10,40,40)		'/// create textbox ///'
 SlideShowPresentationSettings							'/// Set slide show settings ///'
 Kontext "Bildschirmpraesentation"
 NavigatorSichtbar.Check								'/// Navigator visible ///'
 Bildschirmpraesentation.OK
 SlideShowInteraction									'/// open Interaction ///'
 Kontext "Interaktion"
 if Interaktion.exists(5)then
    AktionBeiMausklick.Select 1							'/// Select "Go to previous slide" ///'
 else
    Print "Interaction doesnt exist, something is wrong here."
 endif
 Interaktion.OK
 Kontext "DocumentImpress"
 SlideShowSlideshow								        '/// Run slideshow ///'
 Sleep (2)
 Kontext "DokumentPraesentation"
 DokumentPraesentation.TypeKeys "<ESCAPE>"
 Sleep 2
 Kontext "DocumentImpress"
 gMouseClick 20,20
 EditSelectAll
  try
     EditCopy
     Printlog "  Interaction ->No action works"
     Kontext "DocumentImpress"
  catch
     Warnlog "  - Interaction->does not work properly: Should be: Page 2  but it is: " + GetClipboardText
     DocumentImpress.TypeKeys "<ESCAPE>"
     Kontext "DocumentImpress"
  endcatch
 hCloseDocument
 Call hNewDocument
 Call sSelectEmptyLayout
 Call hRechteckErstellen (50,50,80,80)							'/// create rectangle ///'
 SlideShowInteraction
 Kontext "Interaktion"
 Dim i
 Dim Zaehler
 Zaehler=AktionBeiMausklick.GetItemCount
  For i=2 to Zaehler
      AktionBeiMausklick.Select i							'/// select actions ///'
      SetClipboard AktionBeiMausklick.GetSelText
      Interaktion.OK
      Kontext "DocumentImpress"
      EditSelectAll
      SlideShowInteraction
      Kontext "Interaktion"
        if GetClipboardText<>AktionBeiMausklick.GetSelText Then
           Warnlog "  Action at mouseclick - " + GetClipboardText + " - not taken"		'/// control if action is saved (closing reopening dialog) ///'
        else
           Printlog "  Action at mouseclick - " + GetClipboardText + " - runs"
        end if
  next i
  Interaktion.OK
   sleep 1
 Call hCloseDocument									'/// close document ///'
endcase

testcase tExtrasEffekt
    qaerrorlog "Test not yet ready."
     goto endsub
 Dim i
 Dim j
 Dim k
 Dim l
 Dim m
 Dim n
 Dim Zaehler
 Dim ZaehlerKmh
 Dim ZaehlerText
 Dim ZaehlerTon
 Dim Zufall
 Call hNewDocument									'/// New impress document ///'

   '///  check state of navigator ! expected: closed ///'
   Kontext "Navigator"
   if Navigator.exists then
      '///+ close navigator ! ///'
      Navigator.Close
      Warnlog "Navigator was open. Check earlier tests. Now closed."
   else
      printlog "Navigator: NOT available. Good."
   endif

 gMouseClick 50,50
 hRechteckErstellen (20,20,50,50)					'/// create rectangle ///'
 sleep 2
 DocumentImpress.TypeKeys "<F2>"
 DocumentImpress.TypeKeys "- This is text to test the text effects of the Effects flyer"
 sleep 1
 gMouseClick 90,90									'/// deselect rectangle ///'
 DocumentImpress.TypeKeys "<TAB>"							'/// reselect rectangle ///'

  sleep 1
  Kontext "Effekt"
    Printlog "  - Test effect flyer"

  SlideShowEffects									'/// Open effect flyer ///'
  Kontext "ExtrasEffekt"
  sleep 1
  Effekte.Click
  Zaehler=Effekteliste.GetItemCount					'/// select each effect 1 time, assign effect and close dialog ///'
    for i=1 to Zaehler								'/// reopen dialog and check if the effect is still there ///'
        Effekteliste.Select i
        SetClipboard Effekteliste.GetSelText
        Effekt.TypeKeys "<TAB>"
        Effekteauswahl.TypeKeys "<RIGHT>" ,2
        sleep 1
        Zuweisen.Click
        Effekt.Close
        SlideShowEffects
        Kontext "Effekt"
        sleep 1
         if GetClipboardText<>Effekteliste.GetSelText Then Warnlog "  - Invisible color not chosen"
            printlog GetClipboardText + " should be " + Effekteliste.GetSelText

            sleep 1
    next i
    sleep 2
  Texteffekte.Click									'/// test text effects ///'
  sleep 1
  ZaehlerKmh=Geschwindigkeit.GetItemCount
    for j=1 to ZaehlerKmh
        Geschwindigkeit.Select j
        SetClipboard Geschwindigkeit.GetSelText
        Zuweisen.Click
        Effekt.Close
        SlideShowEffects
        Kontext "Effekt"
        sleep 1
         if GetClipboardText<>Geschwindigkeit.GetSelText Then Warnlog "  - Speed not taken over"
    next j
     Printlog "  - Speed test ok"

     Printlog "  - Test text effects"
  Texteffekte.Click
  sleep 1
  ZaehlerText=TexteffekteListe.GetItemCount
    for k=1 to ZaehlerText
        TexteffekteListe.Select k
        printlog TexteffekteListe.GetSelText + "-effect choosen"
        SetClipboard TexteffekteListe.GetSelText
        Effekt.TypeKeys "<TAB>"
        Texteffektauswahl.TypeKeys "<RIGHT>" ,2
        N:    	Zuweisen.Click
        sleep 1
        Effekt.Close
        SlideShowEffects
        Kontext "Effekt"
        sleep 1
        Texteffekte.Click
        printlog "and when we closed the window and opened again... " + TexteffekteListe.GetSelText + " was choosen"
        sleep 1
         if GetClipboardText<>TexteffekteListe.GetSelText Then Warnlog "  - Texteffect did not changed"
            sleep 1
    next k
     Printlog "  - Test invisible color"
  Effekte.Click
  Effekteliste.Select 1
  Effekteauswahl.TypeKeys "<RIGHT>",2
  Zuweisen.Click
  sleep 1
  Extras.Click
  sleep 1
  randomize
  Zufall=((2*Rnd)+1)
    for l=1 to 4
        UnsichtbarMachen.Click
        sleep 1
         if Abblendfarbe.IsEnabled=True Then Warnlog "  - control shouldn't be enabled"
        Zuweisen.Click
        Effekt.Close
        SlideShowEffects
        Kontext "Effekt"
        sleep 1
        Effekteauswahl.TypeKeys "<RIGHT>",2
        Extras.Click										'/// test extras ///'
        sleep 1
        UnsichtbarMachen.Click
        sleep 1
        Zuweisen.Click
        sleep 1
    next l
     Printlog "  - Blend with color"
    sleep 2
    for m=1 to 5
        MitFarbeAbblenden.Click
        sleep 2
        Abblendfarbe.Select Zufall
        SetClipboard Abblendfarbe.GetSelText
        Zuweisen.Click
        Effekt.Close
        SlideShowEffects
        Kontext "Effekt"
        sleep 1
        Effekteauswahl.TypeKeys "<RIGHT>",3
        sleep 1
        Extras.Click
        sleep 1
        MitFarbeAbblenden.Click
        sleep 1
        Zuweisen.Click
        sleep 1
'	     if GetClipboardText<>Abblendfarbe.GetSelText Then Warnlog "  - Blendingcolor did not take over"
    next m
     Printlog "  - Test order"
  Effekte.Click
  sleep 1
  Effekteliste.Select 2
  Zuweisen.Click
  Reihenfolge.Click
  sleep 1
  sleep 1
  Printlog "  - test preview window"
  Vorschaufenster.Click									'/// open preview window ///'
  sleep 1
   Kontext "Vorschau"
      if Vorschau.Exists = False Then Warnlog "  - Preview window not opened"
   Vorschau.Close
   Kontext "Effekt"
   Extras.Click
    if not Klangliste.IsEnabled Then Klang.Click					'/// insert sound ///'
   sleep 1
'  ZaehlerTon=Klangliste.GetItemCount
   for n=1 to 5
       SetClipboard Klangliste.GetSelText
       Klangliste.Select n
       VollstaendigAbspielen.Click
       Effekt.Close
       SlideShowEffects
       Kontext "Effekt"
       sleep 1
       Extras.Click
       sleep 1
        if GetClipboardText<>Klangliste.GetSelText Then Warnlog "  - sound did not change"
    next n
   Effekt.Close
   sleep 1
 Call hCloseDocument												'/// close document ///'
endcase

testcase tExtrasPraesentationseinstellungen
dim waschecked as boolean
     Printlog "  - SlideShow/Slideshow settings"
  Call hNewDocument							'/// New impress document ///'
  setStartCurrentPage(FALSE)                '/// Set ToolsOptions - Presentation - StartCurrentPage = off ///'
  '///  check state of navigator ! expected: closed ///'
  Kontext "Navigator"
  if Navigator.exists then
     '///+ close navigator ! ///'
     Navigator.Close
     Warnlog "Navigator was open. Check earlier tests. Now closed."
  else
     printlog "Navigator: NOT available. Good."
  endif
  Kontext "DocumentImpress"
   ExtrasPraesentationseinstellung
   Kontext "Bildschirmpraesentation"
   AbDia.Check
  SetClipboard AbDiaName.GetItemCount
  Bildschirmpraesentation.OK
  InsertSlide								'/// insert slide ///'
  sleep 2
  hTypekeys "<Pagedown>"
  sleep 2
  SlideShowPresentationSettings				'/// slideshow settings ///'
  Kontext "Bildschirmpraesentation"
   if AbDiaName.GetItemCount <> GetClipboardText Then
      Printlog "  - Added page appears in list"
   else
      Warnlog "  - Page not added to the list"
   end if
  AlleDias.Check							'/// check all dias///'
      Printlog "  - Test all slides"
  Bildschirmpraesentation.OK
  hCloseDocument							'/// close document ///'
    DateiOeffnen							'/// open document (diashow.odp) ///'
  Kontext "OeffnenDlg"
  if OeffnenDlg.Exists(10) then
     Dateiname.SetText ConvertPath (gTesttoolPath + "graphics\required\input\diashow.odp")
  else
     Warnlog "Took more then 10 seconds to get the file-open -dialogue open. Ending Test."
     goto endsub
  endif
  Oeffnen.Click
  sleep (60)
  ' check if the document is writable
  if fIsDocumentWritable = false then
     ' make the document writable and check if it's succesfull
     if fMakeDocumentWritable = false then
        warnlog "The document can't be make writeable. Test stopped."
        goto endsub
     endif
  endif
  Kontext "DocumentImpress"
  sleep 1
  SlideShowPresentationSettings
  Kontext "Bildschirmpraesentation"
   if Fenster.IsChecked = False Then			'/// slideshow runs in window mode checked ///'
      Fenster.Check
      Printlog "  - Slideshow in window mode"
   else
      Printlog "  - Slideshow in window mode checked"
   end if
  Bildschirmpraesentation.OK

    Printlog "  - Testing slide show"
  SlideShowSlideshow							'/// run slideshow ///'
  sleep (3)
  Kontext "DocumentPresentation"
  DocumentPresentation.TypeKeys "<pagedown>"
   try
      Kontext "DocumentImpress"
      ViewZoom							'/// try using menue entrees (should be disabled while slideshow is running) ///'
      Warnlog "  - In slide show mode controls shouldn't be enabled"
      Kontext "Massstab"
      Massstab.OK
   catch
       Kontext "DocumentPresentation"
       Printlog "  - Slideshow runs"
   endcatch
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>"
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>"
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>"
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>"
  sleep (5)

  kontext "DocumentPresentation"
  if DocumentPresentation.Exists(3) then                                '/// test if application is still in slideshow mode ///'
     warnlog "  - We are still in slideshow mode"
     DocumentPresentation.TypeKeys "<ESCAPE>"
     else
     Printlog "  - Test Abdia page 3 ended"
  endif

  Printlog "  - From slides test"
  SlideShowPresentationSettings						'/// open slideshow settings ///'
  Kontext "Bildschirmpraesentation"
  Fenster.Check						                '/// check slideshow in window mode ///'
  AbDia.Check
  AbDiaName.Select 3							'/// slideshow begins at dia 3///'
        Printlog "  -  From " + AbDiaName.GetSelText + " was the slides shown"
  Bildschirmpraesentation.OK
  SlideShowSlideshow							'/// run slideshow ///'
  sleep (3)
  Kontext "DocumentPresentation"
  DocumentPresentation.TypeKeys "<pagedown>" 'to get to 4
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>" ' to get to the end
  sleep 2
  '/// Click once more to get out of presentation-mode ///'
  DocumentPresentation.TypeKeys "<pagedown>" 'out
  sleep (3)
  kontext "DocumentPresentation"
  if DocumentPresentation.Exists(3) then
     warnlog "  - We are still in slideshow mode"
     DocumentPresentation.TypeKeys "<ESCAPE>"
     else
     Printlog "  - Test Abdia page 3 ended"
  endif
  Printlog "  - Repeat endless"

  Kontext "DocumentImpress"
  SlideShowPresentationSettings						'/// open slideshow settings ///'
  Kontext "Bildschirmpraesentation"
   if AbDia.IsChecked=True Then AlleDias.Check
   if Auto.IsChecked=False Then Auto.Check
  Bildschirmpraesentation.OK
  sleep (1)
  SlideShowSlideshow                            'start from 1
  sleep (3)
  Kontext "DocumentPresentation"
  DocumentPresentation.TypeKeys "<pagedown>"    'to 2
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>"    'to 3
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>"    'to 4
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>"    'to pause
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>"    'to 1
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>"    'to 2
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>"    'to 3
  sleep 2
  DocumentPresentation.TypeKeys "<MOD1 SHIFT F5>"   '/// Open the navigator ///'
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>"    'to 4
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>"    'to pause
  sleep 2
  DocumentPresentation.TypeKeys "<pagedown>"    'to 1
  sleep 3
  DocumentPresentation.TypeKeys "<pagedown>"    'to 2
  sleep 3
  DocumentPresentation.TypeKeys "<pagedown>"    'to 3
  sleep 3
  Kontext "NavigatorDraw"
  if NavigatorDraw.Exists then
     Printlog "   The navigator is open. good."
  else
     Warnlog "  The navigator should be accessable. Opening now."
     Kontext "DocumentImpress"
     DocumentImpress.TypeKeys "<MOD1 SHIFT F5>"
     Kontext "NavigatorDraw"
  end if
  sleep 2
  if Liste.GetSelIndex <> 3 then
     Warnlog "  - Diashow not repeated: We should be at page no 3, but we are at page no: " + Liste.GetSelIndex
     Kontext "DocumentPresentation"
     DocumentPresentation.TypeKeys "<MOD1 SHIFT F5>"            '/// Close the Navigator-window ///'
  else
     Printlog "  - Repeat endless does work"
     Kontext "DocumentPresentation"
     DocumentPresentation.TypeKeys "<MOD1 SHIFT F5>"            '/// Close the Navigator-window ///'
     sleep 2
  endif
  Kontext "DocumentPresentation"
  DocumentPresentation.TypeKeys "<ESCAPE>"
  sleep 2
  Kontext "DocumentImpress"
  gMouseClick 80,80
   try
      SlideShowPresentationSettings					'/// test endless repeating setting ///'
      Kontext "Bildschirmpraesentation"
      Bildschirmpraesentation.OK
      Printlog "  - Repeat endless"
   catch
      Warnlog "  - We are still in slideshow mode"
      DocumentPresentation.TypeKeys "<ESCAPE>"
      sleep 5
    endcatch
   Kontext "DocumentImpress"
    SlideShowPresentationSettings
    Kontext "BildschirmPraesentation"
    Fenster.Check
       Printlog "  - Test slide switching manually"				'/// test dia switch manual ///'
    if DiawechselManuel.IsChecked = False Then DiawechselManuel.Check
    BildschirmPraesentation.Ok
    sleep 1
    Kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<MOD1 F2>"
    try
       ViewZoom
       Kontext "Massstab"
       Warnlog "  - No slide show mode with  'Ctrl+F2'"
    catch
       Printlog "  - Slide show started using 'Ctrl F2'"
    endcatch
    Kontext "DocumentPresentation"
    DocumentPresentation.TypeKeys "<pagedown>"
    sleep 1
    DocumentPresentation.TypeKeys "<MOD1 SHIFT F5>"
'     Kontext "NavigatorDraw"
     sleep 3
    DocumentPresentation.TypeKeys "<pagedown>"
    sleep 3
    DocumentPresentation.TypeKeys "<MOD1 SHIFT F5>"
    Kontext "Navigator"
    sleep 5

   '///  Check state of navigator ! Expected: closed ///'
   Kontext "Navigator"
   if Navigator.exists then
      '///+ Close Navigator ! ///'
      Navigator.Close
      Warnlog "Navigator: Should have been closed. Closing now."
   else
      printlog "Navigator: not available - Good"
   endif
  Kontext "DokumentPraesentation"
  DokumentPraesentation.TypeKeys "<ESCAPE>"  '/// Exit presentation-mode ///'
  Kontext "DocumentImpress"
  Call hCloseDocument					   '/// close document ///'
endcase

testcase tExtrasIndividuellePraesentation
    Printlog "- Slideshow/Individual slideshow"
 Call hNewDocument							'/// new impress document ///'
     '/// Deactivate "Start with current page" in ToolsOptions ///'
     setStartCurrentPage(FALSE)
   '///  check state of navigator ! expected: closed ///'
   Kontext "Navigator"
   if Navigator.exists then
      '///+ close navigator ! ///'
      Navigator.Close
      Warnlog "Navigator was open. Check earlier tests. Now closed."
   else
      printlog "Navigator: NOT available. Good."
   endif
     Printlog "  - insert 3 slides for the individual slide show"
       SlideShowPresentationSettings				'/// open slide show settings ///'
       Kontext "Bildschirmpraesentation"
       Fenster.Check
       Bildschirmpraesentation.OK
  InsertSlide                                                           '/// insert slide ///'
  sleep 2
  hTypekeys "<Pagedown>"
  sleep 2
  InsertSlide
  sleep 2
  hTypekeys "<Pagedown>"
  sleep 2
  InsertSlide								'/// insert another slide ///'
  sleep 2
  hTypekeys "<Pagedown>"
  sleep 2
  Printlog "  - Slides added"
  sleep 1
  Kontext "DocumentImpress"
  DocumentImpress.TypeKeys "<MOD1 SHIFT F5>"		'/// open navigator ///'
  sleep 1
  Kontext "NavigatorDraw"
   if NavigatorDraw.Exists Then
      Printlog "  - Navigator exists"
      Erste.Click							'/// switch to 1st slide ///'
      Kontext "DocumentImpress"
      DocumentImpress.MouseDown ( 50, 50 )  '/// Make a click onto the slide to focus onto that ///'
      DocumentImpress.MouseUp ( 50, 50 )
      hRechteckErstellen (10,10,20,20)		'/// create rectangle ///'
      Kontext "NavigatorDraw"
      Naechste.Click						'/// switch to 2nd slide ///'
      Kontext "DocumentImpress"
      hRechteckErstellen (30,30,40,40)		'/// create another rectangle ///'
      Kontext "NavigatorDraw"
      Naechste.Click						'/// switch to 3rd slide ///'
      Kontext "DocumentImpress"
      hRechteckErstellen (40,40,50,50)		'/// create rectangle ///'
      Kontext "NavigatorDraw"
      Naechste.Click						'/// switch to next slide ///'
      Kontext "DocumentImpress"
      hRechteckErstellen (50,50,60,60)		'/// create rectangle ///'
      Printlog "  - Created rectangles on all slides"
   else
      Warnlog "  No Navigator"
   end if
  sleep 1
  SlideShowCustomSlideshow					'/// open custom slideshow ///'
  Kontext "IndividuellePraesentation"
   Printlog "  - Create new slideshow"
  Neu.Click								    '/// create new individual slideshow ///'
  Kontext "IndividuellePraesentationDefinieren"
  PraesentationName.SetText "Test 1"		'/// set name of individiual slideshow to Test 1 ///'
  Dim i
  Dim Zaehler
  Zaehler=SeitenPraesentation.GetItemCount	'/// add slides to presentation ///'
     For i=1 to Zaehler
         SeitenPraesentation.SetNoSelection
         SeitenPraesentation.Select i
         Hinzufuegen.Click
          if SelectedSlides.GetItemCount=i Then
             Printlog "  - slide added to slideshow"
          else
             Warnlog "  Count does not match selection.  Should be: "+ i + " but is: "+ IndividuellePraesentation.GetItemCount
          end if
      next i
         Printlog "  - Add slides using multiple selections"
      SeitenPraesentation.Select 1
      Hinzufuegen.Click
      SeitenPraesentation.Select 2
      Hinzufuegen.Click
      SeitenPraesentation.Select 3
      Hinzufuegen.Click
      SeitenPraesentation.Select 4
      Hinzufuegen.Click

         if SelectedSlides.GetItemCount=8 Then			'/// control number of added slides ///'
            Printlog "  - slides has been added, multiple selection is working"
         else
            Warnlog "  Multiple selection does not work"
         end if

      IndividuellePraesentationDefinieren.OK
      sleep 2
  Kontext "IndividuellePraesentation"
     if IndividuellePraesentationBenutzen.IsChecked=False Then IndividuellePraesentationBenutzen.Check
     sleep 2
     Starten.Click									     '/// run presentation ///'
     Printlog "  - Individual slideshow started"
     sleep 5
  Kontext "Navigator"                               '/// Check if the navigator exists, if so - close it ///'
   if Navigator.Exists Then
      Printlog "  - Navigator exists, we close it"
      Navigator.Close								'/// close navigator ///'
      sleep 2
   end if
  Kontext "DocumentPresentation"
  sleep 3
  DocumentPresentation.TypeKeys "<RETURN>"
  sleep 2
  DocumentPresentation.TypeKeys "<RETURN>"
  sleep 2
  DocumentPresentation.TypeKeys "<RETURN>"
  sleep 2
  DocumentPresentation.TypeKeys "<RETURN>"
  sleep 2
  DocumentPresentation.TypeKeys "<RETURN>"
  sleep 2
  DocumentPresentation.TypeKeys "<RETURN>"
  sleep 2
  DocumentPresentation.TypeKeys "<RETURN>"
  sleep 2
  DocumentPresentation.TypeKeys "<RETURN>"
  sleep 2
  DocumentPresentation.TypeKeys "<ESCAPE>"
  sleep 5
  Kontext "IndividuellePraesentation"
  if IndividuellePraesentation.IsVisible=False Then
     Warnlog "  Slideshow should have ended"
     Kontext "DocumentPresentation"
     DocumentPresentation.TypeKeys "<ESCAPE>"
  else
     printlog "   Presentation seems to have ended successfully"
  end if

  sleep 2

  Kontext "IndividuellePraesentation"
  IndividuellePraesentation.Close                                '/// Close custom slideshow ///'

  Kontext "NavigatorDraw"                                           '/// Navigator: Control if right slide is displayed ///'
  printlog "  Now we switch to the navigator again"
  if Liste.GetSelIndex <> 4 then          '/// Unless -Start on first slide- is activated? Default = No ///'
     Warnlog "  This is not the right slide, it should be 4 but is: " + Liste.GetSelIndex
  else
     Printlog "  - Individual slideshow seems to work"
  end if
   SlideShowCustomSlideshow                                '/// open custom slideshow ///'
   '/// Set "Start with current page" back to default = on, in ToolsOptions ///'
   Kontext "IndividuellePraesentation"
  if IndividuellePraesentationBenutzen.IsChecked=False Then IndividuellePraesentationBenutzen.Check
  IndividuellePraesentation.Close
  sleep 2
  Kontext "Navigator" 'Draw
   if Navigator.exists then
       Navigator.Close   '/// Close the Navigator ///'
       Kontext "NavigatorDraw"
       if NavigatorDraw.exists then
           NavigatorDraw.Close
       endif
   else
      printlog "Navigator: NOT available. Good."
   endif
   Kontext "DocumentImpress"
   setStartCurrentPage(TRUE)
 Call hCloseDocument       '/// close document ///'
endcase

'****************************************************************************************************

testcase tSlideshowContextMenuOneSlide
    qaerrorlog "Test not yet ready."
     goto endsub
   dim NumberOfEntries as Integer
   Printlog "- ContextMenu in Slideshow"
   '/// New Impress Document ///'
   Call hNewDocument

      '/// Start the Slideshow. ///'
      hTypeKeys "<F5>"
      '/// Check that the right mousebutton brings up the Context-Menu. ///'
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      '/// Check that the right-click brought up a Context-Menu. ///'
      '/// And check the number of Menu-Positions (there should be XXX of them ) ///'
      NumberOfEntries = 0
      sleep 2
      try
         NumberOfEntries = MenuGetItemCount
      catch
         warnlog "   No ContextMenu found? Please inform the Automatic Tester"
         kontext "DocumentImpress"
         hOpenContextMenu
         NumberOfEntries = MenuGetItemCount
      endcatch
      if (NumberOfEntries = 0) then
         Warnlog "   the Context-Menu doesnt contain any entries, or were not up. Test ends."
         Goto Endsub
      endif
      if NumberOfEntries <> 3 then
         warnlog "   Expected three entries in this Menu, but found " + NumberOfEntries
'      if NumberOfEntries <> 6 then
'         warnlog "   Expected six entries in this Menu, but found " + NumberOfEntries
      else
         printlog "   Number of Entries was: " + NumberOfEntries
      endif
      '/// Check that every position contains the expected Undermenu. ("Screen": Black/White. and "End Slideshow") ///'
      '/// And check that Menu-Item one opens an undermenu. ///'
      Printlog "   We open number one: " + MenuGetItemText(MenuGetItemID(1))
      hMenuSelectNr (1)
      sleep 2
      NumberOfEntries = MenuGetItemCount
      printlog "   Menu-entries: " + MenuGetItemCount
      if (NumberOfEntries <> 2) then
         Warnlog "   the third Context-Menu-entry was NOT 'Screen'."
      else
         Printlog "   We open the next number one: " + MenuGetItemText(MenuGetItemID(1))
         hMenuSelectNr (1)
      endif
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      hMenuSelectNr (1) 'Open the Screen -menu.
      sleep 2
      Printlog "   We open number two: " + MenuGetItemText(MenuGetItemID(MenuGetItemCount))
      hMenuSelectNr (2) 'Choose "White"
      sleep 2

      'TODO - Due to existing bug, function not yet available.
      '/// Change to slideshow-ending. Check that the Context-Menu also comes up here. ///'
      'DocumentPresentation.
      hTypeKeys "<SPACE>"
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      sleep 2
      NumberOfEntries = 0
      NumberOfEntries = MenuGetItemCount
      if NumberOfEntries <> 0 then
         printlog "   Menu-entries: " + MenuGetItemCount
      else
         warnlog "   No context-menu at Slideshow-endpage."
         DocumentPresentation.TypeKeys "<ESCAPE>"
      endif

      '/// And that one can go back. ///'
      hMenuSelectNr (1) 'Open the Goto Slide -menu.
      sleep 2
      Printlog "   We open number one (should be 'Goto First Slide'): " + MenuGetItemText(MenuGetItemID(1))
      hMenuSelectNr (2) 'Choose "Back" 'TODO ;: but now we just end the slideshow
      sleep 2

      Kontext "DocumentPresentation"
      DocumentPresentation.TypeKeys "<ESCAPE>"
      DocumentPresentation.TypeKeys "<ESCAPE>"
      Kontext "DocumentImpress"
      InsertSlide

      '/// Start the Slideshow. ///'
      hTypeKeys "<F5>"
      '/// Check that the right mousebutton brings up the Context-Menu. ///'
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      sleep 2

      '/// Check that Menu-Item (three) really finishes the presentation. ///'
      Printlog "   We open the last entry (End Show): " + MenuGetItemText(MenuGetItemID(MenuGetItemCount))
      hMenuSelectNr (4) 'MenuGetItemCount) 'End Slideshow
      if DocumentPresentation.Exists then
         Warnlog "either wrong position for 'End Slideshow', or the command didnt work."
      else
         printlog "The presentation was closed, good."
      endif

      '/// Check that one can step one step forward, even if there is no more than one slide. ///'
      '/// Start the Slideshow. ///'
      hTypeKeys "<F5>"
      '/// Check that the right mousebutton brings up the Context-Menu. ///'
      sleep (2)
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      sleep 2

      hMenuSelectNr (2) 'Open the Goto Slide -menu.

      '/// Select the 'one step forward' -entry ///'
      hMenuSelectNr (1)

      '/// Check that we're on the last slide ///'
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      sleep 2

      hMenuSelectNr (2) 'Open the Goto Slide -menu.
      if MenuIsItemEnabled (MenugetItemID(4)) then
         printlog "Jumped to the right slide"
      else
         warnlog "possibly the 'jump to slide' -menu didnt quite work"
      endif

      '/// Close the Context-Menu ///'
      hMenuSelectNr (0)
      Kontext "DocumentPresentation"

      '/// Check if the context-menu also comes up at the very last page (slideshow-ending) ///'
      DocumentPresentation.TypeKeys "<SPACE>"
'      DocumentPresentation.TypeKeys "<SPACE>"
      sleep 1
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      sleep 2

      if MenuGetItemText (MenuGetItemID(1)) <> "" then
         Printlog "Context-menu came up at the last page: correct."
      else
         Warnlog "Context-menu did NOT come up correctly at the last page: false."
      endif

      '/// Check if we from here, via the context menu, can go back to the first page ///'
      hMenuSelectNr (2) 'Open the Goto Slide -menu.
      sleep 1
      hMenuSelectNr (1) 'First Slide

      '/// Check that we're on the first slide ///'
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      sleep 2

      hMenuSelectNr (2) 'Open the Goto Slide -menu.
      if MenuIsItemEnabled (MenugetItemID(3)) then
         printlog "Jumped to the right slide"
      else
         warnlog "possibly the 'jump to slide' -menu didnt quite work from the last slide"
      endif

      '/// Close the Context-Menu ///'
      MenuSelect (0)
      '/// Close the Presentation ///'
      hTypeKeys "<ESCAPE>"
   '/// Close Document ///'
   Call hCloseDocument
endcase 'tSlideshowContextMenuOneSlide

'****************************************************************************************************

testcase tSlideshowContextMenuMoreSlides
    qaerrorlog "Test not yet ready."
     goto endsub
   dim NumberOfEntries as Integer
   Printlog "- ContextMenu in Slideshow"
   '/// New Impress Document ///'
   Call hNewDocument
      '/// Insert three new Slides ///'
      InsertSlide
      InsertSlide
      InsertSlide
      '/// Start the Slideshow. ///'
      hTypeKeys "<F5>"
      sleep (3)
      '/// Check that the right mousebutton brings up the Context-Menu. ///'
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      '/// Check that the right-click brought up a Context-Menu. ///'
      '/// And check the number of Menu-Positions (there should be XXX of them ) ///'
      NumberOfEntries = 0
      sleep 2
      try
         NumberOfEntries = MenuGetItemCount
      catch
         warnlog "   No ContextMenu found? Please inform the Automatic Tester"
         kontext "DocumentImpress"
         hOpenContextMenu
         NumberOfEntries = MenuGetItemCount
      endcatch
      if (NumberOfEntries = 0) then
         Warnlog "   the Context-Menu doesnt contain any entries, or were not up. Test ends."
         Goto Endsub
      endif
      if NumberOfEntries <> 6 then
         warnlog "   Expected six entries in this Menu, but found " + NumberOfEntries
      else
         printlog "   Number of Entries was: " + NumberOfEntries
      endif
      '/// Check that every position contains the expected Undermenu. ("Screen": Black/White. and "End Slideshow") ///'
      '/// And check that Menu-Item one opens an undermenu. ///'
      Printlog "   We open number four: " + MenuGetItemText(MenuGetItemID(4))
      hMenuSelectNr (3)
      sleep 2
      NumberOfEntries = MenuGetItemCount
      printlog "   Menu-entries: " + MenuGetItemCount
      if (NumberOfEntries <> 2) then
         Warnlog "   the first Context-Menu-entry was NOT 'Screen'."
      else
         Printlog "   We open the next number one: " + MenuGetItemText(MenuGetItemID(1))
         hMenuSelectNr (1)
      endif
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      hMenuSelectNr (3) 'Open the Screen -menu.
      sleep 2
      Printlog "   We open number two: " + MenuGetItemText(MenuGetItemID(MenuGetItemCount))
      hMenuSelectNr (2) 'Choose "White"
      sleep 2

      'TODO - Due to existing bug, function not yet available.
      '/// Change to slideshow-ending. Check that the Context-Menu also comes up here. ///'
      'DocumentPresentation.
      hTypeKeys "<SPACE>"
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      sleep 2
      NumberOfEntries = 0
      NumberOfEntries = MenuGetItemCount
      if NumberOfEntries <> 0 then
         printlog "   Menu-entries: " + MenuGetItemCount
      else
         warnlog "   No context-menu at Slideshow-endpage."
         DocumentPresentation.TypeKeys "<ESCAPE>"
      endif

      '/// And that one can go back. ///'
      hMenuSelectNr (2) 'Open the Goto Slide -menu.
      sleep 2
      Printlog "   We open number one (should be 'Goto First Slide'): " + MenuGetItemText(MenuGetItemID(1))
      hMenuSelectNr (2) 'Choose "Back" 'TODO ;: but now we just end the slideshow
      sleep 2

      Kontext "DocumentPresentation"
      DocumentPresentation.TypeKeys "<ESCAPE>"
      DocumentPresentation.TypeKeys "<ESCAPE>"
      Kontext "DocumentImpress"

      '/// Start the Slideshow. ///'
      hTypeKeys "<F5>"
      '/// Check that the right mousebutton brings up the Context-Menu. ///'
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      sleep 2

      '/// Check that Menu-Item (three) really finishes the presentation. ///'
      Printlog "   We open the last entry (End Show): " + MenuGetItemText(MenuGetItemID(MenuGetItemCount))
      hMenuSelectNr (4)'MenuGetItemCount) 'End Slideshow
      sleep 4
      if DocumentPresentation.Exists then
         Warnlog "either wrong position for 'End Slideshow', or the command didnt work."
         Kontext "DocumentPresentation"
         DocumentPresentation.MouseDown 50, 50, 3
         DocumentPresentation.MouseUp 50, 50, 3
         sleep 2
         Printlog "   We open the last entry (End Show): " + MenuGetItemText(MenuGetItemID(MenuGetItemCount))
         hMenuSelectNr (4)'MenuGetItemCount) 'End Slideshow
      else
         printlog "The presentation was closed, good."
      endif

      '/// Check that one can step one step forward, even if there is no more than one slide. ///'
      '/// Start the Slideshow. ///'
      hTypeKeys "<F5>"
      sleep (3)
      '/// Check that the right mousebutton brings up the Context-Menu. ///'
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      sleep 2

      hMenuSelectNr (2) 'Open the Goto Slide -menu.

      '/// Select the 'one step forward' -entry ///'
      hMenuSelectNr (1)

      '/// Check that we're on the last slide ///'
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      sleep 2

      hMenuSelectNr (2) 'Open the Goto Slide -menu.
      if MenuIsItemEnabled (MenugetItemID(4)) then
         printlog "Jumped to the right slide"
      else
         warnlog "possibly the 'jump to slide' -menu didnt quite work"
      endif

      '/// Close the Context-Menu ///'
      hMenuSelectNr (0)
      Kontext "DocumentPresentation"

      '/// Check if the context-menu also comes up at the very last page (slideshow-ending) ///'
      DocumentPresentation.TypeKeys "<SPACE>"
      sleep 1
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      sleep 2

      if MenuGetItemText (MenuGetItemID(1)) <> "" then
         Printlog "Context-menu came up at the last page: correct."
      else
         Warnlog "Context-menu did NOT come up correctly at the last page: false."
      endif

      '/// Check if we from here, via the context menu, can go back to the first page ///'
      hMenuSelectNr (2) 'Open the Goto Slide -menu.
      sleep 1
      hMenuSelectNr (1) 'First Slide

      '/// Check that we're on the first slide ///'
      Kontext "DocumentPresentation"
      DocumentPresentation.MouseDown 50, 50, 3
      DocumentPresentation.MouseUp 50, 50, 3
      sleep 2

      hMenuSelectNr (2) 'Open the Goto Slide -menu.
      if MenuIsItemEnabled (MenugetItemID(3)) then
         printlog "Jumped to the right slide"
      else
         warnlog "possibly the 'jump to slide' -menu didnt quite work from the last slide"
      endif

      '/// Close the Context-Menu ///'
      MenuSelect (0)
      '/// Close the Presentation ///'
      hTypeKeys "<ESCAPE>"
   '/// Close Document ///'
   Call hCloseDocument
endcase 'tSlideshowContextMenuMoreSlides

'****************************************************************************************************

testcase tiMousePointerHides
    qaerrorlog "Test not yet ready."
     goto endsub
   dim i as Integer
   Printlog "- ContextMenu in Slideshow"
   '/// New Impress Document ///'
   Call hNewDocument

      '/// Start the Slideshow. ///'
      hTypeKeys "<F5>"
      sleep 1
      i = 0
      while ((getMouseStyle = 0) AND (i<20))
          sleep 1
          inc (i)
          printlog getMouseStyle
          if (getMouseStyle <> 0) then i = 20
      wend
      if (getMouseStyle <> 0) then
         printlog "Mousepointer disappeared like it should have"
      else
         warnlog "the mousepointer was still visible, after 20 seconds."
      endif

      hTypeKeys "<ESCAPE>"
      hTypeKeys "<ESCAPE>"

   '/// Close Document ///'
   Call hCloseDocument
endcase 'tSlideshowContextMenuMoreSlides

'****************************************************************************************************

