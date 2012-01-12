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
'* short description : Test of shortcuts in writer
'*
'\***********************************************************************

sub main
    Dim StartTime
    StartTime = Now()

    '/// This test is based on the following spec:
    '/// Localized Shortcuts
    '/// http://specs.openoffice.org/g11n/menus/LocalizedShortcuts.sxw

    use "writer\tools\includes\w_tools.inc"
    use "writer\optional\includes\shortcut\w_shortcuts.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartTime )
    printlog Chr(13) + "******* Writer - Shortcut - Test *******"

    Call hStatusIn ( "writer", "w_shortcuts.bas","Writer Shortcut-Test" )
	Call tShortcutGlobalNew
	Call tShortcutGlobalOpen
	Call tShortcutGlobalSave
	Call tShortcutGlobalSaveAs
	Call tShortcutSelectAll
	Call tShortcutFindAndReplace
	Call tShortcutBold
	Call tShortcutItalic
	Call tShortcutUnderline
	Call tShortcutDoubleUnderline
	Call tShortcutAlign
	Call tShortcutSuperscript
	Call tShortcutSubscript
	Call tShortcutPasteUnformattedText
    Call hStatusOut

    Printlog Chr(13) + "End of Shortcut - Test :"
    Printlog "Duration: "+ WieLange ( StartTime )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end sub
