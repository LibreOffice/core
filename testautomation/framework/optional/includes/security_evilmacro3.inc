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

testcase tSecurityEvilMacro3()

    
    dim acFile( 2 ) as string

        acFile( 1 ) = gTesttoolPath & "framework\optional\input\security\test_macro.html"
        acFile( 1 ) = convertpath( acFile( 1 ) )

        acFile( 2 ) = gTesttoolPath & "framework\optional\input\security\test_macro.odt"
        acFile( 2 ) = convertpath( acFile( 2 ) )

    dim iCurrentFile as integer ' increment variable
    dim cMsg as string          ' string from messagebox
    dim brc as boolean          ' some returnvalue
    


    for iCurrentFile = 1 to ubound( acFile() )

        printlog( "" )
        printlog( "File: " & acFile( iCurrentFile ) )

	FileOpen

	kontext "OeffnenDlg"
	Dateiname.setText( acFile( iCurrentFile ) )

        Oeffnen.click()

      
        kontext "Active"
        if ( active.exists() ) then
            cMsg = active.getText()
            cMsg = hRemoveLineBreaks( cMsg )
            printlog( "Messagebox: " & cMsg )
            active.ok()
        else
            warnlog( "Missing messagebox: Document contains macros ..." )
        endif

        kontext "Active"
        if ( active.exists( 5 ) ) then

            ' warn if macro runs without security warning
            warnlog( "A macro was executed without permission" )

            ' get the string on the messagebox
            cMsg = active.getText()
            cMsg = hRemoveLineBreaks( cMsg )
            warnlog( "Macro was executed: " & cMsg )
            active.ok()
        else
            printlog( "Macro was not executed" )
        endif

        call ExitRestartTheOffice()

    next iCurrentFile


endcase

