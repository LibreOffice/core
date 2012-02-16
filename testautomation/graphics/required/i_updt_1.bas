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
'* short description : Impress Required Test (Part 1)
'*
'\*****************************************************************

public glLocale (15*20) as string
global ExtensionString as String

sub main
    Printlog " -------------------- Impress-Required-Test -----------------------------"
    Call hStatusIn ( "Graphics","i_updt_1.bas")
    
    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\required\includes\global\id_002.inc"                        'Edit
    use "graphics\required\includes\global\id_003.inc"                        'View
    use "graphics\required\includes\global\id_004.inc"                        'Insert
    use "graphics\required\includes\global\id_005.inc"                        'Format
    use "graphics\required\includes\global\id_006.inc"                        'Tools
    use "graphics\required\includes\impress\im_003_.inc"                   'Ansicht
    use "graphics\required\includes\impress\im_004_.inc"                   'Einfuegen

   if hSetLocaleStrings ( gTesttoolPath + "graphics\tools\locale_1.txt" , glLocale () ) = FALSE then
      qaErrorLog "Locales doesn't exist in file : "+gTesttoolPath + "graphics\tools\locale_1.txt"       ' this is needed for spellchecking.
   endif

   call id_002
   Call im_003_
   call id_003
   Call im_004_
   call id_004
   call id_005
   call id_Tools

  Call hStatusOut
end sub

'----------------------------------------------
sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   use "global\required\includes\g_option.inc"
   use "global\required\includes\g_customize.inc"
    gApplication   = "IMPRESS"
    Call GetUseFiles()
end sub

