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
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : Resource Test - Format Menu Part III
'*
'\***********************************************************************

sub c_upd_formatmenu3

    Printlog Chr(13) + "--------- Format Menu Part III (c_upd_formatmenu3.inc) ---------"
    
    call tFormatGraphicPositionAndSize
    call tFormatGraphicLine
    call tFormatGraphicArea
    call tFormatGraphicText
    call tFormatGraphicPoints

end sub

'-----------------------------------------------------------

testcase tFormatGraphicPositionAndSize
'///<u><b>Format - Graphic – Position and Size</b></u>

    '/// Load document from 'gTesttoolPath + "spreadsheet\required\input\twoobjects.ods"'
    printlog " Load document from 'gTesttoolPath + spreadsheet\required\input\twoobjects.ods"
    hFileOpenLocally( gTesttoolPath + "spreadsheet\required\input\twoobjects.ods" )
    '/// Select first object by Navigator
    printlog " Select first object by Navigator"
    Call fNavigatorSelectObject ("drawing objects", 1)
    '/// Open 'Position and Size' – dialog by 'Format – Graphic – Position and Size'
    printlog " Open 'Position and Size' – dialog by 'Format – Graphic – Position and Size'"
    FormatTransform
    
    printlog
    '/// Switch to tabpage 'Position and Size'
    printlog " Switch to tabpage 'Position and Size'"
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    Call DialogTest ( TabPositionAndSize )
    '/// Uncheck 'Position'
    printlog " Uncheck 'Position'"
    ProtectPosition.Uncheck
    '/// Check 'Size'
    printlog " Uncheck 'Size'"
    ProtectSize.Uncheck
    '/// Increase 'Position X'
    printlog " Increase 'Position X'"
    PositionX.More
    '/// Decrease 'Position Y'
    printlog " Decrease 'Position Y'"
    PositionY.Less
    '/// Increase 'Width'
    printlog " Increase 'Width'"
    Width.More
    '/// Decrease 'Height'
    printlog " Decrease 'Height'"
    Height.Less
    '/// Check 'Keep ratio'
    printlog " Check 'Keep ratio'"
    KeepRatio.Check
    
    printlog ""
    '/// Switch to tabpage 'Rotation'
    printlog " Switch to tabpage 'Rotation'"
    Kontext
    Active.SetPage TabDrehung
    Kontext "TabDrehung"
    Call DialogTest ( TabDrehung )
    '/// Increase 'Position X'
    printlog " Increase 'Position X'"
    PositionX.More
    '/// Decrease 'Position Y'
    printlog " Decrease 'Position Y'"
    PositionY.Less
    '/// Increase 'Angle'
    printlog " Increase 'Angle'"
    Winkel.More
    '/// Switch to tabpage 'Slant & Corner Radius'
    printlog " Switch to tabpage 'Slant & Corner Radius'"
    Kontext
    Active.SetPage TabSchraegstellen
    Kontext "TabSchraegstellen"
    Call DialogTest ( TabSchraegstellen )
    '/// Increase radius
    printlog " Increase radius"
    Radius.More
    '/// Increase angle
    printlog " Increase angle"
    Winkel.More
    '/// Close 'Position and Size' – dialog by 'Cancel'
    printlog " Close 'Position and Size' – dialog by 'Cancel'"
    TabSchraegstellen.Close
    '/// Close document
    printlog " Close document"
    call hCloseDocument
    hFileDelete( gLastWorkFile )

endcase

'-----------------------------------------------------------

testcase tFormatGraphicLine
'///<u><b>Format - Graphic - Line</b></u>

    Dim sTitle as string

    '/// Load document from 'gTesttoolPath + "spreadsheet\required\input\twoobjects.ods"'
    printlog " Load document from 'gTesttoolPath + spreadsheet\required\input\twoobjects.ods"
    hFileOpenLocally( gTesttoolPath & "spreadsheet\required\input\twoobjects.ods" )
    
    '/// Select first object by Navigator
    printlog " Select first object by Navigator"
    Call fNavigatorSelectObject ("drawing objects", 1)
    '/// Open 'Format – Line' dialog by 'Format – Graphic – Line'
    printlog " Open 'Format – Line' dialog by 'Format – Graphic – Line'"
    FormatLine
    '/// Switch to tabpage 'Line'
    printlog " Switch to tabpage 'Line'"
    Kontext
    active.setpage TabLinie
    Kontext "TabLinie"
    Call DialogTest ( TabLinie )
    '/// Verify that the 'Style' listbox for 'Line properties' is not empty
    printlog " Verify that the 'Style' listbox for 'Line properties' is not empty"
    If Stil.GetItemCount < 1 then
        warnlog "The listbox for styles is empty"
    end if
    '/// Verify that the 'Color' listbox for 'Line properties' is not empty
    printlog " Verify that the 'Color' listbox for 'Line properties' is not empty"
    If Farbe.GetItemCount < 1 then
        warnlog "The listbox for colors is empty"
    end if
    '/// Increase 'Width'
    printlog " Increase 'Width'"
    Breite.More
    '/// Increase 'Transparency'
    printlog " Increase 'Transparency'"
    Transparenz.More
    '/// Uncheck 'Synchronize ends'
    printlog " Uncheck 'Synchronize ends'"
    EndenSynchronisieren.Uncheck
    '/// Verify that the left 'Style' listbox for 'Arrow Styles' is not empty
    printlog " Verify that the left 'Style' listbox for 'Arrow Styles' is not empty"
    If StilLinks.GetItemCount < 1 then
        warnlog "The listbox for styles is empty"
    end if
    printlog "Choose second entry"
    StilLinks.Select 2
    '/// Verify that the right 'Style' listbox for 'Arrow Styles' is not empty
    printlog " Verify that the right 'Style' listbox for 'Arrow Styles' is not empty"
    If StilRechts.GetItemCount < 1 then
        warnlog "The listbox for styles is empty"
    end if
    printlog "Choose second entry"
    StilRechts.Select 2
    '/// Check the left 'Center' checkbox
    printlog " Check the left 'Center' checkbox"
    ZentriertLinks.Check
    '/// Check the right 'Center' checkbox
    printlog " Check the right 'Center' checkbox"
    ZentriertRechts.Check
    '/// Verify that the 'Style' listbox for 'Corner Style' is not empty
    printlog " Verify that the 'Style' listbox for 'Corner Style' is not empty"
    If CornerStyle.GetItemCount < 1 then
        warnlog "The listbox for styles is empty"
    end if
    
    printlog ""
    '/// Switch to tabpage 'Line Styles'
    printlog " Switch to tabpage 'Line Styles'"
    Kontext
    active.setpage TabLinienstile
    Kontext "TabLinienstile"
    Call DialogTest ( TabLinienstile )
    '/// Verify that the 'Line Style' listbox is not empty
    printlog " Verify that the 'Line Style' listbox is not empty"
    If Linienstil.GetItemCount < 1 then
        warnlog "The listbox for styles is empty"
    end if
    '/// Choose first entry in left 'Type' listbox
    printlog " Choose second entry in left 'Type' listbox"
    TypLinks.Select 2
    '/// Choose second entry in right 'Type' listbox
    printlog " Choose second entry in right 'Type' listbox"
    TypRechts.Select 2
    '/// Increase left 'Number'
    printlog " Increase left 'Number'"
    AnzahlLinks.More
    '/// Decrease right 'Number'
    printlog " Decrease right 'Number'"
    AnzahlRechts.Less
    '/// Increase left 'Length'
    printlog " Increase left 'Length'"
    LaengeLinks.More
    '/// Decrease right 'Length'
    printlog " Decrease right 'Length'"
    LaengeRechts.Less
    '/// Increase 'Spacing'
    printlog " Increase 'Spacing'"
    Abstand.More
    '/// Check 'Fit to line width'
    printlog " Check 'Fit to line width'"
    AnLinienbreite.Check
    '/// Press 'Add' button
    printlog " Press 'Add' button"
    Hinzufuegen.Click
    Kontext "NameDlg"
    Call DialogTest ( NameDlg )
    '/// Enter string 'Testlinestyle'
    printlog " Enter string 'Testlinestyle'"
    Eingabefeld.SetText "Testlinestyle"
    '/// Close 'Name' dialog with 'Cancel'
    printlog " Close 'Name' dialog with 'Cancel'"
    NameDlg.Cancel
    Kontext "TabLinienstile"
    '/// Press 'Modify' button
    printlog " Press 'Modify' button"
    Aendern.Click
    Kontext "NameDlg"
    '/// Close 'Name' dialog with 'Cancel'
    printlog " Close 'Name' dialog with 'Cancel'"
    NameDlg.Cancel
    Kontext "TabLinienstile"
    '/// Press 'Delete' button
    printlog " Press 'Delete' button"
    Loeschen.Click
    Kontext "Active"
    '/// Close messagebox with 'No'
    printlog " Close messagebox with 'No'"
    Active.No
    Kontext "TabLinienstile"
    '/// Press 'Open' button
    printlog " Press 'Open' button"
    Oeffnen.click
    Kontext "OeffnenDlg"
    '/// Close 'Open' dialog with 'Cancel'
    printlog " Close 'Open' dialog with 'Cancel'"
    OeffnenDlg.Cancel
    Kontext "TabLinienstile"
    '/// Press 'Save' button
    printlog " Press 'Save' button"
    Speichern.click
    Kontext "SpeichernDlg"
    '/// Close 'Save' dialog with 'Cancel'
    printlog " Close 'Save' dialog with 'Cancel'"
    SpeichernDlg.Cancel
    Kontext "TabLinienstile"
    
    printlog ""
    '/// Switch to tabpage 'Arrow Styles'
    printlog " Switch to tabpage 'Arrow Styles'"
    Kontext
    active.setpage TabLinienenden
    Kontext "Active"
    If Active.Exists then 
        Kontext "Linienstil"
        Linienstil.Cancel
    end if
    Kontext "TabLinienenden"
    Call DialogTest ( TabLinienenden )
    '/// Set 'Titel' to 'Test'
    printlog " Set 'Titel' to 'Test'"
    'To get the Office back to the default the title must be memorized for restoring
    sTitle = Textfeld.GetText
    Textfeld.SetText "Test"
    '/// Verify that the 'Arrow style' listbox is not empty
    printlog " Verify that the 'Arrow style' listbox is not empty"
    If Liste.GetItemCount < 1 then
        warnlog "The style listbox is empty"
    end if
    '/// Press 'Add' button
    printlog " Press 'Add' button"
    Hinzufuegen.click
    Kontext "NameDlg"
    '/// Enter string 'Testlinestyle'
    printlog " Enter string 'Testlinestyle'"
    Eingabefeld.SetText "Testlinestyle"
    '/// Close 'Name' dialog with 'Cancel'
    printlog " Close 'Name' dialog with 'Cancel'"
    NameDlg.Cancel
    Kontext "TabLinienenden"
    '/// Press 'Modify' button
    printlog " Press 'Modify' button"
    Aendern.Click
    '/// Press 'Modify' button again
    printlog " Press 'Modify' button again"
    Aendern.Click
    '/// Close messagebox with 'OK'
    printlog " Close messagebox with 'OK'"
    Kontext
    if Active.exists (2) then
        active.OK
    end if
    Kontext "NameDlg"
    '/// Enter original title to restore defaults
    printlog " Enter original title to restore defaults"
    Eingabefeld.SetText sTitle
    '/// Close 'Name' dialog with 'OK'
    printlog " Close 'Name' dialog with 'OK'"
    NameDlg.OK
    Kontext "TabLinienenden"
    '/// Press 'Delete' button
    printlog " Press 'Delete' button"
    Loeschen.Click
    Kontext "Active"
    '/// Close messagebox with 'No'
    printlog " Close messagebox with 'No'"
    active.no
    Kontext "TabLinienenden"
    '/// Press 'Open' button
    printlog " Press 'Open' button"
    Oeffnen.Click
    Kontext "OeffnenDLG"
    Kontext "Active"
    If Active.Exists then 
        Active.No
    end if
    Kontext "OeffnenDLG"
    '/// Close 'Open' dialog with 'Cancel'
    printlog " Close 'Open' dialog with 'Cancel'"
    OeffnenDLG.Cancel
    Kontext "TabLinienenden"
    '/// Press 'Save' button
    printlog " Press 'Save' button"
    Speichern.click
    Kontext "SpeichernDLG"
    '/// Close 'Save' dialog with 'Cancel'
    printlog " Close 'Save' dialog with 'Cancel'"
    SpeichernDLg.Cancel
    Kontext "TabLinienenden"
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    TabLinienenden.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument
    hFileDelete( gLastWorkFile )

endcase

'-----------------------------------------------------------

testcase tFormatGraphicArea
'///<u><b>Format -  - Graphic - Area</b></u>

    '/// Load document from 'gTesttoolPath + "spreadsheet\required\input\twoobjects.ods"'
    printlog " Load document from 'gTesttoolPath + spreadsheet\required\input\twoobjects.ods"
    hFileOpenLocally( gTesttoolPath & "spreadsheet\required\input\twoobjects.ods" )
    '/// Select first object by Navigator
    printlog " Select first object by Navigator"
    Call fNavigatorSelectObject ("drawing objects", 1)
    '/// Open 'Format – Area' dialog by 'Format – Area'
    printlog " Open 'Format – Area' dialog by 'Format – Area'"
    FormatArea
    
    printlog ""
    '/// Switch to tabpage 'Area'
    printlog " Switch to tabpage 'Area'"
    Kontext
    active.SetPage TabArea
    Kontext "TabArea" 
    '/// Choose first entry (None) in 'Fill' listbox
    printlog " Choose first entry (None) in 'Fill' listbox"
    FillOptions.Select 1
    Call DialogTest ( TabArea,1 )
    '/// Choose second entry (Color) in 'Fill' listbox
    printlog " Choose second entry (Color) in 'Fill' listbox"
    FillOptions.Select 2
    Call DialogTest ( TabArea,2 )
    '/// Verify that the 'Color' listbox has items
    printlog " Verify that the 'Color' listbox has items"
    If ColourList.GetItemCount < 1 then 
        warnlog "The color list is empty"
    end if
    '/// Choose third entry (Gradient) in 'Fill' listbox
    printlog " Choose third entry (Gradient) in 'Fill' listbox"
    FillOptions.Select 3
    Call DialogTest ( TabArea,3 )
    '/// Verify that the 'Gradient' listbox has items
    printlog " Verify that the 'Gradient' listbox has items"
    If GradientList.GetItemCount < 1 then 
        warnlog "The gradient list is empty"
    end if
    '/// Uncheck 'Automatic'
    printlog " Uncheck 'Automatic'"
    Automatic.UnCheck
    '/// Increase 'Increments'
    printlog " Increase 'Increments'"
    Increments.More
    '/// Choose fourth entry (Hatching) in 'Fill' listbox
    printlog " Choose fourth entry (Hatching) in 'Fill' listbox"
    FillOptions.Select 4
    Call DialogTest ( TabArea,4 )
    '/// Verify that the 'Hatching' listbox has items
    printlog " Verify that the 'Hatching' listbox has items"
    If HatchingList.GetItemCount < 1 then 
        warnlog "The hatching list is empty"
    end if
    '/// Check 'Background color'
    printlog " Check 'Background color'"
    BackgroundColour.Check
    '/// Verify that the 'Color' listbox has items
    printlog " Verify that the 'Color' listbox has items"
    If BackgroundColourList.GetItemCount < 1 then 
        warnlog "The color list is empty"
    end if
    '/// Choose fifth entry (Bitmaps) in 'Fill' listbox
    printlog " Choose fifth entry (Bitmaps) in 'Fill' listbox"
    FillOptions.Select 5
    Call DialogTest ( TabArea,5 )
    '/// Verify that the 'Bitmap' listbox has items
    printlog " Verify that the 'Bitmap' listbox has items"
    If BitmapList.GetItemCount < 1 then 
        warnlog "The bitmap list is empty"
    end if
    '/// Uncheck 'Original'
    printlog " Uncheck 'Original'"
    Original.UnCheck
    '/// Increase 'Width'
    printlog " Increase 'Width'"
    Width.More
    '/// Check 'Relative'
    printlog " Check 'Relative'"
    Relative.Check
    '/// Decrease 'Height'
    printlog " Decrease 'Height'"
    Height.Less
    '/// Check 'Tile'
    printlog " Check 'Tile'"
    Tile.Check
    '/// Increase 'X Offset'
    printlog " Increase 'X Offset'"
    XOffset.More
    '/// Increase 'Y Offset'
    printlog " Increase 'Y Offset'"
    YOffset.More
    '/// Uncheck 'Tile'
    printlog " Uncheck 'Tile'"
    Tile.UnCheck
    '/// Uncheck 'AutoFit'
    printlog " Uncheck 'AutoFit'"
    Stretch.UnCheck
    '/// Check 'Tile'
    printlog " Check 'Tile'"
    Tile.Check
    '/// Check 'Row'
    printlog " Check 'Row'"
    Row.Check
    '/// Check 'Column'
    printlog " Check 'Column'"
    Column.Check
    '/// Increase percentvalue
    printlog " Increase percentvalue"
    Offset.More

    printlog ""
    '/// Switch to tabpage 'Shadow'
    printlog " Switch to tabpage 'Shadow'"
    Kontext
    active.SetPage TabSchatten
    kontext "TabSchatten"
    Call DialogTest ( TabSchatten )
    '/// Check 'Use shadow'
    printlog " Check 'Use shadow'"
    Anzeigen.Check
    '/// Increase 'Distance'
    printlog " Increase 'Distance'"
    Entfernung.More
    '/// Verify that the 'Color' listbox has items
    printlog " Verify that the 'Color' listbox has items"
    if Farbe.getItemCount < 1 then
        warnlog "The listbox for colors is empty"
    end if
    '/// Increase 'Transparency'
    printlog " Increase 'Transparency'"
    Transparenz.More

    printlog ""
    '/// Switch to tabpage 'Transparency'
    printlog " Switch to tabpage 'Transparency'"
    Kontext
    active.SetPage TabTransparenz
    Kontext "TabTransparenz"
    Call DialogTest ( TabTransparenz )
    '/// Check 'Transparency'
    printlog " Check 'Transparency'"
    LineareTransparenz.Check
    '/// Increase percentvalue
    printlog " Increase percentvalue"
    MFLinTransparenz.More
    '/// Check 'Gradient'
    printlog " Check 'Gradient'"
    Transparenzverlauf.Check
    '/// Choose last entry (Square) for 'Type'
    printlog " Choose last entry (Square) for 'Type'"
    TransparenzverlaufTyp.Select 6
    '/// Increase 'Center X'
    printlog " Increase 'Center X'"
    MFZentrumX.More
    '/// Decrease 'Center Y'
    printlog " Decrease 'Center Y'"
    MFZentrumY.Less
    '/// Increase 'Angle'
    printlog " Increase 'Angle'"
    MFWinkel.More
    '/// Increase 'Border'
    printlog " Increase 'Border'"
    MFRand.More
    '/// Increase 'Start value'
    printlog " Increase 'Start value'"
    MFStartwert.More
    '/// Decrease 'End value'
    printlog " Decrease 'End value'"
    MFEndwert.Less

    printlog ""
    '/// Switch to tabpage 'Colors'
    printlog " Switch to tabpage 'Colors'"
    Kontext
    active.SetPage TabFarben
    Kontext "TabFarben"
    Call DialogTest ( TabFarben )
    '/// Enter colorname 'Color_added_by_Testtool'
    printlog " Enter colorname 'Color_added_by_Testtool'"
    FarbName.SetText "Color_added_by_Testtool"
    '/// Verify that the color listbox is not empty
    printlog " Verify that the color listbox is not empty"
    if Farbe.getItemCount < 1 then
        warnlog "The listbox for colors is empty"
    end if
    '/// Choose RGB
    printlog " Choose RGB"
    Farbmodell.Select 1
    '/// Increase 'R'
    printlog " Increase 'R'"
    R.More
    '/// Decrease 'G'
    printlog " Decrease 'G'"
    G.Less
    '/// Increase 'B'
    printlog " Increase 'B'"
    B.More
    '/// Choose 'CMYK'
    printlog " Choose 'CMYK'"
    Farbmodell.Select 2
    '/// Increase 'C'
    printlog " Increase 'C'"
    C.More
    '/// Decrease 'M'
    printlog " Decrease 'M'"
    M.Less
    '/// Increase 'Y'
    printlog " Increase 'Y'"
    Y.More
    '/// Decrease 'K'
    printlog " Decrease 'K'"
    K.Less
    '/// Press 'Add' button
    printlog " Press 'Add' button"
    Hinzufuegen.Click
    '/// Press 'Modify' button
    printlog " Press 'Modify' button"
    Aendern.Click
    '/// Press 'Edit' button
    printlog " Press 'Edit' button"
    Bearbeiten.Click
    Kontext "FarbeDlg"
    Call DialogTest ( FarbeDlg )
    '/// Increase 'Cyan'
    printlog " Increase 'Cyan'"
    Cyan.More
    '/// Decrease 'Magenta'
    printlog " Decrease 'Magenta'"
    Magenta.Less
    '/// Increase 'Yellow'
    printlog " Increase 'Yellow'"
    Yellow.More
    '/// Decrease 'Key'
    printlog " Decrease 'Key'"
    Key.Less
    '/// Increase 'Red'
    printlog " Increase 'Red'"
    Rot.More
    '/// Decrease 'Green'
    printlog " Decrease 'Green'"
    Gruen.Less
    '/// Increase 'Blue'
    printlog " Increase 'Blue'"
    Blau.More
    '/// Increase 'Hue'
    printlog " Increase 'Hue'"
    Farbton.More
    '/// Decrease 'Saturation'
    printlog " Decrease 'Saturation'"
    Saettigung.Less
    '/// Increase 'Brightness'
    printlog " Increase 'Brightness'"
    Hell.More
    '/// Press left arrow button
    printlog " Press left arrow button"
    Links.Click
    '/// Press right arrow button
    printlog " Press right arrow button"
    Rechts.Click
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    FarbeDlg.Cancel
    Kontext "TabFarben"
    '/// Press 'Delete' button
    printlog " Press 'Delete' button"
    Loeschen.Click
    '/// Close messagebox with 'Yes'
    printlog " Close messagebox with 'Yes'"
    Kontext "Active"
    Active.Yes
    Kontext "TabFarben"
    '/// Press 'Open' button to open 'File open' dialog
    printlog " Press 'Open' button to open 'File open' dialog"
    Oeffnen.click
    '/// A messagebox that the modified list is not saved appears. Click 'No'
    printlog " A messagebox that the modified list is not saved appears. Click 'No'"
    Kontext "Active"
    Active.No
    '/// Close 'Open' dialog with 'Cancel'
    printlog " Close 'Open' dialog with 'Cancel'"
    Kontext "OeffnenDLG"
    OeffnenDLG.Cancel
    Kontext "TabFarben"
    '/// Press 'Save' button to open 'Save as' dialog
    printlog " Press 'Save' button to open 'Save as' dialog"
    Speichern.Click
    Kontext "SpeichernDLG"
    '/// Close 'Save' dialog with 'Cancel'
    printlog " Close 'Save' dialog with 'Cancel'"
    SpeichernDLG.Cancel

    printlog ""
    '/// Switch to tabpage 'Gradients'
    printlog " Switch to tabpage 'Gradients'"
    Kontext
    active.SetPage TabFarbverlaeufe
    Kontext "TabFarbverlaeufe"
    '/// Choose last entry (Rectangular) in 'Type' listbox
    printlog " Choose last entry (Rectangular) in 'Type' listbox"
    Typ.Select 6
    '/// Increase 'Center X'
    printlog " Increase 'Center X'"
    ZentrumX.More
    '/// Decrease 'Center Y'
    printlog " Decrease 'Center Y'"
    ZentrumY.Less
    '/// Increase 'Angle'
    printlog " Increase 'Angle'"
    Winkel.More
    '/// Increase 'Border'
    printlog " Increase 'Border'"
    Rand.More
    '/// Choose second entry in 'From' listbox
    printlog " Choose second entry in 'From' listbox"
    Von.Select 2
    '/// Decrease percentvalue
    printlog " Decrease percentvalue"
    VonWert.Less
    '/// Choose fifth entry in 'To' listbox
    printlog " Choose fifth entry in 'To' listbox"
    Bis.Select 5
    '/// Decrease percentvalue
    printlog " Decrease percentvalue"
    BisWert.Less
    '/// Press 'Add' button to open 'Name' dialog
    printlog " Press 'Add' button to open 'Name' dialog"
    Hinzufuegen.Click
    Kontext "NameDlg"
    '/// Close 'name' dialog with 'Cancel'
    printlog " Close 'name' dialog with 'Cancel'"
    NameDlg.Cancel
    Kontext "TabFarbverlaeufe"
    '/// Press 'Modify' button to open 'Name' dialog
    printlog " Press 'Modify' button to open 'Name' dialog"
    Aendern.Click
    Kontext "NameDlg"
    '/// Close 'name' dialog with 'Cancel'
    printlog " Close 'name' dialog with 'Cancel'"
    NameDlg.Cancel
    Kontext "TabFarbverlaeufe"
    '/// Press 'Delete' button
    printlog " Press 'Delete' button"
    Loeschen.Click
    '/// Close messagebox with 'No'
    printlog " Close messagebox with 'No'"
    kontext
    active.no
    Kontext "TabFarbverlaeufe"
    '/// Press 'Open' button to open 'File open' dialog
    printlog " Press 'Open' button to open 'File open' dialog"
    Oeffnen.Click
    Kontext "OeffnenDLG"
    '/// Close 'Open' dialog with 'Cancel'
    printlog " Close 'Open' dialog with 'Cancel'"
    OeffnenDLG.Cancel
    Kontext "TabFarbverlaeufe"
    '/// Press 'Save' button to open 'Save as' dialog
    printlog " Press 'Save' button to open 'Save as' dialog"
    Speichern.Click
    Kontext "SpeichernDLG"
    '/// Close 'Save' dialog with 'Cancel'
    printlog " Close 'Save' dialog with 'Cancel'"
    SpeichernDLG.Cancel

    printlog ""
    '/// Switch to tabpage 'Hatching'
    printlog " Switch to tabpage 'Hatching'"
    Kontext
    active.SetPage TabSchraffuren
    Kontext "TabSchraffuren"
    '/// If a messagebox appears, close messagebox with 'Cancel'
    printlog " If a messagebox appears, close messagebox with 'Cancel'"
    Kontext
    Kontext "Active"
    If Active.Exists then 
        Kontext "Linienstil"
        Linienstil.Cancel
    end if
    Kontext "TabSchraffuren"
    '/// Verify that there are sample hatchings
    printlog " Verify that there are sample hatchings"
    If Tabelle.GetItemCount < 1 then
        warnlog "There are no hatchings available"
    end if
    '/// Decrease 'Spacing'
    printlog " Decrease 'Spacing'"
    Abstand.Less
    '/// Increase 'Angle'
    printlog " Increase 'Angle'"
    Winkel.More
    '/// Verify that the 'Line type' listbox has three entries
    printlog " Verify that the 'Line type' listbox has three entries"
    If Linientyp.GetItemCount < 1 then
        warnlog "There are no linetypes available"
    end if
    '/// Verify that the color listbox is not empty
    printlog " Verify that the color listbox is not empty"
    If Linienfarbe.GetItemCount < 1 then
        warnlog "There are no linecolors available"
    end if
    '/// Press 'Add' button to open 'Name' dialog
    printlog " Press 'Add' button to open 'Name' dialog"
    Hinzufuegen.Click
    '/// Close 'name' dialog with 'Cancel'
    printlog " Close 'name' dialog with 'Cancel'"
    Kontext "NameDlg"
    NameDlg.Cancel
    Kontext "TabSchraffuren"
    '/// Press 'Modify' button to open 'Name' dialog
    printlog " Press 'Modify' button to open 'Name' dialog"
    Aendern.Click
    '/// Close 'name' dialog with 'Cancel'
    printlog " Close 'name' dialog with 'Cancel'"
    Kontext "NameDlg"
    NameDlg.Cancel
    Kontext "TabSchraffuren"
    '/// Press 'Delete' button
    printlog " Press 'Delete' button"
    Loeschen.Click
    '/// Close messagebox with 'No'
    printlog " Close messagebox with 'No'"
    Kontext
    if Active.exists then 
        Active.no
    end if
    Kontext "TabSchraffuren"
    '/// Press 'Open' button to open 'File open' dialog
    printlog " Press 'Open' button to open 'File open' dialog"
    Oeffnen.Click
    Kontext "OeffnenDLG"
    '/// Close 'Open' dialog with 'Cancel'
    printlog " Close 'Open' dialog with 'Cancel'"
    OeffnenDLG.Cancel
    Kontext "TabSchraffuren"
    '/// Press 'Save' button to open 'Save as' dialog
    printlog " Press 'Save' button to open 'Save as' dialog"
    Speichern.Click
    '/// Close 'Save' dialog with 'Cancel'
    printlog " Close 'Save' dialog with 'Cancel'"
    Kontext "SpeichernDLG"
    SpeichernDLG.Cancel
    
    printlog ""
    '/// Switch to tabpage 'Bitmaps'
    printlog " Switch to tabpage 'Bitmaps'"
    Kontext
    active.SetPage TabBitmap
    Kontext "TabBitmap"
    '/// If a messagebox appears, close messagebox with 'Cancel'
    printlog " If a messagebox appears, close messagebox with 'Cancel'"
    Kontext
    Kontext "Active"
    If Active.Exists then 
        Kontext "Linienstil"
        Linienstil.Cancel
    end if
    Kontext "TabBitmap"
    Call DialogTest ( TabBitmap )
    '/// Verify that the listbox for 'Foreground color' is not empty
    printlog " Verify that the listbox for 'Foreground color' is not empty"
    If Vordergrund.GetItemCount < 1 then
        warnlog "There are no colors available"
    end if
    '/// Verify that the listbox for Background color' is not empty
    printlog " Verify that the listbox for Background color' is not empty"
    If Hintergrund.GetItemCount < 1 then
        warnlog "There are no colors available"
    end if
    '/// Verify that the listbox for Bitmaps is not empty
    printlog " Verify that the listbox for Bitmaps is not empty"
    If Tabelle.GetItemCount < 1 then
        warnlog "There are no Bitmaps available"
    end if
    '/// Press 'Add' button to open 'Name' dialog
    printlog " Press 'Add' button to open 'Name' dialog"
    Hinzufuegen.Click
    '/// Close 'name' dialog with 'Cancel'
    printlog " Close 'name' dialog with 'Cancel'"
    Kontext "NameDlg"
    NameDlg.Cancel
    Kontext "TabBitmap"
    '/// Press 'Modify' button to open 'Name' dialog
    printlog " Press 'Modify' button to open 'Name' dialog"
    Aendern.Click
    '/// Close 'name' dialog with 'Cancel'
    printlog " Close 'name' dialog with 'Cancel'"
    Kontext "NameDlg"
    NameDlg.Cancel
    Kontext "TabBitmap"
    '/// Press 'Import' button to open 'Import' dialog
    printlog " Press 'Import' button to open 'Import' dialog"
    Import.Click
    Kontext "GrafikEinfuegenDlg"
    '/// Close 'Import' dialog with 'Cancel'
    printlog " Close 'Import' dialog with 'Cancel'"
    GrafikEinfuegenDlg.Cancel
    Kontext "TabBitmap"
    '/// Press 'Delete' button
    printlog " Press 'Delete' button"
    Loeschen.Click
    '/// Close messagebox with 'No'
    printlog " Close messagebox with 'No'"
    Kontext
    if Active.exists then 
        Active.no
    end if
    Kontext "TabBitmap"
    '/// Press 'Open' button to open 'File open' dialog
    printlog " Press 'Open' button to open 'File open' dialog"
    Oeffnen.Click
    '/// If a messagebox appears, close messagebox with 'No'
    printlog " If a messagebox appears, close messagebox with 'No'"
    Kontext "OeffnenDLG"
    '/// Close 'Open' dialog with 'Cancel'
    printlog " Close 'Open' dialog with 'Cancel'"
    OeffnenDLG.Cancel
    Kontext "TabBitmap"
    '/// Press 'Save' button to open 'Save as' dialog
    printlog " Press 'Save' button to open 'Save as' dialog"
    Speichern.Click
    Kontext "SpeichernDLG"
    '/// Close 'Save' dialog with 'Cancel'
    printlog " Close 'Save' dialog with 'Cancel'"
    SpeichernDLG.Cancel

    printlog ""
    '/// Close 'Area' dialog with 'Cancel'
    printlog " Close 'Area' dialog with 'Cancel'"
    Kontext "TabBitmap"
    TabBitmap.Cancel

    '/// Close document
    printlog " Close document"
    Call hCloseDocument
    hFileDelete( gLastWorkFile )

endcase

'-----------------------------------------------------------

testcase tFormatGraphicText
'///<u><b>Format - Graphic - Text</b></u>

    '/// Load document from 'gTesttoolPath + "spreadsheet\required\input\twoobjects.ods"'
    printlog " Load document from 'gTesttoolPath + spreadsheet\required\input\twoobjects.ods"
    hFileOpenLocally( gTesttoolPath & "spreadsheet\required\input\twoobjects.ods" )
    '/// Select first object by Navigator
    printlog " Select first object by Navigator"
    Call fNavigatorSelectObject ("drawing objects", 1)
    '/// Open 'Format – Line' dialog by 'Format – Graphic – Line'
    printlog " Open 'Format – Text' dialog by 'Format – Graphic – Text'"
    FormatTextCalc
    '/// Switch to tabpage 'Text'
    printlog " Switch to tabpage 'Text'"
    Kontext
    Active.SetPage TabText
    Kontext "TabText"
    Call DialogTest ( TabText )
    '/// Verify that 'Fit width to text' is disabled
    printlog " Verify that 'Fit width to text' is disabled"
    If BreiteAnTextAnpassen.IsEnabled then
        warnlog "Fit width to text is enabled"
    end if
    '/// Verify that 'Fit heights to text' is disabled
    printlog " Verify that 'Fit heights to text' is disabled"
    If HoeheAnTextAnpassen.IsEnabled then
        warnlog "Fit heights to text is enabled"
    end if
    '/// Verify that 'Word wrap text in shape' is disabled
    printlog " Verify that 'Word wrap text in shape' is disabled"
    If WordWrap.IsEnabled then
        warnlog "Word wrap text in shape is enabled"
    end if
    '/// Verify that 'Resize shape to fit text' is disabled
    printlog " Verify that 'Resize shape to fit text' is disabled"
    If ResizeShape.IsEnabled then
        warnlog "Resize shape to fit textt is enabled"
    end if
    '/// Uncheck 'Fit to frame'
    printlog " Uncheck 'Fit to frame'"
    AmRahmenAnpassen.UnCheck
    '/// Uncheck 'Adjust to contour'
    printlog " Uncheck 'Adjust to contour'"
    Konturfluss.UnCheck
    '/// Increase 'Spacing Left'
    printlog " Increase 'Spacing Left'"
    Links.More
    '/// Increase 'Spacing Right'
    printlog " Increase 'Spacing Right'"
    Rechts.More
    '/// Increase 'Spacing Top'
    printlog " Increase 'Spacing Top'"
    Oben.More
    '/// Increase 'Spacing Bottom'
    printlog " Increase 'Spacing Bottom'"
    Unten.More
    '/// Check 'Full Width'
    printlog " Check 'Full Width'"
    GanzeBreite.Check
    
    '/// Switch to tabpage 'Text Animation'
    printlog " Switch to tabpage 'Text Animation'"
    Kontext
    Active.SetPage TabLauftext
    Kontext "TabLauftext"
    Call DialogTest ( TabLauftext )
    '/// Choose 'Effect' 4 (Scroll back and forth)
    Effekt.Select 4
    '/// Click button 'Up'
    printlog " Click button 'Up'"
    NachOben.Click
    '/// Click button 'Left'
    printlog " Click button 'Left'"
    NachLinks.Click
    '/// Click button 'Down'
    printlog " Click button 'Down'"
    NachUnten.Click
    '/// Click button 'Right'
    printlog " Click button 'Right'"
    NachRechts.Click
    '/// UnCheck 'Start inside'
    printlog " UnCheck 'Start inside'"
    TextSichtbarBeimStarten.UnCheck
    '/// UnCheck 'Text visible when exiting'
    printlog " UnCheck 'Text visible when exiting'"
    TextSichtbarBeimBeenden.UnCheck
    '/// Uncheck 'Continous'
    printlog " Uncheck 'Continous'"
    Endlos.UnCheck
    '/// Increase number
    printlog " Increase number"
    Anzahl.More
    '/// Check 'Pixels'
    printlog " Check 'Pixels'"
    Pixel.Check
    '/// Increase number
    printlog " Increase number"
    Schrittweite.More
    '/// Uncheck 'Automatic'
    printlog " Uncheck 'Automatic'"
    Automatisch.UnCheck
    '/// Increase time
    printlog " Increase time"
    Verzoegerung.More
    
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    TabLauftext.Cancel
    '/// Close document
    printlog " Close document"
    call hCloseDocument
    hFileDelete( gLastWorkFile )

endcase

'-----------------------------------------------------------

testcase tFormatGraphicPoints
'///<u><b>Format - Graphic - Points</b></u>

    '/// Load document from 'gTesttoolPath + "spreadsheet\required\input\twoobjects.ods"'
    printlog " Load document from 'gTesttoolPath + spreadsheet\required\input\twoobjects.ods"
    hFileOpenLocally( gTesttoolPath & "spreadsheet\required\input\twoobjects.ods" )
    '/// Select first object by Navigator
    printlog " Select first object by Navigator"
    Call fNavigatorSelectObject ("drawing objects", 1)
    '/// Toggle 'Points' by 'Format - Graphic - Points
    FormatPointsCalc
    FormatPointsCalc
    '/// Close document
    printlog " Close document"
    call hCloseDocument
    hFileDelete( gLastWorkFile )

endcase

'-----------------------------------------------------------

