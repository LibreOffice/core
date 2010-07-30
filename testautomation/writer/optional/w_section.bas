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
