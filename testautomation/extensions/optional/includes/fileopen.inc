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
'*  short description : CWS oxtsysint01 enables installing extensions via file open dialog
'*
'\******************************************************************************

testcase tExtensionFileOpen

    printlog( "Install an extension via File Open - new with CWS oxtsysint01" )
    
    dim cString as string
    
    if ( gBuild < 9305 ) then
        printlog( "No testing for builds prior to 9305"
        goto endsub
    endif

    const EXTENSION_NAME = "locationtest.oxt"
    dim cPath as string 
        cPath = gTesttoolPath & "extensions/optional/input/path_to_extension/" & EXTENSION_NAME
        
    hFileOpen( cPath )
    
    kontext "Active"
    if ( Active.exists() ) then
        printlog( "Extension installation message found, accepting" )
        printlog( Active.getText() )
        Active.ok()
    else
        warnlog( "Extension installation message missing" )
    endif
    
    hExtensionRemoveGUI( EXTENSION_NAME )    

    printlog( "Verify that the Add Extension Dialog remembers the last used path..." )
    hExtensionAddGUI( cPath, "" )
    
    printlog( "Reopen Extension Manager UI, click Add.. and check the directory" )
    ToolsPackageManager
    
    kontext "PackageManager"
    if ( PackageManager.exists( 3 ) ) then
        Add.click()
        
        Kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( 2 ) ) then

            printlog( "Select the last item in the list which should be the extension" )            
            DateiAuswahl.select( Dateiauswahl.getItemCount() )
            cString = DateiAuswahl.getSelText()
            if ( cString <> EXTENSION_NAME ) then
                warnlog( "Incorrect extension listed. Please check path and filename" )
                printlog( "Expected: " & EXTENSION_NAME )
                printlog( "Found...: " & cString )
                printlog( "Issues: #i67122, #i92234" )
            else
                printlog( "Found correct extension, the dialog remembers the path" )
            endif

            kontext "OeffnenDlg"
            OeffnenDlg.cancel()
        else
            warnlog( "Could not access Add Extensions Dialog" )
        endif
        
        kontext "PackageManager"
        PackageManager.close()
    else
        warnlog( "Could not open Extension Manager GUI" )
    endif
        
    hExtensionRemoveGUI( EXTENSION_NAME )    
    
endcase


