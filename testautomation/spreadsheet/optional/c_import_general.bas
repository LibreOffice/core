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
