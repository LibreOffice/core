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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : Resource test of font work gallery
'*
'\************************************************************************

sub sFontworkGalleryUpdate
    dim sApplication
    dim i,a as integer

    sApplication = array("WRITER","MASTERDOCUMENT","CALC","IMPRESS","DRAW")

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
    b = getDocumentCount
    for j = 1 to b
        printlog "closing: " + j
        hCloseDocument()
    next j
    hNewDocument()
    sleep (5)
    Kontext "Drawbar"
    if NOT Drawbar.exists then
        hToolbarSelect("DRAWING",true)
        printlog "opened drawing toolbar"
    endif
    if Drawbar.exists then
        FontworkGallery.click
        
        Kontext "FontworkGallery"
        dialogtest(FontworkGallery)
        FontworkGallery.typeKeys("<right>", true)
        FontworkGallery.mousedown(50,50)
        FontworkGallery.mouseup(50,50)
        FontworkGallery.OK
        
        sleep 5
        Kontext "FontworkObjectbar"
        if FontworkObjectbar.exists then
            FontworkGallery.click
            Kontext "FontworkGallery"
            dialogtest(FontworkGallery)
            FontworkGallery.cancel
        
            Kontext "FontworkObjectbar"
            FontworkShape.click
            Kontext "FontworkShape"
            PlainText.click
            
            Kontext "FontworkObjectbar"
            FontworkSameLetterHeights.click
            sleep 3
            
            Kontext "FontworkObjectbar"
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
            FontworkCharacterSpacing.click
            Kontext "FontworkCharacterSpacing"
            if FontworkCharacterSpacing.exists(5) then
                'dialogtest(FontworkCharacterSpacing)
                FontworkCharacterSpacing.typeKeys ("<down><down><down><down><down><return>")
                Kontext "FontworkCharacterSpacingCustom"
                if FontworkCharacterSpacingCustom.exists(5) then
                    dialogtest(FontworkCharacterSpacingCustom)
                    printlog value.getText
                    FontworkCharacterSpacingCustom.close
                else
                    printlog "dialog FontworkCharacterSpacingCustom didn't came up"
                endif
                Kontext "FontworkCharacterSpacing"
                if FontworkCharacterSpacing.exists(5) then
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

