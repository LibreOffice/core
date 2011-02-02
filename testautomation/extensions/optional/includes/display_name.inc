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
'*  short description : Extension display names
'*
'\******************************************************************************

testcase tExtensionDisplayName

    if ( gISOLang <> "en-US" ) then
        qaerrorlog( "Test is not locale-safe, en-US only" )
        goto endsub
    endif

    '///<h1>Extension display names</h1>
    '///<u><pre>Synopsis</pre></u>Since CWS jl76 a possibility exists to give an extension a name which
    '///+ differs from the filename to be displayed in the extension manager.<br>
    '///<u><pre>Specification</pre></u>http://specs.openoffice.org/appwide/packagemanager/packagemanager_gui_spec.sxw<br>
    '///<u><pre>Files used</pre></u>gTesttoolPath/extensions/features/input/display_name/*.oxt
    
    const EXTENSION_COUNT = 5
    
    dim cExtensionPath as string
        cExtensionPath = gTesttoolPath & "extensions\optional\input\display_name\"
        cExtensionPath = convertpath( cExtensionPath )
        
    dim cExtensionFileName( EXTENSION_COUNT ) as string
        cExtensionFileName( 1 ) = "name1.oxt"
        cExtensionFileName( 2 ) = "name2.oxt"
        cExtensionFileName( 3 ) = "name3.oxt"
        cExtensionFileName( 4 ) = "name4.oxt"
        cExtensionFileName( 5 ) = "name5.oxt"
        
    dim cExtensionDisplayName( EXTENSION_COUNT ) as string
        cExtensionDisplayName( 1 ) = "name1 en-US"
        cExtensionDisplayName( 2 ) = "name2 en-US-region1"
        cExtensionDisplayName( 3 ) = "name3 en"
        cExtensionDisplayName( 4 ) = "name4 en-GB"
        cExtensionDisplayName( 5 ) = "name5 de"     
        
    dim iCurrentExtension as integer     
    dim cCurrentExtension as string      
    dim iStatus as integer     
    dim bStatus as boolean     
        
    '///<u><pre>Test case specification</u></pre>
    '///<ul>
    '///+<li>Install test extensions via GUI</li>    
    for iCurrentExtension = 1 to EXTENSION_COUNT
        
        cCurrentExtension = cExtensionPath & cExtensionFileName( iCurrentExtension )
        
        printlog( "" )
        printlog( "Adding extension by filename: " & cCurrentExtension )
        
        iStatus = hExtensionAddGUI( cCurrentExtension , "NoLicense, InstallForUser, NoUpdate, Verbose" )
        if ( iStatus  < 1 ) then
            warnlog( "Failed to install extension: " & cCurrentExtension )
        endif
        
    next iCurrentExtension
    
    '///+<li>Remove all extensions by display name (which also verifies the display name)</li>
    bStatus = true
    for iCurrentExtension = 1 to EXTENSION_COUNT

        cCurrentExtension =  cExtensionDisplayName( iCurrentExtension )
        
        printlog( "" )
        printlog( "Removing extension by display name: " &  cCurrentExtension )

        iStatus = hExtensionRemoveGUI( cCurrentExtension )
        if ( iStatus <> 0 ) then
            warnlog( "Failed to remove extension: " & cCurrentExtension )
            bStatus = false
        endif
    
    next iCurrentExtension
    
    '///+<li>If anything went wrong, reset the office/userlayer</li>
    if ( not bStatus ) then
        call sResetTheOffice
    endif
    '///</ul>

endcase

