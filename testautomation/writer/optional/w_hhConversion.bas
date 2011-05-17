'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
' 
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* owner :  helge.delfs@oracle.com
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
