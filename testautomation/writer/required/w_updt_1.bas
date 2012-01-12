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
'* short description : Ressource-Test Writer Document Part 1 / 2
'*
'\******************************************************************

sub main
    use "writer\tools\includes\w_tools.inc"                  		' global subroutines for Writer-application
    use "writer\tools\includes\w_tool1.inc"                  		' global subroutines for Writer-application
    use "writer\tools\includes\w_tool2.inc"                  		' global subroutines for Writer-application

    use "writer\required\includes\w_001_.inc"
    use "writer\required\includes\w_001a_.inc"
    use "writer\required\includes\w_001b_.inc"
    use "writer\required\includes\w_002_.inc"
    use "writer\required\includes\w_003_.inc"
    use "writer\required\includes\w_004_.inc"
    use "writer\required\includes\w_004b_.inc"
    use "writer\required\includes\w_005_.inc"
    use "writer\required\includes\w_005b_.inc"
    
    Printlog "******* Ressource-Test Writer Document Part 1 / 2 *******"

    Call wChangeDefaultView()

    Call hStatusIn ( "writer", "w_updt_1.bas" )
    Call w_001_              ' Menu File
    Call w_001a_             ' Menu File
    Call w_001b_             ' Menu File
    Call w_002_              ' Menu Edit
    Call w_003_              ' Menu View
    Call w_004_              ' Menu Insert
    Call w_004b_             ' Menu Insert
    Call w_005_              ' Menu Format
    Call w_005b_             ' Menu Format (2)
    Call hStatusOut

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\required\includes\g_001.inc"
    use "global\required\includes\g_printing.inc"
    use "global\tools\includes\optional\t_xml_filter1.inc"       ' global routines for XML-functionality
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_control_objects.inc"

    gApplication   = "WRITER"
    GetUseFiles ()
end sub
