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
'* short description : Include-file for impress User-Scenario: Presentation.
'*
'\********************************************************************

testcase i_us_presentation

    dim iPictures as integer 'variable for the number of the used picture gallery theme
    dim iAnimations as Integer 'variable used for the number of the used animation gallery theme
    dim iSize as integer 'step variable
    dim sFileName as string 'name for ppt export file
    dim iKeyStroke as integer 'counter variable for hitting space in running presentation
    dim PresentationFile as string 'variable for the file name


    PresentationFile = ConvertPath( gOfficePath + "user\work\PowerPres1.odp" )
    printlog "Setting up an impress document in several steps..."
    printlog "1. New impress document"
    Call hNewDocument
    Kontext "DocumentImpress"
    printlog "Selecting layout: Centered Text"
    DocumentImpress.UseMenu
    hMenuSelectNr (5)
    hMenuSelectNr (13)
    printlog "Changing focus to TaskPane."
    sleep 1
    Kontext "Tasks"
    printlog "to get to the very first position"
    LayoutsPreview.TypeKeys "<HOME>"
    printlog "'to get to the right position"
    LayoutsPreview.TypeKeys "<RIGHT>", 5
    LayoutsPreview.TypeKeys "<RETURN>"
    WaitSlot (2000)
    kontext "DocumentImpress"
    printlog "Add a second Master-Page "
    ViewMasterPage
    kontext "Slides"
    SlidesControl.OpenContextMenu(true)
    sleep (2)
    printlog "New master"
    MenuSelect(MenuGetItemID(1))
    sleep (1)
    printlog "Inserted second Master-Slide"

    printlog "set background to picture(Gallery)"
    Kontext "Gallery"

    if Gallery.Exists(2) then
        warnlog "The Gallery was already visible. Check earlier ran tests for inconsistency."
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
        printlog "Gallery-object correctly copied into Slide."
        ExportierenDlg.Close
        WaitSlot (2000)
    end if

    kontext "GraphicObjectbar"

    if GraphicObjectbar.Exists(5) = FALSE then
        kontext "DocumentImpress"
        ViewToolbarsPicture
    end if

    kontext "GraphicObjectbar"
    printlog "50% Transparency"
    WaitSlot (2000)
    kontext "GraphicObjectbar"
    Transparenz.SetText "50"
    sleep (1)
    printlog "Mosaic 16px x 16px"
    Filter.TearOff
    sleep (1)
    kontext "GraphicFilterBar"
    Mosaic.Click
    kontext "Mosaic"
    if ( Mosaic.exists( 2 ) ) then
        Width.SetText "16"
        Height.SetText "16"
        Mosaic.OK
    else
        warnlog "Dialog did not came up."
    endif
    kontext "GraphicFilterBar"
    printlog "Closing dialog and Gallery."
    GraphicFilterBar.Close
    ToolsGallery
    WaitSlot (1000)
    kontext "DocumentImpress"
    gMouseClick 50,50
    sleep (1)
    FormatPositionAndSize
    kontext
    if ( Active.exists( 2 ) ) then
        active.setPage(TabPositionAndSize)
        kontext "TabPositionAndSize"
        Width.SetText "28"
        Height.SetText "21"
        SizePosition.TypeKeys "<RIGHT><DOWN>"
        TabPositionAndSize.OK
    else
        warnlog( "Page <TabPositionAndSize> did not open" )
    endif

    printlog "Close Master"
    hUseAsyncSlot( "ViewNormal" )
    kontext "DocumentImpress"
    printlog "Inserting title on first slide."
    DocumentImpress.TypeKeys "<TAB>"
    DocumentImpress.TypeKeys "<RETURN>"
    DocumentImpress.TypeKeys "WELCOME!"
    DocumentImpress.TypeKeys "<ESCAPE>", 2
    sleep 1
    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile)
    ActiveDeactivateCTLSupport (FALSE)
    printlog "Close Document"
    hFileCloseAll()

    printlog "-------------------------------------------------------------------------------"

    printlog "2. Loading file again..."
    if ( FileExists(PresentationFile) ) then 'if file exists...
        hFileOpen (PresentationFile)
    else
        warnlog "Testdoc is missing or did not get saved."
        goto endsub
    end if
    sleep (2)
    printlog "New Slide"
    kontext "slides"
    SlidesControl.TypeKeys( "<PAGEDOWN>", 3 )
    SlidesControl.TypeKeys "<SHIFT F10>"   'OpenContextMenu(true)
    sleep (1)
    MenuSelect(MenuGetItemID(1)) 'New Slide  'No 2
    printlog "Inserted second normal Slide"
    printlog "Select 4th Layout: Title and 2 Content blocks"
    Kontext "Tasks"
    printlog "to get to the very first position"
    LayoutsPreview.TypeKeys "<HOME>"
    printlog "'to get to the right position"
    LayoutsPreview.TypeKeys "<RIGHT>", 3
    LayoutsPreview.TypeKeys "<RETURN>"
    hUseAsyncSlot( "ViewNormal" )
    printlog "Hide Slidepane (Oops! The user were too fast: accidently hide the pane)"
    kontext "Slides"
    SlidesControl.FadeOut
    WaitSlot (1000)
    printlog "Restore Pane."
    SlidesControl.FadeIn
    printlog "Did the 'mistake' to FadeIn/Out the Slidepane"
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
    if GrafikEinfuegenDlg.exists(5) then
    printlog "   The Insertgraphics-dialogue showed up correctly."
    sleep (5)
    else
    warnlog "   The Insertgraphics-dialogue didn't show up."
    endif

    printlog "Graphics-Import-dialogue. Select i_us_large.jpg"
    Kontext "GrafikEinfuegenDlg"

    if ( GrafikEinfuegenDlg.exists( 2 ) ) then
        Dateiname.SetText ConvertPath (gTesttoolPath + "graphics\required\input\i_us_large.jpg")
        Oeffnen.Click
        Kontext "DocumentImpress"
        printlog "Deselect graphic"
        DocumentImpress.TypeKeys "<ESCAPE>"
        printlog "Inserted Graphic into the second Slide"
        printlog "Change text on the two text-boxes"
        DocumentImpress.TypeKeys "<TAB>" 'First text.
        DocumentImpress.TypeKeys "<RETURN>" 'To get into edit-mode.
        DocumentImpress.TypeKeys "The World has just become easier now.."
        DocumentImpress.TypeKeys "<ESCAPE><ESCAPE>"
        DocumentImpress.TypeKeys "<TAB><TAB><TAB>"
        DocumentImpress.TypeKeys "<RETURN>"
        DocumentImpress.TypeKeys "100% Recyclable"
        DocumentImpress.TypeKeys "<RETURN>"
        DocumentImpress.TypeKeys "Very durable"
        DocumentImpress.TypeKeys "<RETURN>"
        DocumentImpress.TypeKeys "Priced lower!"
        DocumentImpress.TypeKeys "<RETURN>"
        DocumentImpress.TypeKeys "Astonishing!"
        DocumentImpress.TypeKeys "<RETURN>"
        DocumentImpress.TypeKeys "Energy-efficient"
        DocumentImpress.TypeKeys "<ESCAPE><ESCAPE>"
    else
        warnlog( "Failed to open <Insert Graphics> dialog" )
    endif

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile)
    printlog "Close Document"
    hFileCloseAll()

    printlog "-------------------------------------------------------------------------------"

    printlog "3. Loading file again.."
    if ( FileExists(PresentationFile) ) then 'if file exists...
        hFileOpen(PresentationFile)
    else
        warnlog "Testdoc is missing or did not get saved."
        goto endsub
    end if

    printlog "Insert New Slide, 3rd one"
    kontext "slides"
    SlidesControl.TypeKeys( "<PAGEDOWN>", 3 )
    kontext "DocumentImpress"
    InsertSlide 
    printlog "5. Layout. Title only"
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<HOME>"     'to get to the very first position
    LayoutsPreview.TypeKeys "<RIGHT>", 4 'to get to the right position
    LayoutsPreview.TypeKeys "<RETURN>"
    hUseAsyncSlot( "ViewNormal" )
    kontext "DocumentImpress"
    Call gMouseClick 50,50
    DocumentImpress.TypeKeys "<TAB><RETURN>"
    DocumentImpress.TypeKeys "A new form"
    DocumentImpress.TypeKeys "<ESCAPE>", 2
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
    printlog "Wrote Text, Inserted Graphic, and moved it in the third Slide"
    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile)
    printlog "Close Document"
    hFileCloseAll()

    printlog "-------------------------------------------------------------------------------"

    printlog "4. reopening file..."
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

    if ( FileExists(PresentationFile) ) then 'if file exists...
        hFileOpen( PresentationFile)
    else
        warnlog "Testdoc is missing or did not get saved."
        goto endsub
    end if

    kontext "slides"
    SlidesControl.TypeKeys( "<PAGEDOWN>", 5 )
    kontext "DocumentImpress"
    printlog "insert slide no 4"
    hUseAsyncSlot( "InsertSlide" )
    printlog "Background: picture (Gallery)"
    Kontext "Gallery"

    if ( Not Gallery.Exists() ) then
        ToolsGallery
    end if

    kontext "Gallery"

    if ( Gallery.exists( 2 ) ) then
        Gallerys.Select (iAnimations)

        kontext "Gallery"
        View.TypeKeys "<HOME><RIGHT><RIGHT>"
        sleep 1
        View.TypeKeys "<SHIFT F10>" 'OpenContextMenu
        sleep 1
        MenuSelect(MenuGetItemID(1))   'Insert
        sleep 1
        MenuSelect(MenuGetItemID(1))   'Copy
    else
        warnlog( "Could not access Gallery" )
    endif

    sleep 1
    kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<UP>", 75
    DocumentImpress.TypeKeys "<LEFT>", 100
    printlog "Deselect graphic"
    DocumentImpress.MouseDoubleClick 90,90
    printlog "Close the Gallery"
    ToolsGallery
    printlog "Change Text on slide"
    DocumentImpress.TypeKeys "<TAB>"
    DocumentImpress.TypeKeys "<RETURN>"
    DocumentImpress.TypeKeys "The process starts here.."
    DocumentImpress.TypeKeys "<ESCAPE><ESCAPE>"
    gMouseClick 50,50
    printlog( "Decrease..." )

    for iSize = 80 to 20 step -20
        CreateTextSetEffectAndAngle
        DocumentImpress.TypeKeys "<DOWN>", 70
        DocumentImpress.TypeKeys "<LEFT>", iSize
        gMouseClick 90,90
    next iSize

    printlog( "Increase..." )

    for iSize = 20 to 80 step 20
        CreateTextSetEffectAndAngle
        DocumentImpress.TypeKeys "<DOWN>", 70
        DocumentImpress.TypeKeys "<RIGHT>", iSize
        gMouseClick 90,90
    next iSize

    printlog "Inserted fourth slide with Gallery-object."
    printlog "Switching back task pane to default..."
    Kontext "DocumentImpress"
    DocumentImpress.UseMenu
    hMenuSelectNr (5)
    hMenuSelectNr (13)
    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile)
    printlog "Close Document"
    hFileCloseAll()

    printlog "-------------------------------------------------------------------------------"

    printlog "5. reloading file..."
    if ( FileExists(PresentationFile) ) then
        hFileOpen(PresentationFile)
    else
        warnlog "Testdoc is missing or did not get saved."
        goto endsub
    end if

    kontext "slides"
    SlidesControl.TypeKeys( "<PAGEDOWN>", 7 )
    kontext "DocumentImpress"
    printlog "New Slide. (Insert Menu) (Duplicate slide)"
    InsertDuplicateSlide 
    printlog "Change the text in some way. (the user is making a joke with the audience)"
    gMouseClick 90,90
    DocumentImpress.TypeKeys "<TAB>"
    DocumentImpress.TypeKeys "<RETURN>"
    hUseAsyncSlot( "EditSelectAll" )
    DocumentImpress.TypeKeys "And does it with strength..."
    printlog "Inserted fifth slide with audience-joke."
    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile)
    printlog "Close Document"
    hFileCloseAll()

    printlog "-------------------------------------------------------------------------------"

    printlog "6. reloading file..."
    if ( FileExists(PresentationFile) ) then 'if file exists...
        hFileOpen(PresentationFile)
    else
        warnlog "Testdoc is missing or did not get saved."
        goto endsub
    end if

    kontext "slides"
    SlidesControl.TypeKeys( "<PAGEDOWN>", 6 )
    kontext "DocumentImpress"
    printlog "Q&A Slide"
    InsertSlide
    WaitSlot (1000)
    kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<TAB>"
    DocumentImpress.TypeKeys "<RETURN>"
    DocumentImpress.TypeKeys "Q&A"
    DocumentImpress.TypeKeys "<SHIFT HOME>"
    wait( 500 )
    Kontext "TextObjectbar"

    if ( not TextObjectbar.Exists() ) then
        ViewToolbarsTextFormatting
    end if

    Kontext "TextObjectbar"
    wait( 500 )
    Printlog "Change size of font"
    Schriftgroesse.Select "26"
    Schriftgroesse.TypeKeys "<RETURN>"
    Fett.Click
    kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<ESCAPE><ESCAPE>"
    kontext "Toolbar"
    Auswahl.Click
    gMouseClick 60,60
    hUseAsyncSlot( "EditSelectAll" )
    DocumentImpress.TypeKeys "<DOWN>", 50
    kontext "DocumentImpress"
    printlog "Inserted sixth slide with Q&A."
    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile)
    printlog "Close Document"
    hFileCloseAll()

    printlog "-------------------------------------------------------------------------------"

    printlog "7. reloading file..."
    sFileName = ConvertPath( gOfficePath + "user\work\export-test.ppt" )

    if ( FileExists(PresentationFile) ) then 'if file exists...
        hFileOpen(PresentationFile)
    else
        warnlog "Testdoc is missing or did not get saved."
        goto endsub
    end if

    kontext "slides"
    SlidesControl.TypeKeys( "<PAGEDOWN>",7)
    kontext "DocumentImpress"
    printlog "inserting Ending Slide"
    InsertSlide 'No 7
    DocumentImpress.TypeKeys "End"
    printlog "Inserted ending -slide."
    Kontext "Gallery"

    if Gallery.Exists(2) then
        warnlog "The Gallery was visible. Closed it. Check earlier ran tests for inconsistency."
        ToolsGallery
        WaitSlot (2000)
    end if

    kontext "slides"

    for i = 1 to 8
        sleep 1
        SlidesControl.TypeKeys "<PAGEUP>"
    next i

    hTypeKeys "<F5>"
    kontext "DocumentPresentation"

    for iKeyStroke = 1 to 8
        wait( 3000 )
        DocumentPresentation.TypeKeys "<PAGEDOWN>"
        wait( 2000 )
    next iKeyStroke

    kontext "DocumentPresentation"

    if ( DocumentPresentation.notExists( 5 ) ) then
        printlog( "Presentation closed. Good." )
    else
        warnlog "   Presentation not finished, fullscreen view still open."
        DocumentPresentation.typeKeys "<ESC>"

        kontext "DocumentPresentation"
        if ( DocumentPresentation.notExists( 3 ) ) then
            printlog( "Recovery attempt successful" )
        else
            goto endsub
        endif
    end if

    kontext "DocumentImpress"
    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile)
    printlog( "Save as Powerpoint-file (Using filter at pos. 5 in the filter list)" )
    FileSaveAs
    Kontext "SpeichernDlg"

    if ( SpeichernDlg.exists( 2 ) ) then
        Dateiname.SetText sFileName
        Dateityp.Select 5  ' Powerpoint (possibly)
        printlog "Trying to save with filter: " + Dateityp.GetSelText
        Speichern.Click

        Kontext "Messagebox"
        hCloseDialog( Messagebox, "Yes, optional" )

        Kontext "AlienWarning"
        hCloseDialog( AlienWarning, "ok, optional" )
    else
        warnlog( "<FileSaveAs> dialog did not open" )
    endif

    printlog "Close all open documents"
    hFileCloseAll()
    printlog( "Reload file: " & sFileName )
    hFileOpen sFileName
    sleep( 3 )
    printlog "Close the office-session"
    printlog "Close Documents"
    hFileCloseAll()

endcase 'i_us_presentation