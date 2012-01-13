'encoding UTF-8  Do not remove or change this line!
'**************************************************************
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
'**************************************************************
'*
'*  short description : Macros with all security-levels
'*
'\******************************************************************************

sub tMacroSecurityLevelsControlModule

    dim iApplication as integer

    for iApplication = 1 to 6

        printlog( "" )
        printlog( hNumericDocType( iApplication ) )
        call tMacroSecurityLevels( "current" )
        call tMacroSecurityLevels( "645" )

    next iApplication

end sub

'*******************************************************************************

testcase tMacroSecurityLevels( cFileFormat )

    printlog( "Check macro execution behavior for all macro security levels" )

    dim cWorkFile as string
        cWorkFile = gTesttoolPath & "framework\optional\input\BasicDocs\"
        cWorkFile = cWorkFile & "basic" & hGetSuffix( cFileFormat )

    dim iSecLevel as integer

    const EXPECTED_MESSAGECOUNT = 0
    const ALLOW_ONE_EXTRA_MESSAGEBOX = 1
    const ALLOW_NO_EXTRA_MESSAGEBOXES = 0
    
    ' We have quite a bunch of possible combinations here. Some trigger a security warning
    ' others do not. Depending on the security level we either get no warning at all, the
    ' "classic" Security Warning or a message that the macros will not be executed (Blocked)
        
    '         |---------------------------------------------------------|
    '         | Trusted |     Security Level / Warning displayed        |
    '         |  Path   |   Low     |  Medium   |   High    | Very High |
    '         |---------------------------------------------------------|
    '         |  Yes*)  |     No    |    No     |     No    |    No     |
    '         |---------------------------------------------------------|
    '         |   No    |     No    |    Yes    |  Blocked  |  Blocked  |
    '         |---------------------------------------------------------|
    ' *) Covered by test "f_sec_trusted_path.bas"
    
    printlog( "Load a document with macro for each of the four security levels" )
    for iSecLevel = GC_MACRO_SECURITY_LEVEL_LOW to GC_MACRO_SECURITY_LEVEL_VERYHIGH

        printlog( "Set macro security level to " & iSecLevel )
        hSetMacroSecurityAPI( iSecLevel )
        
        printlog( "Load document..." )
        hFileOpen( cWorkFile )
        
        printlog( "Check security level dependent behavior" )
        select case iSecLevel
        case GC_MACRO_SECURITY_LEVEL_LOW :

            if ( not hIdentifyExecutedMacro() ) then
                if ( gApplication = "MATH" ) then
                    qaerrorlog( "#i110184# - Macro was not executed" )
                else
                    warnlog( "Macro was not excuted" )
                endif
            endif

        case GC_MACRO_SECURITY_LEVEL_MEDIUM :

            if ( not hAllowMacroExecution() )   then
                warnlog( "#i110184# - Macro execution warning missing, check the file/security settings" )
            endif

            if ( not hIdentifyExecutedMacro() ) then
                if ( gApplication = "MATH" ) then
                    qaerrorlog( "#i110184# - Macro was not executed" )
                else
                    warnlog( "Macro was not excuted" )
                endif
            endif

        case GC_MACRO_SECURITY_LEVEL_HIGH :

            kontext "Active"
            hCloseDialog( Active, "ok" )

            if ( not hHandleActivesOnLoad( EXPECTED_MESSAGECOUNT , ALLOW_NO_EXTRA_MESSAGEBOXES ) ) then
                warnlog( "#i53710# unexpected second messagebox" )
            endif

            if ( hIdentifyExecutedMacro() ) then
                warnlog( "Macro was executed" )
            endif

        case GC_MACRO_SECURITY_LEVEL_VERYHIGH :

            kontext "Active"
            hCloseDialog( Active, "ok" )

            if ( not hHandleActivesOnLoad( EXPECTED_MESSAGECOUNT , ALLOW_NO_EXTRA_MESSAGEBOXES ) ) then
                warnlog( "#i53710# unexpected second messagebox" )
            endif

            if ( hIdentifyExecutedMacro() ) then
                warnlog( "Macro was executed" )
            endif

        end select

        printlog( "Close document" )
        hDestroyDocument()

    next iSecLevel

    printlog( "Reset security level to default" )
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )

endcase

'*******************************************************************************

function hIdentifyExecutedMacro() as boolean


    dim cMessage as string
    const CFN = "hIdentifyExecutedMacro::"

    sleep( 1 )

    kontext "active"
    if ( active.exists( 3 ) ) then

        cMessage = active.getText()
        
        if ( cMessage = "TTMacro3" ) then
            printlog( CFN & "The Macro has been executed" )
            hIdentifyExecutedMacro() = true
            active.ok()
        else
            warnlog( CFN & "The WRONG macro/dialog is open" )
            hIdentifyExecutedMacro() = false
            hHandleActivesOnLoad( 0 , 2 )
        endif
        
    else
    
        printlog( CFN & "No macro/dialog is open" )
        hIdentifyExecutedMacro() = false
        
    endif
    
end function
