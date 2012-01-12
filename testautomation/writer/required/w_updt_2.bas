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
'* short description : Ressource-Test Writer Document Part 2 / 2
'*
'\******************************************************************

sub main
    use "writer\tools\includes\w_tools.inc"                  		' global subroutines for Writer-application
    use "writer\tools\includes\w_tool1.inc"                  		' global subroutines for Writer-application
    use "writer\tools\includes\w_tool2.inc"                  		' global subroutines for Writer-application

    use "writer\required\includes\w_006_.inc"
    use "writer\required\includes\w_007_.inc"
    use "writer\required\includes\w_008_.inc"
    use "writer\required\includes\w_009_.inc"
    use "writer\required\includes\w_010_.inc"
    use "writer\required\includes\w_010_1.inc"
    use "writer\required\includes\w_011_.inc"
    use "writer\required\includes\w_020_.inc"
    
    Printlog "******* Ressource-Test Writer Document Part 2 / 2 *******"

    Call wChangeDefaultView()

    Call hStatusIn ( "writer", "w_updt_2.bas" )
    Call w_006_              ' Menu Table
    Call w_007_              ' Menu Tools
    Call w_008_              ' Menu Window
    Call w_009_              ' Menu Help
    Call w_010_              ' Objectbar
    Call w_010_1             ' Objectbar. Part 2
    Call w_011_              ' Rest of Objectbar
    Call w_020_              ' Toolbar
    Call hStatusOut

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\required\includes\g_option.inc"                	' global subroutines for Tools / Options
    use "global\required\includes\g_findbar.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_control_objects.inc"

    gApplication   = "WRITER"
    GetUseFiles ()
end sub
