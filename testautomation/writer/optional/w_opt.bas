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
'* short description : WRITER-Options Test
'*
'\***********************************************************************

global NO_REAL_PRINT as boolean  ' Don't print on real printer
Global mUnit as string

sub main
    Dim StartTime
    StartTime = Now()
    NO_REAL_PRINT = True

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool5.inc"
    use "writer\tools\includes\w_tool6.inc"

    use "writer\optional\includes\option\wr_o_1.inc"
    use "writer\optional\includes\option\wr_o_2.inc"
    use "writer\optional\includes\option\wr_o_3.inc"
    use "writer\optional\includes\option\wr_o_4.inc"
    use "writer\optional\includes\option\wr_o_5.inc"
    use "writer\optional\includes\option\wr_o_a.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartTime )
    printlog Chr(13) + "******* Writer - Options - Test *******"

    gUseSysDlg = FALSE

    Call wSetMacroSecurityLevel(4)  ' Set security level to low
    Call hNewDocument
    mUnit = fSetMeasurementToCM()
    Call hCloseDocument

    Call hStatusIn("writer","w_opt.bas","Writer Option-Test")
    Call wr_o_1          'View / Grid /
    Call wr_o_2          'Defaultfont / Print / Table
    Call wr_o_3          'View 2 / Changes
    Call wr_o_4          'General / -> Caption
    Call wr_o_5          'Language Settings
    Call tToolsOptionsWriterDirectCursor2 
    Call wSetMacroSecurityLevel(3)   ' Set security to default -- medium
    Call wOptionsUndo("All")
    Call hStatusOut

    Printlog Chr(13) + "End of Options - Test :"
    Printlog "Duration: "+ WieLange ( StartTime )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end sub
