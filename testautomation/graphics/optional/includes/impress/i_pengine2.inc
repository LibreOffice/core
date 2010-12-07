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
'* short description : Tests the Presentation-Engines effects
'*
'*******************************************************************

testcase tiPengineAnimationEffectsPreview
    dim bError as boolean

    printlog "open application"
    Call hNewDocument
    printlog "create textbox with text"
    Call hTextrahmenErstellen ("Test text to test text effects", 35, 35, 70, 70 )
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
       TestAnimations
       printlog "Switch to TabPage: Emphasis"
       kontext
       active.setPage(TabEmphasis)
       kontext "TabEmphasis"
          if TabEmphasis.exists(5) then
             DialogTest(TabEmphasis)
             TestAnimations
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
                 TestAnimations
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
                 TestAnimations
              else
                 bError = true
                 warnlog "Impress:Tasks Pane:Custom Animation:TabMotionPaths tabPage doesn't work."
              endif
           kontext

           printlog "Switch to TabPage: Entrance"
           active.setPage(TabEntrance)
           kontext "TabEntrance"
           if TabEntrance.exists(5) then
              Effects.Select 4
              printlog "Close dialog 'Custom Animation' with 'OK'"
              TabEntrance.OK
           else
              warnlog "Error when switching Tab"
           endif
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
            TabEntrance.Cancel
            Kontext "Tasks"
            EffectStart.GetItemCount
            if EffectProperty.IsVisible then
                EffectProperty.GetItemCount
            endif
            printlog "CLick on button '...' (Options)"
            EffectOptions.Click
            kontext "TabEffect"
            if TabEffect.Exists(5) then
                dialogTest(TabEffect)
                Sound.GetItemCount
                AfterAnimation.GetItemCount

                printlog "switch to TabPage 'Timing'"
                Kontext
                Active.SetPage TabTiming
                kontext "TabTiming"
                if TabTiming.Exists(5) then
                    dialogTest(TabTiming)
                    TimingStart.GetItemCount
                    Delay.GetText
                    Speed.GetItemCount
                    Repeat.GetItemCount
                    Rewind.Ischecked
                    TriggerAnimate.IsChecked
                    TriggerStart.IsChecked
                    Shape.GetItemCount
                else
                    warnlog "Impress:Tasks Pane:Custom Animation:Effect Options: Timing TabPage didn't work."
                endif

                printlog "switch to TabPage 'Timing'"
                Kontext
                active.setPage TabTextAnimation
                kontext "TabTextAnimation"
                if TabTextAnimation.Exists(5) then
                    dialogTest(TabTextAnimation)
                    GroupText.GetItemCount
                    AnimateAttachedShape.IsChecked
                    TabTextAnimation.Cancel
                else
                    warnlog "Impress:Tasks Pane:Custom Animation:Effect Options: TextAnimation TabPage didn't work."
                endif
            else
                warnlog "Impress:Tasks Pane:Custom Animation:... button didn't work."
            endif
            Kontext "Tasks"
            EffectSpeed.GetItemCount
            EffectList.GetItemCount
            EffectPlay.Click
            sleep (3)
            EffectSlideShow.Click
            sleep (1)
            kontext "DocumentPresentation"
            if DocumentPresentation.Exists (15) then
                printlog "Presentation started :-)"
                DocumentPresentation.TypeKeys "<SPACE>"
                if DocumentPresentation.Exists (15) then
                    DocumentPresentation.TypeKeys "<SPACE>"
                endif
                if DocumentPresentation.Exists (15) then
                    DocumentPresentation.TypeKeys "<ESCAPE>"
                endif
            else
                warnlog "Impress:Tasks Pane:Custom Animation:Slide Show button doesn't start slideshow!"
            endif
            sleep (2)
            kontext "Tasks"
            EffectAutomaticPreview.Check
            printlog "click button 'Remove'"
            EffectRemove.Click
        endif
        sleep (2)
    printlog "close application"
    Call hCloseDocument
endcase 'tiPengineAnimationEffectsPreview

'------------------------------------------------------------------------------

testcase tiPengineAnimationEffectsOptions

    dim bError as boolean
    dim e as integer
    dim d as integer
    dim i as integer
    dim l as integer
    dim o as integer
    dim p as integer
    dim q as integer
    dim s as integer
    dim t as integer
    dim u as integer
    dim y as integer
    dim w as integer
    dim numberx as integer
    dim Effectname1 as string
    dim StartName1 as string
    dim PropertyName1 as string

    printlog "open application"
    Call hNewDocument
    printlog "create textbox with text"
    Call hTextrahmenErstellen ("Test text to test text effects", 35, 35, 70, 70 )
    printlog "Slide Show->Custom Animation..."
    SlideShowCustomAnimation
    printlog "Dialog 'Custom Animation' comes up"
    Kontext "Tasks"
    printlog "Click button 'Add...' to add an effect to the text"
    EffectAdd.click
    kontext
    printlog "Switch to TabPage: Entrance"
    active.setPage(TabEntrance)
    kontext "TabEntrance"
    if TabEntrance.exists(5) then
      AutomaticPreview.UnCheck
      Printlog "Testing effects in - TabEntrance"
      i = Effects.GetItemCount
      p = 7555
      Effects.TypeKeys "<HOME DOWN>"
      Randomize
      printlog "Choose ten random effects, and test them."
      For e = 1 to 10
          randomize
          for y = 1 to 1
              randomize
              numberx = Int((i*Rnd))
              if (numberx<1) OR (numberx>i) then      '  Just so we get it between 1 and the amount of items.
                 y = y - 1
              endif
          next y
          printlog numberx                                       ' Log what effect were about to select. Just for debugging.
          Effects.TypeKeys "<HOME>"
          Effects.TypeKeys "<DOWN>", numberx      ' Select the effect.
	  sleep 1
          Printlog "Effect Nr: " + (Effects.GetSelIndex -1) + ". Name: " + Effects.GetSelText     ' Log the number and effect-name.
          Effectname1 = Effects.GetSelText
          if AutomaticPreview.IsChecked = TRUE then sleep 1      '  Sleep one sec to at least let the preview start.
          d = Effects.GetSelIndex
          TabEntrance.Ok
          optionstest2                           ' Do the optionstest for this effect.
          Kontext "Tasks"
             if Tasks.Exists then
                EffectChange.Click
             else
                Warnlog "Something wrong when exiting Impress:Tasks Pane:Custom Animation: ... (options)"
             endif
             kontext "TabEntrance"
             printlog "Select the next entry"
	     Effects.TypeKeys "<DOWN>", e
             p = Effects.GetSelIndex
             If p = d Then e = i
             sleep 2
             if Speed.isVisible then
                if Speed.isEnabled then
                   s = Speed.GetItemCount
                   For q = 1 to s
                       try
                          Speed.Select q
                       catch
                          warnlog "The speed nr: " + q + " had some kind of problem. Check it."
                       endcatch
                   Next q
                endif
             else
                Warnlog "Speed in Effect Options were not to be found."
             endif
      Next e
      kontext "TabEntrance"
      Printlog "Test of Entrance-list ended."
      AutomaticPreview.Check
      TabEntrance.Ok
   else
      bError = true
      warnlog "Impress:Tasks Pane:Custom Animation:Add... button didnt work."
   endif
   kontext "Tasks"
   EffectAutomaticPreview.Check
   printlog "click button 'Remove'"
   EffectRemove.Click
   printlog "close application"
   Call hCloseDocument
endcase 'tiPengineAnimationEffectsOptions

'-------------------------------------------------------------------------------

testcase tiPengineAllShapesAndEffects

    dim sFileName as string

    printlog "the Presentation-Engine consists of showing the presentation, with all it's effects."

    printlog "Create a new presentation."
    Call hNewDocument
    Sleep (1)

    printlog "Open the test-file."
    Call hFileOpen (gTesttoolpath + "graphics\required\input\allshapes2.odp") 'effects.odp")
    
    printlog "Start the slideshow."
    Call hTypeKeys "<F5>"
    sleep (10)
    kontext "DocumentPresentation"
    while DocumentPresentation.exists()
        DocumentPresentation.TypeKeys "<SPACE>"
        sleep(10)
    wend
    
    Kontext "DocumentImpress"
    if (DocumentImpress.exists(1)) then
        'nothing
    else
        warnlog "presentation not ended."
    endif

    Call hCloseDocument

    sleep (1)
    printlog "Open the test-file."
    Call hFileOpen (gTesttoolpath + "graphics\required\input\effects.odp")
    Sleep (10)
    
    printlog "Start the slideshow."
    CALL hTypeKeys "<F5>"
    Sleep (10)
    printlog "Press Space again, to continue with slide two."
    kontext "DocumentPresentation"
     while DocumentPresentation.exists()
        DocumentPresentation.TypeKeys "<SPACE>"
        sleep(10)
    wend

    printlog "End of eleventh page."
    if DocumentPresentation.Exists then
        warnlog "DocumentPresentation shouldnt be visible right now."
        DocumentPresentation.TypeKeys "<SPACE>"
    else
        printlog "Presentation ended correctly."
        kontext "DocumentImpress"
    end if

printlog "Close Application"
    Call hCloseDocument
    Printlog "Finished Optional-test for Presentation-Engine"
endcase 'tiPengineAllShapesAndEffects

'-------------------------------------------------------------------------------

testcase tiPengine2ObjectsGetsEffects

    dim i as integer
    dim t as integer
    dim q as integer
    dim e as integer
    dim sFileName as string

    printlog "the Presentation-Engine consists of showing the presentation, with all it's effects."

    printlog "Create a new presentation."
    Call hNewDocument
    Call sSelectEmptyLayout
    sleep (1)

    kontext "Toolbar"
    sleep (1)
    printlog "From the toolbar: Insert three objects:"
    printlog "insert a Smiley."
    kontext "Toolbar"
    if Toolbar.Exists then
        if Toolbar.IsVisible then
            sleep (1)
            try
                SymbolShapes.TearOff
            catch
                warnlog "Issue for GH; .tearoff doesnt tell success"
            endcatch
            sleep (1)
            kontext "SymbolShapes"
            SymbolShapesSmiley.Click
            sleep (1)

            gMouseDown (40,40)
            gMouseMove (40,40,60,60)
            gMouseUp (60,60)

            sleep (1)
            kontext "SymbolShapes"
	    SymbolShapes.Close

            printlog "Unmark all objects"
            hTypeKeys "<ESCAPE>"

            printlog "Mark the Smiley"
            hTypeKeys "<TAB>"
        else
            warnlog "No toolbar visible, please notify the test-administrator"
        end if
    else        
        warnlog "toolbar not visible"
    end if

    printlog "Smiley inserted, time to add some effects."

    printlog "Effect no 1"

    printlog "Slide Show->Custom Animation..."
    SlideShowCustomAnimation
        Kontext "Tasks"
        printlog "Click button 'Add...'"
        EffectAdd.click
        printlog "Dialog 'Custom Animation' comes up"
        kontext
        printlog "Switch to TabPage: Entrance"
        active.setPage(TabEntrance)
        kontext "TabEntrance"
        if TabEntrance.exists(5) then
            DialogTest(TabEntrance)
            printlog "Find 'Dissolve-in'." 
            TabEntrance.TypeKeys "<DOWN>", 6
            Sleep (3)
            TabEntrance.Ok
            Printlog "Added effect 'Dissolve-in'"
        else
            warnlog "Tab Entrance does not exist?"
        end if
        Kontext "Tasks"

    printlog "Effect no 2"

    printlog "Add a second effect to the object"
    EffectAdd.click
    
    printlog "Dialog 'Custom Animation' comes up"
    printlog "Switch to TabPage: Emphasis"
    kontext
    active.setPage(TabEmphasis)
    kontext "TabEmphasis"
    if TabEmphasis.exists(5) then
        DialogTest(TabEmphasis)
        Effects.Select 5    'Transparency
        Sleep (3)
        printlog "Close dialog 'Custom Animation' with 'OK'"
        TabEmphasis.OK
        Printlog "Added effect 'Transparency'"
    else
        warnlog "Error when switching Tab"
    end if
    Kontext "Tasks"
    printlog "Effect no 3"
    printlog "Insert new slide"
    InsertSlide
    Printlog "Inserted new Slide" 

    kontext "Slides"
    printlog "Make sure the last slide is selcted"
    SlidesControl.TypeKeys "<TAB>"
    SlidesControl.TypeKeys "<PAGEDOWN>", 3
    Sleep (1)

    printlog "Insert new object"
    Call hTextrahmenErstellen ("Test text for the second slide to test the PresentationEngine", 35, 35, 70, 70)

    printlog "Add the Transformation-Effect 'put on the brakes'" 
    SlideShowCustomAnimation
        Kontext "Tasks"
        printlog "Click button 'Add...'"
        EffectAdd.click
            printlog "Dialog 'Custom Animation' comes up"
            kontext
            printlog "Switch to TabPage: Entrance"
            active.setPage(TabEntrance)
            kontext "TabEntrance"
            if TabEntrance.exists(5) then
                printlog "Add Transformation-Effect: 'Put on the Breaks'"
                TabEntrance.TypeKeys "<DOWN>", 30  
                Sleep (3)
                TabEntrance.Ok
                Printlog "Added effect 'Put on the Breaks'"
            else
                warnlog "Impress:Tasks Pane:Custom Animation:Add... button didn't work."
            end if
            Kontext "Tasks"

            printlog "Effect no 4"
            printlog "Click button 'Add...'"
            EffectAdd.click
            printlog "Dialog 'Custom Animation' comes up"
            kontext
            printlog "Switch to TabPage: Motion Paths"
            active.setPage(TabMotionPaths)
            kontext "TabMotionPaths"
            if TabMotionPaths.exists(5) then
                printlog "Add motion-path-effect:  'schwosch'"
                TabMotionPaths.TypeKeys "<DOWN>", 66 
                Sleep (3)                
                TabMotionPaths.Ok
                Printlog "Added effect 'Schwosch'"
                kontext "Tasks"
            else
                warnlog "Couldn't find the Tabpage: MotionPaths. Check why."
            end if

        printlog "Press 'PageUp' to get to the first slide"
        hTypeKeys "<PAGEUP>"

        printlog "Run the slideshow with F5."
        hTypeKeys "<F5>"
        Sleep (5)
        kontext "DocumentPresentation"
        if DocumentPresentation.Exists (15) then
            Printlog "Presentation started, calling 1st effect, 1st object."
            DocumentPresentation.TypeKeys "<SPACE>"
            sleep (5)
        else
            warnlog "Slideshow didn't start. Check why."
        end if
        printlog "calling 2nd effect, 1st object."
        DocumentPresentation.TypeKeys "<SPACE>"
        Sleep (5)
        printlog "calling 2nd slide."
        DocumentPresentation.TypeKeys "<SPACE>"
        Sleep (5)
        printlog "calling 1st effect, 2nd object"
        DocumentPresentation.TypeKeys "<SPACE>"
        Sleep (5)
        printlog "calling 2nd effect, 2nd object"
        DocumentPresentation.TypeKeys "<SPACE>"
        Sleep (5)
        printlog "ending presentation"
        DocumentPresentation.TypeKeys "<SPACE>"
        Sleep (5)
        Kontext "DocumentPresentation"
        printlog "getting back to edit view."
        DocumentPresentation.TypeKeys "<SPACE>"
        
        if DocumentPresentation.Exists(5) then 
            warnlog "Presentation should have ended. Check why it didn't."
        end if

        Kontext "DocumentImpress"

    printlog "Close the document"
    Call hCloseDocument
    Printlog "Finished Optional-test for Presentation-Engine"
endcase 'tiPengine2ObjectsGetsEffects

'-------------------------------------------------------------------------------

Function TestAnimations

   printlog "Function: TestAnimations: select in the listbox 'Effects' the second entry"

   Dim i as Integer
   Dim s as Integer
   Dim q as Integer
   Dim e as Integer
   Dim o as Integer
   Dim p as Integer

   i = Effects.GetItemCount
   s = Speed.GetItemCount
   AutomaticPreview.Check
   Effects.TypeKeys "<HOME>"
   For e = 1 to i
       If e <> p Then
           if AutomaticPreview.isChecked = TRUE then sleep 1
           Printlog "Effect has position Nr: " + Effects.GetSelIndex + ". Name of effect: " + Effects.GetSelText
           printlog "Select the next entry"
           Effects.TypeKeys "<DOWN>"
           p = Effects.GetSelIndex
       Else
           Printlog "Test of effects ended."
           e = i
       Endif
   Next e
   if Speed.IsEnabled then
       For q = 1 to s
           Speed.Select q
           sleep 1
       Next q
   endif
   AutomaticPreview.Check
   sleep 1
   AutomaticPreview.Check
   Kontext
end Function