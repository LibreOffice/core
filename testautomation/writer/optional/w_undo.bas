'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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

