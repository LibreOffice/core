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
'* short description : Test of the drawing tools
'*
'\***********************************************************************

sub w_drawing_tools1

    printLog Chr(13) + "- Test of the drawing tools 1"
    
    printLog Chr(13) + " -- Format/Position And Size"
    
    Call tCreateObjects_1            'Create objects from the drawing functions of the Toolbar
    
    Call tPositionAndSize_1          'check checkbox "protect position" of the object
    Call tPositionAndSize_2          'check Undo/Redo function of position assignment
    Call tPositionAndSize_3          'check "keep ratio" option
    Call tPositionAndSize_4          'check Undo/Redo function of size assignment
    Call tPositionAndSize_5          'check rotation angle of an object
    Call tPositionAndSize_6          'check corner radius of rectangles
    Call tPositionAndSize_7          'check corner radius of textboxes
    Call tPositionAndSize_8    
    
    printLog Chr(13) + " -- Format/line"
    'Dialog format/line/line
    Call tLine_1                     'check style, color, width and arrow styles . Test undo and redo .
    Call tLine_2                     'Check call Format.Line dialog via Button in object bar
    Call tLine_3                     'check line attributes are applied in object bar
    Call tLine_4                     'check  "synchronize ends"
    'Dialog format/line/line styles
    Call tline_5                     'check the attributes in format/line/line styles
    'Dialog format/line/arrow styles
    Call tline_6                     'check the attributes in format/line/arrow styles

end sub

'-------------------------------------------------------------------------

testcase tCreateObjects_1

    PrintLog "- Create objects from the drawing functions of the Toolbar , include ..."
    
    Call hNewDocument

    Call hToolbarSelect("DRAWING", true)
    
    PrintLog "- Draw a line"
    Kontext "Drawbar"
    Linie.Click
    gMouseMove(30,30,55,55)
    gMouseClick (75,5)
    Sleep 1
    
    PrintLog "- Draw a rectangle"
    Kontext "Drawbar"
    Rechteck.Click
    gMouseMove(35,35,40,40)
    gMouseClick (75,5)
    Sleep 1
    
    PrintLog "- Draw a ellipse (circle)"
    Kontext "Drawbar"
    Ellipse.Click
    Call gMouseMove(50,30,75,50)
    gMouseClick (75,5)
    Sleep 1
    
    PrintLog "- Draw textobject"
    Kontext "Drawbar"
    Textobjekt.Click
    gMouseMove(40,60,60,75)
    Sleep 1
    Kontext "DocumentWriter"
    Call wTypeKeys "This is StarOffice Test !"
    gMouseClick (75,5)
    
    PrintLog "- Draw callouts"
    Kontext "Drawbar"
    Legende.Click
    gMouseMove(30,80,50,95)
    gMouseClick (75,5)
    Sleep 1
    
    Kontext "Drawbar"
    Drawbar.Close
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase  tPositionAndSize_1

    Dim sPositionX as String
    Dim sPositionY as String
    Dim sPositionXMove as String
    Dim sPositionYMove as String
    
    sPositionX           = "3" + gSeperator + "00" + gMeasurementUnit
    sPositionY           = "3" + gSeperator + "00" + gMeasurementUnit
    sPositionXMove  = "5" + gSeperator + "00" + gMeasurementUnit
    sPositionYMove  = "5" + gSeperator + "00" + gMeasurementUnit
    
    PrintLog "- Check the checkbox 'protect position' of the object"
    
    Call hNewDocument
    
    PrintLog "- Insert a drawing object (rectangle)"
    Call wZeichenobjektEinfuegen("RECHTECK", 30, 40, 60, 60)
    Call gMouseClick(40,50)
    
    printLog "when 'protect position' is Checked"
    PrintLog "- Format/Position and size /Position and size ,"
    PrintLog "- + set positionX and positionY , and Check 'postion' in protect area"
    PrintLog "- + close the dialogue  and reopen it"
    PrintLog "- check wether the rectangle can be removed when 'protect position' is Checked"
    fPositionAndSize("TabPositionAndSizeWriter")
    
    Horizontalby.Settext "3"
    Verticalby.Settext "3"
    ProtectPosition.Check
    
    TabPositionAndSizeWriter.OK
    Sleep 1
    
    Kontext "DocumentWriter"
    Call wTypeKeys "<Right>",2
    Call wTypeKeys "<Down>" ,2
    Sleep 1
    
    fPositionAndSize("TabPositionAndSizeWriter")
    
    if Horizontalby.Gettext <> sPositionX then Warnlog "X-Position is wrong: Not -> "+sPositionX+"  but -> "+Horizontalby.Gettext
    if Verticalby.Gettext <> sPositionY then Warnlog "Y-Position is wrong: Not -> "+sPositionY+"  but -> "+Verticalby.Gettext
    
    ProtectPosition.UnCheck
    TabPositionAndSizeWriter.OK
    
    PrintLog "- check wether the rectangle can be removed when 'protect position' is UnChecked"
    Kontext "DocumentWriter"
    Call wTypeKeys "<Right>",2
    Call wTypeKeys "<Down>" ,2
    Sleep 1
    
    fPositionAndSize("TabPositionAndSizeWriter")
    
    if Horizontalby.Gettext <> sPositionXMove then Warnlog "X-Position is wrong: Not -> "+sPositionXMove+"  but -> "+Horizontalby.Gettext
    if Verticalby.Gettext <> sPositionYMove then Warnlog "Y-Position is wrong: Not -> "+sPositionYMove+"  but -> "+Verticalby.Gettext
    
    TabPositionAndSizeWriter.OK
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase  tPositionAndSize_2

    Dim sPositionX as String
    Dim sPositionY as String
    Dim sPositionXMove as String
    Dim sPositionYMove as String
    
    sPositionX           = "3" + gSeperator + "00" + gMeasurementUnit
    sPositionY           = "3" + gSeperator + "00" + gMeasurementUnit
    sPositionXMove  = "4" + gSeperator + "00" + gMeasurementUnit
    sPositionYMove  = "4" + gSeperator + "00" + gMeasurementUnit
    
    PrintLog "- check Undo/Redo function of position assignment"
    
    Call hNewDocument
    
    PrintLog "- Insert a drawing object (rectangle)"
    Call wZeichenobjektEinfuegen("RECHTECK", 30, 40, 60, 60)
    Call gMouseClick(40,50)
    
    fPositionAndSize("TabPositionAndSizeWriter")
    Horizontalby.Settext "3"
    Verticalby.Settext "3"
    TabPositionAndSizeWriter.OK
    Sleep 1
    
    Kontext "DocumentWriter"
    Call wTypeKeys "<Right>"
    Call wTypeKeys "<Down>"
    Sleep 1
    
    PrintLog "- check undo function"
    EditUndo
    Sleep 1
    EditUndo
    Sleep 1
    
    fPositionAndSize("TabPositionAndSizeWriter")
    if Horizontalby.Gettext <> sPositionX then Warnlog "X-Position is wrong: Not -> "+sPositionX+"  but -> "+Horizontalby.Gettext
    if Verticalby.Gettext <> sPositionY then Warnlog "Y-Position is wrong: Not -> "+sPositionY+"  but -> "+Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    PrintLog "- check redo function"
    EditRedo
    Sleep 1
    EditRedo
    Sleep 1
    
    fPositionAndSize("TabPositionAndSizeWriter")
    if Horizontalby.Gettext <> sPositionXMove then Warnlog "X-Position is wrong: Not -> "+sPositionXMove+"  but -> "+Horizontalby.Gettext
    if Verticalby.Gettext <> sPositionYMove then Warnlog "Y-Position is wrong: Not -> "+sPositionYMove+"  but -> "+Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase  tPositionAndSize_3

    Dim sWidth as String
    Dim sHeight as String
    Dim sWidthChange as String
    Dim sHeightChange as String
    
    sHeightChange  = "8" + gSeperator + "00" + gMeasurementUnit
    sWidthChange   = "6" + gSeperator + "00" + gMeasurementUnit
    
    PrintLog "- check 'keep ratio' option"
    
    Call hNewDocument
    
    PrintLog "- Insert a drawing object (rectangle)"
    Call wZeichenobjektEinfuegen("RECHTECK", 30, 40, 60, 60)
    Call gMouseClick(40,50)
    
    fPositionAndSize("TabPositionAndSizeWriter")
    
    PrintLog "- Check when ratio is 5:4"
    Width.Settext  "5"
    Height.Settext "4"
    
    KeepRatio.Check
    Width.Settext "10"
    Sleep 2
    if Height.GetText <> sHeightChange then Warnlog "Height is wrong: Not -> "+sHeightChange+"  but -> "+Height.GetText
    
    KeepRatio.UnCheck
    PrintLog "- Check when ratio is 3:5"
    Width.Settext  "3"
    Height.Settext "5"
    
    KeepRatio.Check
    Height.Settext "10"
    Sleep 2
    if Width.GetText <> sWidthChange then Warnlog "Width is wrong: Not -> "+sWidthChange+"  but -> "+Width.GetText
    
    'Recover to the default
    KeepRatio.UnCheck
    TabPositionAndSizeWriter.OK
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase  tPositionAndSize_4

    Dim sWidth as String
    Dim sHeight as String
    Dim sWidthChange as String
    Dim sHeightChange as String
    
    sWidth             = "5" + gSeperator + "00" + gMeasurementUnit
    sHeight            = "4" + gSeperator + "00" + gMeasurementUnit
    sWidthChange  = "8" + gSeperator + "00" + gMeasurementUnit
    sHeightChange = "6" + gSeperator + "00" + gMeasurementUnit
    
    PrintLog "- check Undo/Redo function of size assignment"
    
    Call hNewDocument
    
    PrintLog "- Insert a drawing object (rectangle) , and set the first Size"
    Call wZeichenobjektEinfuegen("RECHTECK", 30, 40, 60, 60)
    Call gMouseClick(40,50)
    
    fPositionAndSize("TabPositionAndSizeWriter")
    Width.Settext  sWidth
    Height.Settext sHeight
    TabPositionAndSizeWriter.OK
    Sleep 1
    
    PrintLog "- Set the second Size."
    fPositionAndSize("TabPositionAndSizeWriter")
    Width.Settext  sWidthChange
    Height.Settext sHeightChange
    TabPositionAndSizeWriter.OK
    Sleep 1
    
    PrintLog "- check undo function"
    EditUndo
    Sleep 1
    fPositionAndSize("TabPositionAndSizeWriter")
    if Width.GetText  <> sWidth  then Warnlog "Width is wrong: Not -> "+sWidth+"  but -> "+Width.GetText
    if Height.GetText <> sHeight then Warnlog "Height is wrong: Not -> "+sHeight+"  but -> "+Height.GetText
    TabPositionAndSizeWriter.Cancel
    
    PrintLog "- check redo function"
    EditRedo
    Sleep 1
    
    fPositionAndSize("TabPositionAndSizeWriter")
    if Width.GetText  <> sWidthChange  then Warnlog "Width is wrong: Not -> "+sWidthChange+"  but -> "+Width.GetText
    if Height.GetText <> sHeightChange then Warnlog "Height is wrong: Not -> "+sHeightChange+"  but -> "+Height.GetText
    TabPositionAndSizeWriter.Cancel
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase  tPositionAndSize_5

    PrintLog "- check rotation angle of an object"

    Call hNewDocument

    PrintLog "- Insert a drawing object (rectangle) , and set its rotation angle"
    Call wZeichenobjektEinfuegen("RECHTECK", 30, 40, 60, 60)
    Call gMouseClick(40,50)

    Call fPositionAndSize("TabDrehung")
    Winkel.SetText "45"
    TabDrehung.OK
    Sleep 1

    Call fPositionAndSize("TabDrehung")
    if Left$(Winkel.GetText,5) <> "45" + gSeperator + "00" then
        Warnlog "The rotation angle isn't corroct! It should be 45" & gSeperator & "00, but get " + Left$(Winkel.GetText, 5)
    end if
    TabDrehung.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase  tPositionAndSize_6

    Dim sRadius as String
    
    sRadius = "0" + gSeperator + "50" + gMeasurementUnit
    
    PrintLog "- check corner radius of rectangles"
    
    Call hNewDocument
    
    PrintLog "- Insert a rectangle , and set its corner radius."
    Call wZeichenobjektEinfuegen("RECHTECK", 30, 40, 60, 60)
    Call gMouseClick(40,50)
    
    fPositionAndSize("TabSchraegstellen")
     Radius.SetText sRadius
    TabSchraegstellen.OK
    Sleep 1
    
    fPositionAndSize("TabSchraegstellen")
     if Radius.GetText <> sRadius then Warnlog "The Radius isn't corroct! It should be " +sRadius + " , but was " +Radius.GetText
    TabSchraegstellen.Cancel
    
    Call hCloseDocument
    
endcase

'-------------------------------------------------------------------------

testcase  tPositionAndSize_7

    Dim sRadius as String
    
    sRadius = "0" + gSeperator + "50" + gMeasurementUnit
    
    PrintLog "- check corner radius of textboxes."
    
    Call hNewDocument
    
    PrintLog "- Insert a rectangle , and set its corner radius"
    
    Call wZeichenobjektEinfuegen("TEXT", 30, 40, 60, 60)
    Call wTypeKeys "The test for corner radius of textboxes..."
    Kontext "DocumentWriter"
    Call wTypeKeys "<ESCAPE>"
    
    fPositionAndSize("TabSchraegstellen")
     Radius.SetText sRadius
    TabSchraegstellen.OK
    Sleep 1
    
    fPositionAndSize("TabSchraegstellen")
     if Radius.GetText <> sRadius then Warnlog "The Radius isn't corroct! It should be " +sRadius + " ,but get " +Radius.GetText
    TabSchraegstellen.Cancel
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase  tPositionAndSize_8

    Dim sPositionStartX as String
    Dim sPositionStartY as String
    Dim sWidth as String
    Dim sHeight as String
    Dim sPivotDefaultX as String
    Dim sPivotDefaultY as String
    Dim sPivotStartX as String 
    Dim sPivotStartY as String
    Dim sPivotEndX  as String
    Dim sPivotEndY as String
    
    sPositionStartX   = "3" + gSeperator + "00" + gMeasurementUnit
    sPositionStartY   = "3" + gSeperator + "00" + gMeasurementUnit
    sWidth            = "4" + gSeperator + "00" + gMeasurementUnit
    sHeight           = "3" + gSeperator + "00" + gMeasurementUnit
    
    sPivotDefaultX    = "5" + gSeperator + "00" + gMeasurementUnit
    sPivotDefaultY    = "4" + gSeperator + "50" + gMeasurementUnit
    sPivotStartX      = "3" + gSeperator + "50" + gMeasurementUnit
    sPivotStartY      = "2" + gSeperator + "50" + gMeasurementUnit
    sPivotEndX        = "6" + gSeperator + "50" + gMeasurementUnit
    sPivotEndY        = "6" + gSeperator + "50" + gMeasurementUnit
    
    PrintLog "- check 'keep ratio' option"
    
    Call hNewDocument
    
    PrintLog "- Insert a drawing object (rectangle)"
    Call wZeichenobjektEinfuegen("RECHTECK", 30, 40, 60, 60)
    Call gMouseClick(40,50)
    
    fPositionAndSize("TabPositionAndSizeWriter")
    Width.Settext        sWidth
    Height.Settext       sHeight
    Horizontalby.Settext sPositionStartX
    Verticalby.Settext   sPositionStartY
    TabPositionAndSizeWriter.OK
    Sleep 1
    fPositionAndSize("TabDrehung")
    Winkel.SetText "270"
    TabDrehung.OK
    Sleep 1
    
    fPositionAndSize("TabDrehung")
    if PositionX.Gettext <> sPivotDefaultX then Warnlog "X-Position is wrong: Not -> "+sPivotDefaultX+"  but -> "+PositionX.Gettext
    if PositionY.Gettext <> sPivotDefaultY then Warnlog "Y-Position is wrong: Not -> "+sPivotDefaultY+"  but -> "+PositionY.Gettext
    
    Drehpunkt.TypeKeys "<Up>"
    Drehpunkt.TypeKeys "<Left>"
    
    if PositionX.Gettext <> sPivotStartX then Warnlog "X-Position is wrong: Not -> "+sPivotStartX+"  but -> "+PositionX.Gettext
    if PositionY.Gettext <> sPivotStartY then Warnlog "Y-Position is wrong: Not -> "+sPivotStartY+"  but -> "+PositionY.Gettext
    
    Drehpunkt.TypeKeys "<Right>" , 2
    Drehpunkt.TypeKeys "<Down>"  , 2
    
    if PositionX.Gettext <> sPivotEndX then Warnlog "X-Position is wrong: Not -> "+sPivotEndX+"  but -> "+PositionX.Gettext
    if PositionY.Gettext <> sPivotEndY then Warnlog "Y-Position is wrong: Not -> "+sPivotEndY+"  but -> "+PositionY.Gettext
    TabDrehung.Cancel
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase  tLine_1

    Dim iLineStyle as Integer
    Dim iColor as Integer
    Dim iArrowStyleLeft as Integer
    Dim iArrowStyleRight as Integer
    Dim sLineWidth as String
    Dim sTransparency as String
    Dim sArrowWidthLeft as String
    Dim sArrowWidthRight as String
    
    Dim iLineStyleDefault as Integer
    Dim iColorDefault as Integer
    Dim iArrowStyleLeftDefault as Integer 
    Dim iArrowStyleRightDefault as Integer
    Dim sLineWidthDefault as String
    Dim sTransparencyDefault as String
    Dim sArrowWidthLeftDefault as String
    Dim sArrowWidthRightDefault as String

    sTransparency    = "25%"
    iLineStyle       = 3
    iColor           = 7
    iArrowStyleLeft  = 2
    iArrowStyleRight = 3
    sLineWidth       = "0" + gSeperator + "10" + gMeasurementUnit
    sArrowWidthLeft  = "0" + gSeperator + "55" + gMeasurementUnit
    sArrowWidthRight = "0" + gSeperator + "60" + gMeasurementUnit
    
    PrintLog "- check style, color, width and arrow styles"
    
    Call hNewDocument
    
    PrintLog "- Insert a line"
    Call wZeichenobjektEinfuegen("LINIE", 40,40 ,  60,50 , false)
    Sleep 2
    Call gMouseClick(40,40)
    Sleep 2
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    PrintLog "- modify Style ,color , width and arrow styles"
    fFormatLine("TabLinie")
    
    iLineStyleDefault       = Stil.GetSelIndex
    iColorDefault           = Farbe.GetSelIndex
    slineWidthDefault       = Breite.GetText
    sTransparencyDefault    = Transparenz.GetText
    iArrowStyleLeftDefault  = StilLinks.GetSelIndex
    iArrowStyleRightDefault = StilRechts.GetSelIndex
    sArrowWidthLeftDefault  = BreiteLinks.GetText
    sArrowWidthRightDefault = BreiteRechts.GetText
    
    PrintLog "- Line properties , include :"
    PrintLog "- - Style"
    Stil.Select iLineStyle
    wait 500
    PrintLog "- - Color"
    Farbe.Select iColor
    wait 500
    PrintLog "- - Width"
    Breite.SetText sLineWidth
    wait 500
    PrintLog "- - Transparency"
    Transparenz.SetText sTransparency
    wait 500
    
    PrintLog "- Arrow styles , include :"
    PrintLog "- - Synchronize ends"
    EndenSynchronisieren.UnCheck
    PrintLog "- - Style (left and right)"
    StilLinks.Select  iArrowStyleLeft
    StilRechts.Select iArrowStyleRight
    wait 500
    PrintLog "- - Width (left and right)"
    BreiteLinks.SetText  sArrowWidthLeft
    BreiteRechts.SetText sArrowWidthRight
    PrintLog "- - Center (left and right)"
    ZentriertLinks.Check
    ZentriertRechts.Check
    
    TabLinie.OK
    
    PrintLog "- Check all changes"
    fFormatLine("TabLinie")
    
    if Stil.GetSelIndex    <> iLineStyle    then Warnlog "Line style is wrong."
    if Farbe.GetSelIndex   <> iColor        then Warnlog "Line color is wrong."
    if Breite.GetText      <> sLineWidth    then Warnlog "Line width is wrong."
    if Transparenz.GetText <> sTransparency then Warnlog "Line transparency is wrong."
    
    if EndenSynchronisieren.IsChecked = TRUE         then Warnlog "Synchronize ends should NOT be checked."
    if StilLinks.GetSelIndex     <> iArrowStyleLeft  then Warnlog "The left arrow style is wrong."
    if StilRechts.GetSelIndex    <> iArrowStyleRight then Warnlog "The right arrow style is wrong."
    if BreiteLinks.GetText       <> sArrowWidthLeft  then Warnlog "The left arrow width is wrong."
    if BreiteRechts.GetText      <> sArrowWidthRight then Warnlog "The right arrow width is wrong."
    if ZentriertLinks.IsChecked  <> TRUE             then Warnlog "The left 'center' should be checked"
    if ZentriertRechts.IsChecked <> TRUE             then Warnlog "The right 'center' should be checked"
    TabLinie.Cancel
    
    PrintLog "- Test Undo function"
    Kontext "DocumentWriter"
    EditUndo
    Sleep 1
    
    fFormatLine("TabLinie")
    if Stil.GetSelIndex    <> iLineStyleDefault    then Warnlog "Line style is wrong."
    if Farbe.GetSelIndex   <> iColorDefault        then Warnlog "Line color is wrong."
    if Breite.GetText      <> sLineWidthDefault    then Warnlog "Line width is wrong."
    if Transparenz.GetText <> sTransparencyDefault then Warnlog "Line transparency is wrong."
    
    if EndenSynchronisieren.IsChecked = TRUE                then Warnlog "Synchronize ends should NOT be checked."
    if StilLinks.GetSelIndex     <> iArrowStyleLeftDefault  then Warnlog "The left arrow style is wrong."
    if StilRechts.GetSelIndex    <> iArrowStyleRightDefault then Warnlog "The right arrow style is wrong."
    if BreiteLinks.GetText       <> sArrowWidthLeftDefault  then Warnlog "The left arrow width is wrong."
    if BreiteRechts.GetText      <> sArrowWidthRightDefault then Warnlog "The right arrow width is wrong."
    if ZentriertLinks.IsChecked  = TRUE                     then Warnlog "The left 'center' should be checked"
    if ZentriertRechts.IsChecked = TRUE                     then Warnlog "The right 'center' should be checked"
    TabLinie.Cancel
    
    PrintLog "- Test Redo function"
    Kontext "DocumentWriter"
    EditRedo
    Sleep 1
    
    fFormatLine("TabLinie")
    if Stil.GetSelIndex    <> iLineStyle    then Warnlog "Line style is wrong."
    if Farbe.GetSelIndex   <> iColor        then Warnlog "Line color is wrong."
    if Breite.GetText      <> sLineWidth    then Warnlog "Line width is wrong."
    if Transparenz.GetText <> sTransparency then Warnlog "Line transparency is wrong."
    
    if EndenSynchronisieren.IsChecked = TRUE         then Warnlog "Synchronize ends should NOT be checked."
    if StilLinks.GetSelIndex     <> iArrowStyleLeft  then Warnlog "The left arrow style is wrong."
    if StilRechts.GetSelIndex    <> iArrowStyleRight then Warnlog "The right arrow style is wrong."
    if BreiteLinks.GetText       <> sArrowWidthLeft  then Warnlog "The left arrow width is wrong."
    if BreiteRechts.GetText      <> sArrowWidthRight then Warnlog "The right arrow width is wrong."
    if ZentriertLinks.IsChecked  <> TRUE             then Warnlog "The left 'center' should be checked"
    if ZentriertRechts.IsChecked <> TRUE             then Warnlog "The right 'center' should be checked"
    TabLinie.Cancel
    
    Call hCloseDocument
    
endcase

'-------------------------------------------------------------------------

testcase  tLine_2

    PrintLog "- Check call Format.Line dialog via Button in object bar."
    
    Call hNewDocument
    
    PrintLog "- Insert a line."
    Call wZeichenobjektEinfuegen("LINIE", 40,40 ,  60,50 , false)
    Sleep 2
    Call gMouseClick(40,40)
    Sleep 2
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    PrintLog "- call Format/Line dialog via Button in object bar."
    try
        Kontext "DrawingObjectbar"
        Linie.Click
        Kontext
        active.SetPage TabLinie
        Sleep 1
        Kontext "TabLinie"
        Kontext
        active.SetPage TabLinienstile
        Sleep 1
        Kontext "TabLinienstile"
        Kontext
        active.SetPage TabLinienenden
        Sleep 1
        Kontext "TabLinienenden"
        TabLinienenden.Cancel
    catch
        Warnlog "Can't call format/line via Button in object bar."
    endcatch
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase  tLine_3

    Dim iLineStyle as Integer
    Dim iColor as Integer
    Dim iArrowStyleLeft as Integer 
    Dim iArrowStyleRight as Integer
    Dim sLineWidth as String
    
    iLineStyle          = 4
    iColor                = 5
    iArrowStyleLeft  = 4
    iArrowStyleRight = 6
    
    PrintLog "- check line attributes are applied in object bar ,include -"
    
    Call hNewDocument
    
    PrintLog "- Insert a line"
    Call wZeichenobjektEinfuegen("LINIE", 40,40 ,  60,50 , false)
    Sleep 2
    Call gMouseClick(40,40)
    Sleep 2
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    PrintLog "- set Arrow Styles"
    Kontext "DrawingObjectbar"
    Linienendenstil.TearOff
    Kontext "Linienenden"
    Sleep 1
    Auswahl.TypeKeys "<Down>" , 4
    Sleep 2
    Auswahl.TypeKeys "<Return>"
    Sleep 2
    Auswahl.TypeKeys "<Right>"
    Sleep 2
    Auswahl.TypeKeys "<Down>" , 2
    Sleep 2
    Auswahl.TypeKeys "<Return>"
    Sleep 2
    Linienenden.Close
    Sleep 2
    
    PrintLog "- set Line Style"
    Kontext "DrawingObjectbar"
    Linienstil.Select iLineStyle
    
    PrintLog "- + Line Width"
    Linienbreite.More 4
    sLineWidth = Linienbreite.GetText
    
    PrintLog "- + Line Color"
    Linienfarbe.Select iColor
    
    PrintLog "- check the lines status"
    fFormatLine("TabLinie")
    if Stil.GetSelIndex    <> iLineStyle    then Warnlog "Line style is wrong."
    if Farbe.GetSelIndex   <> iColor        then Warnlog "Line color is wrong."
    if Breite.GetText      <> sLineWidth    then warnlog "Line width is wrong."
    
    if StilLinks.GetSelIndex   <> iArrowStyleLeft  then Warnlog "Arrow style is not correctly shown in format dialog"
    if StilRechts.GetSelIndex  <> iArrowStyleRight then Warnlog "Arrow style is not correctly shown in format dialog"
    
    TabLinie.Cancel
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase  tLine_4

    Dim iArrowStyleLeft as Integer
    Dim iArrowStyleRight as Integer
    
    iArrowStyleLeft  = 2
    iArrowStyleRight = 3
    
    PrintLog "- check  'synchronize ends'"
    
    Call hNewDocument
    
    Call wZeichenobjektEinfuegen("LINIE", 40,40 ,  60,50 , false)
    Sleep 2
    Call gMouseClick(40,40)
    Sleep 2
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    fFormatLine("TabLinie")
    PrintLog "- Check Synchronize ends"
    EndenSynchronisieren.Check
    
    PrintLog "- Style (left and right)"
    StilLinks.Select  iArrowStyleLeft
    Sleep 1
    if StilRechts.GetSelIndex <> iArrowStyleLeft then Warnlog "The right arrow style is wrong."
    
    StilRechts.Select iArrowStyleRight
    Sleep 1
    if StilLinks.GetSelIndex <> iArrowStyleRight then Warnlog "The left arrow style is wrong."
    
    TabLinie.Cancel
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase  tLine_5

    Dim iLineTypeLeft as Integer
    Dim iLineTypeRight as Integer
    Dim sLineStyle as String
    Dim sRightLength as String
    Dim sSpace as String
    Dim sLeftNumber as String
    Dim sRightNumber as String
    
    sLineStyle     = "myTest"
    iLineTypeLeft  =  1
    iLineTypeRight =  2
    sLeftNumber    = "3"
    sRightNumber   = "2"
    sRightLength   = "0" + gSeperator + "30" + gMeasurementUnit
    sSpace         = "0" + gSeperator + "10" + gMeasurementUnit
    
    PrintLog "- check  'check the attributes in format/line/line styles'"
    
    Call hNewDocument
    
    Call wZeichenobjektEinfuegen("LINIE", 40,40 ,  60,50 , false)
    Sleep 2
    Call gMouseClick(40,40)
    Sleep 2
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Call fFormatLine("TabLinienstile")
    
    AnLinienbreite.UnCheck
    
    TypLinks.Select      iLineTypeLeft
    TypRechts.Select     iLineTypeRight
    AnzahlLinks.SetText  sLeftNumber
    AnzahlRechts.SetText sRightNumber
    LaengeRechts.SetText sRightLength
    Abstand.SetText      sSpace
    TabLinienstile.OK
    Sleep 1
    
    Kontext
    Kontext "Linienstil"
    if Linienstil.Exists then
        Hinzufuegen.Click
    else
        Warnlog "The dialog doesn't appear ! No test for this case"
        Kontext "TabLinienstile"
        goto NoTEST
    end if
    Sleep 1
    
    Kontext
    Kontext "LinieName"
    
    if LinieName.Exists then
        Sleep 1
        Eingabefeld.SetText sLineStyle
        Sleep 1
        LinieName.OK
        Sleep 1
    else
        Warnlog "The dialog doesn't appear !"
        Goto endsub
    end if
    
    Call fFormatLine("TabLinienstile")
    Sleep 2
    if AnLinienbreite.IsChecked =  TRUE           then Warnlog "Fit to line width should NOT be checked"
    Sleep 1
    if TypLinks.GetSelIndex     <> iLineTypeLeft  then Warnlog "The left line type is wrong !"
    Sleep 1
    if TypRechts.GetSelIndex    <> iLineTypeRight then Warnlog "The right line type is wrong !"
    Sleep 1
    if AnzahlLinks.GetText      <> sLeftNumber    then Warnlog "The left number is wrong !"
    Sleep 1
    if AnzahlRechts.GetText     <> sRightNumber   then Warnlog "The right number is wrong !"
    Sleep 1
    if LaengeLinks.IsEnabled                      then Warnlog "The left length should be unable !"
    Sleep 1
    if LaengeRechts.GetText     <> sRightLength   then Warnlog "The right length is wrong !"
    Sleep 1
    if Abstand.GetText          <> sSpace         then Warnlog "The spacing is wrong !"
    Sleep 1

NoTEST:
    'delete the testing line type , recover to the default
    Loeschen.Click
    Sleep 1
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.Yes
            Sleep 1
        else
            Warnlog "Something wrong when deleting the new line type!"
        end if
    end if
    
    Kontext
    Kontext "TabLinienstile"
    TabLinienstile.OK
    Sleep 1
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase  tLine_6

    Dim i as Integer
    Dim sArrowTitle1 as String
    Dim sArrowTitle2 as String
    Dim soeFile as String
    Dim soeDefaultFile as String
    
    sArrowTitle1  = "myTest1"
    sArrowTitle2  = "myTest2"
    
    soeFile            = ConvertPath(gOfficePath + "user\work\standard_reserver.soe")
    soeDefaultFile = ConvertPath(gOfficePath + "user\config\standard.soe")
    
    if FileExists ( soeFile ) then app.kill ( soeFile )
    
    PrintLog "- check the attributes in format/line/arrow styles"

    Call hNewDocument

    Call wZeichenobjektEinfuegen("RECHTECK", 32, 40, 40, 60)
    Call wZeichenobjektEinfuegen("ELLIPSE",  32, 65, 40, 85)
    
    Call wZeichenobjektEinfuegen("LINIE", 30,40 , 60,50 , false)
    Call gMouseClick(30,40)
    
    PrintLog "- add the first test arrow type."
    fFormatLine("TabLinienenden")
    PrintLog "- save the arrow types first."
    Speichern.Click
    Kontext
    Kontext "SpeichernDlg"
    Dateiname.SetText soeFile
    Speichern.Click
    Sleep 2
    
    Kontext
    Kontext "TabLinienenden"
    PrintLog "- delete the all arrow types."
    if Loeschen.IsEnabled then
        for i = 1 to Liste.GetItemCount
            Loeschen.Click
            Kontext "Active"
            if Active.Exists then
                if Active.GetRT = 304 then
                    Active.Yes
                else
                    Warnlog "Something wrong when deleting the arrow types!"
                end if
            end if
            Kontext
            Kontext "TabLinienenden"
        next i
    end if
    
    Hinzufuegen.Click
    Kontext
    Kontext "LinieName"
    Eingabefeld.SetText sArrowTitle1
    LinieName.OK
    
    Kontext
    Kontext "TabLinienenden"
    TabLinienenden.OK
    
    PrintLog "- add the second test arrow type."
    Call wZeichenobjektEinfuegen("LINIE", 30,70 , 60,50 , false)
    Call gMouseClick(30,70)
    fFormatLine("TabLinienenden")
    Hinzufuegen.Click
    Kontext
    Kontext "LinieName"
    Eingabefeld.SetText sArrowTitle2
    LinieName.OK
    
    Kontext
    Kontext "TabLinienenden"
    TabLinienenden.OK
    
    Call gMouseClick(30,40)
    fFormatLine("TabLinienenden")
    
    Liste.Select 1
    if TextFeld.GetText <> sArrowTitle1 then Warnlog "The first arraw title is wrong, hope "+sArrowTitle1+ " but get " +TextFeld.GetText
    Liste.Select 2
    if TextFeld.GetText <> sArrowTitle2 then Warnlog "The first arraw title is wrong, hope "+sArrowTitle2+ " but get " +TextFeld.GetText
    
    Oeffnen.Click
    Kontext
    Kontext "OeffnenDlg"
    Pfad.SetText soeFile
    Oeffnen.Click
    Sleep 1
    Kontext
    Kontext "TabLinienenden"
    
    if FileExists ( soeDefaultFile ) then app.kill ( soeDefaultFile )
    
    Speichern.Click
    Kontext
    Kontext "SpeichernDlg"
    Dateiname.SetText soeDefaultFile
    Speichern.Click
    Sleep 2
    
    Kontext
    Kontext "TabLinienenden"
    TabLinienenden.OK
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------
