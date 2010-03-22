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
'**
'** owner : oliver.craemer@Sun.COM
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
