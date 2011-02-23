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
'*  short description : Use unique identifiers instead of filenames for extensions
'*
'\******************************************************************************

testcase tExtensionIdentifiers()

    printlog( "Extension identifiers / reinstallation of extensions" )
    
    if ( gBuild <= 9183 ) then
        printlog( "Build does not support extension identifiers" )
        goto endsub
    endif
    
        
    dim cExtensionPath( 2 ) as string
    dim cCommonPath as string
    dim cExtensionCount( 3 ) as integer
    const EXTENSION_NAME = "identifier.oxt"
    dim cExtensionFlags as string
    
    
    ' paths to the extensions - the have identical names but different identifiers
    cCommonPath = gTesttoolPath & "extensions\optional\input\identifier\"
    cExtensionPath( 1 ) = convertpath( cCommonPath & "legacy\"   & EXTENSION_NAME )
    cExtensionPath( 2 ) = convertpath( cCommonPath & "explicit\" & EXTENSION_NAME )
    cExtensionCount( 0 ) = hGetExtensionCount()
    
    ' Install first extension
    cExtensionFlags = "InstallForUser"
    cExtensionCount( 1 ) = hExtensionAddGUI( cExtensionPath( 1 ), cExtensionFlags )
    if ( cExtensionCount( 1 ) = cExtensionCount( 0 ) + 1 ) then
        printlog( "Extension 1 was installed" )
    else
        warnlog( "Incorrect extension count: " & cExtensionCount( 1 ) )
    endif

    ' Install second extension
    cExtensionFlags = "InstallForUser"
    cExtensionCount( 2 ) = hExtensionAddGUI( cExtensionPath( 2 ), cExtensionFlags )
    if ( cExtensionCount( 2 ) = cExtensionCount( 1 ) + 1 ) then
        printlog( "Extension 2 was installed" )
    else
        warnlog( "Incorrect extension count: " & cExtensionCount( 2 ) )
    endif
    
    ' Install first extension again
    cExtensionFlags = "InstallForUser,AllowUpdate"
    cExtensionCount( 3 ) = hExtensionAddGUI( cExtensionPath( 1 ), cExtensionFlags )
    if ( cExtensionCount( 3 ) = cExtensionCount( 2 ) ) then
        printlog( "Extension 1 was installed, overwriting existing" )
    endif


    ' Install second extension again
    cExtensionFlags = "InstallForUser,AllowUpdate"
    cExtensionCount( 3 ) = hExtensionAddGUI( cExtensionPath( 2 ), cExtensionFlags )
    if ( cExtensionCount( 3 ) = cExtensionCount( 2 ) ) then
        printlog( "Extension 2 was installed, overwriting existing" )
    endif
    
    hExtensionRemoveGUI( EXTENSION_NAME )
    hExtensionRemoveGUI( EXTENSION_NAME )
    
    ' Install first extension
    cExtensionFlags = "InstallForUser"
    cExtensionCount( 1 ) = hExtensionAddGUI( cExtensionPath( 1 ), cExtensionFlags )
    if ( cExtensionCount( 1 ) = cExtensionCount( 0 ) + 1 ) then
        printlog( "Extension 1 was installed again" )
    else
        warnlog( "Extension 1 was not installed" )
    endif

    ' Install second extension
    cExtensionFlags = "InstallForUser"
    cExtensionCount( 2 ) = hExtensionAddGUI( cExtensionPath( 2 ), cExtensionFlags )
    if ( cExtensionCount( 2 ) = cExtensionCount( 1 ) + 1 ) then
        printlog( "Extension 2 was installed again" )
    else
        warnlog( "Extension 2 was not installed" )
    endif
    
    hExtensionRemoveGUI( EXTENSION_NAME )
    hExtensionRemoveGUI( EXTENSION_NAME )
    
    if ( hGetExtensionCount() <> cExtensionCount( 0 ) ) then
        warnlog( "For some reason we have an unexpected number of extensions listed" )
    else
        printlog( "Extensions were handled correctly" )
    endif
    
endcase
