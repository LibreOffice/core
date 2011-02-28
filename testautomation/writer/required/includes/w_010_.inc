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
'* owner : helge.delfs@oracle.com
'*
'* short description :  Testcases for the Objectbars.
'*
'\******************************************************************

sub w_010_

    printLog Chr(13) + "--------- Objectbars (w_010_.inc) ----------"
    gApplication = "WRITER"

    '   ** Objectbar
    Call tFormatting
    Call tNumbering
    Call tEnumerating
    Call tIndentIncreaseDecrease

    '   ** Objectbar Numbering / Aufzaehlung
    Call tNumFormatting
    Call tNumLevelUpDown
    Call tNumSubpointsUpDown
    Call tNumMoveUpMoveDown
    Call tNumSubpointsMoveUpMoveDown
    Call tNumUnnumberedEntry
    Call tNumRestartNumbering
    Call tNumNumberingOff
    Call tNumNumberingEnumerating
    Call tUpFormatting
    Call tUpLevelUpDown
    Call tUpSubpointsUpDown
    Call tUpMoveUpMoveDown
    Call tUpSubpointsMoveUpMoveDown
    Call tUpUnnumberedEntry
    Call tUpRestartNumbering
    Call tUpNumberingOff
    Call tUpNumberingEnumerating

    'Writer Web
    gApplication = "HTML"
    Call tHtmlDocFormatting
    gApplication = "MASTERDOCUMENT"
    Call tMasterDocFormatting
    gApplication = "WRITER"
    
end sub

'-----------------------------------------------------------

testcase tFormatting

    GLOBAL_USE_NEW_SLEEP = TRUE

    printlog "- Objectbar ( normal ) : Formatting"
    Call hNewDocument
    printlog " Open new document"
    Call wTypeKeys ("Formatting <Shift Home>")
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )

    printlog "  - Style"
    try
        Vorlage.Select 1
    catch
        Warnlog "Style could not be changed in TextObjectbar"
    endcatch
    printlog " Choose first 'style' in object bar"
    Sleep 5
    printlog "  - Font"
    printlog " Select 5th 'Font' in object bar"
    try
        Schriftart.Select 5
    catch
        Warnlog "Unable to select font in object bar"
    endcatch
    Schriftart.TypeKeys "<Return>"
    Sleep 1
    printlog "  - Font Size"
    printlog " Select 10th 'Font Size' in object bar"
    try
        Schriftgroesse.Select 10
        Schriftgroesse.TypeKeys "<Return>"
    catch
        Warnlog "Uanble to change font size in toolbar"
    endcatch
    Sleep 1
    printlog "  - Bold"
    printlog " Select 'Bold' in object bar"
    Fett.Click
    Sleep 1
    printlog "  - Italic"
    printlog " Select 'Italic' in object bar"
    Kursiv.Click
    Sleep 1
    printlog "  - Underline"
    printlog " Select 'Underline' in object bar"
    Unterstrichen.Click
    Sleep 1
    printlog "  - Align Left"
    printlog " Select 'Align Left' in object bar"
    Linksbuendig.Click
    Sleep 1
    printlog "  - Align Right"
    printlog " Select 'Align Right' in object bar"
    Rechtsbuendig.Click
    Sleep 1
    printlog "  - Centered"
    printlog " Select 'Centered' in object bar"
    Zentriert.Click
    Sleep 1
    if gApplication <> "HTML" then
        printlog "  - Justify"
        printlog " Select 'Justify' in object bar (not in HTML)"
        Blocksatz.Click
        Sleep 1
    end if
    printlog "  - Font color"
    printlog " Select 'Font color' in object bar"
    Schriftfarbe.Click
    Sleep 1
    printlog "  - Highlighting"
    printlog " Select 'Highlighting' in object bar"
    Zeichenhintergrund.Click
    Sleep 1
    Call hCloseDocument
    printlog " Close active document"

    GLOBAL_USE_NEW_SLEEP = FALSE
    
endcase

'-----------------------------------------------------------

testcase tNumbering
    printlog "- Objectbar ( normal ) : Insert Numbering"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    printlog " Select 'Numbering' in object bar"
    Numerierung.Click
    Call wTypeKeys ("Numbering <Return>", 2)
    printlog " Enter word 'Numbering' and '<Return>' two times"
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tEnumerating
    printlog "- Objectbar ( normal ) : Insert Bullets"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    printlog " In object bar click 'Bullets'"
    BulletsOnOff.Click
    Call wTypeKeys ("Bullets <Return>", 2)
    printlog " Enter word 'Bullets' and '<Return>' two times"
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tIndentIncreaseDecrease
    dim i as Integer

    printlog "- Objectbar ( normal ) : Increase Indent with numbering"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    printlog " Click 'Numbering' in object bar"
    Numerierung.Click
    Sleep 1
    try
        EinzugErhoehen.Click
        printlog " Select 'Increase Indent' in object bar (if not possible switch toolbar)"
    catch
        Warnlog "Unable to select function in Toolbar"
        Call hCloseDocument
        goto endsub
    endcatch

    Sleep 1
    for i=1 to 7
        if EinzugErhoehen.IsEnabled then
            printlog " Select 'Increase Indent' 7 times"
            EinzugErhoehen.Click
        else
            if i = 1 then Warnlog "Increase Indent is disabled !"
        end if
        wait 500
    next i

    printlog "- Objectbar ( normal ) : Decrease Indent with numbering"
    for i=1 to 8
        printlog " Select 'Decrease Indent' 8 times"
        if EinzugVermindern.IsEnabled then
            EinzugVermindern.Click
        else
            if i = 1 then Warnlog "Decrease Indent is disabled !"
        end if
        wait 500
    next i
    printlog " Enter '<Return>' 2 times"
    Call wTypeKeys ("<Return>", 2)
    Sleep 1

    printlog "- Objectbar ( normal ) : Increase Indent with Bullets"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    printlog " Select 'Bullets' in object bar"
    BulletsOnOff.Click
    Sleep 1
    for i=1 to 8
        printlog " Select 'Increase Indent' 8 times"
        if EinzugErhoehen.IsEnabled then
            EinzugErhoehen.Click
        else
            if i = 1 then Warnlog "Increase Indent is disabled !"
        end if
        wait 500
    next i

    printlog "- Objectbar ( normal ) : Decrease Indent with Bullets"
    for i=1 to 8
        printlog " Select 'Decrease Indent' 8 times"
        if EinzugVermindern.IsEnabled then
            EinzugVermindern.Click
        else
            if i = 1 then Warnlog "Decrease Indent is disabled !"
        end if
        wait 500
    next i

    Call wTypeKeys ("<Return>", 2)
    printlog " Enter '<Return>' 2 times"
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tNumFormatting
    printlog "- Objectbar Numbering : Numbering => Style, Font and Size"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    printlog " Select 'Numbering' in object bar"
    Numerierung.Click
    Sleep 1
    Call wTypeKeys ("Objectbar Numbering : Numbering => Style, Font and Size <Shift Home>")
    printlog " Enter some text in document and select it"
    Kontext "TextObjectbar"
    printlog "  - Style"
    printlog " Change 'Style' in object bar'"
    try
        Vorlage.Select 1
    catch
        Warnlog "Unable to select function in Toolbar"
        Call hCloseDocument
        goto endsub
    endcatch
    Sleep 1
    printlog "  - Font"
    printlog " Change 'Font Style' in object bar'"
    try
        Schriftart.Select 10
        Schriftart.TypeKeys "<Return>"
    catch
        Warnlog "Unable to select 'Font Style' in object bar"
    endcatch
    Sleep 1
    printlog "  - Font Size"
    printlog " Change 'Font Size' in object bar'"
    try
        Schriftgroesse.Select 10
        Schriftgroesse.TypeKeys "<Return>"
    catch
        Warnlog "Unable to select 'Font Sitze' in object bar"
    endcatch
    Sleep 1
    printlog " Enter '<Return>'"
    Call wTypeKeys ("<Return>")

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tNumLevelUpDown
    dim i as Integer

    printlog "- Objectbar Numbering : Numbering Level-Up and Down"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    printlog " Select 'Numbering' in object bar"
    Numerierung.Click
    Sleep 1

    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
    printlog "   - 11 times level down"
    Kontext "NumObjectbar"
    printlog " Select 11 times 'Level down' in object bar"
    try
        Hinunterstufen.Click
    catch
        Warnlog "Unable to select function in Toolbar"
        Call hCloseDocument
        goto endsub
    endcatch
    sleep (2)
    for i=1 to 9
        if Hinunterstufen.IsEnabled = True then
            Hinunterstufen.Click
        else
            if i = 1 then Warnlog "Level down is disabled!"
        end if
        sleep (2)
    next i

    printlog "   - 11 times level up"
    printlog " Select 11 times 'Level up' in object bar"
    for i=1 to 11
        if Hochstufen.IsEnabled then
            Hochstufen.Click
        else
            if i = 1 then Warnlog "Level up is disabled!"
        end if
        sleep (2)
    next i

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tNumSubpointsUpDown
    dim i as Integer

    printlog "- Objectbar Numbering : Numbering with subpoints up and down"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    Numerierung.Click
    printlog " Click 'Numbering' in Object bar"
    Call wTypeKeys ("Numbering <Return><Tab>", 2)
    Call wTypeKeys ("<Tab>Numbering<Up><Up><Up>")

    Kontext "NumObjectbar"
    if ( NumObjectbar.Exists( 1 ) ) then 
        ' all good
    else
        Call hToolbarSelect("BULLETSANDNUMBERING", true)
    endif

    printlog "   - 6 times subpoints down"
    printlog " click 6 times subpoints down in object bar"
    if HinunterstufenMitUnterpunkten.IsEnabled then
        HinunterstufenMitUnterpunkten.Click
    else
        if i = 1 then Warnlog "Move up with subpoints is disabled!"
    end if
    Sleep (1)

    for i=1 to 5
        if HinunterstufenMitUnterpunkten.IsEnabled then
            HinunterstufenMitUnterpunkten.Click
        else
            if i = 1 then Warnlog "Move down with subpoints is disabled!"
        end if
        Sleep (1)
    next i

    if HinunterstufenMitUnterpunkten.IsEnabled then
        Warnlog "Move down with subpoints is enabled though on Level 10"
    end if
    Sleep (1)

    printlog "   - 6 times subpoints up"
    printlog " click 6 times subpoints up in object bar"
    for i=1 to 6
        if HochstufenMitUnterpunkten.IsEnabled then
            HochstufenMitUnterpunkten.Click
        else
            if i = 1 then Warnlog "Move up with subpoints is disabled!"
        end if
        Sleep (1)
    next i

    if HochstufenMitUnterpunkten.IsEnabled then
        Warnlog "Move up with subpoints is enabled!"
    end if

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tNumMoveUpMoveDown
    dim i as Integer

    printlog "- Objectbar Numbering : Move Numbering up and down"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    Numerierung.Click
    printlog " Click 'Numbering' in object bar"
    Call wTypeKeys ("Numbering up <Return><Tab>", 3)
    Call wTypeKeys ("<Tab>Hallo<Return>")
    Call wTypeKeys ("<Shift Tab>", 5)
    Call wTypeKeys ("Numbering down <Up><Up>")
    Sleep 1

    printlog "   - 3 times move up"
    printlog " click 3 times move up in object bar"
    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
    try
        Hochschieben.Click
    catch
        Warnlog "Unable to select function in Toolbar"
        Call hCloseDocument
        goto endsub
    endcatch
    Hochschieben.Click

    printlog "   - 4 times move down"
    printlog " click 4 times move down in object bar"
    for i=1 to 4
        Hinunterschieben.Click
        Sleep 1
    next i

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tNumSubpointsMoveUpMoveDown
    dim i as Integer

    printlog "- Objectbar Numbering : Move Numbering with subpoints up and down"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    Numerierung.Click
    printlog " Click 'Numbering' in object bar"
    Call wTypeKeys ("hallo <Return><Tab>", 3)
    Call wTypeKeys ("<Tab>Hallo<Return>")
    Call wTypeKeys ("<Shift Tab>", 5)
    Call wTypeKeys ("Hallo <Up><Up>")
    Sleep 1

    printlog "   - move up 3 times"
    printlog " click 3 times move up in object bar"
    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
    try
        HochschiebenMitUnterpunkten.Click
    catch
        Warnlog "Unable to select function in Toolbar"
        Call hCloseDocument
        goto endsub
    endcatch
    HochschiebenMitUnterpunkten.Click

    printlog "   - move down 3 times"
    if Val(mid$(gMinor, 2, Len(gMinor))) >= 139 and Val(mid$(gMinor, 2, Len(gMinor))) <= 140 then
        QAErrorlog "Loop when moving numbered list up and down with subpoints"
    else
        printlog " click 3 times move down in object bar"
        for i=1 to 3
            HinunterschiebenMitUnterpunkten.Click
            Sleep 1
        next i
    end if

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tNumUnnumberedEntry
    printlog "- Objectbar Numbering: Paragraph without Numbering"

    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    Numerierung.Click
    printlog " Click 'Numbering' in object bar"
    Call wTypeKeys ("Paragraph without numbering")
    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
    try
        printlog " Click 'Insert Unnumbered Entry'"
        AbsatzOhne.Click
    catch
        Warnlog "Unable to select function in Toolbar"
    endcatch

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tNumRestartNumbering
    printlog "- Objectbar Numbering: Restart numbering"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    printlog " Click 'Numbering' in object bar"
    Numerierung.Click
    Call wTypeKeys ("Restart numbering<Return>", 3)
    printlog " Enter Text and hit <Return> 3 times"
    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
    try
        printlog " Click 'Restart Numbering' in object bar"
        NeuStarten.Click
    catch
        Warnlog "Unable to select function in Toolbar"
    endcatch

    Call wTypeKeys ("Restart Numbering<Return>", 3)
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tNumNumberingOff
    printlog "- Objectbar Numbering: Numbering off"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    printlog " Click 'Numbering' in object bar"
    Numerierung.Click
    Call wTypeKeys ("Numbering Off<Return>", 3)
    printlog " Enter Text and hit <Return> 3 times"
    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
    try
        printlog " Click 'Numbering off' in objectbar"
        Ausschalten.Click
    catch
        Warnlog "Unable to select function in Toolbar"
    endcatch

    Call wTypeKeys ("Numbering Off<Return>", 3)
    printlog " Enter Text and hit <Return> 3 times"
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tNumNumberingEnumerating
    printlog "- Objectbar Numbering: Numbering within a Numbering"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    printlog " Click 'Numbering' in object bar"
    Numerierung.Click
    Kontext
    try
        Active.Setpage TabBullet
        Kontext "TabBullet"
        Warnlog "For some unknown reason the Numbering/Bullets dialog has been opened!"
        TabBullet.Cancel
    catch
    endcatch
    Call wTypeKeys ("Numbering within a Numbering<Return>")
    printlog " Enter text and hit <Return>"

    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
    if NumObjectBar.IsDocked then NumObjectbar.Undock
    try
        printlog " Click 'Edit Numbering' in object bar"
        NumerierungAufzaehlung.Click
        wait 500
    catch
        Kontext
        try
            Active.Setpage TabBullet
            Kontext "TabBullet"
            Warnlog "For some unknown reason the Numbering/Bullets dialog is already opened!"
        catch
            Warnlog "Unable to select function in Toolbar"
            Call hCloseDocument
            goto endsub
        endcatch
    endcatch

    Kontext
    printlog " Step through all tabpages in 'Numbering/Bullets' dialog"
    Active.SetPage TabBullet
    Kontext "TabBullet"
    Call DialogTest ( TabBullet )
    Sleep 1

    Kontext
    Active.SetPage TabNumerierungsart
    Kontext "TabNumerierungsart"
    Call DialogTest ( TabNumerierungsart )
    Sleep 1

    Kontext
    Active.SetPage TabNumerierung
    Kontext "TabNumerierung"
    Call DialogTest ( TabNumerierung )
    Sleep 1

    Kontext
    Active.SetPage TabGrafiken
    Kontext "TabGrafiken"
    Call DialogTest ( TabGrafiken )
    Sleep 1

    Kontext
    Active.SetPage TabPositionNumerierung
    Kontext "TabPositionNumerierung"
    Call DialogTest ( TabPositionNumerierung )
    Sleep 1

    Kontext
    Active.SetPage TabOptionenNumerierung
    Kontext "TabOptionenNumerierung"
    Call DialogTest ( TabOptionenNumerierung )
    Sleep 1
    printlog " Close 'Numbering/Bullets' dialog"
    TabOptionenNumerierung.Cancel
    wait 500
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tUpFormatting
    printlog "- Objectbar Numbering : Bullets => Style, Font and -Size"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    printlog " Click 'Bullets on/off' in object bar"
    BulletsOnOff.Click
    Sleep 1
    printlog " Enter some text and select it"
    Call wTypeKeys ("Bullets => Style, Font and -Size <Shift Home>")
    Kontext "TextObjectbar"
    printlog "  - Style"
    try
        Vorlage.Select 1
    catch
        Warnlog "Unable to select style in Toolbar"
        Call hCloseDocument
        goto endsub
    endcatch
    printlog " Select 1st entry in 'Apply Style'"
    Sleep 5
    printlog "  - Font"
    printlog " Select 5th 'Font Name'"
    try
        Schriftart.Select 5
    catch
        Warnlog "Unable to select font in object bar"
    endcatch
    Schriftart.TypeKeys "<Return>"
    Sleep 1
    printlog "  - Font Size"
    printlog " Select 10th 'Font Size'"
    Schriftgroesse.Select 10
    Schriftgroesse.TypeKeys "<Return>"
    Sleep 1
    printlog " Hit <Return> in document"
    Call wTypeKeys ("<Return>")

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tUpLevelUpDown
    dim i as Integer

    printlog "- Objectbar Numbering : Bullets level up and down"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    printlog " Click 'Bullets on/off' in object bar"
    BulletsOnOff.Click
    Sleep 1
    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)

    printlog "   - 11 times level down"
    try
        Hinunterstufen.Click
    catch
        Warnlog "Unable to select function in Toolbar"
        Call hCloseDocument
        goto endsub
    endcatch
    sleep (2)
    for i=1 to 9
        printlog " Click 'Down One Level' in object bar 11 times"
        if Hinunterstufen.IsEnabled then
            Hinunterstufen.Click
        else
            if i = 1 then Warnlog "Level down is disabled !"
        end if
        sleep (2)
    next i

    printlog "   - 11 times level up"
    printlog " Click 'Up One Level' in object bar 11 times"
    for i=1 to 11
        if Hochstufen.IsEnabled then
            Hochstufen.Click
        else
            if i = 1 then Warnlog "Level up is disabled !"
        end if
        sleep (2)
    next i

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tUpSubpointsUpDown
    dim i as Integer

    printlog "- Objectbar Numbering : Bullets with Subpoints up and down"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    printlog " Click 'Bullets on/off' in object bar"
    BulletsOnOff.Click
    printlog " Enter some text, hit 'Return' and 'Tab' 2 times"
    Call wTypeKeys ("Bullets with Subpoints <Return><Tab>", 2)
    printlog " Hit 'Tab', enter some text, and hit 'Cursor Up' 3 times"
    Call wTypeKeys ("<Tab>up and down<Up><Up><Up>")
    Sleep 1

    printlog "   - down 6 times"
    printlog " Click 'Move Down with Subpoints' 11 times "
    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
    try
        HinunterstufenMitUnterpunkten.Click
    catch
        Warnlog "Unable to select function in Toolbar"
        Call hCloseDocument
        goto endsub
    endcatch
    Sleep 1

    for i=1 to 5
        if HinunterstufenMitUnterpunkten.IsEnabled then
            HinunterstufenMitUnterpunkten.Click
        else
            if i = 1 then Warnlog "Move with Subpoints down is disabled!"
        end if
        Sleep 1
    next i
    if HinunterstufenMitUnterpunkten.IsEnabled then
        Warnlog "Move with Subpoints down is enabled though on level 10!"
    end if

    printlog "   - up 6 times"
    for i=1 to 6
        printlog " Click 'Move Up with Subpoints' 11 times "
        if HochstufenMitUnterpunkten.IsEnabled then
            HochstufenMitUnterpunkten.Click
        else
            if i = 1 then Warnlog "Move with Subpoints up is disabled!"
        end if
        Sleep 1
    next i
    if HochstufenMitUnterpunkten.IsEnabled then
        Warnlog "Move with Subpoints up is enabled though on level 1!"
    end if

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tUpMoveUpMoveDown
    dim i as Integer

    printlog "- Objectbar Numbering : move bullets up and down"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    BulletsOnOff.Click
    printlog " Click 'Bullets on/off' in object bar"
    Call wTypeKeys ("move bullets <Return><Tab>", 3)
    printlog " enter some text hit <Return> and <Tab> 3 Times"
    Call wTypeKeys ("<Tab>up<Return>")
    printlog " Enter one word"
    Call wTypeKeys ("<Shift Tab>", 5)
    printlog " Hit <Tab> 5 Times"
    Call wTypeKeys ("and down <Up><Up>")
    printlog " enter text and hit <Cursor Up> 2 times"
    Sleep 1
    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
    printlog "   - 3 times up"
    try
        Hochschieben.Click
    catch
        Warnlog "Unable to select function in Toolbar"
        Call hCloseDocument
        goto endsub
    endcatch

    for i=1 to 2
        printlog " Click 'Move Up' in object bar 3 times"
        Hochschieben.Click
        Sleep 1
    next i

    printlog "   - 4 times down"
    for i=1 to 4
        printlog " Click 'Move Down' in object bar 4 times"
        Hochschieben.Click
        Sleep 1
    next i

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tUpSubpointsMoveUpMoveDown
    dim i as Integer

    printlog "- Objectbar Numbering : Move up and down bullets with subpoints"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    BulletsOnOff.Click
    printlog " Click 'Bullets on/off' in object bar"
    Call wTypeKeys ("move bullets <Return><Tab>", 3)
    printlog " enter some text hit <Return> and <Tab> 3 Times"
    Call wTypeKeys ("<Tab>up and down<Return>")
    printlog " Enter one word"
    Call wTypeKeys ("<Shift Tab>", 5)
    printlog " Hit <Tab> 5 Times"
    Call wTypeKeys ("with subpoints <Up><Up>")
    printlog " enter text and hit <Cursor Up> 2 times"
    Sleep 1
    printlog "   - move up 3 times"
    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
    try
        HochschiebenMitUnterpunkten.Click
    catch
        Warnlog "Unable to select function in Toolbar"
        Call hCloseDocument
        goto endsub
    endcatch

    printlog " Click 'Move Up with subpoints' 3 times"
    HochschiebenMitUnterpunkten.Click

    if Val(mid$(gMinor, 2, Len(gMinor))) >= 139 and Val(mid$(gMinor, 2, Len(gMinor))) <= 140 then
        QAErrorlog "Loop when moving numbered list up and down with subpoints"
    else
        printlog "   - move down 3 times"
        for i=1 to 3
            printlog " Click 'Move Down with subpoints' 3 times"
            HinunterschiebenMitUnterpunkten.Click
            Sleep 1
        next i
    end if

    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tUpUnnumberedEntry
    printlog "- Objectbar Numbering: Paragraph without Bullets"

    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    BulletsOnOff.Click
    printlog " Click 'Bullets on/off' in object bar"
    Call wTypeKeys ("Paragraph without Bullets")
    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)

    try
        printlog " Click 'Insert Unnumbered Entry' in object bar"
        AbsatzOhne.Click
    catch
        Warnlog "Unable to select function in Toolbar"
    endcatch
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tUpRestartNumbering
    printlog "- Objectbar Numbering: restart Numbering"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    BulletsOnOff.Click
    printlog " Click 'Bullets on/off' in object bar"
    Call wTypeKeys ("restart Numbering<Return>", 3)
    printlog " enter Text and hit <Return> 3 Times"
    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
    try
        printlog " Click 'Restart Numbering' in object bar"
        NeuStarten.Click
    catch
        Warnlog "Unable to select function in Toolbar"
    endcatch

    Call wTypeKeys ("restart Numbering<Return>", 3)
    printlog " enter Text and hit <Return> 3 Times"
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tUpNumberingOff
    printlog "- Objectbar Numbering: Numbering off"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    BulletsOnOff.Click
    printlog " Click 'Bullets on/off' in object bar"
    Call wTypeKeys ("Numbering off<Return>", 3)
    printlog " enter Text and hit <Return> 3 Times"
    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
    try
        printlog " Click 'Numbering off' in object bar"
        Ausschalten.Click
    catch
        Warnlog "Unable to select function in Toolbar"
    endcatch

    Call wTypeKeys ("Numbering off<Return>", 3)
    printlog " enter Text and hit <Return> 3 Times"
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tUpNumberingEnumerating
    printlog "- Objectbar Numbering: Numbering and Bullets within numbering"
    Call hNewDocument
    printlog " Open new document"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    BulletsOnOff.Click
    printlog " Click 'Numbering on/off' in object bar"
    Call wTypeKeys ("Numbering and Bullets within numbering<Return>")
    printlog " enter Text and hit <Return>"
    Kontext "NumObjectbar"
    if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
    try
        printlog " Click ' Edit Numbering' in object bar"
        NumerierungAufzaehlung.Click
    catch
        Warnlog "Unable to select function in Toolbar"
        Call hCloseDocument
        goto endsub
    endcatch

    Sleep 1
    printlog " In 'Numbering/Bullets' dialog step through all tabpages"
    Kontext
    Active.SetPage TabBullet
    Kontext "TabBullet"
    Call DialogTest ( TabBullet )

    Kontext
    Active.SetPage TabNumerierungsart
    Kontext "TabNumerierungsart"
    Call DialogTest ( TabNumerierungsart )

    Kontext
    Active.SetPage TabNumerierung
    Kontext "TabNumerierung"
    Call DialogTest ( TabNumerierung )

    Kontext
    Active.SetPage TabGrafiken
    Kontext "TabGrafiken"
    Call DialogTest ( TabGrafiken )

    Kontext
    Active.SetPage TabPositionNumerierung
    Kontext "TabPositionNumerierung"
    Call DialogTest ( TabPositionNumerierung )

    Kontext
    Active.SetPage TabOptionenNumerierung
    Kontext "TabOptionenNumerierung"
    Call DialogTest ( TabOptionenNumerierung )

    printlog " Close 'Numbering/Bullets' dialog"
    TabOptionenNumerierung.Cancel
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------

testcase tHtmlDocFormatting
    printlog "- Objectbar ( normal ) : Formatting"
    gApplication = "HTML"

    printlog " Open new document"
    Call hNewDocument

    Kontext "NavigatorWriter"
    if NavigatorWriter.Exists then ViewNavigator

    Call wTypeKeys ("Formatting <Shift Home>")
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )

    printlog "  - Style"
    try
        Vorlage.Select 1
    catch
        Warnlog "Style could not be changed in TextObjectbar"
    endcatch
    printlog " Choose first 'style' in object bar"
    Sleep 5
    printlog "  - Font"
    printlog " Select 5th 'Font' in object bar"
    try
        Schriftart.Select 5
    catch
        Warnlog "Unable to select font in object bar"
    endcatch
    Schriftart.TypeKeys "<Return>"
    Sleep 1
    printlog "  - Font Size"
    printlog " Select 10th 'Font Size' in object bar"
    try
        Schriftgroesse.Select 10
        Schriftgroesse.TypeKeys "<Return>"
    catch
        Warnlog "Uanble to change font size in toolbar"
    endcatch
    Sleep 1
    printlog "  - Bold"
    printlog " Select 'Bold' in object bar"
    Fett.Click
    Sleep 1
    printlog "  - Italic"
    printlog " Select 'Italic' in object bar"
    Kursiv.Click
    Sleep 1
    printlog "  - Underline"
    printlog " Select 'Underline' in object bar"
    Unterstrichen.Click
    Sleep 1
    printlog "  - Align Left"
    printlog " Select 'Align Left' in object bar"
    Linksbuendig.Click
    Sleep 1
    printlog "  - Align Right"
    printlog " Select 'Align Right' in object bar"
    Rechtsbuendig.Click
    Sleep 1
    printlog "  - Centered"
    printlog " Select 'Centered' in object bar"
    Zentriert.Click
    Sleep 1
    printlog "  - Font color"
    printlog " Select 'Font color' in object bar"
    Schriftfarbe.Click
    Sleep 1
    printlog "  - Highlighting"
    printlog " Select 'Highlighting' in object bar"
    Zeichenhintergrund.Click
    Sleep 1
    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tMasterDocFormatting
    printlog "- Objectbar ( normal ) : Formatting"
    gApplication = "MASTERDOCUMENT"

    printlog " Open new document"
    Call hNewDocument

    Kontext "NavigatorGlobalDoc"
    if NavigatorGlobalDoc.Exists then ViewNavigator


    Call wTypeKeys ("Formatting <Shift Home>")
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )

    printlog "  - Style"
    try
        Vorlage.Select 1
    catch
        Warnlog "Style could not be changed in TextObjectbar"
    endcatch
    printlog " Choose first 'style' in object bar"
    Sleep 5
    printlog "  - Font"
    printlog " Select 5th 'Font' in object bar"
    try
        Schriftart.Select 5
    catch
        Warnlog "Unable to select font in object bar"
    endcatch
    Schriftart.TypeKeys "<Return>"
    Sleep 1
    printlog "  - Font Size"
    printlog " Select 10th 'Font Size' in object bar"
    try
        Schriftgroesse.Select 10
        Schriftgroesse.TypeKeys "<Return>"
    catch
        Warnlog "Uanble to change font size in toolbar"
    endcatch
    Sleep 1
    printlog "  - Bold"
    printlog " Select 'Bold' in object bar"
    Fett.Click
    Sleep 1
    printlog "  - Italic"
    printlog " Select 'Italic' in object bar"
    Kursiv.Click
    Sleep 1
    printlog "  - Underline"
    printlog " Select 'Underline' in object bar"
    Unterstrichen.Click
    Sleep 1
    printlog "  - Align Left"
    printlog " Select 'Align Left' in object bar"
    Linksbuendig.Click
    Sleep 1
    printlog "  - Align Right"
    printlog " Select 'Align Right' in object bar"
    Rechtsbuendig.Click
    Sleep 1
    printlog "  - Centered"
    printlog " Select 'Centered' in object bar"
    Zentriert.Click
    Sleep 1
    printlog "  - Justify"
    printlog " Select 'Justify' in object bar (not in HTML)"
    Blocksatz.Click
    Sleep 1
    printlog "  - Font color"
    printlog " Select 'Font color' in object bar"
    Schriftfarbe.Click
    Sleep 1
    printlog "  - Highlighting"
    printlog " Select 'Highlighting' in object bar"
    Zeichenhintergrund.Click
    Sleep 1
    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------
