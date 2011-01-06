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
'* short description : Tests the Engine for Presentation
'*
'*******************************************************************

testcase tiPengineTabEntrance

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
    dim w as integer
    dim Effectname1 as string
    dim StartName1 as string
    dim PropertyName1 as string
    dim UsedEffect as string
    dim NewEffect as string

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
      AutomaticPreview.UnCheck
      Printlog "Testing effects in - TabEntrance"
      i = Effects.GetItemCount
      For o = 1 to 1'2
          p = 7555
          Effects.TypeKeys "<HOME>"
          For e = 1 to i
              sleep 1
              Printlog "Effect Nr: " + (Effects.GetSelIndex -1) + ". Name: " + Effects.GetSelText
              Effectname1 = Effects.GetSelText
              NewEffect = Effectname1

              if UsedEffect = NewEffect then
                 Warnlog "Wrong effect? The recently tested Effect was: " + Effects.GetSelIndex + ": " + UsedEffect + " and the new one: " + Effects.GetSelIndex + ": " + NewEffect
              endif
              UsedEffect = NewEffect

              if AutomaticPreview.IsChecked = TRUE then sleep 1
              d = Effects.GetSelIndex
              TabEntrance.Ok
              Kontext "Tasks"
              if EffectStart.isEnabled then
                 t = EffectStart.GetItemCount
                 for u = 1 to 1't
                     EffectStart.Select u
                     printlog "EffectStart selected " + EffectStart.GetSelText
                     StartName1 = EffectStart.GetSelText
                     if EffectProperty.Exists then
                        if EffectProperty.isEnabled then
                           if EffectProperty.isVisible then
                              printlog "EffectProperty.!"
                              w = EffectProperty.GetItemCount
                              if w > 0 then
                                 for l = 1 to w
                                     EffectProperty.Select l
                                     sleep 1
                                     PropertyName1 = EffectProperty.GetSelText
                                     printlog "Properties for the effect: " + PropertyName1
'                                     EffectSlideShow.Click
'                                     Sleep (2)
'                                     kontext "DocumentPresentation"
'                                     if DocumentPresentation.Exists (15) then
'                                         DocumentPresentation.TypeKeys "<Space>"
'                                        if DocumentPresentation.Exists (15) then
'                                            DocumentPresentation.TypeKeys "<Space>"
'                                        endif
'                                        Sleep (3)
'                                        if DocumentPresentation.Exists (15) then
'                                            DocumentPresentation.TypeKeys "<Escape>"
'                                        endif
'                                     else
'                                        Warnlog "Effect: " + Effectname1 + " didn't work when it started with: " + StartName1 + "and ended with: " + PropertyName1
'                                     endif
'                                     sleep (2)
                                     Kontext "Tasks"
                                 Next l
                              endif
                            endif '1
                         endif    '2
                      elseif EffectDirection.Exists then
                             if EffectDirection.isEnabled then
                                if EffectDirection.isVisible then
                                   w = EffectDirection.GetItemCount
                                   if w > 0 then
                                      for l = 1 to 1'w
                                      EffectDirection.Select l
                                      PropertyName1 = EffectDirection.GetSelText
                                      printlog "Properties for the effect: " + PropertyName1
                                      sleep (5)
'                                      EffectSlideShow.Click
'                                      Sleep (2)
'                                      kontext "DocumentPresentation"
'                                      if DocumentPresentation.Exists (15) then
'                                         DocumentPresentation.TypeKeys "<Space>"
'                                        if DocumentPresentation.Exists (15) then
'                                            DocumentPresentation.TypeKeys "<Space>"
'                                        endif
'                                        Sleep (3)
'                                        if DocumentPresentation.Exists (15) then
'                                            DocumentPresentation.TypeKeys "<Escape>"
'                                        endif
'                                      else
'                                         warnlog "Effect: " + Effectname1 + " didn't work when it started with: " + StartName1 + "and ended with: " + PropertyName1
'                                      endif
                                      Kontext "Tasks"
                                      sleep (2)
                                  Next l
                               else
                                  warnlog "EffectDirection.isEnabled AND EffectDirection.isVisible, but no items selectable? Effect was: " + Effectname1
                               endif
                            endif '1
                         endif    '2
                     endif        '3
                     kontext "EffectProperty"
                 Next u
              Endif

              Kontext "Tasks"
              if Tasks.Exists then
                 EffectChange.Click
              else
                 Printlog "Presentation didn't end. Error?"
              endif
              kontext "TabEntrance"
              printlog "Select the next entry"
              Effects.TypeKeys "<DOWN>", e 
              p = Effects.GetSelIndex
              If p = d Then e = i
          Next e
          kontext "TabEntrance"

          e = 0
          if Speed.GetItemCount = 0 then
	     warnlog "   'Speed' has 0 entries. Check it."
          else
             do while (e < 1)
                e = Int(Speed.GetItemCount * Rnd)
             loop 
                try
                    Speed.Select (e)
                catch
                    qaerrorlog "#i89498, Custom animation speed field is greyed out"
                endcatch
          endif
          Printlog "Test of Entrance-list ended."
      Next o
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
      warnlog "Impress:Tasks Pane:Custom Animation:Add... button didnt work."
   endif
   kontext "Tasks"
   EffectAutomaticPreview.Check
   printlog "click button 'Remove'"
   EffectRemove.Click
   printlog "close application"
   Call hCloseDocument
endcase 'tiPengineTabEntrance

'-------------------------------------------------------------------------------

testcase tiPengineTabEmphasis

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
    dim w as integer
    dim Effectname1 as string
    dim StartName1 as string
    dim PropertyName1 as string
    dim UsedEffect as string
    dim NewEffect as string

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

      printlog "Switch to TabPage: Emphasis"
      kontext
      active.setPage(TabEmphasis)
      kontext "TabEmphasis"
      if TabEmphasis.exists(10) then
         Printlog "Testing effects in - TabEmphasis"
         i = Effects.GetItemCount
         For o = 1 to 2
             p = 7555
             Effects.TypeKeys "<HOME>"
             For e = 1 to 1'i
                 sleep 2
                 Printlog "Effect Nr: " + (Effects.GetSelIndex -1) + ". Name: " + Effects.GetSelText
                 Effectname1 = Effects.GetSelText
                 if AutomaticPreview.IsChecked = TRUE then sleep 1
                 d = Effects.GetSelIndex
                 TabEmphasis.Ok
                 Kontext "Tasks"
                 if EffectStart.isEnabled then
                    t = EffectStart.GetItemCount
                    for u = 1 to t
                        EffectStart.Select u
                        StartName1 = EffectStart.GetSelText
                        printlog "EffectStart selected " + StartName1

                        if EffectProperty.Exists then
                           if EffectProperty.isEnabled then
                              if EffectProperty.isVisible then
                                 w = EffectProperty.GetItemCount
                                 if w > 0 then
                                    for l = 1 to 1'w
                                        EffectProperty.Select l
                                        PropertyName1 = EffectProperty.GetSelText
                                        printlog "Properties for the effect: " + PropertyName1
                                        sleep 1
'                                        EffectSlideShow.Click
'                                        Sleep (2)
'                                        kontext "DocumentPresentation"
'                                        if DocumentPresentation.Exists (15) then
'                                            DocumentPresentation.TypeKeys "<Space>"
'                                            if DocumentPresentation.Exists (15) then
'                                                DocumentPresentation.TypeKeys "<Space>"
'                                            endif
'                                            Sleep (3)
'                                            if DocumentPresentation.Exists (15) then
'                                                DocumentPresentation.TypeKeys "<Escape>"
'                                            endif
'                                       else
'                                           Warnlog "Effect: " + Effectname1 + " didn't work when it started with: " + StartName1 + "and ended with: " + PropertyName1
'                                       endif
'                                       sleep (2)
                                       Kontext "Tasks"
                                    Next l
                                 endif
                              Else
                                 Printlog "No effect properties for " + StartName1
                                 sleep 1
'                                 EffectSlideShow.Click
'                                 Sleep (2)
'                                 kontext "DocumentPresentation"
'                                 if DocumentPresentation.Exists (15) then
'                                     DocumentPresentation.TypeKeys "<Space>"
'                                     if DocumentPresentation.Exists (15) then
'                                         DocumentPresentation.TypeKeys "<Space>"
'                                     endif
'                                     Sleep (3)
'                                     if DocumentPresentation.Exists (15) then
'                                         DocumentPresentation.TypeKeys "<Escape>"
'                                     endif
'                                 else
'                                    Warnlog "Effect: " + Effectname1 + " didn't work when it started with: " + StartName1
'                                 endif
'                                 sleep (2)
                                 Kontext "Tasks"
                              Endif
                           endif
                        endif
                    Next u
                 Endif
                 Kontext "Tasks"
                 if Tasks.Exists then
                    EffectChange.Click
                 else
                    Printlog "Presentation didn't end. Error?"
                 endif
                 printlog "Switch to TabPage: Emphasis"
                 kontext
                 active.setPage(TabEmphasis)
                 kontext "TabEmphasis"
                 printlog "Select the next entry"
                 Effects.TypeKeys "<DOWN>", e + 1 
                  p = Effects.GetSelIndex
                  If p = d Then e = i
             Next e
             kontext "TabEmphasis"

             e = 0
	     if Speed.GetItemCount = 0 then
	        warnlog "   'Speed' has 0 entries. Check it."
             else
                do while (e < 1)
                   e = Int(Speed.GetItemCount * Rnd)
                loop 
                Speed.Select (e)
             endif

             Printlog "Test of Entrance-list ended."
         Next o
      else
         bError = true
         warnlog "Impress:Tasks Pane:Custom Animation:TabExit tabPage doesnt work."
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
   kontext "Tasks"
   EffectAutomaticPreview.Check
   printlog "click button 'Remove'"
   EffectRemove.Click
   printlog "close application"
   Call hCloseDocument
endcase 'tiPengineTabEmphasis

'-------------------------------------------------------------------------------

testcase tiPengineTabExit

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
    dim w as integer
    dim Effectname1 as string
    dim StartName1 as string
    dim PropertyName1 as string
    dim UsedEffect as string
    dim NewEffect as string

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

      printlog "Switch to TabPage: Exit"
      active.setPage(TabExit)
      kontext "TabExit"
      if TabExit.exists(5) then
         Printlog "Testing effects in - TabExit"
         i = Effects.GetItemCount
         For o = 1 to 2
             p = 7555
             Effects.TypeKeys "<HOME>"
             For e = 1 to 1'i
                 sleep 1
                 Printlog "Effect Nr: " + (Effects.GetSelIndex -1) + ". Name: " + Effects.GetSelText
                 Effectname1 = Effects.GetSelText
                 if AutomaticPreview.IsChecked = TRUE then sleep 1
                 d = Effects.GetSelIndex
                 TabExit.Ok
                 Kontext "Tasks"

                 if EffectStart.isEnabled then
                    t = EffectStart.GetItemCount
                    for u = 1 to 1't
                        EffectStart.Select u
                        StartName1 = EffectStart.GetSelText
                        printlog "EffectStart selected " + StartName1

                        if EffectProperty.isEnabled then
                           if EffectProperty.isVisible then
                              if EffectProperty.isVisible then
                                 w = EffectProperty.GetItemCount
                                 if w > 0 then
                                    for l = 1 to 1'w
                                        EffectProperty.Select l
                                        PropertyName1 = EffectProperty.GetSelText
                                        printlog "Properties for the effect: " + PropertyName1
                                        sleep 1
'                                        EffectSlideShow.Click
'                                        Sleep 2
'                                        kontext "DocumentPresentation"
'                                        if DocumentPresentation.Exists (5) then
'                                            DocumentPresentation.TypeKeys "<Space>"
'                                            if DocumentPresentation.Exists (15) then
'                                                DocumentPresentation.TypeKeys "<Space>"
'                                            endif
'                                            if DocumentPresentation.Exists (15) then
'                                                DocumentPresentation.TypeKeys "<Escape>"
'                                            endif
'                                        else
'                                           Warnlog "Effect: " + Effectname1 + " didn't work when it started with: " + StartName1 + "and ended with: " + PropertyName1
'                                        endif
'                                        sleep (2)
                                        Kontext "Tasks"
                                    Next l
                                 endif
                              Else
'                                 Printlog "No effect properties for " + StartName1
                                 sleep 1
'                                 EffectSlideShow.Click
'                                 Sleep 2
'                                 kontext "DocumentPresentation"
'                                 if DocumentPresentation.Exists (5) then
'                                    DocumentPresentation.TypeKeys "<Space>"
'                                    Sleep 2
'                                    DocumentPresentation.TypeKeys "<Space>"
'                                    DocumentPresentation.TypeKeys "<Escape>"
'                                 else
'                                    Warnlog "Effect: " + Effectname1 + " didn't work when it started with: " + StartName1
'                                 endif
'                                 sleep (2)
                                 Kontext "Tasks"
                              Endif
                           Endif
                        Endif
                    Next u
                 Endif

                 Kontext "Tasks"
                 if Tasks.Exists then
                    EffectChange.Click
                 else
                    Printlog "Presentation didn't end. Error?"
                 endif
                 kontext
                 printlog "Switch to TabPage: Exit"
                 active.setPage(TabExit)
                 kontext "TabExit"
                 printlog "Select the next entry"
                 Effects.TypeKeys "<DOWN>", e + 1 
                 p = Effects.GetSelIndex
                 If p = d Then e = i 'p > 2 AND
             Next e
             randomize

             kontext "TabExit"
             Speed.Typekeys "<DOWN>", (Int(Speed.GetItemCount * Rnd) + 1)
             Printlog "Test of Entrance-list ended."
         Next o
         bError = false
      else
         bError = true
         warnlog "Impress:Tasks Pane:Custom Animation:TabExit tabPage doesnt work."
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
   kontext "Tasks"
   EffectAutomaticPreview.Check
   printlog "click button 'Remove'"
   EffectRemove.Click
   printlog "close application"
   Call hCloseDocument

endcase 'tiPengineTabExit

'-------------------------------------------------------------------------------

testcase tiPengineTabMotionPaths

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
    dim w as integer
    dim Effectname1 as string
    dim StartName1 as string
    dim PropertyName1 as string
    dim UsedEffect as string
    dim NewEffect as string
    
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
      
      printlog "Switch to TabPage: Motion Paths"
      active.setPage(TabMotionPaths)
      kontext "TabMotionPaths"
      if TabMotionPaths.exists(5) then
         Printlog "Testing effects in - TabMotionPaths"
         i = Effects.GetItemCount
         For o = 1 to 1'2
             p = 7555
             Effects.TypeKeys "<DOWN>",4
             For e = 1 to 1'i
                 sleep 1
                 Printlog "Effect Nr: " + (Effects.GetSelIndex -1) + ". Name: " + Effects.GetSelText
                 Effectname1 = Effects.GetSelText
                 if AutomaticPreview.IsChecked = TRUE then sleep 1
                 d = Effects.GetSelIndex
                 TabMotionPaths.Ok
                 Kontext "Tasks"

                 if EffectStart.isEnabled then
                    t = EffectStart.GetItemCount
                    for u = 1 to 1  't
                        EffectStart.Select u
                        StartName1 = EffectStart.GetSelText
                        printlog "EffectStart selected " + StartName1
                        if EffectProperty.isEnabled then
                           if EffectProperty.isVisible then
                              if EffectProperty.isVisible then
                                 w = EffectProperty.GetItemCount
                                 if w > 0 then
                                    for l = 1 to 1'w
                                        EffectProperty.Select l
                                        PropertyName1 = EffectProperty.GetSelText
                                        sleep 1
                                        printlog "Properties for the effect: " + PropertyName1
'                                        EffectSlideShow.Click
'                                        Sleep (2)
'                                        kontext "DocumentPresentation"
'                                        if DocumentPresentation.Exists (15) then
'                                            DocumentPresentation.TypeKeys "<Space>"
'                                            Sleep (2)
'                                            if DocumentPresentation.Exists (15) then
'                                                DocumentPresentation.TypeKeys "<Space>"
'                                            endif
'                                            if DocumentPresentation.Exists (15) then
'                                                DocumentPresentation.TypeKeys "<Escape>"
'                                            endif
'                                        else
'                                           Warnlog "Effect: " + Effectname1 + " didn't work when it started with: " + StartName1 + "and ended with: " + PropertyName1
'                                        endif
'                                        sleep (2)
                                        Kontext "Tasks"
                                    Next l
                                 endif
                              Else
'                                 Printlog "No effect properties for " + StartName1
                                 sleep 1
'                                 EffectSlideShow.Click
'                                 Sleep (2)
'                                 kontext "DocumentPresentation"
'                                 if DocumentPresentation.Exists (15) then
'                                    DocumentPresentation.TypeKeys "<Space>"
'                                    Sleep (2)
'                                    if DocumentPresentation.Exists (15) then
'                                       DocumentPresentation.TypeKeys "<Space>"
'                                    endif
'                                    if DocumentPresentation.Exists (15) then
'                                        DocumentPresentation.TypeKeys "<Escape>"
'                                    endif
'                                 else
'                                    Warnlog "Effect: " + Effectname1 + " didn't work when it started with: " + StartName1
'                                 endif
                                 Kontext "Tasks"
'                                 sleep (2)
                              Endif
                           Endif
                        Endif
                    Next u
                 Endif
                 kontext "DocumentImpress"
                 if DocumentImpress.Exists (15) then 
                     printlog "Returned to the Impress Edit-mode."
                 endif

                 Kontext "Tasks"
                 if Tasks.Exists then
                    EffectChange.Click
                 else
                    Printlog "Presentation didn't end. Error?"
                 endif
                 kontext
                 printlog "Switch to TabPage: Motion Paths"
                 active.setPage(TabMotionPaths)
                 kontext "TabMotionPaths"
                 Effects.TypeKeys "<DOWN>", e + 1
                 printlog "Select the next entry"
'                 if p = 2 then Effects.TypeKeys "<DOWN>"
                  p = Effects.GetSelIndex
                  If p = d Then e = i 'p > 2 AND
             Next e
 
             kontext "TabMotionPaths"
             Speed.Typekeys "<DOWN>", (Int(Speed.GetItemCount * Rnd) + 1)
             Printlog "Test of Entrance-list ended."
         Next o
         bError = false
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
   kontext "Tasks"
   EffectAutomaticPreview.Check
   printlog "click button 'Remove'"
   EffectRemove.Click
   printlog "close application"
   Call hCloseDocument
endcase 'tiPengineTabMotionPaths

'--------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiPengineSlideTransition

    dim a as integer
    dim b as integer
    dim i as integer
    dim lala as integer
    dim iCount as integer
    dim Soundposition as integer

    printlog "open application"
    Call hNewDocument
    printlog "create rectangle"
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    sleep 1
    printlog "Insert a second slide"
    InsertSlide
    sleep 2
    hTypeKeys "<Pagedown>"
    sleep 2
    printlog "create textframe with quotes; Simple: 39; Double: 34;"
    hTextrahmenErstellen ( "Hallo",20,20,60,30)
    printlog "Insert a third slide"
    InsertSlide
    sleep 2
    hTypekeys "<Pagedown>"
    sleep 2
    printlog "Slide Show->Slide Transition"
    SlideShowSlideTransition
    sleep 2
    printlog "The 'Slide Transition' in the right 'Tasks' Pane has to come up"
    Kontext "Tasks"
        printlog "Select every entry in the Listbox 'Apply to Selected Slides'"
        printlog "Then select the next Speed for Transition and retest all the effects"
        printlog "Redo until all effects has been tested with all speeds"
        lala = TransitionApplyToSelectedSlide.GetItemCount
        Printlog "Count of effects : " + TransitionApplyToSelectedSlide.GetItemCount
        Printlog "Count of Speeds  : " + TransitionSpeed.GetItemCount
        For b = 1 to 1  'i
            TransitionSpeed.Typekeys "<DOWN>", (Int(TransitionSpeed.GetItemCount * Rnd))    
            Printlog "Current speed is: " + TransitionSpeed.GetSelText
            For iCount = 50 to lala
                TransitionApplyToSelectedSlide.Select (iCount)
                TransitionPlay.Click
                sleep 5 ' takes some time, until it is running
                Printlog "Selected effect is: " + TransitionApplyToSelectedSlide.GetSelText
            Next iCount
        Next b
        Printlog "Count of Sounds  : " + TransitionSound.GetItemCount
        printlog "One Entry of the Listbox 'Sound' is 'Other sound...', select it"
'        iCount = TransitionSound.GetItemCount
        TransitionSound.TypeKeys "<HOME>"
        i = 0
        randomize
'        for a = 1 to 1  'iCount
            TransitionSound.Select (Int(TransitionSound.GetItemCount * Rnd) + 1)
            Printlog "The Transition Sound tested is: " + TransitionSound.GetSelText + " ( " + TransitionSound.GetSelIndex + " )."
            Soundposition = TransitionSound.GetSelIndex
            kontext "OeffnenDlg"
            if (OeffnenDlg.Exists (5)) then
               printlog "Read all entries in Listbox 'File type'"
               for i = 1 to Dateityp.getItemCount
                   printlog "" + i + ":" + Dateityp.getItemText(i)
               next i
               printlog "cancel dialog 'Open'"
               sleep 1
               OeffnenDlg.cancel
               kontext "Tasks"
               if TransitionSound.GetSelIndex <> 1 then 
                   Warnlog "TransitionSound didn't go back to the " + Soundposition + " position after pressing cancel in dialogue"
               endif
            endif
            kontext "Tasks"
            ' the Checkbox is disabled on the first three entries: <No Sound>, <Stop previous sound>...
            if (TransitionLoopUntilNextSound.IsEnabled AND (TransitionSound.GetSelIndex<4)) then
                qaErrorLog "May be Language specific -> Evaluation of TBO; " + TransitionSound.GetSelText + "( " + TransitionSound.GetSelIndex + " )."
            endif
'        next a

        sleep 5
        kontext "Tasks"

        printlog "check checkbox 'Automatically after'"
        TransitionAutomaticallyAfter.Check
        sleep 5
        printlog "press key 'Page Up' in box"
        TransitionAutomaticallyAfterTime.TypeKeys "<PageUp>"
        sleep 5
        printlog "check the standard checkbox 'On mouse click'"
        TransitionOnMouseClick.Check
        sleep 5
        printlog "press button 'Apply to All Slides'"
        TransitionApplyToAllSlides.Click
        sleep 5
        printlog "press button 'Play'"
        TransitionPlay.Click
        printlog "press button 'Slide Show'"
        sleep (10)
        TransitionSlideShow.Click
        sleep (3)
        kontext "DocumentPresentation"
        if DocumentPresentation.Exists (5) then
            printlog "Presentation started :-)"
            if DocumentPresentation.Exists (15) then
                DocumentPresentation.TypeKeys "<Space>"
            endif
            if DocumentPresentation.Exists (15) then
                DocumentPresentation.TypeKeys "<Space>"
            endif
            if DocumentPresentation.Exists (15) then
                DocumentPresentation.TypeKeys "<Escape>"
            endif
        else
            warnlog "Impress:Tasks Pane:Slide Transition:Slide Show button doesn't start slideshow!"
        endif
        kontext "Tasks"

        printlog "uncheck and check Checkbox 'Automatic Preview'"
        printlog "default is checked"
        if (NOT TransitionAutomaticPreview.IsChecked) then
            warnlog "Impress:Tasks Pane:Slide Transition: Automatic preview has to be checked by default, wasn't!"
        endif
        TransitionAutomaticPreview.UnCheck
        sleep 1
        TransitionAutomaticPreview.Check
    printlog "close application"
    Call hCloseDocument
endcase 'tiPengineSlideTransition

'testcase
' Two different objects for two different effects. Does it work?
'Endcase

'Further ideas...
'testcase tipEngineUserFunctionTest
' Insert some extra slides and see if they changes on themselves. (chain-reaction)
' Save file, close, open again and see if everything is as it were. (Random effects)
'Endcase

'-------------------------------------------------------------------------------

testcase tiPengineTabpageEntrance2

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
    dim w as integer
    dim Effectname1 as string
    dim StartName1 as string
    dim PropertyName1 as string

    printlog "open application"
    Call hNewDocument
    printlog "create textbox with text"
    Call hTextrahmenErstellen ("Test text to test text effects", 35, 35, 70, 70 )
    printlog "Slide Show->Custom Animation..."
    SlideShowCustomAnimation
    Kontext "Tasks"
    printlog "Uncheck AutomaticPreview"
    EffectAutomaticPreview.UnCheck
    printlog "click button 'Add...'"
    EffectAdd.click
    printlog "Dialog 'Custom Animation' comes up"
    kontext
   
   printlog "Switch to TabPage: Entrance"
   active.setPage(TabEntrance)
   kontext "TabEntrance"
   if TabEntrance.exists(5) then
      DialogTest(TabEntrance)
      AutomaticPreview.UnCheck
      Printlog "Testing effects in - TabEntrance"
      i = Effects.GetItemCount
      For o = 1 to 2
          p = 7555
          Effects.TypeKeys "<HOME>"
          For e = 1 to i
              sleep 1
              Printlog "Effect Nr: " + (Effects.GetSelIndex -1) + ". Name: " + Effects.GetSelText
              Effectname1 = Effects.GetSelText
              if AutomaticPreview.IsChecked = TRUE then sleep 1
              d = Effects.GetSelIndex
              TabEntrance.Ok
              Kontext "Tasks"
              EffectPlay.Click
              Sleep 5
'              EffectSlideShow.Click
'                 Sleep (2)
'                 kontext "DocumentPresentation"
'                 if DocumentPresentation.Exists (15) then
'                    DocumentPresentation.TypeKeys "<Space>"
'                    Sleep (2)
'                    DocumentPresentation.TypeKeys "<Space>"
'                    if DocumentPresentation.Exists (15) then
'                        DocumentPresentation.TypeKeys "<Space>"
'                    endif
'                    if DocumentPresentation.Exists (15) then
'                        DocumentPresentation.TypeKeys "<Escape>"
'                    endif
'                 else
'                    Warnlog "Effect: " + Effectname1 + " didn't work when it started with: " + StartName1 + "and ended with: " + PropertyName1
'                 endif
'                 sleep (2)
              Kontext "Tasks"
              if Tasks.Exists(5) then
                 EffectChange.Click
              else
                 Printlog "Presentation didn't end. Error?"
              endif
              kontext "TabEntrance"
              printlog "Select the next entry"
              Effects.TypeKeys "<DOWN>", e 
              p = Effects.GetSelIndex
              If p = d Then e = i
          Next e

          kontext "DocumentPresentation"
          EditSelectAll
          kontext "TabEntrance"
          try 
              Speed.Typekeys "<DOWN>", (Int(Speed.GetItemCount * Rnd))
          catch
              printlog "Speed doesn't seem to work with the effect: " + Effectname1 + ". Check if this is ok."
          endcatch

          Printlog "Test of Entrance-list ended."
      Next o

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
      warnlog "Impress:Tasks Pane:Custom Animation:Add... button didnt work."
   endif
   kontext "Tasks"
   EffectAutomaticPreview.Check
   printlog "click button 'Remove'"
   EffectRemove.Click
   printlog "close application"
   Call hCloseDocument
endcase 'tiPengineTabpageEntrance2

'------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiPengineTabPageEmphasis2

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
    dim w as integer
    dim Effectname1 as string
    dim StartName1 as string
    dim PropertyName1 as string

    printlog "open application"
    Call hNewDocument
    printlog "create textbox with text"
    Call hTextrahmenErstellen ("Test text to test text effects", 35, 35, 70, 70 )
    printlog "Slide Show->Custom Animation..."
    SlideShowCustomAnimation
   Kontext "Tasks"
   printlog "Uncheck AutomaticPreview"
   EffectAutomaticPreview.UnCheck
   printlog "click button 'Add...'"
   EffectAdd.click
   printlog "Dialog 'Custom Animation' comes up"
   kontext

      printlog "Switch to TabPage: Emphasis"
      kontext
      active.setPage(TabEmphasis)
      kontext "TabEmphasis"
      if TabEmphasis.exists(5) then
         kontext
         active.setPage(TabEmphasis)
         Printlog "Testing effects in - TabEmphasis"
         kontext "TabEmphasis"
         i = Effects.GetItemCount
         For o = 1 to 2
             p = 7555
             Effects.TypeKeys "<HOME>"
             For e = 1 to i
                 sleep 1
                 Printlog "Effect Nr: " + (Effects.GetSelIndex -1) + ". Name: " + Effects.GetSelText
                 Effectname1 = Effects.GetSelText
                 if AutomaticPreview.IsChecked = TRUE then sleep 1
                 d = Effects.GetSelIndex
                 TabEmphasis.Ok
                 Kontext "Tasks"
'                 EffectSlideShow.Click
'                     Sleep (1)
'                     kontext "DocumentPresentation"
'                     if DocumentPresentation.Exists (15) then
'                         DocumentPresentation.TypeKeys "<Space>"
'                         Sleep (1)
'                         if DocumentPresentation.Exists (10) then
'                             DocumentPresentation.TypeKeys "<Space>"
'                             Sleep (1)
'                         endif
'                         if DocumentPresentation.Exists (10) then
'                             DocumentPresentation.TypeKeys "<Escape>"
'                         endif
'                     else
'                         Warnlog "Effect: " + Effectname1 + " didn't work when it started with: " + StartName1 + "and ended with: " + PropertyName1
'                     endif
'                     sleep (5)
                     Kontext "Tasks"
                 if Tasks.Exists then
                     EffectChange.Click
                 else
                     Printlog "Presentation didn't end. Error?"
                 endif
                 kontext
                 active.setPage(TabEmphasis)
                 Printlog "Testing effects in - TabEmphasis"
                 kontext "TabEmphasis"
                 printlog "Select the next entry"
                 Effects.TypeKeys "<DOWN>", e + 1 
                 p = Effects.GetSelIndex
                 If p = d Then e = i 'p > 2 AND
             Next e
             kontext
             active.setPage(TabEmphasis)
             kontext "TabEmphasis"

             e = 0
	     if Speed.GetItemCount = 0 then
	        warnlog "   'Speed' has 0 entries. Check it."
             else
                do while (e < 1)
                   e = Int(Speed.GetItemCount * Rnd)
                loop 
                Speed.Select (e)
             endif
             Printlog "Test of Entrance-list ended."
         Next o
      else
         bError = true
         warnlog "Impress:Tasks Pane:Custom Animation:TabExit tabPage doesnt work."
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
   kontext "Tasks"
   EffectAutomaticPreview.Check
   printlog "click button 'Remove'"
   EffectRemove.Click
   printlog "close application"
   Call hCloseDocument
endcase 'tiPengineTabPageEmphasis2

'------------------------------------------------------------------------------------

testcase tiPengineTabPageExit2

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
    dim w as integer
    dim Effectname1 as string
    dim StartName1 as string
    dim PropertyName1 as string

printlog "open application"
Call hNewDocument
printlog "create textbox with text"
Call hTextrahmenErstellen ("Test text to test text effects", 35, 35, 70, 70 )
printlog "Slide Show->Custom Animation..."
SlideShowCustomAnimation
   Kontext "Tasks"
   printlog "Uncheck AutomaticPreview"
   EffectAutomaticPreview.UnCheck
   printlog "click button 'Add...'"
   EffectAdd.click
   printlog "Dialog 'Custom Animation' comes up"
   kontext
 
   printlog "Switch to TabPage: Exit"
   active.setPage(TabExit)
   kontext "TabExit"
   if TabExit.exists(5) then
       Printlog "Testing effects in - TabExit"
       i = Effects.GetItemCount
       For o = 1 to 2
           p = 7555
           Effects.TypeKeys "<HOME>"
           For e = 1 to i
               sleep 1
               Printlog "Effect Nr: " + (Effects.GetSelIndex -1) + ". Name: " + Effects.GetSelText
               Effectname1 = Effects.GetSelText
               if AutomaticPreview.IsChecked = TRUE then sleep 1
               d = Effects.GetSelIndex
               TabExit.Ok
               Kontext "Tasks"
'               EffectSlideShow.Click
'               Sleep (3)
'               kontext "DocumentPresentation"
'               if DocumentPresentation.Exists (15) then
'                  DocumentPresentation.TypeKeys "<Space>"
'                  Sleep (7)
'                  if DocumentPresentation.Exists (15) then
'                      DocumentPresentation.TypeKeys "<Space>"
'                  endif
'                  if DocumentPresentation.Exists (15) then
'                      DocumentPresentation.TypeKeys "<Escape>"
'                  endif
'                  if DocumentPresentation.Exists (15) then
'                      DocumentPresentation.TypeKeys "<Escape>"
'                  endif
'               else
'                  Warnlog "Effect: " + Effectname1 + " didn't work when it started with: " + StartName1 + "and ended with: " + PropertyName1
'               endif
'               sleep (5)
               kontext "DocumentImpress"
               if DocumentImpress.Exists (15) then 
                   printlog "Returned to the Impress Edit-mode."
               else
                   kontext "DocumentPresentation"
                   DocumentPresentation.TypeKeys "<Escape>"
                   sleep (4)
               endif
               
               Kontext "Tasks"
               if Tasks.Exists then
                  EffectChange.Click
               else
                  Printlog "Presentation didn't end. Error?"
               endif
               kontext
               printlog "Switch to TabPage: Exit"
               active.setPage(TabExit)
               kontext "TabExit"
               printlog "Select the next entry"
               Effects.TypeKeys "<DOWN>", e + 1 
               p = Effects.GetSelIndex
               If p = d Then e = i 'p > 2 AND
           Next e
           kontext
           printlog "Switch to TabPage: Exit"
           active.setPage(TabExit)
           kontext "TabExit"
           e = 0
           if Speed.GetItemCount = 0 then
	          warnlog "   'Speed' has 0 entries. Check it."
           else
              do while (e < 1)
                 e = Int(Speed.GetItemCount * Rnd)
              loop 
              Speed.Select (e)
           endif

           Printlog "Test of Entrance-list ended."
       Next o
       bError = false
   else
      bError = true
      warnlog "Impress:Tasks Pane:Custom Animation:TabExit tabPage doesnt work."
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
   kontext "Tasks"
   EffectAutomaticPreview.Check
   printlog "click button 'Remove'"
   EffectRemove.Click
   printlog "close application"
   Call hCloseDocument
endcase 'tiPengineTabPageExit2

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tiPengineTabPageMotionPaths2

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
    dim w as integer
    dim Effectname1 as string
    dim StartName1 as string
    dim PropertyName1 as string

    printlog "open application"
    Call hNewDocument
    printlog "create textbox with text"
    Call hTextrahmenErstellen ("Test text to test text effects", 35, 35, 70, 70 )
    printlog "Slide Show->Custom Animation..."
    SlideShowCustomAnimation
      Kontext "Tasks"
      printlog "Uncheck AutomaticPreview"
      EffectAutomaticPreview.UnCheck
      printlog "click button 'Add...'"
      EffectAdd.click
      printlog "Dialog 'Custom Animation' comes up"
      kontext

      printlog "Switch to TabPage: Motion Paths"
      active.setPage(TabMotionPaths)
      kontext "TabMotionPaths"
      if TabMotionPaths.exists(5) then
         Printlog "Testing effects in - TabMotionPaths"
         i = Effects.GetItemCount
         For o = 1 to 2
             p = 7555
             Effects.TypeKeys "<DOWN>",4
             For e = 4 to i '1 when adepted.
                 sleep 1
                 Printlog "Effect Nr: " + (Effects.GetSelIndex -1) + ". Name: " + Effects.GetSelText
                 Effectname1 = Effects.GetSelText
                 if AutomaticPreview.IsChecked = TRUE then sleep 1
                 d = Effects.GetSelIndex
                 TabMotionPaths.Ok
                 Kontext "Tasks"
'                 EffectSlideShow.Click
'                    Sleep (4)
'                    kontext "DocumentPresentation"
'                    if DocumentPresentation.Exists (15) then
'                       DocumentPresentation.TypeKeys "<Space>"
'                       Sleep (7)
'                       if DocumentPresentation.Exists (15) then
'                           DocumentPresentation.TypeKeys "<Space>"
'                       endif
'                       if DocumentPresentation.Exists (15) then
'                           DocumentPresentation.TypeKeys "<Escape>"
'                       endif
'                    else
'                       Warnlog "Effect: " + Effectname1 + " didn't work when it started with: " + StartName1 + "and ended with: " + PropertyName1
'                    endif
'                    sleep (2)
                    kontext "DocumentPresentation"
                    EditSelectAll ' (Select the text again)
                 Kontext "Tasks"
                 if Tasks.Exists then
                    EffectChange.Click
                 else
                    Printlog "Presentation didn't end. Error?"
                 endif
                 kontext
                 printlog "Switch to TabPage: Motion Paths"
                 active.setPage(TabMotionPaths)
                 kontext "TabMotionPaths"
                 printlog "Select the next entry"
                 Effects.TypeKeys "<DOWN>", e + 1 
                  p = Effects.GetSelIndex
                  If p = d Then e = i 'p > 2 AND
             Next e
             kontext
             printlog "Switch to TabPage: Motion Paths"
             active.setPage(TabMotionPaths)
             kontext "TabMotionPaths"

             e = 0
	     if Speed.GetItemCount = 0 then
	        warnlog "   'Speed' has 0 entries. Check it."
             else
                do while (e < 1)
                   e = Int(Speed.GetItemCount * Rnd)
                loop 
                Speed.Select (e)
             endif

             Printlog "Test of Entrance-list ended."
         Next o
         bError = false
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
   kontext "Tasks"
   EffectAutomaticPreview.Check
   printlog "click button 'Remove'"
   EffectRemove.Click
   printlog "close application"
   Call hCloseDocument
endcase 'tiPengineTabPageMotionPaths2

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------