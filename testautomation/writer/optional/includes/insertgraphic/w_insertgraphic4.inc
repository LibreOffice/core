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
' #1 tInsertGraphic_31      'Test Alignment via format/graphic: Anchor to paragraph -> Left, centered, right, top, center, bottom
' #1 tInsertGraphic_32      'Test Alignment via format/graphic: Anchor to page -> Left, centered, right, top, center, bottom
' #1 tInsertGraphic_33      'Test Alignment via format/graphic: Anchor to character -> Left, centered, right, top, center, bottom
' #1 tInsertGraphic_34      'Test Alignment via format/graphic: Anchor as character -> top, center, bottom of baseline
' #1 tInsertGraphic_35      'Test Wrap via format/graphic : none, Page Wrap, Optimal Page Wrap, Contour Wrap ,Wrap through, in Background , first paragraph
' #1 tInsertGraphic_36      'Test original size
' #1 tInsertGraphic_37      'Test keep ratio
' #1 tInsertGraphic_38      'Check the graphic name in Navigator
' #1 tInsertGraphic_39      'Test checkbox 'Position' in protect area
' #1 tInsertGraphic_40      'Test checkbox 'Print' in Property area"
' #1 tInsertGraphic_41      'Test checkbox 'Keep image size'
' #1 tInsertGraphic_42      'Test checkbox 'Keep scale'
'*
'\***********************************************************************

testcase tInsertGraphic_31

  Dim iLeft as Integer
  Dim iRight as Integer
  Dim iCenterHorizontal as Integer
  Dim iTop as Integer
  Dim iBottom as Integer
  Dim iCenterVertical as Integer

  iLeft = 1 : iRight  = 2  : iCenterHorizontal = 3
  iTop  = 1 : iBottom = 2  : iCenterVertical   = 3

  PrintLog "- Test Alignment via format/graphic: Anchor to paragraph -> Left, centered, right, top, center, bottom"
 '/// Test Alignment via format/graphic: Anchor to paragraph -> Left, centered, right, top, center, bottom

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

   PrintLog "  Left:"
  '/// Format / Graphic tabpage Type ,
  '/// + Anchor to paragraph , and set Horizontal to left
   fFormatGraphic("TabType")
    Kontext "TabType"
    AnchorAtparagraph.Check
    Horizontal.Select iLeft
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Kontext "TabType"
    Sleep 1
    if AnchorAtparagraph.IsChecked <> TRUE  then Warnlog "Anchor should be checked to 'to paragraph' !"
    if Horizontal.GetSelIndex      <> iLeft then Warnlog "The graphic's Horizontal option is not correct ! "
   TabType.Cancel

   PrintLog "  Centered:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor to paragraph , and set Horizontal to Centered
   fFormatGraphic("TabType")
    Kontext "TabType"
    AnchorAtparagraph.Check
    Horizontal.Select iCenterHorizontal
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    Kontext "TabType"
    if AnchorAtparagraph.IsChecked <> TRUE              then Warnlog "Anchor should be checked to 'to paragraph' !"
    if Horizontal.GetSelIndex      <> iCenterHorizontal then Warnlog "The graphic's Horizontal option is not correct ! "
   TabType.Cancel

   PrintLog "  Right:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor to paragraph , and set Horizontal to Right
   fFormatGraphic("TabType")
    Kontext "TabType"
    AnchorAtparagraph.Check
    Horizontal.Select iRight
   TabType.OK
   sleep(1)

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    Kontext "TabType"
    if AnchorAtparagraph.IsChecked <> TRUE    then Warnlog "Anchor should be checked to 'to paragraph' !"
    if Horizontal.GetSelIndex      <> iRight  then Warnlog "The graphic's Horizontal option is not correct ! "
   TabType.Cancel

   PrintLog "  Top:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor to paragraph , and set Vertical to top
   fFormatGraphic("TabType")
    Kontext "TabType"
    AnchorAtparagraph.Check
    Vertical.Select iTop
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    Kontext "TabType"
    if AnchorAtparagraph.IsChecked <> TRUE  then Warnlog "Anchor should be checked to 'to paragraph' !"
    if Vertical.GetSelIndex        <> iTop  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Center:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor to paragraph , and set Vertical to Center
   fFormatGraphic("TabType")
    Kontext "TabType"
    AnchorAtparagraph.Check
    Vertical.Select iCenterVertical
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtparagraph.IsChecked <> TRUE             then Warnlog "Anchor should be checked to 'to paragraph' !"
    if Vertical.GetSelIndex        <> iCenterVertical  then Warnlog "The graphic's Vertical option is not correct ! #bug110281"
   TabType.Cancel

   PrintLog "  Bottom:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor to paragraph , and set Vertical to Bottom
   fFormatGraphic("TabType")
    AnchorAtparagraph.Check
    Vertical.Select iBottom
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtparagraph.IsChecked <> TRUE     then Warnlog "Anchor should be checked to 'to paragraph' !"
    if Vertical.GetSelIndex        <> iBottom  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_32

  Dim iLeft as Integer
  Dim iRight as Integer
  Dim iCenterHorizontal as Integer
  Dim iTop as Integer
  Dim iBottom as Integer
  Dim iCenterVertical as Integer

  iLeft = 1 : iRight  = 2  : iCenterHorizontal = 3
  iTop  = 1 : iBottom = 2  : iCenterVertical   = 3

  PrintLog "- Test Alignment via format/graphic: Anchor to page -> Left, centered, right, top, center, bottom"
 '/// Test Alignment via format/graphic: Anchor to page -> Left, centered, right, top, center, bottom

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

   PrintLog "  Left:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor to page , and set Horizontal to left
   fFormatGraphic("TabType")
    AnchorAtPage.Check
    Horizontal.Select iLeft
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtPage.IsChecked <> TRUE  then Warnlog "Anchor should be checked to 'At Page' !"
    if Horizontal.GetSelIndex <> iLeft then Warnlog "The graphic's Horizontal option is not correct ! "
   TabType.Cancel

   PrintLog "  Centered:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor to page , and set Horizontal to Centered
   fFormatGraphic("TabType")
    AnchorAtPage.Check
    Horizontal.Select iCenterHorizontal
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtPage.IsChecked <> TRUE              then Warnlog "Anchor should be checked to 'At Page' !"
    if Horizontal.GetSelIndex <> iCenterHorizontal then Warnlog "The graphic's Horizontal option is not correct ! "
   TabType.Cancel

   PrintLog "  Right:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor to page , and set Horizontal to Right
   fFormatGraphic("TabType")
    AnchorAtPage.Check
    Horizontal.Select iRight
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtPage.IsChecked <> TRUE    then Warnlog "Anchor should be checked to 'At Page' !"
    if Horizontal.GetSelIndex <> iRight  then Warnlog "The graphic's Horizontal option is not correct ! "
   TabType.Cancel

   PrintLog "  Top:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor to page , and set Vertical to top
   fFormatGraphic("TabType")
    AnchorAtPage.Check
    Vertical.Select iTop
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtPage.IsChecked <> TRUE  then Warnlog "Anchor should be checked to 'At Page' !"
    if Vertical.GetSelIndex   <> iTop  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Center:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor to page , and set Vertical to Center
   fFormatGraphic("TabType")
    AnchorAtPage.Check
    Vertical.Select iCenterVertical
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtPage.IsChecked <> TRUE             then Warnlog "Anchor should be checked to 'At Page' !"
    if Vertical.GetSelIndex   <> iCenterVertical  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Bottom:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor to page , and set Vertical to Bottom
   fFormatGraphic("TabType")
    AnchorAtPage.Check
    Vertical.Select iBottom
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtPage.IsChecked <> TRUE     then Warnlog "Anchor should be checked to 'At Page' !"
    if Vertical.GetSelIndex   <> iBottom  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_33

    Dim iLeft as Integer
    Dim iRight as Integer
    Dim iCenterHorizontal as Integer
    Dim iTop as Integer
    Dim iBottom as Integer
    Dim iCenterVertical as Integer
    
    iLeft = 1 : iRight  = 2  : iCenterHorizontal = 3
    iTop  = 1 : iBottom = 2  : iCenterVertical   = 4
    
    PrintLog "- Test Alignment via format/graphic: Anchor to character -> Left, centered, right, top, center, bottom"
    '/// Test Alignment via format/graphic: Anchor to character -> Left, centered, right, top, center, bottom
    
    Call hNewDocument
    
    '/// Insert a graphics
    fInsertGraphics("Bughunter.jpg")
    
    PrintLog "  Left:"
    '/// Format / Graphic tabpage Type
    '/// + Anchor At Character , and set Horizontal to left
    fFormatGraphic("TabType")
    AnchorAtCharacter.Check
    Horizontal.Select iLeft
    TabType.OK
    
    '/// Check the graphic position
    fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtCharacter.IsChecked <> TRUE  then Warnlog "Anchor should be checked to 'At Character' !"
    if Horizontal.GetSelIndex      <> iLeft then Warnlog "The graphic's Horizontal option is not correct ! "
    TabType.Cancel
    
    PrintLog "  Centered:"
    '/// Format / Graphic tabpage Type
    '/// + Anchor At Character , and set Horizontal to Centered
    fFormatGraphic("TabType")
    AnchorAtCharacter.Check
    Horizontal.Select iCenterHorizontal
    TabType.OK
    
    '/// Check the graphic position
    fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtCharacter.IsChecked <> TRUE              then Warnlog "Anchor should be checked to 'At Character' !"
    if Horizontal.GetSelIndex      <> iCenterHorizontal then Warnlog "The graphic's Horizontal option is not correct ! "
    TabType.Cancel
    
    PrintLog "  Right:"
    '/// Format / Graphic tabpage Type
    '/// + Anchor At Character , and set Horizontal to Right
    fFormatGraphic("TabType")
    AnchorAtCharacter.Check
    Horizontal.Select iRight
    TabType.OK
    
    '/// Check the graphic position
    fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtCharacter.IsChecked <> TRUE    then Warnlog "Anchor should be checked to 'At Character' !"
    if Horizontal.GetSelIndex      <> iRight  then Warnlog "The graphic's Horizontal option is not correct ! "
    TabType.Cancel
    
    PrintLog "  Top:"
    '/// Format / Graphic tabpage Type
    '/// + Anchor At Character , and set Vertical to top
    fFormatGraphic("TabType")
    AnchorAtCharacter.Check
    Vertical.Select iTop
    TabType.OK
    
    '/// Check the graphic position
    fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtCharacter.IsChecked <> TRUE  then Warnlog "Anchor should be checked to 'At Character' !"
    if Vertical.GetSelIndex        <> iTop  then Warnlog "The graphic's Vertical option is not correct ! "
    TabType.Cancel
    
    PrintLog "  Center:"
    '/// Format / Graphic tabpage Type
    '/// + Anchor At Character , and set Vertical to Center
    fFormatGraphic("TabType")
    AnchorAtCharacter.Check
    Vertical.Select iCenterVertical
    TabType.OK
    
    '/// Check the graphic position
    fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtCharacter.IsChecked <> TRUE             then Warnlog "Anchor should be checked to 'At Character' !"
    if Vertical.GetSelIndex        <> iCenterVertical  then
        if iSprache = 82 then
            QAErrorlog "#i59378#Missing vertical position in in Format-Picture-dialog"
        else
            if Vertical.GetItemCount <> 6 then
                QAErrorlog "#i59378#Missing vertical position in in Format-Picture-dialog"
            else
                Warnlog "The graphic's Vertical option is not correct ! "
            end if
        end if
    end if
    TabType.Cancel
    
    PrintLog "  Bottom:"
    '/// Format / Graphic tabpage Type
    '/// + Anchor At Character , and set Vertical to Bottom
    fFormatGraphic("TabType")
    AnchorAtCharacter.Check
    Vertical.Select iBottom
    TabType.OK
    
    '/// Check the graphic position
    fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtCharacter.IsChecked <> TRUE     then Warnlog "Anchor should be checked to 'At Character' !"
    if Vertical.GetSelIndex        <> iBottom  then Warnlog "The graphic's Vertical option is not correct ! "
    TabType.Cancel
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_34

  Dim iTop as Integer
  Dim iBottom as Integer
  Dim iCenterVertical as Integer

  iTop  = 1 : iBottom = 2  : iCenterVertical   = 3

  PrintLog "- Test Alignment via format/graphic: Anchor as character -> top, center, bottom of baseline"
 '/// Test Alignment via format/graphic: Anchor as character -> top, center, bottom of baseline

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

   PrintLog "  Base line at top:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor as character , and set Base line at top
   fFormatGraphic("TabType")
    Sleep 1
    AnchorAsCharacter.Check
    Vertical.Select iTop
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAsCharacter.IsChecked <> TRUE  then Warnlog "Anchor should be checked to 'As character' !"
    if Horizontal.IsEnabled        =  TRUE  then Warnlog "The Horizontal listbox should not be enabled ! "
    if Vertical.GetSelIndex        <> iTop  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Base line at center:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor as character , and set Base line at center
   fFormatGraphic("TabType")
    Sleep 1
    AnchorAsCharacter.Check
    Vertical.Select iCenterVertical
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAsCharacter.IsChecked <> TRUE            then Warnlog "Anchor should be checked to 'As character' !"
    if Horizontal.IsEnabled        =  TRUE            then Warnlog "The Horizontal listbox should not be enabled ! "
    if Vertical.GetSelIndex        <> iCenterVertical then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Base line at bottom:"
  '/// Format / Graphic tabpage Type
  '/// + Anchor as character , and set Base line at bottom
   fFormatGraphic("TabType")
    Sleep 1
    AnchorAsCharacter.Check
    Vertical.Select iBottom
   TabType.OK

  '/// Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAsCharacter.IsChecked <> TRUE     then Warnlog "Anchor should be checked to 'As character' !"
    if Horizontal.IsEnabled        =  TRUE     then Warnlog "The Horizontal listbox should not be enabled ! "
    if Vertical.GetSelIndex        <> iBottom  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_35

  PrintLog "- Test Wrap via dialog : none, Page Wrap, Optimal Page Wrap, Contour Wrap ,Wrap through, in Background , first paragraph"
 '/// <b> Test Wrap via dialog , including </b>
 '/// +<b> none, Page Wrap, Optimal Page Wrap, Contour Wrap, </b>
 '/// +<b> Wrap through, in Background, first paragraph  </b>

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

   PrintLog "  1. No Wrap:"
  '/// Format / Graphic / tabpage Wrap - No page
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Kein.Check
   TabUmlauf.OK

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Kein.IsChecked <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
   TabUmlauf.Cancel

   PrintLog "  2. Page Wrap:"
  '/// Format / Graphic / tabpage Wrap - Page Wrap
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Parallel.Check
   TabUmlauf.OK

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Parallel.IsChecked <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
   TabUmlauf.Cancel

   PrintLog "  3. Optimal Page Wrap:"
  '/// Format / Graphic / tabpage Wrap - Optimal Page Wrap
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Dynamisch.Check
   TabUmlauf.OK

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Dynamisch.IsChecked <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
   TabUmlauf.Cancel

   PrintLog "  4. Wrap through:"
  '/// Format / Graphic / tabpage Wrap - Wrap through
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Durchlauf.Check
   TabUmlauf.OK

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Durchlauf.IsChecked <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
   TabUmlauf.Cancel

   PrintLog "  5. In Background:"
  '/// Format / Graphic / tabpage Wrap - In Background
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Durchlauf.Check
    ImHintergrund.Check
   TabUmlauf.OK

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Durchlauf.IsChecked     <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
    if ImHintergrund.IsChecked <> TRUE then Warnlog "Background configuration is wrong !"
   TabUmlauf.Cancel

   PrintLog "  6. first paragraph:"
  '/// Format / Graphic / tabpage Wrap - first paragraph
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Parallel.Check
    ErsterAbsatz.Check
   TabUmlauf.OK

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Parallel.IsChecked      <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
    if ErsterAbsatz.IsChecked  <> TRUE then Warnlog "first paragraph configuration is wrong !"
   TabUmlauf.Cancel

   PrintLog "  7. Contour:"
  '/// Format / Graphic / tabpage Wrap - first paragraph
   FormatWrapPageWrap
   Sleep 1
   fFormatGraphic("TabUmlauf")
    Sleep 1
    Kontur.Check
   TabUmlauf.OK

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Kontur.IsChecked  <> TRUE then Warnlog "Contour configuration is wrong !"
   TabUmlauf.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_36

  Dim sOriginalWidth as String
  Dim sOriginalHeight as String
  Dim sNewWidth as String
  Dim sNewHeight as String

  sNewWidth  = "7"+ gSeperator + "00" + gMeasurementUnit
  sNewHeight = "5"+ gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test original size "
 '/// Test original size

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

  '/// Format / Graphic tabpage Type , change the graphic's size
   fFormatGraphic("TabType")
    sOriginalWidth  = Width.GetText
    sOriginalHeight = Height.GetText
    KeepRatio.UnCheck
    wait 500
    Width.SetText  sNewWidth
    wait 500
    Height.SetText sNewHeight
    wait 500
   TabType.OK

  '/// Check if original size works well
   fFormatGraphic("TabType")
    if Width.GetText  <> sNewWidth       then Warnlog "The graphic's width is wrong !"
    wait 500
    if Height.GetText <> sNewHeight      then Warnlog "The graphic's height is wrong !"
    wait 500
    OriginalSize.Click
    Sleep 1
    if Width.GetText  <> sOriginalWidth  then Warnlog "The graphic's width isn't recovered to before!"
    wait 500
    if Height.GetText <> sOriginalHeight then Warnlog "The graphic's height isn't recovered to before !"
    wait 500
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_37

  Dim sWidth as String
  Dim sHeight as String
  Dim sNewWidth as String
  Dim sNewHeight as String

  sWidth  = "4"+ gSeperator + "00" + gMeasurementUnit
  sHeight = "3"+ gSeperator + "00" + gMeasurementUnit

  sNewWidth  = "8"+ gSeperator + "00" + gMeasurementUnit
  sNewHeight = "6"+ gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test keep ratio"
 '/// Test keep ratio

   Call hNewDocument

  '/// Insert a graphics and check 'keep ratio'
   fInsertGraphics("Bughunter.jpg")

   fFormatGraphic("TabType")
    KeepRatio.UnCheck
    wait 500
    Width.SetText  sWidth
    wait 500
    Height.SetText sHeight
    wait 500
    KeepRatio.Check
    wait 500
   TabType.OK
   Sleep 1

  '/// Format / Graphic tabpage Type , change the graphic's width
  '/// + Check if keep ratio works well
   fFormatGraphic("TabType")
    Kontext "TabType"
    Width.SetText sNewWidth
    Sleep 1
    if Height.GetText <> sNewHeight then Warnlog "The graphic's height is wrong !"
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_38

    Dim sGraphicName as String
    Dim sNavActualItemName as string

    sGraphicName = "TEST1"

    PrintLog "- Check the graphic name in Navigator"
    '/// Check the graphic name in Navigator

    Call hNewDocument

    '/// Insert a graphics
    Call fInsertGraphics("Bughunter.jpg")

    '/// Format / Graphic / Options , set a new name
    Call fFormatGraphic("TabZusaetze")
    sleep(3)
    Kontext "TabZusaetze"
    ObjektName.SetText sGraphicName
    sleep(3)
    TabZusaetze.OK
    sleep(1)

    '/// Active navigator to check the graphic name
    Call fStartupNavigator( "Graphics" , 2 )
    sleep(2)
    Kontext "NavigatorWriter"
        sNavActualItemName = Auswahlliste.GetitemText(2)
        if sNavActualItemName <> sGraphicName then
            warnlog "The graphic name should be " + sGraphicName & " but get " & sNavActualItemName & "!"
        end if
    Call fCloseNavigator

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_39

  Dim sHorizontalBy as String
  Dim sVerticalBy as String

  PrintLog "- Test checkbox 'Position' in protect area"
 '/// Test checkbox 'Position' in protect area

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

  '/// Format / Graphic / Options , check 'Position'
   fFormatGraphic("TabZusaetze")
    wait 500
    if PositionSchuetzen.IsChecked = TRUE then warnlog "The default situation of checkbox - position should be unchecked"
    wait 500
    PositionSchuetzen.Check
    wait 500
   TabZusaetze.OK

  'Check the original position
   fFormatGraphic("TabType")
    wait 500
    sHorizontalBy = HorizontalBy.GetText
    sVerticalBy   = VerticalBy.GetText
   TabType.Cancel

  '/// move the picture with keyboard
   Call wTypeKeys "<Down><Down><Right><Right>"

  '/// + Check if the position is changed
   fFormatGraphic("TabType")
     if HorizontalBy.GetText <> sHorizontalBy then warnlog "The position Horizontal should be " +sHorizontalBy + " but get " + HorizontalBy.GetText
     if VerticalBy.GetText   <> sVerticalBy   then warnlog "The position Vertical should be " +sVerticalBy + " but get " + VerticalBy.GetText
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_40
    
    Dim sHorizontalBy as String
    Dim sVerticalBy as String
    
    PrintLog "- Test checkbox 'Print' in Property area"
    '/// Test checkbox 'Print' in Property area
    
    Call hNewDocument
    
    '/// Insert a graphics
    fInsertGraphics("Bughunter.jpg")
    
    '/// Format / Graphic / Options , check Position
    fFormatGraphic("TabZusaetze")
    wait 500
    if Drucken.IsChecked <> TRUE then warnlog "The default situation of checkbox - print should be checked"
    wait 500
    PositionSchuetzen.UnCheck
    wait 500
    TabZusaetze.OK
    
    '/// Print to Printer
    PrintLog "-   Please go to printer to check if you get a blank paper!"
    FilePrint
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.Ok
            QAErrorLog "No Default-Printer!"
            Kontext "Printing"
            Sleep 1
            Printing.Cancel
        end if
    else
        Kontext "Printing"
        Sleep 1
        Printing.OK
        Sleep 5
    end if
    
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.Ok
        end if
    end if
    
    Call hCloseDocument
    
endcase

'-------------------------------------------------------------------------------------------
