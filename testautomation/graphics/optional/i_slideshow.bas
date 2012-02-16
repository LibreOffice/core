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
'* short description : Graphics Function: Slideshow
'*
'\******************************************************************

public glLocale (15*20) as string
global S1 as string
global S2 as string
global S3 as string
global S4 as string

sub main
    PrintLog "------------------------- slideshow test -------------------------"
    Call hStatusIn ( "Graphics","i_slideshow.bas")

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\impress\i_slideshow.inc"
    use "graphics\optional\includes\impress\i_slideshow2.inc"
    use "graphics\optional\includes\impress\i_slideshow3.inc"

    hSetLocaleStrings ( gTesttoolPath + "graphics\tools\locale_1.txt" , glLocale () )
    if glLocale (2) = "" then
        warnlog "Add 'slide' to FILE  /input/impress/locale_1.txt (take string from below)!!!"
    endif

    printlog "LOCALE Slide - 2"
    S1 = glLocale (2) + " 1"
    S2 = glLocale (2) + " 2"
    S3 = glLocale (2) + " 3"
    S4 = glLocale (2) + " 4"

    Call tSlideShowSlideTransition
    Call tPraesentationAnimation
    Call tAendernObjektBenennen
    Call tSlideShowRehearseTimings

    Call tSlideShowShowHideSlide
    Call tExtrasInteraktion
    Call tExtrasIndividuellePraesentation
    Call tExtrasPraesentationseinstellungen
    Call tSlideShowSlideShowSettings
    Call tSlideShowSlideShow
    Call tSlideShowInteraction
    Call tSlideShowCustomSlideShow
    Call tSlideshowBackgroundAllSlides
    Call tSlideshowBackgroundSelectedSlide
    Call tPraesentationEffekt
    Call tExtrasEffekt
    Call tSlideshowContextMenuOneSlide
    Call tSlideshowContextMenuMoreSlides
    Call tiMousePointerHides
    Call tSlideShowRehearseTimings

    Call hStatusOut
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_ui_filters.inc"
    gApplication = "IMPRESS"
    Call GetUseFiles
end sub
