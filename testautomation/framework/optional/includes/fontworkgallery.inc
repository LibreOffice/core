'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: fontworkgallery.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:14 $
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
'* owner : thorsten.bosbach@sun.com
'*
'* short description : Resource test of font work gallery
'*
'\************************************************************************

sub sFontworkGalleryUpdate
    dim sApplication
    dim i,a as integer

    sApplication = array("WRITER","MASTERDOC","CALC","IMPRESS","DRAW")

    ' for every application
    a = uBound(sApplication())
    for i = 0 to a
        gApplication = sApplication(i)
        printlog "********** " + gApplication
        tFontworkGalleryUpdate
    next i
end sub

testcase tFontworkGalleryUpdate
    dim j,b as integer
        
'i67024     -     tbo     sj     Help-IDs missing on the windows items for FontworkAlignment and FontworkCharacterSpacing
'i66989     -     tbo     sj     Fontwork Gallery control has no Help-ID
    '/// close all applications, exept the backingwindow, to make sure the right toolbar is used by TestTool ///'
    b = getDocumentCount
    for j = 1 to b
        printlog "closing: " + j
        hCloseDocument()
    next j
    '/// open new application window ///'
    hNewDocument()
    sleep (5)
    '/// if toolbar 'Drawing' is not visible, open it by View - Toolbars - Drawing ///'
    Kontext "Drawbar"
    if NOT Drawbar.exists then
        hToolbarSelect("DRAWING",true)
        printlog "opened drawing toolbar"
    endif
    if Drawbar.exists then
        '/// Click button 'Fontwork Gallery' on toolbar 'Drawing' ///'
        FontworkGallery.click
        
        Kontext "FontworkGallery"
        '/// the dialog 'Fontwork Gallery' has to come up ///'
        dialogtest(FontworkGallery)
        '/// In the dialog 'Fontwork Gallery' type key <cursor right> ///'
        FontworkGallery.typeKeys("<right>", true)
        '/// In the dialog 'Fontwork Gallery' press and release right mouse button in the middle of the dialog ///'
        FontworkGallery.mousedown(50,50)
        FontworkGallery.mouseup(50,50)
        '/// on the dialog 'Fontwork Gallery' press button OK ///'
        FontworkGallery.OK
        
        '/// an Fontwork object will be inserted now ///'
        sleep 5
        '/// The toolbar 'Fontwork' has to come up///'
        Kontext "FontworkObjectbar"
        if FontworkObjectbar.exists then
            '/// On the toolbar 'Fontwork' click the button 'Fontwork Gallery' ///'
            FontworkGallery.click
            '/// dialog 'Fontwork Gallery' has to come up ///'
            Kontext "FontworkGallery"
            dialogtest(FontworkGallery)
            '/// close dialog 'Fontwork Gallery' by clicking CANCEL button ///'
            FontworkGallery.cancel
        
            Kontext "FontworkObjectbar"
            '/// On the toolbar 'Fontwork' click the button 'Fontwork Shape' ///'
            FontworkShape.click
            '/// toolbar 'Fontwork Shape' has to come up ///'
            Kontext "FontworkShape"
            '/// press button 'Plain Text' on toolbar 'Fontwork Shape' ///'
            PlainText.click
            
            Kontext "FontworkObjectbar"
            '/// On the toolbar 'Fontwork' click the button 'Fontwork Same Letter Heights' ///'
            FontworkSameLetterHeights.click
            sleep 3
            
            Kontext "FontworkObjectbar"
            '/// On the toolbar 'Fontwork' click the button 'Fontwork Alignment' ///'
            FontworkAlignment.click
            Kontext "FontworkAlignment"
            if FontworkAlignment.exists(5) then
                dialogtest(FontworkAlignment)
                FontworkAlignment.close
            else
                printlog "beginning workaround lockup"
                hUseMenu()
                hMenuClose()
                sleep 3
                Kontext "FontworkObjectbar"
                FontworkAlignment.click
                Kontext "FontworkAlignment"
                if FontworkAlignment.exists(5) then
                    dialogtest(FontworkAlignment)
                    FontworkAlignment.close
                else
                    warnlog "FontworkAlignment dialog not visible"
                endif
            endif
            
            Kontext "FontworkObjectbar"
            '/// On the toolbar 'Fontwork' click the button 'Fontwork Character Spacing' ///'
            FontworkCharacterSpacing.click
            '/// toolbox 'Fontwork Character Spacing' has to come up ///'
            Kontext "FontworkCharacterSpacing"
            if FontworkCharacterSpacing.exists(5) then
                'dialogtest(FontworkCharacterSpacing)
                '/// try to select the entry 'Custom ...' with keys, since Help Ids are missing ///'
                FontworkCharacterSpacing.typeKeys ("<down><down><down><down><down><return>")
                '/// dialog 'Fontwork Character Spacing' has to come up ///'
                Kontext "FontworkCharacterSpacingCustom"
                if FontworkCharacterSpacingCustom.exists(5) then
                    dialogtest(FontworkCharacterSpacingCustom)
                    printlog value.getText
                    '/// close dialog 'Fontwork Character Spacing' ///'
                    FontworkCharacterSpacingCustom.close
                else
                    printlog "dialog FontworkCharacterSpacingCustom didn't came up"
                endif
                Kontext "FontworkCharacterSpacing"
                if FontworkCharacterSpacing.exists(5) then
                    '/// close dialog 'Fontwork Character Spacing' ///'
                    FontworkCharacterSpacing.close    
                else
                    printlog "dialog FontworkCharacterSpacing was closed in another way"
                endif
            else
                warnlog "fontWork characterspacing drop down menu is not visible"
            endif
        else
            warnlog "fontWork toolbar is not visible"
        endif
    else
        warnlog "Draw toolbar is not visible"
    endif
    hCloseDocument()
endcase

