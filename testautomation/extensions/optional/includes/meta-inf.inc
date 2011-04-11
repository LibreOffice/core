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
'*  short description : Case sensitive META-INF in packages
'*
'\******************************************************************************

testcase tLowerCaseMetaInf
    
    printlog( "The filename <META-INF> in lowercase letters may prevent installation" )

    dim cExtensionName as string
        cExtensionName = "lowercasemetainf.oxt"
    
    dim cExtensionPath as string
        cExtensionPath = gTesttoolPath & "extensions\optional\input\errors\"
        cExtensionPath = cExtensionPath & cExtensionName

    dim irc as integer
    
    printlog( "Install extension: " & cExtensionPath )
    irc = hExtensionAddGUI( cExtensionPath, "AcceptLicense,InstallForUser" )

    if ( irc > 0 ) then
    
        printlog( "Check for unexpected messagebox" )
        kontext "Active"
        if ( Active.exists( 3 ) ) then
            warnlog( "Extension should install after license has been displayed" )
            printlog( Active.getText() )
            
            printlog( "Trying to end test gracefully" )
            Active.ok()

            printlog( "Close Extension Manager" )
            kontext "PackageManager"
            if ( PackageManager.exists( 3 ) ) then
                PackageManager.close()
            else
                warnlog( "Cannot access Extension Manager" )
            endif

        else
            printlog( "No messagebox, good." )
        endif
        
        printlog( "Remove extension" )
        hExtensionRemoveGUI( cExtensionName )
        
    else
    
        warnlog( "The extension was not installed" )
        
    endif

endcase

