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

