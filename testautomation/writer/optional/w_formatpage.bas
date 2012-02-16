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
'* short description : Test the functionality of Page Formatting / Page Styles
'*
'\*******************************************************************

global gSeperator as String
global gMeasurementUnit as String

sub main
    Dim StartZeit

    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool3.inc"
	use "writer\tools\includes\w_tool7.inc"
    use "writer\optional\includes\tools\tools1.inc"
    use "writer\optional\includes\tools\tools2.inc"
    use "writer\optional\includes\formatpage\w_formatpage1.inc"
    use "writer\optional\includes\formatpage\w_formatpage2.inc"
    use "writer\optional\includes\formatpage\w_formatpage3.inc"
    use "writer\optional\includes\formatpage\w_formatpage4.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"
    printlog Chr(13) + "      - Test of page function -"

    Call hStatusIn ( "writer", "w_FormatPage.bas" , "Page" )

    Call wOptionsUndo ( "All" )        ' Sets all writer-options to default
    'Software Configuration:
    'Getting the decimal seperator from global function
    gSeperator = GetDecimalSeperator()
    'Setting the measurement unit to centimeters.
    gMeasurementUnit = fSetMeasurementToCM()

    Call tFormatPage_1
    Call tFormatPage_2
    Call tFormatPage_3
    Call tFormatPage_4
    Call tFormatPage_5
    Call tFormatPage_6
    Call tFormatPage_7
    Call tFormatPage_8
    Call tFormatPage_9
    Call tFormatPage_10
    Call tFormatPage_11
    Call tFormatPage_12
    Call tFormatPage_13
    Call tFormatPage_14
    Call tFormatPage_15
    Call tFormatPage_16
    Call tFormatPage_17
    Call tFormatPage_18
    Call tFormatPage_19
    Call tFormatPage_20
    Call tFormatPage_21
    Call tFormatPage_22
    Call tFormatPage_23
    Call tFormatPage_24
    Call tFormatPage_25
    Call tFormatPage_26
    Call tFormatPage_27
    Call tFormatPage_28
    Call tFormatPage_29
    Call tFormatPage_30
    Call tFormatPage_31
    Call tFormatPage_32
    Call tFormatPage_33
    Call tFormatPage_34
    Call tFormatPage_35
    Call tFormatPage_36
    Call tFormatPage_37
    Call tFormatPage_38
    Call tFormatPage_39
    Call tFormatPage_40
    Call tFormatPage_41
    Call tFormatPage_42
    Call tFormatPage_43
    Call tFormatPage_44
    Call tFormatPage_45
    Call tFormatPage_46
    Call tFormatPage_47
    Call tFormatPage_48
    Call tFormatPage_49
    Call tFormatPage_50
    Call tFormatPage_51
    Call tFormatPage_52
    Call tFormatPage_53
    Call tFormatPage_54
    Call tFormatPage_55
    Call tFormatPage_56
    Call tFormatPage_57
    Call tFormatPage_58
    Call tFormatPage_59
    Call tFormatPage_60

    Call hStatusOut

    Printlog Chr(13) + "End of Level 1 Test (Format page):"
    Printlog "Duration: "+ WieLange ( StartZeit )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end sub

