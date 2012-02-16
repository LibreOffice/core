'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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
