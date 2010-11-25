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
'* Owner : wolfram.garten@sun.com
'*
'* short description : Include-file for impress User-Scenario: Presentation.
'*
'***************************************************************************
' #1 i_us_presentation1
' #1 i_us_presentation2
' #1 i_us_presentation3
' #1 i_us_presentation4
' #1 i_us_presentation5
' #1 i_us_presentation6
' #1 i_us_presentation7
'\********************************************************************

testcase i_us_presentation1
 dim iPictures as integer
    dim PresentationFile1 as string
    PresentationFile1 = ConvertPath ((ConvertPath (gOfficePath + "user\work\PowerPes1.odp")))
    printlog "New impress document"
    Call hNewDocument
    Call sSelectEmptyLayout
    WaitSlot (2000)
    kontext "DocumentImpress"
    printlog "Add second Master-Page "
    ViewMasterPage
    kontext "Slides"
    SlidesControl.OpenContextMenu(true)
    sleep (2)
    printlog "New master"
    MenuSelect(MenuGetItemID(1))
    sleep (1)
    ViewMasterPage
    kontext "Slides"
    SlidesControl.OpenContextMenu(true)
    sleep (2)
    MenuSelect(MenuGetItemID(1)) 'New Master
    sleep (1)
    printlog "   Inserted second Master-Slide"

    printlog "set background to picture(Gallery)"
    Kontext "Gallery"
    if Gallery.Exists(2) then
        warnlog "   The Gallery was already visible. Check earlier ran tests for inconsistency."
        sleep (2)
    else
        ToolsGallery
        WaitSlot (2000)
    end if

    if gOOO = TRUE then   'OpenOffice.org
        select case iSprache
            case 01   : iPictures = 1 'English
            case else : iPictures = 1 'Unknown
                           warnlog "Please insert the entrienumbers for 'Backgrounds'. Language: " + iSprache
        end select

    else  ' StarOffice...
        select case iSprache
             case 01   : iPictures = 21 'English
             case 07   : iPictures = 28 'Russian
             case 31   : iPictures = 04 'Netherlands
             case 33   : iPictures = 22 'French
             case 34   : iPictures = 12 'Spanish
             case 36   : iPictures = 17 'Hungaria
             case 39   : iPictures = 23 'Italian
             case 46   : iPictures = 09 'Swedish
             case 48   : iPictures = 20 'Polish
             case 49   : iPictures = 11 'German
             case 55   : iPictures = 16 'Portuguese
             case 81   : iPictures = 22 'Japanese
             case 82   : iPictures = 04 'Korean
             case 86   : iPictures = 06 'Simplified
             case 88   : iPictures = 17 'Traditional
             case else : iPictures = 21 'Unknown
             warnlog "Please insert the entrienumbers for 'Backgrounds'. Language: " + iSprache
        end select
   end if

    kontext "Gallery"
    Gallerys.Select (iPictures)
    View.TypeKeys "<HOME><RIGHT><RIGHT>"
    printlog "OpenContextMenu"
    View.TypeKeys "<SHIFT F10>"
    sleep (2)
    MenuSelect(MenuGetItemID(1))
    sleep (2)
    MenuSelect(MenuGetItemID(1))
    sleep (2)
    printlog "Check that we really got a copy of the object"
    kontext "DocumentImpress"
    DocumentImpress.OpenContextMenu(true)
    sleep (2)
    MenuSelect 27353
    sleep (2)
    Kontext "ExportierenDlg"
    if ExportierenDlg.IsVisible(5) then
        printlog "   Gallery-object correctly copied into Slide."
        ExportierenDlg.Close
    else
        warnlog "   Doesn't seem like we copied anything from the Gallery... ?"
    end if
    kontext "GraphicObjectbar"
    if GraphicObjectbar.Exists(5) = FALSE then
        kontext "DocumentImpress"
        ViewToolbarsPicture
    end if

    Kontext "Gallery"
   View.TypeKeys "<HOME><RIGHT><RIGHT>"
   printlog "'OpenContextMenu"
   View.TypeKeys "<SHIFT F10>"
   sleep (2)
   MenuSelect(MenuGetItemID(1))   'Insert
   sleep (2)
   MenuSelect(MenuGetItemID(1))   'Copy
   sleep (2)
   printlog "Check that we really got a copy of the object "
   kontext "DocumentImpress"
   DocumentImpress.OpenContextMenu(true)
   sleep (2)
   MenuSelect 27353
   sleep (2)
   Kontext "ExportierenDlg"
   if ExportierenDlg.IsVisible(5) then
      printlog "   Gallery-object correctly copied into Slide."
      ExportierenDlg.Close
   else
      warnlog "   Doesn't seem like we copied anything from the Gallery... ?"
   end if
   kontext "GraphicObjectbar"
      if GraphicObjectbar.Exists(5) = FALSE then
         kontext "DocumentImpress"
         ViewToolbarsPicture
      end if

    printlog " 50% Transparency"
    WaitSlot (2000)
    kontext "GraphicObjectbar"
    Transparenz.SetText "50"
    sleep (1)

    printlog "Mosaic 16px x 16px"
    Filter.TearOff
    sleep (1)
    kontext "GraphicFilterBar"
    Mosaic.Click
    WaitSlot (2000)
    kontext "Mosaic"
    Width.SetText "16"
    Height.SetText "16"
    Mosaic.OK

    kontext "GraphicFilterBar"
    GraphicFilterBar.Close

    ToolsGallery
    WaitSlot (1000)
    kontext "DocumentImpress"
    hTypeKeys "<TAB>"

    sleep (1)
    FormatPositionAndSize
    WaitSlot (1000)
    kontext
    active.setPage(TabPositionAndSize)
    kontext "TabPositionAndSize"
    Width.SetText "800"
    Height.SetText "600"
    SizePosition.TypeKeys "<RIGHT><DOWN>"
    TabPositionAndSize.OK

    printlog "Close Master"
    ViewNormal
    WaitSlot (1000)

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile1)
    printlog "OK   saved at ", PresentationFile1
    WaitSlot (1000)

    ActiveDeactivateCTLSupport (FALSE)
    WaitSlot (2000)
    printlog "Close Document"
    Call hCloseDocument
endcase 'i_us_presentation1

'-------------------------------------------------------------------------------
testcase i_us_presentation2
    dim PresentationFile1 as string
    dim PresentationFile2 as string
    PresentationFile1 = ConvertPath ((ConvertPath (gOfficePath + "user\work\PowerPes1.odp")))
    PresentationFile2 = ConvertPath ((ConvertPath (gOfficePath + "user\work\PowerPes2.odp")))

    if Dir(PresentationFile1) <> "" then 'if file exists...
      hFileOpen (PresentationFile1)
      WaitSlot (10000)
    else
        warnlog "   This test is supposed to run after the previous testcase has been run. Notify the Automatic-tester."
        printlog "New impress document"
        Call hNewDocument
        WaitSlot (2000)
    end if

    printlog "New Slide"
    kontext "slides"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"

    SlidesControl.TypeKeys "<SHIFT F10>"   'OpenContextMenu(true)
    sleep (1)
    MenuSelect(MenuGetItemID(1)) 'New Slide  'No 2
    printlog "   Inserted second normal Slide"

    printlog "2. Layouts: Text. Bild: Gallery: Animation - Gif"
    Kontext "Tasks"
    printlog "to get to the very first position"
    LayoutsPreview.TypeKeys "<HOME>"
    printlog "'to get to the right position"
    LayoutsPreview.TypeKeys "<RIGHT>", 9  
    LayoutsPreview.TypeKeys "<RETURN>"

    Sleep (1)
    ViewNormal

    printlog "3. Hide Slidepane (Oops! The user were too fast: accidently hide the pane)"
    kontext "Slides"
    SlidesControl.FadeOut
    WaitSlot (1000)

    printlog "4. Restore Pane."
    SlidesControl.FadeIn

    printlog "   Did the 'mistake' to FadeIn/Out the Slidepane"
    
    printlog "activating the input picture area"
    kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<TAB>",2
    DocumentImpress.TypeKeys "<RETURN>"
    sleep (1)

    printlog "Graphics-Import-dialogue. Select i_us_large.jpg"
    Kontext "GrafikEinfuegenDlg"
    sleep (2)
    Dateiname.SetText ConvertPath (gTesttoolPath + "graphics\required\input\i_us_large.jpg")
    sleep (2)
    Oeffnen.Click
    WaitSlot (1000)
    Kontext "DocumentImpress"

    printlog "Deselect graphic"
    DocumentImpress.MouseDoubleClick 90,90
    printlog "   Inserted Graphic into the second Slide"

    printlog "Change text on the two text-boxes"
    DocumentImpress.TypeKeys "<TAB>" 'First text.
    DocumentImpress.TypeKeys "<RETURN>" 'To get into edit-mode.
    DocumentImpress.TypeKeys "The World has just become a bit easier"
    DocumentImpress.TypeKeys "<ESCAPE><ESCAPE>"
    DocumentImpress.TypeKeys "<TAB><TAB><TAB>"
    DocumentImpress.TypeKeys "<RETURN>"
    DocumentImpress.TypeKeys "100% Recyclable"
    DocumentImpress.TypeKeys "<RETURN>"
    DocumentImpress.TypeKeys "Very durable"
    DocumentImpress.TypeKeys "<RETURN>"
    DocumentImpress.TypeKeys "Priced lower than its predecessor!"
    DocumentImpress.TypeKeys "<RETURN>"
    DocumentImpress.TypeKeys "Sexy"
    DocumentImpress.TypeKeys "<RETURN>"
    DocumentImpress.TypeKeys "Energy-efficient"
    DocumentImpress.TypeKeys "<ESCAPE><ESCAPE>"

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile2)
    printlog "OK   saved at ", PresentationFile1
    sleep (1)

    ActiveDeactivateCTLSupport (FALSE)
    sleep (2)
    printlog "Close Document"
    Call hCloseDocument
endcase 'i_us_presentation2

'-------------------------------------------------------------------------------
testcase i_us_presentation3
    dim PresentationFile2 as string
    dim PresentationFile3 as string
    PresentationFile2 = ConvertPath ((ConvertPath (gOfficePath + "user\work\PowerPes2.odp")))
    PresentationFile3 = ConvertPath ((ConvertPath (gOfficePath + "user\work\PowerPes3.odp")))

    if Dir(PresentationFile2) <> "" then 'if file exists...
        hFileOpen (PresentationFile2)
        WaitSlot (10000)
    else
        warnlog "   This test is supposed to run after the previous testcase has been run. Notify the Automatic-tester."
        printlog "New impress document"
        Call hNewDocument
        WaitSlot (2000)
    end if

    printlog "Insert New Slide"
    kontext "slides"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"

    kontext "DocumentImpress"
    InsertSlide 'No 3

    printlog "5. Layout. Clip/Text"
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<HOME>"     'to get to the very first position
    LayoutsPreview.TypeKeys "<RIGHT>", 4 'to get to the right position
    LayoutsPreview.TypeKeys "<RETURN>"

    sleep (1)
    ViewNormal

    kontext "DocumentImpress"
    Call gMouseClick 50,50
    DocumentImpress.TypeKeys "<TAB><RETURN>"
    DocumentImpress.TypeKeys "A new form"

    printlog "6. (Fat picture) InsertPictureFromFile: (empty slide) (ev size-fit)"
    printlog "insert graphic file (i_us_large.jpg)"
    InsertGraphicsFromFile
    WaitSlot (1000)
    Kontext "GrafikEinfuegenDlg"
        Dateiname.SetText ConvertPath (gTesttoolPath + "graphics\required\input\i_us_large.jpg")
        sleep (2)
        Oeffnen.Click
        WaitSlot (2000)
    Kontext "DocumentImpress"

    printlog "The user corrects the picture"
    DocumentImpress.MouseDown 50,50
    DocumentImpress.MouseUp 50,50
    DocumentImpress.TypeKeys "<DOWN>", 30

    printlog "Deselect graphic"
    DocumentImpress.MouseDoubleClick 90,90

    printlog "   Wrote Text, Inserted Graphic, and moved it in the third Slide"

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile3)
    printlog "OK   saved at ", PresentationFile3
    sleep (1)

    ActiveDeactivateCTLSupport (FALSE)
    sleep (2)
    printlog "Close Document"
    Call hCloseDocument
endcase 'i_us_presentation3

'-------------------------------------------------------------------------------
testcase i_us_presentation4
    dim PresentationFile3 as string
    dim PresentationFile4 as string
    dim iAnimations as Integer
    PresentationFile3 = ConvertPath ((ConvertPath (gOfficePath + "user\work\PowerPes3.odp")))
    PresentationFile4 = ConvertPath ((ConvertPath (gOfficePath + "user\work\PowerPes4.odp")))

    if Dir(PresentationFile3) <> "" then 'if file exists...
        hFileOpen (PresentationFile3)
        WaitSlot (10000)
    else
        warnlog "   This test is supposed to run after the previous testcase has been run. Notify the Automatic-tester."
        printlog "New impress document"
        Call hNewDocument
        WaitSlot (2000)
    end if

    kontext "slides"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    kontext "DocumentImpress"

printlog "insert slide no 4"
InsertSlide

    printlog "Background: picture (Gallery)"
    Kontext "Gallery"
    if Gallery.Exists(2) then
        warnlog "   The Gallery was already visible. Check earlier ran tests for inconsistency."
        sleep (2)
    else
        ToolsGallery
        WaitSlot (2000)
    end if

    select case iSprache
        case 01   : iAnimations = 01 'English
        case 07   : iAnimations = 01 'Russian
        case 31   : iAnimations = 03 'Netherlands
        case 33   : iAnimations = 01 'French
        case 34   : iAnimations = 01 'Spanish
        case 36   : iAnimations = 01 'Hungaria
        case 39   : iAnimations = 01 'Italian
        case 46   : iAnimations = 01 'Swedish
        case 48   : iAnimations = 01 'Polish
        case 49   : iAnimations = 01 'German
        case 55   : iAnimations = 01 'Portuguese
        case 81   : iAnimations = 02 'Japanese
        case 82   : iAnimations = 17 'Korean
        case 86   : iAnimations = 07 'Simplified
        case 88   : iAnimations = 15 'Traditional
        case else : iAnimations = 01 'Unknown
                            warnlog "Please insert the entrienumbers for 'Backgrounds'. Language: " + iSprache
        end select

    kontext "Gallery"
    Gallerys.Select (iAnimations)

    kontext "Gallery"
        View.TypeKeys "<HOME><RIGHT><RIGHT>"
        sleep (1)
        View.TypeKeys "<SHIFT F10>" 'OpenContextMenu
        sleep (1)
        MenuSelect(MenuGetItemID(1))   'Insert
        sleep (1)
        MenuSelect(MenuGetItemID(1))   'Copy

        printlog "Check that we really got a copy of the object"
        kontext "DocumentImpress"
        DocumentImpress.OpenContextMenu(true)
        WaitSlot (1000)
        MenuSelect 27353

    Kontext "ExportierenDlg"
    if ExportierenDlg.IsVisible(5) then
        printlog "   Gallery-object correctly copied into Slide."
        ExportierenDlg.Close
    else
        warnlog "   Doesn't seem like we copied anything from the Gallery... ?"
    end if
    kontext "DocumentImpress"

    DocumentImpress.TypeKeys "<UP>", 82
    DocumentImpress.TypeKeys "<LEFT>", 130

    printlog "Deselect graphic"
    DocumentImpress.MouseDoubleClick 90,90

    printlog "Close the Gallery"
    ToolsGallery

    printlog "Change Text on slide"
    DocumentImpress.TypeKeys "<TAB>"
    DocumentImpress.TypeKeys "<RETURN>"
    DocumentImpress.TypeKeys "The process starts to flourish"
    DocumentImpress.TypeKeys "<ESCAPE><ESCAPE>"
    gMouseClick 50,50

    ActiveDeactivateCTLSupport (TRUE)

    CreateTextSetEffectAndAngle
    DocumentImpress.TypeKeys "<DOWN>", 80
    DocumentImpress.TypeKeys "<LEFT>", 100
    gMouseClick 90,90

    CreateTextSetEffectAndAngle
    DocumentImpress.TypeKeys "<DOWN>", 80
    DocumentImpress.TypeKeys "<LEFT>", 75
    gMouseClick 90,90

    CreateTextSetEffectAndAngle
    DocumentImpress.TypeKeys "<DOWN>", 80
    DocumentImpress.TypeKeys "<LEFT>", 50
    gMouseClick 90,90

    CreateTextSetEffectAndAngle
    DocumentImpress.TypeKeys "<DOWN>", 80
    DocumentImpress.TypeKeys "<LEFT>", 25
    gMouseClick 90,90

    CreateTextSetEffectAndAngle
    DocumentImpress.TypeKeys "<DOWN>", 80
    DocumentImpress.TypeKeys "<RIGHT>", 25
    gMouseClick 90,90

    CreateTextSetEffectAndAngle
    DocumentImpress.TypeKeys "<DOWN>", 80
    DocumentImpress.TypeKeys "<RIGHT>", 50
    gMouseClick 90,90

    CreateTextSetEffectAndAngle
    DocumentImpress.TypeKeys "<DOWN>", 80
    DocumentImpress.TypeKeys "<RIGHT>", 75
    gMouseClick 90,90

    CreateTextSetEffectAndAngle
    DocumentImpress.TypeKeys "<DOWN>", 80
    DocumentImpress.TypeKeys "<RIGHT>", 100
    gMouseClick 90,90

    printlog "   Inserted fourth slide with Gallery-object."

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile4)
    printlog "OK   saved at ", PresentationFile4
    sleep (1)

    ActiveDeactivateCTLSupport (FALSE)
    sleep (2)
    printlog "Close Document"
    Call hCloseDocument
endcase 'i_us_presentation4

'-------------------------------------------------------------------------------
testcase i_us_presentation5
    dim PresentationFile4 as string
    dim PresentationFile5 as string
    PresentationFile4 = ConvertPath ((ConvertPath (gOfficePath + "user\work\PowerPes4.odp")))
    PresentationFile5 = ConvertPath ((ConvertPath (gOfficePath + "user\work\PowerPes5.odp")))

    if Dir(PresentationFile4) <> "" then 'if file exists...
        hFileOpen (PresentationFile4)
        WaitSlot (10000)
    else
        warnlog "   This test is supposed to run after the previous testcase has been run. Notify the Automatic-tester."
        printlog "New impress document"
        Call hNewDocument
        WaitSlot (2000)
    end if

    kontext "slides"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"

    kontext "DocumentImpress"
    printlog "8. New Slide. (Insert Menu) (Duplicate slide)"
    InsertDuplicateSlide 'No 5
    printlog "Change the text in some way. (the user is making a joke with the audience)"
    gMouseClick 90,90
    DocumentImpress.TypeKeys "<TAB>"
    DocumentImpress.TypeKeys "<RETURN>"
    EditSelectAll   
    DocumentImpress.TypeKeys "And does it with strength..."

    printlog "   Inserted fifth slide with audience-joke."

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile5)
    printlog "OK   saved at ", PresentationFile5
    sleep (1)

    ActiveDeactivateCTLSupport (FALSE)
    sleep (2)
    printlog "Close Document"
    Call hCloseDocument
endcase 'i_us_presentation5

'-------------------------------------------------------------------------------
testcase i_us_presentation6
    dim PresentationFile5 as string
    dim PresentationFile6 as string
    PresentationFile5 = ConvertPath ((ConvertPath (gOfficePath + "user\work\PowerPes5.odp")))
    PresentationFile6 = ConvertPath ((ConvertPath (gOfficePath + "user\work\PowerPes6.odp")))

    if Dir(PresentationFile5) <> "" then 'if file exists...
        hFileOpen (PresentationFile5)
        WaitSlot (10000)
    else
        warnlog "   This test is supposed to run after the previous testcase has been run. Notify the Automatic-tester."
        printlog "New impress document"
        Call hNewDocument
        WaitSlot (2000)
    end if

    kontext "slides"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"

    kontext "DocumentImpress"
    printlog "9. Q&A Slide"
    InsertSlide 
    WaitSlot (1000)
    kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<TAB>"
    DocumentImpress.TypeKeys "<RETURN>"
    DocumentImpress.TypeKeys "Q&A"
    WaitSlot (1000)
    kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<SHIFT HOME>"
    Kontext "TextObjectbar"
        if TextObjectbar.Exists = FALSE then
            ViewToolbarsTextFormatting
        end if
        WaitSlot (2000)
        Printlog "- Change size of font"
        Schriftgroesse.Select "26"
        Schriftgroesse.TypeKeys "<RETURN>"
        Fett.Click
    kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<ESCAPE><ESCAPE>"
    kontext "Toolbar"
        Auswahl.Click
    gMouseClick 60,60
    EditSelectAll
    WaitSlot (1000)
    DocumentImpress.TypeKeys "<DOWN>", 50
    kontext "DocumentImpress"
    printlog "   Inserted sixth slide with Q&A."

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile6)
    printlog "OK   saved at ", PresentationFile6
    sleep (1)

    ActiveDeactivateCTLSupport (FALSE)
    WaitSlot (2000)
    printlog "Close Document"
    Call hCloseDocument
endcase 'i_us_presentation6

'-------------------------------------------------------------------------------
testcase i_us_presentation7

    dim sFilter as string
    dim sFileName as string
    dim PresentationFile6 as string
    dim PresentationFile7 as string
    PresentationFile6 = ConvertPath ((ConvertPath (gOfficePath + "user\work\PowerPes6.odp")))
    PresentationFile7 = ConvertPath ((ConvertPath (gOfficePath + "user\work\PowerPes7.odp")))
    sFileName = (ConvertPath (gOfficePath + "user\work\export-test.ppt"))

    if Dir(PresentationFile6) <> "" then 'if file exists...
        hFileOpen (PresentationFile6)
        WaitSlot (10000)
    else
        warnlog "   This test is supposed to run after the previous testcase has been run. Notify the Automatic-tester."
        printlog "New impress document"
        Call hNewDocument
        WaitSlot (2000)
    end if

    kontext "slides"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"
    SlidesControl.TypeKeys "<PAGEDOWN>"

    kontext "DocumentImpress"
    printlog " inserting Ending Slide"
    InsertSlide 'No 7
    DocumentImpress.TypeKeys "Ende"
    printlog "   Inserted ending -slide."

    Kontext "Gallery"
    if Gallery.Exists(2) then
        warnlog "   The Gallery was visible. Closed it. Check earlier ran tests for inconsistency."
        ToolsGallery
        WaitSlot (2000)
    end if

    kontext "slides"
    for i = 1 to 7
        sleep 1
        SlidesControl.TypeKeys "<PAGEUP>"
    next i
    SlidesControl.TypeKeys "<RETURN>" 'At the first slide

    hTypeKeys "<F5>"
    kontext "DocumentPresentation"
    sleep (5)
    DocumentPresentation.TypeKeys "<PAGEDOWN>"
    sleep (5)
    DocumentPresentation.TypeKeys "<PAGEDOWN>"
    sleep (5)
    DocumentPresentation.TypeKeys "<PAGEDOWN>"
    sleep (10)
    DocumentPresentation.TypeKeys "<PAGEDOWN>"
    sleep (10)
    DocumentPresentation.TypeKeys "<PAGEDOWN>"
    sleep (5)
    DocumentPresentation.TypeKeys "<PAGEDOWN>"
    sleep (5)
    DocumentPresentation.TypeKeys "<PAGEDOWN>"
    sleep (5)
    DocumentPresentation.TypeKeys "<PAGEDOWN>"

    if DocumentPresentation.Exists(5) then
        hTypeKeys "<ESC>"
        warnlog "   Presentation should have ended. Please inform Automatic Tester."
    end if
    kontext "DocumentImpress"

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile7)
    printlog "OK   saved at ", PresentationFile7
    sleep (1)

    printlog "Save as Powerpoint-file"
    FileSaveAs
        Kontext "SpeichernDlg"
        Dateiname.SetText sFileName
        Dateityp.Select 5  ' Powerpoint
        printlog "Trying to save with filter: " + Dateityp.GetSelText + sFilter(5)
        Speichern.Click
        Kontext "Messagebox"
        if Messagebox.Exists(2) then Messagebox.Yes
        Kontext "AlienWarning"
        if AlienWarning.Exists(2) then AlienWarning.OK
        printlog "Saved as: " + sFileName
        sleep (3)

    printlog "Close the office and reload the file"
    FileClose
    Kontext "Messagebox"
    if Messagebox.Exists(2) then Messagebox.Yes
    sleep (3)
    printlog "Open the saved file"
    hFileOpen sFileName
    printlog "File opened: " + sFileName
    sleep (3)

    printlog "Close the office-session"
    ActiveDeactivateCTLSupport (FALSE)
    sleep (2)
    printlog "Close Document"
    Call hCloseDocument
endcase 'i_us_presentation7
