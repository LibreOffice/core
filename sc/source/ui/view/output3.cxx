/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <editeng/eeitem.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include "output.hxx"
#include "drwlayer.hxx"
#include "document.hxx"
#include "tabvwsh.hxx"
#include "fillinfo.hxx"

#include <svx/fmview.hxx>




Point ScOutputData::PrePrintDrawingLayer(long nLogStX, long nLogStY )
{
    Rectangle aRect;
    SCCOL nCol;
    Point aOffset;
    long nLayoutSign(bLayoutRTL ? -1 : 1);

    for (nCol=0; nCol<nX1; nCol++)
        aOffset.X() -= mpDoc->GetColWidth( nCol, nTab ) * nLayoutSign;
    aOffset.Y() -= mpDoc->GetRowHeight( 0, nY1-1, nTab );

    long nDataWidth = 0;
    for (nCol=nX1; nCol<=nX2; nCol++)
        nDataWidth += mpDoc->GetColWidth( nCol, nTab );

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
        aRect.Right() += mpDoc->GetColWidth( nCol, nTab );
    aRect.Bottom() += mpDoc->GetRowHeight( nY1, nY2, nTab );

    aRect.Left()   = (long) (aRect.Left()   * HMM_PER_TWIPS);
    aRect.Top()    = (long) (aRect.Top()    * HMM_PER_TWIPS);
    aRect.Right()  = (long) (aRect.Right()  * HMM_PER_TWIPS);
    aRect.Bottom() = (long) (aRect.Bottom() * HMM_PER_TWIPS);

    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = (pDrawView) ? pDrawView : pViewShell->GetSdrView();

        if(pLocalDrawView)
        {
            
            MapMode aOldMode = mpDev->GetMapMode();
            if (!bMetaFile)
                mpDev->SetMapMode( MapMode( MAP_100TH_MM, aMMOffset, aOldMode.GetScaleX(), aOldMode.GetScaleY() ) );

            
            
            
            Region aRectRegion(aRect);
            mpTargetPaintWindow = pLocalDrawView->BeginDrawLayers(mpDev, aRectRegion, true);
            OSL_ENSURE(mpTargetPaintWindow, "BeginDrawLayers: Got no SdrPaintWindow (!)");

            if (!bMetaFile)
                mpDev->SetMapMode( aOldMode );
        }
    }

    return aMMOffset;
}


void ScOutputData::PostPrintDrawingLayer(const Point& rMMOffset) 
{
    
    
    MapMode aOldMode = mpDev->GetMapMode();

    if (!bMetaFile)
    {
        mpDev->SetMapMode( MapMode( MAP_100TH_MM, rMMOffset, aOldMode.GetScaleX(), aOldMode.GetScaleY() ) );
    }

    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = (pDrawView) ? pDrawView : pViewShell->GetSdrView();

        if(pLocalDrawView)
        {
            
            pLocalDrawView->EndDrawLayers(*mpTargetPaintWindow, true);
            mpTargetPaintWindow = 0;
        }
    }

    
    if (!bMetaFile)
    {
        mpDev->SetMapMode( aOldMode );
    }
}


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

    
    if(bHideAllDrawingLayer || (!mpDoc->GetDrawLayer()))
    {
        return;
    }

    MapMode aOldMode = mpDev->GetMapMode();

    if (!bMetaFile)
    {
        mpDev->SetMapMode( MapMode( MAP_100TH_MM, rMMOffset, aOldMode.GetScaleX(), aOldMode.GetScaleY() ) );
    }

    
    DrawSelectiveObjects( nLayer );

    if (!bMetaFile)
    {
        mpDev->SetMapMode( aOldMode );
    }
}


void ScOutputData::DrawSelectiveObjects(const sal_uInt16 nLayer)
{
    ScDrawLayer* pModel = mpDoc->GetDrawLayer();
    if (!pModel)
        return;

    
    

    SdrOutliner& rOutl = pModel->GetDrawOutliner();
    rOutl.EnableAutoColor( mbUseStyleColor );
    rOutl.SetDefaultHorizontalTextDirection(
                (EEHorizontalTextDirection)mpDoc->GetEditTextDirection( nTab ) );

    
    
    

    pModel->UseHyphenator();

    sal_uLong nOldDrawMode = mpDev->GetDrawMode();
    if ( mbUseStyleColor && Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        mpDev->SetDrawMode( nOldDrawMode | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL |
                            DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );
    }

    
    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = (pDrawView) ? pDrawView : pViewShell->GetSdrView();

        if(pLocalDrawView)
        {
            SdrPageView* pPageView = pLocalDrawView->GetSdrPageView();

            if(pPageView)
            {
                pPageView->DrawLayer(sal::static_int_cast<SdrLayerID>(nLayer), mpDev);
            }
        }
    }

    mpDev->SetDrawMode(nOldDrawMode);

    
    return;
}




void ScOutputData::DrawingSingle(const sal_uInt16 nLayer)
{
    sal_Bool    bHad    = false;
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
            bHad = false;
        }
    }

    if (bHad)
        DrawSelectiveObjects( nLayer );
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
