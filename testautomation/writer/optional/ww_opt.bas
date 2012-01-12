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
'* short description : HTML-Options-Test
'*
'\***********************************************************************

Global mUnit as string

sub main
    Dim StartTime
    StartTime = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool6.inc"

    use "writer\optional\includes\option\wh_o_1.inc"
    use "writer\optional\includes\option\wh_o_2.inc"
    use "writer\optional\includes\option\wh_o_3.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartTime )
    printlog Chr(13) + "******* HTML - Options - Test *******"

    Call hNewDocument
    mUnit = fSetMeasurementToCM()                 ' Set measurement unit to cm
    Call hCloseDocument

    Call hStatusIn("writer","ww_opt.bas","HTML Optiontest")
    Call wh_o_1          'Content / Layout / Grid /
    Call wh_o_2          'Defaultfont / Print / Table
    Call wh_o_3          'Cursor / Changes / Insert / Insert -> Caption
    Call hStatusOut

    Printlog Chr(13) + "End of Options - Test :"
    Printlog "Duration: "+ WieLange ( StartTime )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "HTML"
end sub
