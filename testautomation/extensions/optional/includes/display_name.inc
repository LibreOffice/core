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

