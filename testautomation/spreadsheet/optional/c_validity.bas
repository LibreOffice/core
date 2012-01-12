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
'**
'** short description : extensive test for the validity functionality
'**
'\***********************************************************************

global giWhichSecurityLevel as integer

sub main
    
    PrintLog "---------------------------------------------------------------------"
    Printlog "-----                    Calc test validity                     -----"
    PrintLog "---------------------------------------------------------------------"

    use "spreadsheet/optional/includes/validity/c_validity.inc"
    use "spreadsheet/tools/includes/c_select_tools.inc"
    use "spreadsheet/tools/includes/c_validity_tools.inc"
    
    printlog "Get the security level and set it to lower"
    Call sPreconditions
	
	Call hStatusIn("spreadsheet", "c_validity.bas")
		call tValidity_Check_Message
		call tValidity_Check_Excel        
	Call hStatusOut
	
	printlog "Set the security level to starting value"
    Call sResetSystemUnderTest
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
	use "global\tools\includes\required\t_option2.inc"
    gApplication   = "CALC"
    Call GetUseFiles
end sub

'
'-------------------------------------------------------------------------------
'
sub sPreconditions    
    'Init the iWhichSecurityLevel() variable with 2
    'which would be the normal state if it's impossible
    'to detect the macro security level.
    giWhichSecurityLevel = 2
    'Get the macro security level and remember it.
    'and then set the Macro security level to (0) low
    giWhichSecurityLevel = hSetMacroSecurity(0)
end sub
'
'-------------------------------------------------------------------------------
'
sub sResetSystemUnderTest
    'Set the macro security level back to the value
    'which has it before the system was under test
    call hSetMacroSecurity(giWhichSecurityLevel)
end sub
