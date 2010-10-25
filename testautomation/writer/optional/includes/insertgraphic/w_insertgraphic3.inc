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
'* short description : Test of the insert graphic function - 3
'*
'************************************************************************
'*
' #1 tInsertGraphic_21     'Test Alignment via dialog: anchor to paragraph -> Left, centered, right, top, center, bottom
' #1 tInsertGraphic_22     'Test Alignment via dialog: anchor to page -> Left, centered, right, top, center, bottom
' #1 tInsertGraphic_23     'Test Alignment via dialog: anchor to character -> Left, centered, right, top, center, bottom
' #1 tInsertGraphic_24     'Test Alignment via dialog: anchor as character -> top, center, bottom of baseline
' #1 tInsertGraphic_25     'Test Alignment via Contextmenu: anchor to paragraph -> Left, centered, right, top, center, bottom
' #1 tInsertGraphic_26     'Test Alignment via Contextmenu: anchor to page -> Left, centered, right, top, center, bottom
' #1 tInsertGraphic_27     'Test Alignment via Contextmenu: anchor to character -> Left, centered, right, top, center, bottom
' #1 tInsertGraphic_28     'Test Alignment via Contextmenu: anchor as character -> top, center, bottom of baseline
' #1 tInsertGraphic_29     'Test Wrap via dialog : none, Page Wrap, Optimal Page Wrap, Contour Wrap ,Wrap through, in Background , first paragraph
' #1 tInsertGraphic_30     'Test Wrap via ContextMenuTest : none, Page Wrap, Optimal Page Wrap, Contour Wrap ,Wrap through, in Background , first paragraph
'*
'\***********************************************************************

testcase tInsertGraphic_21

  Dim iLeft as Integer , iRight as Integer  , iCenterHorizontal as Integer
  Dim iTop  as Integer , iBottom as Integer , iCenterVertical   as Integer

  iLeft = 1 : iRight  = 2  : iCenterHorizontal = 3
  iTop  = 1 : iBottom = 2  : iCenterVertical   = 3

  PrintLog "- Test Alignment via dialog: anchor to paragraph -> Left, centered, right, top, center, bottom"
 '/// Test Alignment via dialog: anchor to paragraph -> Left, centered, right, top, center, bottom

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

  '/// Format / anchor / to paragraph
   FormatAnchorToParagraph
   Sleep 1

   PrintLog "  Left:"
  '/// Format / Alignment / Left
   FormatAlignmentLeft
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtparagraph.IsChecked <> TRUE  then Warnlog "Anchor should be checked to 'to paragraph' !"
    if Horizontal.GetSelIndex      <> iLeft then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex        <> iTop  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Centered:"
  '/// Format / Alignment / Centered
   FormatAlignmentCentered
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iCenterHorizontal then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop              then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Right:"
  '/// Format / Alignment / Right
   FormatAlignmentRight
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight  then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop    then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Top:"
  '/// Format / Alignment / Top
   FormatAlignmentTop
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight   then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop     then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Center:"
  '/// Format / Alignment / Center
   FormatAlignmentCenter
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight            then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iCenterVertical   then Warnlog "#110281# The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Bottom:"
  '/// Format / Alignment / Bottom
   FormatAlignmentBottom
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight   then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iBottom  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_22

  Dim iLeft as Integer , iRight  as Integer , iCenterHorizontal as Integer
  Dim iTop  as Integer , iBottom as Integer , iCenterVertical   as Integer

  iLeft = 1 : iRight  = 2  : iCenterHorizontal = 3
  iTop  = 1 : iBottom = 2  : iCenterVertical   = 3

  PrintLog "- Test Alignment via dialog: anchor to page -> Left, centered, right, top, center, bottom"
 '/// Test Alignment via dialog: anchor to page -> Left, centered, right, top, center, bottom

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

  '/// Format / anchor / To Page
   FormatAnchorToPage
   Sleep 1

   PrintLog "  Left:"
  '/// Format / Alignment / Left
   FormatAlignmentLeft
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtPage.IsChecked <> TRUE  then Warnlog "Anchor should be checked to 'To Page' !"
    if Horizontal.GetSelIndex <> iLeft then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Centered:"
  '/// Format / Alignment / Centered
   FormatAlignmentCentered
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iCenterHorizontal then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop              then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Right:"
  '/// Format / Alignment / Right
   FormatAlignmentRight
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight  then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop    then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Top:"
  '/// Format / Alignment / Top
   FormatAlignmentTop
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight   then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop     then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Center:"
  '/// Format / Alignment / Center
   FormatAlignmentCenter
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight            then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iCenterVertical   then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Bottom:"
  '/// Format / Alignment / Bottom
   FormatAlignmentBottom
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight   then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iBottom  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_23

  Dim iLeft as Integer , iRight  as Integer , iCenterHorizontal as Integer
  Dim iTop  as Integer , iBottom as Integer , iCenterVertical   as Integer

  iLeft = 1 : iRight  = 2  : iCenterHorizontal = 3
  iTop  = 1 : iBottom = 2  : iCenterVertical   = 3

  PrintLog "- Test Alignment via dialog: anchor to character -> Left, centered, right, top, center, bottom"
 '/// Test Alignment via dialog: anchor to character -> Left, centered, right, top, center, bottom

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

  '/// Format / anchor / To character
   FormatAnchorTocharacter
   Sleep 1

   PrintLog "  Left:"
  '/// Format / Alignment / Left
   FormatAlignmentLeft
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtCharacter.IsChecked <> TRUE   then Warnlog "Anchor should be checked to 'to paragraph' !"
    if Horizontal.GetSelIndex <> iLeft then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Centered:"
  '/// Format / Alignment / Centered
   FormatAlignmentCentered
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iCenterHorizontal then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop              then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Right:"
  '/// Format / Alignment / Right
   FormatAlignmentRight
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight  then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop    then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Top:"
  '/// Format / Alignment / Top
   FormatAlignmentTop
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight   then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop     then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Center:"
  '/// Format / Alignment / Center
   FormatAlignmentCenter
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight            then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iCenterVertical   then qaErrorLog "#111962# The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Bottom:"
  '/// Format / Alignment / Bottom
   FormatAlignmentBottom
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight   then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iBottom  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_24

  Dim iTop as Integer  , iBottom as Integer , iCenterVertical as Integer

  iTop  = 1 : iBottom = 2  : iCenterVertical   = 3

  PrintLog "- Test Alignment via dialog: anchor as character -> top, center, bottom of baseline"
 '/// Test Alignment via dialog: anchor as character -> top, center, bottom of baseline

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

  '/// Format / anchor / As character
   FormatAnchorAsCharacter
   Sleep 1

   PrintLog "  Base line at top:"
  '/// Format / Alignment / Base line at top
   FormatAlignmentTop
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.IsEnabled  =  TRUE  then Warnlog "The Horizontal listbox should not be enabled ! "
    if Vertical.GetSelIndex  <> iTop  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Base line at center:"
  '/// Format / Alignment / Base line at center
   FormatAlignmentCenter
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.IsEnabled  =  TRUE            then Warnlog "The Horizontal listbox should not be enabled ! "
    if Vertical.GetSelIndex  <> iCenterVertical then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Base line at bottom:"
  '/// Format / Alignment / Base line at bottom
   FormatAlignmentBottom
   Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.IsEnabled  =  TRUE     then Warnlog "The Horizontal listbox should not be enabled ! "
    if Vertical.GetSelIndex  <> iBottom  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_25

  Dim iLeft as Integer , iRight  as Integer , iCenterHorizontal as Integer
  Dim iTop  as Integer , iBottom as Integer , iCenterVertical   as Integer

  iLeft = 1 : iRight  = 2 : iCenterHorizontal = 3
  iTop  = 1 : iBottom = 2 : iCenterVertical   = 3

  PrintLog "- Test Alignment via Contextmenu: anchor to paragraph -> Left, centered, right, top, center, bottom"
 '/// Test Alignment via Contextmenu: anchor to paragraph -> Left, centered, right, top, center, bottom

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

  '/// ContextMenu anchor / to paragraph
   Call wOpenContextMenu
    hMenuSelectNr(3)
    Sleep 2
    hMenuSelectNr(2)
    Sleep 1

   PrintLog "  Left:"
  '/// ContextMenu Alignment / Left
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(1)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtparagraph.IsChecked <> TRUE  then Warnlog "Anchor should be checked to 'to paragraph' !"
    if Horizontal.GetSelIndex      <> iLeft then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex        <> iTop  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Centered:"
  '/// ContextMenu Alignment / Centered
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(2)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iCenterHorizontal then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop              then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Right:"
  '/// ContextMenu Alignment / Right
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(3)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight  then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop    then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Top:"
  '/// ContextMenu Alignment / Top
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(4)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight   then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop     then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Center:"
  '/// ContextMenu Alignment / Center
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(5)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight            then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iCenterVertical   then Warnlog "#110281# The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Bottom:"
  '/// ContextMenu Alignment / Bottom
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(6)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight   then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iBottom  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_26

  Dim iLeft as Integer , iRight  as Integer , iCenterHorizontal as Integer
  Dim iTop  as Integer , iBottom as Integer , iCenterVertical   as Integer

  iLeft = 1 : iRight  = 2  : iCenterHorizontal = 3
  iTop  = 1 : iBottom = 2  : iCenterVertical   = 3

  PrintLog "- Test Alignment via Contextmenu: anchor to page -> Left, centered, right, top, center, bottom"
 '/// Test Alignment via dialog: Contextmenu to page -> Left, centered, right, top, center, bottom

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

  '/// ContextMenu anchor / To Page
   Call wOpenContextMenu
    hMenuSelectNr(3)
    Sleep 2
    hMenuSelectNr(1)
    Sleep 1

   PrintLog "  Left:"
  '/// ContextMenu Alignment / Left
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(1)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtPage.IsChecked <> TRUE  then Warnlog "Anchor should be checked to 'To Page' !"
    if Horizontal.GetSelIndex <> iLeft then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Centered:"
  '/// ContextMenu Alignment / Centered
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(2)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iCenterHorizontal then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop              then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Right:"
  '/// ContextMenu Alignment / Right
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(3)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight  then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop    then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Top:"
  '/// ContextMenu Alignment / Top
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(4)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight   then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop     then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Center:"
  '/// ContextMenu Alignment / Center
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(5)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight            then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iCenterVertical   then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Bottom:"
  '/// ContextMenu Alignment / Bottom
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(6)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight   then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iBottom  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_27

  Dim iLeft as Integer , iRight  as Integer , iCenterHorizontal as Integer
  Dim iTop  as Integer , iBottom as Integer , iCenterVertical   as Integer

  iLeft = 1 : iRight  = 2  : iCenterHorizontal = 3
  iTop  = 1 : iBottom = 2  : iCenterVertical   = 3

  PrintLog "- Test Alignment via Contextmenu: anchor to character -> Left, centered, right, top, center, bottom"
 '/// Test Alignment via Contextmenu: anchor to character -> Left, centered, right, top, center, bottom

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

  '/// ContextMenu anchor / To character
   Call wOpenContextMenu
    hMenuSelectNr(3)
    Sleep 2
    hMenuSelectNr(3)
    Sleep 1

   PrintLog "  Left:"
  '/// ContextMenu Alignment / Left
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(1)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if AnchorAtCharacter.IsChecked <> TRUE  then Warnlog "Anchor should be checked to 'to paragraph' !"
    if Horizontal.GetSelIndex      <> iLeft then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex        <> iTop  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Centered:"
  '/// ContextMenu Alignment / Centered
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(2)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iCenterHorizontal then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop              then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Right:"
  '/// ContextMenu Alignment / Right
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(3)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight  then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop    then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Top:"
  '/// ContextMenu Alignment / Top
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(4)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight   then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iTop     then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Center:"
  '/// ContextMenu Alignment / Center
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(5)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight            then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iCenterVertical   then QAErrorlog "#111962#The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Bottom:"
  '/// ContextMenu Alignment / Bottom
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(6)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.GetSelIndex <> iRight   then Warnlog "The graphic's Horizontal option is not correct ! "
    if Vertical.GetSelIndex   <> iBottom  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_28

  Dim iTop as Integer , iBottom as Integer , iCenterVertical as Integer

  iTop  = 1 : iBottom = 2  : iCenterVertical   = 3

  PrintLog "- Test Alignment via Contextmenu: anchor as character -> top, center, bottom of baseline"
 '/// Test Alignment via Contextmenu: anchor as character -> top, center, bottom of baseline

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

  '/// ContextMenu anchor / As character
   Call wOpenContextMenu
    hMenuSelectNr(3)
    Sleep 2
    hMenuSelectNr(4)
    Sleep 1

   PrintLog "  Base line at top:"
  '/// ContextMenu Alignment / Base line at top
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(1)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.IsEnabled  = TRUE  then Warnlog "The Horizontal listbox should not be enabled ! "
    if Vertical.GetSelIndex <> iTop  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Base line at center:"
  '/// ContextMenu Alignment / Base line at center
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(2)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.IsEnabled  =  TRUE           then Warnlog "The Horizontal listbox should not be enabled ! "
    if Vertical.GetSelIndex <> iCenterVertical then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   PrintLog "  Base line at bottom:"
  '/// ContextMenu Alignment / Base line at bottom
   Call wOpenContextMenu
    hMenuSelectNr(2)
    Sleep 2
    hMenuSelectNr(3)
    Sleep 1

  '/// - Check the graphic position
   fFormatGraphic("TabType")
    Sleep 1
    if Horizontal.IsEnabled  =  TRUE    then Warnlog "The Horizontal listbox should not be enabled ! "
    if Vertical.GetSelIndex <> iBottom  then Warnlog "The graphic's Vertical option is not correct ! "
   TabType.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_29

  PrintLog "- Test Wrap via dialog : none, Page Wrap, Optimal Page Wrap, Contour Wrap ,Wrap through, in Background , first paragraph"
 '/// <b> Test Wrap via dialog , including </b>
 '/// + </b> none, Page Wrap, Optimal Page Wrap, Contour Wrap, </b>
 '/// + </b> Wrap through, in Background, first paragraph </b>

   Call hNewDocument

  '/// Insert a graphics
   fInsertGraphics("Bughunter.jpg")

   PrintLog "  1. No Wrap:"
  '/// Format / Wrap / No Wrap
   FormatWrapNoWrap
   Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Kein.IsChecked <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
   TabUmlauf.Cancel

   PrintLog "  2. Page Wrap:"
  '/// Format / Wrap / Page Wrap
   FormatWrapPageWrap
   Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Parallel.IsChecked <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
   TabUmlauf.Cancel

   PrintLog "  3. Optimal Page Wrap:"
  '/// Format / Wrap / Optimal Page Wrap
   FormatWrapOptimalPageWrap
   Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Dynamisch.IsChecked <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
   TabUmlauf.Cancel

   PrintLog "  4. Wrap through:"
  '/// Format / Wrap / Wrap through
   FormatWrapThrough
   Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Durchlauf.IsChecked <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
   TabUmlauf.Cancel

   PrintLog "  5. In Background:"
  '/// Format / Wrap / in Background
   FormatWrapInBackground
   Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Durchlauf.IsChecked     <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
    if ImHintergrund.IsChecked <> TRUE then Warnlog "Background configuration is wrong !"
   TabUmlauf.Cancel

   PrintLog "  6. first paragraph:"
  '/// Format / Wrap / first paragraph
   FormatWrapFirstparagraph
   Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Parallel.IsChecked      <> TRUE then QaErrorLog "#i94558# - Graphic wrapping situation is wrong !"
    if ErsterAbsatz.IsChecked  <> TRUE then Warnlog "first paragraph configuration is wrong !"
   TabUmlauf.Cancel

   PrintLog "  7. Contour:"
  '/// Format / Wrap / Contour
   FormatWrapPageWrap
   Sleep 1
   FormatWrapContour
   Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Kontur.IsChecked  <> TRUE then Warnlog "Contour configuration is wrong !"
   TabUmlauf.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tInsertGraphic_30

  PrintLog "- Test Wrap via Contextmenu : none, Page Wrap, Optimal Page Wrap, Contour Wrap ,Wrap through, in Background , first paragraph"
 '/// <b> Test Wrap via Contextmenu , including </b>
 '/// + <b> none, Page Wrap, Optimal Page Wrap, Contour Wrap, </b>
 '/// + <b> Wrap through, in Background, first paragraph   </b>

   Call hNewDocument

  '/// + Insert a graphics
   fInsertGraphics("Bughunter.jpg")

   PrintLog "  1. No Wrap:"
  '/// Contextmenu Wrap / No Wrap
   Call wOpenContextMenu
    hMenuSelectNr(4)
    Sleep 2
    hMenuSelectNr(1)
    Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Kein.IsChecked <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
   TabUmlauf.Cancel

   PrintLog "  2. Page Wrap:"
  '/// Contextmenu Wrap / Page Wrap
   Call wOpenContextMenu
    hMenuSelectNr(4)
    Sleep 2
    hMenuSelectNr(2)
    Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Parallel.IsChecked <> TRUE then QaErrorLog "#i94558# - Graphic wrapping situation is wrong !"
   TabUmlauf.Cancel

   PrintLog "  3. Optimal Page Wrap:"
  '/// Contextmenu Wrap / Optimal Page Wrap
   Call wOpenContextMenu
    hMenuSelectNr(4)
    Sleep 2
    hMenuSelectNr(3)
    Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Dynamisch.IsChecked <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
   TabUmlauf.Cancel

   PrintLog "  4. Wrap through:"
  '/// Contextmenu Wrap / Wrap through
   Call wOpenContextMenu
    hMenuSelectNr(4)
    Sleep 2
    hMenuSelectNr(4)
    Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Durchlauf.IsChecked <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
   TabUmlauf.Cancel

   PrintLog "  5. In Background:"
  '/// Contextmenu Wrap / in Background
   Call wOpenContextMenu
    hMenuSelectNr(4)
    Sleep 2
    hMenuSelectNr(5)
    Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Durchlauf.IsChecked     <> TRUE then Warnlog "Graphic wrapping situation is wrong !"
    if ImHintergrund.IsChecked <> TRUE then Warnlog "Background configuration is wrong !"
   TabUmlauf.Cancel

   PrintLog "  6. first paragraph:"
  '/// Contextmenu Wrap / first paragraph
   Call wOpenContextMenu
    hMenuSelectNr(4)
    Sleep 2
    hMenuSelectNr(7)
    Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Parallel.IsChecked      <> TRUE then QaErrorLog "#i94558# - Graphic wrapping situation is wrong !"
    if ErsterAbsatz.IsChecked  <> TRUE then Warnlog "first paragraph configuration is wrong !"
   TabUmlauf.Cancel

   PrintLog "  7. Contour:"
  '/// Contextmenu Wrap / Contour
   Call wOpenContextMenu
    hMenuSelectNr(4)
    Sleep 2
    hMenuSelectNr(2)
    Sleep 1

   Call wOpenContextMenu
    hMenuSelectNr(4)
    Sleep 2
    hMenuSelectNr(6)
    Sleep 1

   fFormatGraphic("TabUmlauf")
    Sleep 1
    if Kontur.IsChecked  <> TRUE then Warnlog "Contour configuration is wrong !"
   TabUmlauf.Cancel

   Call hCloseDocument

endcase
