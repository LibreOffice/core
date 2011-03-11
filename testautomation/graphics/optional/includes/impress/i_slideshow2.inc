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
'* Owner : wolfram.garten@oracle.com
'*
'* short description :
'*
'*******************************************************************

testcase tPraesentationAnimation

    Dim i

    Printlog "- Slideshow/Animation"
    printlog "new document "
    Call hNewDocument
    Call sSelectEmptyLayout
    sleep 2
    printlog "insert graphic"
    InsertGraphicsFromFile
    Kontext "OeffnenDlg"
    Dateiname.SetText ConvertPath (gTesttoolPath + "global/input/graf_inp/desp.bmp")
    sleep 2
    Oeffnen.Click
    sleep 2

    printlog "deselect graphic"
  gMouseClick 90,90									

  printlog "insert 2nd graphic"
  InsertGraphicsFromFile								
  Kontext "OeffnenDlg"
  Dateiname.SetText ConvertPath (gTesttoolPath + "global/input/graf_inp/borabora.jpg")
  sleep 2
  Oeffnen.Click
  sleep 2

  Call hTypekeys "<TAB>"

    printlog "open animation dialogue"
  Opl_SD_EffekteZulassen						                
  Kontext "Animation"

    for i=1 to 10
        printlog "add graphic 10 times"
        BildAufnehmen.Click								
    next i

    sleep 1
    printlog "get number of frames (should be 10) "
     if AnzahlBilder.GetText <> "10" Then						
        WarnLog "  - Adding pictures did not work"
     else
        PrintLog "  Pictures added"
     end if

     if AlleAufnehmen.IsEnabled Then WarnLog "  - Add all should not be enabled because only 1 object is selected"

    Kontext "DocumentImpress"
    printlog "select both graphics in document "
    EditSelectAll

    Kontext "Animation"

    printlog "add them 5 times (10 frames)"
    for i=1 to 5
        printlog i
        AlleAufnehmen.Click								
    next i

    sleep 1
    printlog "compare total numbers of frame"
     if AnzahlBilder.GetText <> "20" Then						
        WarnLog "  - Adding pics does not work"
     else
        PrintLog "  all pics added"
     end if

     printlog "jump to 1st frame"
  ErstesBild.Click									

    printlog "get number index of selected frame"
     if AnzahlBilder.GetText <> "1" Then						
        WarnLog "  - Jump back to beginning does not work"
     else
        PrintLog "  Jump back to beginning works"
     end if

    printlog "jump to last frame"
    LetztesBild.Click

    printlog "control number index of selected frame"
    if AnzahlBilder.GetText <> "20" Then
        WarnLog "  - Jump to end does not work"
     else
        PrintLog "  Jumped to end"
     end if
   sleep 1
   printlog "play animation"
  Abspielen.Click									
    sleep 10
  Kontext "DocumentImpress"
  printlog "delete graphics in document "
  EditSelectAll							
  DocumentImpress.TypeKeys "<DELETE>"
  sleep 2
  Kontext "Animation"
  printlog "create animation"
  Erstellen.Click									
  sleep 10

  Kontext "DocumentImpress"
    try
        printlog "try to copy created animation to make sure it is created"
       EditCopy										
       PrintLog "  Animation created"
    catch
       WarnLog "  - Animation not created"
    endcatch

  Kontext "Animation"

  printlog "set duration time of frames"
     if AnimationsgruppeGruppenobjekt.IsChecked = True Then				
        try
            AnzeigedauerProBild.SetText "1"
            WarnLog "  - Control should be disabled"
        catch
            PrintLog "  Control disabled because Groupobjekt = " + AnimationsgruppeGruppenobjekt.IsChecked
        endcatch
     end if

     printlog "check group object"
  AnimationsgruppeBitmapobjekt.Check							

  printlog "set frame visibility time to 3"
  AnzeigedauerProBild.SetText "3"							
  SetClipboard AnzeigedauerProBild.GetText
  printlog "raise value for frame time"
  AnzeigedauerProBild.More								
   if AnzeigedauerProBild.GetText<> GetClipboardText Then
      PrintLog "  Duration per frame works"
   else
      WarnLog "  - Duration could not be changed"
   end if

  Dim Zaehler
  printlog "test different settings for pass amount"
  Zaehler = AnzahlDurchlaeufe.GetItemCount						
   for i=1 to Zaehler
       AnzahlDurchlaeufe.Select i
   next i
  PrintLog "  Number of loops is set"

  Dim ZaehlerAnpassung
  ZaehlerAnpassung=Anpassung.GetItemCount
   for i=1 to ZaehlerAnpassung
       Anpassung.Select i
   next i

   printlog "delete 1 frame"
 BildLoeschen.Click									
  if AnzahlBilder = "20" Then
     WarnLog "  - Deleting pictures does not work"
  else
     PrintLog "  Picture No.20 deleted"
  end if

  printlog "delete all frames"
 AlleLoeschen.Click									
 Kontext "Active"
 Active.Yes
 sleep 3

 Kontext "Animation"
 printlog "check if play button is still active"
  if Abspielen.IsEnabled = true Then							
    WarnLog "  - Not all pics are deleted"
  else
    PrintLog "  All pics deleted"
  end if
 sleep 2
 printlog "close animation flyer"
 Animation.Close									
 sleep 2
 printlog "close document "
 Call hCloseDocument									
endcase 'tPraesentationAnimation

'---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tPraesentationEffekt

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

 Printlog "- Slideshow/Effect"
 printlog "new document "
 Call hNewDocument									
  printlog " check state of navigator ! expected: closed"
   Kontext "Navigator"
   if Navigator.exists then
      printlog " close navigator !"
      Navigator.Close
      Warnlog "Navigator was open. Check earlier tests. Now closed."
   else
      printlog "Navigator: NOT available. Good."
   endif
 sleep 2
 kontext "DocumentImpress"
 gMouseClick 50,50
 printlog "create rectangle"
 hRechteckErstellen (20,20,50,50)							
  sleep 3
  Kontext "Effekt"
    Printlog "  - Test effects"
    printlog "deselect rectangle"
  gMouseClick 90,90									
  sleep 1
  printlog "select rectangle "
  DocumentImpress.TypeKeys "<TAB>"							
  sleep 1
  printlog "go into text edit mode (F2)"
  DocumentImpress.TypeKeys "<F2>"							
  sleep 1
  printlog "type text into the rectangle"
  DocumentImpress.TypeKeys "test text to test text effects"				
  sleep 1
  DocumentImpress.TypeKeys "<ESCAPE>"
  sleep 2
  DocumentImpress.TypeKeys "<TAB>"
  sleep 1
  SlideShowEffects
  Kontext "Effekt"
  sleep 3
  Effekte.Click
  Zaehler=Effekteliste.GetItemCount
    printlog "Start from the second entry since the first one is No effect"
    for i=2 to Zaehler                              
    Effekteliste.Select i
    SetClipboard Effekteliste.GetSelText
    Effekt.TypeKeys "<TAB>"
    Effekteauswahl.TypeKeys "<RIGHT>" ,2
    sleep 1
    Zuweisen.Click
    sleep 1
    Effekt.Close
    sleep 1
    PraesentationEffekt
    Kontext "Effekt"
    sleep 1
     if GetClipboardText<>Effekteliste.GetSelText Then Warnlog "  - Invisible color not changed"
    sleep 1
    next i
        sleep 1
    Printlog "  - Test speed setting"
        sleep 1
  Texteffekte.Click

  sleep 1
  ZaehlerKmh=Geschwindigkeit.GetItemCount
    for j=1 to ZaehlerKmh
    Geschwindigkeit.Select j
    SetClipboard Geschwindigkeit.GetSelText
    Zuweisen.Click
    Effekt.Close
    sleep 1
    SlideShowEffects
    Kontext "Effekt"
    sleep 1
     if GetClipboardText <> Geschwindigkeit.GetSelText Then Warnlog "  - Speed setting did not change"
    next j
    Printlog "  - Test text effects"
  Texteffekte.Click
  sleep 1
  ZaehlerText=TexteffekteListe.GetItemCount
    for k=1 to ZaehlerText
    TexteffekteListe.Select k
    SetClipboard TexteffekteListe.GetSelText
        Effekt.TypeKeys "<TAB>"
    Texteffektauswahl.TypeKeys "<RIGHT>" ,2
    Zuweisen.Click
    sleep 1
    Effekt.Close
    gMouseClick 90,90
    sleep 2
    gmouseClick 40,40
    PraesentationEffekt
    Kontext "Effekt"
    sleep 1
    Texteffekte.Click
    sleep 1
    next k
    Printlog "  - Test invisible color"
  Effekte.Click
  Effekteliste.Select 1
  Effekteauswahl.TypeKeys "<RIGHT>" ,2
  Zuweisen.Click
  sleep 2
   try
      Extras.Click
   catch
      Warnlog "- Extras Button not active"
   endcatch
  sleep 1
  randomize
  Zufall=((2*Rnd)+1)
    for l=1 to 5
        if (NOT Abblendfarbe.IsEnabled) then
           MitFarbeAbblenden.Click
        endif
        sleep 1
         if Abblendfarbe.IsEnabled = False Then Warnlog "  - Control should not be enabled"
             sleep 1
        Zuweisen.Click
            Effekt.Close
        SlideShowEffects
        Kontext "Effekt"
        sleep 1
        Effekte.Click
        Effekteliste.Select 1
        Effekteauswahl.TypeKeys "<RIGHT>" ,3
        try
            Zuweisen.Click
        catch
            Effekteauswahl.TypeKeys "<RIGHT>"
            Zuweisen.Click
        endcatch
        sleep 2
        Extras.Click
    next l
    Printlog "  - Test blend with color"
  MitFarbeAbblenden.Click
    try
       for m=1 to 5
           sleep 2
           Abblendfarbe.Select Zufall
           SetClipboard Abblendfarbe.GetSelText
           Zuweisen.Click
       next m
    catch
       MitFarbeAbblenden.Click
      for m=1 to 5
           sleep 2
           Abblendfarbe.Select Zufall
           SetClipboard Abblendfarbe.GetSelText
           Zuweisen.Click
       next m
    endcatch
    Printlog "  - Test order"
  Effekte.Click
  Effekteliste.Select 2
  Zuweisen.Click
  Reihenfolge.Click
  sleep 1
    Printlog "  - Test preview"
  Vorschaufenster.Click
  sleep 1
   Kontext "Vorschau"
    if Vorschau.Exists = False Then Warnlog "  - Preview window does not exist"
    Vorschau.Close
  Kontext "Effekt"
  Effekte.Click
  UseBindings
  sleep 1
  Extras.Click
  sleep 1
  if NOT klang.Exists then
     warnlog "- Control Sound does not exist"
  end if
  if Klangliste.Exists = false Then Klang.Click
  sleep 1
  ZaehlerTon = Klangliste.GetItemCount
    for n=1 to ZaehlerTon

    Klangliste.Select n
        SetClipboard Klangliste.GetSelText
    VollstaendigAbspielen.Click
    Effekt.Close
    sleep 1
    Kontext "DocumentImpress"
    gMouseClick 40,40
    sleep 1
    PraesentationEffekt
    Kontext "Effekt"
    sleep 1
    Effekte.Click
    sleep 1
    Extras.Click
     if Klangliste.IsVisible = False Then Klang.Click
    next n
  Effekt.Close
 sleep 1
 Call hCloseDocument
endcase 'tPraesentationEffekt

'---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tSlideShowInteraction
   Dim Datei$
   Dim i as integer
   Dim Zaehler as integer
   dim b115364 as boolean

   Datei$ = ConvertPath (gOfficePath + "user\work\interac.odp")

   printlog "open application"
   Call hNewDocument
   Call sSelectEmptyLayout
   sleep 5
   printlog "Set start with current page to ON"
   setStartCurrentPage(TRUE)   

   printlog "call 'Insert->Slide' three times and name the slides 2, 3, 4 and create a rectangle on it "
   printlog "we now have 4 slides"
   Kontext "DocumentImpress"
   for i = 2 to 4
      InsertSlide
      sleep 2
      hTypekeys "<Pagedown>" , 10 ' workaround for issue i56267. Has to be removed when cws impress73 has be integrated
      sleep 2
      Kontext "DocumentImpress"
      gMouseClick 50,50
      Call hRechteckErstellen (i*10,i*10,i*20,i*20)
      sleep 2
   next i

   printlog " check state of navigator ! expected: closed -> open navigator"
   Kontext "Navigator"
   if Navigator.exists then
      qaerrorlog "Navigator: already open :-("
   else
      printlog "Navigator: NOT available :-) will be opened now."
      Kontext "DocumentImpress"
      DocumentImpress.TypeKeys "<MOD1 SHIFT F5>"
'      ViewNavigator
   endif
   sleep 3

   printlog "Slide Show->Interaction"
   SlideShowInteraction
      Kontext "TabInteraktion"
      sleep 1
      if AktionBeiMausklick.GetItemCount = 11 Then
         Printlog "- List is complete"
      else
         Warnlog "- Number of possible actions is wrong. It should be: 11, but it is: " + AktionBeiMausklick.GetItemCount
      end if
      sleep 1

      AktionBeiMausklick.Select 1                       'Keine Aktion bei Mausclick
         Printlog (AktionBeiMausklick.GetSelText + " chosen")
         If Durchsuchen.IsVisible then Warnlog "- Control should be invisible because AktionBeiMausklick = " + AktionBeiMausKlick.GetSelText
         sleep 1

      Kontext "TabInteraktion"
      AktionBeiMausklick.Select 2                       'Go to previous Slide
         Printlog (AktionBeiMausklick.GetSelText + " chosen")
         if Durchsuchen.IsVisible Then Warnlog "- Control should be invisible, beacause Action = " + AktionBeiMausKlick.GetSelText
      TabInteraktion.OK
      sleep 2
   Kontext "DocumentImpress"
   gMouseClick 90,90
   sleep 1
   gMouseClick 50,50
    sleep 2

   Kontext "NavigatorDraw"
       sleep 2
      if Not Liste.GetSelIndex = 3 Then
         Warnlog "- jumped to wrong slide"
      else
         Printlog "- jumped to correct slide"
      end if
      sleep 1

   Kontext "DocumentImpress"
   EditSelectAll
   sleep 1

   SlideShowInteraction
      sleep 2
      Kontext "TabInteraktion"
      AktionBeiMausklick.select 3                       'Go to next Slide
         Printlog (AktionBeiMausKlick.GetSelText + " chosen")
      TabInteraktion.OK
      sleep 1
   Kontext "DocumentImpress"
   gMouseClick 90,90
   sleep 1
   gMouseClick 50,50
   sleep 2
   gMouseClick 90,90
   Kontext "NavigatorDraw"
      if Liste.GetSelIndex <> 4 Then
         Warnlog "- Jumped to wrong slide. Jumped to " + Liste.GetSelIndex + " instead of 4."
      else
         Printlog " Jumped to correct slide"
      end if
      sleep 1
   kontext "DocumentImpress"
   EditSelectAll
   sleep 1

   SlideShowInteraction
      sleep 1
      Kontext "TabInteraktion"
      AktionBeiMausklick.select 4                        'Go to first Slide
         Printlog AktionBeiMausKlick.GetSelText
         sleep 1
      TabInteraktion.OK
      sleep 1
   Kontext "DocumentImpress"
   gMouseClick 90,90
   sleep 1
   gMouseClick 50,50
   sleep 1
   gMouseClick 90,90
   Kontext "NavigatorDraw"
      if Liste.GetSelIndex <> 1 Then Warnlog "- jumped to wrong slide. Jumped to " + Liste.GetSelIndex + " instead of 1."
      Letzte.Click 'Liste.Select 4
   Kontext "DocumentImpress"
   EditSelectAll
   sleep 2

    try
        SlideShowInteraction
    catch
        warnlog "SlideshowInteraction diasabled :-("
    endcatch
      Kontext "TabInteraktion"
      AktionBeiMausklick.select 5                       'Go to last page
         printlog AktionBeiMausKlick.GetSelText
      TabInteraktion.OK
      sleep 1
   Kontext "NavigatorDraw"
      if Not Liste.GetSelIndex = 1 Then Warnlog "- jumped to wrong slide"

   kontext "DocumentImpress"
   gMouseClick 90,90
   sleep 1
   hMouseClick DocumentImpress,50,50
   Call hRechteckErstellen 20,20,50,50
   sleep 1
   EditSelectAll
   sleep 1
    try
        FormatGroupGroup
    catch
        warnlog "GROUP?"
    endcatch
   sleep 1

   SlideShowInteraction
      Kontext "TabInteraktion"
      AktionBeiMausklick.select 6                       'Sprung zu Seite oder Objekt
         printlog AktionBeiMausKlick.GetSelText
         sleep 2
         if Not ListeSprungZuSeiteObjekt.IsVisible Then Warnlog " list seems to be invisible"
         Seite.SetText S2
         Suchen.Click
            if ListeSprungZuSeiteObjekt.GetSelIndex <> 3 Then
               Warnlog " Search does not work. Current slide is: " + ListeSprungZuSeiteObjekt.GetSelText
            endif
      TabInteraktion.OK
      sleep 1

   Kontext "DocumentImpress"
   gMouseClick 90,90
   sleep 1
   gMouseClick 25,25
   sleep 2
   Kontext "NavigatorDraw"
      if Liste.GetSelIndex <> 2 Then
         Warnlog "- Jumped to wrong destination"
         printlog Liste.GetSelIndex
         Liste.Select 4
         Liste.TypeKeys "<RETURN>"
      endif
   Kontext "DocumentImpress"
   EditSelectAll
   sleep 1

   Kontext "DocumentImpress"
   EditSelectAll
   sleep 1
   SlideShowInteraction
      Kontext "TabInteraktion"
      sleep 1
      AktionBeiMausKlick.Select 7                    'Go to Document
      Printlog AktionBeiMausKlick.GetSelText + " chosen"
      try
         Durchsuchen.Click
         sleep 1
         Kontext "OeffnenDlg"
         sleep 1
         Dateiname.SetText ConvertPath (gTesttoolPath + "graphics\required\input\recht_49.odp")
         Oeffnen.Click
         sleep 10
         Kontext "TabInteraktion"
            sleep 1
            TabInteraktion.OK
         sleep 3
         Kontext "DocumentImpress"
      catch
         Warnlog "- Search button could not be accessed"
      endcatch
      sleep 1
      DocumentImpress.MouseDown 90,90
      DocumentImpress.MouseUp 90,90
      sleep 2
   Kontext "DocumentImpress"
   DocumentImpress.Mousedown 30,30
   kontext
   b115364 = false
   if messagebox.exists then
      printlog "baeh: '" + messagebox.gettext + "'"
      try
         messagebox.ok  ' was default in so7
      catch
         warnlog "behaviour changed #115364# mother document gets closed :-("
         if fileExists(ConvertPath (gOfficePath + "user\work\bug115364.odp")) then
            kill ConvertPath (gOfficePath + "user\work\bug115364.odp")
         endif
         messagebox.yes ' don't save changes, before going on!
         b115364 = true
         kontext "SpeichernDlg"
         Dateiname.setText ConvertPath (gOfficePath + "user\work\bug115364.odp")
         Speichern.click
         sleep 10
      endcatch
   else
      printlog "OK :-)"
   endif
         Kontext "DocumentImpress"
         DocumentImpress.MouseUp 30,30
         sleep 2
         try
              ViewDrawing
              sleep 1
              Kontext "DocumentImpress"
              DocumentImpress.MouseDown 25,25
              DocumentImpress.MouseUp 25,25
         catch
              Warnlog "- Jump to document did not work or preview window did not appear"
         endcatch
         sleep 2
       if (not b115364) then
         Call hCloseDocument
       else
         Call hCloseDocument
         hFileOpen (ConvertPath (gOfficePath + "user\work\bug115364.odp"))
       endif
         sleep 6

   printlog "Do Page down + Page up due to focusing-problems"
   kontext "Slides"
   hTypeKeys "<PAGEDOWN>"
   hTypeKeys "<PAGEUP>"
   Kontext "NavigatorDraw"
   if NavigatorDraw.Exists = FALSE then
      qaerrorlog "Navigator: NOT available :-( will be opened now!"
      Kontext "DocumentImpress"
      DocumentImpress.TypeKeys "<MOD1 SHIFT F5>"
   else
      printlog "Navigator: already open :-)"
   endif
   Kontext "NavigatorDraw"
   sleep 1
   if Naechste.IsEnabled then
      Naechste.Click
   else
      warnlog "'Next' wasn't visible. Clicking 'First' + Two 'Next'."
      Erste.Click
      sleep 2
      Naechste.Click
      Naechste.Click
      sleep 2
   endif

   Kontext "DocumentImpress"
   EditSelectAll
   sleep  1

   SlideShowInteraction
      Kontext "TabInteraktion"
      sleep 1
      AktionBeiMausKlick.Select 8
         Printlog AktionBeiMausKlick.GetSelText + " chosen"
         sleep 1
      Tabinteraktion.OK
      sleep 2
   SlideShowInteraction
      Kontext "TabInteraktion"
      sleep 1
      if not AktionBeiMausKlick.GetSelIndex = 8 Then Warnlog "- Not the right action chosen"
      sleep 1
      AktionBeiMausKlick.Select 8                          'Play sound
           Printlog  AktionBeiMausKlick.GetSelText + "- chosen"
         sleep 1
      Tabinteraktion.OK
      sleep 2
         Kontext "NavigatorDraw"
           sleep 1
           Liste.Select 2
         Kontext "DocumentImpress"
         EditSelectAll
         sleep  1
         SlideShowInteraction
         Kontext "TabInteraktion"
            sleep 1
            Durchsuchen.Click
            sleep 1
            Kontext "OeffnenDlg"
            sleep 1
            Dateiname.SetText ConvertPath (gTesttoolPath + "graphics\required\input\blip.wav")
            sleep 1
            Oeffnen.Click
               sleep 1
               Kontext "Active"
               if Active.Exists Then
                  Warnlog Active.GetText + "  Gallery might be empty"
                  Active.OK
                  sleep 1
                  Kontext "OeffnenDlg"
                  OeffnenDlg.Cancel
                  sleep 1
               end if
             Kontext "TabInteraktion"
             SetClipboard Klangwahl.GetText
             TabInteraktion.OK
         sleep 1
         SlideShowInteraction
            Kontext "TabInteraktion"
            sleep 1
            if Klangwahl.GetText <> GetClipboardText then Warnlog "- Not the right sound chosen"
            TabInteraktion.OK
         sleep 1
         InsertGraphicsFromFile
            sleep 1
            Kontext "GrafikeinfuegenDlg"
            sleep 1
            if Verknuepfen.IsChecked then Verknuepfen.UnCheck
            Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
            Oeffnen.Click
            sleep 3
         Kontext "NavigatorDraw"
            sleep 1
            Liste.Select 4
            sleep 1
         SlideShowInteraction
         Kontext "TabInteraktion"
            sleep 1
            AktionBeiMausKlick.Select 9                       'Objektaktion ausfuehren
            Printlog  AktionBeiMausKlick.GetSelText + " chosen"
            sleep 1
            Zaehler=Effekt.GetItemCount
            for i=1 to Zaehler
               Effekt.Select i
               Printlog Effekt.GetSelText + " chosen"

               TabInteraktion.OK
               sleep 1
               SlideShowInteraction
               Kontext "TabInteraktion"
               sleep 1
            next i
            TabInteraktion.OK
            sleep 1
   SlideShowInteraction
      Kontext "TabInteraktion"
      sleep 1
      AktionBeiMausKlick.Select 11 			'Macro
         Printlog AktionBeiMausKlick.GetSelText + " chosen"
         sleep 2
         Durchsuchen.Click
            sleep 5
            Kontext "ScriptSelector"
            sleep 5
            ScriptSelector.Cancel
         sleep 2
      Kontext "TabInteraktion"
      sleep 1
      TabInteraktion.OK
      sleep 3

   SlideShowInteraction
      Kontext "TabInteraktion"
      sleep 2
      if not AktionBeiMausKlick.GetSelIndex = 11 Then Warnlog "- Wrong action used"
      sleep 1
      AktionBeiMausKlick.Select 12           'End Presentation
         Printlog AktionBeiMausKlick.GetSelText
      Tabinteraktion.OK
      sleep 1

   SlideShowPresentationSettings
      Kontext "Bildschirmpraesentation"
      if not Fenster.IsChecked Then Fenster.Check
      BildschirmPraesentation.OK
   sleep 2
   Kontext "DocumentImpress"
   DocumentImpress.TypeKeys "<F5>"
      sleep 5
      Kontext "DocumentPresentation"
      sleep 2
      DocumentPresentation.MouseDown 50,50
      DocumentPresentation.MouseUp 50,50
      sleep 3
      if DocumentPresentation.Exists then
         qaerrorlog "#i51418# Sound played instead of ending presentation"
         DocumentPresentation.TypeKeys "<ESCAPE>"
         sleep 3
      endif
      try
         Kontext "DocumentImpress"
         EditSelectAll
         Printlog "- Slideshow ended at right point"
      catch
         Warnlog "- Program is still in slideshow mode"
         Kontext "DocumentPresentation"
         DocumentPresentation.TypeKeys "<ESCAPE>"
      endcatch

   Kontext "DocumentImpress"
   printlog "Close the Navigator"
   DocumentImpress.TypeKeys "<MOD1 SHIFT F5>"
   sleep 3
   printlog "Set start with current page to ON = Default"
   setStartCurrentPage(TRUE)
   Call hCloseDocument
endcase 'tSlideShowInteraction

'---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tAendernObjektBenennen
  Printlog "- Context/Name object"
  printlog "new document "
 Call hNewDocument
 Call sSelectEmptyLayout									

  sleep 1
  Kontext "DocumentImpress"
   if NOT DocumentImpress.Exists then
      Warnlog "- Due to Bug 91799 the document disappeared right after loading"
      Call hNewDocument
      sleep 1
   end if
  sleep 1
  hRechteckErstellen (20,20,30,30)
  sleep 1
  hRechteckErstellen (60,60,80,80)

  gMouseMove (10,10,90,90)
    Printlog "  - check if menu entry is disabled for normal objects"
  try
    ContextNameObject
    Warnlog "  function should be only enabled for graphics, ole objects and groups"
  catch
    Printlog "  Function disabled"
  endcatch
  EditSelectAll
  Kontext "DocumentImpress"
   FormatGroupGroup
   sleep 2
  gMouseClick 25,25
  ContextNameObject
  sleep (2)
  Kontext "NameDlgObject"
  NameField.SetText "Test"
  NameDlgObject.OK
  sleep 1
    Printlog "  - Name object worked"
  Kontext "DocumentImpress"
  EditSelectAll
  DocumentImpress.TypeKeys "<DEL>"
  InsertGraphicsFromFile
  Kontext "GrafikEinfuegenDlg"
   Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
   sleep 3
   Preview.Check
   sleep 1
   Oeffnen.Click
   sleep 1
   printlog "  - Name a graphic file"
   try
      ContextNameObject
      Kontext "NameDlgObject"
      Namefield.SetText "Grafik"
      NameDlgObject.OK
      sleep 1
      Printlog "  - Name a graphic object did work"
   catch
      Warnlog "  - Function should have been enabled"
   endcatch
   sleep 2
   Call hCloseDocument
   sleep 2
endcase 'tAendernObjektBenennen

'---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tSlideshowBackgroundAllSlides
    Dim Datei$ as string
    Dim Background$ as string
    Dim i as integer
    Dim Zaehler as integer
    Background$ = ConvertPath(gOfficeBasisPath + "share\gallery\www-back\chocolate.jpg")
    Datei$ = ConvertPath (gOfficePath + "user\work\background.odp")

    printlog "Create a new document"
    Printlog "- SlideshowBackground"
    Call hNewDocument

    printlog "Insert a new Slide"
    Kontext "DocumentImpress"
    InsertSlide

    printlog "Open the Contextmenu"
    DocumentImpress.OpenContextMenu  

    printlog "Select the entry Slide"
    hMenuFindSelect (".uno:PageMenu", true, 1, true)
    printlog "Select the entry Background"
    hMenuSelectNr (4)

    printlog "In the open-file dialogue, choose and open a graphic-file you wish to have as a background"
    kontext "GrafikEinfuegenDlg"
    If GrafikEinfuegenDlg.Exists Then
        DateiName.SetText (Background$)
        Oeffnen.Click
    Else
        warnlog "GrafikEinfuegenDlg not existent... check why."
    End if

    printlog "If an active-message shows up, press yes to apply this background to all slides."
    kontext "Active"
    if Active.Exists then
        Active.Yes
    else
        QaErrorLog "No dialogue considering if we wish to apply the background to all slides."
    end if

    printlog "Save the document"
    Call hFileSaveAsKill (Datei$)
    printlog "Close the document"
    Call hCloseDocument
    printlog "Open the document"
    Call hFileOpen (Datei$)
    printlog "Close the document"
    Call hCloseDocument
endcase 'tSlideshowBackgroundAllSlides

'---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tSlideshowBackgroundSelectedSlide

    Dim Datei$ as string
    Dim Background$ as string
    Dim i as integer
    Dim Zaehler as integer
    Background$ = ConvertPath(gOfficeBasisPath + "share\gallery\www-back\chocolate.jpg")
    Datei$ = ConvertPath (gOfficePath + "user\work\background.odp")

    printlog "Create a new document"
    Printlog "- SlideshowBackground"
    Call hNewDocument

    printlog "Insert a new Slide"
    Kontext "DocumentImpress"
    InsertSlide

    printlog "Open the Contextmenu"
    DocumentImpress.OpenContextMenu  

    printlog "Select the entry Slide"
    hMenuFindSelect (".uno:PageMenu", true, 1, true)
    printlog "Select the entry Background"
    hMenuSelectNr (4)

    printlog "In the open-file dialogue, choose and open a graphic-file you wish to have as a background"
    kontext "GrafikEinfuegenDlg"
    If GrafikEinfuegenDlg.Exists Then
        DateiName.SetText (Background$)
        Oeffnen.Click
    Else
        warnlog "GrafikEinfuegenDlg not existent... check why."
    End if

    printlog "If an active-message shows up, press No to only apply this background to the selected slide."
    kontext "Active"
    if Active.Exists then
        Active.No
    else
        QaErrorLog "No dialogue considering if we wish to apply the background to all slides."
    end if

    printlog "Save the document"
    Call hFileSaveAsKill (Datei$)
    printlog "Close the document"
    Call hCloseDocument
    printlog "Open the document"
    Call hFileOpen (Datei$)
    printlog "Close the document"
    Call hCloseDocument
endcase 'tSlideshowBackgroundSelectedSlide

'---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
