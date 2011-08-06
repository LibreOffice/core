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
'* short description : Test the functionality of textframes
'*
'\*******************************************************************

global gSeperator as String
global gMeasurementUnit as String

sub main

    Dim StartZeit

    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool3.inc"
    use "writer\tools\includes\w_tool6.inc"
    use "writer\optional\includes\tools\tools1.inc"
    use "writer\optional\includes\tools\tools2.inc"
    use "writer\optional\includes\textframes\w_textframes1.inc"
    use "writer\optional\includes\textframes\w_textframes2.inc"
    use "writer\optional\includes\textframes\w_textframes3.inc"
    use "writer\optional\includes\textframes\w_textframes4.inc"
    use "writer\optional\includes\textframes\w_textframes5.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"

    Call hStatusIn ( "writer", "w_textframes.bas","Writer Level1-Test" )
    'Reading the decimal seperator from global routine in tools1.inc.
    gSeperator = GetDecimalSeperator()
    'Setting the measurementunit to 'cm'
    gMeasurementUnit = fSetMeasurementToCM()
    printlog Chr(13) + "      - Test of TextFrame function -"
    Call w_textframes1

    printlog Chr(13) + "      - Test of linked TextFrame -"
    Call w_textframes2

    printlog Chr(13) + "      - Test of MS-Interoperability -"
    Call w_textframes3
    Call w_textframes4
    Call w_textframes5
    Call hStatusOut

    Printlog Chr(13) + "End of Level 1 Test (Textframes):"
    Printlog "Duration: "+ WieLange ( StartZeit )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   Call GetUseFiles
   gApplication = "WRITER"
end Sub
