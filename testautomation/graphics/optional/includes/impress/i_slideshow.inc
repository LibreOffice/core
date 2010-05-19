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
'*
' #1 tSlideShowSlideShow
' #1 tSlideShowRehearseTimings
' #1 tSlideShowSlideShowSettings
' #1 tSlideShowCustomSlideShow
' #1 tSlideShowSlideTransition
' #1 tSlideShowShowHideSlide
'*
'\******************************************************************
testcase tSlideShowSlideShow

    dim bLoaded as boolean
    dim i as integer

    '/// open file 'graphics\\input\\diashow.odp' ///'
    hFileOpen  ConvertPath (gTesttoolPath + "graphics\required\input\diashow.odp")
    sleep 30

    printlog "check if the document is writable"
    sMakeReadOnlyDocumentEditable

   bLoaded = false

   Kontext "Navigator"
   if Navigator.Exists then Navigator.Close

   '/// Deactivate "Start with current page" in ToolsOptions ///'
   setStartCurrentPage(FALSE)

   try
   '/// SlideShow->Slide Show Settings... ///'
      SlideShowPresentationSettings
      bLoaded = true
   catch
      warnlog "SlideShowPresentationSettings could not be run, because document didn't get loaded in time ??! :-("
   endcatch
   if bLoaded then
      Kontext "Bildschirmpraesentation"

      '/// check Radio button 'Type': 'Window' ///'
      Fenster.Check
      '/// close dialog 'Slide Show' with OK ///'
      Bildschirmpraesentation.OK
      sleep 3
      '/// start the slideshow with 'Slide Show->Slide Show' ///'
      SlideShowSlideshow
         sleep 3
         Kontext "DocumentPresentation"
         sleep 5
      '/// end the presentation by typing [ESCAPE] ///'
    DocumentPresentation.TypeKeys "<ESCAPE>"
      sleep 3
        try
           hTypeKeys "<TAB>"
          Printlog "- Slideshow ended using 'ESCAPE'"
       catch
          Warnlog "- Slideshow did not end"
        DocumentPresentation.TypeKeys "<ESCAPE>"
          sleep 3
        endcatch

      '/// start the slideshow with 'Slide Show->Slide Show' ///'
      SlideShowSlideshow
      Kontext "DocumentPresentation"
      Printlog "- Check if slideshow runs"
       sleep 5
      try
         DocumentPresentation.TypeKeys "<pagedown>"
      catch
         Warnlog "Presentation is not running !"
      endcatch
      sleep 5
      '/// type the key [SHIFT + F5] to open the navigator ///'
      DocumentPresentation.TypeKeys "<MOD1 SHIFT F5>"
      Kontext "NavigatorDraw"
         sleep 1
         '/// check in list, if the page changed ///'
         if Liste.GetSelIndex <> 2 Then Warnlog "- pagedown not working: '" + Liste.GetSelText+"'"
         Kontext "DocumentPresentation"
         '/// press key [cursor right] 3 times ///'
         for i = 1 to 3
            sleep 3
            DocumentPresentation.TypeKeys "<right>"
         next i
         sleep (3)
         DocumentPresentation.MouseDown 50,50
         DocumentPresentation.MouseUp 50,50
         sleep 10
      try
         Kontext "DocumentPresentation"
         DocumentPresentation.MouseDown 50,50
         DocumentPresentation.MouseUp 50,50
         Warnlog "- Slideshow still running"
         DocumentPresentation.TypeKeys "<ESCAPE>"
      catch
         Printlog "- Slideshow ended at the right time"
      endcatch
      sleep 2
   else
      warnlog "Document didn't get loaded"
   endif
   Kontext "DocumentImpress"
   '///  Close the Navigator ///'
   DocumentImpress.TypeKeys "<MOD1 SHIFT F5>"
   '/// Set "Start with current page" back to default = on, in ToolsOptions ///'
   setStartCurrentPage(TRUE)
   '/// close the application ///'
   Call hCloseDocument
   
endcase 'tSlideShowSlideShow

'-------------------------------------------------------------------------------

testcase tSlideShowRehearseTimings
    qaerrorlog "#i64783# - tSlideShowRehearseTimings outcommented due to bug"
    goto endsub
    dim bLoaded as boolean
    dim i as integer

   '/// open file 'graphics\\input\\diashow.odp' ///'
   hFileOpen ConvertPath (gTesttoolPath + "graphics\required\input\diashow.odp")
   sleep 10
   ' check if the document is writable
   if fIsDocumentWritable = false then
      ' make the document writable and check if it's succesfull
      if fMakeDocumentWritable = false then
         warnlog "The document can't be make writeable. Test stopped."
         goto endsub
      endif
   endif
   Kontext "Navigator"
   if Navigator.Exists then Navigator.Close
   bLoaded = false
   try
      '/// SlideShow->Slide Show Settings... ///'
        SlideShowPresentationSettings
      bLoaded = true
    catch
      warnlog "SlideShowPresentationSettings could not be run, because document didn't get loaded in time ( ?? ) :-("
   endcatch
   if bLoaded then
       Kontext "Bildschirmpraesentation"
      '/// check Radio button 'Type': 'Window' ///'
       Fenster.Check
      '/// close dialog 'Slide Show' with OK ///'
       Bildschirmpraesentation.OK
       sleep 3
      '/// start the slideshow with 'Slide Show->Rehearse Timings' ///'
      SlideShowRehearseTimings
      Kontext "DocumentPresentation"
      if DocumentPresentation.Exists then
        printlog "OK"
      else
        warnlog "bah"
      endif
     '/// press key [return] 5 times ///'
      for i = 1 to 5
         sleep 5
         DocumentPresentation.TypeKeys "<return>"
      next i
      sleep 5
      try
         gMouseClick 50,50
          Printlog "- Slideshow ended "
      catch
         Kontext "DocumentPresentation"
           Warnlog "- Slideshow still running"
          DocumentPresentation.TypeKeys "<ESCAPE>"
           sleep 3
      endcatch
       sleep 2
   else
      warnlog "Documnet didn't get loaded"
   endif

   '///  check state of navigator ! expected: closed ///'
   Kontext "Navigator"
   if Navigator.exists then
      '///+ close navigator ! ///'
      Navigator.Close
   else
      printlog "Navigator: NOT available"
   endif
   '/// close the application ///'
   Call hCloseDocument
   
endcase 'tSlideShowRehearseTimings

'-------------------------------------------------------------------------------
testcase tSlideShowSlideShowSettings

   dim bLoaded as boolean
   dim i as integer
   dim x as integer
   dim sTemp as string
   dim saSlideNames(4) as string
   dim bCurrentPage as boolean

   S1 = glLocale (2) + " 1"
   S2 = glLocale (2) + " 2"
   S3 = glLocale (2) + " 3"
   S4 = glLocale (2) + " 4"
   saSlideNames(1) = S1
   saSlideNames(2) = S2
   saSlideNames(3) = S3
   saSlideNames(4) = S4

   ' the navigator you open during a presentation is closed after the last slide (in full screen mode)
   ' the state of the navigator in the normal view is not affected
   ' the navigator that is open in normal view, is not visible in windows presenattions

   '/// open application ///'
   Call hNewDocument

'///  check state of navigator ! expected: closed ///'
   Kontext "Navigator"
   if Navigator.exists then
      '///+ close navigator ! ///'
      Navigator.Close
      QaErrorLog "Navigator was open. Check earlier tests. Now closed."
   else
      printlog "Navigator: NOT available. Good."
   endif

   '/// Slide Show -> Slide Show Settings ///'
    SlideShowPresentationSettings
      Kontext "Bildschirmpraesentation"
       '/// check checkbox in section 'Range' - 'From: ///'
       AbDia.Check
      '/// get count of slides from listbox 'From:' ///'
      x = AbDiaName.GetItemCount
      '/// check if count in listbox 'from' is '0' ///'
      if (x = 1) Then
         Printlog "  - Slide count in the list: '" + x + "'"
      else
          Warnlog "  - unexpected slide count; should be '1'; is '" + x + "'"
       end if
   '/// close dialog 'Slide Show' with OK ///'
   Bildschirmpraesentation.OK
   '/// Insert->Slide... ///'
   InsertSlide
   sleep 2
   hTypekeys "<Pagedown>"
   sleep 2
   '/// Slide Show -> Slide Show Settings ///'
   SlideShowPresentationSettings
      Kontext "Bildschirmpraesentation"
      Printlog "'///<b> check if count in listbox 'from' increased </b>///'"
      i = AbDiaName.GetItemCount
      if ((i -1) = x) Then
         Printlog "  - Slide appears in the list; count : '" + i + "'"
      else
          Warnlog "  - Slide not added; is '" + i + "' should: '" + x + "'"
       end if
   '/// close dialog 'Slide Show' with CANCEL ///'
   Bildschirmpraesentation.Cancel
   '/// close application ///'
   Call hCloseDocument
   sleep 5

   bLoaded = false
   '/// open file 'graphics\\input\\diashow.odp' (Slide Show with 4 Slides) ///'
   hFileOpen   ConvertPath (gTesttoolPath + "graphics\required\input\diashow.odp")
   sleep 10
   printlog "check if the document is writable"
   sMakeReadOnlyDocumentEditable

   try
      Kontext "DocumentImpress"
      '/// Slide Show -> Slide Show Settings ///'
      Printlog "'--------------------------------------------------------------------------"
      SlideShowPresentationSettings
      bLoaded = true
   catch
      warnlog "SlideShowPresentationSettings could not be run, because document didn't get loaded in time ( ?? ) :-("
   endcatch
   if bLoaded then
      Kontext "Bildschirmpraesentation"
      if (AlleDias.IsChecked = FALSE) then
         Warnlog "'All Slides' is not checked :-("
      endif
      if (Standard.IsChecked = FALSE) then
         Warnlog "'Default' is not checked :-("
      endif
      if (NavigatorSichtbar.IsChecked) then
         Warnlog "'Navigator visible' is checked :-("
      endif
      '///<b> check checkbox 'All Slides' </b>///'
      AlleDias.Check
      '///<b> check checkbox 'Window' </b>///'
      Fenster.Check
      '///<b> check checkbox 'Navigator visible' </b>///'
      NavigatorSichtbar.Check                 '--------------------------------------------------------------------------
      '/// close dialog 'Slide Show' with OK ///'
      Bildschirmpraesentation.OK
      '/// start the slideshow with 'Slide Show->Slide Show' ///'
      SlideShowSlideshow
      Printlog "'---------------------------- START P -- All -- Window -- Navigator -------------------------"
     sleep 5
      '///  check state of navigator ! expected: open ///'
      Kontext "Navigator"
      if Navigator.exists then
         Printlog "Navigator: open :-)"
      else
         Warnlog "Navigator: NOT available :-( will be opened now!"
         Kontext "DocumentPresentation"
         DocumentPresentation.TypeKeys "<MOD1 SHIFT F5>"
      endif
      sleep 1
      Kontext "DocumentPresentation"
      try
         '/// press pagedown ///'
         DocumentPresentation.TypeKeys "<pagedown>"
      catch
         Warnlog "presentation is not running !"
      endcatch
      sleep 5
      Kontext "NavigatorDraw"
      if Liste.GetSelIndex <> 2 Then
         warnlog "Wrong slide."
      endif
      '/// press the key [Page Down] 3 times ///'
      '///+ check in Navigator list, if the slide is number: (times key pressed) ///'
      for i = 2 to 4
         Kontext "Navigator"
         if Navigator.exists then
            printlog "Navigator was there, good."
         else
            Warnlog "The Navigator should have been activated, but was not. Opening now."
            Kontext "DocumentPresentation"
            DocumentPresentation.TypeKeys "<MOD1 SHIFT F5>"
         endif
         fGetSlideName(saSlideNames(i))
         kontext "DocumentPresentation"
         DocumentPresentation.TypeKeys "<pagedown>"
      next i
      sleep (2)
      '/// press the key [Page Down] to exit presentation ///'
      DocumentPresentation.TypeKeys "<pagedown>"
      sleep (5)      
      try
         gMouseClick 50,50  ' this one works to recognice the not ending ! TBO
         Printlog "- Slideshow ended at the right time"
      catch
         Warnlog "- Slideshow still running "
         DocumentPresentation.TypeKeys "<ESCAPE>"
      endcatch
      sleep 2
      '///  check state of navigator ! expected: open ///'
      Kontext "Navigator"
      if Navigator.exists then
         '///+ close navigator ! ///'
         Navigator.Close
      else
         printlog "Navigator: NOT available"
      endif
      Printlog "'--------------------------------------------------------------------------"

       Kontext "DocumentPresentation"
      '/// type the key [MOD1 SHIFT F5] to open the navigator ///' '--------------------------------------------------------------------------
      hTypeKeys "<MOD1 SHIFT F5>"
      '/// Slide Show -> Slide Show Settings ///'
      SlideShowPresentationSettings
         Kontext "Bildschirmpraesentation"
         if (AlleDias.IsChecked = FALSE) then
            Warnlog "'All Slides' is not checked :-("
         endif
         if (Fenster.IsChecked = FALSE) then
            Warnlog "'Window' is not checked :-("
         endif
         if (NavigatorSichtbar.IsChecked = FALSE) then
            Warnlog "'Navigator visible' is not checked :-("
         endif
         if (DiawechselAufHintergrund.IsChecked = FALSE) then
            Warnlog "'Change slides by clicking on background' is not checked :-("
         endif
           '///<b> check checkbox in section 'Range' - 'From: </b>///'
         AbDia.Check
         '/// select the 3rd item from the top from listbox 'From:' -> 2 slides to go ///'
         AbDiaName.Select 3
         sTemp = AbDiaName.GetSelText
         Printlog "  -  From '" + sTemp + "' will be shown"
           '///<b> check checkbox 'Default' </b>///'
         Standard.Check
         '/// UNcheck checkbox 'Navigator visible'///'
         NavigatorSichtbar.UnCheck
         '/// UNcheck checkbox 'Change slides by clicking on background'///'
         DiawechselAufHintergrund.UnCheck
      '/// close dialog 'Slide Show' with OK ///'
      Bildschirmpraesentation.OK
       sleep 3
       Kontext "DocumentPresentation"
      '/// start the slideshow with 'Slide Show->Slide Show' ///'
      SlideShowSlideshow
         Printlog "'---------------------------- START P -- From -- Default -- Click on Bg -----------------------"
         sleep 5
         '///  check state of navigator ! expected: closed ///'
         Kontext "Navigator"
         if Navigator.exists then
            '///+ close navigator ! ///'
            Navigator.Close
            Warnlog "Navigator: closed"
         else
            Printlog "Navigator: NOT available. Good."
         endif
         sleep 1
         '/// type the key [MOD1 SHIFT F5] to open the navigator ///' '--------------------------------------------------------------------------
         Kontext "DocumentPresentation"
         DocumentPresentation.TypeKeys "<MOD1 SHIFT F5>"
          sleep 3
          '/// check in Navigator list, if the page is THREE! ///'
          fGetSlideName(S3)
          '/// click right mouse button ///'
          sleep 5
          DocumentPresentation.MouseDown 50,50
          DocumentPresentation.MouseUp 50,50
          '/// check in Navigator list, if the page is still THREE. If so, change slide with PageDown ///'
          if (fGetSlideName(S3) <> S3) then
             Warnlog "'Change slides by clicking on background' disable did not work :-("
          else
             DocumentPresentation.TypeKeys "<PAGEDOWN>"
          endif
          '/// check in Navigator list, if the page is FOUR ///'
          fGetSlideName(S4)
          sleep 5
          '/// press the key [Page Down] ///'
          DocumentPresentation.TypeKeys "<pagedown>"
          sleep 5
          '/// press the key [Space] to exit presentation ///'
          DocumentPresentation.TypeKeys "<Space>"
         sleep 10
      try
         ViewZoom '-----------!!!!!1 TBO: Make function!!! ----------------------- works! TBO
         Kontext "Massstab"
         Massstab.OK
      catch
       Kontext "DocumentPresentation"
         DocumentPresentation.TypeKeys "<ESCAPE>"
         Warnlog "  - Program was still in slideshow mode - ended now ?"
      endcatch
      '///  check state of navigator ! expected: open ///'
      Kontext "Navigator"
      if Navigator.exists then
         '///+ close navigator ! ///'
         Navigator.Close
         printlog "Navigator: closed"
      else
         warnlog "Navigator: NOT available"
      endif
      Printlog "'--------------------------------------------------------------------------"

      '/// Slide Show -> Slide Show Settings ///'
      SlideShowPresentationSettings
         Kontext "Bildschirmpraesentation"
         if (AbDia.IsChecked = FALSE) then
            Warnlog "'From' is not checked :-("
         endif
         if (Standard.IsChecked = FALSE) then
            Warnlog "'Default' is not checked :-("
         endif
         if (AbDiaName.GetSelText <> sTemp ) then
            Warnlog "'From Slide' has wrong value :-(; should be: '" + sTemp + "' ; is: '" + AbDiaName.GetSelText + "'"
         endif
         if (DiawechselAufHintergrund.IsChecked) then
            Warnlog "'Change slides by clicking in background' is checked :-("
         endif
         '/// check checkbox 'All Slides' ///'
         AlleDias.Check
         '///<b> check check box 'Auto' -> implies looping of slideshow in fullscreen mode </b>///'
         Auto.Check
         '/// set duration of pause to '00:00:05' ///'
         Zeit.SetText "00:00:05"
         '/// check check box 'Show logo' ///'
         LogoAnzeigen.Check
         '/// check checkbox 'Change slides by clicking on background'///'
         DiawechselAufHintergrund.Check
      '/// close dialog 'Slide Show' with OK ///'
      Bildschirmpraesentation.OK
      '/// start the slideshow with 'Slide Show->Slide Show' ///'
      SlideShowSlideshow
      Printlog "'---------------------------- START P -- All -- Auto -- :05 -- Logo -----------------------------"
      sleep 3
      Kontext "DocumentPresentation"
      '/// type the key [MOD1 SHIFT F5] to open the navigator ///' '--------------------------------------------------------------------------
      DocumentPresentation.TypeKeys "<MOD1 SHIFT F5>"
      '/// press the key [Space] 4 times ///'
      '///+ check in Navigator list, if the slide is number: [(times key pressed) -1 ] ///'
      for i = 1 to 4
         fGetSlideName(saSlideNames(i))
         DocumentPresentation.TypeKeys "<Space>"
         sleep 5
      next i
        '/// wait until autopause ended (5 secs) ///'
        sleep 10
      if (fGetSlideName(saSlideNames(1)) = saSlideNames(1)) then
         Printlog "  - Enless loop works"
      else
         Warnlog " Enless loop does NOT work"
      end if
      sleep 2
      '/// type key [excape] to exit presentation ///'
      Kontext "DocumentPresentation"
      DocumentPresentation.TypeKeys "<ESCAPE>"
      sleep 5
      try
         SlideShowPresentationSettings
         Kontext "Bildschirmpraesentation"
         Bildschirmpraesentation.OK
      catch
         Warnlog "  - Still in slideshow mode, tries to end again..."
         Kontext "DocumentPresentation"
         DocumentImpress.TypeKeys "<ESCAPE>"
         sleep 5
      endcatch
      '///  check state of navigator ! expected: closed ///'
      Kontext "Navigator"
      if Navigator.exists then
         Navigator.Close
         warnlog "Navigator: closed"
      else
         printlog "Navigator: NOT available"
      endif
      Printlog "'--------------------------------------------------------------------------"

      Kontext "DocumentImpress"
      '/// Slide Show -> Slide Show Settings ///'
      SlideShowPresentationSettings
         Kontext "BildschirmPraesentation"
         '/// check checkbox 'default' ///'
         Standard.Check
         '///<b> check checkbox 'Change slides maually' </b>///'
         ' to check this i need an automatic transition somewhere in the presentation !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
         DiawechselManuel.Check
         '///<b> check checkbox 'Mouse pointer as pen' </b>///'
         MauszeigerAlsStift.Check
      '/// close dialog 'Slide Show' with OK ///'
      BildschirmPraesentation.Ok
      sleep 1

      '/// goto the first slide by typing [strg home] ///'
      hTypeKeys "<mod1 home>"
      '///+ -  SlideShow->SlideTransition ///'
      SlideShowSlideTransition
         Kontext "Tasks"
         '///+ -  click button 'Extras' ///'
         sleep 1
         '///+ - + click button 'Automatic Transition' ///'
         TransitionAutomaticallyAfter.Check 'click
         '///+ -  + +  Set Time to '00:00:01' ///'
         TransitionAutomaticallyAfterTime.SetText "2"
         '///+ -  click button 'Assign' ///'
         TransitionApplyToAllSlides.Click
         '///+ - close dialog 'Slide Transition' ///'
      sleep 3

      '/// start the slideshow with keys [STRG F2] ///'
       Kontext "DocumentImpress"
      DocumentImpress.TypeKeys "<MOD1 F2>"
      Printlog "'---------------------------- START P -- Default -- Manuel ------------------------------"
      '/// wait 10 seconds (to get the dia changed automatical has not to happen !) ///'
      sleep 10
      Kontext "DocumentPresentation"
         '/// type the key [MOD1 SHIFT F5] to open the navigator ///' '--------------------------------------------------------------------------
         DocumentPresentation.TypeKeys "<MOD1 SHIFT F5>"
         '/// press key [cursor right] 3 times ///'
         for i = 1 to 4
            sleep 5
            fGetSlideName(saSlideNames(i))
            '///+ use the left mouse button, to use the pen drawing function ///'
            DocumentPresentation.MouseDown (10, 10)
            DocumentPresentation.MouseMove (10, 90)
            DocumentPresentation.MouseUp (10, 90)
            DocumentPresentation.TypeKeys "<right>"
         next i
      '/// press the key [Page Down] to exit presentation ///'
      sleep (2)
      DocumentPresentation.TypeKeys "<pagedown>"
      sleep (2)
      if DocumentPresentation.Exists then
         Warnlog "- Slideshow still running "
         DocumentPresentation.TypeKeys "<ESCAPE>"
      else
         Printlog "- Slideshow ended at the right time"
      endif
      sleep 2
      '///  check state of navigator ! expected: closed ///'
      Kontext "Navigator"
      if Navigator.exists then
         '///+ close navigator ! ///'
         Navigator.Close
         warnlog "Navigator: closed"
      else
         printlog "Navigator: NOT available"
      endif
      Printlog "'--------------------------------------------------------------------------"

      Kontext "DocumentImpress"
      '/// Slide Show -> Slide Show Settings ///'
      SlideShowPresentationSettings
         Kontext "BildschirmPraesentation"
         '/// check checkbox 'default' ///'
         Standard.Check
         '///<b> UNcheck checkbox 'Change slides manually' </b>///'
         DiawechselManuel.UNCheck
         '/// UNcheck checkbox 'Mouse pointer as pen' ///'
         MauszeigerAlsStift.UNCheck
      '/// close dialog 'Slide Show' with OK ///'
      BildschirmPraesentation.Ok
      sleep 1
      '/// goto the first slide by typing [strg home] ///'
      hTypeKeys "<MOD1 HOME>"
      '/// Set all slides to 'wait 10 seconds' to get the slide changed automatically ///'
      '///+ -  SlideShow->SlideTransition ///'
      SlideShowSlideTransition
         Kontext "Tasks"
         '///+ -  click button 'Extras' ///'
         sleep 1
         '///+ - + click button 'Automatic Transition' ///'
         TransitionAutomaticallyAfter.Check 'click
         '///+ -  + +  Set Time to '10sec' ///'
         TransitionAutomaticallyAfterTime.SetText "10"
         '///+ -  click button 'Apply to all Slides' ///'
         TransitionApplyToAllSlides.Click
         '///+ - close dialog 'Slide Transition' ///'
         sleep (1)
      '/// start the slideshow with keys [STRG F2] ///'
      Kontext "DocumentImpress"
      DocumentImpress.TypeKeys "<MOD1 F2>"
      Printlog "'---------------------------- START P -- Default -- Automatic -----------------------------"
      '/// Wait 6 seconds to see if the Automatic Transition changes the slide. ///'
      sleep 6
      Kontext "DocumentPresentation"
         '/// type the key [MOD1 SHIFT F5] to open the navigator ///' '--------------------------------------------------------------------------
         DocumentPresentation.TypeKeys "<MOD1 SHIFT F5>"
         '/// press key [cursor right] 3 times ///'
         for i = 2 to 4
            sleep (5)
            fGetSlideName(saSlideNames(i))
            DocumentPresentation.TypeKeys "<right>"
         next i
      '/// press the key [Page Down] to exit presentation ///'
      sleep (2)
      DocumentPresentation.TypeKeys "<PAGEDOWN>"
      sleep (2)
      if DocumentPresentation.Exists then
         Warnlog "- Slideshow still running "
         DocumentPresentation.TypeKeys "<ESCAPE>"
      else
         Printlog "- Slideshow ended at the right time"
      endif
      sleep 2
      '///  check state of navigator ! expected: closed ///'
      Kontext "Navigator"
      if Navigator.exists then
         '///+ close navigator ! ///'
         Navigator.Close
      endif
   else
      warnlog "document didn't get loaded"
   endif
   '/// Set "Start with current page" back to default = on, in ToolsOptions ///'
   setStartCurrentPage(TRUE)
   '/// Close the document ///'
   call hCloseDocument
   
endcase 'tSlideShowSlideShowSettings

'-------------------------------------------------------------------------------
testcase tSlideShowCustomSlideShow

   Dim Seite as string
   dim i as integer
   dim x as integer
   dim sTemp as string

   Seite = s1
   '/// open application ///'
   Call hNewDocument

   Kontext "Navigator"
   if Navigator.Exists then Navigator.Close
   '/// Deactivate "Start with current page" in ToolsOptions ///'
   setStartCurrentPage(FALSE)

   '/// create a rectangle ///'
   hRechteckErstellen 30,30,60,60
   '/// do 2 times : ///'
   for i = 1 to 2
      '///+ Insert->Slide... ///'
      InsertSlide
      sleep 2
      hTypekeys "<Pagedown>"
      sleep 2
      '///+ create a rectangle ///'
      hRechteckErstellen 30,30,70,70
   next i

   '/// Slide Show -> Slide Show Settings ///'
   SlideShowPresentationSettings
      Kontext "Bildschirmpraesentation"
      if (Fenster.IsChecked = False) Then Fenster.Check
      '/// close dialog 'Slide Show' with OK ///'
      Bildschirmpraesentation.OK

   '/// Slide Show -> Custom Slide Show... ///'
   SlideShowCustomSlideshow
      Kontext "IndividuellePraesentation"
      '///<b> click button 'New...' </b>///'
      Neu.Click

         Kontext "IndividuellePraesentationDefinieren"
         '/// type something in 'Name' ///'
         PraesentationName.SetText "Individually 1"
         '/// select the first entry in the list 'Existing Slides' ///'
         SeitenPraesentation.Select 1
         '/// click button '>>' ///'
         Hinzufuegen.Click
         i = SelectedSlides.GetItemCount
         if (i <> 1) Then
            Warnlog "-  Number of slides 'Selected Slides' is not correct. It should be = '1', but it is = '" + i + "'"
         endif
         '/// close dialog 'Define Custom Slide Show' with OK ///'
         IndividuellePraesentationDefinieren.OK
      Kontext "IndividuellePraesentation"
      '///<b> click button 'Edit...' </b>///'
      Bearbeiten.Click
         Kontext "IndividuellePraesentationDefinieren"
         '/// select an entry in the list 'Selected Slides' ///'
         SelectedSlides.TypeKeys "<pagedown>"
         '/// click button '<<' ///'
         Entfernen.Click
         i = SelectedSlides.GetItemCount
         if (i <> 0) Then
            Warnlog "  Removing slides from list 'Selected Slides' does not work It should be = '1', but it is = '" + i + "'"
         endif
         '/// select the first entry in the list 'Existing Slides' ///'
         SeitenPraesentation.Select 1
         '/// click button '>>' ///'
         Hinzufuegen.Click
         '/// close dialog 'Define Custom Slide Show' with OK ///'
         IndividuellePraesentationDefinieren.OK
      Kontext "IndividuellePraesentation"

      '///<b> check checkbox 'Use Custom Slide Show' </b>///' '-------------------------------------------------------
      IndividuellePraesentationBenutzen.Check
   '///<b> click button 'Start...' </b>///'
   Starten.Click
      sleep 2
      Kontext "DocumentPresentation"
     '/// press key [space] 2 times ///'
      DocumentPresentation.TypeKeys "<space>"
      sleep 2
      DocumentPresentation.TypeKeys "<space>"
      sleep 5
      try
         Kontext "IndividuellePraesentation"
         '/// close dialog 'Custom Slide Shows' with button 'Close' ///'
         IndividuellePraesentation.Close
      catch
         Warnlog "- Slideshow did not end after slide 1"
         DocumentPresentation.TypeKeys "<space>"
         sleep 2
      endcatch
   '/// Slide Show -> Custom Slide Show... ///'
   SlideShowCustomSlideshow
      Kontext "IndividuellePraesentation"
      '/// click button 'Edit...' ///'
      Bearbeiten.Click
         Kontext "IndividuellePraesentationDefinieren"
         '/// select 1st entry in the list 'Selected Slides' ///'
         SelectedSlides.Select 1
         '/// click button '<<' ///'
         Entfernen.Click
         i = SelectedSlides.GetItemCount
         if (i <> 0) Then
            Warnlog "-  Add and remove slides does not work properly. It should be '0', but it is = '"+i+"'"
         endif
         '/// select the first entry in the list 'Existing Slides' ///'
         SeitenPraesentation.Select 1
         '/// click button '>>'  4 times ///'
         for i =1 to 4
            Hinzufuegen.Click
         next i
         for i =1 to 4
            SelectedSlides.Select i
            sTemp = SelectedSlides.GetSelText
            If (sTemp <> Seite) then
               Warnlog "-  Wrong slide added to individual slideshow. Instead of '" + Seite + "', '" + sTemp + "' was added"
            endif
         next i
         '/// close dialog 'Define Custom Slide Show' with OK ///'
         IndividuellePraesentationDefinieren.OK
      sleep 1
      Kontext "IndividuellePraesentation"

      '///<b> UNcheck checkbox 'Use Custom Slide Show' </b>///'
      IndividuellePraesentationBenutzen.UnCheck
      '/// click button 'Start...' ///'
      Starten.Click               '---------------------------- START P ----------------------------------------------
        '/// press key [space] 4 times ///'
         kontext "DocumentPresentation"
         for i = 1 to 4
            sleep 3
            DocumentPresentation.TypeKeys "<space>"
         next i
         sleep 10
      kontext "IndividuellePraesentation"
   '/// close dialog 'Custom Slide Shows' with button 'Close' ///'
    if IndividuellePraesentation.exists then
       IndividuellePraesentation.Close
    else
       Warnlog "'IndividuellePraesentation' was closed :-("
    end if

   '/// Set "Start with current page" back to default = on, in ToolsOptions ///'
   setStartCurrentPage(TRUE)
   '/// close the application ///'
         Kontext "Navigator"
      if Navigator.exists then
         '///+ close navigator ! ///'
         Navigator.Close
      else
         printlog "Navigator: NOT available"
      end if
      Call hCloseDocument
      
endcase 'tSlideShowCustomSlideShow

'-------------------------------------------------------------------------------
testcase tSlideShowSlideTransition

   dim i as integer
   dim x as integer
   dim q as integer
   dim sTemp as string
   dim TimerValue as string
   dim sFile as string
   dim sEffect (3) as string

   sFile = ConvertPath (gOfficePath + "user\work\diashow")

'   '/// open application ///'
    call hNewDocument

   '/// close the navigator///'
   Kontext "Navigator"
   if Navigator.Exists then Navigator.Close
   WaitSlot (3000)
   '/// create 3 slides with a rectangle ///'
   hRechteckErstellen (30,30,60,60)
   for i = 1 to 2
      '///+  - Insert->Slide ///'
      WaitSlot (1000)
      InsertSlide
      WaitSlot (2000)
      hTypekeys "<Pagedown>"
      WaitSlot (2000)
      '///+ - insert rectangle///'
      hRechteckErstellen (30,30,60,60)
   next i
   Printlog "created 3 slides"
   '/// for all 3 slides do: ///'
   for i = 1 to 3
      '///+ -  open navigator with shortcutkey [MOD1 SHIFT F5] ///'
      hTypeKeys "<MOD1 SHIFT F5>"
         Sleep (3)
         Kontext "NavigatorDraw"
         '///+ -  select the next slide in the list ///'
         if Liste.GetItemCount < 3 then
            warnlog "We have " + Liste.GetItemCount + " entries in the Navigator."
            hTypeKeys "<MOD1 SHIFT F5>"
            hTypeKeys "<MOD1 SHIFT F5>"
            Kontext "NavigatorDraw"
         end if
         Liste.Select i
         '///+ - press key [return] ///'
         Liste.TypeKeys "<return>"
         Kontext "Navigator"
      '///+ -  close Navigator ///'
      Navigator.Close
      '///+ -  SlideShow->SlideTransition ///'
      SlideShowSlideTransition
         Kontext "Tasks" '"SlideTransition"' "OL_DIA_Diawechsel"  'OL_DIA_Diawechsel
         TransitionAutomaticPreview.UnCheck
         '///+ -  click button 'Effects' ///'
         TransitionApplyToSelectedSlide.typekeys "<pagedown>"
         Sleep (1)
         '///+ -  in the listbox 'speed' select the i. one from the top ///'
         TransitionSpeed.Select i
         '///+ -  Select an effect-group from the listbox ///'
         TransitionApplyToSelectedSlide.typekeys "<home>"
         for q= 1 to (i+2)
            Sleep (2)
            TransitionApplyToSelectedSlide.typekeys "<down>"
            Sleep (2)
         next q
         sEffect(i) = TransitionApplyToSelectedSlide.GetSelText
         '///+ -  click button 'Extras' ///'
         Sleep (2)
         select case i
            case 1: '///+ - + click button 'Automatic Transition' ///'
                    TransitionAutomaticallyAfter.Check
                    '///+ -  + +  Set Time to '00:00:01' OR ///'
                    TransitionAutomaticallyAfterTime.ToMin 'SetText "1" '"00:00:01"
                    TransitionAutomaticallyAfterTime.More
		            TransitionApplyToSelectedSlide.typekeys "<DOWN>"
                    TransitionApplyToSelectedSlide.typekeys "<UP>"		    
                    TransitionAutomaticallyAfter.Check
                    Tasks.TypeKeys "<TAB><TAB><TAB>"
                    TimerValue = TransitionAutomaticallyAfterTime.GetText
		    printlog "   TimerValue is now: " + TimerValue
            case else: '///+ - + click button 'SemiAutomatic Transition' OR ///'
                    TransitionOnMouseClick.Check
         end select
       Sleep (3)
   next i
   Printlog "assigned transitions to all slides"

   Kontext "DocumentImpress"
   Sleep (3) 
   '/// SlideShow->Slide Show Settings... ///'
   SlideShowPresentationSettings
      Kontext "Bildschirmpraesentation"
      WaitSlot (2000)
      '/// check Radio button 'Type': 'Window' ///'
      Fenster.Check
   '/// close dialog 'Slide Show' ///'
   Bildschirmpraesentation.OK
   WaitSlot (5000)
   Kontext "DocumentImpress"
   Printlog "'/// SlideShow->Slide Show ///'"
   SlideShowSlideshow
   '/// wait 10 sec ///'
   sleep (10) 'WaitSlot (10000)
   Kontext "DocumentPresentation"
   '/// type key [ESCAPE] ///'
   DocumentPresentation.TypeKeys "<ESCAPE>"
   WaitSlot (3000)
   try
      '/// type key [F11] to open the stylist (to check ending of presenation) ///'
      if lcase(gPlatform) = "osx" then
          hTypekeys "<mod1 t>"
      else
          hTypeKeys "<F11>"
      endif
      Sleep (3)
      Kontext "Gestalter"
      Sleep (3)
      '/// close Stylist ///'
      Gestalter.Close
      Printlog "- Slideshow worked"
   catch
      Kontext "DocumentPresentation"
      DocumentPresentation.TypeKeys "<ESCAPE>"
      Warnlog "slideshow somehow didn't end :-("
      Sleep (2)
   endcatch

   '/// close the navigator///'
   Kontext "Navigator"
   if Navigator.Exists then Navigator.Close
   Printlog "'///+ check the settings in SlideView in the Dia object bar ///'"
   for i = 1 to 3
      Printlog " --------------------- " + i + " ---------------------------"
      '///+ -  open navigator with shortcutkey [MOD1 SHIFT F5] ///'
      hTypeKeys "<MOD1 SHIFT F5>"
         sleep (3)
         Kontext "NavigatorDraw"
         '///+ -  select the next slide in the list ///'
         Liste.Select i
         '///+ - press key [return] ///'
         Liste.TypeKeys "<return>"
         Kontext "Navigator"
      '///+ -  close Navigator ///'
      Navigator.Close
      '///+ - View->Master View->Slides View ///'
'      ViewWorkspaceSlidesView
      Sleep (2)
      Kontext "Tasks" 'SlideViewObjectbar"
      '///+ - check 'speed' ///'
      if Tasks.Exists = TRUE AND Tasks.IsVisible = TRUE then
         x = TransitionSpeed.GetSelIndex  'TransitionSpeed
         if (x <> i) then
            Warnlog "Speed changed :-( is: '" + x + "' should be: '" + i + "'"
         end if
         '///+ - check 'Transition' ///'
         x = TransitionApplyToSelectedSlide.GetSelIndex 'Diawechsel.GetSelIndex
         if (x <> (i+3)) then
            Warnlog "Transition changed :-( is: '" + x + "' should be: '" + (i+3) + "'"
            sTemp = TransitionAutomaticallyAfterTime.GetText
            if ((sTemp <> TimerValue)) then  '"00:00:01"  '(i = 3) AND 
               Warnlog "Time is different :-( is: '" + sTemp + "' should: '" + TimerValue + "'"  '"00:00:01"
            end if
         end if

         '///+ - check 'effect group' ///'
         sTemp = TransitionApplyToSelectedSlide.GetSelText
         x = inStr (sTemp,sEffect(i))
         Printlog " Effect tried :-) '" + x + "'"
         if (x < 1) then
            printlog "the string '" + sTemp + "' should contain the words(s): '" + sEffect(i) + "'"
         end if
'         ViewWorkspaceDrawingView
      else
         warnlog "   SlideViewObjectbar doesnt exists or isnt visible"
      end if
   next i
   '/// save file as presentation with name '"user\\work\\diashow")' ///'

    hFileSaveAsWithFilterKill ((sFile + ".odp") , "impress8" )
    
   Printlog "   saved presentation: '" + sFile + "'"  '." + gImpressFilter + "
   '/// close application///'
   hCloseDocument ()
   WaitSlot (10000)
   '/// load file again ///'
   hFileOpen (sFile + ".odp")
   WaitSlot (5000)
   Printlog "   '/// check setings for every slide in the transition dialog ///'"
   '///+ for all 3 slides do: ///'
   for i = 1 to 3
      Printlog " --------------------- " + i + " ---------------------------"
      '///+ -  open navigator with shortcutkey [MOD1 SHIFT F5] ///'
      hTypeKeys "<MOD1 SHIFT F5>"
         Sleep (3)
         Kontext "NavigatorDraw"
         '///+ -  select the next slide in the list ///'
         Liste.Select i
         '///+ - press key [return] ///'
         Liste.TypeKeys "<return>"
         Kontext "Navigator"
      '///+ -  close Navigator ///'
      Navigator.Close
      '///+ -  select the grafik with the key [TAB] ///'
      hTypeKeys ("<tab>")
      '///+ -  SlideShow->SlideTransition ///'
      SlideShowSlideTransition
         Kontext "Tasks"
         Sleep (1)
         '///+ - check 'speed' ///'
         x = TransitionSpeed.GetSelIndex
         if (x <> i) then
            Warnlog "   Speed changed :-( is: '" + x + "' should: '" + i + "'"
         end if
      '///+ - check 'effect group' ///'
         x = TransitionApplyToSelectedSlide.GetSelIndex
         if (x <> (i+3)) then
            Warnlog "   Effect changed :-( is: '" + x + "' should: '" + (i+3) + "'"
         end if
         sleep (1)
         select case i
            case 1: '///+ - click button 'Automatic Transition' ///'
                    if (TransitionAutomaticallyAfter.IsEnabled) then
                       '///+ -  + +  Check Time '1' OR ///'
                       sTemp = TransitionAutomaticallyAfterTime.GetText
                       if (sTemp <> TimerValue) then
                          Warnlog "   Time is different :-( is: '" + sTemp + "' should be: '" + TimerValue + "'" '"00:00:01"
                       end if
                    else
                       Warnlog "   button 'Automatic Transition' not pressed ?"
                    end if
            case 2: if (TransitionAutomaticallyAfter.IsChecked <> FALSE) then
                       '///+ - + check button 'SemiAutomatic Transition' OR ///'
                       Warnlog "   button 'Automatic Transition' is pressed ?"
                    end if
            case 3: if (TransitionAutomaticallyAfter.IsChecked <> FALSE) then
                      '///+ - + click button 'Manual Transition' ///'
                       Warnlog "   button 'Automatic Transition' is pressed ?"
                    end if
         end select
      Sleep (3)
   next i
   TransitionAutomaticPreview.Check
   '/// Set "Start with current page" back to default = on, in ToolsOptions ///'
   setStartCurrentPage(TRUE)
   Kontext "Navigator"
   if Navigator.exists then
      '///+ close navigator ! ///'
      Navigator.Close
   else
      printlog "Navigator: NOT available"
   end if
   '/// Close the document ///'
   call hCloseDocument
   
endcase 'tSlideShowSlideTransition

'-------------------------------------------------------------------------------------
testcase tSlideShowShowHideSlide

    dim saSlideNames(4) as string
    dim i as integer
    
    S1 = glLocale (2) + " 1"
    S2 = glLocale (2) + " 2"
    S3 = glLocale (2) + " 3"
    S4 = glLocale (2) + " 4"
    saSlideNames(1) = S1
    saSlideNames(2) = S2
    saSlideNames(3) = S3
    saSlideNames(4) = S4

    printlog "Open file 'graphics\\input\\diashow.odp' (Slide Show with 4 Slides)"
    hFileOpen   ConvertPath (gTesttoolPath + "graphics\required\input\diashow.odp")
    sleep (10)
    printlog "check if the document is writable"
    if fIsDocumentWritable = false then
        printlog "make the document writable and check if it's succesfull"
        if fMakeDocumentWritable = false then
            warnlog "   The document can't be make writeable. Test stopped."
            goto endsub
        endif
    endif
    Kontext "Navigator"
    if Navigator.exists then
      printlog "close navigator"
      Navigator.Close
    else
      printlog "Navigator: NOT available"
    endif
    printlog "View->Workspace->Slides View"
    ViewWorkspaceSlidesView
    sleep (1)
    Kontext "Slides"
    printlog "Selecting 2nd slide"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    printlog "Hiding 2nd slide"
    SlideShowHideSlide
    printlog "Selecting 4th slide"
    SlidesControl.TypeKeys "<PAGEDOWN>" ,2
    printlog "Hiding 4th slide"
    SlideShowHideSlide
    
    sleep (2)
    printlog "View->Workspace->Drawing View"
    ViewWorkspaceDrawingView
    kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<HOME>"
    sleep (2)
    printlog "SlideShow->Slide Show Settings..."
    SlideShowPresentationSettings
    Kontext "Bildschirmpraesentation"
    sleep (2)
    printlog "check Radio button 'Type': 'Window'"
    Fenster.Check
    sleep (1)
    printlog "close dialog 'Slide Show' with OK "
    Bildschirmpraesentation.OK
    printlog "start the slideshow with 'Slide Show->Slide Show'"
    SlideShowSlideshow
    sleep (1)
    kontext "DocumentPresentation"
    printlog "type the key [MOD1 SHIFT F5] to open the navigator" '--------------------------------------------------------------------------
    DocumentPresentation.TypeKeys "<MOD1 SHIFT F5>"
    sleep (1)
    kontext "NavigatorDraw"
    for i = 1 to 2
        if ((i mod 2) = 1) then ' for all odd numbers (ungerade) do
            printlog "Getting slide names"
            fGetSlideName(saSlideNames(i))
            kontext "DocumentPresentation"
            DocumentPresentation.TypeKeys "<pagedown>"
            sleep (3)
        endif
        Kontext "NavigatorDraw"
        if NavigatorDraw.exists (5) then
            sleep 1
            printlog "check in list, if the page changed"
            if ((i mod 2) = 1) then ' for all odd numbers (ungerade) do
                if i = Liste.GetSelIndex then
                    qaerrorlog "the Navigator didn't update!"
                endif
            endif
        else
            warnlog "   Navigator doesn't exist?!?!?"
        endif
        sleep (1)
    next i
    sleep (3)
    Kontext "DocumentPresentation"
    DocumentPresentation.TypeKeys "<pagedown>"
    sleep (2)
    DocumentPresentation.TypeKeys "<pagedown>"
    sleep (5)
    try
        DocumentPresentation.MouseDown 50,50
        DocumentPresentation.MouseUp 50,50 'This one works to recognice the not ending.
        warnlog "   Presentation did NOT END!!!"
    catch
        printlog "   ended right."
    endcatch
    sleep (2)
    Kontext "Navigator"
    if Navigator.exists then
        printlog "close navigator !"
        Navigator.Close
    else
        printlog "Navigator: NOT available"
    endif
    Call hCloseDocument
    
endcase 'tSlideShowShowHideSlide

'------------------------------------------------------------------------------
