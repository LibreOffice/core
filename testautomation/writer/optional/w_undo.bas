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
'* short description : Testing verbosity and functionality of the undo history
'*
'\***********************************************************************

sub main
    Dim StartTime
    StartTime = Now()

	use "writer\tools\includes\w_tools.inc"
	use "writer\tools\includes\w_tool6.inc"
	use "writer\tools\includes\w_tools_undo.inc"
    use "writer\optional\includes\undo\w_undo1.inc"
	use "writer\optional\includes\undo\w_undo_history_1.inc"
	use "writer\optional\includes\undo\w_undo_history_2.inc"
	use "writer\optional\includes\undo\w_undo_history_3.inc"
	use "writer\optional\includes\undo\w_undo_history_4.inc"
    
	printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartTime )
    printlog Chr(13) + "******* Writer - Undo- Test *******"

    Call hStatusIn ( "writer", "w_undo.bas" )

   	Call tEditUndoRedo
    Call tEditUndoTOX
	Call w_undo_history_1
	Call w_undo_history_2
	Call w_undo_history_3
	Call w_undo_history_4

	Call hStatusOut

    Printlog Chr(13) + "End of Writer - Undo - Test :"
    Printlog "Duration: "+ WieLange ( StartTime )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end sub

