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


    '///<h1>Work with Toolbar on Basic-IDE part 1</h1>

    '///<ul>

    '///+<li>Open a new writer-doc</li>
    printlog "open a new writer-doc"
    gApplication = "WRITER"
    hCreateDocument()

    '///+<li>Create a new module for the new document (named TTModule)</li>
    brc = hOpenBasicOrganizerFromDoc()
    brc = hCreateModuleForDoc()

    '///+<li>Click &quot;Compile&quot; on Toolbar</li>

    kontext "macrobar"
    printlog "- compile"
    Compile.Click
    if ( WaitSlot <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif

    '///+<li>Click &quot;Run Basic&quot; on Toolbar</li>
    printlog "- Run Basic"
    BasicRun.Click
    if ( WaitSlot <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif


    '///+<li>Click &quot;Step Procedure&quot; on Toolbar</li>
    printlog "- Step Procedure"
    ProcedureStep.Click
    if ( WaitSlot <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif


    '///+<li>Click &quot;Stop Basic&quot; on Toolbar</li>
    printlog "- Stop Basic"
    BasicStop.Click
    if ( WaitSlot <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif


    '///+<li>Click &quot;Single Step&quot; on Toolbar</li>
    printlog "- Single Step"
    SingleStep.Click
    if ( WaitSlot <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif


    '///+<li>Click &quot;Step Back&quot; on Toolbar</li>
    printlog "- Step Back"
    StepBack.Click()
    if ( WaitSlot <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif


    '///+<li>Click &quot;Breakpoint&quot; on Toolbar</li>
    printlog "- Breakpoint => activate"
    Breakpoint.Click
    if ( WaitSlot <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif


    '///+<li>Deactivate &quot;Breakpoint&quot; on Toolbar</li>
    printlog "- Breakpoint => deactivate"
    Breakpoint.Click
    if ( WaitSlot <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif


    '///+<li>Click &quot;Add Watch&quot; on Toolbar</li>
    printlog "- Add Watch => deactivate"
    AddWatch.Click
    if ( WaitSlot <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif


    '///+<li>Click &quot;Find Paranthese&quot; on Toolbar</li>
    printlog "- Find Paranthese"

    kontext "macrobar"
    FindParanthese.Click()
    if ( WaitSlot <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif


    '///+<li>&quot;Controls&quot;</li>
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

    '///+<li>&quot;Insert Source Text&quot;<br>
    '///+Insert external file &quot;ResetRegistration.txt&quot;</li>

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

    '///+<li>Click &quot;Save Source Test&quot; on Toolbar, save the file</li>
    SaveSourceTest.Click
    if ( WaitSlot <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif


    kontext "speicherndlg"
    if ( SpeichernDlg.exists( 1 ) ) then
        Dateiname.SetText ConvertPath (gOfficepath + "user\work\global_test.bas")
        Speichern.click()
    else
        warnlog( "File Save dialog is missing" )
    endif

    kontext "active"
    if ( Active.Exists( 1 ) ) then
        Active.Yes()
    endif

    '///+<li>File/Close for Basic-IDE and Document</li></ul>
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

    '///<h1>Work with Toolbar on Basic-IDE part 2</h1>
    '///<ul>

    '///+<li>Open a new writer-doc</li>
    printlog "open a new writer-doc"
    gApplication = "WRITER"
    hCreateDocument()

    '///+<li>Create a new module for the new document (named TTModule)</li>
    printlog "create a new module "
    brc = hOpenBasicOrganizerFromDoc()
    brc = hCreateModuleForDoc()

    '///+<li>Create a new dialog in BasicIDE</li>
    kontext "basicide"
    Tabbar.OpenContextMenu
    hMenuSelectNr ( 1 )
    hMenuSelectNr ( 2 )
    if ( DialogWindow.Exists ( 2 ) ) then
        printlog( "Dialog Editor window is open. Good." )
    else
        warnlog "No dialog window is shown!"
    end if
    
    '///+<li>Macro execution buttons should be disabled</li>
    '///<ul>
    '///+<li>Compile</li>
    try
        kontext "MacroBar"
        Compile.Click()
        warnlog( """Compile"" is enabled" )
    catch
        printlog( """Compile"" is disabled" )
    endcatch

    '///+<li>Run Basic</li>
    try
        kontext "MacroBar"
        BasicRun.Click()
        warnlog( """Run Basic"" is enabled" )
    catch
        printlog( """Run Basic"" is disabled" )
    endcatch

    '///+<li>Step Procedure</li>
    printlog "- Step Procedure (disabled)"
    try
        kontext "MacroBar"
        ProcedureStep.Click()
        warnlog( """Step Procedure"" is enabled" )
    catch
        printlog( """Step Procedure"" is disabled" )
    endcatch

    '///+<li>Stop Basic</li>
    try
        kontext "MacroBar"
        BasicStop.Click()
        warnlog( """Stop Basic"" is enabled" )
    catch
        printlog( """Stop Basic"" is disabled" )
    endcatch

    '///+<li>Single Step</li>
    try
        kontext "MacroBar"
        SingleStep.Click()
        warnlog( """Single Step"" is enabled" )
    catch
        printlog( """Single Step"" is enabled" )
    endcatch

    '///+<li>Single Step Back</li>
    try
        kontext "MacroBar"
        StepBack.Click()
        warnlog( """Single Step Back"" is enabled" )
    catch
        printlog( """Single Step Back"" is enabled" )
    endcatch

    '///+<li>Breakpoint</li>
    try
        kontext "MacroBar"
        Breakpoint.Click()
        warnlog( """Breakpoint"" is enabled" )
    catch
        printlog( """Breakpoint"" is disabled" )
    endcatch


    '///+<li>Add Watch</li>
    try
        kontext "MacroBar"
        AddWatch.Click()
        warnlog( """Add Watch"" is enabled" )
    catch
        printlog( """Add Watch"" is disabled" )
    endcatch


    '///+<li>Find Paranthese</li>
    try
        kontext "MacroBar"
        FindParanthese.Click()
        warnlog( """Find Paranthese"" is enabled" )
    catch
        printlog( """Find Paranthese"" is disabled" )
    endcatch


    '///+<li>Insert Source Text</li>
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

    '///+<li>Click &quot;Save Source Text&quot; (disabled)<br>
    '///+Save it as bas-file</li>
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

    '///</ul>
    
    '///+<li>Tear off controls</li>
    bIsOpen = hShowMacroControls()
    if ( bIsOpen ) then
        ToolsCollectionBar.Close()
    end if

    '///+<li>File/Close on Basic-IDE and Document</li></ul>
    Call hDestroyDocument()   ' for Basic-IDE
    Call hDestroyDocument()   ' the new writer-doc
    '///</ul>

endcase


