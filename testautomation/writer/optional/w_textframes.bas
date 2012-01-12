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
'* short description : Test the functionality of textframes
'*
'\*******************************************************************

global gSeperator as String
global gMeasurementUnit as String

sub main

    Dim StartZeit

    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool3.inc"
    use "writer\tools\includes\w_tool6.inc"
    use "writer\optional\includes\tools\tools1.inc"
    use "writer\optional\includes\tools\tools2.inc"
    use "writer\optional\includes\textframes\w_textframes1.inc"
    use "writer\optional\includes\textframes\w_textframes2.inc"
    use "writer\optional\includes\textframes\w_textframes3.inc"
    use "writer\optional\includes\textframes\w_textframes4.inc"
    use "writer\optional\includes\textframes\w_textframes5.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"

    Call hStatusIn ( "writer", "w_textframes.bas","Writer Level1-Test" )
    'Reading the decimal seperator from global routine in tools1.inc.
    gSeperator = GetDecimalSeperator()
    'Setting the measurementunit to 'cm'
    gMeasurementUnit = fSetMeasurementToCM()
    printlog Chr(13) + "      - Test of TextFrame function -"
    Call w_textframes1

    printlog Chr(13) + "      - Test of linked TextFrame -"
    Call w_textframes2

    printlog Chr(13) + "      - Test of MS-Interoperability -"
    Call w_textframes3
    Call w_textframes4
    Call w_textframes5
    Call hStatusOut

    Printlog Chr(13) + "End of Level 1 Test (Textframes):"
    Printlog "Duration: "+ WieLange ( StartZeit )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   Call GetUseFiles
   gApplication = "WRITER"
end Sub
