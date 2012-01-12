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
'* short description : General Tools/Options test
'*
'\***********************************************************************

sub main

    GLOBAL_USE_NEW_SLEEP = TRUE
    
    use "framework\optional\includes\options_ooo_general.inc"
    use "framework\optional\includes\options_ooo_memory.inc"
    use "framework\optional\includes\options_ooo_userdata.inc"
    use "framework\optional\includes\options_ooo_view.inc"
    use "framework\optional\includes\options_ooo_print.inc"
    use "framework\optional\includes\options_ooo_paths.inc"
    use "framework\optional\includes\options_ooo_colors.inc"
    use "framework\optional\includes\options_ooo_fontreplacement.inc"
    use "framework\optional\includes\options_ooo_security.inc"
    use "framework\optional\includes\options_ooo_appearance.inc"
    use "framework\optional\includes\options_ooo_accessibility.inc"
    use "framework\optional\includes\options_ooo_java.inc"   
    
    Call hStatusIn ( "framework", "f_options_ooo.bas" )
    
    printlog " 1. part : test if all global options are saved"
    
    Call tOOoUserData
    Call tOOoGeneral
    Call tOOoMemory
    Call tOOoView
    Call tOOoPrint
    Call tOOoPaths
    Call tOOoColors
    Call tOOoFontReplacement
    Call tOOoSecurity
    Call tOOoAppearance
    Call tOOoAccessibility
    Call tOOoJava
    
    Call hStatusOut
    
end sub

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_filetools.inc"
    use "global\tools\includes\optional\t_control_objects.inc"
    use "global\tools\includes\optional\t_key_tools.inc"
    
    use "framework\tools\includes\options_tools.inc"
    
    Call GetUseFiles
    gApplication = "WRITER"
   
end sub

