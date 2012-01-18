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

