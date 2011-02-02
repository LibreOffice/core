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
    const FILTER_ITEMS = 10    ' This is the number of filters/extension types we know

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
                '0: All files (*.*)
				'1: Configuration Data
				'2: Configuration Schema
				'3: Extension
				'4: UNO Components
				'5: UNO Dynamic Library Component
				'6: UNO Java Component
				'7: UNO Java Type Library
				'8: UNO Python Component
				'9: UNO RDB Type Library
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
            ExtensionUpdate.close()
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
        
        kontext "PackageManager"
        BrowsePackages.select( 1 )

        printlog( "Check visibility of some controls, many are hidden for bundled extensions" )
        if ( disable.exists() ) then
            if ( disable.isVisible() ) then
                warnlog( "Button <Disable> should not be visible for shared extensions" )
            else
                printlog( "Button <Disable> is hidden" )
            endif
        else
            printlog( "The control <Disable> is not defined in this context" )
        endif

        if ( remove.exists() ) then
            if ( remove.isVisible() ) then
                warnlog( "Button <Remove> should not be visible for shared extensions" )
            else
                printlog( "Button <Remove> is hidden" )
            endif
        else
            printlog( "The control <Remove> is not defined in this context" )
        endif

        kontext "PackageManager"
        PackageManager.close()
        
    else
        warnlog( "Extension Manager UI did not open" )
    endif 
        
endcase


