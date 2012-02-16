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
'* short description : Section test
'*
'\*******************************************************************

global gSeperator , gMeasurementUnit , gDefaultSectionName as String

sub main
    Dim StartZeit
    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
	use "writer\tools\includes\w_tool7.inc"
    use "writer\optional\includes\section\w_section_1.inc"
    use "writer\optional\includes\section\w_section_2.inc"
    use "writer\optional\includes\section\w_section_3.inc"
    use "writer\optional\includes\section\w_section_4.inc"
    use "writer\optional\includes\section\w_section_tools.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Section - Test *******"

    Call hStatusIn ( "writer" , "w_section.bas" , "Section" )

    'Software Configuration:
    'Getting the decimal seperator from global function
    gSeperator = GetDecimalSeperator()
    'Setting the measurement unit to centimeters.
    gMeasurementUnit = fSetMeasurementToCM()

    Select case iSprache
        case 01   : gDefaultSectionName = "Section"    '(English USA)
        case 31   : gDefaultSectionName = "Bereik"    '(Dutch) 
        case 33   : gDefaultSectionName = "Section"    '(French)
        case 34   : gDefaultSectionName = "Sección"    '(Spanish)
        case 36   : gDefaultSectionName = "Szakasz"    '(Hungarian)
        case 39   : gDefaultSectionName = "Sezione"    '(Italian)
        case 46   : gDefaultSectionName = "Område"     '(Swedish)
        case 49   : gDefaultSectionName = "Bereich"    '(German)
        case 55   : gDefaultSectionName = "Seção"    '(Portuguese)
        case 81   : gDefaultSectionName = "範囲"      '(Japanese)
        case 82   : gDefaultSectionName = "구역"      '(Korean)
        case 86   : gDefaultSectionName = "区域"      '(Simplified Chinese)
        case 88   : gDefaultSectionName = "�?�域"       '(Traditional Chinese)
        case else : QAErrorLog "The test does not support the language " + iSprache
        exit sub
    end select

    Call w_section_1
    Call w_section_2
    Call w_section_3
    Call w_section_4

    Call hStatusOut

    Printlog Chr(13) + "End of Level1 - Test :"
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
