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


 printlog " - other"
 printlog "   - years two digits => 1899"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   sSave = Zweistellig.GetText
   Zweistellig.SetText "1899"
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   gApplication = "CALC"
   hNewDocument
   DocumentCalc.TypeKeys "04.03.00<return>"
   DocumentCalc.TypeKeys "<up>"
   DocumentCalc.TypeKeys "<F2><Home><Shift End>"
   Sleep (1)
   EditCopy
   DocumentCalc.TypeKeys "<Escape>"

   ClipText = GetClipboardText
   if ClipText <> "04.03.1900" AND ClipText <> "04/03/1900" then Warnlog "Wrong date not 04.03.1900 or 04/03/1900 :  => '" + ClipText + "'"

   hCloseDocument

 printlog "   - years two digits => 2050"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   Zweistellig.SetText "2050"
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   gApplication = "CALC"
   hNewDocument
   DocumentCalc.TypeKeys "05.03.68<return>"
   DocumentCalc.TypeKeys "<up>"
   DocumentCalc.TypeKeys "<F2><Home><Shift End>"
   Sleep (1)
   EditCopy
   DocumentCalc.TypeKeys "<Escape>"

   ClipText = GetClipboardText
   if ClipText <> "05.03.2068" AND ClipText <> "05/03/2068" then Warnlog "Wrong date not 05.03.2068 or 05/03/2068 :  => '" + ClipText + "'"

   hCloseDocument

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

 printlog "- working with activated HelpAgent"
 printlog "  - check display duration of 10sec"
 printlog "    new writer-doc"
  gApplication = "WRITER"
  hNewDocument

 printlog "    tools / options / staroffice / general -> help agent -> duration to 10 -> "
 printlog "    click also reset to delete the ignore-list for help-agent -> options-dialog OK"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   Zuruecksetzen.Click
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

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
 printlog "    close options-dialog and close the writer-doc"
   hCloseDocument

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

 printlog "    close options-dialog and close the writer-doc"
   hCloseDocument

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

 printlog "    close options-dialog and close the writer-doc"
   hCloseDocument

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

 printlog "    close options-dialog and close the writer-doc"
   hCloseDocument

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

 printlog "    close options-dialog and close the writer-doc"
   hCloseDocument

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

 printlog "    close options-dialog and close the writer-doc"
   hCloseDocument

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


 printlog "StarOffice / General => 'printing sets 'document modified' status'"
 printlog "save a writer-doc"
   hNewDocument
   Kontext "DocumentWriter"
   DocumentWriter.TypeKeys "This is a file, for options test: <return> Tools / Options / StarOffice / General -> 'Printing sets 'document modified' status'"
   DocumentWriter.TypeKeys "<Return><Return>This file is created by an automated test. It can be trashed after one day!<Return>Today is " + Date
   hFileSaveAsKill ( gOfficePath + "user\work\opt.sxw" )

 printlog "- open tools/option/StarOffice/general and save the state of the checkbox"
 printlog "- check it"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   bStatus = DruckenStatus.IsChecked
   DruckenStatus.Check
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

 printlog "- file/print and click OK at the print-dialog"
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
   FileClose
   Kontext "Active"
   if Active.Exists(2) <> TRUE then
      Warnlog "No warning, when this options is checked!"
   else
      Active.Cancel
   end if

 printlog "- file/save"
   FileSave

 printlog "- uncheck the checkbox"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )
   DruckenStatus.UnCheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

 printlog "- file/print"
   FilePrint
   Kontext "DruckenDlg"
   DruckenDlg.OK
   Sleep (5)

 printlog "- file/close"
   FileClose
   Kontext "Active"
   if Active.Exists(2) then
      Warnlog "The document is modified, a messagebox came up at closing"
      Active.Yes
   end if

 printlog "- reset this option"
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


   hNewDocument
 printlog "StarOffice / General => 'use StarOffice dialogs'"
   if gPlatgroup = "unx" then
      printlog "This feature is only available on windows systems"
   else
 printlog "- open tools/options/staroffice/general and uncheck 'use StarOffice dialogs'"
      ToolsOptions
      hToolsOptions ( "StarOffice", "General" )
      bStatus = StarOfficeDialogeBenutzen.IsChecked
      StarOfficeDialogeBenutzen.UnCheck
      Kontext "ExtrasOptionenDlg"
      ExtrasOptionenDlg.OK
      Sleep (2)

 printlog "- file/open and check if the file-dialog is a system-dialog"
      FileOpen
      if ExistsSysDialog ( FilePicker ) = FALSE then
         Warnlog "The file-open-dialog is no system dialog"
         Kontext "OeffnenDlg"
         OeffnenDlg.Cancel
      else
         CloseSysDialog ( FilePicker )
      end if

 printlog Chr(13) + "check that StarOffice dialogs are used => state of the checkbox is checked"
 printlog "- open tools/options/staroffice/general"
      ToolsOptions
      hToolsOptions ( "StarOffice", "General" )
      StarOfficeDialogeBenutzen.Check
      Kontext "ExtrasOptionenDlg"
      ExtrasOptionenDlg.OK
      Sleep (2)

 printlog "- open tools/options/staroffice/general and check 'use StarOffice dialogs'"
      FileOpen
      Kontext "OeffnenDlg"
      if OeffnenDlg.Exists (1) <> TRUE then
         Warnlog "StarOffice dialogs are not used!"
         if ExistsSysDialog ( FilePicker ) then CloseSysDialog ( FilePicker )
      else
         OeffnenDlg.Cancel
      end if

 printlog Chr(13) + "reset the state of 'use StarOffice dialogs' to default"
      ToolsOptions
      hToolsOptions ( "StarOffice", "General" )
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


 printlog " - save"
 printlog "    - undo"
 printlog "    - set number of steps => 1"

   hNewDocument
   ToolsOptions
   hToolsOptions ( "StarOffice", "Memory" )
   sSave = UndoSteps.GetText
   UndoSteps.SetText "1"
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "DocumentWriter"
   DocumentWriter.TypeKeys "Hello "
   DocumentWriter.TypeKeys "friends "

   EditUndo
   try
      EditUndo
      Warnlog "More than 1 undo was possible!"
   catch
   endcatch

 printlog "    - set number of steps => 15"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Memory" )
   UndoSteps.SetText "15"
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "DocumentWriter"
   for i=1 to 20
      DocumentWriter.TypeKeys "Hello<return>"
   next i
   Sleep (2)

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

 printlog "    - set number of steps => 99"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Memory" )
   UndoSteps.SetText "99"
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "DocumentWriter"
   for i=1 to 102
      DocumentWriter.TypeKeys "Hello<return>"
   next i

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

   hCloseDocument

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

hNewDocument
   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
 printlog " - view"
 printlog "   - display"
 printlog "    - scaling"
   sSave = FontScale.GetText
   FontScale.SetText "50"
   FontScale.More
   FontScale.Less
 printlog "      - " + FontScale.GetText
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Call DialogTestForViewOptions

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   FontScale.SetText "150"
   FontScale.More
   FontScale.Less
 printlog "      - " + FontScale.GetText
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Call DialogTestForViewOptions

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   FontScale.SetText(sSave)
   FontScale.More
   FontScale.Less
 printlog "      - " + FontScale.GetText
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

   hNewDocument

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   iSave = MousePositioning.GetSelIndex
   MousePositioning.Select 1
 printlog "      - " + MousePositioning.GetSelText
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "DocumentWriter"
   DocumentWriter.MouseMove 1, 1

   DialogTestForViewOptions

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   MousePositioning.Select 2
 printlog "      - " + MousePositioning.GetSelText
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "DocumentWriter"
   DocumentWriter.MouseMove 1, 1

   DialogTestForViewOptions

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   MousePositioning.Select 3
 printlog "      - " + MousePositioning.GetSelText
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   Kontext "DocumentWriter"
   DocumentWriter.MouseMove 1, 1

   DialogTestForViewOptions

   ToolsOptions
   hToolsOptions ( "StarOffice", "View" )
   MousePositioning.Select iSave
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (2)

   hCloseDocument
endcase


