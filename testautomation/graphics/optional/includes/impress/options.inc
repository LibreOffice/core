'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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
