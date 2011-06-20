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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
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

