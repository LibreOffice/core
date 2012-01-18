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
'*  short description : Load documents containing hidden BASIC scripts
'*
'\******************************************************************************

testcase tBrokenSignature()

    printlog( "Load documents containing hidden BASIC scripts" )

    dim aFile as string
        aFile = "framework\optional\input\security\test_macro (signed).odt"
        aFile = convertpath( gTesttoolPath & aFile )

    printlog( "" )
    printlog( "File: " & aFile )

    FileOpen

    kontext "OeffnenDlg"
    Dateiname.setText( aFile )

    Oeffnen.click()
    hAllowMacroExecution()
    
    kontext "Active"
    if ( active.exists( 2 ) ) then
        printlog( active.getText() )
        
        if ( Active.getButtonCount() > 1 ) then
            warnlog( "This is not the expected dialog" )
            printlog( "The warning about the not recognized signature is missing" )
        else
            printlog( "This appears to be the correct dialog" )
        endif
        active.ok()
    else
        warnlog( "A messagebox was expected informing about the invalid signature" )
    endif

    call hCloseDocument()

endcase

