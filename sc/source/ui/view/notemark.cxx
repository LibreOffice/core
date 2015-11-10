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

#include <svx/svdoutl.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <sfx2/printer.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/itempool.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include "notemark.hxx"
#include "document.hxx"
#include "postit.hxx"

#define SC_NOTEMARK_TIME    800
#define SC_NOTEMARK_SHORT   70

ScNoteMarker::ScNoteMarker( vcl::Window* pWin, vcl::Window* pRight, vcl::Window* pBottom, vcl::Window* pDiagonal,
                            ScDocument* pD, ScAddress aPos, const OUString& rUser,
                            const MapMode& rMap, bool bLeftEdge, bool bForce, bool bKeyboard ) :
    pWindow( pWin ),
    pRightWin( pRight ),
    pBottomWin( pBottom ),
    pDiagWin( pDiagonal ),
    pDoc( pD ),
    aDocPos( aPos ),
    aUserText( rUser ),
    aMapMode( rMap ),
    bLeft( bLeftEdge ),
    bByKeyboard( bKeyboard ),
    pModel( nullptr ),
    pObject( nullptr ),
    bVisible( false )
{
    Size aSizePixel = pWindow->GetOutputSizePixel();
    if( pRightWin )
        aSizePixel.Width() += pRightWin->GetOutputSizePixel().Width();
    if( pBottomWin )
        aSizePixel.Height() += pBottomWin->GetOutputSizePixel().Height();
    Rectangle aVisPixel( Point( 0, 0 ), aSizePixel );
    aVisRect = pWindow->PixelToLogic( aVisPixel, aMapMode );

    aTimer.SetTimeoutHdl( LINK( this, ScNoteMarker, TimeHdl ) );
    aTimer.SetTimeout( bForce ? SC_NOTEMARK_SHORT : SC_NOTEMARK_TIME );
    aTimer.Start();
}

ScNoteMarker::~ScNoteMarker()
{
    InvalidateWin();

    delete pModel;
}

IMPL_LINK_NOARG_TYPED(ScNoteMarker, TimeHdl, Timer *, void)
{
    if (!bVisible)
    {
        SvtPathOptions aPathOpt;
        OUString aPath = aPathOpt.GetPalettePath();
        pModel = new SdrModel(aPath, nullptr, nullptr, false, false);
        pModel->SetScaleUnit(MAP_100TH_MM);
        SfxItemPool& rPool = pModel->GetItemPool();
        rPool.SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
        rPool.FreezeIdRanges();

        OutputDevice* pPrinter = pDoc->GetRefDevice();
        if (pPrinter)
        {
            // On the outliner of the draw model also the printer is set as RefDevice,
            // and it should look uniform.
            Outliner& rOutliner = pModel->GetDrawOutliner();
            rOutliner.SetRefDevice(pPrinter);
        }

        if( SdrPage* pPage = pModel->AllocPage( false ) )
        {
            pObject = ScNoteUtil::CreateTempCaption( *pDoc, aDocPos, *pPage, aUserText, aVisRect, bLeft );
            if( pObject )
            {
                pObject->SetGridOffset( aGridOff );
                aRect = pObject->GetCurrentBoundRect();
            }

            // Insert page so that the model recognise it and also deleted
            pModel->InsertPage( pPage );

        }
        bVisible = true;
    }

    Draw();
}

static void lcl_DrawWin( SdrObject* pObject, vcl::RenderContext* pWindow, const MapMode& rMap )
{
    MapMode aOld = pWindow->GetMapMode();
    pWindow->SetMapMode( rMap );

    DrawModeFlags nOldDrawMode = pWindow->GetDrawMode();
    if ( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        pWindow->SetDrawMode( nOldDrawMode | DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill |
                            DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient );
    }

    pObject->SingleObjectPainter( *pWindow ); // #110094#-17

    pWindow->SetDrawMode( nOldDrawMode );
    pWindow->SetMapMode( aOld );
}

static MapMode lcl_MoveMapMode( const MapMode& rMap, const Size& rMove )
{
    MapMode aNew = rMap;
    Point aOrigin = aNew.GetOrigin();
    aOrigin.X() -= rMove.Width();
    aOrigin.Y() -= rMove.Height();
    aNew.SetOrigin(aOrigin);
    return aNew;
}

void ScNoteMarker::Draw()
{
    if ( pObject && bVisible )
    {
        lcl_DrawWin( pObject, pWindow, aMapMode );

        if ( pRightWin || pBottomWin )
        {
            Size aWinSize = pWindow->PixelToLogic( pWindow->GetOutputSizePixel(), aMapMode );
            if ( pRightWin )
                lcl_DrawWin( pObject, pRightWin,
                                lcl_MoveMapMode( aMapMode, Size( aWinSize.Width(), 0 ) ) );
            if ( pBottomWin )
                lcl_DrawWin( pObject, pBottomWin,
                                lcl_MoveMapMode( aMapMode, Size( 0, aWinSize.Height() ) ) );
            if ( pDiagWin )
                lcl_DrawWin( pObject, pDiagWin, lcl_MoveMapMode( aMapMode, aWinSize ) );
        }
    }
}

void ScNoteMarker::InvalidateWin()
{
    if (bVisible)
    {
        pWindow->Invalidate( OutputDevice::LogicToLogic(aRect, aMapMode, pWindow->GetMapMode()) );

        if ( pRightWin || pBottomWin )
        {
            Size aWinSize = pWindow->PixelToLogic( pWindow->GetOutputSizePixel(), aMapMode );
            if ( pRightWin )
                pRightWin->Invalidate( OutputDevice::LogicToLogic(aRect,
                                        lcl_MoveMapMode( aMapMode, Size( aWinSize.Width(), 0 ) ),
                                        pRightWin->GetMapMode()) );
            if ( pBottomWin )
                pBottomWin->Invalidate( OutputDevice::LogicToLogic(aRect,
                                        lcl_MoveMapMode( aMapMode, Size( 0, aWinSize.Height() ) ),
                                        pBottomWin->GetMapMode()) );
            if ( pDiagWin )
                pDiagWin->Invalidate( OutputDevice::LogicToLogic(aRect,
                                        lcl_MoveMapMode( aMapMode, aWinSize ),
                                        pDiagWin->GetMapMode()) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
