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

#include "DrawViewShell.hxx"
#include "ViewShellImplementation.hxx"
#include <vcl/waitobj.hxx>

#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/imapdlg.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <svx/svxdlg.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/style.hxx>
#include <svx/svdpagv.hxx>
#include <svx/grafctrl.hxx>
#include "stlsheet.hxx"

#include <sfx2/viewfrm.hxx>

#include "app.hrc"
#include "strings.hrc"
#include "helpids.h"
#include "Window.hxx"
#include "imapinfo.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "drawview.hxx"
#include "sdabstdlg.hxx"
#include "brkdlg.hrc"
namespace sd {

/**
 * Returns the global/main ID, i.e. the ID, which is used to execute the toolbox
 */
sal_uInt16 DrawViewShell::GetIdBySubId( sal_uInt16 nSId )
{
    sal_uInt16 nMappedSId = 0;
    switch( nSId )
    {
        case SID_OBJECT_ROTATE:
        case SID_OBJECT_MIRROR:
        case SID_OBJECT_TRANSPARENCE:
        case SID_OBJECT_GRADIENT:
        case SID_OBJECT_SHEAR:
        case SID_OBJECT_CROOK_ROTATE:
        case SID_OBJECT_CROOK_SLANT:
        case SID_OBJECT_CROOK_STRETCH:
        case SID_CONVERT_TO_3D_LATHE:
        {
            nMappedSId = SID_OBJECT_CHOOSE_MODE;
        }
        break;

        case SID_OBJECT_ALIGN_LEFT:
        case SID_OBJECT_ALIGN_CENTER:
        case SID_OBJECT_ALIGN_RIGHT:
        case SID_OBJECT_ALIGN_UP:
        case SID_OBJECT_ALIGN_MIDDLE:
        case SID_OBJECT_ALIGN_DOWN:
        {
            nMappedSId = SID_OBJECT_ALIGN;
        }
        break;

        case SID_FRAME_TO_TOP:
        case SID_MOREFRONT:
        case SID_MOREBACK:
        case SID_FRAME_TO_BOTTOM:
        case SID_BEFORE_OBJ:
        case SID_BEHIND_OBJ:
        case SID_REVERSE_ORDER:
        {
            nMappedSId = SID_POSITION;
        }
        break;

        case SID_ZOOM_OUT:
        case SID_ZOOM_IN:
        case SID_SIZE_REAL:
        case SID_ZOOM_PANNING:
        case SID_SIZE_PAGE:
        case SID_SIZE_PAGE_WIDTH:
        case SID_SIZE_ALL:
        case SID_SIZE_OPTIMAL:
        case SID_ZOOM_NEXT:
        case SID_ZOOM_PREV:
        {
            nMappedSId = SID_ZOOM_TOOLBOX;
        }
        break;

        case SID_ATTR_CHAR:
        case SID_TEXT_FITTOSIZE:
        case SID_DRAW_CAPTION:
        case SID_DRAW_FONTWORK:
        case SID_DRAW_FONTWORK_VERTICAL:
        {
            nMappedSId = SID_DRAWTBX_TEXT;
        }
        break;

        case SID_DRAW_RECT:
        case SID_DRAW_SQUARE:
        case SID_DRAW_RECT_ROUND:
        case SID_DRAW_SQUARE_ROUND:
        case SID_DRAW_RECT_NOFILL:
        case SID_DRAW_SQUARE_NOFILL:
        case SID_DRAW_RECT_ROUND_NOFILL:
        case SID_DRAW_SQUARE_ROUND_NOFILL:
        {
            nMappedSId = SID_DRAWTBX_RECTANGLES;
        }
        break;

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
        {
            nMappedSId = SID_DRAWTBX_ELLIPSES;
        }
        break;

        case SID_DRAW_BEZIER_NOFILL:
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_XPOLYGON_NOFILL:
        case SID_DRAW_FREELINE_NOFILL:
        case SID_DRAW_BEZIER_FILL:
        case SID_DRAW_POLYGON:
        case SID_DRAW_XPOLYGON:
        case SID_DRAW_FREELINE:
        {
            nMappedSId = SID_DRAWTBX_LINES;
        }
        break;

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
        {
            nMappedSId = SID_DRAWTBX_ARROWS;
        }
        break;

        case SID_3D_CUBE:
        case SID_3D_TORUS:
        case SID_3D_SPHERE:
        case SID_3D_SHELL:
        case SID_3D_HALF_SPHERE:
        case SID_3D_CYLINDER:
        case SID_3D_CONE:
        case SID_3D_PYRAMID:
        {
            nMappedSId = SID_DRAWTBX_3D_OBJECTS;
        }
        break;

        case SID_INSERT_DIAGRAM:
        case SID_ATTR_TABLE:
        case SID_INSERTFILE:
        case SID_INSERT_GRAPHIC:
        case SID_INSERT_AVMEDIA:
        case SID_INSERTPAGE:
        case SID_INSERT_MATH:
        case SID_INSERT_FLOATINGFRAME:
        case SID_INSERT_OBJECT:
        case SID_INSERT_PLUGIN:
        case SID_INSERT_SOUND:
        case SID_INSERT_VIDEO:
        case SID_INSERT_TABLE:
        {
            nMappedSId = SID_DRAWTBX_INSERT;
        }
        break;

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
        {
            nMappedSId = SID_DRAWTBX_CONNECTORS;
        }
    }
    return( nMappedSId );
}

/**
 * Fills the SlotArray in order to get the current mapping of the ToolboxSlots
 */
void DrawViewShell::MapSlot( sal_uInt16 nSId )
{
    sal_uInt16 nMappedSId = GetIdBySubId( nSId );

    if( nMappedSId > 0 )
    {
        sal_uInt16 nID = GetArrayId( nMappedSId ) + 1;
        mpSlotArray[ nID ] = nSId;
    }
}

/**
 * Allows a ImageMapping via SlotArray
 */
void DrawViewShell::UpdateToolboxImages( SfxItemSet &rSet, sal_Bool bPermanent )
{
    if( !bPermanent )
    {
        sal_uInt16 nId = GetArrayId( SID_ZOOM_TOOLBOX ) + 1;
        rSet.Put( TbxImageItem( SID_ZOOM_TOOLBOX, mpSlotArray[nId] ) );

        nId = GetArrayId( SID_DRAWTBX_INSERT ) + 1;
        rSet.Put( TbxImageItem( SID_DRAWTBX_INSERT, mpSlotArray[nId] ) );

        nId = GetArrayId( SID_POSITION ) + 1;
        rSet.Put( TbxImageItem( SID_POSITION, mpSlotArray[nId] ) );

        nId = GetArrayId( SID_OBJECT_ALIGN ) + 1;
        rSet.Put( TbxImageItem( SID_OBJECT_ALIGN, mpSlotArray[nId] ) );
    }
    else
    {
        for( sal_uInt16 nId = 0; nId < SLOTARRAY_COUNT; nId += 2 )
        {
            rSet.Put( TbxImageItem( mpSlotArray[nId], mpSlotArray[nId+1] ) );
        }
    }
}


sal_uInt16 DrawViewShell::GetMappedSlot( sal_uInt16 nSId )
{
    sal_uInt16 nSlot = 0;
    sal_uInt16 nId = GetArrayId( nSId );
    if( nId != USHRT_MAX )
        nSlot = mpSlotArray[ nId+1 ];

    /* If the slot is mapped to itself, we have to return 0. Otherwise the slot
       would be executed over and over again. The slot is initial available in
       the array in order to show the image correct.  */
    if( nSId == nSlot )
        return( 0 );

    return( nSlot );
}

/**
 * @returns number of the main slot in the slot array
 */
sal_uInt16 DrawViewShell::GetArrayId( sal_uInt16 nSId )
{
    for( sal_uInt16 i = 0; i < SLOTARRAY_COUNT; i += 2 )
    {
        if( mpSlotArray[ i ] == nSId )
            return( i );
    }
    OSL_FAIL( "Slot in array not found!" );
    return( USHRT_MAX );
}



void DrawViewShell::UpdateIMapDlg( SdrObject* pObj )
{
    if( ( pObj->ISA( SdrGrafObj ) || pObj->ISA( SdrOle2Obj ) ) && !mpDrawView->IsTextEdit() &&
         GetViewFrame()->HasChildWindow( SvxIMapDlgChildWindow::GetChildWindowId() ) )
    {
        Graphic     aGraphic;
        ImageMap*   pIMap = NULL;
        TargetList* pTargetList = NULL;
        SdIMapInfo* pIMapInfo = GetDoc()->GetIMapInfo( pObj );

        // get graphic from shape
        SdrGrafObj* pGrafObj = dynamic_cast< SdrGrafObj* >( pObj );
        if( pGrafObj )
            aGraphic = pGrafObj->GetGraphic();

        if ( pIMapInfo )
        {
            pIMap = (ImageMap*) &pIMapInfo->GetImageMap();
            pTargetList = new TargetList;
            GetViewFrame()->GetTargetList( *pTargetList );
        }

        SvxIMapDlgChildWindow::UpdateIMapDlg( aGraphic, pIMap, pTargetList, pObj );

        // We can delete the target list
        if ( pTargetList )
        {
            for ( size_t i = 0, n = pTargetList->size(); i < n; ++i )
                delete pTargetList->at( i );
            delete pTargetList;
        }
    }
}

// -----------------------------------------------------------------------------

IMPL_LINK( DrawViewShell, NameObjectHdl, AbstractSvxNameDialog*, pDialog )
{
    OUString aName;

    if( pDialog )
        pDialog->GetName( aName );

    return ( aName.isEmpty() || ( GetDoc() && !GetDoc()->GetObj( aName ) ) ) ? 1 : 0;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
