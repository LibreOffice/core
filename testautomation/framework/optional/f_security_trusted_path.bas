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
'*  short description : Execute macros from trusted path
'*
'\******************************************************************************

sub main

    use "framework\optional\includes\security_trusted_path.inc"


    ' this is the trusted path we want to add to the list
    dim cWorkPath as string
        cWorkPath = gTesttoolPath & "framework\optional\input\BasicDocs\"
        cWorkPath = convertpath( cWorkPath )

    ' this is the file to be loaded, located directly below trusted path
    dim cWorkFile as string

    dim iApplication as integer ' Index of the application WRITER, CALC ...
    dim cApplication as string  ' Name of the current application
    
    dim iSecurityLevel as integer ' Security from loweset to highest (0...4)

    call hStatusIn( "framework" , "f_security_trusted_path.bas" )

    ' Add a trusted path which contains a number of files with macros
    hAddTrustedPath( cWorkPath )

    ' For each security level we verify that macros are executed
    for iSecurityLevel = 0 to 3
    
        hSetMacroSecurityAPI( iSecurityLevel )
    
        ' Cycle through the six main applications for the most recent documenttype
        for iApplication = 1 to 6 step 2 ' Do not test all, does not seem to be required.

            cWorkFile = cWorkPath & "basic" & hGetSuffix( "current" )
            cApplication = hNumericDoctype( iApplication )

            printlog( "" )
            printlog( cApplication )
            call tSecTrustedPath( cWorkFile, iSecurityLevel )

        next iApplication
        
    next iSecurityLevel

    ' Reset the trusted path, reset macro security level.
    hRemoveTrustedPath( cWorkPath )
    hSetMacrosecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )

    call hStatusOut()

end sub

'-------------------------------------------------------------------------

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    
    use "global\tools\includes\optional\t_filetools.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_security_tools.inc"
    use "global\tools\includes\optional\t_stringtools.inc"
    
    gApplication = "WRITER"
    call GetUseFiles()

end sub

