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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <editeng/eeitem.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <vcl/svapp.hxx>

#include "output.hxx"
#include "drwlayer.hxx"
#include "document.hxx"
#include "tabvwsh.hxx"
#include "fillinfo.hxx"

#include <svx/fmview.hxx>

// STATIC DATA -----------------------------------------------------------

SdrObject* pSkipPaintObj = NULL;

//==================================================================

// #i72502#
Point ScOutputData::PrePrintDrawingLayer(long nLogStX, long nLogStY )
{
    Rectangle aRect;
    SCCOL nCol;
    Point aOffset;
    long nLayoutSign(bLayoutRTL ? -1 : 1);

    for (nCol=0; nCol<nX1; nCol++)
        aOffset.X() -= pDoc->GetColWidth( nCol, nTab ) * nLayoutSign;
    aOffset.Y() -= pDoc->GetRowHeight( 0, nY1-1, nTab );

    long nDataWidth = 0;
    long nDataHeight = 0;
    for (nCol=nX1; nCol<=nX2; nCol++)
        nDataWidth += pDoc->GetColWidth( nCol, nTab );
    nDataHeight += pDoc->GetRowHeight( nY1, nY2, nTab );

    if ( bLayoutRTL )
        aOffset.X() += nDataWidth;

    aRect.Left() = aRect.Right()  = -aOffset.X();
    aRect.Top()  = aRect.Bottom() = -aOffset.Y();

    Point aMMOffset( aOffset );
    aMMOffset.X() = (long)(aMMOffset.X() * HMM_PER_TWIPS);
    aMMOffset.Y() = (long)(aMMOffset.Y() * HMM_PER_TWIPS);

    if (!bMetaFile)
        aMMOffset += Point( nLogStX, nLogStY );

    for (nCol=nX1; nCol<=nX2; nCol++)
        aRect.Right() += pDoc->GetColWidth( nCol, nTab );
    aRect.Bottom() += pDoc->GetRowHeight( nY1, nY2, nTab );

    aRect.Left()   = (long) (aRect.Left()   * HMM_PER_TWIPS);
    aRect.Top()    = (long) (aRect.Top()    * HMM_PER_TWIPS);
    aRect.Right()  = (long) (aRect.Right()  * HMM_PER_TWIPS);
    aRect.Bottom() = (long) (aRect.Bottom() * HMM_PER_TWIPS);

    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = (pDrawView) ? pDrawView : pViewShell->GetSdrView();

        if(pLocalDrawView)
        {
            // #i76114# MapMode has to be set because BeginDrawLayers uses GetPaintRegion
            MapMode aOldMode = pDev->GetMapMode();
            if (!bMetaFile)
                pDev->SetMapMode( MapMode( MAP_100TH_MM, aMMOffset, aOldMode.GetScaleX(), aOldMode.GetScaleY() ) );

            // #i74769# work with SdrPaintWindow directly
            // #i76114# pass bDisableIntersect = true, because the intersection of the table area
            // with the Window's paint region can be empty
            Region aRectRegion(aRect);
            mpTargetPaintWindow = pLocalDrawView->BeginDrawLayers(pDev, aRectRegion, true);
            OSL_ENSURE(mpTargetPaintWindow, "BeginDrawLayers: Got no SdrPaintWindow (!)");

            if (!bMetaFile)
                pDev->SetMapMode( aOldMode );
        }
    }

    return aMMOffset;
}

// #i72502#
void ScOutputData::PostPrintDrawingLayer(const Point& rMMOffset) // #i74768#
{
    // #i74768# just use offset as in PrintDrawingLayer() to also get the form controls
    // painted with offset
    MapMode aOldMode = pDev->GetMapMode();

    if (!bMetaFile)
    {
        pDev->SetMapMode( MapMode( MAP_100TH_MM, rMMOffset, aOldMode.GetScaleX(), aOldMode.GetScaleY() ) );
    }

    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = (pDrawView) ? pDrawView : pViewShell->GetSdrView();

        if(pLocalDrawView)
        {
            // #i74769# work with SdrPaintWindow directly
            pLocalDrawView->EndDrawLayers(*mpTargetPaintWindow, true);
            mpTargetPaintWindow = 0;
        }
    }

    // #i74768#
    if (!bMetaFile)
    {
        pDev->SetMapMode( aOldMode );
    }
}

// #i72502#
void ScOutputData::PrintDrawingLayer(const sal_uInt16 nLayer, const Point& rMMOffset)
{
    bool bHideAllDrawingLayer(false);

    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = (pDrawView) ? pDrawView : pViewShell->GetSdrView();

        if(pLocalDrawView)
        {
            bHideAllDrawingLayer = pLocalDrawView->getHideOle() && pLocalDrawView->getHideChart()
                    && pLocalDrawView->getHideDraw() && pLocalDrawView->getHideFormControl();
        }
    }

    // #109985#
    if(bHideAllDrawingLayer || (!pDoc->GetDrawLayer()))
    {
        return;
    }

    MapMode aOldMode = pDev->GetMapMode();

    if (!bMetaFile)
    {
        pDev->SetMapMode( MapMode( MAP_100TH_MM, rMMOffset, aOldMode.GetScaleX(), aOldMode.GetScaleY() ) );
    }

    // #109985#
    DrawSelectiveObjects( nLayer );

    if (!bMetaFile)
    {
        pDev->SetMapMode( aOldMode );
    }
}

// #109985#
void ScOutputData::DrawSelectiveObjects(const sal_uInt16 nLayer)
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return;

    //  #i46362# high contrast mode (and default text direction) must be handled
    //  by the application, so it's still needed when using DrawLayer().

    SdrOutliner& rOutl = pModel->GetDrawOutliner();
    rOutl.EnableAutoColor( bUseStyleColor );
    rOutl.SetDefaultHorizontalTextDirection(
                (EEHorizontalTextDirection)pDoc->GetEditTextDirection( nTab ) );

    //  #i69767# The hyphenator must be set (used to be before drawing a text shape with hyphenation).
    //  LinguMgr::GetHyphenator (EditEngine) uses a wrapper now that creates the real hyphenator on demand,
    //  so it's not a performance problem to call UseHyphenator even when it's not needed.

    pModel->UseHyphenator();

    sal_uLong nOldDrawMode = pDev->GetDrawMode();
    if ( bUseStyleColor && Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        pDev->SetDrawMode( nOldDrawMode | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL |
                            DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );
    }

    // #109985#
    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = (pDrawView) ? pDrawView : pViewShell->GetSdrView();

        if(pLocalDrawView)
        {
            SdrPageView* pPageView = pLocalDrawView->GetSdrPageView();

            if(pPageView)
            {
                pPageView->DrawLayer(sal::static_int_cast<SdrLayerID>(nLayer), pDev);
            }
        }
    }

    pDev->SetDrawMode(nOldDrawMode);

    // #109985#
    return;
}

//  Teile nur fuer Bildschirm

// #109985#
void ScOutputData::DrawingSingle(const sal_uInt16 nLayer)
{
    sal_Bool    bHad    = sal_False;
    long    nPosY   = nScrY;
    SCSIZE  nArrY;
    for (nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];

        if ( pThisRowInfo->bChanged )
        {
            if (!bHad)
            {
                bHad = sal_True;
            }
        }
        else if (bHad)
        {
            DrawSelectiveObjects( nLayer );
            bHad = sal_False;
        }
        nPosY += pRowInfo[nArrY].nHeight;
    }

    if (bHad)
        DrawSelectiveObjects( nLayer );
}




