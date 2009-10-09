'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: e_update.inc,v $
'*
'* $Revision: 1.1.2.3 $
'*
'* last change: $Author: jsk $ $Date: 2008/10/08 09:05:51 $
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
'*  short description : Extension Update Test
'*
'\******************************************************************************

testcase tExtensionManagerResources()

    ' Test is based on issue #i86913 and the common rules for update tests (QA)
    
    ' We need to know if the user may change extension parameters. This is the case
    ' when a) the user is root/Administrator or b) the office was installed
    ' for the current user only
    
    dim bUserIsAdmin as boolean  : bUserIsAdmin = hCheckForAdministratorPermissions()
    dim iBreakCounter as integer : iBreakCounter = 0
    dim iFilterItems as integer 
    
    const MAX_ITERATIONS = 50
    const FILTER_ITEMS = 9    ' This is the number of filters/extension types we know

    printlog( "Ressource test for the Extension Manager UI" )
    ToolsPackageManager
    
    kontext "PackageManager"
    if ( PackageManager.exists( 1 ) ) then
    
        ' OOo comes with a few preinstalled, shared - extensions. We need at least one.
        ' This test is bound to fail if we have a user-installed extension at the
        ' first position of the Extension Manager UI list. There is no way we can 
        ' check that for now. However, this is hardly ever the case.
    
        if ( BrowsePackages.getItemCount() = 0 ) then
            warnlog( "This test requires at least one extension to be installed." )
            printlog( "Some of the controls are not available or disabled." )
            goto endsub
        endif
    
        call dialogtest( PackageManager )

        kontext "PackgeManager"
        printlog( "Add..." )                
        Add.click()

        kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( 1 ) ) then
            iFilterItems = DateiTyp.getItemCount
            if ( iFilterItems <> FILTER_ITEMS ) then
                warnlog( "The file type filter list is incorrect" )
                printlog( "Expected: " & FILTER_ITEMS )
                printlog( "Found...: " & iFilterItems )
            else
                printlog( "The number of filters in Add Extensions Dialog is ok" )
            endif
            Oeffnendlg.cancel()
        else
            warnlog( "File Open dialog did not open" )
        endif
                
        kontext "PackageManager"
        printlog( "Check for updates..." )
        updates.click()
        
        kontext "ExtensionUpdate"
        if ( ExtensionUpdate.exists( 2 ) ) then
            call dialogtest( ExtensionUpdate )
            ExtensionUpdate.cancel()
            ExtensionUpdate.notExists( 3 )
        else
            warnlog( "Extenson Update dialog did not open" )      
        endif
        
        kontext "PackageManager"
        printlog( "Controls belonging to extensions" )
        
        BrowsePackages.select( 1 )
        printlog( "Extension name.......: " & BrowsePackages.getItemText( 1 , 1 ) )
        printlog( "Extension version....: " & BrowsePackages.getItemText( 1 , 2 ) )
        printlog( "Extension description: " & BrowsePackages.getItemText( 1 , 3 ) )
        
        if ( options.exists() ) then
            if ( options.isEnabled() ) then
                printlog( "Extension has options" )
                options.click()
                
                kontext "OptionenDlg"
                OptionenDLg.close()
                WaitSlot()
            else
                warnlog( "Extension Options button visible but disabled" )
            endif
        else
            printlog( "Extension Options button does not exist, no options available" )
        endif
        
        if ( bUserIsAdmin ) then
        
            ' If the user is Admin or the office was installed in the users
            ' home directory we have access to the extensions that reside in 
            ' the shared layer. This means that the user can remove and disable
            ' these extensions.
        
            kontext "PackageManager"
            BrowsePackages.select( 1 )
            if ( disable.exists() ) then
                if ( disable.isEnabled() ) then
                
                    disable.click()

                    kontext "Active"
                    if ( Active.exists( 1 ) ) then
                        printlog( "Disable extension: Warning for other office instance. Good." )
                        active.ok()
                    else
                        warnlog( "No warning for other office instances running" )
                    endif
                    
                    ' We now re-enable the extensions. Depending on the "size"
                    ' of the extension it might take the Extension Manager UI
                    ' a few seconds to complete the "disable" action. We need to
                    ' wait for that.
                    kontext "PackageManager"
                    BrowsePackages.select( 1 )
                    iBreakCounter = 0
                    do while( not enable.exists() ) 
                        iBreakCounter = iBreakCounter + 1
                        wait( 100 )
                        if ( iBreakCounter = MAX_ITERATIONS ) then
                            warnlog( "Extension not ready within timeframe. Aborting" )
                            goto endsub
                        endif
                    loop
                    enable.click()
                    
                    kontext "Active"
                    if ( Active.exists( 1 ) ) then
                        printlog( "Enable extension: Warning for other office instance. Good." )
                        active.ok()
                    else
                        warnlog( "No warning for other office instances running" )
                    endif

                    ' The Extension Manager UI remembers when it has warned about 
                    ' other running instances of the office, so the next time we try
                    ' to disable/enable the extension there should be no warning.
                    
                    kontext "PackageManager"
                    iBreakCounter = 0
                    do while( not disable.exists() ) 
                        iBreakCounter = iBreakCounter + 1
                        wait( 100 )
                        if ( iBreakCounter = MAX_ITERATIONS ) then
                            warnlog( "Extension not ready within timeframe. Aborting" )
                            goto endsub
                        endif
                    loop
                    disable.click()

                    kontext "Active"
                    if ( Active.exists( 1 ) ) then
                        warnlog( "There should be no warning when disabling the extension a second time" )
                        active.ok()
                    else
                        printlog( "Disable extension: No warning for other office instances. Good." )
                    endif
                    
                    kontext "PackageManager"
                    iBreakCounter = 0
                    do while( not enable.exists() ) 
                        iBreakCounter = iBreakCounter + 1
                        wait( 100 )
                        if ( iBreakCounter = MAX_ITERATIONS ) then
                            warnlog( "Extension not ready within timeframe. Aborting" )
                            goto endsub
                        endif
                    loop
                    enable.click()
                    
                    kontext "Active"
                    if ( Active.exists( 1 ) ) then
                        warnlog( "There should be no warning when enabling the extension a second time" )
                        active.ok()
                    else
                        printlog( "Enable extension: No warning for other office instances. Good." )
                    endif
                    WaitSlot()
                    
                else
                    warnlog( "User should be able to disable shared extension" )
                endif
            else
                warnlog( "Cannot disable extension. It might be disabled already or defunct" )
            endif
            
            kontext "PackageManager"
            iBreakCounter = 0
            do while( not remove.isEnabled() ) 
                iBreakCounter = iBreakCounter + 1
                wait( 100 )
                if ( iBreakCounter = MAX_ITERATIONS ) then
                    warnlog( "Extension not ready within timeframe. Aborting" )
                    goto endsub
                endif
            loop
    
            kontext "PackageManager"
            if ( remove.isEnabled() ) then
            
                remove.click()
                
                ' Note: First time we try to remove an extension we should get a 
                ' warning that we must make sure that no other office instances 
                ' are running. If this warning is missing we actually remove the 
                ' extension which leaves the installation in an inconsistent
                ' state. In a "real life" installation this would not work, in a
                ' "user space" installation we have to reinstall. Bad that is.
                
                kontext "Active"
                if ( Active.exists( 1 ) ) then
                    printlog( "Remove extension: Warning for other office instances. Good." )
                    active.cancel()
                else
                    warnlog( "No warning for other office instances running" )
                    warnlog( "Installation inconsistency: Extension was removed!" )
                endif
                WaitSlot()
                
                ' Note: Like for the disable/enable buttons we should not get 
                ' a warning for another running office instance when removing
                ' an extension for the second time. 
            
            else
                warnlog( "User should be able to remove extension" )
            endif
        else
       
            ' if the user does not have Administrator rights he may neither 
            ' he may neither disable or remove extensions.
             
            kontext "PackageManager"
            if ( disable.isEnabled() ) then
                warnlog( "Disable-button enabled for shared extension" )
            endif
            
            kontext "PackageManager"
            if ( remove.isEnabled() ) then
                warnlog( "Remove-button enabled for shared extension" )
            endif
        endif
        
        kontext "PackageManager"
        PackageManager.close()
        
    else
        warnlog( "Extension Manager UI did not open" )
    endif 
        
endcase


