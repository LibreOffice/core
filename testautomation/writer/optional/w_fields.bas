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
'* short description : Test of content and update reliability of the fields 
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
    use "writer\optional\includes\fields\w_fields1.inc"
    use "writer\optional\includes\fields\w_fields2.inc"
    use "writer\optional\includes\fields\w_fields3.inc"
    use "writer\optional\includes\fields\w_fields4.inc"
    use "writer\optional\includes\fields\w_fields5.inc"
    use "writer\optional\includes\fields\w_fields6.inc"   
    use "writer\optional\includes\fields\w_fields7.inc"   

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Fields - Test *******"

    Call hStatusIn ( "writer", "w_fields.bas" , "Fields" )

    'Software Configuration:
    'Getting the decimal seperator from global function
    gSeperator = GetDecimalSeperator()
    'Setting the measurement unit to centimeters.
    gMeasurementUnit = fSetMeasurementToCM()

    printlog Chr(13) + " --- Test Fields - General Test ---"
    Call w_fields1
    Call w_fields2
    printlog Chr(13) + " --- Test Fields - Reference ---"
    Call w_fields3
    printlog Chr(13) + " --- Test Fields - Doc information ---"
    Call w_fields4
    printlog Chr(13) + " --- Test Fields - Variables ---"
    Call w_fields5                  
    printlog Chr(13) + " --- Test Fields - Functions ---"
    Call w_fields6
    printlog Chr(13) + " --- Test Fields - L10N ---"
    Call w_fields7

    Call hStatusOut

    Printlog Chr(13) + "End of Level 1 Test - Fields"
    Printlog "Duration: "+ WieLange ( StartZeit )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"  

   Call GetUseFiles
   gApplication = "WRITER"
end Sub
