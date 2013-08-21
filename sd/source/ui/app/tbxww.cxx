/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sfx2/bindings.hxx>
#include <svx/svxids.hrc>
#include <svx/grafctrl.hxx>
#include <svl/cjkoptions.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/imagemgr.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include "sddll.hxx"
#include "GraphicDocShell.hxx"

#include <vcl/toolbox.hxx>

#include "sdmod.hxx"
#include "app.hrc"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "tbx_ww.hxx"

SFX_IMPL_TOOLBOX_CONTROL( SdTbxControl, TbxImageItem )

/**
 * Class for toolbox
 */
SdTbxControl::SdTbxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
        SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}

/*-------------------------------------------------------------------------*/

SfxPopupWindowType SdTbxControl::GetPopupWindowType() const
{
    return( SFX_POPUPWINDOW_ONTIMEOUT );
}

/**
 * We create the window here
 * You can get the position of the toolbox with GetToolBox()
 * rItemRect are screen coordinates
 */

SfxPopupWindow* SdTbxControl::CreatePopupWindow()
{
    SfxPopupWindow *pWin = NULL;
    OUString aToolBarResStr;
    OUStringBuffer aTbxResName( "private:resource/toolbar/" );
    switch( GetSlotId() )
    {
        case SID_OBJECT_ALIGN:
            aTbxResName.appendAscii( "alignmentbar" );
            aToolBarResStr = aTbxResName.makeStringAndClear();
        break;
        case SID_ZOOM_TOOLBOX:
            aTbxResName.appendAscii( "zoombar" );
            aToolBarResStr = aTbxResName.makeStringAndClear();
        break;
        case SID_OBJECT_CHOOSE_MODE:
            aTbxResName.appendAscii( "choosemodebar" );
            aToolBarResStr = aTbxResName.makeStringAndClear();
        break;
        case SID_POSITION:
            aTbxResName.appendAscii( "positionbar" );
            aToolBarResStr = aTbxResName.makeStringAndClear();
        break;
        case SID_DRAWTBX_TEXT:
            aTbxResName.appendAscii( "textbar" );
            aToolBarResStr = aTbxResName.makeStringAndClear();
        break;
        case SID_DRAWTBX_RECTANGLES:
            aTbxResName.appendAscii( "rectanglesbar" );
            aToolBarResStr = aTbxResName.makeStringAndClear();
        break;
        case SID_DRAWTBX_ELLIPSES:
            aTbxResName.appendAscii( "ellipsesbar" );
            aToolBarResStr = aTbxResName.makeStringAndClear();
        break;
        case SID_DRAWTBX_LINES:
            aTbxResName.appendAscii( "linesbar" );
            aToolBarResStr = aTbxResName.makeStringAndClear();
        break;
        case SID_DRAWTBX_ARROWS:
            aTbxResName.appendAscii( "arrowsbar" );
            aToolBarResStr = aTbxResName.makeStringAndClear();
        break;
        case SID_DRAWTBX_3D_OBJECTS:
            aTbxResName.appendAscii( "3dobjectsbar" );
            aToolBarResStr = aTbxResName.makeStringAndClear();
        break;
        case SID_DRAWTBX_CONNECTORS:
            aTbxResName.appendAscii( "connectorsbar" );
            aToolBarResStr = aTbxResName.makeStringAndClear();
        break;
        case SID_DRAWTBX_INSERT:
            aTbxResName.appendAscii( "insertbar" );
            aToolBarResStr = aTbxResName.makeStringAndClear();
        break;
    }

    if ( !aToolBarResStr.isEmpty() )
        createAndPositionSubToolBar( aToolBarResStr );

    return( pWin );
}

/*-------------------------------------------------------------------------*/

void SdTbxControl::StateChanged( sal_uInt16 nSId,
                        SfxItemState eState, const SfxPoolItem* pState )
{
    SfxToolBoxControl::StateChanged( nSId, eState, pState );

    if( eState == SFX_ITEM_AVAILABLE )
    {
        TbxImageItem* pItem = PTR_CAST( TbxImageItem, pState );
        // StarDesktop can also receive another item,
        // but it is not allowed to evaluate it
        if( pItem )
        {
            ToolBox& rTbx = GetToolBox();
            sal_uInt16 nImage = pItem->GetValue();
            if( nImage == 0 )
            {
                if( rTbx.IsItemChecked( nSId ) )
                    rTbx.CheckItem( nSId, sal_False );
            }
            else
            {
                OUString aSlotURL( "slot:" );
                aSlotURL += OUString::number( nImage);
                Image aImage = GetImage( m_xFrame,
                                         aSlotURL,
                                         hasBigImages()
                                       );

                // !-operator checks if image is not present
                if( !!aImage )
                {
                    rTbx.SetItemImage( GetId(), aImage );
                    rTbx.CheckItem( GetId(), IsCheckable( nImage ) );

                    if( nSId != SID_ZOOM_TOOLBOX &&
                        nSId != SID_DRAWTBX_INSERT &&
                        nSId != SID_POSITION &&
                        nSId != SID_OBJECT_ALIGN )
                    {
                        if( nSId != SID_OBJECT_CHOOSE_MODE &&
                            rTbx.IsItemChecked( SID_OBJECT_CHOOSE_MODE ) )
                            rTbx.CheckItem( SID_OBJECT_CHOOSE_MODE, sal_False );
                        if( nSId != SID_DRAWTBX_TEXT &&
                            rTbx.IsItemChecked( SID_DRAWTBX_TEXT ) )
                             rTbx.CheckItem( SID_DRAWTBX_TEXT, sal_False );
                        if( nSId != SID_DRAWTBX_RECTANGLES &&
                            rTbx.IsItemChecked( SID_DRAWTBX_RECTANGLES ) )
                               rTbx.CheckItem( SID_DRAWTBX_RECTANGLES, sal_False );
                        if( nSId != SID_DRAWTBX_ELLIPSES &&
                            rTbx.IsItemChecked( SID_DRAWTBX_ELLIPSES ) )
                               rTbx.CheckItem( SID_DRAWTBX_ELLIPSES, sal_False );
                        if( nSId != SID_DRAWTBX_LINES &&
                            rTbx.IsItemChecked( SID_DRAWTBX_LINES ) )
                            rTbx.CheckItem( SID_DRAWTBX_LINES, sal_False );
                        if( nSId != SID_DRAWTBX_ARROWS &&
                            rTbx.IsItemChecked( SID_DRAWTBX_ARROWS ) )
                            rTbx.CheckItem( SID_DRAWTBX_ARROWS, sal_False );
                        if( nSId != SID_DRAWTBX_3D_OBJECTS &&
                            rTbx.IsItemChecked( SID_DRAWTBX_3D_OBJECTS ) )
                            rTbx.CheckItem( SID_DRAWTBX_3D_OBJECTS, sal_False );
                        if( nSId != SID_DRAWTBX_CONNECTORS &&
                            rTbx.IsItemChecked( SID_DRAWTBX_CONNECTORS ) )
                            rTbx.CheckItem( SID_DRAWTBX_CONNECTORS, sal_False );
                    }
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

sal_Bool SdTbxControl::IsCheckable( sal_uInt16 nSId )
{
    switch( nSId )
    {
        case SID_OBJECT_ROTATE:
        case SID_OBJECT_MIRROR:
        case SID_OBJECT_CROP:
        case SID_OBJECT_TRANSPARENCE:
        case SID_OBJECT_GRADIENT:
        case SID_OBJECT_SHEAR:
        case SID_OBJECT_CROOK_ROTATE:
        case SID_OBJECT_CROOK_SLANT:
        case SID_OBJECT_CROOK_STRETCH:
        case SID_CONVERT_TO_3D_LATHE:

        case SID_ATTR_CHAR:
        case SID_ATTR_CHAR_VERTICAL:
        case SID_TEXT_FITTOSIZE:
        case SID_TEXT_FITTOSIZE_VERTICAL:
        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
        case SID_DRAW_FONTWORK:
        case SID_DRAW_FONTWORK_VERTICAL:

        case SID_DRAW_RECT:
        case SID_DRAW_SQUARE:
        case SID_DRAW_RECT_ROUND:
        case SID_DRAW_SQUARE_ROUND:
        case SID_DRAW_RECT_NOFILL:
        case SID_DRAW_SQUARE_NOFILL:
        case SID_DRAW_RECT_ROUND_NOFILL:
        case SID_DRAW_SQUARE_ROUND_NOFILL:

        case SID_DRAW_ELLIPSE:
        case SID_DRAW_CIRCLE:
        case SID_DRAW_PIE:
        case SID_DRAW_CIRCLEPIE:
        case SID_DRAW_ELLIPSECUT:
        case SID_DRAW_CIRCLECUT:
        case SID_DRAW_ARC:
        case SID_DRAW_CIRCLEARC:
        case SID_DRAW_ELLIPSE_NOFILL:
        case SID_DRAW_CIRCLE_NOFILL:
        case SID_DRAW_PIE_NOFILL:
        case SID_DRAW_CIRCLEPIE_NOFILL:
        case SID_DRAW_ELLIPSECUT_NOFILL:
        case SID_DRAW_CIRCLECUT_NOFILL:

        case SID_DRAW_BEZIER_NOFILL:
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_XPOLYGON_NOFILL:
        case SID_DRAW_BEZIER_FILL:
        case SID_DRAW_POLYGON:
        case SID_DRAW_XPOLYGON:
        case SID_DRAW_FREELINE:
        case SID_DRAW_FREELINE_NOFILL:

        case SID_DRAW_LINE:
        case SID_DRAW_XLINE:
        case SID_DRAW_MEASURELINE:
        case SID_LINE_ARROW_START:
        case SID_LINE_ARROW_END:
        case SID_LINE_ARROWS:
        case SID_LINE_ARROW_CIRCLE:
        case SID_LINE_CIRCLE_ARROW:
        case SID_LINE_ARROW_SQUARE:
        case SID_LINE_SQUARE_ARROW:

        case SID_3D_CUBE:
        case SID_3D_SPHERE:
        case SID_3D_CYLINDER:
        case SID_3D_CONE:
        case SID_3D_PYRAMID:
        case SID_3D_TORUS:
        case SID_3D_SHELL:
        case SID_3D_HALF_SPHERE:

        case SID_TOOL_CONNECTOR:
        case SID_CONNECTOR_ARROW_START:
        case SID_CONNECTOR_ARROW_END:
        case SID_CONNECTOR_ARROWS:
        case SID_CONNECTOR_CIRCLE_START:
        case SID_CONNECTOR_CIRCLE_END:
        case SID_CONNECTOR_CIRCLES:
        case SID_CONNECTOR_LINE:
        case SID_CONNECTOR_LINE_ARROW_START:
        case SID_CONNECTOR_LINE_ARROW_END:
        case SID_CONNECTOR_LINE_ARROWS:
        case SID_CONNECTOR_LINE_CIRCLE_START:
        case SID_CONNECTOR_LINE_CIRCLE_END:
        case SID_CONNECTOR_LINE_CIRCLES:
        case SID_CONNECTOR_CURVE:
        case SID_CONNECTOR_CURVE_ARROW_START:
        case SID_CONNECTOR_CURVE_ARROW_END:
        case SID_CONNECTOR_CURVE_ARROWS:
        case SID_CONNECTOR_CURVE_CIRCLE_START:
        case SID_CONNECTOR_CURVE_CIRCLE_END:
        case SID_CONNECTOR_CURVE_CIRCLES:
        case SID_CONNECTOR_LINES:
        case SID_CONNECTOR_LINES_ARROW_START:
        case SID_CONNECTOR_LINES_ARROW_END:
        case SID_CONNECTOR_LINES_ARROWS:
        case SID_CONNECTOR_LINES_CIRCLE_START:
        case SID_CONNECTOR_LINES_CIRCLE_END:
        case SID_CONNECTOR_LINES_CIRCLES:

            return( sal_True );
    }
    return( sal_False );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
