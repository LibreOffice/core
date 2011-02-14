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
'/************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
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
