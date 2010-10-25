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
'* short description : (quick test) Checking functionality and all Gallery themes
'*
'\****************************************************************

global iSaveSetting as Integer

sub main
    call hStatusIn ( "Graphics", "gallery.bas")

    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\global\gallery.inc"

    printlog "------------------------ F u n c t i o n a l i t y ----------------------"
    call tGallery_DialogTest                           ' open gallery, pin - unpin, dock - undock, close gallery
    call tGallery_ActivateAndUpdateAllThemes           ' activate all gallery-themes and update this theme
    call tGallery_CheckNames                           ' check the names for all themes depends on language
    call tGallery_CheckContextMenuForGalleryThemes     ' test the context-menu on themes-window
    call tGallery_GalleryView_Preview                  ' preview-feature in gallery-beamer-window
    call tGallery_GalleryView_Insert                   ' insert galler-objects in all applications
    call tGallery_GalleryView_Delete                   ' delete gallery-objects out of beamer-window
    call tGallery_CreateAndWorkWithANewGalleryThemes   ' create a new theme, add graphics and work with it
    call tGallery_Quick_check                          ' take 2 graphics and chek if they are

    call hStatusOut
end sub

'-------------------------------------------------------------------------

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    gApplication = "WRITER"
    call GetUseFiles
end sub

'-------------------------------------------------------------------------
