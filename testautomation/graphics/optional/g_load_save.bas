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
'* short description : Graphics Function: Load and Save
'*
'\******************************************************************

public glLocale (15*20) as string
public q as integer
public ExtensionString as String

sub main
    PrintLog "------------------------- g_load_save test -------------------------"
    Call hStatusIn ( "Graphics","g_load_save.bas")

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\global\g_load_save.inc"

    hSetLocaleStrings ( gTesttoolPath + "graphics\tools\locale_1.txt" , glLocale () )

    PrintLog "-------------------------" + gApplication + "-------------------"
    Call tFileOpenSaveEtc
    Call tFileProperties
    Call tFilePassword52
    Call tFileOpenImport
    Call tTestDXF255CharBug
    Call tDeletedTemplate
    call tSaveLoadLayoutEmpty
    call tSaveLoadLayoutOLE
    call tSaveLoadLayoutText
    call tSaveLoadLayoutOutline
    call tSaveLoadLayoutPicture
    call tSaveLoadLayoutChart
    call tSaveLoadLayoutSpreadsheet

    gApplication = "IMPRESS"
    PrintLog "-------------------------" + gApplication + "-------------------"
    Call tFileOpenSaveEtc
    Call tFileProperties
    Call tFilePassword52
    Call tFileOpenImport
    Call tTestDXF255CharBug
    Call tDeletedTemplate
    call tSaveLoadLayoutEmpty
    call tSaveLoadLayoutOLE
    call tSaveLoadLayoutText
    call tSaveLoadLayoutOutline
    call tSaveLoadLayoutPicture
    call tSaveLoadLayoutChart
    call tSaveLoadLayoutSpreadsheet

    Call hStatusOut
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "DRAW"
end sub


