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
'* short description : Test of the insert graphic function - 2
'*
'************************************************************************
'*
' #1 tInsertGraphic_13      'Test Set different attributes via dialog Arrange (send to back)
' #1 tInsertGraphic_14      'Test Set different attributes via dialog Arrange (send backward)
' #1 tInsertGraphic_15      'Test Set different attributes via dialog Arrange (bring to front)
' #1 tInsertGraphic_16      'Test Set different attributes via dialog Arrange (bring forward)
' #1 tInsertGraphic_17      'Test Set different attributes via Contextmenu Arrange (send to back)
' #1 tInsertGraphic_18      'Test Set different attributes via Contextmenu Arrange (send backward)
' #1 tInsertGraphic_19      'Test Set different attributes via Contextmenu Arrange (bring to front)
' #1 tInsertGraphic_20      'Test Set different attributes via Contextmenu Arrange (bring forward)
'*
'\***********************************************************************

testcase tInsertGraphic_13

  Dim sGraphicName1 as String
  Dim sGraphicName2 as String
  Dim sGraphicName3 as String
  Dim sWidth as String
  Dim sHeight as String

  sGraphicName1 = "TEST1"
  sGraphicName2 = "TEST2"
  sGraphicName3 = "TEST3"

  sWidth       = "7"+ gSeperator + "00" + gMeasurementUnit
  sHeight      = "6"+ gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Set different attributes via dialog Arrange (send to back)"
 '/// Test Set different attributes via dialog Arrange (send to back)

   Call hNewDocument

   Call wTypeKeys "<Return>"

  '/// Insert three Graphics at the same place , and named in series

   'the 1st graphic
   fInsertGraphics("Bughunter.jpg")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName1
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText    sWidth
   wait 500
   Height.SetText   sHeight
   wait 500
   TabType.OK

   Call wTypeKeys "<ESCape>"

   'the 2nd graphic
   fInsertGraphics("flowers.gif")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName2
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText    sWidth
   wait 500
   Height.SetText   sHeight
   wait 500
   TabType.OK

   Call wTypeKeys "<ESCape>"

   'the 3rd graphic
   fInsertGraphics("game.bmp")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName3
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText    sWidth
   wait 500
   Height.SetText   sHeight
   wait 500
   TabType.OK

   '/// Format / Arrange / Send to back
   FormatArrangeSendToBack
   Sleep 1

   Call wTypeKeys "<ESCape>"
   Sleep 1

   '/// Check if the attributes Arrange (send to back) works well
   Call wNavigatorAuswahl(4,1)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName1  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName1 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call wNavigatorAuswahl(4,2)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName2  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName2 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call wNavigatorAuswahl(4,3)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName3  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName3 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_14

  Dim sGraphicName1 as String
  Dim sGraphicName2 as String
  Dim sGraphicName3 as String
  Dim sWidth as String
  Dim sHeight as String

  sGraphicName1 = "TEST1"
  sGraphicName2 = "TEST2"
  sGraphicName3 = "TEST3"

  sWidth       = "7"+ gSeperator + "00" + gMeasurementUnit
  sHeight      = "6"+ gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Set different attributes via dialog Arrange (send backward)"
 '/// Test Set different attributes via dialog Arrange (send backward)

   Call hNewDocument

   Call wTypeKeys "<Return>"

  '/// Insert three Graphics at the same place , and named in series

   'the 1st graphic
   fInsertGraphics("Bughunter.jpg")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName1
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText    sWidth
   wait 500
   Height.SetText   sHeight
   wait 500
   TabType.OK

   Call wTypeKeys "<ESCape>"

   'the 2nd graphic
   fInsertGraphics("flowers.gif")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName2
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText    sWidth
   wait 500
   Height.SetText   sHeight
   wait 500
   TabType.OK

   Call wTypeKeys "<ESCape>"

   'the 3rd graphic
   fInsertGraphics("game.bmp")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName3
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText    sWidth
   wait 500
   Height.SetText   sHeight
   wait 500
   TabType.OK

   '/// Format / Arrange / Send backward
   FormatArrangeSetBackward
   Sleep 1

   Call wTypeKeys "<ESCape>"
   Sleep 1

   '/// Check if the attributes Arrange (send to back) works well
   Call wNavigatorAuswahl(4,1)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName1  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName1 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call wNavigatorAuswahl(4,2)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName2  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName2 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call wNavigatorAuswahl(4,3)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName3  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName3 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_15

  Dim sGraphicName1 as String
  Dim sGraphicName2 as String
  Dim sGraphicName3 as String
  Dim sWidth as String
  Dim sHeight as String

  sGraphicName1 = "TEST1"
  sGraphicName2 = "TEST2"
  sGraphicName3 = "TEST3"

  sWidth       = "7"+ gSeperator + "00" + gMeasurementUnit
  sHeight      = "6"+ gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Set different attributes via dialog  Arrange (bring to front)"
 '/// Test Set different attributes via dialog Arrange (bring to front)

  Call hNewDocument

  Call wTypeKeys "<Return>"

  '/// Insert three Graphics at the same place , and named in series

   'the 1st graphic
   fInsertGraphics("Bughunter.jpg")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName1
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText    sWidth
   wait 500
   Height.SetText   sHeight
   wait 500
   TabType.OK

   Call wTypeKeys "<ESCape>"

   'the 2nd graphic
   fInsertGraphics("flowers.gif")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName2
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText    sWidth
   wait 500
   Height.SetText   sHeight
   wait 500
   TabType.OK

   Call wTypeKeys "<ESCape>"

   'the 3rd graphic
   fInsertGraphics("game.bmp")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName3
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText    sWidth
   wait 500
   Height.SetText   sHeight
   wait 500
   TabType.OK

   '/// Format / Arrange / Send to back
   FormatArrangeSendToBack
   Sleep 2

   '/// Format / Arrange / bring to front
   FormatArrangeBringToFront
   Sleep 1

   Call wTypeKeys "<ESCape>"
   Sleep 1

   '/// Check if the attributes Arrange (send to back) works well
   Call wNavigatorAuswahl(4,1)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName1  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName1 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call wNavigatorAuswahl(4,2)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName2  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName2 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call wNavigatorAuswahl(4,3)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName3  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName3 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_16

  Dim sGraphicName1 as String
  Dim sGraphicName2 as String
  Dim sGraphicName3 as String
  Dim sWidth as String
  Dim sHeight as String

  sGraphicName1 = "TEST1"
  sGraphicName2 = "TEST2"
  sGraphicName3 = "TEST3"

  sWidth       = "7"+ gSeperator + "00" + gMeasurementUnit
  sHeight      = "6"+ gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Set different attributes via dialog  Arrange (bring forward)"
 '/// Test Set different attributes via dialog Arrange (bring forward)

   Call hNewDocument

   Call wTypeKeys "<Return>"

  '/// Insert three Graphics at the same place , and named in series

   'the 1st graphic
   fInsertGraphics("Bughunter.jpg")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName1
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText        sWidth
   wait 500
   Height.SetText       sHeight
   wait 500
   TabType.OK

   Call wTypeKeys "<ESCape>"

   'the 2nd graphic
   fInsertGraphics("flowers.gif")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName2
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText        sWidth
   wait 500
   Height.SetText       sHeight
   wait 500
   TabType.OK

   Call wTypeKeys "<ESCape>"

   'the 3rd graphic
   fInsertGraphics("game.bmp")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName3
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText        sWidth
   wait 500
   Height.SetText       sHeight
   wait 500
   TabType.OK

   '/// Format / Arrange / Send to back
   FormatArrangeSendToBack
   Sleep 1

   '/// Format / Arrange / bring forward
   FormatArrangeBringForward
   Sleep 1

   Call wTypeKeys "<ESCape>"
   Sleep 1

   '/// Check if the attributes Arrange (send to back) works well
   Call wNavigatorAuswahl(4,1)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName1  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName1 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call wNavigatorAuswahl(4,2)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName2  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName2 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call wNavigatorAuswahl(4,3)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName3  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName3 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_17

  Dim sGraphicName1 as String
  Dim sGraphicName2 as String
  Dim sGraphicName3 as String
  Dim sWidth as String
  Dim sHeight as String

  sGraphicName1 = "TEST1"
  sGraphicName2 = "TEST2"
  sGraphicName3 = "TEST3"

  sWidth       = "7"+ gSeperator + "00" + gMeasurementUnit
  sHeight      = "6"+ gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Set different attributes via Contextmenu Arrange (send to back)"
 '/// Test Set different attributes via Contextmenu Arrange (send to back)

   Call hNewDocument

   Call wTypeKeys "<Return>"

  '/// Insert three Graphics at the same place , and named in series

   'the 1st graphic
   fInsertGraphics("Bughunter.jpg")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName1
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText        sWidth
   wait 500
   Height.SetText       sHeight
   wait 500
   TabType.OK

   Call wTypeKeys "<ESCape>"

   'the 2nd graphic
   fInsertGraphics("flowers.gif")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName2
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText        sWidth
   wait 500
   Height.SetText       sHeight
   wait 500
   TabType.OK

   Call wTypeKeys "<ESCape>"

   'the 3rd graphic
   fInsertGraphics("game.bmp")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName3
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText        sWidth
   wait 500
   Height.SetText       sHeight
   wait 500
   TabType.OK

   '/// ContextMenu  Arrange / Send to back
   Call wOpenContextMenu
    hMenuSelectNr(1)
    Sleep 2
    hMenuSelectNr(4)
    Sleep 1

   Call wTypeKeys "<ESCape>"
   Sleep 1

   '/// Check if the attributes Arrange (send to back) works well
   Call wNavigatorAuswahl(4,1)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName1  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName1 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call wNavigatorAuswahl(4,2)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName2  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName2 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call wNavigatorAuswahl(4,3)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName3  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName3 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_18

    Dim sGraphicName1 as String
    Dim sGraphicName2 as String
    Dim sGraphicName3 as String
    Dim sWidth as String
    Dim sHeight as String

    sGraphicName1 = "TEST1"
    sGraphicName2 = "TEST2"
    sGraphicName3 = "TEST3"

    sWidth       = "7"+ gSeperator + "00" + gMeasurementUnit
    sHeight      = "6"+ gSeperator + "00" + gMeasurementUnit

    PrintLog "- Test Set different attributes via Contextmenu Arrange (send backward)"
    '/// Test Set different attributes via Contextmenu Arrange (send backward)

    Call hNewDocument

    Call wTypeKeys "<Return>"

    '/// Insert three Graphics at the same place , and named in series

    'the 1st graphic
    fInsertGraphics("Bughunter.jpg")

    fFormatGraphic("TabZusaetze")
    Sleep 1
    ObjektName.SetText sGraphicName1
    Kontext
    Active.Setpage TabType
    Kontext "TabType"
    KeepRatio.UnCheck
    Sleep 1
    Width.SetText        sWidth
    wait 500
    Height.SetText       sHeight
    wait 500
    TabType.OK

    Call wTypeKeys "<ESCape>"

    'the 2nd graphic
    fInsertGraphics("flowers.gif")

    fFormatGraphic("TabZusaetze")
    Sleep 1
    ObjektName.SetText sGraphicName2
    TabZusaetze.OK

    fFormatGraphic("TabType")
    KeepRatio.UnCheck
    Sleep 1
    Width.SetText        sWidth
    wait 500
    Height.SetText       sHeight
    wait 500
    TabType.OK

    Call wTypeKeys "<ESCape>"

    'the 3rd graphic
    fInsertGraphics("game.bmp")

    fFormatGraphic("TabZusaetze")
    Sleep 1
    ObjektName.SetText sGraphicName3
    TabZusaetze.OK

    fFormatGraphic("TabType")
    KeepRatio.UnCheck
    Sleep 1
    Width.SetText        sWidth
    wait 500
    Height.SetText       sHeight
    wait 500
    TabType.OK

    '/// ContextMenu  Arrange / Send backward
    Call wOpenContextMenu
    hMenuSelectNr(1)
    Sleep 2
    hMenuSelectNr(3)
    Sleep 1

    Call wTypeKeys "<ESCape>"
    Sleep 1

    '/// Check if the attributes Arrange (send to back) works well
    Call wNavigatorAuswahl(4,1)
    Sleep 1
    fFormatGraphic("TabZusaetze")
    Sleep 1
    if ObjektName.GetText <> sGraphicName1  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName1 + " but get " +ObjektName.GetText
    TabZusaetze.Cancel

    Call wNavigatorAuswahl(4,2)
    Sleep 1
    fFormatGraphic("TabZusaetze")
    Sleep 1
    if ObjektName.GetText <> sGraphicName2  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName2 + " but get " +ObjektName.GetText
    TabZusaetze.Cancel

    Call wNavigatorAuswahl(4,3)
    Sleep 1
    fFormatGraphic("TabZusaetze")
    Sleep 1
    if ObjektName.GetText <> sGraphicName3  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName3 + " but get " +ObjektName.GetText
    TabZusaetze.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_19

    Dim sGraphicName1 as String
    Dim sGraphicName2 as String
    Dim sGraphicName3 as String
    Dim sWidth as String
    Dim sHeight as String

    sGraphicName1 = "TEST1"
    sGraphicName2 = "TEST2"
    sGraphicName3 = "TEST3"

    sWidth       = "7"+ gSeperator + "00" + gMeasurementUnit
    sHeight      = "6"+ gSeperator + "00" + gMeasurementUnit

    PrintLog "- Test Set different attributes via Contextmenu Arrange (bring to front)"
    '/// Test Set different attributes via Contextmenu Arrange (bring to front)

    Call hNewDocument

    Call wTypeKeys "<Return>"

    '/// Insert three Graphics at the same place , and named in series

    'the 1st graphic
    fInsertGraphics("Bughunter.jpg")

    fFormatGraphic("TabZusaetze")
    Sleep 1
    ObjektName.SetText sGraphicName1

    Kontext
    Active.Setpage TabType
    Kontext "TabType"

    KeepRatio.UnCheck
    Sleep 1
    Width.SetText        sWidth
    wait 500
    Height.SetText       sHeight
    wait 500
    TabType.OK

    Call wTypeKeys "<ESCape>"

    'the 2nd graphic
    fInsertGraphics("flowers.gif")

    fFormatGraphic("TabZusaetze")
    Sleep 1
    ObjektName.SetText sGraphicName2
    TabZusaetze.OK

    fFormatGraphic("TabType")
    KeepRatio.UnCheck
    Sleep 1
    Width.SetText        sWidth
    wait 500
    Height.SetText       sHeight
    wait 500
    TabType.OK

    Call wTypeKeys "<ESCape>"

    'the 3rd graphic
    fInsertGraphics("game.bmp")

    fFormatGraphic("TabZusaetze")
    Sleep 1
    ObjektName.SetText sGraphicName3
    TabZusaetze.OK

    fFormatGraphic("TabType")
    KeepRatio.UnCheck
    Sleep 1
    Width.SetText        sWidth
    wait 500
    Height.SetText       sHeight
    wait 500
    TabType.OK

    '/// Format / Arrange / Send to back
    FormatArrangeSendToBack
    Sleep 2

    '/// ContextMenu Arrange / bring to front
    Call wOpenContextMenu
    hMenuSelectNr(1)
    Sleep 2
    hMenuSelectNr(1)
    Sleep 1

    Call wTypeKeys "<ESCape>"
    Sleep 1

    '/// Check if the attributes Arrange (send to back) works well
    Call wNavigatorAuswahl(4,1)
    Sleep 1
    fFormatGraphic("TabZusaetze")
    Sleep 1
    if ObjektName.GetText <> sGraphicName1  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName1 + " but get " +ObjektName.GetText
    TabZusaetze.Cancel

    Call wNavigatorAuswahl(4,2)
    Sleep 1
    fFormatGraphic("TabZusaetze")
    Sleep 1
    if ObjektName.GetText <> sGraphicName2  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName2 + " but get " +ObjektName.GetText
    TabZusaetze.Cancel

    Call wNavigatorAuswahl(4,3)
    Sleep 1
    fFormatGraphic("TabZusaetze")
    Sleep 1
    if ObjektName.GetText <> sGraphicName3  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName3 + " but get " +ObjektName.GetText
    TabZusaetze.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_20

  Dim sGraphicName1 as String
  Dim sGraphicName2 as String
  Dim sGraphicName3 as String
  Dim sWidth as String
  Dim sHeight as String

  sGraphicName1 = "TEST1"
  sGraphicName2 = "TEST2"
  sGraphicName3 = "TEST3"

  sWidth       = "7"+ gSeperator + "00" + gMeasurementUnit
  sHeight      = "6"+ gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Set different attributes via Contextmenu Arrange (bring forward)"
 '/// Test Set different attributes via Contextmenu Arrange (bring forward)

   Call hNewDocument

   Call wTypeKeys "<Return>"

  '/// Insert three Graphics at the same place , and named in series

   'the 1st graphic
   fInsertGraphics("Bughunter.jpg")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName1
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText        sWidth
   wait 500
   Height.SetText       sHeight
   wait 500
   TabType.OK

   Call wTypeKeys "<ESCape>"

   'the 2nd graphic
   fInsertGraphics("flowers.gif")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName2
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText        sWidth
   wait 500
   Height.SetText       sHeight
   wait 500
   TabType.OK

   Call wTypeKeys "<ESCape>"

   'the 3rd graphic
   fInsertGraphics("game.bmp")

   fFormatGraphic("TabZusaetze")
   Sleep 1
   ObjektName.SetText sGraphicName3
   TabZusaetze.OK

   fFormatGraphic("TabType")
   KeepRatio.UnCheck
   Sleep 1
   Width.SetText        sWidth
   wait 500
   Height.SetText       sHeight
   wait 500
   TabType.OK

   '/// Format / Arrange / Send to back
   FormatArrangeSendToBack
   Sleep 1

   '/// ContextMenu Arrange / bring forward
   Call wOpenContextMenu
    hMenuSelectNr(1)
    Sleep 2
    hMenuSelectNr(2)
    Sleep 1

   Call wTypeKeys "<ESCape>"
   Sleep 1

   '/// Check if the attributes Arrange (send to back) works well
   Call wNavigatorAuswahl(4,1)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName1  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName1 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call wNavigatorAuswahl(4,2)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName2  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName2 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call wNavigatorAuswahl(4,3)
   Sleep 1
   fFormatGraphic("TabZusaetze")
   Sleep 1
   if ObjektName.GetText <> sGraphicName3  then Warnlog "The graphic's name is not correct ,should be "+sGraphicName3 + " but get " +ObjektName.GetText
   TabZusaetze.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------
