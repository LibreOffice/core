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
'* short description : Fontwork test
'*
'\*******************************************************************

global gSeperator, gMeasurementUnit as String
sub main
    Dim StartZeit
    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\optional\includes\fontwork\w_fontwork1.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Fontwork - Test *******"

    gSeperator = GetDecimalSeperator()
    'Setting the measurement unit to centimeters.
    gMeasurementUnit = fSetMeasurementToCM()

    Call hStatusIn ( "writer" , "w_fontwork.bas" )
    Call tFontWork_1    'Test pre-defined shapes (upper/lower Semicircle,...) to the selected text object
    Call tFontWork_2    'Test Orientation
    Call tFontWork_3    'Test Align , include Left , Center, Right and Autosize
    Call tFontWork_4    'Test distance and indent
    Call tFontWork_5    'Test Contour - include contour and text contour
    Call tFontWork_6    'Test shadow - include vertical and slanted
    Call hStatusOut

    Printlog Chr(13) + "End of Fontwork - Test :"
    Printlog "Duration: "+ WieLange ( StartZeit )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end sub
