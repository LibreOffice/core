'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_ctrl_1.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-13 10:27:08 $
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
'* owner : marc.neumann@sun.com
'*
'* short description : 
'*
'*******************************************************************
'*
' #1 hChangeControlSettings
'*
'\******************************************************************

function hChangeControlSettings ( sType as String, lsProps( ) ) as Boolean

   if bAsianLan = TRUE then
      printlog " ******************************************* "
      printlog " ***   running on asian office version   *** "
      printlog " ******************************************* "
   else
      printlog " ******************************************* "
      printlog " *** running on non-asian office version *** "
      printlog " ******************************************* "
   endif 

   Kontext "TabGeneralControl"
   if TabGeneralControl.Exists = FALSE then
      Kontext "TB_MacroControls"
      printlog "   activate properties for '" + sType + "'"
      Properties.Click
      Kontext "TabGeneralControl"
      Sleep (1)
   end if

   printlog "- change global settings"
   NameText.SetText "tt_" + sType + "_tt"
   ListAppend ( lsProps(), NameText.GetText )
   
   if Enabled.GetSelIndex = 1 then
      Enabled.Select 2
   else
      Enabled.Select 1
   end if
   
   ListAppend ( lsProps(), Enabled.GetSelText )
   
   if sType <> "dialog" then
      if Printable.GetSelIndex = 1 then
         Printable.Select 2
      else
         Printable.Select 1
      end if
      ListAppend ( lsProps(), Printable.GetSelText )
   else
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
   end if
   
'      PageStep.More 2
'        ListAppend ( lsProps(), PageStep.GetText )
         ListAppend ( lsProps(), "not testable" )             ' dummy entry
   Order.More 2
   Height.Less 3
     ListAppend ( lsProps(), Order.GetText )                  ' have to be checked after another control is changed ( it depends on the number of controls )
     ListAppend ( lsProps(), Height.GetText )
   Width.More 4
     ListAppend ( lsProps(), Width.GetText )
'   PositionX.More 3
'     ListAppend ( lsProps(), PositionX.GetText )
         ListAppend ( lsProps(), "not testable" )             ' dummy entry
'   PositionY.Less 2
'     ListAppend ( lsProps(), PositionY.GetText )
         ListAppend ( lsProps(), "not testable" )             ' dummy entry
   Information.SetText "tt info"
     ListAppend ( lsProps(), Information.GetText )
   Help.SetText "tt help"
     ListAppend ( lsProps(), Help.GetText )
   HelpURL.SetText "www.mopo.de"
     ListAppend ( lsProps(), HelpURL.GetText )

   printlog "- change special settings for '" + sType + "'"
'##### CommandButton #####
   if instr ( lcase ( sType ), "commandbutton" ) then
'      SetControlType CTBrowseBox
'      Label.TypeKeys "tt_label_tt"
'      ListAppend ( lsProps(), Label.GetText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
      TabStop.Select 3
      ListAppend ( lsProps(), TabStop.GetSelText )
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 5 )
      else
         Font.Select ( 5 )
      endif 
      TabFont.OK
        
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), CharacterSet.GetText )
      Background.Select 17
      ListAppend ( lsProps(), Background.GetSelText )
'      ButtonType.Select 3
'      ListAppend ( lsProps(), ButtonType.GetSelText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
'      if State.GetSelIndex = 1 then
'         State.Select 2
'      else
'         State.Select 1
'      end if
'      ListAppend ( lsProps(), State.GetSelText )

      ListAppend ( lsProps(), "not testable" )             ' dummy entry
      if DefaultButton.GetSelIndex = 1 then
         DefaultButton.Select 2
      else
         DefaultButton.Select 2
      end if
      
      ListAppend ( lsProps(), DefaultButton.GetSelText )
      GraphicsButton.Click
      
      Kontext "GrafikEinfuegenDlg"
      Dateiname.SetText ( ConvertPath ( gTesttoolPath + "global\input\graf_inp\baer.tif" ) )
      DateiTyp.Select 1                               ' set the filter to 'all formats'
      Oeffnen.Click
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), Graphics.GetText )
      GraphicsAlignment.Select 4
      ListAppend ( lsProps(), GraphicsAlignment.GetSelText )
   end if

'##### ImageControl #####
   if instr ( lcase ( sType ), "imagecontrol" ) then
      Background.Select 14
      ListAppend ( lsProps(), Background.GetSelText )
      GraphicsButton.Click
      
      Kontext "GrafikEinfuegenDlg"
      Dateiname.SetText ( ConvertPath ( gTesttoolPath + "global\input\graf_inp\baer.tif" ) )
      DateiTyp.Select 1                               ' set the filter to 'all formats'
      Oeffnen.Click
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), Graphics.GetText )
'      if Scale.GetSelIndex = 1 then
'         Scale.Select 2
'      else
'         Scale.Select 1
'      end if
'        ListAppend ( lsProps(), Scale.GetSelText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
   end if

'##### CheckBox #####
   if instr ( lcase ( sType ), "checkbox" ) then
'      SetControlType CTBrowseBox
'      Label.TypeKeys "tt_label_tt"
'        ListAppend ( lsProps(), Label.GetText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
      TabStop.Select 3
      ListAppend ( lsProps(), TabStop.GetSelText )

'      if State.GetSelIndex = 1 then
'         State.Select 2
'      else
'         State.Select 1
'      end if
'        ListAppend ( lsProps(), State.GetSelText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry

'      if TriState.GetSelIndex = 1 then
'         TriState.Select 2
'      else
'         TriState.Select 1
'      end if
'        ListAppend ( lsProps(), TriState.GetSelText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
   end if

'##### OptionButton #####
   if instr ( lcase ( sType ), "optionbutton" ) then
'      SetControlType CTBrowseBox
'      Label.TypeKeys "tt_label_tt"
'        ListAppend ( lsProps(), Label.GetText )
         ListAppend ( lsProps(), "not testable" )             ' dummy entry
      TabStop.Select 3
      ListAppend ( lsProps(), TabStop.GetSelText )
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 5 )
      else
         Font.Select ( 5 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), CharacterSet.GetText )

'      if State.GetSelIndex = 1 then
'         State.Select 2
'      else
'         State.Select 1
'      end if
'        ListAppend ( lsProps(), State.GetSelText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
   end if

'##### Label #####
   if instr ( lcase ( sType ), "label" ) then
'      SetControlType CTBrowseBox
'      Label.TypeKeys "tt_label_tt"
'        ListAppend ( lsProps(), Label.GetText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
      TabStop.Select 3
      ListAppend ( lsProps(), TabStop.GetSelText )

      CharacterSetButton.Click

      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 5 )
      else
         Font.Select ( 5 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), CharacterSet.GetText )

      Align.Select 4
      ListAppend ( lsProps(), Align.GetSelText )
      Background.Select 20
      ListAppend ( lsProps(), Background.GetSelText )
      Border.Select 3
      ListAppend ( lsProps(), Border.GetSelText )
      
      if MultiLine.GetSelIndex = 1 then
         MultiLine.Select 2
      else
         MultiLine.Select 1
      end if
      
      ListAppend ( lsProps(), MultiLine.GetSelText )
   end if

'##### TextField #####
   if instr ( lcase ( sType ), "textfield" ) then
'      SetControlType CTBrowseBox
'      TextText.TypeKeys "tt_text_tt"
'        ListAppend ( lsProps(), TextText.GetText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
      MaxTextLen.More 5
      ListAppend ( lsProps(), MaxTextLen.GetText )
      
      if Readonly.GetSelIndex = 1 then
         Readonly.Select 2
      else
         Readonly.Select 1
      end if
      
      ListAppend ( lsProps(), Readonly.GetSelText )
      TabStop.Select 3
      ListAppend ( lsProps(), TabStop.GetSelText )
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 5 )
      else
         Font.Select ( 5 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), CharacterSet.GetText )
      Align.Select 1
      ListAppend ( lsProps(), Align.GetSelText )
      Background.Select 20
      ListAppend ( lsProps(), Background.GetSelText )
      Border.Select 3
      ListAppend ( lsProps(), Border.GetSelText )
      
      if MultiLine.GetSelIndex = 1 then
         MultiLine.Select 2
      else
         MultiLine.Select 1
      end if
      
      ListAppend ( lsProps(), MultiLine.GetSelText )
      
      if ManualLineBreak.GetSelIndex = 1 then
         ManualLineBreak.Select 2
      else
         ManualLineBreak.Select 1
      end if
      
      ListAppend ( lsProps(), ManualLineBreak.GetSelText )
      
      if HorizontalScroll.GetSelIndex = 1 then
         HorizontalScroll.Select 2
      else
         HorizontalScroll.Select 1
      end if
      
      ListAppend ( lsProps(), HorizontalScroll.GetSelText )
      
      if VerticalScroll.GetSelIndex = 1 then
         VerticalScroll.Select 2
      else
         VerticalScroll.Select 1
      end if
      
      ListAppend ( lsProps(), VerticalScroll.GetSelText )
      Password.SetText "t"
      ListAppend ( lsProps(), Password.GetText )
      
   end if

'##### Listbox #####
   if instr ( lcase ( sType ), "listbox" ) then
   
      if Readonly.GetSelIndex = 1 then
         Readonly.Select 2
      else
         Readonly.Select 1
      end if
      
      ListAppend ( lsProps(), Readonly.GetSelText )
      TabStop.Select 3
      ListAppend ( lsProps(), TabStop.GetSelText )
'      SetControlType CTBrowseBox
'      ListEntries.TypeKeys "tt_text_tt"
'        ListAppend ( lsProps(), ListEntries.GetText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 5 )
      else
         Font.Select ( 5 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), CharacterSet.GetText )
      Background.Select 1
      ListAppend ( lsProps(), Background.GetSelText )
      Border.Select 1
      ListAppend ( lsProps(), Border.GetSelText )
      
      if DropDown.GetSelIndex = 1 then
         DropDown.Select 2
      else
         DropDown.Select 1
      end if
      
      ListAppend ( lsProps(), DropDown.GetSelText )
      LineCount.Less 1
      ListAppend ( lsProps(), LineCount.GetText )
      
      if MultiSelection.GetSelIndex = 1 then
         MultiSelection.Select 2
      else
         MultiSelection.Select 1
      end if
      
      ListAppend ( lsProps(), MultiSelection.GetSelText )
   end if

'##### Combobox #####
   if instr ( lcase ( sType ), "combobox" ) then
      
      TextText.SetText "tt_text_tt"
      ListAppend ( lsProps(), TextText.GetText )
      MaxTextLen.More 2
      ListAppend ( lsProps(), MaxTextLen.GetText )
      
      if Readonly.GetSelIndex = 1 then
         Readonly.Select 2
      else
         Readonly.Select 1
      end if
      
      ListAppend ( lsProps(), Readonly.GetSelText )
      TabStop.Select 3
      ListAppend ( lsProps(), TabStop.GetSelText )
'      SetControlType CTBrowseBox
'      ListEntries.TypeKeys "tt_text_tt"
'        ListAppend ( lsProps(), ListEntries.GetText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 5 )
      else
         Font.Select ( 5 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), CharacterSet.GetText )
      Background.Select 1
      ListAppend ( lsProps(), Background.GetSelText )
      Border.Select 1
      ListAppend ( lsProps(), Border.GetSelText )
      
      if DropDown.GetSelIndex = 1 then
         DropDown.Select 2
      else
         DropDown.Select 1
      end if
      
      ListAppend ( lsProps(), DropDown.GetSelText )
      
      if AutoComplete.GetSelIndex = 1 then
         AutoComplete.Select 2
      else
         AutoComplete.Select 1
      end if
      
      ListAppend ( lsProps(), AutoComplete.GetSelText )
      LineCount.More 5
      ListAppend ( lsProps(), LineCount.GetText )
      Border.Select 2                    ' sometimes the line count is not saved correctly, when it was changed as last property
      Border.Select 1                    ' as work-around : change another property and then set it to the last entry
      
   end if

'##### Scrollbar #####
   if instr ( lcase ( sType ), "scrollbar" ) then
   
'      ScrollValue.More 5
'        ListAppend ( lsProps(), ScrollValue.GetText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
'      ScrollValueMax.Less 5
'        ListAppend ( lsProps(), ScrollValueMax.GetText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
'      LineIncrement.More 1
'        ListAppend ( lsProps(), LineIncrement.GetText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
'      BlockIncrement.Less 1
'        ListAppend ( lsProps(), BlockIncrement.GetText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
'      VisibleSize.Less 2
'        ListAppend ( lsProps(), VisibleSize.GetText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
'      if Orientation.GetSelIndex = 1 then
'         Orientation.Select 2
'      else
'         Orientation.Select 1
'      end if
'        ListAppend ( lsProps(), Orientation.GetSelText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
      Border.Select 2
      ListAppend ( lsProps(), Border.GetSelText )
   end if

'##### FrameControl #####
   if instr ( lcase ( sType ), "framecontrol" ) then
'      SetControlType CTBrowseBox
'      Label.TypeKeys "tt_label_tt"
'        ListAppend ( lsProps(), Label.GetText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 3 )
      else
         Font.Select ( 3 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), CharacterSet.GetText )
      
   end if

'##### progressBar #####
   if instr ( lcase ( sType ), "progressbar" ) then
   
'      ScrollValue.More 5
'        ListAppend ( lsProps(), ScrollValue.GetText )
         ListAppend ( lsProps(), "not testable" )             ' dummy entry
'      ScrollValueMax.Less 5
'        ListAppend ( lsProps(), ScrollValueMax.GetText )
         ListAppend ( lsProps(), "not testable" )             ' dummy entry
'      LineIncrement.More 1
'        ListAppend ( lsProps(), LineIncrement.GetText )
         ListAppend ( lsProps(), "not testable" )             ' dummy entry
'      BlockIncrement.Less 1
'        ListAppend ( lsProps(), BlockIncrement.GetText )
         ListAppend ( lsProps(), "not testable" )             ' dummy entry
'      VisibleSize.Less 2
'        ListAppend ( lsProps(), VisibleSize.GetText )
         ListAppend ( lsProps(), "not testable" )             ' dummy entry
'      if Orientation.GetSelIndex = 1 then
'         Orientation.Select 2
'      else
'         Orientation.Select 1
'      end if
'        ListAppend ( lsProps(), Orientation.GetSelText )
         ListAppend ( lsProps(), "not testable" )             ' dummy entry
      Background.Select 4
        ListAppend ( lsProps(), Background.GetSelText )
   end if

'##### FixedLine #####
   if instr ( lcase ( sType ), "fixedline" ) then
'      SetControlType CTBrowseBox
'      Label.TypeKeys "tt_label_tt"
'        ListAppend ( lsProps(), Label.GetText )
         ListAppend ( lsProps(), "not testable" )             ' dummy entry
'      if Orientation.GetSelIndex = 1 then
'         Orientation.Select 2
'      else
'         Orientation.Select 1
'      end if
'        ListAppend ( lsProps(), Orientation.GetSelText )
         ListAppend ( lsProps(), "not testable" )             ' dummy entry
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 3 )
      else
         Font.Select ( 3 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
        ListAppend ( lsProps(), CharacterSet.GetText )
   end if


'##### DateField #####
   if instr ( lcase ( sType ), "datefield" ) then
      if StrictFormat.GetSelIndex = 1 then
         StrictFormat.Select 2
      else
         StrictFormat.Select 1
      end if
        ListAppend ( lsProps(), StrictFormat.GetSelText )
      if Readonly.GetSelIndex = 1 then
         Readonly.Select 2
      else
         Readonly.Select 1
      end if
         ListAppend ( lsProps(), Readonly.GetSelText )
      TabStop.Select 2
        ListAppend ( lsProps(), TabStop.GetSelText )
      DateField.More 3
        ListAppend ( lsProps(), DateField.GetText )
      DateMin.More 3
        ListAppend ( lsProps(), DateMin.GetText )
      DateMax.Less 3
        ListAppend ( lsProps(), DateMax.GetText )
      DateFormat.Select 7
        ListAppend ( lsProps(), DateFormat.GetSelText )
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 3 )
      else
         Font.Select ( 3 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
        ListAppend ( lsProps(), CharacterSet.GetText )
      Background.Select 1
        ListAppend ( lsProps(), Background.GetSelText )
      Border.Select 1
        ListAppend ( lsProps(), Border.GetSelText )
      if DropDown.GetSelIndex = 1 then
         DropDown.Select 2
      else
         DropDown.Select 1
      end if
        ListAppend ( lsProps(), DropDown.GetSelText )
      if Spin.GetSelIndex = 1 then
         Spin.Select 2
      else
         Spin.Select 1
      end if
        ListAppend ( lsProps(), Spin.GetSelText )
   end if

'##### TimeField #####
   if instr ( lcase ( sType ), "timefield" ) then
   
      if StrictFormat.GetSelIndex = 1 then
         StrictFormat.Select 2
      else
         StrictFormat.Select 1
      end if
      
      ListAppend ( lsProps(), StrictFormat.GetSelText )
      
      if Readonly.GetSelIndex = 1 then
         Readonly.Select 2
      else
         Readonly.Select 1
      end if
      
      ListAppend ( lsProps(), Readonly.GetSelText )
      TabStop.Select 1
      ListAppend ( lsProps(), TabStop.GetSelText )
      TimeField.Less 3
      ListAppend ( lsProps(), TimeField.GetText )
      TimeMin.More 2
      ListAppend ( lsProps(), TimeMin.GetText )
      TimeMax.Less 3
      ListAppend ( lsProps(), TimeMax.GetText )
      TimeFormat.Select 3
      ListAppend ( lsProps(), TimeFormat.GetSelText )
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 3 )
      else
         Font.Select ( 3 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), CharacterSet.GetText )
      Background.Select 1
      ListAppend ( lsProps(), Background.GetSelText )
      Border.Select 1
      ListAppend ( lsProps(), Border.GetSelText )
      
      if Spin.GetSelIndex = 1 then
         Spin.Select 2
      else
         Spin.Select 1
      end if
      
      ListAppend ( lsProps(), Spin.GetSelText )
   end if

'##### NumericField #####
   if instr ( lcase ( sType ), "numericfield" ) then
   
      if StrictFormat.GetSelIndex = 1 then
         StrictFormat.Select 2
      else
         StrictFormat.Select 1
      end if
      
      ListAppend ( lsProps(), StrictFormat.GetSelText )
      
      if Readonly.GetSelIndex = 1 then
         Readonly.Select 2
      else
         Readonly.Select 1
      end if
      
      ListAppend ( lsProps(), Readonly.GetSelText )
      TabStop.Select 1
      ListAppend ( lsProps(), TabStop.GetSelText )
      Value.Less 3
      ListAppend ( lsProps(), Value.GetText )
      ValueMin.Less 2
      ListAppend ( lsProps(), ValueMin.GetText )
      ValueMax.Less 3
      ListAppend ( lsProps(), ValueMax.GetText )
      ValueStep.More 5
      ListAppend ( lsProps(), ValueStep.GetText )
      Accuray.More 3
      ListAppend ( lsProps(), Accuray.GetText )
      
      if ThousandSeperator.GetSelIndex = 1 then
         ThousandSeperator.Select 2
      else
         ThousandSeperator.Select 1
      end if
      
      ListAppend ( lsProps(), ThousandSeperator.GetSelText )
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 3 )
      else
         Font.Select ( 3 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
        ListAppend ( lsProps(), CharacterSet.GetText )
      Background.Select 1
        ListAppend ( lsProps(), Background.GetSelText )
      Border.Select 1
        ListAppend ( lsProps(), Border.GetSelText )
      if Spin.GetSelIndex = 1 then
         Spin.Select 2
      else
         Spin.Select 1
      end if
        ListAppend ( lsProps(), Spin.GetSelText )
   end if


'##### CurrencyField #####
   if instr ( lcase ( sType ), "currencyfield" ) then
   
      if StrictFormat.GetSelIndex = 1 then
         StrictFormat.Select 2
      else
         StrictFormat.Select 1
      end if
      
      ListAppend ( lsProps(), StrictFormat.GetSelText )
      
      if Readonly.GetSelIndex = 1 then
         Readonly.Select 2
      else
         Readonly.Select 1
      end if
      
      ListAppend ( lsProps(), Readonly.GetSelText )
      TabStop.Select 1
      ListAppend ( lsProps(), TabStop.GetSelText )
      Value.Less 3
      ListAppend ( lsProps(), Value.GetText )
      ValueMin.Less 2
      ListAppend ( lsProps(), ValueMin.GetText )
      ValueMax.Less 3
      ListAppend ( lsProps(), ValueMax.GetText )
      ValueStep.More 5
      ListAppend ( lsProps(), ValueStep.GetText )
      Accuray.More 2
      ListAppend ( lsProps(), Accuray.GetText )
        
      if ThousandSeperator.GetSelIndex = 1 then
         ThousandSeperator.Select 2
      else
         ThousandSeperator.Select 1
      end if
      
      ListAppend ( lsProps(), ThousandSeperator.GetSelText )
      CurrencySymbol.SetText "#"
      ListAppend ( lsProps(), CurrencySymbol.GetText )
      
      if CurrSymPosition.GetSelIndex = 1 then
         CurrSymPosition.Select 2
      else
         CurrSymPosition.Select 1
      end if
      
      ListAppend ( lsProps(), CurrSymPosition.GetSelText )
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 3 )
      else
         Font.Select ( 3 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), CharacterSet.GetText )
      Background.Select 1
      ListAppend ( lsProps(), Background.GetSelText )
      Border.Select 1
      ListAppend ( lsProps(), Border.GetSelText )
      
      if Spin.GetSelIndex = 1 then
         Spin.Select 2
      else
         Spin.Select 1
      end if
      
      ListAppend ( lsProps(), Spin.GetSelText )
   end if

'##### FormattedField #####
   if instr ( lcase ( sType ), "formattedfield" ) then
      MaxTextLen.More 4
      ListAppend ( lsProps(), MaxTextLen.GetText )
      
      if StrictFormat.GetSelIndex = 1 then
         StrictFormat.Select 2
      else
         StrictFormat.Select 1
      end if
      
      ListAppend ( lsProps(), StrictFormat.GetSelText )
      
      if Readonly.GetSelIndex = 1 then
         Readonly.Select 2
      else
         Readonly.Select 1
      end if
      
      ListAppend ( lsProps(), Readonly.GetSelText )
      TabStop.Select 1
      ListAppend ( lsProps(), TabStop.GetSelText )
'      Effective.SetText "2"
'        ListAppend ( lsProps(), Effective.GetText )
      ListAppend ( lsProps(), "not testable" )             ' dummy entry
      EffectiveMin.SetText "1"
      ListAppend ( lsProps(), EffectiveMin.GetText )
      EffectiveMax.SetText "1"
      ListAppend ( lsProps(), EffectiveMax.GetText )
      FormatkeyButton.Click
      
      Kontext "ZahlenFormat"
      Kategorie.Select 3
      Kategorieformat.Select ( Kategorieformat.GetItemCount )
      ZahlenFormat.OK
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), FormatKey.GetText )
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 3 )
      else
         Font.Select ( 3 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), CharacterSet.GetText )
      Align.Select 1
      ListAppend ( lsProps(), Align.GetSelText )
      Background.Select 1
      ListAppend ( lsProps(), Background.GetSelText )
      Border.Select 1
      ListAppend ( lsProps(), Border.GetSelText )
        
      if Spin.GetSelIndex = 1 then
         Spin.Select 2
      else
         Spin.Select 1
      end if
      
      ListAppend ( lsProps(), Spin.GetSelText )
   end if

'##### PatternField #####
   if instr ( lcase ( sType ), "patternfield" ) then
      TextText.SetText "tt_text_tt"
      ListAppend ( lsProps(), TextText.GetText )
      MaxTextLen.More 4
      ListAppend ( lsProps(), MaxTextLen.GetText )
      EditMask.SetText "aeiopu"
      ListAppend ( lsProps(), EditMask.GetText )
      LiteralMask.SetText "upqpsd"
      ListAppend ( lsProps(), LiteralMask.GetText )
      
      if StrictFormat.GetSelIndex = 1 then
         StrictFormat.Select 2
      else
         StrictFormat.Select 1
      end if
      
      ListAppend ( lsProps(), StrictFormat.GetSelText )
        
      if Readonly.GetSelIndex = 1 then
         Readonly.Select 2
      else
         Readonly.Select 1
      end if
      
      ListAppend ( lsProps(), Readonly.GetSelText )
      TabStop.Select 1
      ListAppend ( lsProps(), TabStop.GetSelText )
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 3 )
      else
         Font.Select ( 3 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), CharacterSet.GetText )
      Background.Select 1
      ListAppend ( lsProps(), Background.GetSelText )
      Border.Select 1
      ListAppend ( lsProps(), Border.GetSelText )
   end if

'##### FileControl #####
   if instr ( lcase ( sType ), "filecontrol" ) then
      TextText.SetText "tt_text_tt"
      ListAppend ( lsProps(), TextText.GetText )
      
      if Readonly.GetSelIndex = 1 then
         Readonly.Select 2
      else
         Readonly.Select 1
      end if
      
      ListAppend ( lsProps(), Readonly.GetSelText )
      TabStop.Select 1
      ListAppend ( lsProps(), TabStop.GetSelText )
      CharacterSetButton.Click
      
      Kontext "TabFont"
      if bAsianLan = TRUE then
         FontEast.Select ( 3 )
      else
         Font.Select ( 3 )
      endif 
      TabFont.OK
      
      Kontext "TabGeneralControl"
      ListAppend ( lsProps(), CharacterSet.GetText )
      Background.Select 1
      ListAppend ( lsProps(), Background.GetSelText )
      Border.Select 1
      ListAppend ( lsProps(), Border.GetSelText )
   end if

end function
