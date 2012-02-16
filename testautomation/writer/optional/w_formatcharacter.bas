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
'* short description : Formatting of characters, setting styles etc.
'*
'\***********************************************************************

sub main
    Dim StartTime
    StartTime = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\optional\includes\formatcharacter\w_format_character1.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartTime )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"

    Call hStatusIn ( "writer", "w_formatcharacter.bas","Formatting of characters" )

        Call tFormatCharacter
        Call tFormatStyles1                        '-> Format/Styles with contect menu
        Call tFormatCharacter1                '-> Format/Pair Kerning
        Call tFormatCharacter2                '-> Format/Default
        Call tFormatCharacter3                '-> Format/Character
        Call tFormatCharacterOverline1    '-> Format/Overline
        Call tFormatCharacterOverline2    '-> Format/Overline (with save)
        Call tFormatHyperlink                    '-> Autocorrection of hyperlinks

    Call hStatusOut

    Printlog Chr(13) + "End of Test :"
    Printlog "Duration: "+ WieLange ( StartTime )
    Printlog "Date: " +  Date + "    Time: " + Time
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_extension_manager_tools.inc"
    use "global\tools\includes\optional\t_ui_filters.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end sub
