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
'* short description : Tests the shaddow-function on a picture
'*
'*******************************************************************
'*
' #1 tiPenginefast
'*
'\*******************************************************************

testcase tiPenginefast
    '   dim i,t,q as integer
    '   dim e as string
    dim sFileName as string

    printlog "the Presentation-Engine consists of showing the presentation, with all it's effects."
    printlog "Create a new presentation."
    Call hNewDocument
    sleep 1

    printlog "Open the test-file."
    Call hDateiOeffnen (gTesttoolpath + "graphics\required\input\allshapes.odp")

    printlog "Start the slideshow."
    CALL hTypeKeys "<F5>"
    sleep (5)
    kontext "DocumentPresentation"
    DocumentPresentation.TypeKeys "<SPACE>"
    printlog "Wait for the presentation to reach a certain moment."
    sleep (10)
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (7)
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (7)
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (7)
    printlog "Press "Space" again, to continue with slide two."
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (7)
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (7)
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (7)
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (7)
    printlog "And press "Space" again, to exit the presentation-mode."
    DocumentPresentation.TypeKeys "<SPACE>"
    FileClose

    Call hNewDocument
    sleep 1
    printlog "Open the test-file."
    Call hDateiOeffnen (gTesttoolpath + "graphics\required\input\effects.odp")
    printlog "Start the slideshow."
    CALL hTypeKeys "<F5>"
    sleep (200)
    printlog "Press Space again, to continue with slide two."
    kontext "DocumentPresentation"
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (40)
    printlog "End of first page."
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (12)
    printlog "End of second page."
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (25)
    printlog "End of third page."
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (15)
    printlog "End of fourth page."
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (2)
    printlog "End of fifth page."
    DocumentPresentation.TypeKeys "<SPACE>"
    printlog "End of sixth page."
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (5)
    printlog "End of seventh page."
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (1)
    printlog "End of eight page."
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (1)
    printlog "End of ninth page."
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (1)
    printlog "End of tenth page."
    DocumentPresentation.TypeKeys "<SPACE>"
    sleep (7)
    printlog "End of eleventh page."
    if DocumentPresentation.Exists then
        warnlog "DocumentPresentation shouldnt be visible right now."
        DocumentPresentation.TypeKeys "<SPACE>"
    else
        printlog "Presentation ended correctly."
        kontext "DocumentImpress"
    end if

    printlog "Insert a new slide."
    '   InsertSlide
    '   sleep 2
    '   hTypekeys "<Pagedown>"
    '   sleep 2

    printlog "Menu: Slideshow: Check every menu-item."

    'Exists:
    ' #1 tSlideShowSlideShow
    ' #1 tSlideShowRehearseTimings
    ' #1 tSlideShowSlideShowSettings
    ' #1 tSlideShowCustomSlideShow
    ' #1 tSlideShowSlideTransition
    ' #1 tSlideShowShowHideSlide

    '   Slideshow menu 1 - Slide show
    '   Slideshow menu 2 - Slide show settings
    '   Slideshow menu 3 - Rehearse timings
    '   Slideshow menu 4 - Interaction
    '   Slideshow menu 6 - Slide Transition
    '   Slideshow menu 7 - Show / Hide slide
    '   Slideshow menu 8 - Custom Slide show


    'Printlog " Here starts the test "

    'Presentation - Start / Stop. Different ways to do it.
    'Presentation - Effects.
    'LeftWindow

    printlog "Slideshow Settings: Range: test all three alternatives."
    printlog "Test if the Slideshow-types (Default, Window, Auto) works."
    printlog "Check if the checkboxes works."
    printlog "Close dialogue."
    printlog "Add an object."
    printlog "Custom Animation: Add an Animation to the object."
    printlog "Run the slideshow."

    printlog "Change to the next effect and continue through every animation in"
    printlog "Entrance, Emphasis, Exit and Motion Paths."
    printlog "While testing - test the speed-choices for every kind of animation."
    printlog "Check if the "Change" and "Remove"-buttons."
    printlog "Check the Start, "Direction" and "Speed" variables."
    printlog "Add two effects to one object and change the order."
    printlog "Try the Play and "Slide Show"-button."
    printlog "Try the Automatic preview-button."

    printlog "Slidetransition."
    printlog "Go through every kind of effect, speed, and sound."
    printlog "Check loop until next sound."
    printlog "Check the Advance slide with mouseclick-function."
    printlog "Check the "Apply to all slides", "Play", and "Slide Show"-buttons."
    printlog "Check Automatic preview."

    printlog "Some effects doesnt have a right / left orientation. Test these."
    printlog "Possibility: check random effects and see if something hangs."

    printlog "Load/Save-test: Does the settings last?"

    printlog "Are two different objects dependant or independent from each other?"

    printlog "Close Application"
    Call hCloseDocument
    Printlog "Finished Optional-test for Presentation-Engine"
endcase 'tiPenginefast

'-------------------------------------------------------------------------------

testcase tSlideShowInteraction
    Dim Datei$
    Dim i as integer
    Dim Zaehler as integer
    dim b115364 as boolean

    Datei$ =ConvertPath (gOfficePath + "user\work\interac.sxi")

'   printlog "save file as presentation with name '"user\\work\\diashow.sxi")'"
'   hFileSaveAsWithFilterKill ( sFile , gImpressFilter, FALSE )
'   Printlog "saved presetation: '" + sFile + "'"

   printlog "open application"
   Call hNewDocument
   sleep 5
   setStartCurrentPage(FALSE)   printlog "Set "start with current page to OFF"

   printlog "call 'Insert->Slide' three times and name the slides 2, 3, 4 and create a rectangle on it "
   for i = 2 to 4
      InsertSlide
      sleep 2
      hTypekeys "<Pagedown>"
      sleep 2
      Call hRechteckErstellen (i*10,i*10,i*20,i*20)
      sleep 2
   next i

   printlog "check state of navigator ! expected: closed -> open navigator"
   Kontext "Navigator"
   if Navigator.exists then
      warnlog "Navigator: already open :-("
   else
      printlog "Navigator: NOT available :-( will be opened now!"
      ViewNavigator
   endif
   sleep 3

    printlog "Slide Show->Interaction"
    SlideShowInteraction
    Kontext "TabInteraktion"
    sleep 1
    if AktionBeiMausklick.GetItemCount = 13 Then
        Printlog "- List is complete"
    else
        Warnlog "- Number of possible actions is wrong. It should be: 13, but it is: " + AktionBeiMausklick.GetItemCount
    end if
    sleep 1

    AktionBeiMausklick.Select 1                       'Keine Aktion bei Mausclick
    Printlog (AktionBeiMausklick.GetSelText + " chosen")
    If Durchsuchen.IsVisible then Warnlog "- Control should be invisible because AktionBeiMausklick = " + AktionBeiMausKlick.GetSelText
    sleep 1

    Kontext "TabInteraktion"
    AktionBeiMausklick.Select 2                       'Sprung zur vorhergehenden Seite
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
    AktionBeiMausklick.select 3                       'Sprung zur naechsten Seite
     Printlog (AktionBeiMausKlick.GetSelText + " chosen")
    TabInteraktion.OK
    sleep 1

   Kontext "DocumentImpress"
   gMouseClick 90,90
   sleep 2
   gMouseClick 50,50
   Kontext "NavigatorDraw"
    if Liste.GetSelIndex <> 4 Then
        Warnlog "- Jumped to wrong slide"
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
    AktionBeiMausklick.select 4                        'Sprung zur ersten Seite
    Printlog AktionBeiMausKlick.GetSelText
    sleep 1
    TabInteraktion.OK
    sleep 1
    Kontext "DocumentImpress"
    gMouseClick 90,90
    gMouseClick 50,50
    Kontext "NavigatorDraw"
    if Liste.GetSelIndex <> 1 Then Warnlog "- jumped to wrong slide"
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
    AktionBeiMausklick.select 5                       'Sprung zur letzten Seite
    printlog AktionBeiMausKlick.GetSelText
    TabInteraktion.OK
    sleep 1
    Kontext "NavigatorDraw"
    if Not Liste.GetSelIndex = 1 Then Warnlog "- jumped to wrong slide"

    kontext "DocumentImpress"
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
    if ListeSprungZuSeiteObjekt.GetSelIndex <> 3 Then Warnlog " Search does not work"
    printlog ListeSprungZuSeiteObjekt.GetSelIndex
    TabInteraktion.OK
    sleep 1

    Kontext "DocumentImpress"
    gMouseClick 90,90
    gMouseClick 25,25,
    sleep 2
    Kontext "NavigatorDraw"
    if Liste.GetSelIndex <> 2 Then Warnlog "- Jumped to wrong destination"
    printlog Liste.GetSelIndex
    Liste.Select 4
    Kontext "DocumentImpress"
    EditSelectAll
    sleep 1

    Kontext "DocumentImpress"
    EditSelectAll
    sleep 1
    SlideShowInteraction
    Kontext "TabInteraktion"
    sleep 1
    AktionBeiMausKlick.Select 7                    'Sprung zu Dokument
    Printlog AktionBeiMausKlick.GetSelText + " chosen"
    try
        Durchsuchen.Click
        sleep 1
        Kontext "OeffnenDlg"
        sleep 1
        Dateiname.SetText ConvertPath (gTesttoolPath + "graphics\required\input\recht_49.sxi")
        Oeffnen.Click
        sleep 10
        ' check if the document is writable
        if fIsDocumentWritable = false then
            ' make the document writable and check if it's succesfull
            if fMakeDocumentWritable = false then
                warnlog "The document can't be make writeable. Test stopped."
                goto endsub
            endif
        endif
    catch
        Warnlog "- Search button could not be accessed"
    endcatch
    sleep 1
    Kontext "TabInteraktion"
    sleep 1
    TabInteraktion.OK
    sleep 3
    Kontext "DocumentImpress"
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
            if fileExists(ConvertPath (gOfficePath + "user\work\bug115364.sxi")) then
                kill ConvertPath (gOfficePath + "user\work\bug115364.sxi")
            endif
            messagebox.yes ' don't save changes, before going on!
            b115364 = true
            kontext "SpeichernDlg"
            Dateiname.setText ConvertPath (gOfficePath + "user\work\bug115364.sxi")
            Speichern.click
            sleep 10
        endcatch
    else
        printlog "OK :-)"
    endif
    Kontext "DocumentImpress"
    DocumentImpress.MouseUp 30,30

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
        hDateiOeffnen (ConvertPath (gOfficePath + "user\work\bug115364.sxi"))
    endif
    sleep 5

    Kontext "NavigatorDraw"
    sleep 1
    Naechste.click

    Kontext "DocumentImpress"
    EditSelectAll
    sleep  1

    SlideShowInteraction
    Kontext "TabInteraktion"
    sleep 1
    AktionBeiMausKlick.Select 9
    Printlog AktionBeiMausKlick.GetSelText + " chosen"
    sleep 1
    Tabinteraktion.OK
    sleep 2

    SlideShowInteraction
    Kontext "TabInteraktion"
    sleep 1
    if not AktionBeiMausKlick.GetSelIndex = 9 Then Warnlog "- Not the right action chosen"
    sleep 1
    AktionBeiMausKlick.Select 9                          'Klang abspielen
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
    AktionBeiMausKlick.Select 11                       'Objektaktion ausfuehren
    Printlog  AktionBeiMausKlick.GetSelText + " chosen"
    sleep 1
    Zaehler=Effekt.GetItemCount
    for i=1 to Zaehler
        Effekt.Select i
        Printlog Effekt.GetSelText + " chosen"
        if i=1 Then
        if Langsam.IsEnabled Then Warnlog "- Control should not be enabled, because no effect chosen"
        if Mittel.IsEnabled Then Warnlog "- Control should not be enabled, because no effect chosen"
        if Schnell.IsEnabled Then Warnlog "- Control should not be enabled, because no effect chosen"
        end if
        sleep 1

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
    AktionBeiMausKlick.Select 10                       'Objekt ausblenden
    Printlog AktionBeiMausKlick.GetSelText + " chosen"
    ListeObjektAktion.Select 1
    sleep 1
    TabInteraktion.OK
    sleep 1
    gMouseClick 90,90
    sleep 1
    gMouseClick 55,55
    sleep 1
    Kontext "DocumentImage"

    SlideShowInteraction
    Kontext "TabInteraktion"
    sleep 1
    AktionBeiMausKlick.Select 13 			'Makro
    Printlog AktionBeiMausKlick.GetSelText + " chosen"
    sleep 2
    Durchsuchen.Click
    sleep 5
    Kontext "Makro"
    sleep 5
    Makro.Cancel
    sleep 2
    Kontext "TabInteraktion"
    sleep 1
    TabInteraktion.OK
    sleep 3

    SlideShowInteraction
    Kontext "TabInteraktion"
    sleep 2
    if not AktionBeiMausKlick.GetSelIndex = 12 Then Warnlog "- Wrong action used"
    sleep 1
    AktionBeiMausKlick.Select 14           'Praesentation beenden
     Printlog AktionBeiMausKlick.GetSelText
    Tabinteraktion.OK
    sleep 1

    SlideShowPresentationSettings
    Kontext "Bildschirmpraesentation"
    if not Fenster.IsChecked Then Fenster.Check
    BildschirmPraesentation.OK
    sleep 2
    Kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<MOD1 F2>"
    sleep 5
    Kontext "DocumentPresentation"
    sleep 2
    DocumentPresentation.MouseDown 50,50
    DocumentPresentation.MouseUp 50,50
    sleep 5
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
    DocumentImpress.TypeKeys "<SHIFT MOD1 F5>"
    sleep 3
    setStartCurrentPage(TRUE)   printlog "Set "start with current page to ON = Default"
    Call hCloseDocument
endcase 'tSlideShowInteraction

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
        printlog "click button 'Add...'"
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
                Effects.select(2)
            else
                bError = true
                warnlog "Impress:Tasks Pane:Custom Animation:TabMotionPaths tabPage doesn't work."
            endif
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
            printlog "CLick on button '...' (Options)"
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
endcase