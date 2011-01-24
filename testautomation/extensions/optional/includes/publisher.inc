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
'*  short description : Extension publisher and release notes
'*
'\******************************************************************************

testcase tExtensionPublisher

    printlog( "Extension Manager and Release Notes" )

    if ( gISOLang <> "en-US" ) then
        qaerrorlog( "Test does not handle translated strings, aborting" )
        goto endsub
    endif

    ' Specification document
    ' http://specs.openoffice.org/appwide/packagemanager/online_update_for_extensions.odt
    ' http://specs.openoffice.org/appwide/packagemanager/online_update_for_extensions.odt

    ' This is the path where the test-extensions and the info file are located
    dim cBasePath as string
        cBasePath = gTesttoolPath & "extensions/optional/input/publisher/"
        cBasePath = convertpath( cBasePath )

    ' Modify this file to add test cases or attributes
    dim cInfoFile as string : cinfoFile = cBasePath & "extension.info"

    ' The number of files to test. Information about this is stored in the file
    ' specified by cInfoFile
    dim iFileCount as integer
    dim iCurrentFile as integer
    dim iExtensionCount as integer
    dim iExitCounter as integer
    dim iCurrentExtensionCount as integer : iCurrentExtensionCount = 0
    dim iTry as integer

    ' This is the data we get from the configuration file
    dim cCurrentFileName as string
    dim cHasReleaseNotes as string
    dim cReleaseNotes as string
    dim cHasPublisher as string
    dim cPublisher as string

    ' This is where we store the configuration data. This array must have more
    ' lines than cInfoFile contains.
    dim cConfigArray( 60 ) as string

    dim cTestString as string
    dim bInstallationComplete as boolean : bInstallationComplete = false

    iExtensionCount = hGetExtensionCount()


    hGetDataFileSection( cInfoFile, cConfigArray(), "info", "", "" )
    iFileCount = int( hGetValueForKeyAsString( cConfigArray() , "filecount" ) )
    printlog( cInfoFile & " specifies " & iFileCount & " documents to load" )

    iExtensionCount = iExtensionCount + iFileCount
    printlog( "Expected number of extensions after installation: " & iExtensionCount )

    printlog( "Installing extensions" )
    for iCurrentFile = 1 to iFileCount

        hGetDataFileSection( cInfoFile, cConfigArray(), iCurrentFile, "", "" )
        cCurrentFileName = hGetValueForKeyAsString( cConfigArray() , "file" )
        cCurrentFileName = cBasePath & cCurrentFileName
        sExtensionCLI( "add" , cCurrentFileName )
        wait 1000 ' This is required, sExtensionCLI is asynchronous.

    next iCurrentFile
    printlog( "Finished installing extensions, now verifying..." )

    hNewDocument()
    ToolsPackageManager

    kontext "PackageManager"
    if ( PackageManager.exists( 3 ) ) then

        for iTry = 1 to 30

            iCurrentExtensionCount = BrowsePackages.getItemCount()

            if ( iCurrentExtensionCount = iExtensionCount ) then
                printlog( "All extensions have been installed, good." )
                bInstallationComplete = true
                exit for
            endif

            printlog( "Waiting for listbox: Number of extensions: " & iCurrentExtensionCount )
            wait( 1000 ) ' Required, the list takes considerable time to fill.

        next iTry

        ' If one or more extensions have not been added to the list we better abort the
        ' test as the remaining parts need all extensions to be installed causing
        ' lots of cryptig errors to appear in the log.
        if ( not bInstallationComplete ) then
            warnlog( "Not all extensions have been installed. Aborting test." )
            kontext "PackageManager"
            if ( PackageManager.exists() ) then
                PackageManager.cancel()
                PackageManager.notExists( 2 )
            endif
            goto endsub
        endif
    else
        warnlog( "The Extension Manager did not open, Aborting test" )
        goto endsub
    endif


    kontext "PackageManager"
    printlog( "Going for the update-dialog" )
    if ( Updates.isEnabled() ) then

        printlog( "Updates-button is enabled" )
        Updates.click()

        kontext "ExtensionUpdate"
        if ( ExtensionUpdate.exists( 2 ) ) then

            iExitCounter = 0
            do while ( AvailableUpdatesList.getItemCount() < iFileCount )
                printlog( "Waiting for Updates List to populate..." )
                wait( 1000 )
                iExitCounter = iExitCounter + 1
                if ( iExitCounter = 200 ) then
                    warnlog( "Update list for extensions is stalled (expected: 7), " & _
                             "found: " & AvailableUpdatesList.getItemCount() )
                    kontext "ExtensionUpdate"
                    ExtensionUpdate.close()
                    ExtensionUpdate.notExists( 2 )
                    kontext "PackageManager"
                    PackageManager.cancel()
                    PackageManager.notExists( 2 )
                    goto endsub
                endif
            loop

            if ( ShowAllUpdates.exists() ) then
                if ( ShowAllUpdates.isEnabled() ) then
                    warnlog( """Show all updates"" should be disabled" )
                else
                    printlog( """Show all updates"" is disabled, good" )
                endif
            else
                qaerrorlog( """Show all updates"" checkbox could not be accessed" )
            endif

            if ( AvailableUpdatesList.isEnabled() ) then

                printlog( "Available updates list populated in " & iExitCounter & " seconds" )

                if ( iExitCounter < 20 ) then

                    for iCurrentFile = 1 to iFileCount

                        hGetDataFileSection( cInfoFile, cConfigArray(), iCurrentFile, "", "" )
                        cCurrentFileName = hGetValueForKeyAsString( cConfigArray() , "display_name" )

                        cHasReleaseNotes = hGetValueForKeyAsString( cConfigArray() , "has_release_notes" )
                        cReleaseNotes    = hGetValueForKeyAsString( cConfigArray() , "release_notes" )

                        cHasPublisher = hGetValueForKeyAsString( cConfigArray() , "has_publisher" )
                        cPublisher    = hGetValueForKeyAsString( cConfigArray() , "publisher" )

                        AvailableUpdatesList.select( iCurrentFile )

                        if ( AvailableUpdatesList.getText() = cCurrentFileName ) then
                            printlog( "Testing extension: " & cCurrentFileName )

                            if ( lcase( cHasReleaseNotes ) = "yes" ) then
                                if ( ReleaseNotesText.isVisible() ) then
                                    printlog( " - Release notes text is visible. Good." )
                                    qaerrorlog( "#i84700# cannot read static text on extension update dialog" )
                                    'cTestString = ReleaseNotesText.getSelText()
                                    'if ( cTestString = cReleaseNotes ) then
                                    '    printlog( " - Release notes text is ok." )
                                    'else
                                    '    warnlog( "Incorrect release notes text" )
                                    '    printlog( "Expected: " & cReleaseNotes )
                                    '    printlog( "Found...: " & ctestString   )
                                    'endif
                                else
                                    warnlog( "Release notes text is not visible" )
                                endif
                            else
                                if ( ReleaseNotesText.isVisible() ) then
                                    warnlog( "Release notes should not be visible" )
                                else
                                    printlog( " - No release notes for this extension." )
                                endif
                            endif

                            if ( lcase( cHasPublisher ) = "yes" ) then
                                if ( UpdatePublisherText.isVisible() ) then
                                    printlog( " - Publisher text is visible. Good." )
                                    qaerrorlog( "#i84700# cannot read static text" )
                                    'cTestString = UpdatePublisherText.getText()
                                    'if ( cTestString = cPublisher ) then
                                    '    printlog( " - Release notes text is ok." )
                                    'else
                                    '    warnlog( "Incorrect release notes text" )
                                    '    printlog( "Expected: " & cPublisher )
                                    '    printlog( "Found...: " & ctestString   )
                                    'endif
                                else
                                    warnlog( "Publisher text is not visible." )
                                endif
                            else
                                if ( UpdatePublisherText.isVisible() ) then
                                    warnlog( "Publisher should not be visible." )
                                else
                                    printlog( " - No Publisher for this extension." )
                                endif
                            endif

                        else
                            printlog( "The extension is not at the expected position" )
                            printlog( "Expected: " & cCurrentFileName )
                            printlog( "Found...: " & AvailableUpdatesList.getText() )
                            exit for
                        endif

                    next iCurrentFile
                else
                    warnlog( "Updates list not fully populated in 15 seconds -> fail" )
                endif
            else
                warnlog( "No extensions available for update. Installation failed?" )
            endif

            kontext "ExtensionUpdate"
            ExtensionUpdate.close()
            ExtensionUpdate.notExists( 2 )
        else
            warnlog( "Extension Update dialog is not open" )
        endif

    else
        warnlog( "Update-button on extension manager ui is disabled" )
    endif

    kontext "PackageManager"
    PackageManager.close()
    PackageManager.notExists( 2 )

    printlog( "Removing extensions" )
    for iCurrentFile = 1 to iFileCount
        hGetDataFileSection( cInfoFile, cConfigArray(), iCurrentFile, "", "" )
        cCurrentFileName = hGetValueForKeyAsString( cConfigArray() , "file" )
        sExtensionCLI( "remove" , cCurrentFileName )
    next iCurrentFile
    printlog( "Finished removing extensions" )

    hFileCloseAll()

endcase

