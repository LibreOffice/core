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
namespace sd {

/**
 * Returns the ID of the group button, if it should be toggled.
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

        case SID_BEFORE_OBJ:
        case SID_BEHIND_OBJ:
        {
            nMappedSId = SID_POSITION;
        }
        break;

        case SID_ZOOM_PANNING:
        case SID_ZOOM_MODE:
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
    return nMappedSId;
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
            pIMap = const_cast<ImageMap*>(&pIMapInfo->GetImageMap());
            pTargetList = new TargetList;
            GetViewFrame()->GetTargetList( *pTargetList );
        }

        SvxIMapDlgChildWindow::UpdateIMapDlg( aGraphic, pIMap, pTargetList, pObj );

        // We can delete the target list
        if ( pTargetList )
        {
            delete pTargetList;
        }
    }
}

IMPL_LINK( DrawViewShell, NameObjectHdl, AbstractSvxNameDialog*, pDialog )
{
    OUString aName;

    if( pDialog )
        pDialog->GetName( aName );

    return ( aName.isEmpty() || ( GetDoc() && !GetDoc()->GetObj( aName ) ) ) ? 1 : 0;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
