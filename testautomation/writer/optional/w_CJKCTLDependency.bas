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
'* short description : Test CJK and CTL support dependency
'*
'\*******************************************************************

sub main

    Dim StartZeit

    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool3.inc"
    use "writer\optional\includes\tools\tools1.inc"
    use "writer\optional\includes\tools\tools2.inc"
    use "writer\optional\includes\CJKCTLDependency\CJKCTLDependency1.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"

    Call hStatusIn ( "writer", "w_CJKCTLDependency.bas" , "CJK Dependency test" )

    printlog Chr(13) + "      - Test CJK support dependency"

    Call tCJKDependency_1
    Call tCJKDependency_2
    Call tCJKDependency_3
    Call tCJKDependency_4
    Call tCJKDependency_5

    printlog Chr(13) + "      - Test CTL support dependency"

    Call tCTLDependency_1
    Call tCTLDependency_2
    Call tCTLDependency_3
    Call tCTLDependency_4
    Call tCTLKashidaCheck
    
    gApplication = "HTML"
    Call tCTLDependency_5
    Call tCTLDependency_6
    Call tCTLDependency_7
    Call tCTLDependency_8
    
      
   Call hStatusOut

   Printlog Chr(13) + "End of Level 1 Test - CJK/CTL Dependency"
   Printlog "Duration: "+ WieLange ( StartZeit )
   Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   use "global\tools\includes\optional\t_toolbar_tools1.inc"
   use "global\tools\includes\optional\t_toolbar_writer.inc"
   Call GetUseFiles
   gApplication = "WRITER"
end Sub
