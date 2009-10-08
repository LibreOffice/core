'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: im_007_.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:43:01 $
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
'* Owner : wolfram.garten@sun.com
'*
'* short description : Impress Required Test Library (7)
'*
'\*****************************************************************

' this menue is only in impress availble
testcase tSlideShowSlideShow

'/// open application ///'
 Call hNewDocument
'/// Slide Show->Slide Show Settings ///'
    SlideShowPresentationSettings
       Kontext "Bildschirmpraesentation"
  '/// check if 'type' 'default' is checked, it has to be the default !///'
       if standard.IsChecked then
          if LogoAnzeigen.isEnabled then
             Warnlog "'Show Logo' is Enabled :-("
          endif
       else
          Warnlog "type 'default' is not checked as default :-("
       endif
  '/// check checkbox 'Window' ///'
       Fenster.Check
       Printlog "-  Presentation in window mode is checked"
  '/// close dialog with OK 'Slide Show' ///'
    Bildschirmpraesentation.Ok
    sleep 3
'/// Slide Show->Slide Show ///'
    SlideShowSlideshow
    Sleep 5
    try
       Kontext "DocumentPresentation"
       sleep 3
'/// Press <Esc> to leave presentation mode ///'
       DocumentPresentation.Typekeys ("<Escape>")
       Sleep 3
       Kontext "DocumentImpress"
       DocumentImpress.MouseDoubleClick ( 50, 50 )
       Sleep 3
    catch
'       FileClose
       warnlog "had to catch <the ball> :-( "
'   	 Kontext "Messagebox"
'       if Messagebox.Exists (5) Then Messagebox.No
'       Kontext
'       sleep (12)
'       start sAppExe$
'       sleep (6)
'       Kontext
'       if Office.Exists(2) then Resetapplication
'       Warnlog "Slide didn't end, application wasn't in document edit mode"
    endcatch
'/// close application ///'
 Call hCloseDocument

 endcase 'tSlideShowSlideShow
'-------------------------------------------------------------------------------
testcase tSlideShowRehearseTimings

    goto endsub
    '/// open application ///'
    Call hNewDocument
    '/// Slide Show->Slide Show Settings ///'
    SlideShowPresentationSettings
    Kontext "Bildschirmpraesentation"
    '/// check checkbox 'Window' ///'
    Fenster.Check
    '/// close dialog with OK 'Slide Show' ///'
    Bildschirmpraesentation.Ok
    '/// Slide Show->Rehearse Timings ///'
    SlideShowRehearseTimings
    sleep 2
    Kontext "DocumentPresentation"
    '/// Press <Esc> to leave presentation mode ///'
      if DocumentPresentation.Exists (5) then
         DocumentPresentation.Typekeys ("<Escape>")
      else
         Warnlog "SlideShowRehearseTimings mode not accessible"
      endif
      Sleep 3
      if DocumentPresentation.Exists then ' the kontext hasnt to be available, else ERROR
         DocumentPresentation.TypeKeys "<ESCAPE>" ' => I mustn't be here ever !
         Warnlog "- Slide show mode should have ended"
      else
         printlog "SlideShowRehearseTimings mode not accessible :-) "
      end if
      sleep 3
      try
         Kontext "DocumentImpress"
         DocumentImpress.MouseDoubleClick ( 50, 50 )

         sleep 3
      catch
         warnlog "Had to catch <the ball> :-( "
      endcatch
    sleep 3
'/// close application ///'
 Call hCloseDocument

 endcase 'tSlideShowRehearseTimings
'-------------------------------------------------------------------------------
testcase tSlideShowSlideShowSettings

'/// open application ///'
 Call hNewDocument
   sleep 2
'/// Slide Show->Slide Show Settings ///'
   SlideShowPresentationSettings
      Kontext "Bildschirmpraesentation"
      call Dialogtest (Bildschirmpraesentation)
      '/// check checkbox in section 'Range' - 'From: ///'
      AbDia.Check
         '/// select the 3rd item from the top from listbox 'From:' ///'
         AbDiaName.GetSelText
      '/// check checkbox 'All Slides' ///'
      AlleDias.Check
'///'      RangeCustomSlideShow         ' gets tested in tSlideShowCustomSlideShow ///'
'         IndividuellePraesentationName
      '///<b> check checkbox 'window' </b>///'
      Fenster.Check
      '/// check checkbox 'default' ///'
      Standard.Check
      '///<b> check check box 'Auto' -> implies looping of slideshow in fullscreen mode </b>///'
      Auto.Check
         '/// set duration of pause to '00:00:05' ///'
         Zeit.GetText
         '/// check check box 'Show logo' ///'
         LogoAnzeigen.Check
      '///<b> check checkbox 'Change slides maually' </b>///'
      DiawechselManuel.Check
      '///<b> check checkbox 'Mouse pointer as pen' </b>///'
         MauszeigerAlsStift.Check
      '///<b> UNcheck checkbox 'Mouse pointer visible' </b>///'
      MauszeigerSichtbar.UnCheck
      '///<b> check checkbox 'Navigator visible' </b>///'
      NavigatorSichtbar.Check
      '///<b> UNcheck checkbox 'animations allowed' </b>///'
      AnimationenZulassen.UnCheck
      '///<b> UNcheck checkbox 'Change slides by clicking on background' </b>///'
      DiaWechselAufHintergrund.UnCheck
      '///<b> check checkbox 'Presentation always on top' </b>///'
      PraesentationImmerImVordergrund.Check
   '/// cancel dialog 'Slide Show' ///'
   Bildschirmpraesentation.Cancel
'/// close application ///'
 Call hCloseDocument
 
endcase 'tSlideShowSlideShowSettings
'-------------------------------------------------------------------------------
testcase tSlideShowCustomSlideShow

   '/// open application ///'
   Call hNewDocument
   sleep 2
   '/// Slide Show->Custom Slide Show ///'
   SlideShowCustomSlideshow
      Kontext "IndividuellePraesentation"
      call Dialogtest (IndividuellePraesentation)
      '/// click button 'New' ///'
      Neu.Click
         Kontext "IndividuellePraesentationDefinieren"
         Call DialogTest (IndividuellePraesentationDefinieren)
         '/// select the first entry in the list 'Existing Slides' ///'
         SeitenPraesentation.Select 1
         '/// click button '>>' ///'
         Hinzufuegen.Click
         '/// close dialog 'Define Custom Slide Show' with OK ///'
      IndividuellePraesentationDefinieren.OK
      Kontext "IndividuellePraesentation"
      '/// click button 'Copy' ///'
      Kopieren.Click
      '/// click button 'Delete' ///'
      Loeschen.Click
      '/// click button 'Edit' ///'
      Bearbeiten.Click
         Kontext "IndividuellePraesentationDefinieren"
         '/// select 1st entry in the list 'Selected Slides' ///'
         SelectedSlides.Select 1
         '/// click button '<<' ///'
         Entfernen.Click
         '/// close dialog 'Define Custom Slide Show' with Cancel ///'
      IndividuellePraesentationDefinieren.Cancel
      Kontext "IndividuellePraesentation"
      '/// check checkbox 'Use Custom Slide Show' ///'
      IndividuellePraesentationBenutzen.Check
      sleep 1
      '///+ UNcheck checkbox 'Use Custom Slide Show' ///'
      IndividuellePraesentationBenutzen.UnCheck
      '/// click button 'Start...' ///'
      Starten.Click
         sleep 5
         '/// press key [space] 2 times ///'
         kontext "DocumentPresentation"
         DocumentPresentation.TypeKeys "<space>"
         sleep 1
         DocumentPresentation.TypeKeys "<space>"
         sleep 1
      '/// close dialog 'Custom Slide Shows' ///'
'   IndividuellePraesentation.Close ' slide show ends dialog !
   '/// Slide Show->Slide Show Settings ///'
   try
      SlideShowPresentationSettings
   catch
      warnlog "Presentation did not end :-("
      DocumentPresentation.TypeKeys "<escape>"
   endcatch
      Kontext "Bildschirmpraesentation"
      if Bildschirmpraesentation.exists (5) then
         '/// check checkbox 'Custom Slide Show' ///'
         RangeCustomSlideShow.Check
         printlog "check: '" + IndividuellePraesentationName.GetSelText + "'"
         '/// cancel dialog 'Slide Show' ///'
         Bildschirmpraesentation.Cancel
      else
         warnlog "Dialog not open? SlideShowPresentationSettings"
      endif
   '/// Slide Show->Custom Slide Show ///'
   SlideShowCustomSlideshow
      Kontext "IndividuellePraesentation"
      if (IndividuellePraesentation.exists (5)) then
         '/// click button 'Delete' ///'
         Loeschen.Click
         '/// close dialog 'Custom Slide Shows' ///'
         IndividuellePraesentation.Close
      else
         warnlog "Dialog not open? SlideShowCustomSlideshow"
      endif
   '/// close application ///'
   Call hCloseDocument
   
endcase 'tSlideShowCustomSlideShow
'------------------------------------------------------------------------------
testcase tSlideShowSlideTransition

    goto endsub '"#149943# - Outcommented tSlideShowSlideTransition due to bug."
    dim i as integer
    dim a as integer
    dim iCount as integer

    '/// open application ///'
    Call hNewDocument
    '/// create rectangle ///'
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    sleep 1
    '/// Slide Show->Slide Transition ///'
    SlideShowSlideTransition
    sleep 2
    '/// The 'Slide Transition' in the right 'Tasks' Pane has to come up ///'
    Kontext "Tasks"
        '/// Select the second entry from teh Listbox 'Applay to selected slides' ///'
        TransitionApplyToSelectedSlide.select (2)
        sleep 5 ' takes some time, until it is run
        Printlog "Count of effects : "+TransitionApplyToSelectedSlide.GetItemCount
        Printlog "Count of Speeds  : "+TransitionSpeed.GetItemCount
        iCount = TransitionSound.GetItemCount
        Printlog "Count of Sounds  : " + iCount

        '/// One Entry of the Listbox 'Sound' is 'Other sound...', select it ///'
        TransitionSound.typeKeys "<home>"
        i = 0
        for a = 1 to iCount
            TransitionSound.select (a)
            kontext "OeffnenDlg"
            if (OeffnenDlg.exists (5)) then
                if (0=i) then
                    ' remember when dialog came up
                    i = a
                    OeffnenDlg.cancel
                else
                    warnlog "File Open Dialog comes up a second time!"
                    OeffnenDlg.cancel
                endif
            endif
            kontext "Tasks"
            ' the Checkbox is disabled on teh first three entries: <No Sound>, <Stop previous sound>...
            if (TransitionLoopUntilNextSound.isEnabled AND (a<4)) then
                qaErrorLog "May be Language specific -> Evaluation of TBO; " + a
            endif
        next a
        TransitionSound.select (i)
        '/// The dialog 'Open' comes up///'
        sleep 1
        kontext "OeffnenDlg"
        if (OeffnenDlg.exists (5)) then
            '/// Read all entries in Listbox 'File type' ///'
            for i = 1 to Dateityp.getItemCount
                printlog "" + i + ":" + Dateityp.getItemText(i)
            next i
            '/// cancel dialog 'Open' ///'
            OeffnenDlg.cancel
        else
            warnlog "Impress:Tasks Pane:Slide Transition:Sound:Other sound... disdn't bring up teh File Open Dialog!"
        endif
        kontext "Tasks"
        sleep (2)
        '/// check checkbox 'Automatically after' ///'
        TransitionAutomaticallyAfter.check
        sleep (2)
        '/// press key 'Page Up' in box ///'
        TransitionAutomaticallyAfterTime.typeKeys "<PageUp>"
        sleep 9
        '/// check the standard checkbox 'On mouse click' ///'
        TransitionOnMouseClick.check
        sleep (2)
        '/// press button 'Apply to All Slides' ///'
        TransitionApplyToAllSlides.click
        sleep (2)
        '/// press button 'Play' ///'
        TransitionPlay.click
        sleep 10
        '/// press button 'Slide Show' ///'
        TransitionSlideShow.click
        sleep 2
        kontext "DocumentPresentation"
        if DocumentPresentation.exists (10) then
            printlog "Presentation started :-)"
            DocumentPresentation.typeKeys "<escape>"
        else
            warnlog "Impress:Tasks Pane:Slide Transition:Slide Show button doesn't start slideshow!"
        endif
        kontext "Tasks"

        '/// uncheck and check Checkbox 'Automatic Preview' ///'
        '/// default is checked ///'
        if (NOT TransitionAutomaticPreview.isChecked) then
            warnlog "Impress:Tasks Pane:Slide Transition: Automatic preview has to be checked by default, wasn't!"
        endif
        sleep (2)
        TransitionAutomaticPreview.unCheck
        sleep (2)
        TransitionAutomaticPreview.Check
    '/// close application ///'
    Call hCloseDocument
    
endcase 'tSlideShowSlideTransition
'-------------------------------------------------------------------------------
testcase tSlideShowShowHideSlide

'/// open application ///'
 Call hNewDocument
'/// create rectangle ///'
   Call hRechteckErstellen ( 10, 10, 20, 40 )
'/// View->Master View->Slides View ///'
   ViewWorkspaceSlidesView
   sleep 1
'/// Slide Show->Hide Slide ///'
   SlideShowHideSlide
   sleep 1
'/// Slide Show->Show Slide ///'
   SlideShowShowSlide
'/// close application ///'
 Call hCloseDocument
endcase

testcase tSlideShowAnimation
'/// open application ///'
   Call hNewDocument
   sleep 1
'/// create rectangle ///'
   Call hRechteckErstellen ( 10, 10, 20, 40 )
   sleep 1
'/// Insert ->Animated image ///'
 Opl_SD_EffekteZulassen
 Kontext "Animation"
     sleep 1
'/// click button 'Apply Object' ///'
     BildAufnehmen.Click  'BildAufnehmen
'/// click button 'Create' ///'
      Erstellen.Click
     sleep 1
'/// Select 1st entry from top in 'Alignment' ///'
      Anpassung.Select 1
     sleep 1
'/// click button 'Create' ///'
      Erstellen.Click
     sleep 1
'/// click button 'Apply Objects Individually' ///'
      AlleAufnehmen.Click
     sleep 1
'/// click button 'First Image' ///'
      ErstesBild.Click
     sleep 1
'/// click button 'Last Image' ///'
      LetztesBild.Click
     sleep 1
'/// click button 'BAckwards' ///'
      Rueckwaerts.Click
     sleep 1
'/// click button 'Play' ///'
      Abspielen.Click
     sleep 1
'/// click in Number field 'Image Number' Less - More ///'
      AnzahlBilder.Less
     sleep 1
      AnzahlBilder.More
     sleep 1
'/// check 'Bitmap Object' ///'
   AnimationsgruppeBitmapobjekt.Check
     sleep 1
'/// Type '10' into the field 'Duration' ///'
      AnzeigedauerProBild.SetText "10"
'/// click button 'Play' ///'
      Abspielen.Click
'/// wait 5 seconds ///'
      sleep 5
'/// click button 'Stop' ///'
     try
        Stopp.Click
     catch
        warnlog "Stopbutton doesn't work"
     endcatch
     sleep 1
'/// Select 1st entry from top in 'Loop Count' ///'
      AnzahlDurchlaeufe.Select 1
     sleep 1
'/// click button 'Delete Current Image' ///'
      BildLoeschen.Click
     sleep 1
'/// check 'Group Object' ///'
   AnimationsgruppeGruppenobjekt.Check
     sleep 1
'/// click button 'Delete All Images' ///'
      AlleLoeschen.Click
      kontext "Messagebox"
'/// there has to be a messagebox 'Really delete?' say YES!; else ERROR ///'
      if Messagebox.exists (5) then
         Messagebox.YES
      else
         warnlog "No one cares about my data :-( No one asked if all shall be deleted :-( "
      endif
     sleep 1
     kontext "Animation"
'/// close dialog 'Animation' ///'
      Animation.Close
'/// close application ///'
   Call hCloseDocument
   
endcase 'tSlideShowShowHideSlide
'-------------------------------------------------------------------------------
testcase tSlideShowCustomAnimation

    dim bError as boolean

    printlog "open application"
    Call hNewDocument
    printlog "create textbox with text"
    Call hTextrahmenErstellen ("Test text to test text effects", 10, 10, 20, 40 )
    printlog "Slide Show->Custom Animation..."
    SlideShowCustomAnimation
        Kontext "Tasks"
        printlog "click button 'Add...'printlog "
        EffectAdd.click
        printlog "Dialog 'Custom Animation' comes up"
        kontext
        printlog "Switch to TabPage: Entrance"
        active.setPage(TabEntrance)
        kontext "TabEntrance"
        if TabEntrance.exists(5) then
            DialogTest(TabEntrance)
            printlog "select in the listbox 'Effects' the second entry"
            Effects.select(2)
            Speed.getItemCount
            AutomaticPreview.unCheck
            sleep 1
            AutomaticPreview.Check
            kontext
            
            printlog "Switch to TabPage: Emphasis"
            active.setPage(TabEmphasis)
            kontext "TabEmphasis"
            if TabEmphasis.exists(5) then
                DialogTest(TabEmphasis)
            else
                bError = true
                warnlog "Impress:Tasks Pane:Custom Animation:TabEmphasis tabPage doesn't work."
            endif
            kontext
            
            printlog "Switch to TabPage: Exit"
            active.setPage(TabExit)
            kontext "TabExit"
            if TabExit.exists(5) then
                DialogTest(TabExit)
            else
                bError = true
                warnlog "Impress:Tasks Pane:Custom Animation:TabExit tabPage doesn't work."
            endif
            kontext
            
            printlog "Switch to TabPage: Motion Paths"
            active.setPage(TabMotionPaths)
            kontext "TabMotionPaths"
            if TabMotionPaths.exists(5) then
                DialogTest(TabMotionPaths)
                Effects.select(7)
            else
                bError = true
                warnlog "Impress:Tasks Pane:Custom Animation:TabMotionPaths tabPage doesn't work."
            endif
            kontext
            '-----------------------------------------------------------------------
            'printlog "Switch to tabpage: Misc Effects"
            'active.setPage(TabMiscEffects)
            'kontext "TabMiscEffects"
            'if TabMiscEffects.exists(5) then
            '    DialogTest(TabMiscEffects)
            'else
            '    bError = true
            '    warnlog "Impress:TaskPane:CustomAnimation:TabMiscEffects tabPage does not work."
            'endif
            'kontext
            '-----------------------------------------------------------------------
            printlog "Close dialog 'Custom Animation' with 'OK'"
            TabMotionPaths.OK
            bError = false
        else
            bError = true
            warnlog "Impress:Tasks Pane:Custom Animation:Add... button didn't work."
        endif

        Kontext "Tasks"
        if (NOT bError) then
            printlog "click button 'Change...'"
            EffectChange.click
            printlog "Dialog 'Custom Animation' comes up"
            kontext
            printlog "Switch to TabPage: Entrance"
            active.setPage(TabEntrance)
            kontext "TabEntrance"
            if (NOT TabEntrance.exists(5)) then
                warnlog "Impress:Tasks Pane:Custom Animation:Change... button didn't work."
            endif
            TabEntrance.cancel
            Kontext "Tasks"
            EffectStart.getItemCount
            if EffectProperty.isEnabled then
                EffectProperty.getItemCount
            endif
            printlog "Click on button '...' (Options)"
            EffectOptions.click
            kontext "TabEffect"
            if TabEffect.exists(5) then
                dialogTest(TabEffect)
                Sound.getItemCount
                AfterAnimation.getItemCount
                printlog "switch to TabPage 'Timing'"
                Kontext
                active.setPage TabTiming
                kontext "TabTiming"
                if TabTiming.exists(5) then
                    dialogTest(TabTiming)
                    TimingStart.getItemCount
                    Delay.getText
                    Speed.getItemCount
                    Repeat.getItemCount
                    Rewind.ischecked
                    TriggerAnimate.isChecked
                    TriggerStart.isChecked
                    Shape.getItemCount
                else
                    warnlog "Impress:Tasks Pane:Custom Animation:Effect Options: Timing TabPage didn't work."
                endif
                printlog "switch to TabPage 'Timing'"
                Kontext
                active.setPage TabTextAnimation
                kontext "TabTextAnimation"
                if TabTextAnimation.exists(5) then
                    dialogTest(TabTextAnimation)
                    GroupText.getItemCount
                    AnimateAttachedShape.isChecked
                    TabTextAnimation.cancel
                else
                    warnlog "Impress:Tasks Pane:Custom Animation:Effect Options: TextAnimation TabPage didn't work."
                endif
            else
                warnlog "Impress:Tasks Pane:Custom Animation:... button didn't work."
            endif
            Kontext "Tasks"
            EffectSpeed.getItemCount
            EffectList.getItemCount
            EffectPlay.click
            printlog "Wait five seconds so the Playfunction has ended "
            sleep 5
            EffectSlideShow.click
            sleep 1
            kontext "DocumentPresentation"
            if DocumentPresentation.exists (5) then
                printlog "Presentation started :-)"
                DocumentPresentation.typeKeys "<escape>"
            else
                warnlog "Impress:Tasks Pane:Custom Animation:Slide Show button doesn't start slideshow!"
            endif
            kontext "Tasks"
            EffectAutomaticPreview.isChecked
            printlog "click button 'Remove'"
            EffectRemove.click
        endif
    printlog "close application"
    Call hCloseDocument
    
endcase 'tSlideShowCustomAnimation
'-------------------------------------------------------------------------------
testcase tSlideShowInteraction

'/// open application ///'
 Call hNewDocument
 sleep 2
'/// create rectangle ///'
   Call hRechteckErstellen (10, 10, 20, 20)
   sleep 3
'/// Slide Show->Interaction ///'
   SlideShowInteraction
      Kontext "TabInteraktion"
      Call DialogTest2 (TabInteraktion, 1)
   '///+ Select 6th entry from top in 'Action at mouse click' : 'Go to page or object' ///'
      AktionBeiMausklick.select 6
      Printlog AktionBeiMausklick.GetSelText + " chosen"
      Call DialogTest2 (TabInteraktion, 2)
   '///+ click button 'Find' ///'
      sleep 1
      suchen.click
      Kontext "TabInteraktion"
'/// Select 7th entry from top in 'Action at mouse click' : 'Go to document' ///'
      sleep 1
      AktionBeiMausklick.select 7
      sleep 1
      Printlog AktionBeiMausklick.GetSelText + " chosen"
      Kontext "TabInteraktion"
      Call DialogTest2 (TabInteraktion, 3)
   '///+ click button 'Browse...' ///'
      Durchsuchen.click
        sleep 1
         kontext "OeffnenDlg"
         call Dialogtest (OeffnenDlg)
   '///+ cancel dialog 'open' ///'
      OeffnenDlg.cancel
      Kontext "TabInteraktion"
      sleep 1
'/// Select 9th entry from top in 'Action at mouse click' : 'Play Sound' ///'
      AktionBeiMausklick.select 8
      Printlog AktionBeiMausklick.GetSelText + " chosen"
      Call DialogTest2 (TabInteraktion, 4)
   '///+ click button 'Browse...' ///'
      Durchsuchen.click
   sleep 1
      Kontext "OeffnenDlg"
      Call dialogTest (OeffnenDlg)
      '///+ cancel dialog 'open' ///'
         OeffnenDlg.Cancel
      sleep 1
     Kontext "TabInteraktion"
'/// Select 8th entry from top in 'Action at mouse click' : 'Run Program' ///'
         AktionBeiMausklick.select 9
         Printlog AktionBeiMausklick.GetSelText + " chosen"
            Call DialogTest2 (TabInteraktion, 7)
            Kontext "TabInteraktion"
   '///+ click button 'Browse...' ///'
            Durchsuchen.Click
            sleep 1
               Kontext "OeffnenDlg"
               Call dialogTest (OeffnenDlg)
   '///+ cancel dialog 'open' ///'
               OeffnenDlg.Cancel
         sleep 1
'/// Select 9th entry from top in 'Action at mouse click' : 'Run Macro' ///'
      Kontext "TabInteraktion"
      AktionBeiMausklick.select 10
        Printlog AktionBeiMausklick.GetSelText + " chosen"
      sleep 3
      Call DialogTest2 (TabInteraktion, 6)
   '///+ click button 'Browse...' ///'
           Durchsuchen.Click
           sleep 1
           Kontext "ScriptSelector"
           sleep 1
           Call DialogTest ( ScriptSelector, 1)
           sleep 1
   '///+ cancel dialog 'ScriptSelector' ///'
           ScriptSelector.Cancel
           sleep 1
'/// Select 10th entry from top in 'Action at mouse click' : 'Exit Presentation' ///'
      Kontext "TabInteraktion"
      AktionBeiMausklick.select 11
      Printlog AktionBeiMausklick.GetSelText + " chosen"
      Call DialogTest2 (TabInteraktion, 7)
      Kontext "TabInteraktion"
'/// close dialog 'Interaction' ///'
      TabInteraktion.Close
      sleep 2
'/// close application ///'
 Call hCloseDocument
 
endcase 'tSlideShowInteraction
