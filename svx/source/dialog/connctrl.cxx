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

#include <svx/connctrl.hxx>
#include <svx/dlgutil.hxx>

#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <svx/sxelditm.hxx>
#include <sxmkitm.hxx>

#include <vcl/builderfactory.hxx>
#include <vcl/settings.hxx>
#include <memory>

SvxXConnectionPreview::SvxXConnectionPreview()
    : pEdgeObj(nullptr)
    , pView(nullptr)
{
    SetMapMode(MapMode(MapUnit::Map100thMM));
}

void SvxXConnectionPreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    weld::CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize(pDrawingArea->get_ref_device().LogicToPixel(Size(118 , 121), MapMode(MapUnit::MapAppFont)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    SetOutputSizePixel(aSize);
}

SvxXConnectionPreview::~SvxXConnectionPreview()
{
}

void SvxXConnectionPreview::Resize()
{
    AdaptSize();

    Invalidate();
}

void SvxXConnectionPreview::AdaptSize()
{
    // Adapt size
    if( !pSdrPage )
        return;

    SetMapMode(MapMode(MapUnit::Map100thMM));

    OutputDevice* pOD = pView->GetFirstOutputDevice(); // GetWin( 0 );
    tools::Rectangle aRect = pSdrPage->GetAllObjBoundRect();

    MapMode aMapMode = GetMapMode();
    aMapMode.SetMapUnit( pOD->GetMapMode().GetMapUnit() );
    SetMapMode( aMapMode );

    MapMode         aDisplayMap( aMapMode );
    Point           aNewPos;
    Size            aNewSize;
    const Size      aWinSize = GetDrawingArea()->get_ref_device().PixelToLogic(GetOutputSizePixel(), aDisplayMap);
    const long      nWidth = aWinSize.Width();
    const long      nHeight = aWinSize.Height();
    if (aRect.GetHeight() == 0)
        return;
    double          fRectWH = static_cast<double>(aRect.GetWidth()) / aRect.GetHeight();
    if (nHeight == 0)
        return;
    double          fWinWH = static_cast<double>(nWidth) / nHeight;

    // Adapt bitmap to Thumb size (not here!)
    if ( fRectWH < fWinWH)
    {
        aNewSize.setWidth( static_cast<long>( static_cast<double>(nHeight) * fRectWH ) );
        aNewSize.setHeight( nHeight );
    }
    else
    {
        aNewSize.setWidth( nWidth );
        aNewSize.setHeight( static_cast<long>( static_cast<double>(nWidth) / fRectWH ) );
    }

    Fraction aFrac1( aWinSize.Width(), aRect.GetWidth() );
    Fraction aFrac2( aWinSize.Height(), aRect.GetHeight() );
    Fraction aMinFrac( aFrac1 <= aFrac2 ? aFrac1 : aFrac2 );

    // Implement MapMode
    aDisplayMap.SetScaleX( aMinFrac );
    aDisplayMap.SetScaleY( aMinFrac );

    // Centering
    aNewPos.setX( ( nWidth - aNewSize.Width() )  >> 1 );
    aNewPos.setY( ( nHeight - aNewSize.Height() ) >> 1 );

    aDisplayMap.SetOrigin(OutputDevice::LogicToLogic(aNewPos, aMapMode, aDisplayMap));
    SetMapMode( aDisplayMap );

    // Origin
    aNewPos = aDisplayMap.GetOrigin();
    aNewPos -= Point( aRect.TopLeft().X(), aRect.TopLeft().Y() );
    aDisplayMap.SetOrigin( aNewPos );
    SetMapMode( aDisplayMap );

    MouseEvent aMEvt( Point(), 1, MouseEventModifiers::NONE, MOUSE_RIGHT );
    MouseButtonDown( aMEvt );
}

void SvxXConnectionPreview::Construct()
{
    DBG_ASSERT( pView, "No valid view is passed on! ");

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    const size_t nMarkCount = rMarkList.GetMarkCount();

    if( nMarkCount >= 1 )
    {
        bool bFound = false;

        for( size_t i = 0; i < nMarkCount && !bFound; ++i )
        {
            const SdrObject* pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
            SdrInventor nInv = pObj->GetObjInventor();
            sal_uInt16 nId = pObj->GetObjIdentifier();
            if( nInv == SdrInventor::Default && nId == OBJ_EDGE )
            {
                bFound = true;

                // potential memory leak here (!). Create SdrObjList only when there is
                // not yet one.
                if(!pSdrPage)
                {
                    pSdrPage.reset( new SdrPage(
                        pView->getSdrModelFromSdrView(),
                        false) );
                }

                const SdrEdgeObj* pTmpEdgeObj = static_cast<const SdrEdgeObj*>(pObj);
                pEdgeObj = pTmpEdgeObj->CloneSdrObject(pSdrPage->getSdrModelFromSdrPage());

                SdrObjConnection& rConn1 = pEdgeObj->GetConnection( true );
                SdrObjConnection& rConn2 = pEdgeObj->GetConnection( false );

                rConn1 = pTmpEdgeObj->GetConnection( true );
                rConn2 = pTmpEdgeObj->GetConnection( false );

                SdrObject* pTmpObj1 = pTmpEdgeObj->GetConnectedNode( true );
                SdrObject* pTmpObj2 = pTmpEdgeObj->GetConnectedNode( false );

                if( pTmpObj1 )
                {
                    SdrObject* pObj1 = pTmpObj1->CloneSdrObject(pSdrPage->getSdrModelFromSdrPage());
                    pSdrPage->InsertObject( pObj1 );
                    pEdgeObj->ConnectToNode( true, pObj1 );
                }

                if( pTmpObj2 )
                {
                    SdrObject* pObj2 = pTmpObj2->CloneSdrObject(pSdrPage->getSdrModelFromSdrPage());
                    pSdrPage->InsertObject( pObj2 );
                    pEdgeObj->ConnectToNode( false, pObj2 );
                }

                pSdrPage->InsertObject( pEdgeObj );
            }
        }
    }

    if( !pEdgeObj )
    {
        pEdgeObj = new SdrEdgeObj(pView->getSdrModelFromSdrView());
    }

    AdaptSize();
}

void SvxXConnectionPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.Push(PushFlags::ALL);

    rRenderContext.SetMapMode(GetMapMode());

    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    rRenderContext.SetDrawMode(rStyles.GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR);
    rRenderContext.SetBackground(Wallpaper(rStyles.GetFieldColor()));

    if (pSdrPage)
    {
        // This will not work anymore. To not start at Adam and Eve, i will
        // ATM not try to change all this stuff to really using an own model
        // and a view. I will just try to provide a mechanism to paint such
        // objects without own model and without a page/view with the new
        // mechanism.

        // New stuff: Use a ObjectContactOfObjListPainter.
        sdr::contact::SdrObjectVector aObjectVector;

        for (size_t a = 0; a < pSdrPage->GetObjCount(); ++a)
        {
            SdrObject* pObject = pSdrPage->GetObj(a);
            DBG_ASSERT(pObject,
                "SvxXConnectionPreview::Paint: Corrupt ObjectList (!)");
            aObjectVector.push_back(pObject);
        }

        sdr::contact::ObjectContactOfObjListPainter aPainter(rRenderContext, aObjectVector, nullptr);
        sdr::contact::DisplayInfo aDisplayInfo;

        // do processing
        aPainter.ProcessDisplay(aDisplayInfo);
    }

    rRenderContext.Pop();
}

void SvxXConnectionPreview::SetAttributes( const SfxItemSet& rInAttrs )
{
    pEdgeObj->SetMergedItemSetAndBroadcast(rInAttrs);

    Invalidate();
}

// Get number of lines which are offset based on the preview object

sal_uInt16 SvxXConnectionPreview::GetLineDeltaCount()
{
    const SfxItemSet& rSet = pEdgeObj->GetMergedItemSet();
    sal_uInt16 nCount(0);

    if(SfxItemState::DONTCARE != rSet.GetItemState(SDRATTR_EDGELINEDELTACOUNT))
        nCount = rSet.Get(SDRATTR_EDGELINEDELTACOUNT).GetValue();

    return nCount;
}

bool SvxXConnectionPreview::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bZoomIn  = rMEvt.IsLeft() && !rMEvt.IsShift();
    bool bZoomOut = rMEvt.IsRight() || rMEvt.IsShift();
    bool bCtrl    = rMEvt.IsMod1();

    if( bZoomIn || bZoomOut )
    {
        MapMode aMapMode = GetMapMode();
        Fraction aXFrac = aMapMode.GetScaleX();
        Fraction aYFrac = aMapMode.GetScaleY();
        std::unique_ptr<Fraction> pMultFrac;

        if( bZoomIn )
        {
            if( bCtrl )
                pMultFrac.reset(new Fraction( 3, 2 ));
            else
                pMultFrac.reset(new Fraction( 11, 10 ));
        }
        else
        {
            if( bCtrl )
                pMultFrac.reset(new Fraction( 2, 3 ));
            else
                pMultFrac.reset(new Fraction( 10, 11 ));
        }

        aXFrac *= *pMultFrac;
        aYFrac *= *pMultFrac;
        if( static_cast<double>(aXFrac) > 0.001 && static_cast<double>(aXFrac) < 1000.0 &&
            static_cast<double>(aYFrac) > 0.001 && static_cast<double>(aYFrac) < 1000.0 )
        {
            aMapMode.SetScaleX( aXFrac );
            aMapMode.SetScaleY( aYFrac );
            SetMapMode( aMapMode );

            Size aOutSize(GetOutputSizePixel());
            aOutSize = GetDrawingArea()->get_ref_device().PixelToLogic(aOutSize);

            Point aPt( aMapMode.GetOrigin() );
            long nX = static_cast<long>( ( static_cast<double>(aOutSize.Width()) - ( static_cast<double>(aOutSize.Width()) * static_cast<double>(*pMultFrac)  ) ) / 2.0 + 0.5 );
            long nY = static_cast<long>( ( static_cast<double>(aOutSize.Height()) - ( static_cast<double>(aOutSize.Height()) * static_cast<double>(*pMultFrac)  ) ) / 2.0 + 0.5 );
            aPt.AdjustX(nX );
            aPt.AdjustY(nY );

            aMapMode.SetOrigin( aPt );
            SetMapMode( aMapMode );

            Invalidate();
        }
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
