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
'* short description: includefile for Paneview-testing
'*
'**************************************************************************************
' #1 tiMinimize_Maximize_Docking_SlidePane
' #1 tiMinimize_Maximize_Docking_TaskPane
' #1 tdMinimize_Maximize_Docking_Pagepane
' #1 tiPaneview_LoadSave_Windowsstate
' #1 tdPaneview_LoadSave_Windowsstate
'\*************************************************************************************

testcase tiMinimize_Maximize_Docking_SlidePane
    Dim xPos%
    Dim yPos%
    Dim xSize%
    Dim ySize%

    printlog "- SlidePane : using all possible things to manipulate the SlidePane"
    Call  hNewDocument
    Kontext "Slides"
    if (Slides.isVisible = false) Then   'isReallyVisible ?
        qaErrorLog "- Expected SlidePane to be open; But is closed. Check tests."
        Kontext "DocumentImpress"
        ViewTaskSlide
        sleep 1
    else
        printlog "SlidePane was open - good."
    end if
    Kontext "Slides"

    'Only because of overflow-error when some part of the pane is outside the window.

    if Slides.IsDocked then Slides.UnDock

        Slides.Move 15, 15


        xSize% = Slides.GetSizeX
        ySize% = Slides.GetSizeY
        xPos% = Slides.GetPosX
        yPos% = Slides.GetPosY
        Slides.Move 300, 400
        Sleep 1
        Slides.Move 590, 790
        Sleep 1
        Slides.Move 10, 10
        Sleep 1

        Slides.Size 200, 200
        Sleep 1
        Slides.Size 300, 400
        Sleep 1
        Slides.Size 170, 250
        Sleep 1
        Slides.Size xSize%, ySize%
        Sleep 1

        printlog "  -   Close the SlidePane, reopen it, check if appearence is same as before"
        xPos% = Slides.GetPosX
        yPos% = Slides.GetPosY
        xSize% = Slides.GetSizeX
        ySize% = Slides.GetSizeY
        Kontext "DocumentImpress"
        ViewTaskSlide
        ViewTaskSlide
        Kontext "Slides"
        Sleep 1
        if xPos% <> Slides.GetPosX then Warnlog "The x-Pos. has changed :  Should be ->  "+ xPos% + "  Is -> "+ Slides.GetPosX
            if yPos% <> Slides.GetPosY then Warnlog "The y-Pos. has changed :  Should be ->  "+ yPos% + "  Is -> "+ Slides.GetPosY
                if xSize% <> Slides.GetSizeX then Warnlog  "The x-Size has changed   : Should be -> "+ xSize% + "  Is ->  "+ Slides.GetSizeX
                    if ySize% <> Slides.GetSizeY then Warnlog  "Die y-Size has changed   : Should be -> "+ ySize% + "  Is ->  "+ Slides.GetSizeY

                        printlog "  -   dock SlidePane"
                        Slides.Dock
                        Sleep 1
                        if (NOT Slides.IsDocked) then
                            Warnlog "SlidePane not docked but should be"
                        end if
                        if (Slides.IsDocked) Then
                            Slides.UnDock
                        end if
                        Sleep 1

                        printlog "  -   maximize/minimize SlidePane"
                        Slides.Minimize
                        Sleep 1
                        Slides.Maximize
                        Sleep 1
                        if (NOT Slides.IsMax) then
                            Warnlog "The Slides-docking-window did not return to maximized state"
                        end if
                        Slides.Close
                        Kontext "DocumentImpress"
                        ViewTaskSlide
                        Sleep (2)
                        Kontext "Slides"
                        if (Slides.isVisible = false) Then   'isReallyVisible ?
                            warnlog "the Slidepane wasnt visible"
                        end if
                        Sleep (1)
                        Slides.Move xPos%, yPos%
                        Sleep 1
                        if (Slides.IsDocked = True) then
                            warnlog "the SlidePane was docked - should have been undocked."
                        else
                            printlog "the SlidePane was not docked, good."
                            Slides.Dock
                        end if
                        Call  hCloseDocument
endcase 'tiMinimize_Maximize_Docking_SlidePane

'------------------------------------------------------------------------------------'

testcase tiMinimize_Maximize_Docking_TaskPane
    Dim xPos%
    Dim yPos%
    Dim xSize%
    Dim ySize%

    printlog "- TaskPane : using all possible things to manipulate the TaskPane"
    Call  hNewDocument
    Kontext "Tasks"
    if (Tasks.isVisible = false) Then   'isReallyVisible ?
        qaErrorLog "- Expected TaskPane to be open; But is closed. Check tests."
        sleep 1
        Kontext "DocumentImpress"
        ViewTaskPane
    else
        printlog "TaskPane was open - good."
    end if
    Kontext "Tasks"

    'Only because of overflow-error when some part of the pane is outside the window.
    if Tasks.IsDocked then Tasks.UnDock

        Tasks.Move 15, 15

        xSize% = Tasks.GetSizeX
        ySize% = Tasks.GetSizeY

        xPos% = Tasks.GetPosX
        yPos% = Tasks.GetPosY
        Tasks.Move 300, 400
        Sleep 1
        Tasks.Move 590, 790
        Sleep 1
        Tasks.Move 10, 10
        Sleep 1
        Tasks.Size 200, 200
        Sleep 1
        Tasks.Size 300, 400
        Sleep 1
        Tasks.Size 150, 250
        Sleep 1
        Tasks.Size xSize%, ySize%
        Sleep 1

        printlog "  -   Close the TaskPane, reopen it, check if appearence is same as before"
        xPos% = Tasks.GetPosX
        yPos% = Tasks.GetPosY
        xSize% = Tasks.GetSizeX
        ySize% = Tasks.GetSizeY
        Kontext "DocumentImpress"
        ViewTaskPane
        ViewTaskPane
        Sleep 1
        Kontext "Tasks"
        if xPos% <> Tasks.GetPosX then Warnlog "The x-Pos. has changed :  Should be ->  "+ xPos% + "  Is -> "+ Tasks.GetPosX
            if yPos% <> Tasks.GetPosY then Warnlog "The y-Pos. has changed :  Should be ->  "+ yPos% + "  Is -> "+ Tasks.GetPosY
                if xSize% <> Tasks.GetSizeX then Warnlog "The x-Size has changed : Should be -> "+ xSize% + "  Is ->  "+ Tasks.GetSizeX
                    if ySize% <> Tasks.GetSizeY then Warnlog "Die y-Size has changed : Should be -> "+ ySize% + "  Is ->  "+ Tasks.GetSizeY

                        printlog "  -   dock TaskPane"
                        Tasks.Dock
                        Sleep 1
                        if (NOT Tasks.IsDocked) then
                            Warnlog "TaskPane not docked but should be"
                        end if
                        if (Tasks.IsDocked) Then
                            Tasks.UnDock
                        end if
                        Sleep 1

                        printlog "  -   maximize/minimize TaskPane"
                        Tasks.Minimize
                        Sleep 1
                        Tasks.Maximize
                        Sleep 1
                        if (NOT Tasks.IsMax) then
                            Warnlog "The Task-docking-window did not return to maximized state"
                        end if
                        Tasks.Close
                        Sleep (1)
                        Kontext "DocumentImpress"
                        ViewTaskPane
                        Sleep (2)
                        Kontext "Tasks"
                        if (Tasks.isVisible = false) Then   'isReallyVisible ?
                            warnlog "the Taskpane wasnt visible"
                        end if
                        Tasks.Move xPos%, yPos%
                        Sleep 1
                        if (Tasks.IsDocked = True) then
                            warnlog "the TaskPane was docked - should have been undocked."
                        else
                            printlog "the TaskPane was not docked, good."
                            Tasks.Dock
                        end if
                        Call hCloseDocument
endcase 'tiMinimize_Maximize_Docking_TaskPane

'------------------------------------------------------------------------------------'

testcase tiPaneview_LoadSave_Windowsstate
    QaErrorLog "#i84057# - tiPaneview_LoadSave_Windowsstate deactivated."
    goto endsub
    Dim xPos%
    Dim yPos%
    Dim xSize%
    Dim ySize%
    Dim Filenamex as string
    dim applicationchar as string

    Filenamex = ConvertPath (gOfficePath + "user\work\Task-postion")

    printlog "- TaskPane : using all possible things to manipulate the TaskPane"
    Call  hNewDocument
    kontext "Tasks"
    if (Tasks.isVisible = false) Then   'isReallyVisible ?
        qaErrorLog "- Expected TaskPane to be open; But is closed. Check tests."
        sleep 1
        Kontext "DocumentImpress"
        ViewTaskPane
    else
        printlog "TaskPane was open - good."
    end if

    Kontext "Tasks"
    'Only because of overflow-error when some part of the pane is outside the window.

    if Tasks.IsDocked then Tasks.UnDock
        Sleep (1)
        Tasks.Move 15, 15

        xSize% = Tasks.GetSizeX
        ySize% = Tasks.GetSizeY

        xPos% = Tasks.GetPosX
        yPos% = Tasks.GetPosY
        Sleep (2)
        Tasks.Size 200, 200
        Sleep 1
        Tasks.Move 10, 10
        Tasks.Size 300, 400
        Sleep 1
        Tasks.Move 550, 290
        Sleep 1

        printlog "  -   Close the TaskPane, reopen it, check if appearence is same as before"
        xPos% = Tasks.GetPosX
        yPos% = Tasks.GetPosY
        xSize% = Tasks.GetSizeX
        ySize% = Tasks.GetSizeY

        if Dir(Filenamex)<>"" then app.Kill(Filenamex) ' kill old file

            printlog "OK   saved at ", Filenamex
            sleep 1

            Call hCloseDocument

            hDateiOeffnen (Filenamex + "." + ExtensionString)

            Kontext "Tasks"
            if (Tasks.IsVisible = false) then
                warnlog "Taskpanel wasnt visible"
                Kontext "DocumentImpress"
                ViewTaskPane
                Sleep 1
            else
                printlog "Taskpanel was visible, good"
            end if

            Kontext "Tasks"
            if xPos% <> Tasks.GetPosX then Warnlog "The x-Pos. has changed :  Should be ->  "+ xPos% + "  Is -> "+ Tasks.GetPosX
                if yPos% <> Tasks.GetPosY then Warnlog "The y-Pos. has changed :  Should be ->  "+ yPos% + "  Is -> "+ Tasks.GetPosY
                    if xSize% <> Tasks.GetSizeX then Warnlog  "The x-Size has changed   : Should be -> "+ xSize% + "  Is ->  "+ Tasks.GetSizeX
                        if ySize% <> Tasks.GetSizeY then Warnlog  "Die y-Size has changed   : Should be -> "+ ySize% + "  Is ->  "+ Tasks.GetSizeY
                            Sleep (2)
                            Tasks.Size xSize%, ySize%
                            Sleep 1

                            printlog "  -   dock TaskPane"
                            Tasks.Dock
                            Call hCloseDocument
endcase

'----------------------------------------------------------------------------------------'

testcase tdPaneview_LoadSave_Windowsstate
    QaErrorLog "#i84057# - tiPaneview_LoadSave_Windowsstate deactivated."
    goto endsub
    Dim xPos%
    Dim yPos%
    Dim xSize%
    Dim ySize%
    Dim Filenamex as string

    Filenamex = ConvertPath (gOfficePath + "user\work\Task-postion")

    printlog "- Pagepane : using all possible things to manipulate the Pagepane"
    Call  hNewDocument
    kontext "Pagepane"
    if (Pagepane.isVisible = false) Then   'isReallyVisible ?
        qaErrorLog "- Expected Pagepane to be open; But is closed. Check tests."
        sleep 1
        Kontext "DocumentDraw"
        ViewPagepane
    else
        printlog "Pagepane was open - good."
    end if

    'Only because of overflow-error when some part of the pane is outside the window.

    if Pagepane.IsDocked then Pagepane.UnDock

        Pagepane.Move 15, 15


        Kontext "Pagepane"
        xSize% = Pagepane.GetSizeX
        ySize% = Pagepane.GetSizeY

        xPos% = Pagepane.GetPosX
        yPos% = Pagepane.GetPosY
        Pagepane.Size 200, 100
        Sleep 1
        Pagepane.Move 10, 10
        Pagepane.Size 300, 150
        Sleep 1
        Pagepane.Move 550, 290
        Sleep 1

        printlog "  -   Close the Pagepane, reopen it, check if appearence is same as before"
        xPos% = Pagepane.GetPosX
        yPos% = Pagepane.GetPosY
        xSize% = Pagepane.GetSizeX
        ySize% = Pagepane.GetSizeY

        if Dir(Filenamex)<>"" then app.Kill(Filenamex) ' kill old file

            printlog "OK   saved at ", Filenamex
            sleep 1

            Call hCloseDocument

            hDateiOeffnen (Filenamex + "." + ExtensionString)

            Kontext "Pagepane"
            if (Pagepane.IsVisible = false) then
                warnlog "Pagepane wasnt visible"
                Kontext "DocumentDraw"
                ViewPagepane
                Sleep 1
            else
                printlog "Pagepane was visible, good"
            end if

            Kontext "Pagepane"
            if xPos% <> Pagepane.GetPosX then Warnlog "The x-Pos. has changed :  Should be ->  "+ xPos% + "  Is -> "+ Pagepane.GetPosX
                if yPos% <> Pagepane.GetPosY then Warnlog "The y-Pos. has changed :  Should be ->  "+ yPos% + "  Is -> "+ Pagepane.GetPosY
                    if xSize% <> Pagepane.GetSizeX then Warnlog  "The x-Size has changed   : Should be -> "+ xSize% + "  Is ->  "+ Pagepane.GetSizeX
                        if ySize% <> Pagepane.GetSizeY then Warnlog  "Die y-Size has changed   : Should be -> "+ ySize% + "  Is ->  "+ Pagepane.GetSizeY

                            Pagepane.Size xSize%, ySize%
                            Sleep 1

                            printlog "  -   dock Pagepane"
                            Pagepane.Dock
                            Call hCloseDocument
endcase 'tdPaneview_LoadSave_Windowsstate

'--------------------------------------------------------------------------------------'

testcase tdMinimize_Maximize_Docking_Pagepane
    Dim xPos%
    Dim yPos%
    Dim xSize%
    Dim ySize%

    printlog "- Pagepane : using all possible things to manipulate the Pagepane"
    Call  hNewDocument
    Kontext "Pagepane"
    if (Pagepane.isVisible = false) Then   'isReallyVisible ?
        qaErrorLog "- Expected Pagepane to be open; But is closed. Check tests."
        sleep 1
        Kontext "DocumentDraw"
        ViewPagepane
    else
        printlog "Pagepane was open - good."
    end if
    Kontext "Pagepane"

    'Only because of overflow-error when some part of the pane is outside the window.

    if Pagepane.IsDocked then Pagepane.UnDock

        Pagepane.Move 10, 10
        Sleep 1

        xPos% = Pagepane.GetPosX
        yPos% = Pagepane.GetPosY
        xSize% = Pagepane.GetSizeX
        ySize% = Pagepane.GetSizeY
        Pagepane.Move 300, 400
        Sleep 1
        Pagepane.Move 590, 790
        Sleep 1
        Pagepane.Move xPos%, yPos%
        Sleep 1

        Pagepane.Size 200, 200
        Sleep 1
        Pagepane.Size 300, 400
        Sleep 1
        Pagepane.Size 170, 250
        Sleep 1
        Pagepane.Size xSize%, ySize%
        Sleep 1

        printlog "  -   Close the Pagepane, reopen it, check if appearence is same as before"
        xPos% = Pagepane.GetPosX
        yPos% = Pagepane.GetPosY
        xSize% = Pagepane.GetSizeX
        ySize% = Pagepane.GetSizeY
        Kontext "DocumentDraw"
        ViewPagepane
        ViewPagepane
        Sleep 1
        Kontext "Pagepane"
        if xPos% <> Pagepane.GetPosX then Warnlog "The x-Pos. has changed :  Should be ->  "+ xPos% + "  Is -> "+ Pagepane.GetPosX
            if yPos% <> Pagepane.GetPosY then Warnlog "The y-Pos. has changed :  Should be ->  "+ yPos% + "  Is -> "+ Pagepane.GetPosY
                if xSize% <> Pagepane.GetSizeX then Warnlog  "The x-Size has changed   : Should be -> "+ xSize% + "  Is ->  "+ Pagepane.GetSizeX
                    if ySize% <> Pagepane.GetSizeY then Warnlog  "Die y-Size has changed   : Should be -> "+ ySize% + "  Is ->  "+ Pagepane.GetSizeY

                        printlog "  -   dock Pagepane"
                        Pagepane.Dock
                        Sleep 1
                        if (NOT Pagepane.IsDocked) then
                            Warnlog "Pagepane not docked but should be"
                        end if
                        if (Pagepane.IsDocked) Then
                            Pagepane.UnDock
                        end if
                        Sleep 1

                        printlog "  -   maximize/minimize Pagepane"
                        Pagepane.Minimize
                        Sleep 1
                        Pagepane.Maximize
                        Sleep 1
                        if (NOT Pagepane.IsMax) then
                            warnlog "The Pagepane-docking-window did not return to maximized state"
                        end if
                        Kontext "DocumentDraw"
                        ViewPagepane
                        sleep (1)
                        Kontext "Pagepane"
                        if Pagepane.Exists(5)  Then
                            warnlog "the Pagepane wasnt closed properly"
                        else
                            Kontext "DocumentDraw"
                            ViewPagepane
                        end if
                        sleep (1)
                        Kontext "Pagepane"
                        if (Pagepane.Exists = true) AND (Pagepane.isVisible = false) Then
                            warnlog "the Pagepane wasnt visible"
                        end if

                        if (Pagepane.IsDocked = True) then
                            warnlog "the Pagepane was docked - should have been undocked."
                        else
                            printlog "the Pagepane was not docked, good."
                            Pagepane.Dock
                        end if
                        Call hCloseDocument
endcase 'tdMinimize_Maximize_Docking_Pagepane

'--------------------------------------------------------------------------------------'
'TODO FHA : testcase tClick_everywhere
'   Call hNewDocument
'   Kontext "LayerTabBar"
'Click everywhere

'Draw:
'Page Pane

'Imp:
'Slides:SlidesControl
'   kontext "Slides"
'   SlidesControl.MouseClick
'   kontext "Tasks"
'   View = same as slidescontrol
'   LayoutsPreview     HID_SD_TASK_PANE_PREVIEW_LAYOUTS
'endcase 'tClick_everywhere
'--------------------------------------------------------------------------------------'
