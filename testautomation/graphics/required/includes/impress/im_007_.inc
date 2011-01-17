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
'* short description : Impress Required Test Library (7)
'*
'\*****************************************************************

sub im_007_

    call tSlideShowSlideShow()
    call tSlideShowRehearseTimings()
    call tSlideShowSlideShowSettings()
    call tSlideShowCustomSlideShow()
    call tSlideShowSlideTransition()
    call tSlideShowShowHideSlide()
    call tSlideShowCustomAnimation()
    call tSlideShowInteraction()
    call tSlideShowAnimation()

end sub

'*******************************************************************************

testcase tSlideShowSlideShow

    printlog "open application "
    Call hNewDocument
    printlog "Slide Show->Slide Show Settings "
    SlideShowPresentationSettings
    Kontext "Bildschirmpraesentation"
    printlog "check if 'type' 'default' is checked, it has to be the default !"
    if standard.IsChecked then
        if LogoAnzeigen.isEnabled then
            Warnlog "'Show Logo' is Enabled :-("
        endif
    else
        Warnlog "type 'default' is not checked as default :-("
    endif
    printlog "check checkbox 'Window' "
    Fenster.Check
    Printlog "-  Presentation in window mode is checked"
    printlog "close dialog with OK 'Slide Show' "
    Bildschirmpraesentation.Ok
    sleep 3
    printlog "Slide Show->Slide Show "
    SlideShowSlideshow
    Sleep 5
    try
        Kontext "DocumentPresentation"
        sleep 3
        printlog "Press <Esc> to leave presentation mode "
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
    printlog "close application "
    Call hCloseDocument
endcase 'tSlideShowSlideShow

'-------------------------------------------------------------------------------

testcase tSlideShowRehearseTimings

    goto endsub
    printlog "open application "
    Call hNewDocument
    printlog "Slide Show->Slide Show Settings "
    SlideShowPresentationSettings
    Kontext "Bildschirmpraesentation"
    printlog "check checkbox 'Window' "
    Fenster.Check
    printlog "close dialog with OK 'Slide Show' "
    Bildschirmpraesentation.Ok
    printlog "Slide Show->Rehearse Timings "
    SlideShowRehearseTimings
    sleep 2
    Kontext "DocumentPresentation"
    printlog "Press <Esc> to leave presentation mode "
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
    printlog "close application "
    Call hCloseDocument
endcase 'tSlideShowRehearseTimings

'-------------------------------------------------------------------------------

testcase tSlideShowSlideShowSettings
    printlog "open application "
    Call hNewDocument
    sleep 2
    printlog "Slide Show->Slide Show Settings "
    SlideShowPresentationSettings
    Kontext "Bildschirmpraesentation"
    call Dialogtest (Bildschirmpraesentation)
    printlog "check checkbox in section 'Range' - 'From: "
    AbDia.Check
    printlog "select the 3rd item from the top from listbox 'From:' "
    AbDiaName.GetSelText
    printlog "check checkbox 'All Slides' "
    AlleDias.Check
    '"      RangeCustomSlideShow         ' gets tested in tSlideShowCustomSlideShow "
    '         IndividuellePraesentationName
    printlog "check checkbox 'window'"
    Fenster.Check
    printlog "check checkbox 'default' "
    Standard.Check
    printlog "check check box 'Auto' -> implies looping of slideshow in fullscreen mode"
    Auto.Check
    printlog "set duration of pause to '00:00:05' "
    Zeit.GetText
    printlog "check check box 'Show logo' "
    LogoAnzeigen.Check
    printlog "check checkbox 'Change slides maually'"
    DiawechselManuel.Check
    printlog "check checkbox 'Mouse pointer as pen'"
    MauszeigerAlsStift.Check
    printlog "UNcheck checkbox 'Mouse pointer visible'"
    MauszeigerSichtbar.UnCheck
    printlog "check checkbox 'Navigator visible'"
    NavigatorSichtbar.Check
    printlog "UNcheck checkbox 'animations allowed'"
    AnimationenZulassen.UnCheck
    printlog "UNcheck checkbox 'Change slides by clicking on background'"
    DiaWechselAufHintergrund.UnCheck
    printlog "check checkbox 'Presentation always on top'"
    PraesentationImmerImVordergrund.Check
    printlog "cancel dialog 'Slide Show' "
    Bildschirmpraesentation.Cancel
    printlog "close application "
    Call hCloseDocument

endcase 'tSlideShowSlideShowSettings
'-------------------------------------------------------------------------------
testcase tSlideShowCustomSlideShow

    printlog "open application "
    Call hNewDocument
    sleep 2
    printlog "Slide Show->Custom Slide Show "
    SlideShowCustomSlideshow
    Kontext "IndividuellePraesentation"
    call Dialogtest (IndividuellePraesentation)
    printlog "click button 'New' "
    Neu.Click
    Kontext "IndividuellePraesentationDefinieren"
    Call DialogTest (IndividuellePraesentationDefinieren)
    printlog "select the first entry in the list 'Existing Slides' "
    SeitenPraesentation.Select 1
    printlog "click button '>>' "
    Hinzufuegen.Click
    printlog "close dialog 'Define Custom Slide Show' with OK "
    IndividuellePraesentationDefinieren.OK
    Kontext "IndividuellePraesentation"
    printlog "click button 'Copy' "
    Kopieren.Click
    printlog "click button 'Delete' "
    Loeschen.Click
    printlog "click button 'Edit' "
    Bearbeiten.Click
    Kontext "IndividuellePraesentationDefinieren"
    printlog "select 1st entry in the list 'Selected Slides' "
    SelectedSlides.Select 1
    printlog "click button '<<' "
    Entfernen.Click
    printlog "close dialog 'Define Custom Slide Show' with Cancel "
    IndividuellePraesentationDefinieren.Cancel
    Kontext "IndividuellePraesentation"
    printlog "check checkbox 'Use Custom Slide Show' "
    IndividuellePraesentationBenutzen.Check
    sleep 1
    printlog "+ UNcheck checkbox 'Use Custom Slide Show' "
    IndividuellePraesentationBenutzen.UnCheck
    printlog "click button 'Start...' "
    Starten.Click
    sleep 5
    printlog "press key [space] 2 times "
    kontext "DocumentPresentation"
    DocumentPresentation.TypeKeys "<space>"
    sleep 1
    DocumentPresentation.TypeKeys "<space>"
    sleep 1
    printlog "close dialog 'Custom Slide Shows' "
    '   IndividuellePraesentation.Close ' slide show ends dialog !
    printlog "Slide Show->Slide Show Settings "
    try
        SlideShowPresentationSettings
    catch
        warnlog "Presentation did not end :-("
        DocumentPresentation.TypeKeys "<escape>"
    endcatch
    Kontext "Bildschirmpraesentation"
    if Bildschirmpraesentation.exists (5) then
        printlog "check checkbox 'Custom Slide Show' "
        RangeCustomSlideShow.Check
        printlog "check: '" + IndividuellePraesentationName.GetSelText + "'"
        printlog "cancel dialog 'Slide Show' "
        Bildschirmpraesentation.Cancel
    else
        warnlog "Dialog not open? SlideShowPresentationSettings"
    endif
    printlog "Slide Show->Custom Slide Show "
    SlideShowCustomSlideshow
    Kontext "IndividuellePraesentation"
    if (IndividuellePraesentation.exists (5)) then
        printlog "click button 'Delete' "
        Loeschen.Click
        printlog "close dialog 'Custom Slide Shows' "
        IndividuellePraesentation.Close
    else
        warnlog "Dialog not open? SlideShowCustomSlideshow"
    endif
    printlog "close application "
    Call hCloseDocument
endcase 'tSlideShowCustomSlideShow

'------------------------------------------------------------------------------

testcase tSlideShowSlideTransition
    dim i as integer
    dim a as integer
    dim iCount as integer

    printlog "open application "
    Call hNewDocument
    printlog "create rectangle "
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    sleep 1
    printlog "Slide Show->Slide Transition "
    SlideShowSlideTransition
    sleep 2
    printlog "The 'Slide Transition' in the right 'Tasks' Pane has to come up "
    Kontext "Tasks"
    printlog "Select the second entry from teh Listbox 'Applay to selected slides' "
    TransitionApplyToSelectedSlide.select (2)
    sleep 5 ' takes some time, until it is run
    Printlog "Count of effects : "+TransitionApplyToSelectedSlide.GetItemCount
    Printlog "Count of Speeds  : "+TransitionSpeed.GetItemCount
    iCount = TransitionSound.GetItemCount
    Printlog "Count of Sounds  : " + iCount

    printlog "One Entry of the Listbox 'Sound' is 'Other sound...', select it "
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
    printlog "The dialog 'Open' comes up"
    sleep 1
    kontext "OeffnenDlg"
    if (OeffnenDlg.exists (5)) then
        printlog "Read all entries in Listbox 'File type' "
        for i = 1 to Dateityp.getItemCount
            printlog i + ":" + Dateityp.getItemText(i)
        next i
        printlog "cancel dialog 'Open' "
        OeffnenDlg.cancel
    else
        warnlog "Impress:Tasks Pane:Slide Transition:Sound:Other sound... disdn't bring up teh File Open Dialog!"
    endif
    kontext "Tasks"
    sleep (2)
    printlog "check checkbox 'Automatically after' "
    TransitionAutomaticallyAfter.check
    sleep (2)
    printlog "press key 'Page Up' in box "
    TransitionAutomaticallyAfterTime.typeKeys "<PageUp>"
    sleep 9
    printlog "check the standard checkbox 'On mouse click' "
    TransitionOnMouseClick.check
    sleep (2)
    printlog "press button 'Apply to All Slides' "
    TransitionApplyToAllSlides.click
    sleep (2)
    printlog "press button 'Play' "
    TransitionPlay.click
    sleep 10
    printlog "press button 'Slide Show' "
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

    printlog "uncheck and check Checkbox 'Automatic Preview' "
    printlog "default is checked "
    if (NOT TransitionAutomaticPreview.isChecked) then
        warnlog "Impress:Tasks Pane:Slide Transition: Automatic preview has to be checked by default, wasn't!"
    endif
    sleep (2)
    TransitionAutomaticPreview.unCheck
    sleep (2)
    TransitionAutomaticPreview.Check
    printlog "close application "
    Call hCloseDocument

endcase 'tSlideShowSlideTransition

'-------------------------------------------------------------------------------

testcase tSlideShowShowHideSlide

    printlog "open application "
    Call hNewDocument
    printlog "create rectangle "
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    printlog "View->Master View->Slides View "
    ViewWorkspaceSlidesView
    sleep 1
    printlog "Slide Show->Hide Slide "
    SlideShowHideSlide
    sleep 1
    printlog "Slide Show->Show Slide "
    SlideShowShowSlide
    printlog "close application "
    Call hCloseDocument
endcase 'tSlideShowShowHideSlide

'-------------------------------------------------------------------------------

testcase tSlideShowAnimation
    printlog "open application "
    Call hNewDocument
    sleep 1
    printlog "create rectangle "
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    sleep 1
    printlog "Insert ->Animated image "
    Opl_SD_EffekteZulassen
    Kontext "Animation"
    sleep 1
    printlog "click button 'Apply Object' "
    BildAufnehmen.Click  'BildAufnehmen
    printlog "click button 'Create' "
    Erstellen.Click
    sleep 1
    printlog "Select 1st entry from top in 'Alignment' "
    Anpassung.Select 1
    sleep 1
    printlog "click button 'Create' "
    Erstellen.Click
    sleep 1
    printlog "click button 'Apply Objects Individually' "
    AlleAufnehmen.Click
    sleep 1
    printlog "click button 'First Image' "
    ErstesBild.Click
    sleep 1
    printlog "click button 'Last Image' "
    LetztesBild.Click
    sleep 1
    printlog "click button 'BAckwards' "
    Rueckwaerts.Click
    sleep 1
    printlog "click button 'Play' "
    Abspielen.Click
    sleep 1
    printlog "click in Number field 'Image Number' Less - More "
    AnzahlBilder.Less
    sleep 1
    AnzahlBilder.More
    sleep 1
    printlog "check 'Bitmap Object' "
    AnimationsgruppeBitmapobjekt.Check
    sleep 1
    printlog "Type '10' into the field 'Duration' "
    AnzeigedauerProBild.SetText "10"
    printlog "click button 'Play' "
    Abspielen.Click
    printlog "wait 5 seconds "
    sleep 5
    printlog "click button 'Stop' "
    try
        Stopp.Click
    catch
        warnlog "Stopbutton doesn't work"
    endcatch
    sleep 1
    printlog "Select 1st entry from top in 'Loop Count' "
    AnzahlDurchlaeufe.Select 1
    sleep 1
    printlog "click button 'Delete Current Image' "
    BildLoeschen.Click
    sleep 1
    printlog "check 'Group Object' "
    AnimationsgruppeGruppenobjekt.Check
    sleep 1
    printlog "click button 'Delete All Images' "
    AlleLoeschen.Click
    kontext "Messagebox"
    printlog "there has to be a messagebox 'Really delete?' say YES!; else ERROR "
    if Messagebox.exists (5) then
        Messagebox.YES
    else
        warnlog "No one cares about my data :-( No one asked if all shall be deleted :-( "
    endif
    sleep 1
    kontext "Animation"
    printlog "close dialog 'Animation' "
    Animation.Close
    printlog "close application "
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

    printlog "open application "
    Call hNewDocument
    sleep 2
    printlog "create rectangle "
    Call hRechteckErstellen (10, 10, 20, 20)
    sleep 3
    printlog "Slide Show->Interaction "
    SlideShowInteraction
    Kontext "TabInteraktion"
    Call DialogTest (TabInteraktion, 1)
    printlog "+ Select 6th entry from top in 'Action at mouse click' : 'Go to page or object' "
    AktionBeiMausklick.select 6
    Printlog AktionBeiMausklick.GetSelText + " chosen"
    Call DialogTest (TabInteraktion, 2)
    printlog "+ click button 'Find' "
    sleep 1
    suchen.click
    Kontext "TabInteraktion"
    printlog "Select 7th entry from top in 'Action at mouse click' : 'Go to document' "
    sleep 1
    AktionBeiMausklick.select 7
    sleep 1
    Printlog AktionBeiMausklick.GetSelText + " chosen"
    Kontext "TabInteraktion"
    Call DialogTest (TabInteraktion, 3)
    printlog "+ click button 'Browse...' "
    Durchsuchen.click
    sleep 1
    kontext "OeffnenDlg"
    call Dialogtest (OeffnenDlg)
    printlog "+ cancel dialog 'open' "
    OeffnenDlg.cancel
    Kontext "TabInteraktion"
    sleep 1
    printlog "Select 9th entry from top in 'Action at mouse click' : 'Play Sound' "
    AktionBeiMausklick.select 8
    Printlog AktionBeiMausklick.GetSelText + " chosen"
    Call DialogTest (TabInteraktion, 4)
    printlog "+ click button 'Browse...' "
    Durchsuchen.click
    sleep 1
    Kontext "OeffnenDlg"
    Call dialogTest (OeffnenDlg)
    printlog "cancel dialog 'open' "
    OeffnenDlg.Cancel
    sleep 1
    Kontext "TabInteraktion"
    printlog "Select 8th entry from top in 'Action at mouse click' : 'Run Program' "
    AktionBeiMausklick.select 9
    Printlog AktionBeiMausklick.GetSelText + " chosen"
    Call DialogTest (TabInteraktion, 7)
    Kontext "TabInteraktion"
    printlog "+ click button 'Browse...' "
    Durchsuchen.Click
    sleep 1
    Kontext "OeffnenDlg"
    Call dialogTest (OeffnenDlg)
    printlog "+ cancel dialog 'open' "
    OeffnenDlg.Cancel
    sleep 1
    printlog "Select 9th entry from top in 'Action at mouse click' : 'Run Macro' "
    Kontext "TabInteraktion"
    AktionBeiMausklick.select 10
    Printlog AktionBeiMausklick.GetSelText + " chosen"
    sleep 3
    Call DialogTest (TabInteraktion, 6)
    printlog "+ click button 'Browse...' "
    Durchsuchen.Click
    sleep 1
    Kontext "ScriptSelector"
    sleep 1
    Call DialogTest ( ScriptSelector, 1)
    sleep 1
    printlog "+ cancel dialog 'ScriptSelector' "
    ScriptSelector.Cancel
    sleep 1
    printlog "Select 10th entry from top in 'Action at mouse click' : 'Exit Presentation' "
    Kontext "TabInteraktion"
    AktionBeiMausklick.select 11
    Printlog AktionBeiMausklick.GetSelText + " chosen"
    Call DialogTest (TabInteraktion, 7)
    Kontext "TabInteraktion"
    printlog "close dialog 'Interaction' "
    TabInteraktion.Close
    sleep 2
    printlog "close application "
    Call hCloseDocument

endcase 'tSlideShowInteraction