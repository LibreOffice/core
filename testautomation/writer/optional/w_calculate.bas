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
'* short description : Basic calculation functionality check
'*
'\***********************************************************************

global DecimalSeparator as string

sub main
    Dim StartTime
    StartTime = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool3.inc"
    use "writer\tools\includes\w_tool6.inc"
    use "writer\optional\includes\calculate\w_calculate.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartTime )
    printlog Chr(13) + "******* Writer - Calculation - Test *******"

    ' Seperator to calculate with is not the same as used in UI
    ' so we can't use GetDecimalSeparator here
    Call hNewDocument
    Call wTypeKeys ("1/2")
    Call wTypeKeys ("<Shift Home>")
    ToolsCalculate
    if Instr(GetClipBoardtext, ",") > 0 then DecimalSeparator = ","
    if Instr(GetClipBoardtext, ".") > 0 then DecimalSeparator = "."
    Call hCloseDocument

    Call hStatusIn ( "writer", "w_calculate.bas","Writer Calculation-Test" )
    Call w_calculate
    Call hStatusOut

    Printlog Chr(13) + "End of Calculation - Test :"
    Printlog "Duration: "+ WieLange ( StartTime )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end sub
