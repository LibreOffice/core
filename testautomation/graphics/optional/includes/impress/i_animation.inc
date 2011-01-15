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
'* short description :
'*
'*********************************************************************
' #1 tExtrasAnimation
'\********************************************************************

'    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
' Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\grafix3.ras")

testcase tExtrasAnimation

    Dim i
    Dim Zaehler
    Dim ZaehlerAnpassung


    Printlog "- Tools/Animation"
    printlog "New impress document "
    Call hNewDocument
    Call sSelectEmptyLayout
    sleep 2
    printlog "insert graphic file (sample.bmp) "
    InsertGraphicsFromFile
    sleep 1
    Kontext "GrafikEinfuegenDlg"
    sleep 2

    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
    sleep 2
    Oeffnen.Click
    sleep 2
    Kontext "DocumentImpress"
    printlog "Deselect graphic "
    DocumentImpress.MouseDoubleClick 90,90
    sleep 1
    InsertGraphicsFromFile
    sleep 2
    Kontext "GrafikEinfuegenDlg"
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\grafix3.ras")
    sleep 2
    Oeffnen.Click
    sleep 2
    Opl_SD_EffekteZulassen
    Kontext "Animation"

    printlog "add selected picture 10 times into the animation "
    for i=1 to 10
        BildAufnehmen.Click
    next i
    sleep 1
    printlog "compare frame number in animation dialog "
    if AnzahlBilder.GetText <> "10" Then
        WarnLog "  - Adding graphics did not work"
    else
        printlog "Pictures added"
    end if

    printlog "test if Add all is eneabled (shouldn't be because of only 1 object selected) "
    if AlleAufnehmen.IsEnabled Then WarnLog "  - Add all should not be enabled, there is only 1 graphic selected"
    Kontext "DocumentImpress"
    printlog "Select both pictures "
    EditSelectAll
    Kontext "Animation"
    printlog "Add all (5 times) "
    for i=1 to 5
        AlleAufnehmen.Click
    next i
    sleep 1
    printlog "check if number of frames is now 20 "
    if AnzahlBilder.GetText <> "20" Then
        WarnLog "  - Adding pics did not work"
    else
        printlog "All pics added"
    end if

    printlog "Click First Picture  "
    ErstesBild.Click

    printlog "Control if we are at frame 1 "
    if AnzahlBilder.GetText <> "1" Then
        WarnLog "  - Jump back to start did not work"
    else
        printlog "Jumped back to first picture"
    end if

    printlog "Go to last picture "
    LetztesBild.Click

    printlog "Control frame number "
    if AnzahlBilder.GetText <> "20" Then
        WarnLog "  - Jump to end did not work"
    else
        printlog "Jump to end did work"
    end if

    printlog "Play animation "
    Abspielen.Click
    sleep 3
    Kontext "DocumentImpress"
    printlog "Select all in document "
    EditSelectAll
    printlog "Delete content "
    DocumentImpress.TypeKeys "<DELETE>"

    Kontext "Animation"
    printlog "Create animation"
    Erstellen.Click
    sleep 10

    Kontext "DocumentImpress"
    try
        printlog "Try to copy ccreated animation into clipboard "
        EditCopy
        printlog "animation created"
    catch
        WarnLog "  - Animation not created"
    endcatch

    Kontext "Animation"

    if AnimationsgruppeGruppenobjekt.IsChecked=True Then
        try
            printlog "Try setting duration per frame "
            AnzeigedauerProBild.SetText "1"
            WarnLog "  - Edit field should be disabled"
        catch
            printlog "Edit field not enabled because groupobject status is = " + AnimationsgruppeGruppenobjekt.IsChecked
        endcatch
    end if


    printlog "check animation group object "
    AnimationsgruppeBitmapobjekt.Check

    printlog "Set duration per frame to 3"
    AnzeigedauerProBild.SetText "3"
    SetClipboard AnzeigedauerProBild.GetText
    printlog "Raise value for duration "
    AnzeigedauerProBild.More
    if AnzeigedauerProBild.GetText <> GetClipboardText Then
        printlog "Time per pic could be edited"
    else
        WarnLog "  - Time per pic could be edited"
    end if

    Zaehler = AnzahlDurchlaeufe.GetItemCount
    printlog "Change number of plays "
    for i=1 to Zaehler
    AnzahlDurchlaeufe.Select i
    next i
    printlog "Number of loops checked"

    printlog "change ZaehlerAnpassung "
    ZaehlerAnpassung=Anpassung.GetItemCount
    for i=1 to ZaehlerAnpassung
        Anpassung.Select i
    next i
    printlog "Loop count changed"

    printlog "Delete 1 frame "
    BildLoeschen.Click
    if AnzahlBilder = "20" Then
        WarnLog "  - Delete pictures from animation did not work"
    else
        printlog "Picture No20 deleted"
    end if

    printlog "Delete all frames "
    AlleLoeschen.Click
    Kontext "Active"
    Active.Yes
    sleep 3

    Kontext "Animation"
    if Abspielen.IsEnabled=true Then
        WarnLog "  - Not all pics could be deleted"
    else
        printlog "all pics deleted"
    end if
    sleep 2
    Animation.Close
    sleep 2
    printlog "close document"
    Call hCloseDocument
endcase
