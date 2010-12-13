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
'* owner : wolfram.garten@oracle.com
'*
'* short description : Checking all gallery themes
'*
'*****************************************************************
'*
' #1 tLoad100GalleryGraphicFiles  'Inserting 100 gallery graphics and checking the sizes
' #1 tCheckAllGalleryGraphicFiles 'Checking the size ( > 0 Byte ) of all gallery-files.
' #1 tInsertGalleryObjects        'Inserting random objects via contextmenu-insert-copy.
' #0 LoadGraphic
' #0 CheckGraphic
' #0 makeNumOutOfText
' #1 tSettingsToCM 'Measurement unit for textdocuments to cm
' #1 tResetSettings 'Resetting the measurement unit for textdocuments.
'*
'\****************************************************************

testcase tSettingsToCM

    printlog " Setting the measurement unit for textdocuments to cm."
    printlog " +Tools / options / text documents / general"
    ExitRestartTheOffice
    WaitSlot (10000)
    Call hNewDocument
    ToolsOptions
    hToolsOptions ( "WRITER", "General" )
    iSaveSetting = Masseinheit.GetSelIndex
    Masseinheit.Select 2
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    Call hCloseDocument

endcase
'-------------------------------------------------------------------------
testcase tLoad100GalleryGraphicFiles

    Dim lsFiles (3000) as String
    Dim lsGraphics (3000) as String
    Dim i as Integer
    Dim y as Integer
    Dim t as Integer
    Dim iCount as Integer
    Dim corLoad as Boolean
    Dim x as boolean

    if gNetzInst = TRUE then
        GetAllFileList ( ConvertPath ( gOfficeBasisPath + "share\gallery\" ), "*.*", lsFiles () )
    else
        GetAllFileList ( ConvertPath ( gOfficePath + "share\gallery\" ), "*.*", lsFiles () )
    end if
    call GetOnlyGraphics ( lsFiles (), lsGraphics() )
    iCount = ListCount ( lsGraphics() )
    printlog "   We have " + iCount + " graphics in the gallery."
    call hNewDocument

    for i = 1 to 100
        for y = 1 to 1
            randomize
            t = Int((iCount*Rnd))
            if (t<1) then 'just so we get it between 1 and the amount of items.
                y = y - 1
            end if
        next y

        corLoad = FALSE
        try
            x = LoadGraphic ( lsGraphics(t), corLoad )
            printlog
        catch
            ExceptLog
            if corLoad = FALSE then
                warnlog "Problems with " + lsGraphics(t)
                ResetApplication
                call hNewDocument
            end if
        endcatch
        if (not x) then
            printlog "   Tested nr: " + t + " : " + lsGraphics(t)
        end if
    next i

    ToolsGallery
    WaitSlot (2000)
    call hCloseDocument
endcase 'tLoadAllGalleryGraphicFiles

'-------------------------------------------------------------------------

testcase tCheckAllGalleryGraphicFiles
    Dim lsFiles (3000) as String
    Dim lsGraphics (3000) as String
    Dim i as Integer
    Dim y as Integer
    Dim t as Integer
    Dim iCount as Integer
    Dim corLoad as Boolean
    Dim x as boolean

    if gNetzInst = TRUE then
        GetAllFileList ( ConvertPath ( gOfficeBasisPath + "share\gallery\" ), "*.*", lsFiles () )
    else
        GetAllFileList ( ConvertPath ( gOfficePath + "share\gallery\" ), "*.*", lsFiles () )
    end if
    call GetOnlyGraphics ( lsFiles (), lsGraphics() )
    iCount = ListCount ( lsGraphics() )
    printlog "   We have " + iCount + " graphics in the gallery."
    call hNewDocument

    for i=1 to iCount
        corLoad = FALSE
        try
            x = CheckGraphic ( lsGraphics(i), corLoad )
        catch
            ExceptLog
            if corLoad = FALSE then
                warnlog "Problems with " + lsGraphics(i)
                ResetApplication
                call hNewDocument
            end if
        endcatch
        if (not x) then
            printlog "   Tested nr: " + i + " : " + lsGraphics(i)
        end if
    next i
    ToolsGallery
    WaitSlot (2000)
    call hCloseDocument
endcase 'tLoadAllGalleryGraphicFiles

'-------------------------------------------------------------------------

testcase tInsertGalleryObjects
    Dim lsFiles (3000) as String
    Dim lsGraphics (3000) as String
    Dim CountOfThemes
    Dim HowManyItems as Integer
    Dim WhichOne as Integer
    Dim CountOfItems as Integer
    Dim ct as Integer
    Dim i as Integer
    Dim d as Integer
    Dim iCount as Integer
    Dim corLoad, x as Boolean

    call hNewDocument
    call hOpenGallery
    kontext "Gallery"
    CountOfThemes = Gallerys.GetItemCount
    for ct = 1 to CountOfThemes
        Gallerys.Select ct
        printlog "   Selected Gallery-Theme nr: " + ct + ": " + Gallerys.GetSelText
        kontext "Gallery"
        CountOfItems = View.GetItemCount()
        for d = 1 to 1
            randomize
            HowManyItems=Int((5*Rnd)+(3*Rnd))
            if CountOfItems = 0 then
                QaErrorLog "   There were no objects in the the gallery-theme on position: " + ct
            else
                if (HowManyItems<3) then 'just so we get it between 3 and 5.
                    d = d - 1
                else
                    printlog "   Will now select and copy " + HowManyItems + " items from this Theme."
                end if
                for i = 1 to HowManyItems
                    for x = 1 to 1
                        randomize
                        WhichOne=Int(CountOfItems*RND) '(5*Rnd)+(CountOfItems*Rnd))
                        if (WhichOne<1) then 'just so we get it between 1 and the amount of items.  '>(CountOfItems + 1)) OR (WhichOne<1) then '
                            x = x - 1
                        end if
                    next x
                    printlog "   Will copy object nr: " + WhichOne
                    View.Mousemove (1,1)
                    View.TypeKeys "<HOME>"
                    View.TypeKeys "<RIGHT>", (WhichOne)
                    kontext "GraphicObjectBar"
                    if GraphicObjectBar.Exists then
                        if GraphicObjectBar.IsDocked = False then GraphicObjectBar.Dock
                        end if
                        kontext "Gallery"
                        sleep (1)
                        View.TypeKeys("<shift f10>")
                        sleep (1)
                        try
                            dim number as integer
                            number = MenuGetItemCount
                            if (number > 2) AND (number < 10) then
                                hMenuSelectNr (1) 'Insert
                                hMenuSelectNr (1) 'As Copy
                                sleep (1)
                            else
                                Warnlog "   The contextmenu came up, but the number of entries were strange."
                                printlog "   Number of entries:" + number
                                MenuSelect(0)
                            end if
                        catch
                            warnlog "   A contextmenu didnt come up for the gallery-theme on position: " + ct
                            i = HowManyItems
                        endcatch
                    next i
                end if 'if the theme didnt have any objects, we landed here.
            next d
        next ct

        Kontext "Gallery"
        if Gallery.Exists(2) then
            ToolsGallery
            WaitSlot (2000)
        end if

        hTypeKeys "<DELETE>"
        call hCloseDocument

endcase 'tInsertGalleryObjects

'-------------------------------------------------------------------------
testcase tResetSettings

    printlog "Resetting the measurement unit for textdocuments."
    printlog "+Tools / options / text documents / general"
    call hNewDocument
    ToolsOptions
    hToolsOptions ( "WRITER", "General" )
    Masseinheit.Select iSaveSetting
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    call hCloseDocument

endcase 'tResetSettings
'-------------------------------------------------------------------------
