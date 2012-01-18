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

