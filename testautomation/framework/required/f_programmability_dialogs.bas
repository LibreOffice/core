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
'*  short description : Update-Test for some dialogs in Basic-IDE
'*
'\******************************************************************************

sub main

    use "framework\required\includes\basic_macroassignment.inc"
    use "framework\required\includes\basic_dialog_i18n.inc"
    use "framework\required\includes\basic_organizer.inc"
    use "framework\required\includes\basic_dialog_export.inc"
    use "framework\required\includes\basic_dialog_i18n_import.inc"
    use "framework\required\includes\script_organizers.inc"

    call hStatusIn( "framework" , "f_programmability_dialogs.bas" )
    call tUpdtMacroAssignment()
    call tUpdtBasicDialogI18n()
    call tUpdtBasicOrganizer()
    call tUpdtDialogExport()
    call tBasicDialogI18nImport()
    call tUpdtScripts() ' script organizer nodes with one open document
    call hStatusOut()

end sub

'*******************************************************************************

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"

    use "global\tools\includes\optional\t_basic_ide_tools.inc"
    use "global\tools\includes\optional\t_basic_organizer_tools.inc"
    use "global\tools\includes\optional\t_treelist_tools.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_filetools.inc"
    use "global\tools\includes\optional\t_listfuncs.inc"

    use "framework\tools\includes\formcontrols.inc"
    use "framework\tools\includes\pbrowser_tools.inc"

    gApplication = "WRITER"
    call GetUseFiles()

end sub

'-------------------------------------------------------------------------
