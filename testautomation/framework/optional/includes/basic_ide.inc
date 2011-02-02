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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : Macro Dialogs opened from Basic-IDE
'*
'\******************************************************************************

testcase tBasic_IDE_Toolbar_Module

    printlog( "BASIC toolbars for modules" )

    const DIALOG_DELAY = 2
    const RC_FAILURE = -1

    dim cTargetFile as string
        cTargetFile = convertpath( gOfficepath & "user\work\global_test.bas" )

    dim cSourceFile as string
        cSourceFile = gTesttoolpath & "framework\optional\input\resetregistration.txt"
        cSourceFile = convertpath( cSourceFile )

    printlog "open a new writer-doc"
    gApplication = "WRITER"
    hCreateDocument()

    if ( hInitBasicIDE( "tBasic_IDE_Toolbar_Module" ) ) then

        kontext "macrobar"
        printlog "- compile"
        hClickButton( Compile )

        printlog "- Run Basic"
        hClickButton( BasicRun )

        printlog "- Step Procedure"
        hClickButton( ProcedureStep )

        printlog "- Stop Basic"
        hClickButton( BasicStop )

        printlog "- Single Step"
        hClickButton( SingleStep )

        printlog "- Step Back"
        hClickButton( StepBack )

        printlog "- Breakpoint => activate"
        hClickButton( Breakpoint )


        printlog "- Breakpoint => deactivate"
        hClickButton( Breakpoint )

        printlog "- Add Watch => deactivate"
        hClickButton( AddWatch )

        printlog "- Find Paranthese"

        kontext "macrobar"
        hClickButton( FindParanthese )

        try
            Controls.TearOff()
            warnlog "Controls are active in a module window => bug!"
        catch
            printlog( "Controls are not active->OK" )
        endcatch
        if ( WaitSlot <> WSFinished ) then
            warnlog( "Slot not finished within 1 second" )
        endif

        kontext "MacroBar"
        printlog( "- Insert Source Text (" & cSourceFile & ")" )
        if ( hClickButton( InsertSourceText ) <> RC_FAILURE ) then

            kontext "oeffnendlg"
            if ( OeffnenDlg.exists( DIALOG_DELAY ) ) then
                Dateiname.SetText( cSourceFile )
                Oeffnen.click()
                WaitSlot( 5000 )
            else
                warnlog( "File Open dialog did not open" )
            endif

            ' possible 'could not read from file' error-message -> this is a bug
            kontext "Active"
            if ( Active.exists( DIALOG_DELAY ) ) then
                warnlog( "Unexpected active: " & active.gettext() )
                active.ok()
            endif
        else
            warnlog( "<File Open> dialog is not open, <Insert Source Text> button failed" )
        endif

        kontext "macrobar"
        printlog "- Save Source Text "
        if ( hClickButton( SaveSourceText ) <> RC_FAILURE ) then

            kontext "speicherndlg"
            if ( SpeichernDlg.exists( DIALOG_DELAY ) ) then
                Dateiname.SetText( cTargetFile )
                Speichern.click()
            else
                warnlog( "File Save dialog is missing" )
            endif

            kontext "active"
            if ( Active.Exists( DIALOG_DELAY ) ) then
                printlog( "Closing Messagebox with <Yes>")
                Active.Yes()
            endif
        else
            warnlog( "<File Save> dialog did not open, slot <SaveSourceText> did not work" )
        endif

        hFileCloseAll()
    else
        warnlog( "Unable to init BASIC IDE" )
    endif

endcase

'*******************************************************************************

testcase tBasic_IDE_Toolbar_Dialogs

    printlog( "BASIC toolbar for Dialogs" )

    const DIALOG_DELAY = 2

    const TABBAR_MENU_NEW = 1
    const TABBAR_MENU_NEW_DIALOG = 2

    dim bIsOpen as boolean
    dim brc as boolean
    
    dim sBasfile1 as string
        sBasFile1 = gTesttoolpath & "framework\optional\input\resetregistration.txt"
        sBasFile1 = convertpath( sBasFile1 )
        
    dim sBasFile2 as string
        sBasFile2 = convertpath( gOfficepath & "user\work\global_test.bas" )


    printlog "open a new writer-doc"
    gApplication = "WRITER"
    hCreateDocument()

    if ( hInitBasicIDE( "tBasic_IDE_Toolbar_Dialogs" ) ) then

        kontext "basicide"
        Tabbar.OpenContextMenu
        hMenuSelectNr ( TABBAR_MENU_NEW        )
        hMenuSelectNr ( TABBAR_MENU_NEW_DIALOG )

        if ( DialogWindow.Exists ( DIALOG_DELAY ) ) then
            printlog( "Dialog Editor window is open. Good." )
        else
            warnlog "No dialog window is shown!"
        end if

        try
            kontext "MacroBar"
            Compile.Click()
            warnlog( """Compile"" is enabled" )
        catch
            printlog( """Compile"" is disabled" )
        endcatch

        try
            kontext "MacroBar"
            BasicRun.Click()
            warnlog( """Run Basic"" is enabled" )
        catch
            printlog( """Run Basic"" is disabled" )
        endcatch

        printlog "- Step Procedure (disabled)"
        try
            kontext "MacroBar"
            ProcedureStep.Click()
            warnlog( """Step Procedure"" is enabled" )
        catch
            printlog( """Step Procedure"" is disabled" )
        endcatch

        try
            kontext "MacroBar"
            BasicStop.Click()
            warnlog( """Stop Basic"" is enabled" )
        catch
            printlog( """Stop Basic"" is disabled" )
        endcatch

        try
            kontext "MacroBar"
            SingleStep.Click()
            warnlog( """Single Step"" is enabled" )
        catch
            printlog( """Single Step"" is enabled" )
        endcatch

        try
            kontext "MacroBar"
            StepBack.Click()
            warnlog( """Single Step Back"" is enabled" )
        catch
            printlog( """Single Step Back"" is enabled" )
        endcatch

        try
            kontext "MacroBar"
            Breakpoint.Click()
            warnlog( """Breakpoint"" is enabled" )
        catch
            printlog( """Breakpoint"" is disabled" )
        endcatch


        try
            kontext "MacroBar"
            AddWatch.Click()
            warnlog( """Add Watch"" is enabled" )
        catch
            printlog( """Add Watch"" is disabled" )
        endcatch


        try
            kontext "MacroBar"
            FindParanthese.Click()
            warnlog( """Find Paranthese"" is enabled" )
        catch
            printlog( """Find Paranthese"" is disabled" )
        endcatch


        try
            kontext "macrobar"
            InsertSourceText.Click()
            warnlog( """Insert Source Text"" is enabled" )

            kontext "oeffnendlg"
            if ( OeffnenDlg.exists( DIALOG_DELAY ) ) then
                Dateiname.SetText( sBasFile1 )
                Oeffnen.Click()
            else
                warnlog( "File Open dialog missing" )
            endif
        catch
            printlog( """Insert Source Text"" is disabled" )
        endcatch


        printlog "- Save Source Test  (disabled)"

        try
            kontext "macrobar"
            SaveSourceText.Click()
            warnlog( """Save Source Text"" is enabled" )

            kontext "speicherndlg"
            if ( SpeichernDlg.exists( DIALOG_DELAY ) ) then
                Dateiname.SetText( sBasFile2 )
                Speichern.Click()
            else
                warnlog( "File Save dialog is missing" )
            endif

            kontext "active"
            if ( Active.Exists( DIALOG_DELAY ) ) then
                Active.Yes()
            endif
        catch
            printlog( """Save Source Text"" is disabled" )
        endcatch

        hCloseBasicIDE()
        hDestroyDocument()
    else
        warnlog( "Unable to open BASIC IDE" )
    endif

endcase

