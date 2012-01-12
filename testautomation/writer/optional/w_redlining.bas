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
'* short description : Test of the Redlining function 
'*
'\*******************************************************************

sub main
    Dim StartZeit
    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool2.inc"
    use "writer\tools\includes\w_tool3.inc"
    use "writer\optional\includes\tools\tools1.inc"
    use "writer\optional\includes\tools\tools2.inc"
    use "writer\optional\includes\redlining\w_redlining1.inc"
    use "writer\optional\includes\redlining\w_redlining2.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"

    Call hStatusIn ( "writer", "w_redlining.bas" , "Redlining" )

    Call tRedlining_1
    Call tRedlining_2
    Call tRedlining_3
    Call tRedlining_4
    Call tRedlining_5
    Call tRedlining_6
    Call tRedlining_7
    Call tRedlining_8
    Call tRedlining_9
    Call tRedlining_10
    Call tRedlining_11
    Call tRedlining_12
    Call tRedlining_13
    Call tRedlining_14
    Call tRedlining_15
    Call tRedlining_16

    Call tAcceptReject_1
    Call tAcceptReject_2
    Call tAcceptReject_3
    Call tAcceptReject_4
    Call tAcceptReject_5
    Call tAcceptReject_6
    Call tAcceptReject_7
    Call tAcceptReject_8
    Call tAcceptReject_9
    Call tAcceptReject_10
    Call tAcceptReject_11
    Call tAcceptReject_12
    Call tAcceptReject_13
    Call tAcceptReject_14
    Call tAcceptReject_15

    Call hStatusOut

    Printlog Chr(13) + "End of Level 1 Test (Redlining):"
    Printlog "Duration: "+ WieLange ( StartZeit )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\required\t_lists.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end sub
