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

testcase tiViewPanes

    qaerrorlog( "This test is disabled, it needs rewriting" )
    goto endsub 'TODO WG, tiViewPanes outcommented due to reconstruction of test
    dim bState as boolean
    printlog "open application "
    Call hCloseDocument
    Call hNewDocument

    kontext "Tasks"
    if ( not Tasks.exists( 2 ) ) then
        warnlog "Tasks Panel not visible on opening application. Opening now."
        ViewTaskPane
    endif
    kontext "Slides"
    if (NOT Slides.exists) then
        warnlog "Slides Panel not visible on opening application. Opening now."
        ViewTaskSlide
    endif
    kontext "Tasks"
    printlog "Deactivate all but masterpages "
    View.OpenMenu
    hMenuSelectNr (2)
    View.OpenMenu
    hMenuSelectNr (3)
    View.OpenMenu
    hMenuSelectNr (4)

    sleep 1

    Tasks.TypeKeys ("<RIGHT><RIGHT><RIGHT>")

    try
        kontext "recentlyUsed"
        printlog "Toggeling Master Pages now with <space>"
        kontext "MasterPages"
        MasterPages.typeKeys "<space>"
        kontext "recentlyUsed"
        if (recentlyUsed.IsVisible = FALSE) then   'exists = FALSE) then
            warnlog "View-menu didn't work"
        endif
        kontext "Tasks"
        sleep 1
        printlog "View->Task Pane "
        ViewTaskPane
        sleep 1
        if (Tasks.exists) then
            warnlog "View->Task Panel failed"
            ViewTaskPane
        endif
        printlog "View->Task Pane "
        ViewTaskPane
        sleep 1
        if (NOT Tasks.exists) then
            warnlog "View->Task Panel failed"
            ViewTaskPane
        endif
    catch
        warnlog "View->Task Pane couldn't get executed"
    endcatch
    kontext "Slides"
    try
        ViewTaskSlide
        if (Slides.exists) then
            warnlog "View->Slide Panel failed."
            ViewTaskSlide
        endif
        printlog "View->Slide Pane "
        ViewTaskSlide
        sleep 1
        if (NOT Slides.exists) then
            warnlog "View->Slide Panel failed."
            ViewTaskSlide
        endif
    catch
        warnlog "View->Slide Pane couldn't get executed"
    endcatch

    printlog "Reactivate all pages in the Task-panel "
    kontext "Tasks"
    View.OpenMenu
    hMenuSelectNr (2)
    View.OpenMenu
    hMenuSelectNr (3)
    View.OpenMenu
    hMenuSelectNr (4)

endcase 'tiViewPanes

'---------------------------------------------------------

testcase tiViewMasterView
    printlog "open application"
    Call hNewDocument
    printlog "View->Master View->Drawing View"
    hUseAsyncSlot( "ViewWorkspaceDrawingView" )
    printlog "View->Master View->Outline View "
    hUseAsyncSlot( "ViewWorkspaceOutlineView" )
    printlog "View->Master View->Slides View "
    ViewWorkspaceSlidesView
    sleep 1
    printlog "View->Master View->Notes View "
    hUseAsyncSlot( "ViewWorkspaceNotesView" )
    printlog "View->Master View->Handout View "
    hUseAsyncSlot( "ViewWorkspaceHandoutView" )
    printlog "View->Master View->Drawing View "
    hUseAsyncSlot( "ViewWorkspaceDrawingView" )
    printlog "close application "
    Call hCloseDocument
endcase 'tiViewMasterView

'---------------------------------------------------------

testcase tiViewSlideMaster

    printlog "open application with : File->Autopilot->Presentation; OK; OK "
    FileAutopilotPresentation                ' to get a title :-)
    Kontext "AutopilotPraesentation1"
    if ( AutopilotPraesentation1.exists( 2 ) ) then
        hCloseDialog( AutopilotPraesentation1, "ok" )
    else
        warnlog( "Dialog <AutopilotPraesentation1> did not open" )
    endif
    kontext "DocumentImpress"
    
    printlog "View->Slide"
    hUseAsyncSlot( "ViewSlide" )
    
    printlog "View->Master->Drawing"
    hUseAsyncSlot( "ViewDrawing" )

    printlog "View->Slide"
    hUseAsyncSlot( "ViewSlide" )

    printlog "View->Master->Handout"
    hUseAsyncSlot( "ViewHandout" )

    printlog "View->Master->Notes"
    hUseAsyncSlot( "ViewNotes" )
    kontext "DocumentImpress"

    printlog "View->Slide"
    hUseAsyncSlot( "ViewSlide" )
    sleep 1

    printlog "close application "
    Call  hCloseDocument

endcase 'tiViewSlideMaster

'--------------------------------------------------------

testcase tiViewToolbar_1

    const DEFAULT_DELAY = 3

    Dim NumberOfGraphicModes as integer
    Dim iCurrentGraphicsMode as integer
    dim TestFile as string
    dim i as integer
    
    TestFile = ConvertPath (gTesttoolPath + "global\input\graf_inp\desp.bmp")
    printlog "open application "
    Call hNewDocument

    printlog "use the empty layout"
    Call sSelectEmptyLayout

    printlog "delete default content"
    hUseAsyncSlot( "EditSelectAll" )
    Kontext "DocumentImpress"
    DocumentImpress.typeKeys("<DELETE>",true)
    printlog "Insert->Graphic... : global\input\graf_inp\desp.bmp "
    hUseAsyncSlot( "InsertGraphicsFromFile" )
    Kontext "GrafikEinfuegenDlg"
    if ( GrafikEinfuegenDlg.exists( DEFAULT_DELAY ) ) then
        Dateiname.SetText( TestFile )
        Oeffnen.Click

        Kontext "Messagebox"
        if ( Messagebox.Exists( DEFAULT_DELAY ) ) then
            Warnlog Messagebox.GetText
            hCloseDialog( Messagebox, "ok" )
        end if

        Kontext "DocumentImpress"
        printlog "select graphic "
        hUseAsyncSlot( "EditSelectAll" )

        printlog "The Graphics Toolbar has to be visible now; If not -> ERROR "
        ' workaround for i113609; there should have been a style selected without elements on creating the document - what failed; the elements should have been deleted before inserting the grafik in this test - which failed; so the workaround is to use <tab> to travel to the graphic selection here:
        for i=1 to 3
        Kontext "GraphicObjectbar"
        if ( not GraphicObjectbar.Exists( DEFAULT_DELAY ) ) Then
            hTypeKeys("<tab>")
            qaerrorlog "delete default content failed"
        end if
        next i

        Kontext "GraphicObjectbar"
        if ( GraphicObjectbar.Exists( DEFAULT_DELAY ) ) Then

            Printlog "- graphic object toolbar exists"
            NumberOfGraphicModes=Grafikmodus.GetItemCount

            for iCurrentGraphicsMode = 1 to NumberOfGraphicModes
                Printlog( "- access all controls in the toolbar (" & iCurrentGraphicsMode & "/" & NumberOfGraphicModes & ")" )
                Grafikmodus.Select( iCurrentGraphicsMode )
                sleep ( DEFAULT_DELAY )
            next iCurrentGraphicsMode
        else
            Warnlog "- No graphic function toolbar visible"
        end if
    else
        warnlog( "Insert Graphics dialog did not open" )
    endif

    printlog "close application "
    Call hCloseDocument

endcase 'tiViewToolbar_1