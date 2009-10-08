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
'* last change: $Author: jsi $ $Date: 2008-06-16 10:43:00 $
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
    '/// open application ///'
    Call hNewDocument

    Kontext "NavigatorDraw"
        '/// if 'Navigator' isn't already open, open it (View->Navigator)///'
        if Not NavigatorDraw.Exists Then
            ViewNavigator
        end if
    Kontext "NavigatorDraw"
        Call DialogTest ( NavigatorDraw )

        try
            Kontext "Navigator"
                '/// close 'Navigator' ///'
                Navigator.Close
        catch
            Errorlog "  Navigator wasn't closed, second try with Menu"
            ViewNavigator
        endcatch
    '/// close application ///'
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tiViewZoom
'/// open application ///'
   Call  hNewDocument
   UseBindings
   ViewZoom
   Kontext "Massstab"
      DialogTest ( Massstab )
      Massstab.Cancel
'/// close application ///'
   Call  hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tiViewToolbar
'/// including ruler & statusbar ///'
'/// open application ///'
   Call  hNewDocument

'/// View->Toolbars->ThreeDSettings ///'
    ViewToolbarsThreeDSettings
    WaitSlot (1000)
'/// View->Toolbars->ThreeDSettings ///'
    ViewToolbarsThreeDSettings
    WaitSlot (1000)

'/// View->Toolbars->Align ///'
    ViewToolbarsAlign
    WaitSlot (1000)
'/// View->Toolbars->Align ///'
    ViewToolbarsAlign
    WaitSlot (1000)

'/// View->Toolbars->Tools ///'
    ViewToolbarsTools
    WaitSlot (1000)
'/// View->Toolbars->Tools ///'
    ViewToolbarsTools
    WaitSlot (1000)

'/// View->Toolbars->Bezier ///'
    ViewToolbarsBezier
    WaitSlot (1000)
'/// View->Toolbars->Bezier ///'
    ViewToolbarsBezier
    WaitSlot (1000)

'/// View->Toolbars->Fontwork ///'
    ViewToolbarsFontwork
    WaitSlot (1000)
'/// View->Toolbars->Fontwork ///'
    ViewToolbarsFontwork
    WaitSlot (1000)

    '   if LCase(gApplication) = "impress" then
    '/// View->Toolbars->Presentation ///'
    '      ViewToolbarsPresentation ' only in impress, not draw
    '/// View->Toolbars->Presentation ///'
    '      ViewToolbarsPresentation
    '   endif

'/// View->Toolbars->Form Controls ///'
    ViewToolbarsFormControls
    WaitSlot (1000)
'/// View->Toolbars->Form Controls ///'
    ViewToolbarsFormControls
    WaitSlot (1000)

'-----------------
'/// View->Toolbars->Form Design ///'
    ViewToolbarsFormDesign
    WaitSlot (1000)
'/// View->Toolbars->Form Design ///'
    ViewToolbarsFormDesign
    WaitSlot (1000)

'/// View->Toolbars->FormNavigation ///'
    ViewToolbarsFormNavigation
    WaitSlot (1000)
'/// View->Toolbars->FormNavigation ///'
    ViewToolbarsFormNavigation
    WaitSlot (1000)

'/// View->Toolbars->Gluepoints ///'
    ViewToolbarsGluepoints
    WaitSlot (1000)
'/// View->Toolbars->Gluepoints ///'
    ViewToolbarsGluepoints
    WaitSlot (1000)
'/// View->Toolbars->Insert ///'
    ViewToolbarsInsert
    WaitSlot (1000)
'/// View->Toolbars->Insert ///'
    ViewToolbarsInsert
    WaitSlot (1000)

'/// View->Toolbars->Graphic ///'
    ViewToolbarsGraphic
    WaitSlot (1000)
'/// View->Toolbars->Graphic ///'
    ViewToolbarsGraphic
    WaitSlot (1000)

'/// View->Toolbars->MediaPlayback ///'
    ViewToolbarsMediaPlayback
    WaitSlot (1000)
'/// View->Toolbars->MediaPlayback ///'
    ViewToolbarsMediaPlayback
    WaitSlot (1000)

'/// View->Toolbars->Optionbar ///'
    ViewToolbarsOptionbar
    WaitSlot (1000)
'/// View->Toolbars->Optionbar ///'
    ViewToolbarsOptionbar
    WaitSlot (1000)

'/// View->Toolbars->Picture ///'
    ViewToolbarsPicture
    WaitSlot (1000)
'/// View->Toolbars->Picture ///'
    ViewToolbarsPicture
    WaitSlot (1000)

'/// View->Toolbars->Standard ///'
    ViewToolbarsStandard
    WaitSlot (1000)
'/// View->Toolbars->Standard ///'
    ViewToolbarsStandard
    WaitSlot (1000)

'/// View->Toolbars->Standard View ///'
    ViewToolbarsStandardView
    WaitSlot (1000)
'/// View->Toolbars->Standard View ///'
    ViewToolbarsStandardView
    WaitSlot (1000)

'/// View->Toolbars->Hyperlinkbar ///'
    ViewToolbarsHyperlinkbar
    WaitSlot (1000)
'/// View->Toolbars->Hyperlinkbar ///'
    ViewToolbarsHyperlinkbar
    WaitSlot (1000)

'/// View->Toolbars->ColorBar ///'
    ViewToolbarsColorBar
    WaitSlot (1000)
'/// View->Toolbars->ColorBar ///'
    ViewToolbarsColorBar
    WaitSlot (1000)

'/// View->Toolbars->Customize ///'
    ViewToolbarsCustomize
    WaitSlot (1000)
    Kontext
    '/// switch to tabpage 'Menus' ///'
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
        '/// click button 'new' ///'
        BtnNew.Click
        sleep 1
        '/// Dialog 'New Menu' comes up ///'
        Kontext "MenuOrganiser"
        Call DialogTest ( MenuOrganiser )
        '/// Cancel dialog 'New Menu' ///'
        MenuOrganiser.cancel
        sleep 1
        Kontext "TabCustomizeMenu"
        TabCustomizeMenu.Close
    end if
    sleep (1)

'/// close application ///'
   Call  hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tiViewDisplayQuality
    '/// open application ///'
    Call hNewDocument

    '/// create a rectangle ///'
    Call hRechteckErstellen 20,20,40,40

    try
    '/// View->Display Quality->Black and White ///'
        ViewQualityBlackWhite
        Printlog "- Quality set to black and white"
    catch
        Warnlog "- Slot could not be accessed"
    endcatch
    WaitSlot (1000)
    try
        '/// View->Display Quality->Greyscale ///'
        ViewQualityGreyscale
        Printlog "- View quality set to greyscale"
    catch
        Warnlog "- View quality greyscale could not be accessed"
    endcatch
    WaitSlot (1000)
    try
        '/// View->Display Quality->Colour ///'
        ViewQualityColour
        Printlog "- View quality set to colour"
    catch
        Warnlog "- View quality colour could not be accessed"
    endcatch
    '/// close application ///'
    Call hClosedocument
endcase

'-------------------------------------------------------------------------

testcase tiViewLayer
    '/// open application ///'
    Call hNewDocument
    '/// Insert->Layer / in impress: ???? ///'

    '/// View->Layer ///'
    ViewLayer
    WaitSlot (1000)
    '/// View->Layer ///'
    ViewLayer
    '/// close application ///'
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tViewGrid
    '/// open application ///'
    Call  hNewDocument

    '/// View->Grid-> ///'
    ViewGridVisible
    '/// View->Grid-> ///'
    ViewGridUse
    '/// View->Grid-> ///'
    ViewGridFront
    '/// View->Grid-> ///'
    ViewGridVisible
    '/// View->Grid-> ///'
    ViewGridUse
    '/// View->Grid-> ///'
    ViewGridFront
    WaitSlot (1000)
    '/// close application ///'
    Call  hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tViewSnapLines
    '/// open application ///'
    Call  hNewDocument

    '/// View->Snap Lines-> ///'
    ViewSnapLinesVisible
    '/// View->Snap Lines-> ///'
    ViewSnapLinesUse
    '/// View->Snap Lines-> ///'
    ViewSnapLinesFront
    '/// View->Snap Lines-> ///'
    ViewSnapLinesVisible
    '/// View->Snap Lines-> ///'
    ViewSnapLinesUse
    '/// View->Snap Lines-> ///'
    ViewSnapLinesFront
    WaitSlot (1000)
    '/// close application ///'
    Call  hCloseDocument
endcase

