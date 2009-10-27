'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: id_003.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:41 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* Owner : wolfram.garten@sun.com
'*
'* short description :
'*
'***********************************************************************************
' #1 tiViewNavigator
' #1 tiViewZoom
' #1 tiViewToolbar
' #1 tiViewDisplayQuality
' #1 tiViewLayer
' #1 tViewSnapLines
' #1 tViewGrid
'\**********************************************************************************

testcase tiViewNavigator
    Call hNewDocument

    Kontext "NavigatorDraw"
    if Not NavigatorDraw.Exists Then
        ViewNavigator
    end if
    Kontext "NavigatorDraw"
    Call DialogTest ( NavigatorDraw )

    try
        Kontext "Navigator"
        Navigator.Close
    catch
        Errorlog "  Navigator wasn't closed, second try with Menu"
        ViewNavigator
    endcatch
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tiViewZoom
    Call  hNewDocument
    UseBindings
    ViewZoom
    Kontext "Massstab"
    DialogTest ( Massstab )
    Massstab.Cancel
    Call  hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tiViewToolbar
    Call  hNewDocument

    ViewToolbarsThreeDSettings
    WaitSlot (1000)
    ViewToolbarsThreeDSettings
    WaitSlot (1000)

    ViewToolbarsAlign
    WaitSlot (1000)
    ViewToolbarsAlign
    WaitSlot (1000)

    ViewToolbarsTools
    WaitSlot (1000)
    ViewToolbarsTools
    WaitSlot (1000)

    ViewToolbarsBezier
    WaitSlot (1000)
    ViewToolbarsBezier
    WaitSlot (1000)

    ViewToolbarsFontwork
    WaitSlot (1000)
    ViewToolbarsFontwork
    WaitSlot (1000)

    '   if gApplication = "IMPRESS" then
    '      ViewToolbarsPresentation ' only in impress, not draw
    '      ViewToolbarsPresentation
    '   endif

    ViewToolbarsFormControls
    WaitSlot (1000)
    ViewToolbarsFormControls
    WaitSlot (1000)

    '-----------------
    ViewToolbarsFormDesign
    WaitSlot (1000)
    ViewToolbarsFormDesign
    WaitSlot (1000)

    ViewToolbarsFormNavigation
    WaitSlot (1000)
    ViewToolbarsFormNavigation
    WaitSlot (1000)

    ViewToolbarsGluepoints
    WaitSlot (1000)
    ViewToolbarsGluepoints
    WaitSlot (1000)
    ViewToolbarsInsert
    WaitSlot (1000)
    ViewToolbarsInsert
    WaitSlot (1000)

    ViewToolbarsGraphic
    WaitSlot (1000)
    ViewToolbarsGraphic
    WaitSlot (1000)

    ViewToolbarsMediaPlayback
    WaitSlot (1000)
    ViewToolbarsMediaPlayback
    WaitSlot (1000)

    ViewToolbarsOptionbar
    WaitSlot (1000)
    ViewToolbarsOptionbar
    WaitSlot (1000)

    ViewToolbarsPicture
    WaitSlot (1000)
    ViewToolbarsPicture
    WaitSlot (1000)

    ViewToolbarsStandard
    WaitSlot (1000)
    ViewToolbarsStandard
    WaitSlot (1000)

    ViewToolbarsStandardView
    WaitSlot (1000)
    ViewToolbarsStandardView
    WaitSlot (1000)

    ViewToolbarsHyperlinkbar
    WaitSlot (1000)
    ViewToolbarsHyperlinkbar
    WaitSlot (1000)

    ViewToolbarsColorBar
    WaitSlot (1000)
    ViewToolbarsColorBar
    WaitSlot (1000)

    ViewToolbarsCustomize
    WaitSlot (1000)
    Kontext
    try
        Messagebox.SetPage TabCustomizeMenu             ' 1 ------------------
    catch
        warnlog "couldn't switch to tabpage 'Menus'"
    endcatch
    Kontext "TabCustomizeMenu"
    if TabCustomizeMenu.exists(5) then
        Call DialogTest ( TabCustomizeMenu )
        Menu.typeKeys("<down>")
        Entries.typeKeys("<down>")
        sleep 2
        BtnNew.Click
        sleep 1
        Kontext "MenuOrganiser"
        Call DialogTest ( MenuOrganiser )
        MenuOrganiser.cancel
        sleep 1
        Kontext "TabCustomizeMenu"
        TabCustomizeMenu.Close
    end if
    sleep (1)

    Call  hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tiViewDisplayQuality
    Call hNewDocument

    Call hRechteckErstellen 20,20,40,40

    try
        ViewQualityBlackWhite
        Printlog "- Quality set to black and white"
    catch
        Warnlog "- Slot could not be accessed"
    endcatch
    WaitSlot (1000)
    try
        ViewQualityGreyscale
        Printlog "- View quality set to greyscale"
    catch
        Warnlog "- View quality greyscale could not be accessed"
    endcatch
    WaitSlot (1000)
    try
        ViewQualityColour
        Printlog "- View quality set to colour"
    catch
        Warnlog "- View quality colour could not be accessed"
    endcatch
    Call hClosedocument
endcase

'-------------------------------------------------------------------------

testcase tiViewLayer
    Call hNewDocument

    ViewLayer
    WaitSlot (1000)
    ViewLayer
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tViewGrid
    Call  hNewDocument

    ViewGridVisible
    ViewGridUse
    ViewGridFront
    ViewGridVisible
    ViewGridUse
    ViewGridFront
    WaitSlot (1000)
    Call  hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tViewSnapLines
    Call  hNewDocument

    ViewSnapLinesVisible
    ViewSnapLinesUse
    ViewSnapLinesFront
    ViewSnapLinesVisible
    ViewSnapLinesUse
    ViewSnapLinesFront
    WaitSlot (1000)
    Call  hCloseDocument
endcase

