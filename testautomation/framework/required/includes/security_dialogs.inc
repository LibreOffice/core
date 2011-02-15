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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Security dialogs in Tools/Options
'*
'\******************************************************************************

testcase tUpdtSecurityDialogs

    printlog( "Security dialogs in Tools/Options" )
    
    const C_PASSWORD = "huhuhu"
    
    printlog( "Tools/Options" )
    ToolsOptions
    
    hToolsOptions( "StarOffice", "Security" )
    
    kontext "TabSecurity"
    call DialogTest( TabSecurity )
    
    Options.click()
    
    kontext "TabSecurityOptionsAndWarnings"
    if ( TabSecurityOptionsAndWarnings.exists( 1 ) ) then
        call DialogTest( TabSecurityOptionsAndWarnings )
        printlog( "Options dialog is present" )
        
        TabSecurityOptionsAndWarnings.cancel()
    else
        warnlog( "options dialog is missing" )
    endif
    
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
    
    kontext "MasterPasswordDefine"
    if ( MasterPasswordDefine.exists( 1 ) ) then

        printlog( "Define Master password dialog is open" )
        call DialogTest( MasterPasswordDefine )

        Password1.setText( C_PASSWORD )
        Password2.setText( C_PASSWORD )        
        MasterPasswordDefine.ok()
        
    else
        warnlog( "Define Master Password dialog is missing" )
    endif
    
    kontext "TabSecurity"    
    MasterPassword.click()
    
    kontext "MasterPasswordEnter"
    if ( MasterPasswordEnter.exists( 1 ) ) then
        printlog( "Enter master Password dialog is open" )
        call DialogTest( MasterPasswordEnter )
        
        MasterPasswordEnter.cancel()
    else
        warnlog( "Enter Password dialog is missing" )
    endif

  
    kontext "TabSecurity"
    ShowPasswords.click()
    
    kontext "MasterPasswordEnter"
    if ( MasterPasswordEnter.exists( 1 ) ) then
        printlog( "Enter password dialog is open" )
        
        Password.setText( C_PASSWORD )
        
        MasterPasswordEnter.ok()
        
        kontext "StoredPasswordsDialog"
        if ( StoredPasswordsDialog.exists( 1 ) ) then
            printlog( "Stored passwords dialog is open" )
            call DialogTest( StoredPasswordsDialog )
            
            StoredPasswordsDialog.close()
        else
            warnlog( "Stored passwords dialog is missing" )
        endif
    else
        warnlog( "Enter Master Password dialog is missing" )
    endif
        
    kontext "TabSecurity"
    MacroSecurity.click()
    
    kontext "TabSecurityLevel"
    if ( TabSecurityLevel.exists( 1 ) and TabSecurityLevel.isVisible() ) then
        printlog( "Security Level tabpage is open." )
        
        kontext
        active.setPage TabTrustedSources
        
        kontext "TabTrustedSources"
        if ( TabTrustedSources.isVisible() ) then
            printlog( "Trusted Sources tabpage is visible" )
            
            LocationsAdd.click()
            
            kontext "OeffnenDlg" 
            if ( OeffnenDlg.exists( 1 ) ) then
                printlog( "File open dialog is visible" )
                call DialogTest( OeffnenDlg )
                OeffnenDlg.cancel()
                
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
    
    Kontext "TabSecurity"
    PersistentlySavePasswords.unCheck()
    
    kontext "active"
    if( active.exists( 1 ) ) then
        printlog( "Passwords deletion warning is displayed. Good" )
        call DialogTest( active )
        active.yes()
    else
        warnlog( "Password deletion warning is missing" )
    endif

    
    kontext "OptionenDlg"
    OptionenDlg.cancel()    
    

endcase

