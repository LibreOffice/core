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
'* Owner : wolfram.garten@sun.com
'*
'* short description : Impress Resource Test: View Menu
'*
'***********************************************************************************
' #1 tiViewPanes
' #1 tiViewMasterView
' #1 tiViewSlideMaster
' #1 tiViewToolbar_1
'\**********************************************************************************
testcase tiViewPanes

    goto endsub 'TODO WG, tiViewPanes outcommented due to reconstruction of test
    dim bState as boolean
    printlog "open application "
    Call hCloseDocument
    Call hNewDocument
    sleep 1
    kontext "Tasks"
    if (NOT Tasks.exists) then
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
    sleep 1
    ViewWorkspaceDrawingView
     sleep 5
    printlog "View->Master View->Outline View "
    ViewWorkspaceOutlineView
     sleep 1
    printlog "View->Master View->Slides View "
    ViewWorkspaceSlidesView
     sleep 1
    printlog "View->Master View->Notes View "
    ViewWorkspaceNotesView
      sleep 1
    printlog "View->Master View->Handout View "
    ViewWorkspaceHandoutView
      sleep 1
    printlog "View->Master View->Drawing View "
    ViewWorkspaceDrawingView
    printlog "close application "
      sleep 1
    Call hCloseDocument
   
endcase 'tiViewMasterView
'---------------------------------------------------------
testcase tiViewSlideMaster
    
    printlog "open application with : File->Autopilot->Presentation; OK; OK "
    FileAutopilotPresentation                ' to get a title :-)
    sleep 2
    Kontext "AutopilotPraesentation1"
    AutopilotPraesentation1.Ok
    sleep 1
    Kontext "Seitenlayout"  ' aka: Modify Slide
    if Seitenlayout.exists(5) then
        warnlog "Slidelayout has to vanish; moved to sidebar"
        Seitenlayout.OK
    endif
    kontext "DocumentImpress"
    sleep 1
    printlog "View->Slide "
    ViewSlide
    Sleep 1
    printlog "View->Master->Drawing "
    ViewDrawing
    Sleep 1
    printlog "View->Slide "
    ViewSlide
    Sleep 1
    printlog "View->Master->Title "
    try
    ViewTitle
    Errorlog "View - Master - Title Slide Master should NOT be accessable"
    catch
    printlog "View - Master - Title Slide Master not accessable - good"
    endcatch
    Sleep 1
    printlog "View->Slide "
    ViewSlide
    Sleep 1
    printlog "View->Master->Handout "
    ViewHandout
    Sleep 1
    '   ViewSlide
    Sleep 1
    printlog "View->Master->Notes "
    ViewNotes
    kontext "DocumentImpress"
    Sleep 1
    printlog "View->Slide "
    ViewSlide
    Sleep 1
    printlog "close application "
    Call  hCloseDocument
   
endcase 'tiViewSlideMaster
'--------------------------------------------------------
testcase tiViewToolbar_1
    
    Dim Zaehler as integer
    Dim i as integer
    
    printlog "open application "
    Call hNewDocument
    sleep 2
    printlog "Insert->Graphic... : global\input\graf_inp\desp.bmp "
    InsertGraphicsFromFile
    sleep 2
    Kontext "GrafikEinfuegenDlg"
    sleep 1
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\desp.bmp")
    sleep 1
    Oeffnen.Click
    sleep 3
    Kontext "Messagebox"
    if Messagebox.Exists then
        Warnlog Messagebox.GetText
        Messagebox.OK
    end if
    Kontext "DocumentImpress"
    printlog "select graphic "
    hTypeKeys "<TAB>", 3
    sleep 2
    Kontext "GraphicObjectbar"
    sleep 1
    printlog "The Graphics Toolbar has to be visible now; If not -> ERROR "
    if GraphicObjectbar.Exists Then
        Printlog "- graphic object toolbar exists"
        Zaehler=Grafikmodus.GetItemCount
        for i = 1 to Zaehler
            Printlog "- access all controls in the toolbar (" +i+"/"+Zaehler+")"
            Grafikmodus.Select i
            sleep 3
        next i
        sleep 3
    else
        Warnlog "- No graphic function toolbar visible"
    end if
    printlog "close application "
    Call hCloseDocument
    
endcase 'tiViewToolbar_1
