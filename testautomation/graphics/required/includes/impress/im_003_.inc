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
'* Owner : wolfram.garten@oracle.com
'*
'* short description : Impress Resource Test: View Menu
'*
'\******************************************************************************

sub im_003_

    call tiViewPanes()
    call tiViewMasterView()
    call tiViewSlideMaster()
    call tiViewToolbar_1()

end sub

'*******************************************************************************

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