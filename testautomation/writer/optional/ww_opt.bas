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
'* owner : helge.delfs@oracle.coms
'*
'* short description : HTML-Options-Test
'*
'\***********************************************************************

Global mUnit as string

sub main
    Dim StartTime
    StartTime = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool6.inc"

    use "writer\optional\includes\option\wh_o_1.inc"
    use "writer\optional\includes\option\wh_o_2.inc"
    use "writer\optional\includes\option\wh_o_3.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartTime )
    printlog Chr(13) + "******* HTML - Options - Test *******"

    Call hNewDocument
    mUnit = fSetMeasurementToCM()                 ' Set measurement unit to cm
    Call hCloseDocument

    Call hStatusIn("writer","ww_opt.bas","HTML Optiontest")
    Call wh_o_1          'Content / Layout / Grid /
    Call wh_o_2          'Defaultfont / Print / Table
    Call wh_o_3          'Cursor / Changes / Insert / Insert -> Caption
    Call hStatusOut

    Printlog Chr(13) + "End of Options - Test :"
    Printlog "Duration: "+ WieLange ( StartTime )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "HTML"
end sub
