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
'* short description : Test CJK and CTL support dependency
'*
'\*******************************************************************

sub main

    Dim StartZeit

    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool3.inc"
    use "writer\optional\includes\tools\tools1.inc"
    use "writer\optional\includes\tools\tools2.inc"
    use "writer\optional\includes\CJKCTLDependency\CJKCTLDependency1.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"

    Call hStatusIn ( "writer", "w_CJKCTLDependency.bas" , "CJK Dependency test" )

    printlog Chr(13) + "      - Test CJK support dependency"

    Call tCJKDependency_1
    Call tCJKDependency_2
    Call tCJKDependency_3
    Call tCJKDependency_4
    Call tCJKDependency_5

    printlog Chr(13) + "      - Test CTL support dependency"

    Call tCTLDependency_1
    Call tCTLDependency_2
    Call tCTLDependency_3
    Call tCTLDependency_4
    Call tCTLKashidaCheck
    
    gApplication = "HTML"
    Call tCTLDependency_5
    Call tCTLDependency_6
    Call tCTLDependency_7
    Call tCTLDependency_8
    
      
   Call hStatusOut

   Printlog Chr(13) + "End of Level 1 Test - CJK/CTL Dependency"
   Printlog "Duration: "+ WieLange ( StartZeit )
   Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   use "global\tools\includes\optional\t_toolbar_tools1.inc"
   use "global\tools\includes\optional\t_toolbar_writer.inc"
   Call GetUseFiles
   gApplication = "WRITER"
end Sub
