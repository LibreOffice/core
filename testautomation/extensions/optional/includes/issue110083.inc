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

