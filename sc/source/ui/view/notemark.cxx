/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <svx/svdoutl.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <sfx2/printer.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/itempool.hxx>
#include <vcl/svapp.hxx>

#include "notemark.hxx"
#include "document.hxx"
#include "postit.hxx"

#define SC_NOTEMARK_TIME    800
#define SC_NOTEMARK_SHORT   70

// -----------------------------------------------------------------------

ScNoteMarker::ScNoteMarker( Window* pWin, Window* pRight, Window* pBottom, Window* pDiagonal,
                            ScDocument* pD, ScAddress aPos, const String& rUser,
                            const MapMode& rMap, sal_Bool bLeftEdge, sal_Bool bForce, sal_Bool bKeyboard ) :
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
    pModel( NULL ),
    pObject( NULL ),
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

IMPL_LINK_NOARG(ScNoteMarker, TimeHdl)
{
    if (!bVisible)
    {
        SvtPathOptions aPathOpt;
        String aPath = aPathOpt.GetPalettePath();
        pModel = new SdrModel(aPath);
        pModel->SetScaleUnit(MAP_100TH_MM);
        SfxItemPool& rPool = pModel->GetItemPool();
        rPool.SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
        rPool.FreezeIdRanges();

        OutputDevice* pPrinter = pDoc->GetRefDevice();
        if (pPrinter)
        {
            //  Am Outliner des Draw-Model ist auch der Drucker als RefDevice gesetzt,
            //  und es soll einheitlich aussehen.
            Outliner& rOutliner = pModel->GetDrawOutliner();
            rOutliner.SetRefDevice(pPrinter);
        }

        if( SdrPage* pPage = pModel->AllocPage( false ) )
        {
            pObject = ScNoteUtil::CreateTempCaption( *pDoc, aDocPos, *pPage, aUserText, aVisRect, bLeft );
            if( pObject )
                aRect = pObject->GetCurrentBoundRect();

            // Page einfuegen damit das Model sie kennt und auch deleted
            pModel->InsertPage( pPage );

        }
        bVisible = true;
    }

    Draw();
    return 0;
}

static void lcl_DrawWin( SdrObject* pObject, Window* pWindow, const MapMode& rMap )
{
    MapMode aOld = pWindow->GetMapMode();
    pWindow->SetMapMode( rMap );

    sal_uLong nOldDrawMode = pWindow->GetDrawMode();
    if ( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        pWindow->SetDrawMode( nOldDrawMode | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL |
                            DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );
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
        pWindow->Invalidate( pWindow->LogicToLogic(aRect, aMapMode, pWindow->GetMapMode()) );

        if ( pRightWin || pBottomWin )
        {
            Size aWinSize = pWindow->PixelToLogic( pWindow->GetOutputSizePixel(), aMapMode );
            if ( pRightWin )
                pRightWin->Invalidate( pRightWin->LogicToLogic(aRect,
                                        lcl_MoveMapMode( aMapMode, Size( aWinSize.Width(), 0 ) ),
                                        pRightWin->GetMapMode()) );
            if ( pBottomWin )
                pBottomWin->Invalidate( pBottomWin->LogicToLogic(aRect,
                                        lcl_MoveMapMode( aMapMode, Size( 0, aWinSize.Height() ) ),
                                        pBottomWin->GetMapMode()) );
            if ( pDiagWin )
                pDiagWin->Invalidate( pDiagWin->LogicToLogic(aRect,
                                        lcl_MoveMapMode( aMapMode, aWinSize ),
                                        pDiagWin->GetMapMode()) );
        }
    }
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
