'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
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
'*  short description : CWS sb112 fixes a problem with configuration schemas
'*
'\******************************************************************************

testcase tExtensionIssue110083()

    printlog( "Configuration/-schema .xcu/.xcs-files which might trigger exceptions from unopkg/Extension Manager" )

    const NUMBER_OF_TEST_EXTENSIONS = 2

    dim cExtensionNames( NUMBER_OF_TEST_EXTENSIONS ) as string
        cExtensionNames( 1 ) = "LoggingOptions.xcs"
        cExtensionNames( 2 ) = "Logging.xcu"

    dim cErrorIdentifier( NUMBER_OF_TEST_EXTENSIONS ) as string
        cErrorIdentifier( 1 ) = "no component element"
        cErrorIdentifier( 2 ) = "no component element"

    dim cExtensionPath as string
        cExtensionPath = gTesttoolPath & "extensions/optional/input/issues/"

    dim cExtension as string

    dim iCurrentExtension as integer
        

    for iCurrentExtension = 1 to NUMBER_OF_TEST_EXTENSIONS

        cExtension = convertpath( cExtensionPath & cExtensionNames( iCurrentExtension ) )
        hExtensionAddGUI( cExtension, "" )

        kontext "Active"
        if ( Active.exists() ) then
            if ( instr( Active.getText, cErrorIdentifier( iCurrentExtension ) ) > 0 ) then
                warnlog( "#i110083# - Error installing .xcs/.xcu-file: " & cExtensionNames( iCurrentExtension ) )
                Active.ok()

                kontext "PackageManager"
                PackageManager.close()
            else
                warnlog( "Unexpected messagebox displayed. It cannot be handled" )
            endif
        else
            hExtensionRemoveGUI( cExtensionNames( iCurrentExtension ) )
        endif

    next iCurrentExtension

endcase

