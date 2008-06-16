'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: security_dialogs.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:03 $
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
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Security dialogs in Tools/Options
'*
'\******************************************************************************

testcase tUpdtSecurityDialogs

    '///<h1>Security dialogs in Tools/Options</h1>

    '///<u><pre>Synopsis</pre></u>Quick ressource test for the security dialogs
    '///+ below Tools/Options -> OpenOffice.org/Security<br>
    '///<u><pre>Specification document</pre></u><a href="http://specs.openoffice.org/appwide/security/Electronic_Signatures_and_Security.sxw">
    '///+ Electronic signatures and security</a><br>
    '///<u><pre>Files used</pre></u>None<br>
    '///<u><pre>Test case specification</pre></u>
    '///<ul>
    
    const C_PASSWORD = "huhuhu"
    
    '///+<li>Open Tools/Options</li>
    printlog( "Tools/Options" )
    ToolsOptions
    
    '///+<li>Select the OpenOffice.org/Security page</li>
    hToolsOptions( "StarOffice", "Security" )
    
    kontext "TabSecurity"
    call DialogTest( TabSecurity )
    
    '///+<li>Click the &quot;Options...&quot; button<br>
    '///+ The &quot;Options&quot; dialog should open</li>
    Options.click()
    
    kontext "TabSecurityOptionsAndWarnings"
    if ( TabSecurityOptionsAndWarnings.exists( 1 ) ) then
        call DialogTest( TabSecurityOptionsAndWarnings )
        printlog( "Options dialog is present" )
        
        '///+<li>Cancel Options dialog</li>
        TabSecurityOptionsAndWarnings.cancel()
    else
        warnlog( "options dialog is missing" )
    endif
    
    '///+<li>Check &quot;Persistently save passwords...&quot;<br>
    kontext "TabSecurity"
    if ( PersistentlySavePasswords.isChecked() ) then
        qaerrorlog( "Security settings already changed by another test" )
        Kontext "TabSecurity"
        PersistentlySavePasswords.unCheck()
        kontext "active"
        if( active.exists( 1 ) ) then
            active.yes()
        else
            warnlog( "failed to reset password configuration, aborting" )
            kontext "OptionenDlg"
            OptionenDlg.cancel()
        endif
    endif
    
    kontext "TabSecurity"
    PersistentlySavePasswords.check()
    
    '///+ The &quot;Define Master Password&quot; dialog should appear</li>
    kontext "MasterPasswordDefine"
    if ( MasterPasswordDefine.exists( 1 ) ) then
        printlog( "Define Master password dialog is open" )
        call DialogTest( MasterPasswordDefine )

        '///+<li>Set password &quot;huhuhu&quot;</li>
        Password1.setText( C_PASSWORD )
        
        '///+<li>Confirm password</li>
        Password2.setText( C_PASSWORD )        
        
        '///+<li>Click &quot;OK&quot;</li>
        MasterPasswordDefine.ok()
        
    else
        warnlog( "Define Master Password dialog is missing" )
    endif
    
    '///+<li>Click &quot;Master Password...&quot;<br>
    kontext "TabSecurity"    
    MasterPassword.click()
    
    '///+ The &quot;Enter Password&quot; dialog should appear</li>
    kontext "MasterPasswordEnter"
    if ( MasterPasswordEnter.exists( 1 ) ) then
        printlog( "Enter master Password dialog is open" )
        call DialogTest( MasterPasswordEnter )
        
        '///+<li>Click &quot;Cancel&quot;</li>
        MasterPasswordEnter.cancel()
    else
        warnlog( "Enter Password dialog is missing" )
    endif

  
    '///+<li>Click &quot;Show Passwords...&quot;<br>
    kontext "TabSecurity"
    ShowPasswords.click()
    
    '///+ The &quot;Enter Master Password&quot; dialog should open</li>
    kontext "MasterPasswordEnter"
    if ( MasterPasswordEnter.exists( 1 ) ) then
        printlog( "Enter password dialog is open" )
        
        '///+<li>Enter &quot;huhuhu&quot;</li>
        Password.setText( C_PASSWORD )
        
        '///+<li>Click &quot;OK&quot;<br>
        MasterPasswordEnter.ok()
        
        '///+ The &quot;Stored Web Connection Information&quot; dialog should open</li>
        kontext "StoredPasswordsDialog"
        if ( StoredPasswordsDialog.exists( 1 ) ) then
            printlog( "Stored passwords dialog is open" )
            call DialogTest( StoredPasswordsDialog )
            
            '///+<li>Close the &quot;Stored Web Connection Information&quot; dialog</li>
            StoredPasswordsDialog.close()
        else
            warnlog( "Stored passwords dialog is missing" )
        endif
    else
        warnlog( "Enter Master Password dialog is missing" )
    endif
        
    '///+<li>Click &quot;Macro Security...&quot;<br>
    kontext "TabSecurity"
    MacroSecurity.click()
    
    '///+ The Macro Security/Security Level page should open</li>
    kontext "TabSecurityLevel"
    if ( TabSecurityLevel.exists( 1 ) and TabSecurityLevel.isVisible() ) then
        printlog( "Security Level tabpage is open." )
        
        '///+<li>Switch to the &quot;Trusted Sources&quot; tabpage</li>
        kontext
        active.setPage TabTrustedSources
        
        kontext "TabTrustedSources"
        if ( TabTrustedSources.isVisible() ) then
            printlog( "Trusted Sources tabpage is visible" )
            
            '///+<li>Click &quot;Add...&quot;</li>
            LocationsAdd.click()
            
            '///+<li>Cancel &quot;Select Path&quot; dialog</li>
            kontext "OeffnenDlg" 
            if ( OeffnenDlg.exists( 1 ) ) then
                printlog( "File open dialog is visible" )
                call DialogTest( OeffnenDlg )
                OeffnenDlg.cancel()
                
                '///+<li>Cancel &quot;Macro Security&quot; dialog</li>
                kontext "TabTrustedSources"
                if ( TabTrustedSources.exists() ) then
                    TabTrustedSources.cancel()
                else
                    warnlog( "Trusted Sources tabpage not available" )
                endif
            else
                warnlog( "Trusted Paths selector not visible" )
            endif
        else
            warnlog( "Trusted Sources tabpage is not available" )
        endif
    else
        warnlog( "The Macro Security settings dialog is not open" )
    endif
    
    '///+<li><i><b>Todo: &quot;Protect...&quot; button</b></i></li>
    
    '///+<li>Uncheck &quot;Persistently save passwords...&quot;</li>
    Kontext "TabSecurity"
    PersistentlySavePasswords.unCheck()
    
    '///+<li>Confirm to delete all stored passwords</li>
    kontext "active"
    if( active.exists( 1 ) ) then
        printlog( "Passwords deletion warning is displayed. Good" )
        call DialogTest( active )
        active.yes()
    else
        warnlog( "Password deletion warning is missing" )
    endif

    
    '///+<li>Cancel &quot;Tools/Options&quot; dialog</li>
    kontext "OptionenDlg"
    OptionenDlg.cancel()    
    
    '///</ul>

endcase

