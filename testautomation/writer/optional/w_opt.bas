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
'* owner : helge.delfs@oracle.com
'*
'* short description : WRITER-Options Test
'*
'\***********************************************************************

global NO_REAL_PRINT as boolean  ' Don't print on real printer
Global mUnit as string

sub main
    Dim StartTime
    StartTime = Now()
    NO_REAL_PRINT = True

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool5.inc"
    use "writer\tools\includes\w_tool6.inc"

    use "writer\optional\includes\option\wr_o_1.inc"
    use "writer\optional\includes\option\wr_o_2.inc"
    use "writer\optional\includes\option\wr_o_3.inc"
    use "writer\optional\includes\option\wr_o_4.inc"
    use "writer\optional\includes\option\wr_o_5.inc"
    use "writer\optional\includes\option\wr_o_a.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartTime )
    printlog Chr(13) + "******* Writer - Options - Test *******"

    gUseSysDlg = FALSE

    Call wSetMacroSecurityLevel(4)  ' Set security level to low
    Call hNewDocument
    mUnit = fSetMeasurementToCM()
    Call hCloseDocument

    Call hStatusIn("writer","w_opt.bas","Writer Option-Test")
    Call wr_o_1          'View / Grid /
    Call wr_o_2          'Defaultfont / Print / Table
    Call wr_o_3          'View 2 / Changes
    Call wr_o_4          'General / -> Caption
    Call wr_o_5          'Language Settings
    Call tToolsOptionsWriterDirectCursor2 
    Call wSetMacroSecurityLevel(3)   ' Set security to default -- medium
    Call wOptionsUndo("All")
    Call hStatusOut

    Printlog Chr(13) + "End of Options - Test :"
    Printlog "Duration: "+ WieLange ( StartTime )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end sub
