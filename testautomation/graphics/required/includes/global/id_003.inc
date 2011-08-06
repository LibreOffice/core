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
'\******************************************************************************

sub id_003

    printLog Chr(13) + "---------    id_003    ----------"

    call tiViewNavigator
    call tiViewZoom
    call tiViewToolbar
    Call tToolsCustomize			     'global\required\include
    call tiViewDisplayQuality
    call tiViewLayer
    call tViewSnapLines
    call tViewGrid
end sub
'-------------------------------------------------------------------------
testcase tiViewNavigator

    printlog " open application "
    Call hNewDocument

    Kontext "NavigatorDraw"
        printlog " if 'Navigator' isn't already open, open it (View->Navigator)"
        if Not NavigatorDraw.Exists Then
            ViewNavigator
        end if
    Kontext "NavigatorDraw"
        Call DialogTest ( NavigatorDraw )

        try
            Kontext "Navigator"
                printlog " close 'Navigator' "
                Navigator.Close
        catch
            Errorlog "  Navigator wasn't closed, second try with Menu"
            ViewNavigator
        endcatch
    printlog " close application "
    Call hCloseDocument
    
endcase 'tiViewNavigator
'-------------------------------------------------------------------------
testcase tiViewZoom

    printlog " open application "
    Call  hNewDocument
    UseBindings
    ViewZoom
    Kontext "Massstab"
    DialogTest ( Massstab )
    Massstab.Cancel
    printlog " close application "
    Call  hCloseDocument
   
endcase 'tiViewZoom
'-------------------------------------------------------------------------
testcase tiViewToolbar

    printlog " including ruler & statusbar "
    printlog " open application "
   Call  hNewDocument

    printlog " View->Toolbars->ThreeDSettings "
    ViewToolbarsThreeDSettings
    WaitSlot (1000)
    printlog " View->Toolbars->ThreeDSettings "
    ViewToolbarsThreeDSettings
    WaitSlot (1000)

    printlog " View->Toolbars->Align "
    ViewToolbarsAlign
    WaitSlot (1000)
    printlog " View->Toolbars->Align "
    ViewToolbarsAlign
    WaitSlot (1000)

    printlog " View->Toolbars->Tools "
    ViewToolbarsTools
    WaitSlot (1000)
    printlog " View->Toolbars->Tools "
    ViewToolbarsTools
    WaitSlot (1000)

    printlog " View->Toolbars->Bezier "
    ViewToolbarsBezier
    WaitSlot (1000)
    printlog " View->Toolbars->Bezier "
    ViewToolbarsBezier
    WaitSlot (1000)

    printlog " View->Toolbars->Fontwork "
    ViewToolbarsFontwork
    WaitSlot (1000)
    printlog " View->Toolbars->Fontwork "
    ViewToolbarsFontwork
    WaitSlot (1000)

    '   if gApplication = "IMPRESS" then
    printlog " View->Toolbars->Presentation "
    '      ViewToolbarsPresentation ' only in impress, not draw
    printlog " View->Toolbars->Presentation "
    '      ViewToolbarsPresentation
    '   endif

    printlog " View->Toolbars->Form Controls "
    ViewToolbarsFormControls
    WaitSlot (1000)
    printlog " View->Toolbars->Form Controls "
    ViewToolbarsFormControls
    WaitSlot (1000)

'-----------------
    printlog " View->Toolbars->Form Design "
    ViewToolbarsFormDesign
    WaitSlot (1000)
    printlog " View->Toolbars->Form Design "
    ViewToolbarsFormDesign
    WaitSlot (1000)

    printlog " View->Toolbars->FormNavigation "
    ViewToolbarsFormNavigation
    WaitSlot (1000)
    printlog " View->Toolbars->FormNavigation "
    ViewToolbarsFormNavigation
    WaitSlot (1000)

    printlog " View->Toolbars->Gluepoints "
    ViewToolbarsGluepoints
    WaitSlot (1000)
    printlog " View->Toolbars->Gluepoints "
    ViewToolbarsGluepoints
    WaitSlot (1000)
    printlog " View->Toolbars->Insert "
    ViewToolbarsInsert
    WaitSlot (1000)
    printlog " View->Toolbars->Insert "
    ViewToolbarsInsert
    WaitSlot (1000)

    printlog " View->Toolbars->Graphic "
    ViewToolbarsGraphic
    WaitSlot (1000)
    printlog " View->Toolbars->Graphic "
    ViewToolbarsGraphic
    WaitSlot (1000)

    printlog " View->Toolbars->MediaPlayback "
    ViewToolbarsMediaPlayback
    WaitSlot (1000)
    printlog " View->Toolbars->MediaPlayback "
    ViewToolbarsMediaPlayback
    WaitSlot (1000)

    printlog " View->Toolbars->Optionbar "
    ViewToolbarsOptionbar
    WaitSlot (1000)
    printlog " View->Toolbars->Optionbar "
    ViewToolbarsOptionbar
    WaitSlot (1000)

    printlog " View->Toolbars->Picture "
    ViewToolbarsPicture
    WaitSlot (1000)
    printlog " View->Toolbars->Picture "
    ViewToolbarsPicture
    WaitSlot (1000)

    printlog " View->Toolbars->Standard "
    ViewToolbarsStandard
    WaitSlot (1000)
    printlog " View->Toolbars->Standard "
    ViewToolbarsStandard
    WaitSlot (1000)

    printlog " View->Toolbars->Standard View "
    ViewToolbarsStandardView
    WaitSlot (1000)
    printlog " View->Toolbars->Standard View "
    ViewToolbarsStandardView
    WaitSlot (1000)

    printlog " View->Toolbars->Hyperlinkbar "
    ViewToolbarsHyperlinkbar
    WaitSlot (1000)
    printlog " View->Toolbars->Hyperlinkbar "
    ViewToolbarsHyperlinkbar
    WaitSlot (1000)

    printlog " View->Toolbars->ColorBar "
    ViewToolbarsColorBar
    WaitSlot (1000)
    printlog " View->Toolbars->ColorBar "
    ViewToolbarsColorBar
    WaitSlot (1000)

    printlog " View->Toolbars->Customize "
    ViewToolbarsCustomize
    WaitSlot (1000)
    Kontext
    printlog " switch to tabpage 'Menus' "
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
        printlog " click button 'new' "
        BtnNew.Click
        sleep 1
        printlog " Dialog 'New Menu' comes up "
        Kontext "MenuOrganiser"
        Call DialogTest ( MenuOrganiser )
        printlog " Cancel dialog 'New Menu' "
        MenuOrganiser.cancel
        sleep 1
        Kontext "TabCustomizeMenu"
        TabCustomizeMenu.Close
    end if
    sleep (1)

    printlog " close application "
    Call  hCloseDocument
   
endcase 'tiViewToolbar
'-------------------------------------------------------------------------
testcase tiViewDisplayQuality

    printlog " open application "
    Call hNewDocument

    printlog " create a rectangle "
    Call hRechteckErstellen 20,20,40,40

    try
    printlog " View->Display Quality->Black and White "
        ViewQualityBlackWhite
        Printlog "- Quality set to black and white"
    catch
        Warnlog "- Slot could not be accessed"
    endcatch
    WaitSlot (1000)
    try
        printlog " View->Display Quality->Greyscale "
        ViewQualityGreyscale
        Printlog "- View quality set to greyscale"
    catch
        Warnlog "- View quality greyscale could not be accessed"
    endcatch
    WaitSlot (1000)
    try
        printlog " View->Display Quality->Colour "
        ViewQualityColour
        Printlog "- View quality set to colour"
    catch
        Warnlog "- View quality colour could not be accessed"
    endcatch
    printlog " close application "
    Call hClosedocument
    
endcase 'tiViewDisplayQuality
'-------------------------------------------------------------------------
testcase tiViewLayer

    printlog " open application "
    Call hNewDocument
    printlog " Insert->Layer / in impress: ???? "

    printlog " View->Layer "
    ViewLayer
    WaitSlot (1000)
    printlog " View->Layer "
    ViewLayer
    printlog " close application "
    Call hCloseDocument
    
endcase 'tiViewLayer
'-------------------------------------------------------------------------
testcase tViewGrid

    printlog " open application "
    Call  hNewDocument

    printlog " View->Grid-> "
    ViewGridVisible
    printlog " View->Grid-> "
    ViewGridUse
    printlog " View->Grid-> "
    ViewGridFront
    printlog " View->Grid-> "
    ViewGridVisible
    printlog " View->Grid-> "
    ViewGridUse
    printlog " View->Grid-> "
    ViewGridFront
    WaitSlot (1000)
    printlog " close application "
    Call  hCloseDocument
    
endcase 'tViewGrid
'-------------------------------------------------------------------------
testcase tViewSnapLines

    printlog " open application "
    Call  hNewDocument

    printlog " View->Snap Lines-> "
    ViewSnapLinesVisible
    printlog " View->Snap Lines-> "
    ViewSnapLinesUse
    printlog " View->Snap Lines-> "
    ViewSnapLinesFront
    printlog " View->Snap Lines-> "
    ViewSnapLinesVisible
    printlog " View->Snap Lines-> "
    ViewSnapLinesUse
    printlog " View->Snap Lines-> "
    ViewSnapLinesFront
    WaitSlot (1000)
    printlog " close application "
    Call  hCloseDocument
    
endcase 'tViewSnapLines
