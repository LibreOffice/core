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
'* short description : Test the functionality of Paragraph
'*
'\*******************************************************************

global gSeperator as String
global gMeasurementUnit as String

sub main

    Dim StartZeit

    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool3.inc"
    use "writer\optional\includes\tools\tools1.inc"
    use "writer\optional\includes\tools\tools2.inc"
    use "writer\optional\includes\formatparagraph\w_formatparagraph1.inc"
    use "writer\optional\includes\formatparagraph\w_formatparagraph2.inc"
    use "writer\optional\includes\formatparagraph\w_formatparagraph3.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"
    printlog Chr(13) + "      - Test of paragraph function -"

    Call hStatusIn ( "writer", "w_formatparagraph.bas" , "Paragrahp" )

    Call wOptionsUndo ( "All" )        ' Sets all writer-options to default
    'Software Configuration:
    'Getting the decimal seperator from global function
    gSeperator = GetDecimalSeperator()
    'Setting the measurement unit to centimeters.
    gMeasurementUnit = fSetMeasurementToCM()

    Call tFormatParagraph1
    Call tParagraphIndent1
    Call tParagraphIndent2
    Call tParagraphIndent3
    Call tParagraphIndent4
    Call tParagraphIndent5
    Call tParagraphIndent6
    Call tParagraphIndent7
    Call tParagraphIndent8
    Call tParagraphIndent9
    Call tParagraphIndent10
    Call tParagraphIndent11
    Call tParagraphIndent12
    Call tParagraphIndent13
    Call tParagraphIndent14
    Call tParagraphIndent15
    Call tParagraphIndent16
    Call tParagraphIndent17
    Call tParagraphIndent18
    Call tParagraphAlignment1
    Call tParagraphAlignment2
    Call tParagraphAlignment3
    Call tParagraphAlignment4
    Call tParagraphAlignment5
    Call tParagraphAlignment6
    Call tParagraphAlignment7
    Call tParagraphAlignment8
    Call tParagraphAlignment9
    Call tParagraphTextFlow1
    Call tParagraphTextFlow2
    Call tParagraphTextFlow3
    Call tParagraphTextFlow4
    Call tParagraphTextFlow5
    Call tParagraphTextFlow6
    Call tParagraphTextFlow7
    Call tParagraphTextFlow8
    Call tParagraphTextFlow9
    Call tParagraphTextFlow10
    Call tParagraphTextFlow11
    Call tParagraphTextFlow12
    Call tParagraphTextFlow13
    Call tParagraphTabs1
    Call tParagraphTabs2
    Call tParagraphTabs3
    Call tParagraphTabs4
    Call tParagraphTabs5
    Call tParagraphTabs6
    Call tParagraphDropCaps1
    Call tParagraphDropCaps2
    Call tParagraphDropCaps3
    Call tParagraphDropCaps4
    Call tParagraphDropCaps5
    Call tParagraphDropCaps6
    Call tParagraphDropCaps7
    Call tParagraphDropCaps8
    Call tParagraphDropCaps9
    Call tParagraphNumbering1
    Call tParagraphBorder1
    Call tParagraphBorder2
    Call tParagraphBorder3
    Call tParagraphBorder4
    Call tParagraphBorder5
    Call tParagraphBorder6

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
