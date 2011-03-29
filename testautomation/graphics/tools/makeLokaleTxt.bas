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
'* Owner : wolfram.garten@oracle.com
'*
'* short description : Graphics: get language dependant strings
'*
'\******************************************************************

printlog "Run this script, if you are testing a new language."
printlog "it will add all relevant strings to the file qatesttool/graphics/tools/locale_1.txt "
printlog "you just have to make sure, that every language only appears once in the file! "

public glLocale (15*20) as string
public S1 as string
public S2 as string
public S3 as string
public S4 as string

sub main
    use "graphics\tools\id_tools.inc"
    use "graphics\optional\includes\impress\i_slideshow.inc"
    use "graphics\optional\includes\impress\i_slideshow2.inc"
    use "graphics\optional\includes\global\g_stylist.inc"        'format -> stylist
    use "graphics\optional\includes\global\g_spellcheck.inc"         'tools

    '    GetOLEDefaultNames

    if hSetLocaleStrings ( gTesttoolPath + "graphics\tools\locale_1.txt" , glLocale () ) = FALSE then
        warnlog "Locales file doesn't exist: '" + gTesttoolPath + "graphics\tools\locale_1.txt'"
    endif

    printlog "Current saved strings are: "
    printlog "O 1: " + glLocale (1) 'Handouts        ' im_103.inc::tViewWorkspaceHandoutView
    printlog "O 2: " + glLocale (2) 'Slide           ' impress/slideshow.inc::
    printlog "O 3: " + glLocale (3) 'Default
    printlog "O 4: " + glLocale (4) 'English (USA)
    printlog "O 5: " + glLocale (5) 'Background
    printlog "O 6: " + glLocale (6) 'German (Germany)

    printlog "LOCALE Slide - 2 "
    S1 = glLocale (2) + " 1"
    S2 = glLocale (2) + " 2"
    S3 = glLocale (2) + " 3"
    S4 = glLocale (2) + " 4"

    printlog "Strings from current office: "

    '(1)tViewWorkspaceHandoutView
    hNewDocument
    '   hUseMenu
    '   hMenuSelectNr(3)
    '   hMenuSelectNr(11)
    '   printlog hMenuItemGetText (4)
    '   printlog "view background handout"
    '  ViewWorkspaceHandoutView						
    '   sleep 2
    '  FormatPage
    sleep 2
    glLocale (1) = "Dummy" 'Seitenname.GetText
    Printlog "L1: '" + glLocale (1) + "'"
    '3  Seitenlayout.Close
    '  ViewWorkspaceDrawingView

    '(2)tSlideShowInteraction/fGetSlideName
    Kontext "Navigator"
    if NOT Navigator.exists then
        hTypeKeys "<SHIFT MOD1 F5>"
    endif
    Kontext "NavigatorDraw"
    if NavigatorDraw.exists (5) then
        sleep 3
        glLocale (2) = left(Liste.GetSelText, len(Liste.GetSelText)-2)
        printlog "L2: '" + glLocale (2) + "'"
    else
        warnlog "Navigator not open!"
    endif
    Kontext "Navigator"
    if Navigator.Exists then
        Navigator.Close
    endif

    hCloseDocument

    '(3)tFormatStylistTemplateSelect 'stylist.inc::tFormatStylistTemplateSelect
    gApplication = "WRITER"
    hNewDocument
    kontext "Gestalter"
    if NOT Gestalter.Exists then
        FormatStylesFormatting
        sleep 1
    endif
    glLocale (3) = Vorlagenliste.getSelText    '   HID_STYLE_LISTBOX   ' get "Default"
    printlog "L3: '" + glLocale (3) + "'"
    hCloseDocument
    gApplication = "IMPRESS"

    '(4)ASIANONLY tiToolsSpellcheckError
    ' no need to exclude eurtopean!
    ' this entry will get English (USA)
    '   spellbook language as alternative for not existijng asians one
    '    tiToolsSpellcheckError
    gApplication = "DRAW"
    hDateiOeffnen (convertPath(gTesttoolpath + "graphics/required/input/recht_1.sxd"))
    sleep 5
    hTypeKeys "<Tab><F2><Mod1 Home><Shift End>"
    sleep 5
    FormatCharacter
    sleep 1
    Kontext
    Messagebox.SetPage TabFont
    kontext "TabFont"
    sleep 1
    try
        glLocale (4) = LanguageWest.getSelText
        printlog "L4: '" + glLocale (4) + "'"
    catch
        glLocale (4) = Language.getSelText
        printlog "L4: '" + glLocale (4) + "'"
    endcatch
    TabFont.Ok
    sleep 5
    Call hCloseDocument
    gApplication = "IMPRESS"

    '(5)tFormatStylistBackground
    '    stylist only one tabpage area!
    Dim i as Integer : Dim AlterWert as String : Dim NeuerWert as String : Dim rightentry as Integer : Dim qd as Integer : Dim numberofentries as Integer
    hNewDocument

    kontext "DocumentImpress"
    if iSprache <> "36" then
        Kontext "Gestalter"
        if NOT Gestalter.Exists(1) then
            hTypeKeys "<F11>"
            Kontext "Gestalter"
        endif
        Praesentationsvorlagen.Click
        sleep (2)
        for qd = 1 to Vorlagenliste.GetItemCount
            Kontext "Gestalter"
            Vorlagenliste.Select qd
            Vorlagenliste.OpenContextMenu
            hMenuSelectNr (1)
            kontext "TabArea"
            if TabArea.exists then
                rightentry = qd
                TabArea.Close
            else
                kontext "TabVerwalten"
                if TabVerwalten.Exists(1) then
                    TabVerwalten.Close
                endif
                kontext "TabLinie"
                if TabLinie.Exists(1) then
                    TabLinie.Close
                endif
                kontext "TabSchatten"
                if TabSchatten.Exists(1) then
                    TabSchatten.Close
                endif
                kontext "TabTransparenz"
                if TabTransparenz.Exists(1) then
                    TabTransparenz.Close
                endif
                kontext "TabFont"
                if TabFont.Exists(1) then
                    TabFont.Close
                endif
                kontext "TabFontEffects"
                if TabFontEffects.Exists(1) then
                    TabFontEffects.Close
                endif
                kontext "TabEinzuegeUndAbstaende"
                if TabEinzuegeUndAbstaende.Exists(1) then
                    TabEinzuegeUndAbstaende.Close
                endif
                kontext "TabAusrichtungAbsatz"
                if TabAusrichtungAbsatz.Exists(1) then
                    TabAusrichtungAbsatz.Close
                endif
                kontext "TabTabulator"
                if TabTabulator.Exists(1) then
                    TabTabulator.Close
                endif
                kontext "TabBullet"
                if TabBullet.Exists(1) then
                    TabBullet.Close
                endif
                kontext "TabNumerierungsart"
                if TabNumerierungsart.Exists(1) then
                    TabNumerierungsart.Close
                endif
                kontext "TabGrafiken"
                if TabGrafiken.Exists(1) then
                    TabGrafiken.Close
                endif
                kontext "TabOptionenNumerierung"
                if TabOptionenNumerierung.Exists(1) then
                    TabOptionenNumerierung.Close
                endif
            endif
            Kontext "Gestalter"
        next qd

        dim stringname as string
        Vorlagenliste.Select rightentry
        stringname = Vorlagenliste.GetSelText '(1) 'SelText 'MenuGetItemText(MenugetItemid(1)) '8
        glLocale (5) = stringname ' "dummy" 'right(stringname, len(stringname)-1)
        printlog "L5: '" + glLocale (5) + "'"
    endif

    '(6)
    ' this entry will get German (Germany)
    gApplication = "DRAW"
    hDateiOeffnen (convertPath(gTesttoolpath + "graphics/required/input/recht_49.sxd"))
    sleep 5
    hTypeKeys "<Tab><F2><Mod1 Home><Shift End>"
    sleep 5
    FormatCharacter
    sleep 1
    Kontext
    Messagebox.SetPage TabFont
    kontext "TabFont"
    sleep 1
    try
        glLocale (6) = LanguageWest.getSelText
        printlog "L6: '" + glLocale (6) + "'"
    catch
        glLocale (6) = Language.getSelText
        printlog "L6: '" + glLocale (6) + "'"
    endcatch
    TabFont.Ok
    sleep 5
    Call hCloseDocument

    gApplication = "IMPRESS"

    dim lLocale (15*20) as string ' list, where file gets loaded into
    dim flocale as string

    flocale = convertPath (gTesttoolPath + "graphics\tools\locale_1.txt")
    ListRead (lLocale (), fLocale, "UTF8" )
    ListAppend(lLocale (), iSprache)
    for i = 1 to 6
        ListAppend(lLocale (), glLocale (i))
    next i
    ListWrite (lLocale (), fLocale, "UTF8" )

    hCloseDocument

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    'use "global\tools\includes\optional\t_ole.inc"
    gApplication = "IMPRESS"
    Call GetUseFiles
end sub


