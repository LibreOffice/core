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
'* short description : Impress Required (Test Part 2)
'*
'\*****************************************************************

public glLocale (15*20) as string
global ExtensionString as String

sub main
      Printlog " -------------------- Impress-Required-Test -----------------------------"
      Call hStatusIn ( "Graphics","i_updt_2.bas")

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\required\includes\global\id_001.inc"                        'File
    use "graphics\required\includes\global\id_007.inc"                        'Kontext
    use "graphics\required\includes\global\id_008.inc"                        'Window
    use "graphics\required\includes\global\id_009.inc"                        'Help
    use "graphics\required\includes\global\id_011.inc"                        'Toolbars
    use "graphics\required\includes\impress\im_007_.inc"                   'Praesentation

   if hSetLocaleStrings ( gTesttoolPath + "graphics\tools\locale_1.txt" , glLocale () ) = FALSE then
      qaErrorLog "Locales doesn't exist in file : "+gTesttoolPath + "graphics\tools\locale_1.txt"    ' this is needed for spellchecking.
   endif

   Call tFileExportAsPDF
   Call tExportAsPDFButton
   call id_001
   Call im_007_
   call id_008
   call id_009
   call id_007
   call id_011
   call g_printing

  Call hStatusOut
end sub

'----------------------------------------------
sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   use "global\required\includes\g_001.inc"
   use "global\required\includes\g_printing.inc"
   use "global\tools\includes\optional\t_docfuncs.inc"
   use "global\tools\includes\optional\t_control_objects.inc"

   gApplication   = "IMPRESS"
    Call GetUseFiles()
end sub

