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
'* short description : Test Hangul/Hanja Conversion 
'*
'\*******************************************************************

sub main

    Dim StartZeit
    Dim bAsianLanguage   as Boolean
    StartZeit = Now()
    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool3.inc"
    use "writer\optional\includes\tools\tools1.inc"
    use "writer\optional\includes\tools\tools2.inc"
    use "writer\optional\includes\hangulhanjaonversion\hhConversion1.inc"
    use "writer\optional\includes\hangulhanjaonversion\hhConversion2.inc"
    use "writer\optional\includes\hangulhanjaonversion\hhConversion3.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"

    Call hStatusIn ( "writer", "w_hhConversion.bas" , "HH Converstion" )

    printlog Chr(13) + "      - Test Hangul/Hanja Conversion"

    'Enable 'Asian Language support' ON
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)

    printlog Chr(13) + "      - No selection "

    Call tHHNoSelction_1
    Call tHHNoSelction_2
    Call tHHNoSelction_3
    Call tHHNoSelction_4
    Call tHHNoSelction_5
    Call tHHNoSelction_6
    Call tHHNoSelction_7

    printlog Chr(13) + "      - Single selection "

    Call tHHSingleSelction_1

    printlog Chr(13) + "      - Multi selection "

    Call tHHMultiSelction_1

    printlog Chr(13) + "      - in text box and draw box "

    Call tHHTextBox_1
    Call tHHDrawBox_1

    printlog Chr(13) + "      - Hangul/Hanja Conversion"

    Call   tHHConversion_1
    Call   tHHConversion_2
    Call   tHHConversion_3
    Call   tHHConversion_4
    Call   tHHConversion_5
    Call   tHHConversion_6
    Call   tHHConversion_7
    Call   tHHConversion_8
    Call   tHHConversion_9
    Call   tHHConversion_10
    Call   tHHConversion_11
    Call   tHHConversion_12
    Call   tHHConversion_13
    Call   tHHConversion_14
    Call   tHHConversion_15
    Call   tHHConversion_16
    Call   tHHConversion_17
    Call   tHHConversion_18
    Call   tHHConversion_19
    Call   tHHConversion_20
    Call   tHHConversion_21
    Call   tHHConversion_22
    Call   tHHConversion_23
    Call   tHHConversion_24
    Call   tHHConversion_25
    Call   tHHConversion_26

    printlog Chr(13) + "      - Hangul/Hanja Options"

    Call   tHHOptions_1
    Call   tHHOptions_2
    Call   tHHOptions_3
    Call   tHHOptions_4
    Call   tHHOptions_5
    Call   tHHOptions_6
    Call   tHHOptions_7
    Call   tHHOptions_8
    Call   tHHOptions_9
    Call   tHHOptions_10
    Call   tHHOptions_11
    Call   tHHOptions_12

    'Set the 'Asian Language support' to default
    Call ActiveDeactivateAsianSupport(bAsianLanguage)
    Call hStatusOut

    Printlog Chr(13) + "End of Level 1 Test - Hangul/Hanja Conversion"
    Printlog "Duration: "+ WieLange ( StartZeit )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end Sub
