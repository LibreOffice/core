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
'\********************************************************************

testcase i_us_presentation1

    dim iPictures as integer
    dim PresentationFile1 as string

    PresentationFile1 = ConvertPath( gOfficePath + "user\work\PowerPes1.odp" )

    printlog "New impress document"
    Call hNewDocument
    WaitSlot (2000)
    kontext "DocumentImpress"

    printlog "Add second Master-Page"
    ViewMasterPage

    kontext "Slides"
    printlog "   Insert second Master-Slide"
    SlidesControl.OpenContextMenu(true)
    sleep (2)
    MenuSelect(MenuGetItemID(1)) 'New Master

    printlog "Background: picture (Gallery)"
    Kontext "Gallery"
    if Gallery.Exists(2) then
        warnlog "   The Gallery was already visible. Check earlier ran tests for inconsistency."
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

    Kontext "ExportierenDlg"
    if ( ExportierenDlg.exists( 5 ) ) then
        printlog "   Gallery-object correctly copied into Slide."
        ExportierenDlg.Close
    else
        warnlog "   Doesn't seem like we copied anything from the Gallery... ?"
    end if

    kontext "GraphicObjectbar"
    if ( not GraphicObjectbar.Exists( 5 ) ) then
        kontext "DocumentImpress"
        ViewToolbarsPicture
        WaitSlot( 2000 )
    end if

    printlog "50% Transparency"
    kontext "GraphicObjectbar" ' the one with Transparency
    Transparenz.SetText "50"

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
        warnlog( "Dialog <Mosaic> did not open" )
    endif

    kontext "GraphicFilterBar"
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
        Width.SetText "800"
        Height.SetText "600"
        SizePosition.TypeKeys "<RIGHT><DOWN>"
        TabPositionAndSize.OK
    else
        warnlog( "Page <TabPositionAndSize> did not open" )
    endif

    printlog "Close Master"
    hUseAsyncSlot( "ViewNormal" )

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile1)

    ActiveDeactivateCTLSupport (FALSE)

    printlog "Close Document"
    hFileCloseAll()
    
endcase 'i_us_presentation1

'-------------------------------------------------------------------------------

testcase i_us_presentation2

    dim PresentationFile1 as string
    dim PresentationFile2 as string

    PresentationFile1 = ConvertPath( gOfficePath + "user\work\PowerPes1.odp" )
    PresentationFile2 = ConvertPath( gOfficePath + "user\work\PowerPes2.odp" )

    if ( FileExists( PresentationFile1 ) ) then 'if file exists...
        hFileOpen (PresentationFile1)
    else
        warnlog "   This test is supposed to run after the previous testcase has been run. Notify the Automatic-tester."
        goto endsub
    end if

    printlog "New Slide"
    kontext "slides"
    SlidesControl.TypeKeys( "<PAGEDOWN>", 3 )
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

    hUseAsyncSlot( "ViewNormal" )

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
    if ( GrafikEinfuegenDlg.exists( 2 ) ) then

        Dateiname.SetText ConvertPath (gTesttoolPath + "graphics\required\input\i_us_large.jpg")
        Oeffnen.Click

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
    else
        warnlog( "Failed to open <Insert Graphics> dialog" )
    endif

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile2)

    ActiveDeactivateCTLSupport (FALSE)

    printlog "Close Document"
    hFileCloseAll()

endcase 'i_us_presentation2

'-------------------------------------------------------------------------------

testcase i_us_presentation3

    dim PresentationFile2 as string
    dim PresentationFile3 as string

    PresentationFile2 = ConvertPath( gOfficePath + "user\work\PowerPes2.odp" )
    PresentationFile3 = ConvertPath( gOfficePath + "user\work\PowerPes3.odp" )

    if ( FileExists( PresentationFile2 ) ) then 'if file exists...
        hFileOpen( PresentationFile2 )
    else
        warnlog "   This test is supposed to run after the previous testcase has been run. Notify the Automatic-tester."
        goto endsub
    end if

    printlog "Insert New Slide"
    kontext "slides"
    SlidesControl.TypeKeys( "<PAGEDOWN>", 3 )

    kontext "DocumentImpress"
    InsertSlide 'No 3

    printlog "5. Layout. Clip/Text"
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<HOME>"     'to get to the very first position
    LayoutsPreview.TypeKeys "<RIGHT>", 4 'to get to the right position
    LayoutsPreview.TypeKeys "<RETURN>"

    hUseAsyncSlot( "ViewNormal" )

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

    ActiveDeactivateCTLSupport (FALSE)

    printlog "Close Document"
    hFileCloseAll()
    
endcase 'i_us_presentation3

'-------------------------------------------------------------------------------

testcase i_us_presentation4

    dim PresentationFile3 as string
    dim PresentationFile4 as string
    dim iAnimations as Integer
    dim iSize as integer

    PresentationFile3 = ConvertPath( gOfficePath + "user\work\PowerPes3.odp" )
    PresentationFile4 = ConvertPath( gOfficePath + "user\work\PowerPes4.odp" )

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

    if ( FileExists( PresentationFile3 ) ) then 'if file exists...
        hFileOpen( PresentationFile3 )
    else
        warnlog "   This test is supposed to run after the previous testcase has been run. Notify the Automatic-tester."
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
        wait( 200 )
        View.TypeKeys "<SHIFT F10>" 'OpenContextMenu
        wait( 200 )
        MenuSelect(MenuGetItemID(1))   'Insert
        wait( 200 )
        MenuSelect(MenuGetItemID(1))   'Copy
    else
        warnlog( "Could not access Gallery" )
    endif

    printlog "Check that we really got a copy of the object"
    kontext "DocumentImpress"
    DocumentImpress.OpenContextMenu(true)
    WaitSlot (1000)
    MenuSelect 27353

    Kontext "ExportierenDlg"
    if ( ExportierenDlg.exists( 5 ) ) then
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

    printlog( "Decrease..." )
    for iSize = 100 to 25 step -25
        CreateTextSetEffectAndAngle
        DocumentImpress.TypeKeys "<DOWN>", 80
        DocumentImpress.TypeKeys "<LEFT>", iSize
        gMouseClick 90,90
    next iSize

    printlog( "Increase..." )
    for iSize = 25 to 100 step 25
        CreateTextSetEffectAndAngle
        DocumentImpress.TypeKeys "<DOWN>", 80
        DocumentImpress.TypeKeys "<LEFT>", iSize
        gMouseClick 90,90
    next iSize

    printlog "   Inserted fourth slide with Gallery-object."

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile4)

    ActiveDeactivateCTLSupport (FALSE)

    printlog "Close Document"
    hFileCloseAll()
    
endcase 'i_us_presentation4

'-------------------------------------------------------------------------------

testcase i_us_presentation5

    dim PresentationFile4 as string
    dim PresentationFile5 as string

    PresentationFile4 = ConvertPath( gOfficePath + "user\work\PowerPes4.odp" )
    PresentationFile5 = ConvertPath( gOfficePath + "user\work\PowerPes5.odp" )

    if ( FileExists( PresentationFile4 ) ) then 'if file exists...
        hFileOpen( PresentationFile4 )
    else
        warnlog "   This test is supposed to run after the previous testcase has been run. Notify the Automatic-tester."
        goto endsub
    end if

    kontext "slides"
    SlidesControl.TypeKeys( "<PAGEDOWN>", 7 )

    kontext "DocumentImpress"
    printlog "8. New Slide. (Insert Menu) (Duplicate slide)"
    InsertDuplicateSlide 'No 5
    printlog "Change the text in some way. (the user is making a joke with the audience)"
    gMouseClick 90,90
    DocumentImpress.TypeKeys "<TAB>"
    DocumentImpress.TypeKeys "<RETURN>"
    hUseAsyncSlot( "EditSelectAll" )
    DocumentImpress.TypeKeys "And does it with strength..."

    printlog "   Inserted fifth slide with audience-joke."

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile5)

    ActiveDeactivateCTLSupport (FALSE)

    printlog "Close Document"
    hFileCloseAll()

endcase 'i_us_presentation5

'-------------------------------------------------------------------------------

testcase i_us_presentation6

    dim PresentationFile5 as string
    dim PresentationFile6 as string

    PresentationFile5 = ConvertPath( gOfficePath + "user\work\PowerPes5.odp" )
    PresentationFile6 = ConvertPath( gOfficePath + "user\work\PowerPes6.odp" )

    if ( FileExists( PresentationFile5 ) ) then 'if file exists...
        hFileOpen( PresentationFile5 )
    else
        warnlog "   This test is supposed to run after the previous testcase has been run. Notify the Automatic-tester."
        goto endsub
    end if

    kontext "slides"
    SlidesControl.TypeKeys( "<PAGEDOWN>", 6 )

    kontext "DocumentImpress"
    printlog "9. Q&A Slide"
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
    Printlog "- Change size of font"
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
    printlog "   Inserted sixth slide with Q&A."

    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile6)

    ActiveDeactivateCTLSupport (FALSE)

    printlog "Close Document"
    hFileCloseAll()

endcase 'i_us_presentation6

'-------------------------------------------------------------------------------

testcase i_us_presentation7

    const KEY_STROKE_REPEAT = 8

    dim sFilter as string
    dim sFileName as string
    dim PresentationFile6 as string
    dim PresentationFile7 as string
    dim iKeyStroke as integer

    PresentationFile6 = ConvertPath( gOfficePath + "user\work\PowerPes6.odp" )
    PresentationFile7 = ConvertPath( gOfficePath + "user\work\PowerPes7.odp" )
    sFileName         = ConvertPath( gOfficePath + "user\work\export-test.ppt" )

    if ( FileExists( PresentationFile6 ) ) then 'if file exists...
        hFileOpen( PresentationFile6 )
    else
        warnlog "   This test is supposed to run after the previous testcase has been run. Notify the Automatic-tester."
        goto endsub
    end if

    kontext "slides"
    SlidesControl.TypeKeys( "<PAGEDOWN>", KEY_STROKE_REPEAT )

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
    for iKeyStroke = 1 to KEY_STROKE_REPEAT
        wait( 3000 )
        DocumentPresentation.TypeKeys "<PAGEDOWN>"
        wait( 2000 )
    next iKeyStroke

    kontext "DocumentPresentation"
    if ( DocumentPresentation.notExists( 5 ) then
        printlog( "Presentation closed. Good." )
    else
        warnlog "   Presentation not finished, fullscreen view still open."
        DocumentPresentation.typeKeys "<ESC>"
    end if

    kontext "DocumentImpress"
    printlog "Save Document"
    call hFileSaveAsKill (PresentationFile7)

    printlog( "Save as Powerpoint-file (Using filter at pos. 5 in the filter list)" )
    FileSaveAs

    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then

        Dateiname.SetText sFileName
        Dateityp.Select 5  ' Powerpoint (possibly)
        printlog "Trying to save with filter: " + Dateityp.GetSelText + sFilter(5)
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
    ActiveDeactivateCTLSupport (FALSE)

    printlog "Close Documents"
    hFileCloseAll()
    
endcase 'i_us_presentation7
