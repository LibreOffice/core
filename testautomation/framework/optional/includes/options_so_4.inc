'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: options_so_4.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:15 $
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
'* owner : thorsten.bosbach@sun.com
'*
'* short description : options test (General/Memory/View) 
'*
'\******************************************************************************

testcase func_StarOfficeGeneral_1
   gApplication = "WRITER"
  Dim sSave as String
  Dim ClipText as String

'///StarOffice / General => 'Years ( two digits )'  - functionality test

 printlog " - other"
 printlog "   - years two digits => 1899"
'///+open tools/option/StarOffice/general
'///+save the default settings
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   sSave = Zweistellig.GetText
'///+insert 1899 in 'Interpret as years between' and click OK for the options dialog
   Zweistellig.SetText "1899"
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open a calc-doc and insert 04.03.00 <Return>
   gApplication = "CALC"
   hNewDocument
   DocumentCalc.TypeKeys "04.03.00<return>"
   DocumentCalc.TypeKeys "<up>"
   DocumentCalc.TypeKeys "<F2><Home><Shift End>"
   Sleep (1)
   EditCopy
   DocumentCalc.TypeKeys "<Escape>"
'///+select the cell and press F2 => now the correct date in the cell must be 04/03/1900

   ClipText = GetClipboardText
   if ClipText <> "04.03.1900" AND ClipText <> "04/03/1900" then Warnlog "Wrong date not 04.03.1900 or 04/03/1900 :  => '" + ClipText + "'"

'///+close the calc doc
   hCloseDocument

 printlog "   - years two digits => 2050"
'///open tools/option/StarOffice/general
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
'///+insert 2050 in 'Interpret as years between' and click OK for the options dialog
   Zweistellig.SetText "2050"
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open a calc-doc and insert 04.03.00 <Return>
   gApplication = "CALC"
   hNewDocument
   DocumentCalc.TypeKeys "05.03.68<return>"
   DocumentCalc.TypeKeys "<up>"
   DocumentCalc.TypeKeys "<F2><Home><Shift End>"
   Sleep (1)
   EditCopy
   DocumentCalc.TypeKeys "<Escape>"

'///+select the cell and press F2 => now the correct date in the cell must be 04/03/1900
   ClipText = GetClipboardText
   if ClipText <> "05.03.2068" AND ClipText <> "05/03/2068" then Warnlog "Wrong date not 05.03.2068 or 05/03/2068 :  => '" + ClipText + "'"

'///+close the calc doc
   hCloseDocument

'///open tools/option/StarOffice/general and reset all to the default data ( 1930 )
 printlog "   - reset to default"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   Zweistellig.SetText sSave
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   gApplication = "WRITER"
endcase

' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase func_StarOfficeGeneral_2
  Dim bSave as Boolean
  Dim sSave as String
  Dim i as Integer, iTime as Integer

  gApplication = "WRITER"

'///StarOffice / General => 'Help Agent'
'///check the maximum and the minimum of display duration ( 60sec and 5sec )
'///+save the data for help agent
'///+tools / options / staroffice / general -> help agent
 printlog "tools / options / staroffice / general -> help agent"
 printlog "- check the minimum and the maximum of display duration for help agent ( 60sec and 5sec )"
 printlog "   activate the help agent options-dialog -> OK"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   bSave = Aktivieren.IsChecked
   Aktivieren.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///working with activated HelpAgent
'///+open a new writer-doc
 printlog "- working with activated HelpAgent"
 printlog "  - check display duration of 10sec"
 printlog "    new writer-doc"
  gApplication = "WRITER"
  hNewDocument

'///+tools / options / staroffice / general -> help agent
'///+activate the help agent for 10 seconds ( click reset to delete the ignore-list for helpagent )
 printlog "    tools / options / staroffice / general -> help agent -> duration to 10 -> "
 printlog "    click also reset to delete the ignore-list for help-agent -> options-dialog OK"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   Zuruecksetzen.Click
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open Format/Character - Font-Tabpage => help-agent must be activated ( 10seconds )
'///+check if agent closed after ~10sec
 printlog "    Format/Character - Font-Tabpage => help-agent must be activated"
 printlog "    check if agent closed after ~10sec"
   FormatCharacter
   Kontext
   active.SetPage TabFont

   Kontext "HelpAgent"
   if HelpAgent.Exists(1) <> TRUE then
      Warnlog "The help agent wasn't activated for autocorrection!"
   else
      for i=1 to 20
         if HelpAgent.Exists then
            Sleep 1
         else
            iTime = i
            i=21
         end if
      next i
      if iTime < 9 then Warnlog "The help agent is closed faster than 9 seconds!"
      if iTime > 12 then Warnlog "The help agent isn't closed after 12 seconds!"
      if HelpAgent.Exists then
         Warnlog "The help agent isn't closed after 20 seconds, the test close it!"
         HelpAgent.Close
      end if
   end if

   Kontext "TabFont"
   TabFont.Cancel
'///+close the options-dialog and close the writer-doc
 printlog "    close options-dialog and close the writer-doc"
   hCloseDocument

'///check display duration of 40sec
'///+new writer-doc
'///+tools / options / staroffice / general -> help agent
'///+activate the help agent for 40 seconds
 printlog " - check another display duration of 40sec"
 printlog "    new writer-doc"
   hNewDocument
 printlog "    tools / options / staroffice / general -> help agent -> duration to 40 -> options-dialog OK"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   Aktivieren.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)
'///+open Format/Character - Font-Tabpage => help-agent must be activated ( 40seconds )
'///+check if agent closed after ~40sec
 printlog "    Format/Character - Font-Tabpage => help-agent must be activated"
 printlog "    check if agent closed after ~40sec"
   FormatCharacter
   Kontext
   active.SetPage TabFont

   Kontext "HelpAgent"
   if HelpAgent.Exists(1) <> TRUE then
      Warnlog "The help agent wasn't activated for autocorrection!"
   else
      for i=1 to 60
         if HelpAgent.Exists then
            Sleep 1
         else
            iTime = i
            i=61
         end if
      next i
      if iTime < 35 then Warnlog "The help agent is closed faster than 35 seconds!"
      if iTime > 45 then Warnlog "The help agent isn't closed after 45 seconds!"
      if HelpAgent.Exists then
         Warnlog "The help agent isn't closed after 40 seconds, the test close it!"
         HelpAgent.Close
      end if
   end if
   Kontext "TabFont"
   TabFont.Cancel

'///+close the options-dialog and close the writer-doc
 printlog "    close options-dialog and close the writer-doc"
   hCloseDocument

'///check another display duration of 60sec
'///+ new writer-doc
'///+tools / options / staroffice / general -> help agent
'///+activate the help agent for 60 seconds
 printlog " - check another display duration of 60sec"
 printlog "    new writer-doc"
   hNewDocument
 printlog "    tools / options / staroffice / general -> help agent -> duration to 60 -> options-dialog OK"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   Aktivieren.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)
'///+open Format/Character - Font-Tabpage => help-agent must be activated ( 60seconds )
'///+check if agent closed after ~60sec
 printlog "    Format/Character - Font-Tabpage => help-agent must be activated"
   FormatCharacter
   Kontext
   active.SetPage TabFont

   Kontext "HelpAgent"
   if HelpAgent.Exists(1) <> TRUE then
      Warnlog "The help agent wasn't activated for autocorrection!"
   else
      for i=1 to 80
         if HelpAgent.Exists then
            Sleep 1
         else
            iTime = i
            i=81
         end if
      next i
      if iTime < 55 then Warnlog "The help agent is closed faster than 55 seconds!"
      if iTime > 65 then Warnlog "The help agent isn't closed after 65 seconds!"
      if HelpAgent.Exists then
         Warnlog "The help agent isn't closed after 60 seconds, the test close it!"
         HelpAgent.Close
      end if
   end if

   Kontext "TabFont"
   TabFont.Cancel

'///+close the options-dialog and close the writer-doc
 printlog "    close options-dialog and close the writer-doc"
   hCloseDocument

'///check if the help agent isn't active, when you ignore it 3 times ( inserted in the ignore-list )
'///+ new writer-doc
'///+tools / options / staroffice / general -> help agent
'///+activate the help agent for 10 seconds
 printlog " - check if the help agent isn't active, when you ignore it 3 times ( inserted in the ignore-list )"
 printlog "    new writer-doc"
   hNewDocument
  printlog "    tools / options / staroffice / general -> help agent -> duration to 10 -> options-dialog OK"
  ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   Aktivieren.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)
'///+open Format/Character - Font-Tabpage => help-agent should not be activated
 printlog "    Format/Character - Font-Tabpage => help-agent should not be activated"
   FormatCharacter
   Kontext
   active.SetPage TabFont

   Kontext "HelpAgent"
   if HelpAgent.Exists then
      Warnlog "Ignore HelpAgent 3 time for the same slot does not work, HelpAgent is active!"
      HelpAgent.Close
   end if
   Kontext "TabFont"
   TabFont.Cancel

'///+close the options-dialog and close the writer-doc
 printlog "    close options-dialog and close the writer-doc"
   hCloseDocument

'///check if ignore-list can be deleted
'///+ new writer-doc
'///+tools / options / staroffice / general -> help agent
'///+activate the help agent for 10 seconds and reset the ignore-list
 printlog " - check if ignore-list can be deleted"
 printlog "    new writer-doc"
   hNewDocument
  printlog "    tools / options / staroffice / general -> help agent -> click on reset -> options-dialog OK"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   Aktivieren.Check
   Zuruecksetzen.Click
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)
'///+open Format/Character - Font-Tabpage => help-agent should not be activated
 printlog "    Format/Character - Font-Tabpage => help-agent should not be activated"
   FormatCharacter
   Kontext
   active.SetPage TabFont

   Kontext "HelpAgent"
   if HelpAgent.Exists(2) <> TRUE then
      Warnlog "Ignore-list isn't reset, the HelpAgent is not active"
   else
      HelpAgent.Close
   end if
   Kontext "TabFont"
   TabFont.Cancel

'///+close the options-dialog and close the writer-doc
 printlog "    close options-dialog and close the writer-doc"
   hCloseDocument

'///working with deactivated HelpAgent
'///+create a new writer-doc
'///+tools / options / staroffice / general -> help agent
'///+deactivate the help agent
 printlog " - working with deactivated the help agent"
 printlog "    new writer-doc"
   hNewDocument
  printlog "    tools / options / staroffice / general -> deactivate help agent -> options-dialog OK"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   Aktivieren.UnCheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open Format/Character - Font-Tabpage => help-agent ust be activated ( 10seconds )
 printlog "    Format/Character - Font-Tabpage => help-agent ust be activated"
   FormatCharacter
   Kontext
   active.SetPage TabFont

   Kontext "HelpAgent"
   if HelpAgent.Exists(1) = TRUE then
      Warnlog "The help agent was active!"
      HelpAgent.Close
   end if
   Kontext "TabFont"
   TabFont.Cancel

'///+close the options-dialog and close the writer-doc
 printlog "    close options-dialog and close the writer-doc"
   hCloseDocument

'///reset help-agent-options
 printlog "  - reset help-agent-options"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   Aktivieren.Check
   Zuruecksetzen.Click
   if bSave = TRUE then Aktivieren.Check else Aktivieren.UnCheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)
endcase

' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase func_StarOfficeGeneral_3
  Dim bStatus as Boolean

  gApplication = "WRITER"

'///StarOffice / General => 'printing sets 'document modified' status'
'///+open a new writer-doc and save it

 printlog "StarOffice / General => 'printing sets 'document modified' status'"
 printlog "save a writer-doc"
   hNewDocument
   Kontext "DocumentWriter"
   DocumentWriter.TypeKeys "This is a file, for options test: <return> Tools / Options / StarOffice / General -> 'Printing sets 'document modified' status'"
   DocumentWriter.TypeKeys "<Return><Return>This file is created by an automated test. It can be trashed after one day!<Return>Today is " + Date
   hFileSaveAsKill ( gOfficePath + "user\work\opt.sxw" )

 printlog "- open tools/option/StarOffice/general and save the state of the checkbox"
 printlog "- check it"
'///+ open tools/option/StarOffice/general
'///+'printing sets 'document modified' status' => checked
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   bStatus = DruckenStatus.IsChecked
   DruckenStatus.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)
'///+ close options dialog with OK

 printlog "- file/print and click OK at the print-dialog"
'///+file / print and click OK at the print-dialog
   FilePrint
   
   Kontext "Active"
   if Active.Exists() then
      warnlog( "Unexpected dialog, no default printer found (#108776?)" )
      Active.OK()
   else
      Kontext "DruckenDlg"
      DruckenDlg.OK
      Sleep (5)
   endif

 printlog "- file/close"
'///+file / close => a messagebox must warn about changes in the document
   FileClose
   Kontext "Active"
   if Active.Exists(2) <> TRUE then
      Warnlog "No warning, when this options is checked!"
   else
'///+- cancel the messagebox
      Active.Cancel
   end if

 printlog "- file/save"
'///+file / save
   FileSave

 printlog "- uncheck the checkbox"
'///'printing sets 'document modified' status' => unchecked
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   DruckenStatus.UnCheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

 printlog "- file/print"
'///+file / print
   FilePrint
   Kontext "DruckenDlg"
   DruckenDlg.OK
   Sleep (5)

 printlog "- file/close"
'///+file / close => no modify of the document => no messagebox for warning
   FileClose
   Kontext "Active"
   if Active.Exists(2) then
      Warnlog "The document is modified, a messagebox came up at closing"
      Active.Yes
   end if

 printlog "- reset this option"
'///set this option to default
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   if bStatus = TRUE then DruckenStatus.Check else DruckenStatus.UnCheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

endcase

' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase func_StarOfficeGeneral_4
  Dim bStatus as Boolean

  gApplication = "WRITER"

'///StarOffice / General => 'use StarOffice dialogs' ( only on windows systems )

'///check that system dialogs are used => state of the checkbox is unchecked
'///+open a new writer-doc
   hNewDocument
 printlog "StarOffice / General => 'use StarOffice dialogs'"
   if gPlatgroup = "unx" then
      printlog "This feature is only available on windows systems"
   else
'///+open tools/options/staroffice/general
 printlog "- open tools/options/staroffice/general and uncheck 'use StarOffice dialogs'"
      ToolsOptions
      hToolsOptions ( "StarOffice", "General" )
'///+save state of 'use StarOffice dialogs' and uncheck it
      bStatus = StarOfficeDialogeBenutzen.IsChecked
      StarOfficeDialogeBenutzen.UnCheck
      Kontext "ExtrasOptionenDlg"
      ExtrasOptionenDlg.OK
      Sleep (2)

'///+file/open and check if the file-dialog is a system-dialog
 printlog "- file/open and check if the file-dialog is a system-dialog"
      FileOpen
      if ExistsSysDialog ( FilePicker ) = FALSE then
         Warnlog "The file-open-dialog is no system dialog"
         Kontext "OeffnenDlg"
         OeffnenDlg.Cancel
      else
         CloseSysDialog ( FilePicker )
      end if

'///check that StarOffice dialogs are used => state of the checkbox is checked
'///+open tools/options/staroffice/general
 printlog Chr(13) + "check that StarOffice dialogs are used => state of the checkbox is checked"
 printlog "- open tools/options/staroffice/general"
      ToolsOptions
      hToolsOptions ( "StarOffice", "General" )
'///+save state of 'use StarOffice dialogs' and uncheck it
      StarOfficeDialogeBenutzen.Check
      Kontext "ExtrasOptionenDlg"
      ExtrasOptionenDlg.OK
      Sleep (2)

'///+file/open and check if the file-dialog is a StarOffice-dialog
 printlog "- open tools/options/staroffice/general and check 'use StarOffice dialogs'"
      FileOpen
      Kontext "OeffnenDlg"
      if OeffnenDlg.Exists (1) <> TRUE then
         Warnlog "StarOffice dialogs are not used!"
         if ExistsSysDialog ( FilePicker ) then CloseSysDialog ( FilePicker )
      else
         OeffnenDlg.Cancel
      end if

'///reset the state of 'use StarOffice dialogs' to default
'///+open tools/options/staroffice/general
 printlog Chr(13) + "reset the state of 'use StarOffice dialogs' to default"
      ToolsOptions
      hToolsOptions ( "StarOffice", "General" )
'///+save state of 'use StarOffice dialogs' and uncheck it
      if bStatus = TRUE then StarOfficeDialogeBenutzen.Check else StarOfficeDialogeBenutzen.UnCheck
      Kontext "ExtrasOptionenDlg"
      ExtrasOptionenDlg.OK
      Sleep (2)
   end if

   hCloseDocument

endcase


' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase func_StarOfficeMemory_1
   Dim sSave as String
   Dim i, ilast as Integer
   gApplication = "WRITER"

'///StarOffice / Memory => 'Undo steps'

 printlog " - save"
 printlog "    - undo"
 printlog "    - set number of steps => 1"

'///<b>undo-step = 1</b>
'///+open a new writer-doc and tools/options/staroffice/memory
   hNewDocument
   ToolsOptions
   hToolsOptions ( "StarOffice", "Memory" )
'///+save the default setting for 'Undo Steps' and set it to 1
   sSave = UndoSteps.GetText
   UndoSteps.SetText "1"
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+write two words in the writer doc
   Kontext "DocumentWriter"
   DocumentWriter.TypeKeys "Hello "
   DocumentWriter.TypeKeys "friends "

'///+try edit/undo more than 1 times, after the first click the menu-item must be inactiv and the undo-button in the function bar too
   EditUndo
   try
      EditUndo
      Warnlog "More than 1 undo was possible!"
   catch
   endcatch

'///<b>undo-step = 15</b>
'///+open tools/options/staroffice/memory
 printlog "    - set number of steps => 15"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Memory" )
'///+set 'Undo Steps' to 15
   UndoSteps.SetText "15"
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+write a short text with more than 20 words
   Kontext "DocumentWriter"
   for i=1 to 20
      DocumentWriter.TypeKeys "Hello<return>"
   next i
   Sleep (2)

'///+check if you can use 'edit/undo' more than 15 times
   for i=1 to 20
      if i<16 then
         try
            EditUndo
         catch
            iLast = i-1
            Warnlog "The 15th undo was impossible! The test can do only " + iLast + " undos!  => BugID 83891"
         endcatch
      else
         try
            EditUndo
            Warnlog "More than 15 undo was possible!"
         catch
         endcatch
      end if
   next i

'///<b>undo-step = 99</b>
'///+open tools/options/staroffice/memory
 printlog "    - set number of steps => 99"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Memory" )
'///+set 'Undo Steps' to 99
   UndoSteps.SetText "99"
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+write a short text with more than 100 words
   Kontext "DocumentWriter"
   for i=1 to 102
      DocumentWriter.TypeKeys "Hello<return>"
   next i

'///+check if you can use 'edit/undo' more than 99 times
   for i=1 to 102
      if i<100 then
         try
            EditUndo
         catch
            iLast = i-1
            Warnlog "The 99th undo was impossible! The test can do only " + iLast + " undos!  => BugID 83891"
            i=105
         endcatch
      else
         try
            EditUndo
            Warnlog "More than 99 undo are possible!"
         catch
            i=105
         endcatch
      end if
   next i

'///close the document
   hCloseDocument

'///set this option to default
 printlog "    - reset the number of steps"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Memory" )
   UndoSteps.SetText sSave
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

endcase

' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase func_StarOfficeView_1
  Dim bSave as Boolean
  Dim iSave as Integer
  Dim sSave as String

'///StarOffice / View => 'Display : Look & Feel' and 'Scaling'
hNewDocument
'///+open tools/options/staroffice/view
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
 printlog " - view"
 printlog "   - display"
'///scaling
 printlog "    - scaling"
'///+save the default setting for scaling
   sSave = FontScale.GetText
'///+set Scaling to 50
   FontScale.SetText "50"
   FontScale.More
   FontScale.Less
 printlog "      - " + FontScale.GetText
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open some dialogs/tab dialogs and flyer to have a look at it ( testtool only can test, if this feature doesn't crash )
   Call DialogTestForViewOptions

'///+open tools/options/staroffice/view
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+set Scaling to 150
   FontScale.SetText "150"
   FontScale.More
   FontScale.Less
 printlog "      - " + FontScale.GetText
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+open some dialogs/tab dialogs and flyer to have a look at it ( testtool only can test, if this feature doesn't crash )
   Call DialogTestForViewOptions

'///+open tools/options/staroffice/view
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+set Scaling to default setting
   FontScale.SetText(sSave)
   FontScale.More
   FontScale.Less
 printlog "      - " + FontScale.GetText
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)
   hclosedocument
endcase

' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase func_StarOfficeView_2
   Dim iSave as Integer

 printlog " - view"
 printlog "   - mouse positioning"

'///StarOffice / View => 'mouse positioning'
'///+open a new writer doc
   hNewDocument

'///+open tools/options/staroffice/view
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+save the default setting for 'mouse positioning'
   iSave = MousePositioning.GetSelIndex
'///+select the 1st entry for 'mouse positioning'
   MousePositioning.Select 1
 printlog "      - " + MousePositioning.GetSelText
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+move the mouse to position 1,1 on the writer doc
   Kontext "DocumentWriter"
   DocumentWriter.MouseMove 1, 1

'///+open some dialogs/tab dialogs and flyer to have a look at the mouse position ( testtool only can test, if this feature doesn't crash )
   DialogTestForViewOptions

'///+open tools/options/staroffice/view
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+select the 2nd entry for 'mouse positioning'
   MousePositioning.Select 2
 printlog "      - " + MousePositioning.GetSelText
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+move the mouse to position 1,1 on the writer doc
   Kontext "DocumentWriter"
   DocumentWriter.MouseMove 1, 1

'///+open some dialogs/tab dialogs and flyer to have a look at the mouse position ( testtool only can test, if this feature doesn't crash )
   DialogTestForViewOptions

'///+open tools/options/staroffice/view
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+select the 3rd entry for 'mouse positioning'
   MousePositioning.Select 3
 printlog "      - " + MousePositioning.GetSelText
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+move the mouse to position 1,1 on the writer doc
   Kontext "DocumentWriter"
   DocumentWriter.MouseMove 1, 1

'///+open some dialogs/tab dialogs and flyer to have a look at the mouse position ( testtool only can test, if this feature doesn't crash )
   DialogTestForViewOptions

'///+open tools/options/staroffice/view
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
'///+select the default setting for 'mouse positioning'
   MousePositioning.Select iSave
'///+click OK for the options dialog
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

'///+close the writer doc
   hCloseDocument
endcase


