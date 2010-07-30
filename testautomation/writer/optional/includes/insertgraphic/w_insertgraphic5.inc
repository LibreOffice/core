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
'* short description : Test of the insert graphic function - 4
'*
'************************************************************************
'*
' #1 tInsertGraphic_41         'Test checkbox 'Keep image size'
' #1 tInsertGraphic_42         'Test checkbox 'Keep scale'
' #1 tInsertGraphic_43         'Test spacing in tabpage 'wrap'
' #1 tInsertGraphic_44         'Set the possible Options for the different Wrap types
' #1 tInsertGraphic_45         'Test Scale in Corp tabpage
' #1 tInsertGraphic_46         'Test Image size in Corp tabpage
' #1 tInsertGraphic_47         'Test Restore via "Original Size" button
' #1 tInsertGraphic_48         'Test link area via a embeded file
' #1 tInsertGraphic_49         'Test link area via a linked file
'*
'\***********************************************************************

testcase tInsertGraphic_41

  Dim sLeft as String
  Dim sRight as String
  Dim sUp as String
  Dim sBottom as String
  Dim sScaleWidth as String
  Dim sScaleHeight as String
  Dim sImageWidth as String
  Dim sImageHeight as String
  Dim dDummy1 as double
  Dim dDummy2 as double

  sLeft    = "1"+ gSeperator + "00" + gMeasurementUnit
  sRight   = "1"+ gSeperator + "00" + gMeasurementUnit
  sUp      = "1"+ gSeperator + "00" + gMeasurementUnit
  sBottom  = "1"+ gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test checkbox 'Keep image size'"
 '/// Test checkbox 'Keep image size'

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("game.bmp")

  '/// Format / Graphic / Crop , check 'Keep image size' ,
  '/// - increase the value , check if "keep image size" is efficient
   fFormatGraphic("TabZuschneiden")
    wait 500
    GroesseBeibehalten.Check
    wait 500

    sScaleWidth  = MassstabBreite.GetText
    sScaleHeight = MassstabHoehe.GetText
    sImageWidth  = GroesseBreite.GetText
    sImageHeight = GroesseHoehe.GetText

   PrintLog "- increase Left "
    Links.SetText sLeft
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <= StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <> StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <> StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <> StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

    sScaleWidth  = MassstabBreite.GetText

   PrintLog "- increase Right"
    Rechts.SetText sRight
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <= StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <> StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <> StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <> StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

    sScaleWidth  = MassstabBreite.GetText

   PrintLog "- increase Top "
    Oben.SetText sUp
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <> StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <= StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <> StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <> StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

    sScaleHeight  = MassstabHoehe.GetText

   PrintLog "- increase Bottom "
    Unten.SetText sBottom
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <> StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <= StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <> StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <> StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

   TabZuschneiden.Cancel

   '/// - decrease value , check if "keep image size" is efficient
   fFormatGraphic("TabZuschneiden")
    wait 500
    GroesseBeibehalten.Check
    wait 500

    sScaleWidth  = MassstabBreite.GetText
    sScaleHeight = MassstabHoehe.GetText
    sImageWidth  = GroesseBreite.GetText
    sImageHeight = GroesseHoehe.GetText

   PrintLog "- decrease Left "
    Links.SetText "-" + sLeft
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) >= StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <> StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <> StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <> StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

    sScaleWidth  = MassstabBreite.GetText

   PrintLog "- decrease Right"
    Rechts.SetText "-" + sRight
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) >= StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <> StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <> StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <> StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

    sScaleWidth  = MassstabBreite.GetText

   PrintLog "- decrease Top "
    Oben.SetText "-" + sUp
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <> StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  >= StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <> StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <> StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

    sScaleHeight  = MassstabHoehe.GetText

   PrintLog "- decrease Bottom "
    Unten.SetText "-" + sBottom
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <> StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  >= StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <> StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <> StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

   TabZuschneiden.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_42

  Dim sLeft as String
  Dim sRight as String
  Dim sUp as String
  Dim sBottom as String
  Dim sScaleWidth as String
  Dim sScaleHeight as String
  Dim sImageWidth as String
  Dim sImageHeight as String
  Dim dDummy1 as double
  Dim dDummy2 as double

  sLeft    = "1"+ gSeperator + "00" + gMeasurementUnit
  sRight   = "1"+ gSeperator + "00" + gMeasurementUnit
  sUp      = "1"+ gSeperator + "00" + gMeasurementUnit
  sBottom  = "1"+ gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test checkbox 'Keep scale'"
 '/// Test checkbox 'Keep scale'

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("game.bmp")

  '/// Format / Graphic / Crop , check Keep scale
  '/// - increase the value
   fFormatGraphic("TabZuschneiden")
    wait 500
    MassstabBeibehalten.Check
    wait 500

    sScaleWidth  = MassstabBreite.GetText
    sScaleHeight = MassstabHoehe.GetText
    sImageWidth  = GroesseBreite.GetText
    sImageHeight = GroesseHoehe.GetText

   PrintLog "- increase Left "
    Links.SetText sLeft
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <> StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <> StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  >= StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <> StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

    sImageWidth  = GroesseBreite.GetText

   PrintLog "- increase Right"
    Rechts.SetText sRight
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <> StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <> StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  >= StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <> StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

    sImageWidth  = GroesseBreite.GetText

   PrintLog "- increase Top "
    Oben.SetText sUp
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <> StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <> StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <> StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   >= StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

    sImageHeight = GroesseHoehe.GetText

   PrintLog "- increase Bottom "
    Unten.SetText sBottom
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <> StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <> StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <> StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   >= StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

   TabZuschneiden.Cancel

   '/// - decrease value
   fFormatGraphic("TabZuschneiden")
    wait 500
    MassstabBeibehalten.Check
    wait 500

    sScaleWidth  = MassstabBreite.GetText
    sScaleHeight = MassstabHoehe.GetText
    sImageWidth  = GroesseBreite.GetText
    sImageHeight = GroesseHoehe.GetText

   PrintLog "- decrease Left "
    Links.SetText "-" + sLeft
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <> StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <> StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <= StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <> StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

    sImageWidth  = GroesseBreite.GetText

   PrintLog "- decrease Right"
    Rechts.SetText "-" + sRight
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <> StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <> StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <= StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <> StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

    sImageWidth  = GroesseBreite.GetText

   PrintLog "- decrease Top "
    Oben.SetText "-" + sUp
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <> StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <> StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <> StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <= StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

    sImageHeight = GroesseHoehe.GetText

   PrintLog "- decrease Bottom "
    Unten.SetText "-" + sBottom
    Sleep 2

    if StrToDouble(MassstabBreite.GetText) <> StrToDouble(sScaleWidth)  then warnlog "Scale Width  isn't right !"
    if StrToDouble(MassstabHoehe.GetText)  <> StrToDouble(sScaleHeight) then warnlog "Scale Height isn't right !"
    if StrToDouble(GroesseBreite.GetText)  <> StrToDouble(sImageWidth)  then warnlog "Image Width  isn't right !"
    if StrToDouble(GroesseHoehe.GetText)   <= StrToDouble(sImageHeight) then warnlog "Image Height isn't right !"

   TabZuschneiden.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_43

  Dim sLeft as String
  Dim sRight as String
  Dim sUp as String
  Dim sBottom as String

  sLeft    = "1"+ gSeperator + "00" + gMeasurementUnit
  sRight   = "2"+ gSeperator + "00" + gMeasurementUnit
  sUp      = "3"+ gSeperator + "00" + gMeasurementUnit
  sBottom  = "4"+ gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test spacing in tabpage 'wrap'"
 '/// Test spacing in tabpage 'wrap'

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("game.bmp")

  '/// Format / Graphic / Wrap , set value in spacing area
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Kein.Check
    wait 500
    AbstaendeLinks.SetText  sLeft
    wait 500
    AbstaendeRechts.SetText sRight
    wait 500
    AbstaendeOben.SetText   sUp
    wait 500
    AbstaendeUnten.SetText  sBottom
    wait 500
   TabUmlauf.OK

  '/// Check if the set value is correct
   fFormatGraphic("TabUmlauf")
    if AbstaendeLinks.GetText  <> sLeft   then  warnlog "Spacing - Lefe isn't correct!"
    if AbstaendeRechts.GetText <> sRight  then  warnlog "Spacing - Right isn't correct!"
    if AbstaendeOben.GetText   <> sUp     then  warnlog "Spacing - Top isn't correct!"
    if AbstaendeUnten.GetText  <> sBottom then  warnlog "Spacing - Bottom isn't correct!"
   TabUmlauf.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_44

  PrintLog "- Set the possible Options for the different Wrap types"
 '/// Set the possible Options for the different Wrap types

  Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

   PrintLog "  1. No Wrap:"
  '/// Format / Graphic / tabpage Wrap - No page , check possible options
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Kein.Check
    Sleep 1
    if ErsterAbsatz.IsEnabled   = TRUE then Warnlog "Checkbox - First paragraph should be unenabled!"
    if ImHintergrund.IsEnabled  = TRUE then Warnlog "Checkbox - In background should be unenabled!"
    if Kontur.IsEnabled         = TRUE then Warnlog "Checkbox - Contour should be unenabled!"
    if NurAussen.IsEnabled      = TRUE then Warnlog "Checkbox - Outside only should be unenabled!"
   TabUmlauf.Cancel

   PrintLog "  2. Before:"
  '/// Format / Graphic / tabpage Wrap - Before , check possible options
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Links.Check
    Sleep 1
    if ErsterAbsatz.IsEnabled   <> TRUE then Warnlog "Checkbox - First paragraph should be enabled!"
    if ImHintergrund.IsEnabled  =  TRUE then Warnlog "Checkbox - In background should be unenabled!"
    if Kontur.IsEnabled         <> TRUE then Warnlog "Checkbox - Contour should be enabled!"
    if NurAussen.IsEnabled      =  TRUE then Warnlog "Checkbox - Outside only should be unenabled!"

    Kontur.Check
    wait 500
    if NurAussen.IsEnabled      <> TRUE then Warnlog "Checkbox - Outside only should be enabled after checking contour!"
   TabUmlauf.Cancel

   PrintLog "  3. After:"
  '/// Format / Graphic / tabpage Wrap - After , check possible options
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Rechts.Check
    Sleep 1
    if ErsterAbsatz.IsEnabled   <> TRUE then Warnlog "Checkbox - First paragraph should be enabled!"
    if ImHintergrund.IsEnabled  =  TRUE then Warnlog "Checkbox - In background should be unenabled!"
    if Kontur.IsEnabled         <> TRUE then Warnlog "Checkbox - Contour should be enabled!"
    if NurAussen.IsEnabled      =  TRUE then Warnlog "Checkbox - Outside only should be unenabled!"

    Kontur.Check
    wait 500
    if NurAussen.IsEnabled      <> TRUE then Warnlog "Checkbox - Outside only should be enabled after checking contour!"
   TabUmlauf.Cancel

   PrintLog "  4. Parallel:"
  '/// Format / Graphic / tabpage Wrap - Parallel , check possible options
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Parallel.Check
    Sleep 1
    if ErsterAbsatz.IsEnabled   <> TRUE then Warnlog "Checkbox - First paragraph should be enabled!"
    if ImHintergrund.IsEnabled  =  TRUE then Warnlog "Checkbox - In background should be unenabled!"
    if Kontur.IsEnabled         <> TRUE then Warnlog "Checkbox - Contour should be enabled!"
    if NurAussen.IsEnabled      =  TRUE then Warnlog "Checkbox - Outside only should be unenabled!"

    Kontur.Check
    wait 500
    if NurAussen.IsEnabled      <> TRUE then Warnlog "Checkbox - Outside only should be enabled after checking contour!"
   TabUmlauf.Cancel

   PrintLog "  5. Through:"
  '/// Format / Graphic / tabpage Wrap - Through , check possible options
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Durchlauf.Check
    Sleep 1
    if ErsterAbsatz.IsEnabled   <> TRUE then Warnlog "Checkbox - First paragraph should be enabled!"
    if ImHintergrund.IsEnabled  <> TRUE then Warnlog "Checkbox - In background should be enabled!"
    if Kontur.IsEnabled         =  TRUE then Warnlog "Checkbox - Contour should be unenabled!"
    if NurAussen.IsEnabled      =  TRUE then Warnlog "Checkbox - Outside only should be unenabled!"
   TabUmlauf.Cancel

   PrintLog "  6. Optimal:"
  '/// Format / Graphic / tabpage Wrap - Optimal , check possible options
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Dynamisch.Check
    Sleep 1
    if ErsterAbsatz.IsEnabled   <> TRUE then Warnlog "Checkbox - First paragraph should be enabled!"
    if ImHintergrund.IsEnabled  =  TRUE then Warnlog "Checkbox - In background should be unenabled!"
    if Kontur.IsEnabled         <> TRUE then Warnlog "Checkbox - Contour should be enabled!"
    if NurAussen.IsEnabled      =  TRUE then Warnlog "Checkbox - Outside only should be unenabled!"

    Kontur.Check
    wait 500
    if NurAussen.IsEnabled      <> TRUE then Warnlog "Checkbox - Outside only should be enabled after checking contour!"
   TabUmlauf.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_45

  Dim sScaleWidth as String
  Dim sScaleHeight as String
  Dim sImageWidth as String
  Dim sImageHeight as String

  sScaleWidth  = "100%"
  sScaleHeight = "100%"

  PrintLog "- Test Scale in Corp tabpage"
 '/// Test Scale in Corp tabpage

  Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("game.bmp")
   Sleep 1

  '/// Format/Graphic tabpage corp , double Scale (width and height)
   fFormatGraphic("TabZuschneiden")
    wait 500
    MassstabBreite.SetText sScaleWidth
    MassstabHoehe.SetText  sScaleHeight
   TabZuschneiden.OK

   fFormatGraphic("TabZuschneiden")
    wait 500
    sImageWidth  = GroesseBreite.GetText
    sImageHeight = GroesseHoehe.GetText

    sScaleWidth  = "200%"
    sScaleHeight = "200%"

    MassstabBreite.SetText sScaleWidth
    wait 500
    MassstabHoehe.SetText  sScaleHeight
    wait 500

    sImageWidth  = Str(StrToDouble(sImageWidth) *2) + gMeasurementUnit
    sImageHeight = Str(StrToDouble(sImageHeight)*2) + gMeasurementUnit

   '/// Check if the image size (width and height) are changed corresponding
    if fCalculateTolerance(GroesseBreite.GetText , sImageWidth) > 0.03 then
       Warnlog "Image Width is wrong, should be " + sImageWidth + " but get " + GroesseBreite.GetText
    end if

    if fCalculateTolerance(GroesseHoehe.GetText , sImageHeight) > 0.03 then
       Warnlog "Image Height is wrong, should be " + sImageHeight + " but get " + GroesseHoehe.GetText
    end if
   TabZuschneiden.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_46

  Dim sScaleWidth as String
  Dim sScaleHeight as String
  Dim sImageWidth as String
  Dim sImageHeight as String

  sImageWidth  = "4"+ gSeperator + "00" + gMeasurementUnit
  sImageHeight = "2"+ gSeperator + "80" + gMeasurementUnit

  PrintLog "- Test Image size in Corp tabpage"
 '/// Test Image size in Corp tabpage

  Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("game.bmp")
   Sleep 1

  '/// Format/Graphic tabpage corp , double Imagin size (width and height)
   fFormatGraphic("TabZuschneiden")
    wait 500
    GroesseBreite.SetText  sImageWidth
    GroesseHoehe.SetText   sImageHeight
   TabZuschneiden.OK

   fFormatGraphic("TabZuschneiden")
    wait 500
    sScaleWidth  = MassstabBreite.GetText
    sScaleHeight = MassstabHoehe.GetText

    sImageWidth  = "8"+ gSeperator + "00" + gMeasurementUnit
    sImageHeight = "5"+ gSeperator + "60" + gMeasurementUnit

    GroesseBreite.SetText  sImageWidth
    wait 500
    GroesseHoehe.SetText   sImageHeight
    wait 500

   '/// Check if the Scale(width and height) are changed corresponding
    if Abs(StrToDouble(sScaleWidth) *2 - StrToDouble(MassstabBreite.GetText) ) > 2 then
       Warnlog "Scale Width is wrong, should be " + Str(StrToDouble(sScaleWidth) *2) + "%" + " but get " + MassstabBreite.GetText
    end if

    if Abs(StrToDouble(sScaleHeight) *2 - StrToDouble(MassstabHoehe.GetText) ) > 2 then
       Warnlog "Scale Height is wrong, should be " + Str(StrToDouble(sScaleHeight)*2) + "%" + " but get " + MassstabHoehe.GetText
    end if
   TabZuschneiden.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_47

  Dim sScaleWidth as String
  Dim sScaleHeight as String
  Dim sImageWidth as String
  Dim sImageHeight as String
  Dim sNewImageWidth as String
  Dim sNewImageHeight as String

  sNewImageWidth  = "8"+ gSeperator + "00" + gMeasurementUnit
  sNewImageHeight = "5"+ gSeperator + "60" + gMeasurementUnit

  PrintLog "- Test Restore via 'Original Size' button"
 '/// Test Restore via "Original Size" button

  Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("game.bmp")
   Sleep 1

  '/// Format/Graphic tabpage corp , double Image size (width and height)
   fFormatGraphic("TabZuschneiden")
    wait 500
    sScaleWidth  = MassstabBreite.GetText
    sScaleHeight = MassstabHoehe.GetText
    sImageWidth  = GroesseBreite.GetText
    sImageHeight = GroesseHoehe.GetText

    GroesseBreite.SetText  sNewImageWidth
    wait 500
    GroesseHoehe.SetText   sNewImageHeight
    wait 500

   '/// + Click 'Original size' button
    Originalgroesse.Click
    Sleep 1

   '/// Check if the Scale and Image size are recovered to the original size
    if MassstabBreite.GetText <> sScaleWidth  then warnlog "Scale Width  isn't right !"
    if MassstabHoehe.GetText  <> sScaleHeight then warnlog "Scale Height isn't right !"
    if GroesseBreite.GetText  <> sImageWidth  then warnlog "Image Width  isn't right !"
    if GroesseHoehe.GetText   <> sImageHeight then warnlog "Image Height isn't right !"
   TabZuschneiden.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_48

  Dim sFileName as String

  sFileName = "game.bmp"

  PrintLog "- Test link area via a embedded file"
 '/// Test link area via a embedded file

  Call hNewDocument

  '/// Insert a graphics (embeded)
   InsertGraphicsFromFile
   Kontext "GrafikEinfuegenDlg"
   Dateiname.Settext (ConvertPath (gTesttoolpath + "writer\optional\input\insertgraphic\" + sFileName) )
   Sleep 1
   Link.UnCheck
   Sleep 1
   Oeffnen.Click
   Sleep 2

  '/// Format/Graphic tabpage Graphics , check if we can get [None] in file name in Link area
   fFormatGraphic("TabGrafik")
    wait 500
    if left(Dateiname.GetText , 1) <> "["  then warnlog "Should be none in file name, but get " + Dateiname.GetText
    if Durchsuchen.IsEnabled       =  TRUE then Warnlog "Button should be unEnabled!"
   TabGrafik.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_49

  Dim sFileName   as String
  Dim sHopeResult as String

  sFileName   = "game.bmp"
  sHopeResult = fConvertBacklashToSlash(gTesttoolpath + "writer\optional\input\insertgraphic\" + sFileName)

  if gPlatGroup <> "unx" then
     sHopeResult = "file:///" + sHopeResult
  else
     sHopeResult = "file://"  + sHopeResult
  end if

  PrintLog "- Test link area via a linked file"
 '/// Test link area via a linked file

  Call hNewDocument

  '/// Insert a graphics (linked)
   fInsertGraphics(sFileName)

  '/// Format/Graphic tabpage Graphics , check if we can get
  '/// + the correct name in 'file name' in Link area
   fFormatGraphic("TabGrafik")
    wait 500
    if Dateiname.GetText <> sHopeResult  then warnlog "linked file name is wrong, should be " + sHopeResult + " but get " + Dateiname.GetText
    try
       Durchsuchen.Click
       Kontext
       Kontext "GrafikEinfuegenDlg"
       if Dateiname.GetSelText <> sFileName then warnlog "file name is wrong, should be " + sFileName + " but get " + Dateiname.GetText
       if Link.IsChecked       <> TRUE      then warnlog "The linked checkbox should be checked!"
       GrafikEinfuegenDlg.Cancel
    catch
       warnlog "Button should be enabled!"
    endcatch

   Kontext "TabGrafik"
   TabGrafik.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

function fCalculateTolerance( sNumber1 , sNumber2 )

'/// change 2 parameters to number , then Compare them and return the result.

   Dim dNumber1 as Single
   dim dNumber2 as Single
   Dim iCommaLocation1 as integer
   Dim iCommaLocation2 as Integer

  'the code below 4 lines is set "2,00cm" to "2.00"cm if needed
   iCommaLocation1 = InStr(sNumber1 , ",")
   if iCommaLocation1 > 0 then
      sNumber1 = Left(sNumber1 , iCommaLocation1-1) + "." + Right(sNumber1,Len(sNumber1) - iCommaLocation1)
   end if

   iCommaLocation2 = InStr(sNumber2 , ",")
   if iCommaLocation2 > 0 then
      sNumber2 = Left(sNumber2 , iCommaLocation2-1) + "." + Right(sNumber2,Len(sNumber2) - iCommaLocation2)
   end if

   dNumber1 = Val( Left(sNumber1, Len(sNumber1)-Len(gMeasurementUnit) ))
   dNumber2 = Val( Left(sNumber2, Len(sNumber2)-Len(gMeasurementUnit) ))
  fCalculateTolerance = Abs( dNumber1 - dNumber2 )

end function

