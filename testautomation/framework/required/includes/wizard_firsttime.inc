'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: wizard_firsttime.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:04 $
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
'*  short description : Updatetest for the first time wizard
'*
'\******************************************************************************

testcase tUpdtWizardFirsttime

    '///<h1>Updatetest for the First-Time / Migration-Wizard</h1>
    
    dim iClick as integer
    dim cFirstName as string
    dim cLastName as string
    dim cInitials as string
    
    dim bMigrate as boolean ' Do we migrate?
    
    cFirstName = "Tom"
    cLastName = "Cat"
    cInitials = "TC"
    
    dim cQuickstarterPath as string
        cQuickStarterPath = gNetzOfficePath & "program\quickstart.exe"
    
    ' look for .sversionrc/sversion.ini. If it exists, we assume migration
    printlog( "Do we migrate from an older version?" )
    bMigrate = hTestForOldOffice()
    
    '///<ul>
    
    '///+<li>Make sure exactly one single writer document is open</li>
    hInitSingleDoc()
    
    '///+<li>Open the wizard</li>
    printlog( "Open the Wizard" )
    FirstTimeWizard
    
    ' give the wizard a maximum of 2 seconds to open, it's a serve-url and
    ' should open almost instantly
    kontext "WelcomeDialog"
    if ( WelcomeDialog.exists( 2 ) ) then
        printlog( "The Wizard is open" )
        call dialogtest( WelcomeDialog )
        
        '///+<li>Click the 'next' button to get to the second page</li>
        printlog( "click 'next'")
        NextBtn.click()
    endif
    
    

    
    
    '///+<li>Check the license page</li>
    printlog( "Check the license page" )
    kontext "TabFirstStartLicense"
    if ( TabFirstStartLicense.exists( 2 ) ) then
        printlog( "License page is visible" )
        call dialogtest( TabFirstStartLicense )

    
        '///+<li>Enable the next-button by 'reading' the licensetext</li>
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
    
    
    
    '///+<li>Data Migration Page</li>
    printlog( "Handle Data-Migration page" )
    kontext "TabPersonalDataMigration"
    if ( TabPersonalDataMigration.exists( 2 ) ) then
        call dialogtest( TabPersonalDataMigration )
        printlog( "The data migration page is visible" )
        if ( hTestForOldOffice() ) then
            printlog( "Assuming migration data exists" )
            printlog( "Unchecking migration" )
            TransferPersonalData.uncheck()
        else
            ' at this point i have no clue about the status of the dialog
            qaerrorlog( "There doesn't appear to be anything to migrate" )
        endif
    
        '///+<li>Click the 'next' button to get to the next page</li>    
        kontext "WelcomeDialog"
        NextBtn.click()
    endif
    
    
    
    '///+<li>Page to enter personal data</li>
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
    
    '///+<li>Handle the welcome Dialog - Internet Update page which should NOT appear</li>
    Kontext "TabFirstStartRegistration"
    if ( Not TabFirstStartRegistration.exists( 3 ) ) then
	    warnlog( "An additional page appears on rerun of wizard" )
        kontext "WelcomeDialog"
        NextBtn.click()
    endif
 
    '///+<li>Last page: Registration. Choose not to register</li>
    printlog( "Registration page: " )
    Kontext "TabFirstStartRegistration"
    if ( TabFirstStartRegistration.exists( 2 ) ) then
        printlog( "Page is present." )
        call dialogtest( TabFirstStartRegistration )
        printlog( "Choose not to register" )
        DoNotWantRegister.check()
        
        '///+<li>Finally close the wizard</li>
        printlog( "close the wizard" )
    	kontext "WelcomeDialog"
     	WelcomeDialog.ok()
    endif
    
    '///+<li>Verify that the data made it into tools/options instantly</li>
    '///<ol>
    printlog( "Verify that the userdata made it to tools/options" )
    ToolsOptions
    hToolsOptions( "StarOffice" , "UserData" )

    '///+<li>First name</li>    
    if ( Vorname.getText() <> cFirstname ) then
        warnlog( "First name is not transferred to Tools/options/userdata" )
    endif
   
    '///+<li>Last name</li>
    if ( ZuName.getText() <> cLastName ) then
        warnlog( "Last name is not transferred to Tools/options/userdata" )
    endif
    
    '///+<li>Initials</li>
    if ( Kuerzel.getText() <> cInitials ) then
        warnlog( "Initials are not transferred to Tools/options/userdata" )
    endif
    
    '///+<li>Go to the OpenOffice.org / Memory page</li>
    hToolsOptions( "StarOffice", "Memory" )
    
    if ( LoadQuickstarter.exists() ) then
        if ( LoadQuickstarter.isEnabled() ) then
            printlog( "Quickstarter checkbox is enabled" )
            if ( LoadQuickstarter.isChecked() ) then
                printlog( "Quickstart is activated, turning it off." )
                LoadQuickstarter.unCheck()
                printlog( "Closing Tools/Options" )
                Kontext "OptionenDlg"
                OptionenDlg.cancel()
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
    '///</ol>
        
    Kontext "OptionenDlg"
    if ( OptionenDlg.exists() ) then
        OptionenDlg.cancel()
    endif
   
    '///</ul>
endcase

'*******************************************************************************

function hTestForOldOffice() as boolean

    '///<h3>Check if old office exists (look for .sversionrc)</h3>
    '///<i>BEWARE: Environment variable $HOME is evaluated</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE: .sversionrc/sversion.ini exists</li>
    '///+<li>FALSE: No trace of old office version</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    ' this function looks for the presence of the .sversionrc (unix)
    ' or sversion.ini (windows). If the file exists, we assume that
    ' there is data to be migrated and the function returns true
    ' if $HOME is not set, the function will fail!

    dim sSVersion as string ' name of the .sversionrc/sversion.ini
    dim sIniPath as string

    '///+<li>Determine the platform dependent name of .sversionrc/sversion.ini</li>
    if ( gPlatGroup = "unx" ) then
        sSVersion = ".sversionrc"
    else
        sSVersion = "sversion.ini"
    endif
        
    '///+<li>Retrieve the location of the file from the environment ($HOME)</li>
    sIniPath = environ( "HOME" )
    sIniPath = convertpath( sIniPath & "/" & sSVersion )
    
    '///+<li>Test if the file is present</li>
    if ( dir( sIniPath ) <> "" ) then
        printlog( "  Yes: .sversionrc/soffice.ini at: " & sIniPath )
        hTestForOldOffice() = true
    else
        printlog( "  No: .sversionrc/soffice.ini not present." )
        hTestForOldOffice() = false
    endif
    '///</ul>
    
end function

