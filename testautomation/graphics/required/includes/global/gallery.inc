'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: gallery.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:43:00 $
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
'* owner : wolfram.garten@sun.com
'*
'* short description : Global Required/resource test: Checking the gallery
'*
'*******************************************************************
'*
' #1   tGallery_DialogTest                           ' Open gallery, pin - unpin, dock - undock, close gallery
' #1   tGallery_ActivateAndUpdateAllThemes           ' Activate all gallery-themes and update this theme
' #1   tGallery_CheckNames                           ' Check the names for all themes depends on language
' #1   tGallery_CheckContextMenuForGalleryThemes     ' Test the context menu on themes window
' #1   tGallery_CreateAndWorkWithANewGalleryThemes   ' Create a new theme, add graphics and work with it
' #1   tGallery_GalleryView_Preview                  ' Preview-feature in Gallery Beamer-window
' #1   tGallery_GalleryView_Insert                   ' Insert gallery objects in all applications
' #1   tGallery_GalleryView_Delete                   ' Delete gallery objects out of Beamer window
' #0   hOpenGallery                                  ' Open the gallery
' #1   tGallery_Quick_check
'*
'\******************************************************************
testcase tGallery_DialogTest
    '///Open a new Writer document
    '///Tools / Gallery
    printlog "- Working with Gallery-Beamer!"
    gApplication = "Writer"
    call hNewDocument
    call hOpenGallery
    Kontext "DocumentWriter"
        '///+Undock the <i>Gallery Beamer</i>
        printlog "- undock the Gallery Beamer"
        Kontext "Gallery"
            Gallery.Undock ( AlignTop )
            sleep(2)
            '///+<ul><li>Move the gallery-window</li></ul>
            printlog "  - move the gallery window"
            Gallery.move ( 20, 20 )
            sleep(2)
            '///+Dock the <i>Gallery Beamer</i>
            printlog "- dock the Gallery Beamer"
            Gallery.Dock ( AlignTop )
            sleep(2)
            '///+Close the <i>Gallery Beamer</i> (Tools / Gallery)
            printlog "- close the Gallery Beamer"
        ToolsGallery
    call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tGallery_ActivateAndUpdateAllThemes
    Dim iThemeCount as Integer
    Dim i as Integer 
    Dim j as Integer
    Dim Gallerytext as string
    '///Activate and update all gallery themes (NET installation: Activate only!)
    '///Open a new Writer document
    '///+Tools / Gallery
    '///+Click on each theme and update it (via context menu)
    printlog "activate and update all gallery-themes"
    gApplication = "Writer"
    call hNewDocument
    Kontext
    call hOpenGallery
    Kontext "Gallery"
        iThemeCount = Gallerys.GetItemCount
        for i=1 to iThemeCount
            if gNetzInst = FALSE then
                Kontext "Gallery"
                    Gallerytext = Gallerys.GetItemText(i)
                    printlog "- " + i + ". entry (" & Gallerytext & ")"
                    Gallerys.Select i
                    Gallerys.MouseMove ( 10, 10 )
                    Gallerys.OpenContextMenu
                    sleep (3)
                    printlog "  - update"
                    hMenuSelectNr (1)
                    sleep (3)
                    Kontext "Messagebox"
                    if Messagebox.Exists(3) then
                        if Messagebox.getRT=304 then
                            printlog "- 'MyTheme' is the " & i & "'s entry."
                            Messagebox.No
                        end if
                    else
                        for j = 1 to 800
                            Kontext "AktualisierenGallery"
                                if AktualisierenGallery.Exists then
                                    printlog "DEBUG (j): " & j
                                    sleep (1)
                                else
                                    printlog "DEBUG: 800 reached!"
                                    j=801
                                end if
                        next j
                    end if
            end if
        next i
    call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tGallery_CheckNames
    Dim iThemeCount as Integer
    Dim i as Integer
    Dim j as Integer
    Dim ssList (100) as String
    Dim siList (100) as String
    Dim sFileName as String
    Dim iGalleryThemes as integer

    if gOOO = true then 
        sFileName = ConvertPath ( gTesttoolPath + "graphics\required\input\gallery\gal_oo_" + iSprache + ".txt" )
    else
        sFileName = ConvertPath ( gTesttoolPath + "graphics\required\input\gallery\gal_" + iSprache + ".txt" )
    end if

    '///Check the names for the gallery themes
    '///Open a new Writer document
    '///+Tools / Gallery
    gApplication = "Writer"
    call hNewDocument
    Kontext
    call hOpenGallery
    '///+Check the number of gallery themes. For StarOffice: should be 32. For OpenOffice.Org: should be 6.
    printlog "- check the number of gallery-themes"
    Kontext "Gallery"
        if NOT gOOO then 
            if bAsianLan then
                iGalleryThemes = 29 ' Flags are not allowed!
            else
                iGalleryThemes = 30
            end if
        else
            if bAsianLan then
                iGalleryThemes = 6  ' Flags are not allowed!
            else
                iGalleryThemes = 6
            end if
        end if
        iThemeCount = Gallerys.GetItemCount
        if (iThemeCount <> iGalleryThemes) then
            warnlog "Difference in count of gallery themes; found: '" + iThemeCount + "'; expected: '"+ iGalleryThemes +"'"
        end if
        for i=1 to iThemeCount
        Gallerys.Select i
        ListAppend ( siList(), Gallerys.GetSelText )
    next i
    '///+Check all names with a list which depends on language (<i>gTestToolPath</i>/graphics/udpate/input/gallery)
    printlog "- check the names of gallery-themes"
    if Dir ( sFilename ) = "" then
        warnlog "The file for comparison does not exists. The file will be written!"
        warnlog "Please control : " + sFilename
        ListWrite ( siList(), sFilename, "utf8" )
    else
        printlog "  file for comparison is : " + sFilename
        ListRead ( ssList (), sFilename, "uft8" )
        gCompare2Lists ( siList(), ssList () )
    end if
    ToolsGallery
    call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tGallery_CheckContextMenuForGalleryThemes
    Dim iForBidden as Integer
    Dim iMenuEntries as Integer
    Dim i as Integer
    Dim iThemeCount as Integer
    '/// Check the contextmenu for the gallery themes
    '/// Open a new Writer document
    '/// +Tools / Gallery
    printlog "check the contextmenu for the gallery-themes"
    gApplication = "Writer"
    call hNewDocument
    call hOpenGallery
    '/// +Testing <i>rename</i> and <i>properties</i> for each gallery theme (via context-menu)
    '/// +<ul><li>Only the private gallery theme must be deleteable (different 3 entries in the context-menu than the rest)</li></ul>
    Kontext "Gallery"
        iThemeCount = Gallerys.GetItemCount
        Gallerys.MouseMove ( 10, 10 )
        for i=1 to iThemeCount
            Kontext "Gallery"
                printlog "- " + i + ". entry"
                sleep 1
                Gallerys.Select 1
                sleep (1)
                Gallerys.Select i
                sleep (1)
                Gallerys.OpenContextMenu
                sleep (2)
                iMenuEntries = hMenuItemGetCount
                if iMenuEntries <> 3 then
                    if gNetzInst = FALSE then
                        warnlog "There are not 3 entries! => no test on the contextmenu!"
                    else
                        if iMenuEntries <> 1 then
                            warnlog "Net-Inst : There are not 1 entries! => no test on the contextmenu!"
                        else
                            sleep (3)
                            hMenuSelectNr(1)
                            Kontext
                            if Active.GetPageCount <> 1 then
                                warnlog "There are more than 1 Tabpage in Net-Installation ( perhaps no root-installation )!"
                            end if
                            Active.SetPage TabAllgemeinGallery
                            Kontext "TabAllgemeinGallery"
                                TabAllgemeinGallery.Cancel
                                sleep (1)
                        end if
                    end if
                else
                    sleep (1)
                    hMenuSelectNr(2)
                    Kontext "GalleryNewTitle"
                        GalleryNewTitle.Cancel
                        sleep(1)
                        Kontext "Gallery"
                            Gallerys.OpenContextMenu
                            sleep (3)
                            hMenuSelectNr (3)
                            Kontext
                            Active.SetPage TabAllgemeinGallery
                            Active.SetPage TabDateien
                            Kontext "TabDateien"
                                TabDateien.Cancel
                                sleep(1)
                end if
        next i
        ToolsGallery
    call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tGallery_CreateAndWorkWithANewGalleryThemes
    Dim iMenuEntries as Integer
    Dim j as Integer
    '/// Open a new Writer document
    '/// Tools / Gallery
    printlog "create a new gallery-theme ( TT-theme )"
    gApplication = "Writer"
    call hNewDocument
    call hOpenGallery
    iMenuEntries = Gallerys.GetItemCount
    printlog "- Clicking on 'New Theme' and creating a new theme"
    '/// Create a new gallery-theme
    '/// + by clicking on <i>New Theme</i>
    NewTheme.Click
    '/// +Activate <i>General</i> tabpage and insert <b><i>TT-theme</b></i> as name
    Kontext
    Active.SetPage TabAllgemeinGallery
    Kontext "TabAllgemeinGallery"
        NeuesThema.SetText "TT-theme"
        sleep (1)
        printlog "  - named the new theme 'TT-theme' on the general-page"
        printlog "  - insert all files out of '[gTestToolpath]\global\input\graf_inp' in the files-page"
        '/// +Activate <i>Files</i> tabpage
        Kontext
        Active.SetPage TabDateien
        Kontext "TabDateien"
            printlog "     - click 'add' without a selected file => insert-graphic-dialog has to be be visible"
            '/// +Click on <i>Add</i> (If no file is selected the <i>Graphic-Insert</i> dialog has to be be opened -> close it)
            Hinzufuegen.Click
            sleep (1)
            Kontext "GrafikEinfuegenDlg"
                GrafikEinfuegenDlg.Cancel
                sleep (1)
                printlog "     - click 'find files' to insert the path for graphics"
                '///+Click on <i>Find Files</i> => <i>select-path</i> dialog will be visible
        Kontext "TabDateien"
            Suche.Click
            sleep (1)
            '/// +Insert [gTesttoolPath]/input/global/graf_inp as path-name as click on <i>Select</i>
        Kontext "OeffnenDlg"
            Pfad.SetText ( convertPath ( gTesttoolPath + "global\input\graf_inp" ) )
            sleep (1)
            Auswaehlen.Click
            sleep (10)
            '/// +Click on <i>Add</i> for some graphics
            printlog "     - 'add' one by one"
        Kontext "TabDateien"
        DateiListe.Select 1
        WaitSlot(1000)
        Hinzufuegen.Click
        sleep (1)
        DateiListe.Select 1
        Hinzufuegen.Click
        sleep (1)
        '/// +Click on <i>Add all</i> for rest of the graphics
        '/// +<ul><li>apply-dialog -> cancel</li></ul>
        '/// +click on <i>Add all</i> for rest of the graphics
        printlog "     - 'add all' for the rest of the files"
        DateiListe.Select 1
        HinzufuegenAlle.Click
        printlog "     - apply-dialog -> cancel"
        Kontext "ApplyGallery"
            for j=1 to 100
                if ApplyGallery.Exists then
                    try
                        ApplyGallery.Cancel
                    catch
                    endcatch
                else
                    if j>5 then j=101
                        sleep (1)
                    end if
                next j
        Kontext "TabDateien"
        if DateiListe.GetItemCount > 1 then
            printlog "     - 'add all' for the rest of the files"
            DateiListe.Select 1
            HinzufuegenAlle.Click
            sleep (2)
            Kontext "ApplyGallery"
                for j=1 to 100
                    If ApplyGallery.Exists then
                        sleep (1)
                    else
                        j=101
                    end if
                next j
        end if
        Kontext "TabDateien"
            printlog "  - click 'OK' for the tabpages"
            TabDateien.OK
            '/// +Check if the new entry is inserted and select it
            printlog "  - check if the new entrie is inserted and select it"
        Kontext "Gallery"
            if Gallerys.GetItemCount <> ( iMenuEntries + 1 ) then
                warnlog "No new theme was created => the test ends here"
                ToolsGallery
                call hCloseDocument
            else
                gMouseClick 50,50            '/// make mouseclick to set focus ///'
                Kontext "Gallery"
                Gallerys.Select "TT-theme"
                Gallerys.MouseMove ( 10, 10 )
                sleep (1)
                Gallerys.OpenContextMenu
                '/// +Update the entry (1. entry in context menu ) => cancel it
                printlog "  - update the entry => cancel the update"
                sleep (3)
                hMenuSelectNr (1)
                sleep (1)
                Kontext "AktualisierenGallery"
                    if AktualisierenGallery.Exists <> TRUE then
                        AktualisierenGallery.Cancel
                    end if
                    do
                        sleep 1
                    loop while AktualisierenGallery.Exists
                    Kontext "Gallery"
                        Gallerys.Select "TT-theme"
                        Gallerys.OpenContextMenu
                        '/// +Update it (1. entry in context menu )
                        printlog "  - update"
                        sleep (3)
                        hMenuSelectNr (1)
                        for j=1 to 100
                            Kontext "AktualisierenGallery"
                            if AktualisierenGallery.Exists <> TRUE then
                                j=101
                            else
                                sleep (1)
                            end if
                        next j
                    '/// +Rename it (3. entry in context menu)
                    printlog "  - rename ( 3. entry in context-menu )"
                    Kontext "Gallery"
                        Gallerys.OpenContextMenu
                        sleep (3)
                        hMenuSelectNr (3)
                        Kontext "GalleryNewTitle"
                            Title.SetText "New TT-theme"
                            GalleryNewTitle.OK
                    Kontext "Gallery"
                        Gallerys.Select "New TT-theme"
                        '///+Add new files in properties (4. entry in context menu)
                        printlog "  - add new graphics in properties ( 4. entry in context-menu )"
                    Kontext "Gallery"
                        Gallerys.OpenContextMenu
                        sleep (3)
                        hMenuSelectNr (4)
                        Kontext
                        Active.SetPage TabDateien
                        Kontext "TabDateien"
                            Suche.Click
                            Kontext "OeffnenDlg"
                                Pfad.SetText ( convertPath ( gTesttoolPath + "global\input\graf_inp" ) )
                                Auswaehlen.Click
                                sleep (10)
                        Kontext "TabDateien"
                            DateiListe.Select 1
                            Hinzufuegen.Click
                            TabDateien.OK
                            '/// +Delete the entry (2. entry in context menu)
                            printlog "  - delete the entry ( 2. entry in context-menu )"
                            printlog "     - messagebox -> no"
                    Kontext "Gallery"
                        Gallerys.Select "New TT-theme"
                        sleep (3)
                        Gallerys.OpenContextMenu
                        sleep (3)
                        hMenuSelectNr (2)
                        Kontext "Active"
                            sleep (1)
                            Active.No
                            printlog "     - messagebox -> yes"
                    Kontext "Gallery"
                        sleep (1)
                        Gallerys.Select "New TT-theme"
                        sleep (1)
                        Gallerys.OpenContextMenu
                        sleep (3)
                        hMenuSelectNr (2)
                        Kontext "Active"
                            Active.Yes
                    try
                        Kontext "Gallery"
                        Gallerys.Select "New TT-theme"
                        warnlog "The entry isn't deleted!"
                    catch
                    endcatch
                end if
            sleep 10
        ToolsGallery
    call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tGallery_GalleryView_Preview
    Dim jpeg_bkg as Integer
    Dim iAnimation as Integer
    Dim iSound as Integer
    Dim i as Integer
    Dim iPreview as Integer
    Dim iTitle as Integer
    if NOT gOOO then
        select case iSprache
        case 01     :   jpeg_bkg =  3    : iAnimation =  1   :   iSound = 28
        case 07     :   jpeg_bkg = 29   : iAnimation =  1   :   iSound =  6
        case 31     :   jpeg_bkg =  3    : iAnimation =  3   :   iSound = 11
        case 33     :   jpeg_bkg = 13   : iAnimation =  1   :   iSound = 29
        case 34     :   jpeg_bkg = 11   : iAnimation =  1   :   iSound = 28
        case 36     :   jpeg_bkg = 12   : iAnimation =  1   :   iSound = 10
        case 39     :   jpeg_bkg = 10   : iAnimation =  1   :   iSound = 27
        case 46     :   jpeg_bkg =  2    : iAnimation =  1   :   iSound = 17
        case 48     :   jpeg_bkg =  2    : iAnimation =  1   :   iSound = 17
        case 49     :   jpeg_bkg = 12   : iAnimation =  1   :   iSound = 16
        case 55     :   jpeg_bkg = 21   : iAnimation =  1   :   iSound = 28
        case 81     :   jpeg_bkg = 21   : iAnimation =  1   :   iSound = 10  'FHA TODO: Find out the right numbers for Asian languages.
        case 82     :   jpeg_bkg =  1    : iAnimation = 17  :   iSound = 12
        case 86     :   jpeg_bkg =  1    : iAnimation =  9   :   iSound = 13
        case 88     :   jpeg_bkg =  1    : iAnimation =  7   :   iSound = 20
        case else :   jpeg_bkg = 10   : iAnimation =  1    :   iSound = 17
                  warnlog "Please insert the entrienumbers for 'Backgrounds', 'Sounds' and one with normal files ( Animations )"
        end select
    else  ' Testing OOO
        select case iSprache
            case 01     :   jpeg_bkg =  1   :   iAnimation =  5     :   iSound =  2
            case 07     :   jpeg_bkg =  1   :   iAnimation =  5     :   iSound =  2
            case 31     :   jpeg_bkg =  3   :   iAnimation =  1     :   iSound =  1
            case 33     :   jpeg_bkg =  3   :   iAnimation =  1     :   iSound =  2
            case 34     :   jpeg_bkg =  1   :   iAnimation =  1     :   iSound =  2
            case 36     :   jpeg_bkg =  2   :   iAnimation =  1     :   iSound =  2
            case 39     :   jpeg_bkg =  2   :   iAnimation =  1     :   iSound =  2
            case 46     :   jpeg_bkg =  2   :   iAnimation =  5     :   iSound =  2
            case 48     :   jpeg_bkg =  2   :   iAnimation =  5     :   iSound =  2
            case 49     :   jpeg_bkg =  2   :   iAnimation =  5     :   iSound =  2
            case 55     :   jpeg_bkg =  2   :   iAnimation =  5     :   iSound =  2
            case 81     :   jpeg_bkg =  2   :   iAnimation =  5     :   iSound =  2  'FHA TODO: Find out the right numbers for Asian languages.
            case 82     :   jpeg_bkg =  1   :   iAnimation =  5     :   iSound =  2
            case 86     :   jpeg_bkg =  1   :   iAnimation =  5     :   iSound =  3
            case 88     :   jpeg_bkg =  1   :   iAnimation =  5     :   iSound =  2
            case else  :  jpeg_bkg =  1    :   iAnimation =  5     :   iSound =  2
                warnlog "Please insert the entrienumbers for 'Backgrounds', 'Sounds' and one with normal files ( Animations )"
        end select
    end if
    '/// Open a new Writer document
    '/// Tools / Gallery
    printlog "open a writer and the gallery"
    gApplication = "Writer"
    call hNewDocument
    Kontext
    call hOpenGallery
    '/// Check the view for Background-pictures (JPEGs), for standard graphic files (Animation) and for <i>Sound</i> objects
    for i=1 to 3
        if i=1 then
            Gallerys.Select jpeg_bkg
            printlog "selected gallery-theme : Backgrounds (Jpeg-files)"
        end if
        if i=2 then
            Gallerys.Select iAnimation
            printlog "selected gallery-theme : Animation"
        end if
        if i=3 then
            Gallerys.Select iSound
            printlog "selected gallery-theme : Sound"
        end if
        if gNetzInst = FALSE then
            iPreview = 2 : iTitle = 3
        else
            iPreview = 2 : iTitle = 0
        end if
        '/// Test the list-view
        '/// +Click on List-View-Button in toolbar
        printlog "- list-view"
        printlog "  - open the list-view"
        ListView.Click
        sleep (1)
        printlog "  - Press Home + Enter to focus and enter Preview-mode."
        '/// +Double-click on an object (preview has to be visible)
        View.TypeKeys "<HOME>"
        View.TypeKeys "<RETURN>"
        sleep (2)
        '/// +Back to normal view with Enter
        View.TypeKeys "<RETURN>"
        sleep (2)
        '/// +Preview out of context menu on/off
        printlog "  - preview out of context menu on/off"
        View.TypeKeys "<HOME>"
        Sleep (2)
        View.OpenContextMenu true
        sleep (3)
        hMenuSelectNr ( iPreview )
        sleep (3)
        kontext "Gallery"
            Sleep (2)
            View.OpenContextMenu true
            sleep (2)
            hMenuSelectNr ( iPreview )
            sleep (3)
            '/// +Title (FAT installation only)
            if gNetzInst = FALSE then
                printlog "  - title"
                Kontext "Gallery"
                    sleep (2)
                    View.OpenContextMenu true
                    sleep (3)
                    hMenuSelectNr ( iTitle )
                    Kontext "GalleryNewTitle"
                        sleep (1)
                        GalleryNewTitle.Cancel
                        sleep (1)
                    Kontext "Gallery"
            end if
            View.OpenContextMenu true
            sleep (3)
            hMenuSelectNr ( iPreview )
            sleep (3)
            '/// Should now be in Preview-mode. Check if we are. ///'
            View.OpenContextMenu true
            sleep (3)
            printlog MenuIsItemChecked (MenuGetItemID(iPreview+1))
            if MenuIsItemChecked (MenuGetItemID(iPreview+1)) then
                Printlog "   Entered Preview-mode correctly"
                sleep (1)
            else
                Warnlog "   Didnt seem to enter preview-mode correctly."
            end if
            MenuSelect (0)
            sleep (1)
            '/// From here, try switching to icon-view ///'
            '///+Click on Icon-View-Button in toolbar
            printlog "- icon-view"
            if IconView.isEnabled then
                IconView.Click
                qaErrorLog "The bug 64543 has been fixed! Please report to FHA."
            end if
            sleep (1)
            View.TypeKeys "<HOME>"
            printlog "  - Enter to get out of Preview-mode"
            '/// +Press Return on an object (preview has to be visible)
            View.TypeKeys "<RETURN>"
            sleep (2)
            '/// +Back to normal view with Return
            View.TypeKeys "<RETURN>"
            sleep (2)
            '/// +Preview out of context menu on/off
            printlog "  - preview out of context menu on/off"
            View.TypeKeys "<HOME>"
            sleep (3)
            View.OpenContextMenu true
            sleep (3)
            hMenuSelectNr ( iPreview )
            sleep (3)
            View.OpenContextMenu true
            sleep (3)
            hMenuSelectNr ( iPreview )
            sleep (3)
            '/// +Title (FAT installation only)
            if gNetzInst = FALSE then
                printlog "  - title"
                try
                    View.OpenContextMenu true
                    sleep (3)
                catch
                    warnlog "Could not open Contextmenu for Title nr:" +iTitle
                endcatch
                hMenuSelectNr ( iTitle )
                Kontext "GalleryNewTitle"
                    GalleryNewTitle.Cancel
                Kontext "Gallery"
            end if
        next i
        sleep 1 ' else crash
   ToolsGallery

    '/// If the MediaPlayer exists - close it ///'
    kontext "Mplayer"
        if Mplayer.Exists then
            if (lcase(gApplication) = "impress") then
                kontext "DocumentImpress"
            else
                kontext "DocumentDraw"
            end if
            ToolsMediaPlayer
        else
            if (lcase(gApplication) = "impress") then
                kontext "DocumentImpress"
            else
                kontext "DocumentDraw"
            end if
        end if

    call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tGallery_GalleryView_Insert
    Dim jpeg_bkg as Integer
    Dim iAnimation as Integer
    Dim i as Integer
    Dim j as Integer
    Dim k as Integer
    '/// Test gallery-view -> insert gallery-object with context-menu (3D-Object (<i>internal object</i>), Animation (<i>as file</i>))
    if NOT gOOO then
        select case iSprache
            case 01     :   jpeg_bkg = 3     :   iAnimation =  1
            case 07     :   jpeg_bkg = 29   :   iAnimation =  1
            case 31     :   jpeg_bkg = 3     :   iAnimation =  3
            case 33     :   jpeg_bkg = 13   :   iAnimation =  1
            case 34     :   jpeg_bkg = 7     :   iAnimation =  1
            case 36     :   jpeg_bkg = 12   :   iAnimation =  1
            case 39     :   jpeg_bkg = 10   :   iAnimation =  1
            case 46     :   jpeg_bkg = 1     :   iAnimation =  2
            case 48     :   jpeg_bkg = 3     :   iAnimation =  1
            case 49     :   jpeg_bkg = 1     :   iAnimation =  2
            case 55     :   jpeg_bkg = 8     :   iAnimation =  1
            case 81     :   jpeg_bkg = 1     :   iAnimation =  3
            case 82     :   jpeg_bkg = 1     :   iAnimation = 17
            case 86     :   jpeg_bkg = 17   :   iAnimation = 15 
            case 88     :   jpeg_bkg = 17   :   iAnimation = 15
            case else  :  jpeg_bkg = 1     :   iAnimation =  2
                                warnlog "Please insert the entrienumbers for 'Backgrounds' and one with normal files ( Animations )"
        end select
    else  ' Testing OOO
        select case iSprache
            case 01   : jpeg_bkg =  3 : iAnimation =  1
            case 07   : jpeg_bkg =  5 : iAnimation =  1
            case 31   : jpeg_bkg =  3 : iAnimation =  3
            case 33   : jpeg_bkg =  5 : iAnimation =  1
            case 34   : jpeg_bkg =  6 : iAnimation =  1
            case 36   : jpeg_bkg =  5 : iAnimation =  1
            case 39   : jpeg_bkg =  5 : iAnimation =  1
            case 46   : jpeg_bkg =  1 : iAnimation =  2
            case 48   : jpeg_bkg =  3 : iAnimation =  1
            case 49   : jpeg_bkg =  1 : iAnimation =  2
            case 55   : jpeg_bkg =  6 : iAnimation =  1
            case 81   : jpeg_bkg =  1 : iAnimation =  3
            case 82   : jpeg_bkg =  1 : iAnimation =  1
            case 86   : jpeg_bkg =  5 : iAnimation =  5 
            case 88   : jpeg_bkg =  5 : iAnimation =  1
            case else : jpeg_bkg =  1 : iAnimation =  2
                                warnlog "Please insert the entrienumbers for 'Backgrounds' and one with normal files ( Animations )"
        end select
    end if
    '/// Test following for all applications (Writer, Calc, Impress, Draw)
    for i=1 to 4           ' each application
        if i=1 then gApplication = "Writer"
        if i=2 then gApplication = "Calc"
        if i=3 then gApplication = "Impress"
        if i=4 then gApplication = "Draw"

        '/// +Open a new document
        '/// +Open the Gallery
        printlog gApplication  'Chr(13)
        call hNewDocument
        call hOpenGallery
        '/// +Select 3D-theme (these are internal objects and not realy files)
        '/// +<ul><li>on context menu there are only 2 entries for insert (copy, link)
        '/// +<li>insert the object per <i>copy</i></li>
        '/// +<li>insert the object per <i>link</i></li></ul>
        '/// +Select Animation theme (these are realy files)
        '/// +<ul><li>On context menu there are only 2 entries for insert (copy, link ) for Calc, Draw Impress</li>
        '/// +<li>On context menu there are only 3 entries for insert (copy, link, background / page, paragraph ) for Writer</li>
        '/// +<li>insert the object per <i>copy</i></li>
        '/// +<li>insert the object per <i>link</i></li>
        '/// +<li>Only for the Writer:<ul>
        '/// +<li>Insert the object per background / page</li>
        '/// +<li>insert the object per background / paragraph</li></ul></li></ul>
        '/// +Close the Gallery
        '/// +Close the document
        '/// Since the GraphicObjectbar could disturb our testing, we close it if it is open ///'
        '      kontext "GraphicObjectbar"
        '      if GraphicObjectbar.Exists then GraphicObjectbar.Close
        for j=1 to 2        ' the two gallery-theme
            if j=1 then
                Gallerys.Select jpeg_bkg
                printlog " selected gallery-theme : Backgrounds"
            end if

            if j=2 then
                Gallerys.Select iAnimation
                printlog " selected gallery-theme : Animation"
            end if

            if i=1 then       ' test for writer
                for k=1 to 2
                    if k=1 then
                        ListView.Click
                        printlog "  - insert on list view"
                    else
                        IconView.Click
                        printlog "  - insert on icon view"
                    end if

                    sleep (2)
                    View.Mousedown ( 5, 18 )
                    View.Mouseup ( 5, 18 )
                    View.typekeys "<DOWN>,<UP>"

                    sleep (3)
                    View.TypeKeys "<SHIFT F10>" ' OpenContextMenu
                    sleep (3)
                    hMenuSelectNr ( 1 )

                    if j=1 then
                        printlog "   - copy"
                        sleep (1)
                        hMenuSelectNr ( 1 )
                        sleep (2)
                    else
                        printlog "   - copy"
                        sleep (1)
                        hMenuSelectNr ( 1 )
                        sleep (3)

                        printlog "   - link"
                        View.TypeKeys "<SHIFT F10>" 'OpenContextMenu
                        sleep (1)
                        hMenuSelectNr ( 1 )
                        hMenuSelectNr ( 2 )
                        sleep (3)

                        printlog "   - background -> page"
                        View.TypeKeys "<SHIFT F10>" 'OpenContextMenu
                        sleep (2)
                        hMenuSelectNr ( 1 )
                        hMenuSelectNr ( 3 )
                        hMenuSelectNr ( 1 )
                        sleep (3)

                        printlog "   - background -> paragraph"
                        View.TypeKeys "<SHIFT F10>" 'OpenContextMenu
                        sleep (2)
                        hMenuSelectNr ( 1 )
                        hMenuSelectNr ( 3 )
                        hMenuSelectNr ( 2 )
                        sleep (3)
                    end if

                    kontext "GraphicObjectbar"
                    if GraphicObjectbar.Exists then
                        if GraphicObjectbar.isDocked = False then
                            GraphicObjectbar.Move 900,900
                        end if
                    end if
                    kontext "Gallery"

                next k
            else              ' test for other applications ( calc, impress, draw )
                kontext "GraphicObjectbar"
                    if GraphicObjectbar.Exists then
                        if GraphicObjectbar.isDocked = False then
                            GraphicObjectbar.Move 900,900
                        end if
                    end if
                kontext "Gallery"

                kontext "Gluepointsobjectbar"
                    if Gluepointsobjectbar.Exists then
                        if Gluepointsobjectbar.isDocked = false then
                            Gluepointsobjectbar.Move 900,900
                        end if
                    end if
                kontext "Gallery"

                kontext "Optionsbar"
                    if Optionsbar.Exists then
                        if Optionsbar.isDocked = false then
                            Optionsbar.Move 900,900
                        end if
                    end if
                kontext "Gallery"

                    for k=1 to 2
                        if k=1 then
                            ListView.Click
                            printlog "  - insert on list view"
                        else
                            IconView.Click
                            printlog "  - insert on icon view"
                        end if

                    sleep (1)
                    View.Mousedown ( 5, 18 )
                    View.Mouseup ( 5, 18 )
                    View.typekeys "<DOWN>,<UP>"

                    sleep (1)
                    View.TypeKeys "<SHIFT F10>" 'OpenContextMenu
                    printlog "    - copy"
                    sleep (3)
                    hMenuSelectNr ( 1 )
                    hMenuSelectNr ( 1 )
                    sleep (3)

                    if j=2 then
                        View.TypeKeys "<SHIFT F10>" 'OpenContextMenu
                        printlog "    - link"
                        sleep (3)
                        hMenuSelectNr ( 1 )
                        hMenuSelectNr ( 2 )
                        sleep (2)
                    end if

                next k
            end if
        next j
        ToolsGallery
        WaitSlot (2000)
        call hCloseDocument
        WaitSlot (2000)
    next i
endcase

'-------------------------------------------------------------------------

testcase tGallery_GalleryView_Delete
    Dim jpeg_bkg as Integer
    Dim iAnimation as Integer
    Dim iSound as Integer
    Dim i as Integer
    Dim j as Integer
    Dim k as Integer

    if gNetzInst = TRUE then
        ' TODO: since we now only know net- installations, make it work!
        printlog "No test for 'net' installations, because there are no rights to delete objects out of Gallery!"
        'NOTE: Jump to NotForNetInst...
        Goto NotForNetInst
    end if

    '/// Test gallery view -> delete objects with context-menu ( Backgrounds (<i>internal object</i>), Animation (<i>file</i>), Sound (<i>file</i>))
    select case iSprache
        case 01   : jpeg_bkg = 1    :   iAnimation = 2   :  iSound = 28
        case 07   : jpeg_bkg = 29  :   iAnimation = 1   :  iSound =  6
        case 31   : jpeg_bkg = 1    :   iAnimation = 2   :  iSound = 28
        case 33   : jpeg_bkg = 13  :   iAnimation = 1   :  iSound = 29
        case 34   : jpeg_bkg = 7    :   iAnimation = 1   :  iSound = 28
        case 36   : jpeg_bkg = 12  :   iAnimation = 1   :  iSound = 10
        case 39   : jpeg_bkg = 10  :   iAnimation = 1   :  iSound = 27
        case 46   : jpeg_bkg = 1    :   iAnimation = 2   :  iSound = 18
        case 49   : jpeg_bkg = 1    :   iAnimation = 2   :  iSound = 17
        case 55   : jpeg_bkg = 8    :   iAnimation = 1   :  iSound = 30
        case 81   : jpeg_bkg = 1    :   iAnimation = 3   :  iSound = 10
        case 82   : jpeg_bkg = 1    :   iAnimation = 17 :  iSound = 12
        case 86   : jpeg_bkg = 1    :   iAnimation = 9   :  iSound = 13
        case 88   : jpeg_bkg = 1    :   iAnimation = 7   :  iSound = 20
        case else : jpeg_bkg = 1   :  iAnimation = 2   :  iSound = 17
                  warnlog "Please insert the entrienumbers for '3D-graphics', 'Sounds' and one with normal files ( Animations )"
    end select

    '/// Open a new writer-doc
    '/// Tools / Gallery
    gApplication = "Writer"
    call hNewDocument
    call hOpenGallery
    '/// Delete one object in list-view and icon-view for 3D, Animation and Sound
    for i=1 to 3
        Kontext "Gallery"
            if i=1 then
                Gallerys.Select jpeg_bkg
                printlog " selected gallery-theme : Backgrounds"
            end if

            if i=2 then
                Gallerys.Select iAnimation
                printlog " selected gallery-theme : Animation"
            end if

            if i=3 then
                Gallerys.Select iSound
                printlog " selected gallery-theme : Sound"
            end if

            for j=1 to 2
                Kontext "Gallery"
                    if j=1 then
                        IconView.Click
                        printlog "   Icon view"
                        sleep 1
                    end if
                    if j=2 then
                        ListView.Click
                        printlog "   List view"
                    end if

                    for k=1 to 2
                        Kontext "Gallery"
                            sleep (1)
                            View.MouseMove ( 5, 15 )
                            View.TypeKeys "<SHIFT F10>" 'OpenContextMenu true
                            hMenuSelectNr ( 4 )
                            kontext "Active"
                                if k=1 then
                                    Active.No
                                    printlog "      delete one object => no"
                                else
                                    Active.Yes
                                    printlog "      delete one object => yes"
                                end if
                    next k
            next j
    next i
    ToolsGallery
    call hCloseDocument
    'NOTE: Jump End NotForNetInst
    NotForNetInst:
endcase

'-------------------------------------------------------------------------
testcase tGallery_Quick_check

    '/// Short check, if at least 2 files exist in the gallery, and if they are > 0 in size ///'
    '/// One in the gallery folder itself and the other one in a subfolder. ///'
    dim sFile(1) as string
    dim i as integer

    sFile(0) = ConvertPath ( gOfficeBasisPath + "share/gallery/apples.gif" ) '
    sFile(1) = ConvertPath ( gOfficeBasisPath + "share/gallery/bullets/coffee_1.gif" ) '
    '/// Open a new document
    call hNewDocument
    for i = 0 to 1
    '/// Click to deselect any selected objects ///'
        gMouseclick 1, 50
        if FileExists(sFile(i)) then
            if (FileLen(sFile(i)) > 0 ) then
                call hGrafikEinfuegen ( sFile(i) )
            else
                warnlog "File lenght is 0: '" + sFile(i) + "'"
            end if
        else
            warnlog "File doesn't exist: '" + sFile(i) + "'"
        end if
    next i
    '///close the document
    call hCloseDocument
endcase

