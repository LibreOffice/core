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
#include <svl/itempool.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <notemark.hxx>
#include <document.hxx>
#include <postit.hxx>
#include <drawview.hxx>

#define SC_NOTEMARK_TIME    800
#define SC_NOTEMARK_SHORT   70

ScNoteMarker::ScNoteMarker( vcl::Window* pWin, vcl::Window* pRight, vcl::Window* pBottom, vcl::Window* pDiagonal,
                            ScDocument* pD, const ScAddress& aPos, const OUString& rUser,
                            const MapMode& rMap, bool bLeftEdge, bool bForce, bool bKeyboard) :
    m_pWindow( pWin ),
    m_pRightWin( pRight ),
    m_pBottomWin( pBottom ),
    m_pDiagWin( pDiagonal ),
    m_pDoc( pD ),
    m_aDocPos( aPos ),
    m_aUserText( rUser ),
    m_aMapMode( rMap ),
    m_bLeft( bLeftEdge ),
    m_bByKeyboard( bKeyboard ),
    m_bVisible( false )
{
    Size aSizePixel = m_pWindow->GetOutputSizePixel();
    if( m_pRightWin )
        aSizePixel.AdjustWidth(m_pRightWin->GetOutputSizePixel().Width() );
    if( m_pBottomWin )
        aSizePixel.AdjustHeight(m_pBottomWin->GetOutputSizePixel().Height() );
    tools::Rectangle aVisPixel( Point( 0, 0 ), aSizePixel );
    m_aVisRect = m_pWindow->PixelToLogic( aVisPixel, m_aMapMode );

    m_aTimer.SetInvokeHandler( LINK( this, ScNoteMarker, TimeHdl ) );
    m_aTimer.SetTimeout( bForce ? SC_NOTEMARK_SHORT : SC_NOTEMARK_TIME );
    m_aTimer.Start();
}

ScNoteMarker::~ScNoteMarker()
{
    if (m_pModel)
        m_xObject.release();     // deleting pModel also deletes the SdrCaptionObj

    InvalidateWin();

    m_pModel.reset();
}

IMPL_LINK_NOARG(ScNoteMarker, TimeHdl, Timer *, void)
{
    if (!m_bVisible)
    {
        m_pModel.reset( new SdrModel() );
        m_pModel->SetScaleUnit(MapUnit::Map100thMM);
        SfxItemPool& rPool = m_pModel->GetItemPool();
        rPool.SetDefaultMetric(MapUnit::Map100thMM);
        rPool.FreezeIdRanges();

        OutputDevice* pPrinter = m_pDoc->GetRefDevice();
        if (pPrinter)
        {
            // On the outliner of the draw model also the printer is set as RefDevice,
            // and it should look uniform.
            Outliner& rOutliner = m_pModel->GetDrawOutliner();
            rOutliner.SetRefDevice(pPrinter);
        }

        if( SdrPage* pPage = m_pModel->AllocPage( false ) )

        {
            m_xObject = ScNoteUtil::CreateTempCaption( *m_pDoc, m_aDocPos, *pPage, m_aUserText, m_aVisRect, m_bLeft );
            if( m_xObject )
            {
                // Here, SyncForGrid and GetGridOffset was used with the comment:
                // // Need to include grid offset: GetCurrentBoundRect is removing it
                // // but we need to know actual rect position
                // This is no longer true - SdrObject::RecalcBoundRect() uses the
                // GetViewContact().getViewIndependentPrimitive2DContainer()) call
                // that now by default adds the eventually needed GridOffset. Thus
                // I have removed that adaption stuff.
                m_aRect = m_xObject->GetCurrentBoundRect();
            }

            // Insert page so that the model recognise it and also deleted
            m_pModel->InsertPage( pPage );

        }
        m_bVisible = true;
    }

    Draw();
}

static void lcl_DrawWin( const SdrObject* pObject, vcl::RenderContext* pWindow, const MapMode& rMap )
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
    aOrigin.AdjustX( -(rMove.Width()) );
    aOrigin.AdjustY( -rMove.Height() );
    aNew.SetOrigin(aOrigin);
    return aNew;
}

void ScNoteMarker::Draw()
{
    if ( m_xObject && m_bVisible )
    {
        lcl_DrawWin( m_xObject.get(), m_pWindow, m_aMapMode );

        if ( m_pRightWin || m_pBottomWin )
        {
            Size aWinSize = m_pWindow->PixelToLogic( m_pWindow->GetOutputSizePixel(), m_aMapMode );
            if ( m_pRightWin )
                lcl_DrawWin( m_xObject.get(), m_pRightWin,
                                lcl_MoveMapMode( m_aMapMode, Size( aWinSize.Width(), 0 ) ) );
            if ( m_pBottomWin )
                lcl_DrawWin( m_xObject.get(), m_pBottomWin,
                                lcl_MoveMapMode( m_aMapMode, Size( 0, aWinSize.Height() ) ) );
            if ( m_pDiagWin )
                lcl_DrawWin( m_xObject.get(), m_pDiagWin, lcl_MoveMapMode( m_aMapMode, aWinSize ) );
        }
    }
}

void ScNoteMarker::InvalidateWin()
{
    if (m_bVisible)
    {
        m_pWindow->Invalidate( OutputDevice::LogicToLogic(m_aRect, m_aMapMode, m_pWindow->GetMapMode()) );

        if ( m_pRightWin || m_pBottomWin )
        {
            Size aWinSize = m_pWindow->PixelToLogic( m_pWindow->GetOutputSizePixel(), m_aMapMode );
            if ( m_pRightWin )
                m_pRightWin->Invalidate( OutputDevice::LogicToLogic(m_aRect,
                                        lcl_MoveMapMode( m_aMapMode, Size( aWinSize.Width(), 0 ) ),
                                        m_pRightWin->GetMapMode()) );
            if ( m_pBottomWin )
                m_pBottomWin->Invalidate( OutputDevice::LogicToLogic(m_aRect,
                                        lcl_MoveMapMode( m_aMapMode, Size( 0, aWinSize.Height() ) ),
                                        m_pBottomWin->GetMapMode()) );
            if ( m_pDiagWin )
                m_pDiagWin->Invalidate( OutputDevice::LogicToLogic(m_aRect,
                                        lcl_MoveMapMode( m_aMapMode, aWinSize ),
                                        m_pDiagWin->GetMapMode()) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
