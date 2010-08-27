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
'* owner : oliver.craemer@oracle.com
'*
'* short description : Simply load max. 1000 files in .../spreadsheet/required/input/loadsave/
'*
'\***********************************************************************

global gsSourceDocument(1000) as string
global giWhichSecurityLevel as integer
global gsSourcePath as string

sub main
    use "spreadsheet\optional\includes\import_general\c_import_general.inc"
    
    gsSourcePath = ConvertPath(gTestToolPath + "spreadsheet\optional\input\loadsave\")
    'Checking in .../common sub-directory if there is a file
    'to be sure to have access to the files
    if Dir(gsSourcePath) & "common\" = "" then
        warnlog "qa:qatesttool:spreadsheet:optional:c_import_general.bas: Unable to find document directory! Please adapt 'gsSourcePath'"
        exit sub
    endif    

    Call sPreconditions
        Call hStatusIn ( "spreadsheet", "c_import_general.bas" )        
            Call tLoadAllDocuments (gsSourcePath)        
        Call hStatusOut
    Call sResetSystemUnderTest
end sub
'
'-------------------------------------------------------------------------------
'
sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "CALC"
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
'
'-------------------------------------------------------------------------------
'
