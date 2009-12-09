'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_ide.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:13 $
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
'* owner : joerg.skottke@sun.com
'*
'* short description : Macro Dialogs opened from Basic-IDE
'*
'\******************************************************************************

testcase tBasic_IDE_Toolbar_Module

    qaerrorlog( "Replace this test asap, the approach is unusable" )

    dim brc as boolean
    dim cSourceFile as string
        cSourceFile = gTesttoolpath & "framework\optional\input\resetregistration.txt"
        cSourceFile = convertpath( cSourceFile )




    printlog "open a new writer-doc"
    gApplication = "WRITER"
    hCreateDocument()

    brc = hOpenBasicOrganizerFromDoc()
    brc = hCreateModuleForDoc()


    kontext "macrobar"
    printlog "- compile"
    hClickToolbarButton( Compile )

    printlog "- Run Basic"
    hClickToolbarButton( BasicRun )

    printlog "- Step Procedure"
    hClickToolbarButton( ProcedureStep )

    printlog "- Stop Basic"
    hClickToolbarButton( BasicStop )

    printlog "- Single Step"
    hClickToolbarButton( SingleStep )

    printlog "- Step Back"
    hClickToolbarButton( StepBack )

    printlog "- Breakpoint => activate"
    hClickToolbarButton( Breakpoint )


    printlog "- Breakpoint => deactivate"
    hClickToolbarButton( Breakpoint )

    printlog "- Add Watch => deactivate"
    hClickToolbarButton( AddWatch )

    printlog "- Find Paranthese"

    kontext "macrobar"
    hClickToolbarButton( FindParanthese )

    try
        Controls.TearOff()
        warnlog "Controls are active in a module window => bug!"
    catch
        printlog( "Controls are not active->OK" )
    endcatch
    if ( WaitSlot <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif

    printlog( "- Insert Source Text (" & cSourceFile & ")" )


    InsertSourceText.Click()
    kontext "oeffnendlg"
    if ( OeffnenDlg.exists( 1 ) ) then
        Dateiname.SetText( cSourceFile )
        Oeffnen.click()
    else
        warnlog( "File Open dialog did not open" )
    endif        
    
    ' possible 'could not read from file' error-message -> this is a bug
    kontext "Active"
    if ( Active.exists( 1 ) ) then
        warnlog( "Unexpected active: " & active.gettext() )
        active.ok()
    endif

    kontext "macrobar"
    printlog "- Save Source Test "

    hClickToolbarButton( SaveSourceTest )

    kontext "speicherndlg"
    if ( SpeichernDlg.exists( 1 ) ) then
        Dateiname.SetText ConvertPath (gOfficepath + "user\work\global_test.bas")
        Speichern.click()
    else
        warnlog( "File Save dialog is missing" )
    endif

    kontext "active"
    if ( Active.Exists( 1 ) ) then
        printlog( "Closing Messagebox with <Yes>")
        Active.Yes()
    endif

    hDestroyDocument()   ' for Basic-IDE
    hDestroyDocument()   ' the new writer-doc

endcase

'*******************************************************************************

testcase tBasic_IDE_Toolbar_Dialogs

    qaerrorlog( "Replace this test asap, the approach is unusable" )

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

    printlog "create a new module "
    brc = hOpenBasicOrganizerFromDoc()
    brc = hCreateModuleForDoc()

    kontext "basicide"
    Tabbar.OpenContextMenu
    hMenuSelectNr ( 1 )
    hMenuSelectNr ( 2 )
    if ( DialogWindow.Exists ( 2 ) ) then
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
        if ( OeffnenDlg.exists( 2 ) ) then
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
        SaveSourceTest.Click()
        warnlog( """Save Source Text"" is enabled" )

        kontext "speicherndlg"
        if ( SpeichernDlg.exists( 2 ) ) then
            Dateiname.SetText( sBasFile2 )
            Speichern.Click()
        else
            warnlog( "File Save dialog is missing" )
        endif

        kontext "active"
        if ( Active.Exists( 1 ) ) then
            Active.Yes()
        endif
    catch
        printlog( """Save Source Text"" is disabled" )
    endcatch

    
    bIsOpen = hShowMacroControls()
    if ( bIsOpen ) then
        ToolsCollectionBar.Close()
    end if

    Call hDestroyDocument()   ' for Basic-IDE
    Call hDestroyDocument()   ' the new writer-doc

endcase

'******************************************************************************'

function hClickToolbarButton( oButton as object ) as boolean

    const CFN = "hClickToolbarButton(): "

    dim iTry as integer
    dim brc as boolean : brc = false
    
    printlog( CFN & "Clicking button on toolbar: " & oButton.name() )
    for iTry = 1 to 5
    
        try
            oButton.click()
            brc = true
            exit for
        catch
            printlog( CFN & "Retrying..." )
        endcatch
        
        if ( iTry = 5 ) then
            exit for
        endif
        
        wait( 500 )
        
    next iTry
    
    if ( WaitSlot( 2000 ) <> WSFinished ) then
        warnlog( CFN & "Slot not finished within 1 second" )
    endif

end function
