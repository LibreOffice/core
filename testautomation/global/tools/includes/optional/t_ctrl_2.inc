'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
'/************************************************************************
'*
'* owner : marc.neumann@sun.com
'*
'* short description : 
'*
'*******************************************************************
'*
' #1 hCheckControlSettings
'*
'\******************************************************************

function hCheckControlSettings ( sType as String, lsProps( ) ) as Boolean

  Kontext "TabGeneralControl"
  if TabGeneralControl.Exists = FALSE then
     Kontext "TB_MacroControls"
  printlog "   activate properties for '" + sType + "'"
     Properties.Click
     Kontext "TabGeneralControl"
     Sleep (1)
  end if

  printlog "- check global settings"
   if NameText.GetText   <> lsProps(1) then Warnlog "Name => changes not saved!  should : '" + lsProps(1) + "'   is : '" + NameText.GetText + "'"
   if Enabled.GetSelText <> lsProps(2) then Warnlog "Enabled => changes not saved!  should : '" + lsProps(2) + "'   is : '" + Enabled.GetSelText + "'"
   if sType <> "dialog" then
      if Printable.GetSelText <> lsProps(3) then Warnlog "Printable => changes not saved!  should : '" + lsProps(3) + "'   is : '" + Printable.GetSelText + "'"
   end if
'   if PageStep.GetText <> lsProps(4) then Warnlog "Page Step => changes not saved!  should : '" + lsProps(4) + "'   is : '" + PageStep.GetText + "'"
   if Order.GetText <> lsProps(5) then Warnlog "Order => changes not saved!  should : '" + lsProps(5) + "'   is : '" + Order.GetText + "'"
   if Height.GetText <> lsProps(6) then Warnlog "Height => changes not saved!  should : '" + lsProps(6) + "'   is : '" + Height.GetText + "'"
   if Width.GetText <> lsProps(7) then Warnlog "Width => changes not saved!  should : '" + lsProps(7) + "'   is : '" + Width.GetText + "'"
'   if PositionX.GetText <> lsProps(8) then Warnlog "Position x => changes not saved!  should : '" + lsProps(8) + "'   is : '" + PositionX.GetText + "'"
'   if PositionY.GetText <> lsProps(9) then Warnlog "Position y => changes not saved!  should : '" + lsProps(9) + "'   is : '" + PositionY.GetText + "'"
   if Information.GetText <> lsProps(10) then Warnlog "Information => changes not saved!  should : '" + lsProps(10) + "'   is : '" + Information.GetText + "'"
   if Help.GetText <> lsProps(11) then Warnlog "Help => changes not saved!  should : '" + lsProps(11) + "'   is : '" + Help.GetText + "'"
   if HelpURL.GetText <> lsProps(12) then Warnlog "Help URL => changes not saved!  should : '" + lsProps(12) + "'   is : '" + HelpURL.GetText + "'"

   printlog "- check special settings for " + sType

'##### CommandButton #####
   if instr ( lcase ( sType ), "commandbutton" ) then
'      if Label.GetText <> lsProps(13) then Warnlog "Label => changes not saved!  should : '" + lsProps(13) + "'   is : '" + Label.GetText + "'"
      if TabStop.GetSelText <> lsProps(14) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(14) + "'   is : '" + TabStop.GetSelText + "'"
      if CharacterSet.GetText <> lsProps(15) then Warnlog "Character set => changes not saved!  should : '" + lsProps(15) + "'   is : '" + CharacterSet.GetText + "'"
      if Background.GetSelText <> lsProps(16) then Warnlog "Background => changes not saved!  should : '" + lsProps(16) + "'   is : '" + Background.GetSelText + "'"
'      if ButtonType.GetSelText <> lsProps(17) then Warnlog "Button type => changes not saved!  should : '" + lsProps(17) + "'   is : '" + ButtonType.GetSelText + "'"
'      if State.GetSelText <> lsProps(18) then Warnlog "State => changes not saved!  should : '" + lsProps(18) + "'   is : '" + State.GetSelText + "'"
      if DefaultButton.GetSelText <> lsProps(19) then Warnlog "Default button => changes not saved!  should : '" + lsProps(19) + "'   is : '" + DefaultButton.GetSelText + "'"
      if Graphics.GetText <> lsProps(20) then Warnlog "Graphics => changes not saved!  should : '" + lsProps(20) + "'   is : '" + Graphics.GetText + "'"
      if GraphicsAlignment.GetSelText <> lsProps(21) then Warnlog "Graphics alignment => changes not saved!  should : '" + lsProps(21) + "'   is : '" + GraphicsAlignment.GetSelText + "'"
   end if

'##### ImageControl #####
   if instr ( lcase ( sType ), "imagecontrol" ) then
      if Background.GetSelText <> lsProps(13) then Warnlog "Background => changes not saved!  should : '" + lsProps(13) + "'   is : '" + Background.GetSelText + "'"
      if Graphics.GetText <> lsProps(14) then Warnlog "Graphics => changes not saved!  should : '" + lsProps(14) + "'   is : '" + Graphics.GetText + "'"
'      if Scale.GetSelText <> lsProps(15) then Warnlog "Scale => changes not saved!  should : '" + lsProps(15) + "'   is : '" + Scale.GetSelText + "'"
   end if

'##### CheckBox #####
   if instr ( lcase ( sType ), "checkbox" ) then
'      if Label.GetText <> lsProps(13) then Warnlog "Label => changes not saved!  should : '" + lsProps(13) + "'   is : '" + Label.GetText + "'"
      if TabStop.GetSelText <> lsProps(14) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(14) + "'   is : '" + TabStop.GetSelText + "'"
'      if State.GetSelText <> lsProps(15) then Warnlog "State => changes not saved!  should : '" + lsProps(15) + "'   is : '" + State.GetSelText + "'"
'      if TriState.GetSelText <> lsProps(16) then Warnlog "TriState => changes not saved!  should : '" + lsProps(16) + "'   is : '" + TriState.GetSelText + "'"
   end if

'##### OptionButton #####
   if instr ( lcase ( sType ), "optionbutton" ) then
'      if Label.GetText <> lsProps(13) then Warnlog "Label => changes not saved!  should : '" + lsProps(13) + "'   is : '" + Label.GetText + "'"
      if TabStop.GetSelText <> lsProps(14) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(14) + "'   is : '" + TabStop.GetSelText + "'"
      if CharacterSet.GetText <> lsProps(15) then Warnlog "Character set => changes not saved!  should : '" + lsProps(15) + "'   is : '" + CharacterSet.GetText + "'"
'      if State.GetSelText <> lsProps(16) then Warnlog "State => changes not saved!  should : '" + lsProps(16) + "'   is : '" + State.GetSelText + "'"
   end if

'##### Label #####
   if instr ( lcase ( sType ), "label" ) then
'      if Label.GetText <> lsProps(13) then Warnlog "Label => changes not saved!  should : '" + lsProps(13) + "'   is : '" + Label.GetText + "'"
      if TabStop.GetSelText <> lsProps(14) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(14) + "'   is : '" + TabStop.GetSelText + "'"
      if CharacterSet.GetText <> lsProps(15) then Warnlog "Character set => changes not saved!  should : '" + lsProps(15) + "'   is : '" + CharacterSet.GetText + "'"
      if Align.GetSelText <> lsProps(16) then Warnlog "Alignment => changes not saved!  should : '" + lsProps(16) + "'   is : '" + Align.GetSelText + "'"
      if Background.GetSelText <> lsProps(17) then Warnlog "Background => changes not saved!  should : '" + lsProps(17) + "'   is : '" + Background.GetSelText + "'"
      if Border.GetSelText <> lsProps(18) then Warnlog "Border => changes not saved!  should : '" + lsProps(18) + "'   is : '" + Border.GetSelText + "'"
      if MultiLine.GetSelText <> lsProps(19) then Warnlog "MultiLine => changes not saved!  should : '" + lsProps(19) + "'   is : '" + MultiLine.GetSelText + "'"
   end if

'##### TextField #####
   if instr ( lcase ( sType ), "textfield" ) then
'      if TextText.GetText <> lsProps(13) then Warnlog "Text => changes not saved!  should : '" + lsProps(13) + "'   is : '" + TextText.GetText + "'"
      if MaxTextLen.GetText <> lsProps(14) then Warnlog "Max text lengh => changes not saved!  should : '" + lsProps(14) + "'   is : '" + MaxTextLen.GetText + "'"
      if Readonly.GetSelText <> lsProps(15) then Warnlog "Readonly => changes not saved!  should : '" + lsProps(15) + "'   is : '" + Readonly.GetSelText + "'"
      if TabStop.GetSelText <> lsProps(16) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(16) + "'   is : '" + TabStop.GetSelText + "'"
      if CharacterSet.GetText <> lsProps(17) then Warnlog "Character set => changes not saved!  should : '" + lsProps(17) + "'   is : '" + CharacterSet.GetText + "'"
      if Align.GetSelText <> lsProps(18) then Warnlog "Alignment => changes not saved!  should : '" + lsProps(18) + "'   is : '" + Align.GetSelText + "'"
      if Background.GetSelText <> lsProps(19) then Warnlog "Background => changes not saved!  should : '" + lsProps(19) + "'   is : '" + Background.GetSelText + "'"
      if Border.GetSelText <> lsProps(20) then Warnlog "Border => changes not saved!  should : '" + lsProps(20) + "'   is : '" + Border.GetSelText + "'"
      if MultiLine.GetSelText <> lsProps(21) then Warnlog "MultiLine => changes not saved!  should : '" + lsProps(21) + "'   is : '" + MultiLine.GetSelText + "'"
      if ManualLineBreak.GetSelText <> lsProps(22) then Warnlog "Manual line break => changes not saved!  should : '" + lsProps(22) + "'   is : '" + ManualLineBreak.GetSelText + "'"
      if HorizontalScroll.GetSelText <> lsProps(23) then Warnlog "Horizontal scrollbar => changes not saved!  should : '" + lsProps(23) + "'   is : '" + HorizontalScroll.GetSelText + "'"
      if VerticalScroll.GetSelText <> lsProps(24) then Warnlog "Vertical scrollbar => changes not saved!  should : '" + lsProps(24) + "'   is : '" + VerticalScroll.GetSelText + "'"
      if Password.GetText <> lsProps(25) then Warnlog "Password => changes not saved!  should : '" + lsProps(25) + "'   is : '" + Password.GetText + "'"
   end if

'##### Listbox #####
   if instr ( lcase ( sType ), "listbox" ) then
      if Readonly.GetSelText <> lsProps(13) then Warnlog "Readonly => changes not saved!  should : '" + lsProps(13) + "'   is : '" + Readonly.GetSelText + "'"
      if TabStop.GetSelText <> lsProps(14) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(14) + "'   is : '" + TabStop.GetSelText + "'"
'      if ListEntries.GetText <> lsProps(15) then Warnlog "List entries => changes not saved!  should : '" + lsProps(15) + "'   is : '" + ListEntries.GetText + "'"
      if CharacterSet.GetText <> lsProps(16) then Warnlog "Character set => changes not saved!  should : '" + lsProps(16) + "'   is : '" + CharacterSet.GetText + "'"
      if Background.GetSelText <> lsProps(17) then Warnlog "Background => changes not saved!  should : '" + lsProps(17) + "'   is : '" + Background.GetSelText + "'"
      if Border.GetSelText <> lsProps(18) then Warnlog "Border => changes not saved!  should : '" + lsProps(18) + "'   is : '" + Border.GetSelText + "'"
      if DropDown.GetSelText <> lsProps(19) then Warnlog "Drop Down => changes not saved!  should : '" + lsProps(19) + "'   is : '" + DropDown.GetSelText + "'"
      if LineCount.GetText <> lsProps(20) then Warnlog "Line counte => changes not saved!  should : '" + lsProps(20) + "'   is : '" + LineCount.GetText + "'"
      if MultiSelection.GetSelText <> lsProps(21) then Warnlog "Multi selection. => changes not saved!  should : '" + lsProps(21) + "'   is : '" + MultiSelection.GetSelText + "'"
   end if

'##### Combobox #####
   if instr ( lcase ( sType ), "combobox" ) then
      if TextText.GetText <> lsProps(13) then Warnlog "Text => changes not saved!  should : '" + lsProps(13) + "'   is : '" + TextText.GetText + "'"
      if MaxTextLen.GetText <> lsProps(14) then Warnlog "Max text lenght => changes not saved!  should : '" + lsProps(14) + "'   is : '" + MaxTextLen.GetText + "'"
      if Readonly.GetSelText <> lsProps(15) then Warnlog "Readonly => changes not saved!  should : '" + lsProps(15) + "'   is : '" + Readonly.GetSelText + "'"
      if TabStop.GetSelText <> lsProps(16) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(16) + "'   is : '" + TabStop.GetSelText + "'"
'      if ListEntries.GetText <> lsProps(17) then Warnlog "List entries => changes not saved!  should : '" + lsProps(17) + "'   is : '" + ListEntries.GetText + "'"
      if CharacterSet.GetText <> lsProps(18) then Warnlog "Character set => changes not saved!  should : '" + lsProps(18) + "'   is : '" + CharacterSet.GetText + "'"
      if Background.GetSelText <> lsProps(19) then Warnlog "Background => changes not saved!  should : '" + lsProps(19) + "'   is : '" + Background.GetSelText + "'"
      if Border.GetSelText <> lsProps(20) then Warnlog "Border => changes not saved!  should : '" + lsProps(20) + "'   is : '" + Border.GetSelText + "'"
      if DropDown.GetSelText <> lsProps(21) then Warnlog "Drop Down => changes not saved!  should : '" + lsProps(21) + "'   is : '" + DropDown.GetSelText + "'"
      if AutoComplete.GetSelText <> lsProps(22) then Warnlog "Auto complete => changes not saved!  should : '" + lsProps(22) + "'   is : '" + AutoComplete.GetSelText + "'"
      if LineCount.GetText <> lsProps(23) then Warnlog "Line count => changes not saved!  should : '" + lsProps(23) + "'   is : '" + LineCount.GetText + "'"
   end if

'##### Scrollbar #####
   if instr ( lcase ( sType ), "scrollbar" ) then
'      if ScrollValue.GetText <> lsProps(13) then Warnlog "Scroll value => changes not saved!  should : '" + lsProps(13) + "'   is : '" + ScrollValue.GetText + "'"
'      if ScrollValueMax.GetText <> lsProps(14) then Warnlog "Scroll value max => changes not saved!  should : '" + lsProps(14) + "'   is : '" + ScrollValueMax.GetText + "'"
'      if LineIncrement.GetText <> lsProps(15) then Warnlog "Line increment => changes not saved!  should : '" + lsProps(15) + "'   is : '" + LineIncrement.GetText + "'"
'      if BlockIncrement.GetText <> lsProps(16) then Warnlog "Block increment => changes not saved!  should : '" + lsProps(16) + "'   is : '" + BlockIncrement.GetText + "'"
'      if VisibleSize.GetText <> lsProps(17) then Warnlog "Visible size => changes not saved!  should : '" + lsProps(17) + "'   is : '" + VisibleSize.GetText + "'"
'      if Orientation.GetSelText <> lsProps(18) then Warnlog "Orientation => changes not saved!  should : '" + lsProps(18) + "'   is : '" + Orientation.GetSelText + "'"
      if Border.GetSelText <> lsProps(19) then Warnlog "Border => changes not saved!  should : '" + lsProps(19) + "'   is : '" + Border.GetSelText + "'"
   end if

'##### FrameControl #####
   if instr ( lcase ( sType ), "framecontrol" ) then
'      if Label.GetText <> lsProps(13) then Warnlog "Label => changes not saved!  should : '" + lsProps(13) + "'   is : '" + Label.GetText + "'"
      if CharacterSet.GetText <> lsProps(14) then Warnlog "Character set => changes not saved!  should : '" + lsProps(14) + "'   is : '" + CharacterSet.GetText + "'"
   end if


'##### progressBar #####
   if instr ( lcase ( sType ), "progressbar" ) then
'      if ScrollValue.GetText <> lsProps(13) then Warnlog "Scroll value => changes not saved!  should : '" + lsProps(13) + "'   is : '" + ScrollValue.GetText + "'"
'      if ScrollValueMax.GetText <> lsProps(14) then Warnlog "Scroll value max. => changes not saved!  should : '" + lsProps(14) + "'   is : '" + ScrollValueMax.GetText + "'"'
'      if LineIncrement.GetText <> lsProps(15) then Warnlog "Line increment => changes not saved!  should : '" + lsProps(15) + "'   is : '" + LineIncrement.GetText + "'"'
'      if BlockIncrement.GetText <> lsProps(16) then Warnlog "Block increment => changes not saved!  should : '" + lsProps(16) + "'   is : '" + BlockIncrement.GetText + "'"'
'      if VisibleSize.GetText <> lsProps(17) then Warnlog "Visible size => changes not saved!  should : '" + lsProps(17) + "'   is : '" + VisibleSize.GetText + "'"'
'      if Orientation.GetSelText <> lsProps(18) then Warnlog "Orientation => changes not saved!  should : '" + lsProps(18) + "'   is : '" + Orientation.GetSelTextGetText + "'"'
      if Background.GetSelText <> lsProps(19) then Warnlog "Background => changes not saved!  should : '" + lsProps(19) + "'   is : '" + Background.GetSelText + "'"
   end if

'##### FixedLine #####
   if instr ( lcase ( sType ), "fixedline" ) then
'      if Label.GetText <> lsProps(13) then Warnlog "Label => changes not saved!  should : '" + lsProps(13) + "'   is : '" + Label.GetText + "'"
'      if Orientation.GetSelText <> lsProps(14) then Warnlog "Orientation => changes not saved!  should : '" + lsProps(14) + "'   is : '" + Orientation.GetSelTextGetText + "'"'
      if CharacterSet.GetText <> lsProps(15) then Warnlog "Character set => changes not saved!  should : '" + lsProps(15) + "'   is : '" + CharacterSet.GetText + "'"
   end if

'##### DateField #####
   if instr ( lcase ( sType ), "datefield" ) then
      if StrictFormat.GetSelText <> lsProps(13) then Warnlog "Strict format => changes not saved!  should : '" + lsProps(13) + "'   is : '" + StrictFormat.GetSelText + "'"'
      if Readonly.GetSelText <> lsProps(14) then Warnlog "Read only => changes not saved!  should : '" + lsProps(14) + "'   is : '" + Readonly.GetSelText + "'"
      if TabStop.GetSelText <> lsProps(15) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(15) + "'   is : '" + TabStop.GetSelText + "'"
      if DateField.GetText <> lsProps(16) then Warnlog "Date => changes not saved!  should : '" + lsProps(16) + "'   is : '" + DateField.GetSelText + "'"
      if DateMin.GetText <> lsProps(17) then Warnlog "Date min. => changes not saved!  should : '" + lsProps(17) + "'   is : '" + DateMin.GetSelText + "'"
      if DateMax.GetText <> lsProps(18) then Warnlog "Date max => changes not saved!  should : '" + lsProps(18) + "'   is : '" + DateMax.GetSelText + "'"
      if DateFormat.GetSelText <> lsProps(19) then Warnlog "Date format => changes not saved!  should : '" + lsProps(19) + "'   is : '" + DateFormat.GetSelText + "'"
      if CharacterSet.GetText <> lsProps(20) then Warnlog "Character set => changes not saved!  should : '" + lsProps(20) + "'   is : '" + CharacterSet.GetText + "'"
      if Background.GetSelText <> lsProps(21) then Warnlog "Background => changes not saved!  should : '" + lsProps(21) + "'   is : '" + Background.GetSelText + "'"
      if Border.GetSelText <> lsProps(22) then Warnlog "Border => changes not saved!  should : '" + lsProps(22) + "'   is : '" + Border.GetSelText + "'"
      if DropDown.GetSelText <> lsProps(23) then Warnlog "Drop Down => changes not saved!  should : '" + lsProps(23) + "'   is : '" + DropDown.GetSelText + "'"
      if Spin.GetSelText <> lsProps(24) then Warnlog "Spin button => changes not saved!  should : '" + lsProps(24) + "'   is : '" + Spin.GetSelText + "'"
   end if

'##### TimeField #####
   if instr ( lcase ( sType ), "timefield" ) then
      if StrictFormat.GetSelText <> lsProps(13) then Warnlog "Strict format => changes not saved!  should : '" + lsProps(13) + "'   is : '" + StrictFormat.GetSelText + "'"'
      if Readonly.GetSelText <> lsProps(14) then Warnlog "Read only => changes not saved!  should : '" + lsProps(14) + "'   is : '" + Readonly.GetSelText + "'"
      if TabStop.GetSelText <> lsProps(15) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(15) + "'   is : '" + TabStop.GetSelText + "'"
      if TimeField.GetText <> lsProps(16) then Warnlog "Time => changes not saved!  should : '" + lsProps(16) + "'   is : '" + TimeField.GetText + "'"
      if TimeMin.GetText <> lsProps(17) then Warnlog "Time min. => changes not saved!  should : '" + lsProps(17) + "'   is : '" + TimeMin.GetText + "'"
      if TimeMax.GetText <> lsProps(18) then Warnlog "Time max => changes not saved!  should : '" + lsProps(18) + "'   is : '" + TimeMax.Getext + "'"
      if TimeFormat.GetSelText <> lsProps(19) then Warnlog "Time format => changes not saved!  should : '" + lsProps(19) + "'   is : '" + TimeFormat.GetSelText + "'"
      if CharacterSet.GetText <> lsProps(20) then Warnlog "Character set => changes not saved!  should : '" + lsProps(20) + "'   is : '" + CharacterSet.GetText + "'"
      if Background.GetSelText <> lsProps(21) then Warnlog "Background => changes not saved!  should : '" + lsProps(21) + "'   is : '" + Background.GetSelText + "'"
      if Border.GetSelText <> lsProps(22) then Warnlog "Border => changes not saved!  should : '" + lsProps(22) + "'   is : '" + Border.GetSelText + "'"
      if Spin.GetSelText <> lsProps(23) then Warnlog "Spin button => changes not saved!  should : '" + lsProps(23) + "'   is : '" + Spin.GetSelText + "'"
   end if

'##### NumericField #####
   if instr ( lcase ( sType ), "numericfield" ) then
      if StrictFormat.GetSelText <> lsProps(13) then Warnlog "Strict format => changes not saved!  should : '" + lsProps(13) + "'   is : '" + StrictFormat.GetSelText + "'"'
      if Readonly.GetSelText <> lsProps(14) then Warnlog "Read only => changes not saved!  should : '" + lsProps(14) + "'   is : '" + Readonly.GetSelText + "'"
      if TabStop.GetSelText <> lsProps(15) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(15) + "'   is : '" + TabStop.GetSelText + "'"

      if Value.GetText <> lsProps(16) then
         if Value.GetText <> lsProps(16) + "000" then
            Warnlog "Value => changes not saved!  should : '" + lsProps(16) + "'   is : '" + Value.GetText + "'"
         else
            Warnlog "Value => changes not saved!  should : '" + lsProps(16) + "'   is : '" + Value.GetText + "'"
         end if
      end if
      if ValueMin.GetText <> lsProps(17) then
         if ValueMin.GetText <> "-1,000,002.00000" then
            Warnlog "Value min. => changes not saved!  should : '-1,000,002.00000'   is : '" + ValueMin.GetText + "'"
         else
            Warnlog "Value min. => changes not saved!  should : '" + lsProps(17) + "'   is : '" + ValueMin.GetText + "'"
         end if
      end if
      if ValueMax.GetText <> lsProps(18) then
         if ValueMax.GetText <> "999,997.00000" then
            Warnlog "Value max => changes not saved!  should : '999,997.00000'   is : '" + ValueMax.GetText + "'"
         else
            Warnlog "Value max => changes not saved!  should : '" + lsProps(18) + "'   is : '" + ValueMax.GetText + "'"
         end if
      end if
      if ValueStep.GetText <> lsProps(19) then Warnlog "Incr./decrement value => changes not saved!  should : '" + lsProps(19) + "'   is : '" + ValueStep.GetText + "'"
      if Accuray.GetText <> lsProps(20) then Warnlog "Dec. accuracy => changes not saved!  should : '" + lsProps(20) + "'   is : '" + Accuray.GetText + "' Bugid #106852#"
      if ThousandSeperator.GetSelText <> lsProps(21) then Warnlog "Thousands seperator => changes not saved!  should : '" + lsProps(21) + "'   is : '" + ThousandSeperator.GetSelText + "' Bugid #106852#"
      if CharacterSet.GetText <> lsProps(22) then Warnlog "Character set => changes not saved!  should : '" + lsProps(22) + "'   is : '" + CharacterSet.GetText + "'"
      if Background.GetSelText <> lsProps(23) then Warnlog "Background => changes not saved!  should : '" + lsProps(23) + "'   is : '" + Background.GetSelText + "'"
      if Border.GetSelText <> lsProps(24) then Warnlog "Border => changes not saved!  should : '" + lsProps(24) + "'   is : '" + Border.GetSelText + "'"
      if Spin.GetSelText <> lsProps(25) then Warnlog "Spin button => changes not saved!  should : '" + lsProps(25) + "'   is : '" + Spin.GetSelText + "'"
   end if

'##### CurrencyField #####
   if instr ( lcase ( sType ), "currencyfield" ) then
      if StrictFormat.GetSelText <> lsProps(13) then Warnlog "Strict format => changes not saved!  should : '" + lsProps(13) + "'   is : '" + StrictFormat.GetSelText + "'"'
      if Readonly.GetSelText <> lsProps(14) then Warnlog "Read only => changes not saved!  should : '" + lsProps(14) + "'   is : '" + Readonly.GetSelText + "'"
      if TabStop.GetSelText <> lsProps(15) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(15) + "'   is : '" + TabStop.GetSelText + "'"
      if Value.GetText <> lsProps(16) then
         if Value.GetText <> lsProps(16) + "000" then
            Warnlog "Value => changes not saved!  should : '" + lsProps(16) + "'   is : '" + Value.GetText + "'"
         else
            Warnlog "Value => changes not saved!  should : '" + lsProps(16) + "'   is : '" + Value.GetText + "'"
         end if
      end if
      if ValueMin.GetText <> lsProps(17) then
         if ValueMin.GetText <> "-1,000,002.00000" then
            Warnlog "Value min. => changes not saved!  should : '-1,000,002.00000'   is : '" + ValueMin.GetText + "'"
         else
            Warnlog "Value min. => changes not saved!  should : '" + lsProps(17) + "'   is : '" + ValueMin.GetText + "'"
         end if
      end if
      if ValueMax.GetText <> lsProps(18) then
         if ValueMax.GetText <> "999,997.00000" then
            Warnlog "Value max => changes not saved!  should : '999,997.00000'   is : '" + ValueMax.GetText + "'"
         else
            Warnlog "Value max => changes not saved!  should : '" + lsProps(18) + "'   is : '" + ValueMax.GetText + "'"
         end if
      end if
      if ValueStep.GetText <> lsProps(19) then Warnlog "Incr./decrement value => changes not saved!  should : '" + lsProps(19) + "'   is : '" + ValueStep.GetText + "'"
      if Accuray.GetText <> lsProps(20) then Warnlog "Dec. accuracy => changes not saved!  should : '" + lsProps(20) + "'   is : '" + Accuray.GetText + "' Bugid #106852#"
      if ThousandSeperator.GetSelText <> lsProps(21) then Warnlog "Thousands seperator => changes not saved!  should : '" + lsProps(21) + "'   is : '" + ThousandSeperator.GetSelText + "' Bugid #106852#"
      if CurrencySymbol.GetText <> lsProps(22) then Warnlog "Currency symbol => changes not saved!  should : '" + lsProps(22) + "'   is : '" + CurrencySymbol.GetSelText + "'"
      if CurrSymPosition.GetSelText <> lsProps(23) then Warnlog "Prefix symbol => changes not saved!  should : '" + lsProps(23) + "'   is : '" + CurrSymPosition.GetSelText + "'"
      if CharacterSet.GetText <> lsProps(24) then Warnlog "Character set => changes not saved!  should : '" + lsProps(24) + "'   is : '" + CharacterSet.GetText + "'"
      if Background.GetSelText <> lsProps(25) then Warnlog "Background => changes not saved!  should : '" + lsProps(25) + "'   is : '" + Background.GetSelText + "'"
      if Border.GetSelText <> lsProps(26) then Warnlog "Border => changes not saved!  should : '" + lsProps(26) + "'   is : '" + Border.GetSelText + "'"
      if Spin.GetSelText <> lsProps(27) then Warnlog "Spin button => changes not saved!  should : '" + lsProps(27) + "'   is : '" + Spin.GetSelText + "'"
   end if

'##### FormattedField #####
   if instr ( lcase ( sType ), "formattedfield" ) then
      if MaxTextLen.GetText <> lsProps(13) then Warnlog "Max text lenght => changes not saved!  should : '" + lsProps(13) + "'   is : '" + MaxTextLen.GetText + "'"
      if StrictFormat.GetSelText <> lsProps(14) then Warnlog "Strict format => changes not saved!  should : '" + lsProps(14) + "'   is : '" + StrictFormat.GetSelText + "'"'
      if Readonly.GetSelText <> lsProps(15) then Warnlog "Read only => changes not saved!  should : '" + lsProps(15) + "'   is : '" + Readonly.GetSelText + "'"
      if TabStop.GetSelText <> lsProps(16) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(16) + "'   is : '" + TabStop.GetSelText + "'"
'      if Effective.GetText <> lsProps(17) then Warnlog "Value => changes not saved!  should : '" + lsProps(17) + "'   is : '" + Effective.GetText + "'"
      if EffectiveMin.GetText <> lsProps(18) then
         printlog " Bugid #106852#:"
         if EffectiveMin.GetText <> "1.00" then
            Warnlog "Value min => changes not saved!  should : '" + lsProps(18) + "'   is : '" + EffectiveMin.GetText + "'"
         else
            Warnlog "Value min => changes not saved!  should : '1.00'   is : '" + EffectiveMin.GetText + "'"
         end if
      end if
      if EffectiveMax.GetText <> lsProps(19) then
         printlog " Bugid #106852#:"
         if EffectiveMax.GetText <> "1.00" then
            Warnlog "Value max => changes not saved!  should : '1.00'   is : '" + EffectiveMax.GetText + "'"
         else
            Warnlog "Value max => changes not saved!  should : '" + lsProps(19) + "'   is : '" + EffectiveMax.GetText + "'"
         end if
      end if
      if FormatKey.GetText <> lsProps(20) then Warnlog "Format key => changes not saved!  should : '" + lsProps(20) + "'   is : '" + FormatKey.GetText + "'"
      if CharacterSet.GetText <> lsProps(21) then Warnlog "Character set => changes not saved!  should : '" + lsProps(21) + "'   is : '" + CharacterSet.GetText + "'"
      if Align.GetSelText <> lsProps(22) then Warnlog "Alignment => changes not saved!  should : '" + lsProps(22) + "'   is : '" + Align.GetSelText + "' BugId #106872#"
      if Background.GetSelText <> lsProps(23) then Warnlog "Background => changes not saved!  should : '" + lsProps(23) + "'   is : '" + Background.GetSelText + "'"
      if Border.GetSelText <> lsProps(24) then Warnlog "Border => changes not saved!  should : '" + lsProps(24) + "'   is : '" + Border.GetSelText + "'"
      if Spin.GetSelText <> lsProps(25) then Warnlog "Spin button => changes not saved!  should : '" + lsProps(25) + "'   is : '" + Spin.GetSelText + "'"
   end if

'##### PatternField #####
   if instr ( lcase ( sType ), "patternfield" ) then
      if TextText.GetText <> lsProps(13) then Warnlog "Text => changes not saved!  should : '" + lsProps(13) + "'   is : '" + TextText.GetText + "'"
      if MaxTextLen.GetText <> lsProps(14) then Warnlog "Max text lenght => changes not saved!  should : '" + lsProps(14) + "'   is : '" + MaxTextLen.GetText + "'"
      if EditMask.GetText <> lsProps(15) then Warnlog "Edit mask => changes not saved!  should : '" + lsProps(15) + "'   is : '" + EditMask.GetText + "'"
      if LiteralMask.GetText <> lsProps(16) then Warnlog "Literal mask => changes not saved!  should : '" + lsProps(16) + "'   is : '" + LiteralMask.GetText + "'"
      if StrictFormat.GetSelText <> lsProps(17) then Warnlog "Strict format => changes not saved!  should : '" + lsProps(17) + "'   is : '" + StrictFormat.GetSelText + "'"'
      if Readonly.GetSelText <> lsProps(18) then Warnlog "Read only => changes not saved!  should : '" + lsProps(18) + "'   is : '" + Readonly.GetSelText + "'"
      if TabStop.GetSelText <> lsProps(19) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(19) + "'   is : '" + TabStop.GetSelText + "'"
      if CharacterSet.GetText <> lsProps(20) then Warnlog "Character set => changes not saved!  should : '" + lsProps(20) + "'   is : '" + CharacterSet.GetText + "'"
      if Background.GetSelText <> lsProps(21) then Warnlog "Background => changes not saved!  should : '" + lsProps(21) + "'   is : '" + Background.GetSelText + "'"
      if Border.GetSelText <> lsProps(22) then Warnlog "Border => changes not saved!  should : '" + lsProps(22) + "'   is : '" + Border.GetSelText + "'"
   end if

'##### FileControl #####
   if instr ( lcase ( sType ), "filecontrol" ) then
      if TextText.GetText <> lsProps(13) then Warnlog "Text => changes not saved!  should : '" + lsProps(13) + "'   is : '" + TextText.GetText + "'"
      if Readonly.GetSelText <> lsProps(14) then Warnlog "Read only => changes not saved!  should : '" + lsProps(14) + "'   is : '" + Readonly.GetSelText + "'"
      if TabStop.GetSelText <> lsProps(15) then Warnlog "Tab stop => changes not saved!  should : '" + lsProps(15) + "'   is : '" + TabStop.GetSelText + "'"
      if CharacterSet.GetText <> lsProps(16) then Warnlog "Character set => changes not saved!  should : '" + lsProps(16) + "'   is : '" + CharacterSet.GetText + "'"
      if Background.GetSelText <> lsProps(17) then Warnlog "Background => changes not saved!  should : '" + lsProps(17) + "'   is : '" + Background.GetSelText + "'"
      if Border.GetSelText <> lsProps(18) then Warnlog "Border => changes not saved!  should : '" + lsProps(18) + "'   is : '" + Border.GetSelText + "'"
   end if


end function
