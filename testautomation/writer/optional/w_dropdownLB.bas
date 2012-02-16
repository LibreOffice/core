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
'* short description : Test Drop Down List Box
'*
'\*******************************************************************

sub main

    Dim StartZeit
    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool3.inc"
    use "writer\optional\includes\tools\tools1.inc"
    use "writer\optional\includes\tools\tools2.inc"
    use "writer\optional\includes\dropdownLB\w_dropdownLB1.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"

    Call hStatusIn ( "writer", "w_dropdownLB.bas","Dropdown listbox" )
    Call wOptionsUndo ( "All" )        ' Sets all writer-options to default

    printlog Chr(13) + "      - Test Drop Down List Box"

    Call tDropDownLB_1
    Call tDropDownLB_2
    Call tDropDownLB_3
    Call tDropDownLB_4
    Call tDropDownLB_5
    Call tDropDownLB_6
    Call tDropDownLB_7
    Call tDropDownLB_8
    Call tDropDownLB_9
    Call tDropDownLB_10
    Call tDropDownLB_11
    Call tDropDownLB_12
    Call tDropDownLB_13
    Call tDropDownLB_14
    Call tDropDownLB_15
    Call tDropDownLB_16
    Call tDropDownLB_17

    Call hStatusOut

    Printlog Chr(13) + "End of Level 1 Test - Drop Down List Box"
    Printlog "Duration: "+ WieLange ( StartZeit )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end Sub
