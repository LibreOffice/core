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

#include <vcl/svapp.hxx>

#include "svx/connctrl.hxx"
#include "svx/dlgutil.hxx"

#include <svx/dialmgr.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdpage.hxx> // SdrObjList
#include <svx/svdview.hxx>
#include <svx/sxelditm.hxx>
#include <svx/sxmkitm.hxx>

#include <vcl/builder.hxx>

SvxXConnectionPreview::SvxXConnectionPreview( Window* pParent, WinBits nStyle)
    : Control(pParent, nStyle)
    , pEdgeObj(NULL)
    , pObjList(NULL)
    , pView(NULL)
{
    SetMapMode( MAP_100TH_MM );
    SetStyles();
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxXConnectionPreview(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = 0;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    return new SvxXConnectionPreview(pParent, nWinStyle);
}

SvxXConnectionPreview::~SvxXConnectionPreview()
{
    delete pObjList;
}

void SvxXConnectionPreview::Resize()
{
    Control::Resize();

    AdaptSize();

    Invalidate();
}

Size SvxXConnectionPreview::GetOptimalSize() const
{
    return LogicToPixel(Size(118 , 121), MapMode(MAP_APPFONT));
}

void SvxXConnectionPreview::AdaptSize()
{
    // Adapt size
    if( pObjList )
    {
        SetMapMode( MAP_100TH_MM );

        OutputDevice* pOD = pView->GetFirstOutputDevice(); // GetWin( 0 );
        Rectangle aRect = pObjList->GetAllObjBoundRect();

        MapMode aMapMode = GetMapMode();
        aMapMode.SetMapUnit( pOD->GetMapMode().GetMapUnit() );
        SetMapMode( aMapMode );

        MapMode         aDisplayMap( aMapMode );
        Point           aNewPos;
        Size            aNewSize;
        const Size      aWinSize = PixelToLogic( GetOutputSizePixel(), aDisplayMap );
        const long      nWidth = aWinSize.Width();
        const long      nHeight = aWinSize.Height();
        double          fRectWH = (double) aRect.GetWidth() / aRect.GetHeight();
        double          fWinWH = (double) nWidth / nHeight;

        // Adapt bitmap to Thumb size (not here!)
        if ( fRectWH < fWinWH)
        {
            aNewSize.Width() = (long) ( (double) nHeight * fRectWH );
            aNewSize.Height()= nHeight;
        }
        else
        {
            aNewSize.Width() = nWidth;
            aNewSize.Height()= (long) ( (double) nWidth / fRectWH );
        }

        Fraction aFrac1( aWinSize.Width(), aRect.GetWidth() );
        Fraction aFrac2( aWinSize.Height(), aRect.GetHeight() );
        Fraction aMinFrac( aFrac1 <= aFrac2 ? aFrac1 : aFrac2 );

        // Implement MapMode
        aDisplayMap.SetScaleX( aMinFrac );
        aDisplayMap.SetScaleY( aMinFrac );

        // Centering
        aNewPos.X() = ( nWidth - aNewSize.Width() )  >> 1;
        aNewPos.Y() = ( nHeight - aNewSize.Height() ) >> 1;

        aDisplayMap.SetOrigin( LogicToLogic( aNewPos, aMapMode, aDisplayMap ) );
        SetMapMode( aDisplayMap );

        // Origin
        aNewPos = aDisplayMap.GetOrigin();
        aNewPos -= Point( aRect.TopLeft().X(), aRect.TopLeft().Y() );
        aDisplayMap.SetOrigin( aNewPos );
        SetMapMode( aDisplayMap );

        Point aPos;
        MouseEvent aMEvt( aPos, 1, 0, MOUSE_RIGHT );
        MouseButtonDown( aMEvt );
    }
}

void SvxXConnectionPreview::Construct()
{
    DBG_ASSERT( pView, "No valid view is passed on! ");

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    sal_uIntPtr nMarkCount = rMarkList.GetMarkCount();

    if( nMarkCount >= 1 )
    {
        bool bFound = false;
        const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();


        for( sal_uInt16 i = 0; i < nMarkCount && !bFound; i++ )
        {
            pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
            sal_uInt32 nInv = pObj->GetObjInventor();
            sal_uInt16 nId = pObj->GetObjIdentifier();
            if( nInv == SdrInventor && nId == OBJ_EDGE )
            {
                bFound = true;
                SdrEdgeObj* pTmpEdgeObj = (SdrEdgeObj*) pObj;
                pEdgeObj = (SdrEdgeObj*) pTmpEdgeObj->Clone();

                SdrObjConnection& rConn1 = (SdrObjConnection&)pEdgeObj->GetConnection( sal_True );
                SdrObjConnection& rConn2 = (SdrObjConnection&)pEdgeObj->GetConnection( sal_False );

                rConn1 = pTmpEdgeObj->GetConnection( sal_True );
                rConn2 = pTmpEdgeObj->GetConnection( sal_False );

                SdrObject* pTmpObj1 = pTmpEdgeObj->GetConnectedNode( sal_True );
                SdrObject* pTmpObj2 = pTmpEdgeObj->GetConnectedNode( sal_False );

                // potential memory leak here (!). Create SdrObjList only when there is
                // not yet one.
                if(!pObjList)
                {
                    pObjList = new SdrObjList( pView->GetModel(), NULL );
                }

                if( pTmpObj1 )
                {
                    SdrObject* pObj1 = pTmpObj1->Clone();
                    pObjList->InsertObject( pObj1 );
                    pEdgeObj->ConnectToNode( sal_True, pObj1 );
                }
                if( pTmpObj2 )
                {
                    SdrObject* pObj2 = pTmpObj2->Clone();
                    pObjList->InsertObject( pObj2 );
                    pEdgeObj->ConnectToNode( sal_False, pObj2 );
                }
                pObjList->InsertObject( pEdgeObj );
            }
        }
    }

    if( !pEdgeObj )
        pEdgeObj = new SdrEdgeObj();

    AdaptSize();
}

void SvxXConnectionPreview::Paint( const Rectangle& )
{
    if( pObjList )
    {
        // #110094#
        // This will not work anymore. To not start at Adam and Eve, i will
        // ATM not try to change all this stuff to really using an own model
        // and a view. I will just try to provide a mechanism to paint such
        // objects without own model and without a page/view with the new
        // mechanism.

        // New stuff: Use a ObjectContactOfObjListPainter.
        sdr::contact::SdrObjectVector aObjectVector;

        for(sal_uInt32 a(0L); a < pObjList->GetObjCount(); a++)
        {
            SdrObject* pObject = pObjList->GetObj(a);
            DBG_ASSERT(pObject,
                "SvxXConnectionPreview::Paint: Corrupt ObjectList (!)");
            aObjectVector.push_back(pObject);
        }

        sdr::contact::ObjectContactOfObjListPainter aPainter(*this, aObjectVector, 0);
        sdr::contact::DisplayInfo aDisplayInfo;

        // do processing
        aPainter.ProcessDisplay(aDisplayInfo);
    }
}

void SvxXConnectionPreview::SetAttributes( const SfxItemSet& rInAttrs )
{
    pEdgeObj->SetMergedItemSetAndBroadcast(rInAttrs);

    Invalidate();
}


// Get number of lines which are offset based on the preview object

sal_uInt16 SvxXConnectionPreview::GetLineDeltaAnz()
{
    const SfxItemSet& rSet = pEdgeObj->GetMergedItemSet();
    sal_uInt16 nCount(0);

    if(SFX_ITEM_DONTCARE != rSet.GetItemState(SDRATTR_EDGELINEDELTAANZ))
        nCount = ((const SdrEdgeLineDeltaAnzItem&)rSet.Get(SDRATTR_EDGELINEDELTAANZ)).GetValue();

    return nCount;
}

void SvxXConnectionPreview::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bZoomIn  = rMEvt.IsLeft() && !rMEvt.IsShift();
    bool bZoomOut = rMEvt.IsRight() || rMEvt.IsShift();
    bool bCtrl    = rMEvt.IsMod1();

    if( bZoomIn || bZoomOut )
    {
        MapMode aMapMode = GetMapMode();
        Fraction aXFrac = aMapMode.GetScaleX();
        Fraction aYFrac = aMapMode.GetScaleY();
        Fraction* pMultFrac;

        if( bZoomIn )
        {
            if( bCtrl )
                pMultFrac = new Fraction( 3, 2 );
            else
                pMultFrac = new Fraction( 11, 10 );
        }
        else
        {
            if( bCtrl )
                pMultFrac = new Fraction( 2, 3 );
            else
                pMultFrac = new Fraction( 10, 11 );
        }

        aXFrac *= *pMultFrac;
        aYFrac *= *pMultFrac;
        if( (double)aXFrac > 0.001 && (double)aXFrac < 1000.0 &&
            (double)aYFrac > 0.001 && (double)aYFrac < 1000.0 )
        {
            aMapMode.SetScaleX( aXFrac );
            aMapMode.SetScaleY( aYFrac );
            SetMapMode( aMapMode );

            Size aOutSize( GetOutputSize() );

            Point aPt( aMapMode.GetOrigin() );
            long nX = (long)( ( (double)aOutSize.Width() - ( (double)aOutSize.Width() * (double)*pMultFrac  ) ) / 2.0 + 0.5 );
            long nY = (long)( ( (double)aOutSize.Height() - ( (double)aOutSize.Height() * (double)*pMultFrac  ) ) / 2.0 + 0.5 );
            aPt.X() +=  nX;
            aPt.Y() +=  nY;

            aMapMode.SetOrigin( aPt );
            SetMapMode( aMapMode );

            Invalidate();
        }
        delete pMultFrac;
    }
}

void SvxXConnectionPreview::SetStyles()
{
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );
    SetBackground( Wallpaper( Color( rStyles.GetFieldColor() ) ) );
}

void SvxXConnectionPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetStyles();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
