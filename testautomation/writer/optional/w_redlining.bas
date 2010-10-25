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
'* short description : Test of the Redlining function 
'*
'\*******************************************************************

sub main
    Dim StartZeit
    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool2.inc"
    use "writer\tools\includes\w_tool3.inc"
    use "writer\optional\includes\tools\tools1.inc"
    use "writer\optional\includes\tools\tools2.inc"
    use "writer\optional\includes\redlining\w_redlining1.inc"
    use "writer\optional\includes\redlining\w_redlining2.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"

    Call hStatusIn ( "writer", "w_redlining.bas" , "Redlining" )

    Call tRedlining_1
    Call tRedlining_2
    Call tRedlining_3
    Call tRedlining_4
    Call tRedlining_5
    Call tRedlining_6
    Call tRedlining_7
    Call tRedlining_8
    Call tRedlining_9
    Call tRedlining_10
    Call tRedlining_11
    Call tRedlining_12
    Call tRedlining_13
    Call tRedlining_14
    Call tRedlining_15
    Call tRedlining_16

    Call tAcceptReject_1
    Call tAcceptReject_2
    Call tAcceptReject_3
    Call tAcceptReject_4
    Call tAcceptReject_5
    Call tAcceptReject_6
    Call tAcceptReject_7
    Call tAcceptReject_8
    Call tAcceptReject_9
    Call tAcceptReject_10
    Call tAcceptReject_11
    Call tAcceptReject_12
    Call tAcceptReject_13
    Call tAcceptReject_14
    Call tAcceptReject_15

    Call hStatusOut

    Printlog Chr(13) + "End of Level 1 Test (Redlining):"
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
