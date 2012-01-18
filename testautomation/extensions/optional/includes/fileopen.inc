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


