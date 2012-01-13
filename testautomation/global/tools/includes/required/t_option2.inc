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

*****

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
