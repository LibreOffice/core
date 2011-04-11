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
'* short description : Impress Required Test Library (11)
'*
'\*****************************************************************

sub m_011_

    call tiDiaLeiste()

end sub

'*******************************************************************************

testcase tiDiaLeiste

    ' only in IMPRESS
    dim sTemp as string
    Dim i,x as integer

    printlog " open application "
    Call hNewDocument
    sleep 1
    printlog " Change options so the presentation won't start from the current, but the first slide. "
    ToolsOptions
    hToolsOptions ( "IMPRESS", "General" )
    MitAktuellerSeite.Uncheck
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog " insert a graphic: global\input\graf_inp\desp.bmp) "
    Printlog "- Insert graphic from file so there is something for the slide mode"
    hGrafikEinfuegen ConvertPath (gTesttoolPath + "global\input\graf_inp\desp.bmp")
    sleep 3
    printlog "+ Insert->Slide "
    InsertSlide
    sleep 2
    hTypekeys "<Pagedown>"
    sleep 2
    printlog "+ insert a graphic: "global\input\graf_inp\desp.bmp") "
    hGrafikeinfuegen ConvertPath (gTesttoolPath + "global\input\graf_inp\desp.bmp")
    sleep 3
    printlog "+ View->Master View->Slides View "
    ViewWorkspaceSlidesView
    sleep 2
    '-------------------------------- Ueberblendeffekt ----------------------------------------
    Printlog "- Test blend effect"
    Kontext "SlideViewObjectbar"  ' CORRECT
    sleep 5

    if Ueberblendeffekt.GetItemCount <> 57 then warnlog "these are not 57: " + Ueberblendeffekt.GetItemCount
    printlog " select last entry 'Automatic (random)' in listbox 'Slide Effects' on object toolbar "
    Ueberblendeffekt.Select (Ueberblendeffekt.GetItemCount)
    Printlog " select last effect (random effect): " + Ueberblendeffekt.GetSelText

    '   Printlog Geschwindigkeit.GetRT  ' 341 listbox
    '   Printlog Diawechsel.GetRT       ' 341 listbox
    '   Printlog Zeit.GetRT             ' 353 spinfield
    '   Printlog DiasProReihe.GetRT     ' 353 spinfield
    '   Printlog PraesentationMitZeitnahme.GetRT doesn't work, but behave as button
    '   Printlog DiaAnzeigen.GetRT               doesn't work, but behave as button

    '-------------------------------- Geschwindigkeit -----------------------------------------
    Printlog "- Check different speed settings"
    Kontext "SlideViewObjectbar"
    printlog " select every item in list 'Transition Speed' "
    x = Geschwindigkeit.GetItemCount
    for i = 1 to x
        Geschwindigkeit.Select i
        Printlog "     changed to: " + Geschwindigkeit.GetSelText
    next i
    '-------------------------------- Diawechsel ----------------------------------------------
    Printlog "- Style of slide change"
    Kontext "SlideViewObjectbar"
    printlog " select every item in list 'Auto Transition' "
    x = Diawechsel.GetItemCount
    for i = 1 to x
        Diawechsel.Select i
        Printlog "     Changed to: " + Diawechsel.GetSelText + "; is time enabled ?: "+Zeit.IsEnabled
    next i
    '-------------------------------- Diawechsel Zeitintervall --------------------------------
    Printlog "- Zeitintervall testen"
    Kontext "SlideViewObjectbar"
    printlog " select last entry 'Automatic' in listbox 'Auto Transition' "
    Diawechsel.Select (Diawechsel.GetItemCount) ' automatic is usually the last one
    sleep 1
    if (Zeit.IsEnabled = FALSE) Then Warnlog "- Time should be editable, if automatic is chosen"
    printlog " type "15" into the field 'Time' "
    Zeit.SetText "15"
    sleep 1
    Printlog "    Time set to: " + Zeit.GetText
    '-------------------------------- Praesentation mit Zeitnahme ------------------------------
    Printlog "- Presentation with rehearsed timings"
    Kontext "SlideViewObjectbar"
    printlog " click button 'Rehearse Timings' "
    PraesentationMitZeitnahme.Click
    sleep 2
    Kontext "DocumentPresentation"
    if DocumentPresentation.exists (5) then
        sleep 5
        printlog " wait some seconds and click with mouse "
        DocumentPresentation.MouseDown 50,50
        DocumentPresentation.MouseUp 50,50
        sleep 2
        printlog " wait some seconds and click with mouse "
        DocumentPresentation.MouseDown 50,50
        DocumentPresentation.MouseUp 50,50
        sleep 3
    else
        warnlog "Didn't switch into presentation mode :-("
    endif
    Kontext "DocumentPresentation"
    if DocumentPresentation.exists (5) then
        warnlog "We are still in presentation mode :-( WHY!!!!???"
    endif
    '------------------------------- Dia anzeigen ja/nein --------------------------------------
    Printlog "- Show slide yes/no"
    Kontext "SlideViewObjectbar"
    printlog " click button 'Show/Hide Slide' "
    DiaAnzeigen.Click
    sleep 3
    printlog " click button 'Show/Hide Slide' "
    DiaAnzeigen.Click
    '------------------------------- Dias pro Reihe --------------------------------------------
    Printlog "- Presentation with rehearsed timings"
    Kontext "SlideViewObjectbar"
    sTemp = DiasProReihe.GetText
    printlog " press button 'less' in field 'Slides Per Row' "
    DiasProReihe.Less
    if sTemp = DiasProReihe.GetText then warnlog "   nothing changed (less)"
    sTemp = DiasProReihe.GetText
    printlog " press button 'more' in field 'Slides Per Row' "
    DiasProReihe.more
    if sTemp = DiasProReihe.GetText then warnlog "   nothing changed (more 1)"
    sTemp = DiasProReihe.GetText
    printlog " press button 'more' in field 'Slides Per Row' "
    DiasProReihe.more
    if sTemp = DiasProReihe.GetText then warnlog "   nothing changed (more 2)"
    kontext
    if active.exists then
        warnlog "active (1): '"+active.gettext+"'"
    endif
    printlog " Restore default settings in ToolsOptions "
    ToolsOptions
    hToolsOptions ( "IMPRESS", "General" )
    MitAktuellerSeite.Check
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    printlog " close application "
    Call hCloseDocument
    sleep 2

endcase 'tiDiaLeiste