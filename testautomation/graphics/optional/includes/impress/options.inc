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
'*
' #1 tiPEngineOptionsTest
' #1 tiPEngineOptionsTest2
'*
'\*******************************************************************

function tiPEngineOptionsTest
    dim filedialogue as boolean
    dim lala, optsound, os, oa, odc, ota, ets, etspeed, etrep, etshap, etgt as integer

     Kontext "Tasks"
        printlog "CLick on button '...' (Options)"
        EffectOptions.Click
        kontext "TabEffect"
        if TabEffect.Exists(5) then
           optsound = Sound.GetItemCount
           for os = 1 to optsound
               Sound.Select os
               kontext "OeffnenDlg"
               if OeffnenDlg.Exists (5) then
                  filedialogue = TRUE
                  OeffnenDlg.Close
                  kontext "TabEffect"
               else
                  kontext "TabEffect"
               endif
           next os
           if AfterAnimation.isEnabled AND AfterAnimation.isVisible then
              for oa = 1 to AfterAnimation.GetItemCount
                  AfterAnimation.Select oa
                  if DimColor.isEnabled then
                     for odc = 1 to DimColor.GetItemCount
                         DimColor.Select odc
                     next odc
                  endif
                  if DelayBetweenCharacters.isEnabled then
                     for odc = 1 to DelayBetweenCharacters.GetItemCount
                         DelayBetweenCharacters.Select odc
                     next odc
                  endif
              next oa
           else
              if DelayBetweenCharacters.isEnabled then
                 for odc = 1 to DelayBetweenCharacters.GetItemCount
                     DelayBetweenCharacters.Select odc
                 next odc
              endif
           endif
           for ota = 1 to TextAnimation.GetItemCount
               TextAnimation.Select ota
           next ota
           printlog "switch to TabPage 'Timing'"
           Kontext
           Active.SetPage TabTiming
           kontext "TabTiming"
           if TabTiming.Exists(5) then
              for ets = 1 to TimingStart.GetItemCount
                  TimingStart.Select ets
              next ets
              if Delay.isVisible AND Delay.isEnabled then
                 Delay.GetText
              else
                 Warnlog "Delay in Effect Options were not to be found."
              endif
              if Speed.isVisible AND Speed.isEnabled then
                 for etspeed = 1 to Speed.GetItemCount
                     Speed.Select etspeed
                 next etspeed
              else
                 printlog " No Speed-entry for this effect."
              endif
              if Repeat.isVisible AND Repeat.isEnabled then
                 for etrep = 1 to Speed.GetItemCount
                 Repeat.Select etrep
                 next etrep
              else
                 Printlog "Repeat in Effect Options were not to be found."
              endif
              Rewind.Check
              Rewind.UnCheck
              TriggerAnimate.IsChecked
              TriggerStart.IsChecked
              if Shape.isVisible AND Shape.isEnabled then
                 for etshap = 1 to Shape.GetItemCount
                 Shape.Select etshap
                 next etshap
              else
                 Warnlog "Shape in Effect Options were not to be found."
              endif
           else
              warnlog "Impress:Tasks Pane:Custom Animation:Effect Options: Timing TabPage didn't work."
           endif
           printlog "switch to TabPage 'Timing'"
           Kontext
           active.setPage TabTextAnimation
           kontext "TabTextAnimation"
           if TabTextAnimation.Exists(5) then
              lala = GroupText.GetItemCount
              for etgt = 1 to lala
                  GroupText.Select etgt
                  if AutomaticallyAfter.IsEnabled then
                     AutomaticallyAfter.Check
                     AutomaticallyAfter.TypeKeys "<UP>"
                  endif
                  if AnimateAttachedShape.IsEnabled then
                     AnimateAttachedShape.Check
                     if AnimateAttachedShape.IsChecked = FALSE then
                        Warnlog "AnimateAttachedShape should have been checked"
                     endif
                  endif
                  if InreverseOrder.IsEnabled then
                     InreverseOrder.Check
                     if InreverseOrder.IsChecked = FALSE then
                        Warnlog "InreverseOrder should have been checked"
                     endif
                  endif
              next etgt
              TabTextAnimation.Cancel
           else
              warnlog "Impress:Tasks Pane:Custom Animation:Effect Options: TextAnimation TabPage didn't work."
           endif
        else
           warnlog "Impress:Tasks Pane:Custom Animation:... button didn't work."
        endif
        Kontext "Tasks"
end function 'tiPEngineOptionsTest

'-------------------------------------------------------------------------------

function optionstest2
    dim filedialogue as boolean
    dim lala, optsound, os, oa, odc, ota, ets, etspeed, etrep, etshap, etgt as integer

     Kontext "Tasks"
        EffectOptions.Click
        kontext "TabEffect"
        if TabEffect.Exists(5) then
           Sound.Select 5
           AfterAnimation.Select 2
           if DimColor.isEnabled then
              DimColor.Select 5
           else
              Warnlog "DimColor should have been enabled"
           endif
           TextAnimation.Select 3
           if DelayBetweenCharacters.isEnabled then
              DelayBetweenCharacters.More 5
           else
              Warnlog "DelayBetweenCharacters should have been enabled"
           endif
           printlog "switch to TabPage 'Timing'"
           Kontext
           Active.SetPage TabTiming
           kontext "TabTiming"
           if TabTiming.Exists(5) then
              TimingStart.Select 2
              if Delay.isVisible AND Delay.isEnabled then
                 Delay.More 5
              else
                 Warnlog "Delay in Effect Options were not to be found."
              endif
              if Speed.isVisible AND Speed.isEnabled then
                 Speed.Select 3
              else
                 Printlog "Speed in Effect Options were not to be found."
              endif
              if Repeat.isVisible AND Repeat.isEnabled then
                 for etrep = 1 to Speed.GetItemCount
                 Repeat.Select etrep
                 next etrep
              else
                 Warnlog "Repeat in Effect Options were not to be found."
              endif
              if Rewind.isVisible AND Rewind.isEnabled then
                 Rewind.Check
              else
                 Printlog "Rewind in Effect Options were not to be found."
              endif
              if Rewind.isVisible AND Rewind.isEnabled then
                 Rewind.Check
                 Rewind.UnCheck
              else
                 Warnlog "Rewind in Effect Options were not to be found."
              endif
              TriggerAnimate.IsChecked
              TriggerStart.IsChecked
              if Shape.isVisible AND Shape.isEnabled then
                 for etshap = 1 to Shape.GetItemCount
                 Shape.Select etshap
                 next etshap
              else
                 Warnlog "Shape in Effect Options were not to be found."
              endif
           else
              warnlog "Impress:Tasks Pane:Custom Animation:Effect Options: Timing TabPage didn't work."
           endif
           printlog "switch to TabPage 'Timing'"
           Kontext
           active.setPage TabTextAnimation
           kontext "TabTextAnimation"
           if TabTextAnimation.Exists(5) then
              lala = GroupText.GetItemCount
              for etgt = 1 to lala
                  GroupText.Select etgt
                  if AutomaticallyAfter.IsEnabled then
                     AutomaticallyAfter.Check
                     AutomaticallyAfter.TypeKeys "<UP>"
                  endif
                  if AnimateAttachedShape.IsEnabled then
                     AnimateAttachedShape.Check
                     if AnimateAttachedShape.IsChecked = FALSE then
                        Warnlog "AnimateAttachedShape should have been checked"
                     endif
                  endif
                  if InreverseOrder.IsEnabled then
                     InreverseOrder.Check
                     if InreverseOrder.IsChecked = FALSE then
                        Warnlog "InreverseOrder should have been checked"
                     endif
                  endif
              next etgt
              TabTextAnimation.Cancel
           else
              warnlog "Impress:Tasks Pane:Custom Animation:Effect Options: TextAnimation TabPage didn't work."
           endif
        else
           warnlog "Impress:Tasks Pane:Custom Animation:... button didn't work."
        endif
        Kontext "Tasks"
end function 'optionstest2