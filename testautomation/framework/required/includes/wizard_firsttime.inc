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
'*  short description : Updatetest for the first time wizard
'*
'\******************************************************************************

testcase tUpdtWizardFirsttime

    printlog( "Resource test for the first start wizard" )
    
    dim iClick as integer
    dim cFirstName as string
    dim cLastName as string
    dim cInitials as string
    
    cFirstName = "Tom"
    cLastName = "Cat"
    cInitials = "TC"
    
    dim cQuickstarterPath as string
        cQuickStarterPath = gNetzOfficePath & "program\quickstart.exe"
    
    
    hInitSingleDoc()
    
    printlog( "Open the Wizard" )
    FirstTimeWizard
    
    ' give the wizard a maximum of 2 seconds to open, it's a serve-url and
    ' should open almost instantly
    kontext "WelcomeDialog"
    if ( WelcomeDialog.exists( 2 ) ) then
        printlog( "The Wizard is open" )
        call dialogtest( WelcomeDialog )
        
        printlog( "click 'next'")
        NextBtn.click()
    endif
    
    printlog( "Check the license page" )
    kontext "TabFirstStartLicense"
    if ( TabFirstStartLicense.exists( 2 ) ) then
        printlog( "License page is visible" )
        call dialogtest( TabFirstStartLicense )

    
        printlog( "scroll down (button!) the licensetext to enable the >>-button" )
        for iClick = 1 to 50
    
            kontext "TabFirstStartLicense"
            if ( ScrollDown.isEnabled( 1 ) ) then
                ScrollDown.click()
            else
                kontext "WelcomeDialog"
                if ( NextBtn.isEnabled() ) then
                    printlog( "Needed " & iClick & " clicks on 'scroll down' " )
                    exit for
                else
                    warnlog( "Both Scroll-button and Next-button are disabled" )
                    goto endsub
                endif
            endif
        next iClick
    
        sleep( 1 )
    
        kontext "WelcomeDialog"
        if ( WelcomeDialog.exists( 2 ) ) then
            NextBtn.click()
        else
            warnlog( "Cannot access welcome dialog" )
        endif
    endif
    
    
    
    printlog( "Handle Data-Migration page" )
    kontext "TabPersonalDataMigration"
    if ( TabPersonalDataMigration.exists( 2 ) ) then
        call dialogtest( TabPersonalDataMigration )
        printlog( "The data migration page is visible" )
        TransferPersonalData.uncheck()
        kontext "WelcomeDialog"
        NextBtn.click()
    endif
    
    
    
    printlog( "The personal data page should show up" )
    Kontext "TabFirstStartUser"
    if ( TabFirstStartUser.exists( 2 ) ) then
    
        call dialogtest( TabFirstStartUser )
        
        Kontext "TabFirstStartUser"
        printlog( "Enter lastname, name and initials" )
        FirstName.setText( cFirstname )
        LastName.setText( cLastName )
        Initials.setText( cInitials )
        
        kontext "WelcomeDialog"
        NextBtn.click()
    else
        warnlog( "Could not access TabFirstStartUser" )
    endif
    
    if (gAutoUpdate) then
        kontext "TabFirstStartOnlineUpdate"
        if ( TabFirstStartOnlineUpdate.exists( 2 ) ) then
            call dialogtest( TabFirstStartOnlineUpdate )
            CheckForUpdates.unCheck()
            kontext "WelcomeDialog"
            NextBtn.click()
        else
            warnlog( "Online Update Page is missing" )
        endif
    else
            qaErrorlog( "Online Update Page is missing" )
    endif
        
    
    Kontext "TabFirstStartRegistration"
    if ( TabFirstStartRegistration.exists( 3 ) ) then
        printlog( "Registration page visible." )
    else
	    qaerrorlog( "An additional page appears on rerun of wizard" )
        kontext "WelcomeDialog"
        NextBtn.click()
    endif
 
    printlog( "Registration page: " )
    Kontext "TabFirstStartRegistration"
    if ( TabFirstStartRegistration.exists( 2 ) ) then
        printlog( "Page is present." )
        call dialogtest( TabFirstStartRegistration )
        printlog( "Choose not to register" )
        DoNotWantRegister.check()
        
        printlog( "close the wizard" )
    	kontext "WelcomeDialog"
     	WelcomeDialog.ok()
    endif
    
    printlog( "Verify that the userdata made it to tools/options" )
    ToolsOptions
    hToolsOptions( "StarOffice" , "UserData" )

    if ( Vorname.getText() <> cFirstname ) then
        warnlog( "First name is not transferred to Tools/options/userdata" )
    endif
   
    if ( ZuName.getText() <> cLastName ) then
        warnlog( "Last name is not transferred to Tools/options/userdata" )
    endif
    
    if ( Kuerzel.getText() <> cInitials ) then
        warnlog( "Initials are not transferred to Tools/options/userdata" )
    endif
    
    hToolsOptions( "StarOffice", "Memory" )
    
    if ( LoadQuickstarter.exists() ) then
        if ( LoadQuickstarter.isEnabled() ) then
            printlog( "Quickstarter checkbox is enabled" )
            if ( LoadQuickstarter.isChecked() ) then
                printlog( "Quickstart is activated, turning it off." )
                LoadQuickstarter.unCheck()
                printlog( "Closing Tools/Options" )
                Kontext "OptionenDlg"
                hCloseDialog( OptionenDlg, "cancel" )
                printlog( "Restarting program" )
                call ExitRestartTheOffice()
            else
                printlog( "Quickstarter is not turned on." )
            endif
        else
            printlog( "Quickstarter checkbox is not enabled for this system" )
        endif
    else
        if ( gPlatGroup <> "unx" ) then
            if ( dir( cQuickStarterPath ) <> "" ) then
                warnlog( "Quickstarter checkbox is missing on options page" )
            else
                printlog( "Quickstarter is not installed/no checkbox present" )
            endif
        endif
    endif
        
    Kontext "OptionenDlg"
    hCloseDialog( OptionenDlg, "cancel, optional" )
    
    ' due to issue i105248 the Quickstart disabler needs to get called again, because the First Start Wizard resets the veto
    call hDisableQuickstarterAPI()

    hFileCloseAll()
   
endcase

