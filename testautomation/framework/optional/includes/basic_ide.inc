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

*****

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

