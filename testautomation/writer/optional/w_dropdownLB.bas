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
'* short description : Test Drop Down List Box
'*
'\*******************************************************************

sub main

    Dim StartZeit
    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool3.inc"
    use "writer\optional\includes\tools\tools1.inc"
    use "writer\optional\includes\tools\tools2.inc"
    use "writer\optional\includes\dropdownLB\w_dropdownLB1.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"

    Call hStatusIn ( "writer", "w_dropdownLB.bas","Dropdown listbox" )
    Call wOptionsUndo ( "All" )        ' Sets all writer-options to default

    printlog Chr(13) + "      - Test Drop Down List Box"

    Call tDropDownLB_1
    Call tDropDownLB_2
    Call tDropDownLB_3
    Call tDropDownLB_4
    Call tDropDownLB_5
    Call tDropDownLB_6
    Call tDropDownLB_7
    Call tDropDownLB_8
    Call tDropDownLB_9
    Call tDropDownLB_10
    Call tDropDownLB_11
    Call tDropDownLB_12
    Call tDropDownLB_13
    Call tDropDownLB_14
    Call tDropDownLB_15
    Call tDropDownLB_16
    Call tDropDownLB_17

    Call hStatusOut

    Printlog Chr(13) + "End of Level 1 Test - Drop Down List Box"
    Printlog "Duration: "+ WieLange ( StartZeit )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end Sub
