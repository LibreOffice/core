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
'*  short description : Tools to ease testing of security related funtionality
'*
'\******************************************************************************

function hSetPasswordRecommendation( bEnable as boolean ) as boolean

    '///<h3>Toggle Tools/Options/OOo/Security: Password recommendation</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Mode (boolean)</li>
    '///<ul>
    '///+<li>TRUE: Switch password recommendation on</li>
    '///+<li>FALSE: Switch password recommendation off</li>
    '///</ul>
    '///</ol>

    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Prior state (boolean)</li>
    '///<ul>
    '///+<li>TRUE: Password recommendation was on</li>
    '///+<li>FALSE: Password recommendation was off</li>
    '///</ul>
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hSetPasswordRecommendation::"

    '///+<li>Open Tools/Options</li>
    ToolsOptions
    
    kontext "OptionenDlg"
    if ( OptionenDlg.exists( 2 ) ) then

        '///+<li>Go to the security page</li>
        hToolsOptions( "STAROFFICE" , "SECURITY" )

        '///+<li>Click on the &quot;Options...&quot; button</li>
        kontext "TabSecurity"
        if ( hClickButton( Options ) ) then
        
            kontext "TabSecurityOptionsAndWarnings"
            if ( TabSecurityOptionsAndWarnings.exists( 1 ) ) then

                '///+<li>Retrieve the current setting for passwor recommendation</li>
                if ( RecommendPasswordProtectionOnSaving.isChecked() ) then
                    hSetPasswordRecommendation() = true
                else
                    hSetPasswordRecommendation() = false
                endif

                '///+<li>Set the requested state</li>
                if ( bEnable ) then
                    printlog( CFN & "Enabled password recommendation" )
                    RecommendPasswordProtectionOnSaving.check()
                else
                    printlog( CFN & "Disabled password recommendation" )
                    RecommendPasswordProtectionOnSaving.uncheck()
                endif

                hCloseDialog( TabSecurityOptionsAndWarnings, "ok" )

            else
                warnlog( "Failed to open security options page" )
                hSetPasswordRecommendation() = false
            endif
            
        else
            warnlog( CFN & "Unable to click options button" )
            hSetPasswordRecommendation() = false
        endif

        '///+<li>Close Tools/Options</li>
        Kontext "ToolsOptionsDlg"
        hCloseDialog( ToolsOptionsDlg, "ok" )
    
    else
        warnlog( "Failed to open Tools/Options" )
    endif
    '///</ul>

end function

'******************************************************************************

function hOpenDigitalSignaturesDialog() as boolean

    '///<h3>Open the Digital Signatures Dialog via File-Menu</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>

    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE: The Digital Signatures Dialog is open</li>
    '///+<li>FALSE: The Digital Signatures Dialog is not open</li>
    '///</ul>
    '///</ol>

    '///<u>Description</u>:
    '///<ul>

    const CFN = "hOpenDigitalSignaturesDialog::"
    const FILE_MENU_POSITION = 1
    const SIGNATURES_MENU_POSITION_OTHER = 15
    const SIGNATURES_MENU_POSITION_MATH  = 14

    hOpenDigitalSignaturesDialog() = false

    '///+<li>Open the file-menu</li>
    call hUseMenu()
    call hMenuSelectNr( FILE_MENU_POSITION )
                                                                                
    '///+<li>Select &quot;Digital signatures...&quot;</li>
    try
        if ( gApplication = "MATH" ) then
            call hMenuSelectNr( SIGNATURES_MENU_POSITION_MATH )
        else
            call hMenuSelectNr( SIGNATURES_MENU_POSITION_OTHER )
        endif
    catch
        warnlog( CFN & "Failed to execute menuitem <Digital Signature...>" )
    endcatch

    ' At this stage either the digital signatures dialog or the messagebox
    ' <The document needs to be saved> is open. In the latter case the function
    ' returns false. The possibility that neither of both dialogs are open
    ' must be handled in the parent function. This means there is no warning
    ' here either

    '///+<li>Verify that the Digital Signatures dialog is open</li>
    kontext "DigitalSignature"
    if ( DigitalSignature.exists() ) then
        printlog( CFN & "Digital signatures is open" )
        hOpenDigitalSignaturesDialog() = true
    endif
    '///</ul>

end function

'*******************************************************************************

function hAllowMacroExecution() as boolean

    '///<h3>Allow macro execution on Macro Security Warning dialog</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>

    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE: Macro dialog is open, successfully clicked &quot;Run&quot;</li>
    '///+<li>FALSE: Dialog was not opened</li>
    '///</ul>
    '///</ol>

    '///<u>Description</u>:
    '///<ul>

    const CFN = "hAllowMacroExecution::"
    const MAX_WAIT_FOR_SECURITY_DIALOG = 5

    '///+<li>Verify that the Macro security Warning is displayed</li>
    kontext "SecurityWarning"
    if ( SecurityWarning.exists( MAX_WAIT_FOR_SECURITY_DIALOG ) ) then

        '///+<li>Click &quot;Run&quot; to allow macro execution</li>
        printlog( CFN & "Allowing macro execution" )
        hAllowMacroExecution() = hCloseDialog( SecurityWarning, "ok" )
    else
        printlog( CFN & "Macro dialog not displayed" )
        hAllowMacroExecution() = false
    endif
    '///</ul>

end function

'*******************************************************************************

function hDenyMacroExecution() as boolean

    '///<h3>Deny macro execution on Macro Security Warning dialog</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>

    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE: Macro dialog is open, successfully clicked &quot;Deny&quot;</li>
    '///+<li>FALSE: Dialog was not opened</li>
    '///</ul>
    '///</ol>

    '///<u>Description</u>:
    '///<ul>

    const CFN = "hDenyMacroExecution::"

    '///+<li>Verify that the Macro security Warning is displayed</li>
    kontext "SecurityWarning"
    if ( SecurityWarning.exists() ) then

        '///+<li>Click &quot;Cancel&quot; to deny execution</li>
        printlog( CFN & "Denied macro execution" )
        hDenyMacroExecution() = hCloseDialog( SecurityWarning, "cancel" )
    else
        printlog( CFN & "Macro dialog not displayed" )
        hDenyMacroExecution() = false
    endif
    '///</ul>

end function

'*******************************************************************************

function hSwitchMacroSecurityTab( cTab as string )

    '///<h3>Switch between Security Level and Trusted Paths</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>The Tabpage to be activated (string). Valid options are:</li>
    '///<ul>
    '///+<li>&quot;SecurityLevel&quot;</li>
    '///+<li>&quot;TrustedPaths&quot;</li>
    '///</ul>
    '///</ol>

    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>

    '///<u>Description</u>:
    '///<ul>

    kontext
   
    '///+<li>Switch to the given Tabpage</li>
    select case ( lcase( cTab ) )
    case "securitylevel"   : active.setPage TabSecurityLevel
    case "trustedpaths"    : active.setPage TabTrustedSources
    end select 

    '///</ul>

end function

'*******************************************************************************

function hAddTrustedPath( cPath as string ) as boolean

    '///<h3>Add a directory to the list of trusted paths</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Fully qualified path to the directory (string)</li>
    '///</ol>

    '///<u>Return</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE = Path was successfully added</li>
    '///+<li>FALSE = Failed to add the path</li>
    '///</ul>
    '///</ol>

    const CFN = "hAddTrustedPath::"

    dim iPathCount as integer

    '///<u>Description</u>:
    '///<ul>

    '///+<li>Open Tools/Options</li>
    ToolsOptions

    '///+<li>Switch to the Security page</li>
    hToolsOptions( "StarOffice" , "Security" )

    '///+<li>Click Macro Security</li>
    hClickButton( MacroSecurity )

    '///+<li>Switch to the Trusted Paths tab</li>
    hSwitchMacroSecurityTab( "trustedpaths" )
    kontext "TabTrustedSources"

    '///+<li>Get the number of currently listed items from the list</li>
    iPathCount = LocationsListBox.getItemCount()

    '///+<li>Click the &quot;Add&quot; button</li>
    hClickButton( LocationsAdd )

    '///+<li>Enter a path to some files containing macros</li>
    kontext "OeffnenDlg"
    DateiName.setText( cPath )

    '///+<li>Click &quot;Select&quot;</li>
    hClickButton( Oeffnen )

    '///+<li>Verify that the item has been added to the list</li>
    kontext "TabTrustedSources"
    if ( LocationsListBox.getItemCount() <> ( iPathCount + 1 ) ) then
        warnlog( CFN & "Incorrect number of items in locations listbox" )
        hAddTrustedPath() = false
    else
        printlog( "Added: " & cPath )
        hAddTrustedPath() = true
    endif

    '///+<li>Close the dialog</li>
    hCloseDialog( TabTrustedSources, "ok" )

    '///+<li>Close Tools/Options</li>
    Kontext "OptionenDlg"
    hCloseDialog( OptionenDlg, "ok" )
    '///</ul>

end function

'*******************************************************************************

function hRemoveTrustedPath( cPath as string ) as boolean

    '///<h3>Remove a directory from the list of trusted paths</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Fully qualified path to the directory (string)</li>
    '///</ol>

    '///<u>Return</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE = Path was successfully added</li>
    '///+<li>FALSE = Failed to add the path</li>
    '///</ul>
    '///</ol>

    const CFN = "hRemoveTrustedPath()"

    dim iPathCount as integer
    dim iCurrentPathItem as integer

    hRemoveTrustedPath() = false

    '///<u>Description</u>:
    '///<ul>

    '///+<li>Open Tools/Options</li>
    ToolsOptions

    '///+<li>Switch to the Security page</li>
    hToolsOptions( "StarOffice" , "Security" )

    '///+<li>Click Macro Security</li>
    hClickButton( MacroSecurity )

    '///+<li>Switch to the Trusted Paths tab</li>
    hSwitchMacroSecurityTab( "trustedpaths" )
    kontext "TabTrustedSources"

    if ( TabTrustedSources.exists() ) then

        '///+<li>Get the number of currently listed items from the list</li>
        iPathCount = LocationsListBox.getItemCount()

        '///+<li>Find the entry in the list</li>
        for iCurrentPathItem = 1 to iPathCount

            '///+<li>If it is there, delete it</li>
            LocationsListBox.select( iCurrentPathItem )
            if ( LocationsListBox.getSelText() = cPath ) then
                LocationsRemove.click()
                printlog( CFN & "Removed item at pos. " & iCurrentPathItem )
                hRemoveTrustedPath() = true
                exit for
            endif

        next iCurrentPathItem

        '///+<li>Close the dialog</li>
        hCloseDialog( TabTrustedSources, "ok" )

    else
        warnlog( CFN & "Trusted Sources Tab did not open" )
        hRemoveTrustedPath() = false
    endif

    '///+<li>Close Tools/Options</li>
    Kontext "OptionenDlg"
    hCloseDialog( OptionenDlg, "ok" )

    '///</ul>

end function


'*******************************************************************************

function hSecurityEnterPasswordOnSave( cPassword as string ) as boolean


    '///<h3>Enter password when saving a document</h3>
    '///<i>This function enters a password into the password dialog if the &quot;
    '///+ Save with password&quot; checkbox is checked. It will automatically
    '///+ generate a faulty password to be entered on first try (to see whether
    '///+ the &quot;Invalid password confirmation&quot; dialog pops up, on second
    '///+ attempt the password is confirmed correctly and thus the document should
    '///+ save ok. The office should return to the document, the File Save dialog
    '///+ should close after save.</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>Password to use (String)</li>
    '///<ul>
    '///+<li>Password should contain strange characters and glyphs if possible</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Errorcondition (Boolean)</li>
    '///<ul>
    '///+<li>TRUE if all went well</li>
    '///+<li>FALSE on any error</li>
    '///</ul>
    '///</ol>

    const CFN = "hSecurityEnterPasswordOnSave::"
    if ( GVERBOSE ) then printlog( CFN & "Enter" )

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Verify that the password dialog is present</li>
    kontext "PasswordFileSave"
    if ( PasswordFileSave.exists( 2 ) ) then
    
        printlog( CFN & "Password dialog is displayed" )
    
        '///+<li>Enter the password</li>
        Password.setText( cPassword )
        
        '///+<li>Confirm with incorrect password (e.g. append a number at random)</li>
        PasswordConfirm.setText( cPassword & "1" )
        
        '///+<li>Click on OK</li>
        hCloseDialog( PasswordFileSave, "ok" )
        
        '///+<li>There should be a warning about a faulty password confirmation</li>
        kontext "Active"
        if ( Active.exists( 1 ) ) then
            printlog( CFN & "Message: " & Active.getText() )
            
            '///+<li>Close messagebox with OK</li>
            Active.OK()
            Active.notExists( 2 )
        else
            qaerrorlog( CFN & "Warning about faulty password confirmation is missing" )
            hSecurityEnterPasswordOnSave() = false
        endif
        
        '///+<li>We should now be back on the password dialog</li>
        kontext "PasswordFileSave"
        if ( PasswordFileSave.exists() ) then
        
            printlog( CFN & "Password dialog is displayed" )
    
            '///+<li>Enter the password</li>
            Password.setText( cPassword )
        
            '///+<li>Confirm with correct password</li>
            PasswordConfirm.setText( cPassword )
        
            '///+<li>Click on OK</li>
            hCloseDialog( PasswordFileSave, "ok" )
            hSecurityEnterPasswordOnSave() = true
            
        else
            
            qaerrorlog( CFN & "The password dialog is missing after confirmation error" )     
            hSecurityEnterPasswordOnSave() = false
        
        endif
        
    else
    
        qaerrorlog( CFN & "The password dialog did not open" )
        hSecurityEnterPasswordOnSave() = false
        
    endif
   
    '///+<li>Verify that the &quot;File Save&quot; dialog is closed after saving</li>
    kontext "SpeichernDlg"
    if ( SpeichernDlg.exists() ) then
        qaerrorlog( CFN & "File Save dialog is open, cancelling." )
        SpeichernDlg.cancel()
        hSecurityEnterPasswordOnSave() = false
    endif
    '///</ul>

    if ( GVERBOSE ) then printlog( CFN & "Exit" )

end function


'*******************************************************************************

function hSecurityEnterPasswordOnLoad( cPassword as string, bValid as boolean ) as boolean

    '///<h3>Enter a password while loading a document</h3>
    '///<i>Enters a valid or invalid password while loading a passworded file</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>Password (String)</li>
    '///<ul>
    '///+<li>The password should contain special characters and glyphs</li>
    '///</ul>

    '///+<li>Password validity (Boolean)</li>
    '///<ul>
    '///+<li>TRUE: The password is correct, the document should load</li>
    '///+<li>FALSE: The password is incorrect, the document should not load</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Errorcondition (Boolean)</li>
    '///<ul>
    '///+<li>TRUE if the password protection worked as expected</li>
    '///+<li>FALSE on any error</li>
    '///</ul>
    '///</ol>

    const CFN = "hSecurityEnterPasswordOnLoad::"
    if ( GVERBOSE ) then
        printlog( CFN & "Enter with option (Password): " & cPassword )
        printlog( CFN & "Enter with option (Validity): " & bValid )
    endif

    hSecurityEnterPasswordOnLoad() = true

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Verify that the password dialog exists</li>
    kontext "PasswordFileOpen"
    if ( PasswordFileOpen.exists( 3 ) ) then
    
        '///+<li>Enter password, click OK</li>
        PasswortName.setText( cPassword )
        hCloseDialog( PasswordFileOpen, "ok" )
        
        '///+<li>If the password was incorrect:</li>
        '///<ul>
        if ( not bValid ) then
        
            '///+<li>Look for a warning message</li>
            kontext "Active"
            if ( Active.exists( 1 ) ) then
                printlog( CFN & "Message: " & Active.getText() )
                
                '///+<li>Close the messagebox</li>
                hCloseDialog( Active, "ok" )
                
                '///+<li>Close the Password dialog with &quot;Cancel&quot;</li>
                kontext "PasswordFileOpen"
                if ( PasswordFileopen.exists( 2 ) ) then
                    hCloseDialog( PasswordFileOpen, "cancel" )
                else
                    qaerrorlog( CFN & "Password dialog is not present" )
                    hSecurityEnterPasswordOnLoad() = false
                endif
            else
                qaerrorlog( CFN & "Warning about incorrect password is missing" )
                hSecurityEnterPasswordOnLoad() = false
            endif
            
        else
            kontext "Active"
            if ( Active.exists( 1 ) ) then
                warnlog( CFN & "Unexpected messagebox: " & Active.getText() )
                hSecurityEnterPasswordOnLoad() = false
            endif
        endif
        '///</ul>
        
    else
        warnlog( CFN & "Password dialog is missing" )
        hSecurityEnterPasswordOnLoad() = false
    endif
    '///</ul>
  
end function

'*******************************************************************************

function hSelectXMLSecTab( cTab as string ) as boolean

    const CFN = "hSelectXMLSecTab::"
    hSelectXMLSecTab() = false

    kontext

    select case ( ucase( cTab ) )
    case "GENERAL"     : active.setpage TabXMLSecGeneral
                         if ( TabXMLSecGeneral.exists() and TabXMLSecGeneral.isVisible() ) then
                             printlog( CFN & "Switched to General page" )
                             hSelectXMLSecTab() = true
                             exit function
                         endif

    case "DETAILS"     : active.setpage TabXMLSecDetails
                         if ( TabXMLSecDetails.exists() and TabXMLSecDetails.isVisible() ) then
                             printlog( CFN & "Switched to Details page" )
                             hSelectXMLSecTab() = true
                             exit function
                         endif

    case "PATH"        : active.setpage TabXMLSecCertPath
                         if ( TabXMLSecCertPath.exists() and TabXMLSecCertPath.isVisible() ) then
                             printlog( CFN & "Switched to Certification Path page" )
                             hSelectXMLSecTab() = true
                             exit function
                         endif
    end select

    warnlog( CFN & "Failed to switch XML Security Tabpage" )

end function

