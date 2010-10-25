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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : Tools library for options testcases
'*
'\***********************************************************************

function OptionTabPageZaehler ( SollAnzahl as Integer, optional Ausnahme as Boolean )
    'parameter Ausnahme is depracted
    Dim IstAnzahl
    Kontext "ExtrasOptionenDlg"
    IstAnzahl = Optionsliste.GetItemCount - iSectionNumber
    if Ausnahme = FALSE then
        if IstAnzahl <> SollAnzahl then WarnLog "Number of tabpages old :  " + SollAnzahl + "   new :  " + IstAnzahl
    end if
    OptionTabPageZaehler = IstAnzahl
end function

'-------------------------------------------------------------------------

sub ToPosInOptionlist ( Sprung as Integer )
    Dim i as Integer
    Kontext "OptionenDlg"
    Optionsliste.TypeKeys "<HOME>"
    for i = 1 to 12
        Optionsliste.TypeKeys "-<DOWN>"
    next i
    Optionsliste.Select Sprung
    Optionsliste.TypeKeys "+"
end sub

'-------------------------------------------------------------------------

sub DialogeFuerTypenKontrollieren
    ' Complete test for path-options, for all entries the file-dialog or the path-dialog will be opened
    Dim i as Integer
    Dim iCount as Integer
    Dim iErrorCount as integer
    dim iExitCounter as integer : iExitCounter = 0
    Kontext "TabPfade"
    Typ.TypeKeys "<Down><Down><Home>"
    iCount = 0
    iErrorCount = 0
    for i=1 to Typ.GetItemCount
        if i<>1 then Typ.TypeKeys "<Down>"
        printlog "      "+typ.getText
        if ( bAsianLan <> TRUE ) then
            try
                Bearbeiten.Click
                
                do while( iExitCounter <= 10 )
                
                    kontext "Active"
                    if ( active.exists() ) then
                        iErrorCount = iErrorCount + 1
                        if (iErrorCount > 1) then
                            Warnlog active.getText
                        endif
                        qaErrorlog "#i69014# gallery path doesn't exist: '" + active.getText + "'"
                        printlog active.getText
                        active.ok
                    endif
                    
                    Kontext "OeffnenDlg"
                    if OeffnenDlg.Exists() then
                        if ( Dateityp.IsVisible ) then 
                            Warnlog "The normal FileOpen-Dialog is visible with the Filetype-Listbox => BUG!"
                        endif
                        OeffnenDlg.Cancel
                        exit do
                    endif
                    
                    Kontext "PfadeAuswaehlen"
                    if ( PfadeAuswaehlen.exists() ) then
                        PfadeAuswaehlen.Cancel
                        exit do
                    endif
                    
                    iExitCounter = iExitCounter + 1
                loop
                Kontext "TabPfade"
            catch
                Warnlog "Error on entry " & i & "!"
                Exceptlog
            endcatch
        else
            try
                if ( Bearbeiten.IsEnabled ) then
                    Bearbeiten.Click
                    
                    do while( iExitCounter <= 10 ) 
                    
                        kontext "Active"
                        if active.exists () then
                            iErrorCount = iErrorCount + 1
                            if (iErrorCount > 1) then
                                Warnlog active.getText
                            endif
                            qaErrorlog "WorkAround for #109107# has to come up only once @'Add-Ins'! else BUG!"
                            active.ok
                        endif
                        
                        Kontext "OeffnenDlg"
                        if OeffnenDlg.Exists() then
                            if ( Dateityp.IsVisible ) then 
                                Warnlog "The normal FileOpen-Dialog is visible with the Filetype-Listbox => BUG!"
                            endif
                            OeffnenDlg.Cancel
                            exit do
                        endif
                        
                        Kontext "PfadeAuswaehlen"
                        if ( PfadeAuswaehlen.exists() ) then
                            PfadeAuswaehlen.Cancel
                            exit do
                        end if

                        iExitCounter = iExitCounter + 1
                    loop
                    Kontext "TabPfade"
                else
                    iCount = iCount + 1
                end if
            catch
                Warnlog "Error on entry " & i & "!"
                Exceptlog
            endcatch
        end if
    next i
    if iCount > 3 then Warnlog "There are more than 3 entries are disabled!"

end sub

'-------------------------------------------------------------------------

function hSetMacroSecurity( iLevel as integer ) as integer

    '///<h3>Set macro security level via GUI</h3>
    '///<i>Set the macro security by accessing the Tools/Options->
    '///+ OpenOffice.org/Security::Macro... Tabpage</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>Desired macro security level (Integer). Following symbolic names are defined:</li>
    '///<ul>
    '///+<li>GC_MACRO_SECURITY_LEVEL_LOW (0) for low security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_MEDIUM (1) for medium security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_HIGH (2) for high security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_VERYHIGH (3) for very high security</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Previous security level (Integer)</li>
    '///<ul>
    '///+<li>GC_MACRO_SECURITY_LEVEL_LOW (0) for low security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_MEDIUM (1) for medium security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_HIGH (2) for high security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_VERYHIGH (3) for very high security</li>
    '///+<li>-1 on error</li>
    '///</ul>
    '///</ol>



    '///Switch between macro security levels in Tools/Options
    '///<ul>
    const CFN = "hSetMacroSecurity::"
    const RETVAL_FAILURE = -1
    const DEFAULT_WAIT_TIME = 2

    dim iOldSecurityLevel as integer

    dim caLevel( 3 ) as string
    caLevel( 0 ) = "low"
    caLevel( 1 ) = "medium"
    caLevel( 2 ) = "high"
    caLevel( 3 ) = "very high"

    if ( ( iLevel < GC_MACRO_SECURITY_LEVEL_LOW ) or ( iLevel > GC_MACRO_SECURITY_LEVEL_VERYHIGH ) ) then
        warnlog( CFN & "Invalid index (0...3) passed to function: " & ilevel )
        hSetMacroSecurity() = RETVAL_FAILURE
        exit function
    end if

    '///+<li>Open Tools/Options -> OpenOffice.org/Security</li>
    ToolsOptions
    hToolsOptions( "Staroffice" , "Security" )

    '///+<li>Click on the macro security button</li>
    Kontext "TabSecurity"
    if ( MacroSecurity.exists( DEFAULT_WAIT_TIME ) ) then
        MacroSecurity.click()

        '///+<li>Ensure we are on the Security Level page</li>
        kontext "Active"
        if ( Active.exists( DEFAULT_WAIT_TIME ) ) then

            Kontext
            active.setpage TabSecurityLevel

            '///+<li>Get the current setting (=returnvalue)</li>
            Kontext "TabSecurityLevel"
            if ( TabSecurityLevel.exists( 2 ) ) then
                if ( veryhigh.isChecked() ) then
                    iOldSecurityLevel = GC_MACRO_SECURITY_LEVEL_VERYHIGH
                elseif( high.isChecked() ) then
                    iOldSecurityLevel = GC_MACRO_SECURITY_LEVEL_HIGH
                elseif( medium.isChecked() ) then
                    iOldSecurityLevel = GC_MACRO_SECURITY_LEVEL_MEDIUM
                elseif( low.isChecked() ) then
                    iOldSecurityLevel = GC_MACRO_SECURITY_LEVEL_LOW
                end if
            else
                printlog( CFN & "Security Tabpage not available. Aborting." )
                kontext "OptionenDlg"
                if ( OptionenDlg.exists( DEFAULT_WAIT_TIME ) ) then
                    OptionenDlg.cancel()
                else
                    warnlog( CFN & "Unrecoverable error, status unknown." )
                endif
                hSetMacroSecurity() = RETVAL_FAILURE
                exit function
            endif

            '///+<li>Set the new security level</li>
            select case iLevel
            case GC_MACRO_SECURITY_LEVEL_LOW      : low.check()
            case GC_MACRO_SECURITY_LEVEL_MEDIUM   : medium.check()
            case GC_MACRO_SECURITY_LEVEL_HIGH     : high.check()
            case GC_MACRO_SECURITY_LEVEL_VERYHIGH : veryhigh.check()
            end select

            printlog( CFN & "Setting macro security level to " & caLevel( iLevel ) )

        else
            printlog( CFN & "Macro Security Dialog did not open. Aborting." )
            kontext "OptionenDlg"
            if ( OptionenDlg.exists( DEFAULT_WAIT_TIME ) ) then
                OptionenDlg.cancel()
            else
                warnlog( CFN & "Unrecoverable error, status unknown." )
            endif
            hSetMacroSecurity() = RETVAL_FAILURE
            exit function
        endif

        '///+<li>Close Tools/Options</li>
        TabSecurityLevel.ok()
    else
        warnlog( CFN & "The Macro Security Button is not available" )
        iOldSecurityLevel = RETVAL_FAILURE
    end if
    Kontext "OptionenDLG"
    OptionenDLG.OK()
    '///</ul>
    hSetMacroSecurity() = iOldSecurityLevel
end function

'*******************************************************************************

function hGetMacroSecurityAPI() as integer


    '///<h3>Retrieve the macro security level via API</h3>
    '///<i>Use remote UNO to quickly retrieve the current macro security level.
    '///+ This function runs silent unless an error is encountered.</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>
    '///+<li>No input parameters</li>
    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Macro Security Level (Integer)</li>
    '///<ul>
    '///+<li>GC_MACRO_SECURITY_LEVEL_LOW (0) for low security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_MEDIUM (1) for medium security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_HIGH (2) for high security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_VERYHIGH (3) for very high security</li>
    '///+<li>-1 = Error</li>
    '///</ul>
    '///</ol>

    const CFN = "hGetMacroSecurityAPI::"
    const RETVAL_FAILURE = -1

    dim oUnoOfficeConnection as object
    dim oUnoConfigurationAccess as object
    dim aPropertyValue( 1 ) As new com.sun.star.beans.PropertyValue
    dim iLevel as integer
    dim xViewRoot

    try

        oUnoOfficeConnection=hGetUnoService(TRUE)
        oUnoConfigurationAccess=oUnoOfficeConnection.createInstance("com.sun.star.configuration.ConfigurationProvider")

        aPropertyValue( 0 ).Name  = "nodepath"
        aPropertyValue( 0 ).Value = "/org.openoffice.Office.Common/Security/Scripting"
        aPropertyValue( 1 ).Name  = "lazywrite"
        aPropertyValue( 1 ).Value = FALSE

        xViewRoot=oUnoConfigurationAccess.createInstanceWithArguments( "com.sun.star.configuration.ConfigurationUpdateAccess", aPropertyValue() )
        iLevel = xViewRoot.getByName( "MacroSecurityLevel" )
        xViewRoot.dispose()

    catch

        warnlog( CFN & "Failed to retrieve macro security Level via API" )
        iLevel = RETVAL_FAILURE

    endcatch

    hGetMacroSecurityAPI() = iLevel

end function


'*******************************************************************************

function hSetMacroSecurityAPI( iSecLevel as integer ) as integer


    '///<h3>Set macro security level using API</h3>
    '///<i>Set the macro security using remote uno. This implementation does exactly
    '///+ the same as hSetMacroSecurity but is considerably faster</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>Desired macro security level (Integer). Following symbolic names are defined:</li>
    '///<ul>
    '///+<li>GC_MACRO_SECURITY_LEVEL_LOW (0) for low security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_MEDIUM (1) for medium security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_HIGH (2) for high security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_VERYHIGH (3) for very high security</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Previous security level (Integer)</li>
    '///<ul>
    '///+<li>GC_MACRO_SECURITY_LEVEL_LOW (0) for low security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_MEDIUM (1) for medium security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_HIGH (2) for high security</li>
    '///+<li>GC_MACRO_SECURITY_LEVEL_VERYHIGH (3) for very high security</li>
    '///+<li>-1 on error</li>
    '///</ul>
    '///</ol>

    const CFN = "hSetMacroSecurityAPI::"

    dim oUnoOfficeConnection as object
    dim oUnoConfigurationAccess as object
    dim aPropertyValue( 1 ) As new com.sun.star.beans.PropertyValue
    dim iLevel as integer
    dim xViewRoot

    dim caLevel( 3 ) as string
    caLevel( 0 ) = "low"
    caLevel( 1 ) = "medium"
    caLevel( 2 ) = "high"
    caLevel( 3 ) = "very high"

    aPropertyValue( 0 ).Name  = "nodepath"
    aPropertyValue( 0 ).Value = "/org.openoffice.Office.Common/Security/Scripting"
    aPropertyValue( 1 ).Name  = "lazywrite"
    aPropertyValue( 1 ).Value = FALSE

    '///<u>Description:</u>
    '///<ul>

    iLevel = 0

    '///+<li>Verify input parameter, quit function with a warning on error</li>
    if ( iSecLevel < GC_MACRO_SECURITY_LEVEL_LOW or iSecLevel > GC_MACRO_SECURITY_LEVEL_VERYHIGH ) then

        warnlog( CFN & "Invalid security level passed to function: " & iSecLevel )
        hSetMacroSecurityAPI() = -1
        exit function

    endif


    '///+<li>Get current security level, set the new one</li>
    try

        oUnoOfficeConnection=hGetUnoService(TRUE)
        oUnoConfigurationAccess=oUnoOfficeConnection.createInstance("com.sun.star.configuration.ConfigurationProvider")

        xViewRoot=oUnoConfigurationAccess.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPropertyValue())
        iLevel = xViewRoot.getByName( "MacroSecurityLevel" )            ' Get the current value
        xViewRoot.setPropertyValue( "MacroSecurityLevel" , iSecLevel )  ' Set the new value
        xViewRoot.commitChanges()
        xViewRoot.dispose()

        'printlog( CFN & "Setting macro security level to " & caLevel( iSecLevel )

    catch

        warnlog( CFN & "Failed to get/set new macro security level via API:" & iSecLevel )
        iLevel = -1

    endcatch
    '///</ul>

    hSetMacroSecurityAPI() = iLevel

end function

'*******************************************************************************

function hIsOOoImprovementTabVisible() as boolean

    ' This function tries to find out whether the OOo Improvement Program
    ' Tabpage in Tools/Options is visible or not.

    dim oUnoConnect as object
    dim oOOoImprovementController as object
    dim bExists as boolean
    const CFN = "hIsOOoImprovementTabVisible()::"

    oUnoConnect = hGetUnoService( TRUE )
    if ( isNull( oUnoConnect ) ) then
        warnlog( CFN & "Failed to establish UNO connection, hGetUnoService failed" )
        hIsOOoImprovementTabVisible() = FALSE
        exit function
    else
        try
            oOOoImprovementController = oUnoConnect.createInstance( "com.sun.star.oooimprovement.CoreController" )
            bExists = oOOoImprovementController.showBuiltinOptionsPage( 1 )
            if ( bExists ) then
                'printlog( CFN & "The OOo Improvement Program Tabpage is visible" )
                hIsOOoImprovementTabVisible() = TRUE
            else
                printlog( CFN & "The OOo Improvement Program Tabpage is not visible" )
                hIsOOoImprovementTabVisible() = FALSE
            endif
        catch
            warnlog( CFN & "Method com.sun.star.oooimprovement.CoreController.showBuiltinOptionsPage() not available." )
            printlog( CFN & "This indicates that the OOo Improvement Program feature is not included in this build." )
            hIsOOoImprovementTabVisible() = FALSE
        endcatch
    endif

end function

'*******************************************************************************

function hEnableOOoImprovementProgramUNO() as boolean

    ' This function tries to enable the OOo Improvement Program (switch it on)
    dim oUnoOfficeConnection as object
    dim oUnoConfigurationAccess as object
    dim aPropertyValue(1) As new com.sun.star.beans.PropertyValue ' Array of pairs: Property with Value
    dim xViewRoot

    const CFN = "hEnableOOoImprovementProgramUNO()::"

    aPropertyValue(0).Name = "nodepath"
    aPropertyValue(0).Value = "/org.openoffice.Office.OOoImprovement.Settings/Participation"
    aPropertyValue(1).Name = "lazywrite"
    aPropertyValue(1).Value = False

    oUnoOfficeConnection = hGetUnoService( TRUE )
    if ( isNull( oUnoOfficeConnection )) then
        warnlog( CFN & "Failed to establish UNO connection, hGetUnoService failed" )
        hEnableOOoImprovementProgramUNO() = FALSE
    else
        oUnoConfigurationAccess = oUnoOfficeConnection.createInstance("com.sun.star.configuration.ConfigurationProvider")
        xViewRoot = oUnoConfigurationAccess.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPropertyValue())
        xViewRoot.replaceByName( "ShowedInvitation", TRUE )
        xViewRoot.replaceByName( "InvitationAccepted", TRUE )
        xViewRoot.commitChanges()
        hEnableOOoImprovementProgramUNO() = TRUE
        if ( xViewRoot.hasPendingChanges() ) then
            hEnableOOoImprovementProgramUNO() = FALSE
        endif
        xViewRoot.dispose()
    endif

end function

'*******************************************************************************

function hDisableOOoImprovementProgramUNO() as boolean

    ' This function tries to enable the OOo Improvement Program (switch it on)
    dim oUnoOfficeConnection as object
    dim oUnoConfigurationAccess as object
    dim aPropertyValue(1) As new com.sun.star.beans.PropertyValue ' Array of pairs: Property with Value
    dim xViewRoot

    const CFN = "hDisableOOoImprovementProgramUNO()::"

    aPropertyValue(0).Name = "nodepath"
    aPropertyValue(0).Value = "/org.openoffice.Office.OOoImprovement.Settings/Participation"
    aPropertyValue(1).Name = "lazywrite"
    aPropertyValue(1).Value = False

    oUnoOfficeConnection = hGetUnoService( TRUE )
    if ( isNull( oUnoOfficeConnection )) then
        warnlog( CFN & "Failed to establish UNO connection, hGetUnoService failed" )
        hDisableOOoImprovementProgramUNO() = FALSE
    else
        oUnoConfigurationAccess = oUnoOfficeConnection.createInstance("com.sun.star.configuration.ConfigurationProvider")
        xViewRoot = oUnoConfigurationAccess.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPropertyValue())
        xViewRoot.replaceByName( "ShowedInvitation", TRUE )
        xViewRoot.replaceByName( "InvitationAccepted", FALSE )
        xViewRoot.commitChanges()
        hDisableOOoImprovementProgramUNO() = TRUE
        if ( xViewRoot.hasPendingChanges() ) then
            hDisableOOoImprovementProgramUNO() = FALSE
        endif
        xViewRoot.dispose()
    endif

end function
