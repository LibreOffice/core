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
'/**************************************************************************
'*
'* Owner : wolfram.garten@oracle.com
'*
'* short description : Impress User-Scenario: Beginner makes presentation.
'*
'\**************************************************************************

testcase i_us2_pres1

    dim Result1, Result2 as integer
    dim iPictures as integer
    dim PresentationFile1 as string
    PresentationFile1 = (ConvertPath (gOfficePath + "user\work\PwrPres1.odp"))

    kontext "DocumentImpress"
    printlog "Starting with New - Wizard - Presentation."
    FileAutopilotPresentation
    printlog "Called File-Autopilot-Presentation, create from template."
    kontext "AutoPilotPraesentation1"
    FromTemplate.Check
    if gOOO = TRUE then
        TemplateRegion.Select (1)
        printlog "Choose a Presentation"
        TemplateList.Select (2)
    else
        TemplateRegion.Select (1)
        if TemplateList.GetItemCount < 40 then
            TemplateRegion.Select (2)
            if TemplateList.GetItemCount < 40 then
                TemplateRegion.Select (3)
                if TemplateList.GetItemCount < 40 then
                    Warnlog "No Templates selectable in the wizard. Please check."
                    goto endsub
                endif
            endif
        endif
        printlog "Choose a Presentation"
        TemplateList.Select (4)
    endif
    printlog "Next"
    Nextbutton.Click
    sleep (1)
    Backbutton.Click
    sleep (1)
    Nextbutton.Click

    printlog "Switched to the second Wizard-page."
    sleep (1)
    printlog "Presentations"
    kontext "AutoPilotPraesentation2"
    printlog "Output: Screen"
    if gOOO = TRUE then 
        Background.Select (2)
        if Backgroundchoice.GetItemCount < 2 then
            Background.Select (1)
            if Backgroundchoice.GetItemCount = 0 then
                Warnlog "No Backgrounds selectable in the wizard. Please check."
                goto endsub
            endif
        endif
        Backgroundchoice.Select (2)
    else 
        Background.Select (3)
        if Backgroundchoice.GetItemCount < 10 then
            Background.Select (2)
            if Backgroundchoice.GetItemCount < 10 then
                Background.Select (3)
                if Backgroundchoice.GetItemCount < 10 then
                    Warnlog "No Backgrounds selectable in the wizard. Please check."
                    goto endsub
                endif
            endif
        endif
        Backgroundchoice.Select (5)
    endif
    Screen.Check
    printlog "Next"
    Nextbutton.Click

    printlog "Switched to the third Wizard-page."
    sleep (1)
    kontext "AutoPilotPraesentation3"
    printlog "Random Effect. Random Speed. Click through every choice."
    Effect.Select (15)
    Speed.Select (3)
    WaitSlot (5000)
    Automatic.Check
    Timebutton.SetText "00:00:15"
    Break.SetText "00:00:20"
    Logo.Check
    printlog "Next"
    Nextbutton.Click

    printlog "Switched to the fourth Wizard-page."
    sleep (1)
    kontext "AutoPilotPraesentation4"
    printlog "Fill in some company-name-subject-text"
    AddName.SetText "Lala1"
    AddTopic.SetText "Lala2"
    Askinformation.SetText "Lala3"
    printlog "Next"
    Nextbutton.Click
    printlog "   Switched to the fifth Wizard-page."
    sleep (1)
    kontext "AutoPilotPraesentation5"
    Summary.Check
    printlog "Create"
    WaitSlot (2000)
    AutoPilotPraesentation5.OK

    sleep (1)
    printlog "Pressed 'Create'."
    printlog "Add a Slide via Insert - Slide."
    Kontext "DocumentImpress"
    DocumentImpress.UseMenu
    hMenuSelectNr (4)
    hMenuSelectNr (1)

    Kontext "DocumentImpress"
    DocumentImpress.UseMenu
    hMenuSelectNr (5)
    hMenuSelectNr (13)
    printlog "Changing focus to TaskPane."
    kontext "Tasks"
    sleep (1)
    printlog "Selecting 5th layout."
    LayoutsPreview.TypeKeys "<HOME>"
    sleep (1)
    LayoutsPreview.TypeKeys "<RIGHT>", 4
    LayoutsPreview.TypeKeys "<RETURN>"
    sleep (1)
    kontext "DocumentImpress"

    sleep (1)
    hTypeKeys "What we will talk about"
    gMouseClick 1,1

    kontext "Slides"
    SlidesControl.TypeKeys "<HOME>"
    kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<TAB>"
    DocumentImpress.TypeKeys "Text we just wrote..."
    DocumentImpress.TypeKeys "<ESCAPE>"
    printlog "Switch to the second slide."
    kontext "Slides"
    SlidesControl.TypeKeys "<DOWN>"
    kontext "DocumentImpress"
    printlog "Insert three lines with text, all with different formatting."
    sleep (1)
    call hTextrahmenErstellen ("First line with text",20,50,70,50)
    sleep (1)
    call hTextrahmenErstellen ("Second line with text",20,60,70,60)
    sleep (1)
    call hTextrahmenErstellen ("Third line with text",20,70,70,70)
    printlog "Go down to the third slide."
    kontext "Slides"
    SlidesControl.TypeKeys "<HOME>"
    SlidesControl.TypeKeys "<PAGEDOWN>", 3
    printlog "Insert Smiley, + two circles around the eyes, + two new pupils,"
    printlog "placed at some interesteing place inside the circles."
    Kontext "Toolbar"
    SymbolShapes.TearOff
    sleep (1)
    kontext "SymbolShapes"
    SymbolShapesSmiley.Click
    sleep (1)
    kontext "DrawingObjectbar"
    AreaFilling.TypeKeys "<HOME>"
    AreaFilling.TypeKeys "<DOWN>", 10
    printlog "Select (11) 'Lime-Green"
    AreaFilling.TypeKeys "<RETURN>"
    sleep (1)
    gMouseMove (53,50,90,80)
    Kontext "Toolbar"
    Ellipsen.Click
    kontext "SymbolShapes"
    SymbolShapes.Close
    Kontext "DocumentImpress"
    gMouseMove (60,43,70,65)
    hTypeKeys "<Escape>"
    Kontext "Toolbar"
    Ellipsen.Click
    gMouseMove (65,57,69,63)
    hTypeKeys "<Escape>"
    Kontext "Toolbar"
    Ellipsen.Click
    gMouseMove (70,50,82,65)
    hTypeKeys "<Escape>"
    Kontext "Toolbar"
    Ellipsen.Click
    gMouseMove (77,57,80,63)
    hTypeKeys "<Escape>"
    sleep (1)

    kontext "Slides"
    printlog "get to the last slide."
    SlidesControl.TypeKeys "<PAGEDOWN>", 3
    printlog "Insert 4th slide."
    SlidesControl.TypeKeys "<RETURN>"
    printlog "Delete everything on the page."
    kontext "DocumentImpress"
    gMouseClick 1,1
    EditSelectAll
    DocumentImpress.TypeKeys "<DELETE>"
    printlog "Insert-Picture-From File. (some crazy picture which fits the theme)"
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
    printlog "Open graphic"
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\ball.eps")
    Oeffnen.Click
    gMouseClick 1,1
    printlog "Add a text above the picture. 'There is movement..."
    call hTextrahmenErstellen ("There is movement...",20,40,70,30)

    printlog "Add a new slide."
    InsertSlide
    printlog "Inserted new slide."
    kontext "DocumentImpress"
    printlog "Bla bla about everything..."
    DocumentImpress.TypeKeys "<TAB>"
    DocumentImpress.TypeKeys "Bla bla about everything..."
    DocumentImpress.TypeKeys "<ESCAPE>",2

    printlog "Add a new slide."
    InsertSlide
    printlog "   Inserted new slide."
    kontext "DocumentImpress"
    printlog "Write text:  Q & A"
    DocumentImpress.TypeKeys "<TAB>"
    DocumentImpress.TypeKeys "Q & A"
    DocumentImpress.TypeKeys "<ESCAPE>",2
    sleep (1)

    printlog "Add a new slide."
    InsertSlide
    kontext "DocumentImpress"

    printlog "Thanks for listening, time for coffee... (ending)"
    DocumentImpress.TypeKeys "<TAB>"
    DocumentImpress.TypeKeys "Class dismissed..."
    DocumentImpress.TypeKeys "<ESCAPE>",2

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile1)
    printlog "OK, saved at ", PresentationFile1
    sleep (1)

    printlog "Close Document"
    Call hCloseDocument
endcase 'i_us2_pres1

'------------------------------------------------------------------------------

testcase i_us2_pres2
    qaerrorlog "not yet ready."
    goto endsub
    dim Result1, Result2 as integer
    dim iPictures as integer
    dim PresentationFile1, PresentationFile2, PresentationFileToBeLoaded as string
    PresentationFile1 = (ConvertPath (gTesttoolPath + "graphics\required\input\rightfont.odp"))
    PresentationFile2 = (ConvertPath (gOfficePath + "user\work\xxxxxx9.odp"))
    kontext "DocumentImpress"
    printlog "New - Wizard - Presentation"
    FileAutopilotPresentation
    printlog "   Called File-Autopilot-Presentation."
    printlog "Create from Template"
    kontext "AutoPilotPraesentation1"
    EmptyPresentation.Check
    AutoPilotPraesentation1.TypeKeys "<DOWN>"
    sleep (3)
    AutoPilotPraesentation1.TypeKeys "<DOWN>"
    if Existingpresentation.IsChecked(5) then
        printlog "   Pressing down worked fine"
    else
        warnlog "   Pressing down didnt work as expected."
    endif
    printlog "Click the Open-button"
    ExistingOpen.Click
    printlog "Check if the Open-dialogue is there"
    kontext "GeneralFileDialog"
    if GeneralFileDialog.Exists(10) = FALSE then
        warnlog "   Open-file-dialogue didnt show up."
    else
        GeneralFileDialog.Cancel
    endif
    sleep (3)
    kontext "AutoPilotPraesentation1"
    AutoPilotPraesentation1.TypeKeys "<SHIFT TAB>"
    AutoPilotPraesentation1.TypeKeys "<UP>"
    AutoPilotPraesentation1.TypeKeys "<DOWN>"
    AutoPilotPraesentation1.TypeKeys "<TAB><TAB>"
    sleep (1)
    Existinglist.TypeKeys "<HOME>"
    sleep (1)
    Existinglist.TypeKeys "<DOWN>"
    sleep (3)
    if Existinglist.GetSelText = "" then
        Warnlog "   No entries in the recently-opened-files-list. Check if it's correct."
    else
        PresentationFileToBeLoaded = Existinglist.GetSelText
        AutoPilotPraesentation1.OK
    endif

    kontext "DocumentImpress"
    if DocumentImpress.Exists(10) = FALSE then
        if DocumentImpress.StatusIsProgress = FALSE then
            sleep (10)
            if DocumentImpress.StatusIsProgress then
                warnlog "   Took over 20 seconds to load the document Document: " + PresentationFileToBeLoaded + ". Stalled?"
            endif
            goto endsub
        endif
    else
        printlog "   Document " + PresentationFileToBeLoaded + " seemed to have been loaded correctly."
    endif
    sleep (3)
    if DocumentImpress.StatusIsProgress then
        sleep (5)
    endif
    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile2)
    printlog "OK   saved at ", PresentationFile2
    sleep (1)

    printlog "Close Document"
    Call hCloseDocument
endcase 'i_us2_pres2
