'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: security_macrosecurity.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-08-28 11:40:28 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'*  owner : joerg.skottke@sun.com
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
        cWorkFile = convertpath( cWorkFile )
        cWorkFile = convertToURL( cWorkFile )

    dim iSecLevel as integer
    dim brc as boolean
    
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
    for iSecLevel = 0 to 3

        printlog( "Set macro security level to " & iSecLevel )
        hSetMacroSecurityAPI( iSecLevel )
        
        printlog( "Load document..." )
        hFileOpen( cWorkFile )
        
        printlog( "Check security level dependent behavior" )
        select case iSecLevel
        case 0 : brc = hIdentifyExecutedMacro()
                 if ( not brc ) then
                     if ( gApplication = "MATH" ) then
                         qaerrorlog( "#i68291# Math document forgets eventbinding" )
                     else
                         warnlog( "#i53711# Macro was not found/executed" )
                     endif
                 endif   
                 
        case 1 : brc = hAllowMacroExecution()
                 if ( not brc ) then
                     warnlog( "Macro execution warning missing, chek the file/security settings" )
                 endif
                 brc = hIdentifyExecutedMacro()
                 if ( not brc ) then
                     if ( gApplication = "MATH" ) then
                         qaerrorlog( "#i68291# Math document forgets eventbinding" )
                     else
                         warnlog( "#i53711# Macro was not found/executed" )
                     endif
                 endif
                 
        case 2 : brc = hHandleActivesOnLoad( 1 , 1 )
                 if ( not brc ) then
                     qaerrorlog( "#i53710# unexpected second messagebox" )
                 endif
                 brc = hIdentifyExecutedMacro()
                 if ( brc ) then
                     warnlog( "Macro was executed" )
                 endif                 
                 
        case 3 : brc = hHandleActivesOnLoad( 1 , 0 )
                 brc = hIdentifyExecutedMacro()
                 if ( brc ) then
                     warnlog( "Macro was executed" )
                 endif
        end select

        printlog( "Close document" )
        brc = hDestroyDocument()

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
