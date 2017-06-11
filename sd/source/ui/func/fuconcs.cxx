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

#include "fuconcs.hxx"
#include <svx/svdpagv.hxx>

#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>

#include "app.hrc"
#include <svl/aeitem.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnwtit.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/sdtmfitm.hxx>
#include <svx/sxekitm.hxx>
#include <svx/sderitm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdocirc.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <editeng/adjustitem.hxx>
#include <svx/xtable.hxx>
#include <svx/sdasitm.hxx>
#include <svx/svdoashp.hxx>
#include <svx/sdtagitm.hxx>

#include <svx/svdocapt.hxx>

#include <svx/svdomeas.hxx>
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"
#include <editeng/writingmodeitem.hxx>
#include <svx/gallery.hxx>
#include <svl/itempool.hxx>

#include "sdresid.hxx"
#include "View.hxx"
#include "sdpage.hxx"
#include "Window.hxx"
#include "stlpool.hxx"
#include "drawdoc.hxx"

#include "strings.hrc"

namespace sd {


FuConstructCustomShape::FuConstructCustomShape (
        ViewShell*          pViewSh,
        ::sd::Window*       pWin,
        ::sd::View*         pView,
        SdDrawDocument*     pDoc,
        SfxRequest&         rReq ) :
    FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuConstructCustomShape::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuConstructCustomShape* pFunc;
    rtl::Reference<FuPoor> xFunc( pFunc = new FuConstructCustomShape( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    pFunc->SetPermanent( bPermanent );
    return xFunc;
}

void FuConstructCustomShape::DoExecute( SfxRequest& rReq )
{
    FuConstruct::DoExecute( rReq );

    const SfxItemSet* pArgs = rReq.GetArgs();
    if ( pArgs )
    {
        const SfxStringItem& rItm = static_cast<const SfxStringItem&>(pArgs->Get( rReq.GetSlot() ));
        aCustomShape = rItm.GetValue();
    }

    mpViewShell->GetViewShellBase().GetToolBarManager()->SetToolBar(
        ToolBarManager::ToolBarGroup::Function,
        ToolBarManager::msDrawingObjectToolBar);
}

bool FuConstructCustomShape::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

        mpWindow->CaptureMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        mpView->BegCreateObj(aPnt, nullptr, nDrgLog);

        SdrObject* pObj = mpView->GetCreateObj();
        if ( pObj )
        {
            SetAttributes( pObj );
            bool bForceFillStyle = true;
            bool bForceNoFillStyle = false;
            if ( static_cast<SdrObjCustomShape*>(pObj)->UseNoFillStyle() )
            {
                bForceFillStyle = false;
                bForceNoFillStyle = true;
            }
            SfxItemSet aAttr(mpDoc->GetPool());
            SetStyleSheet( aAttr, pObj, bForceFillStyle, bForceNoFillStyle );
            pObj->SetMergedItemSet(aAttr);
        }
    }

    return bReturn;
}

bool FuConstructCustomShape::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn(false);

    if(mpView->IsCreateObj() && rMEvt.IsLeft())
    {
        SdrObject* pObj = mpView->GetCreateObj();
        if( pObj && mpView->EndCreateObj( SdrCreateCmd::ForceEnd ) )
        {
            bReturn = true;
        }
    }
    bReturn = FuConstruct::MouseButtonUp (rMEvt) || bReturn;

    if (!bPermanent)
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);

    return bReturn;
}

void FuConstructCustomShape::Activate()
{
    mpView->SetCurrentObj( OBJ_CUSTOMSHAPE );
    FuConstruct::Activate();
}

/**
 * set attribute for the object to be created
 */
void FuConstructCustomShape::SetAttributes( SdrObject* pObj )
{
    bool bAttributesAppliedFromGallery = false;

    if ( GalleryExplorer::GetSdrObjCount( GALLERY_THEME_POWERPOINT ) )
    {
        std::vector< OUString > aObjList;
        if ( GalleryExplorer::FillObjListTitle( GALLERY_THEME_POWERPOINT, aObjList ) )
        {
            for ( std::vector<OUString>::size_type i = 0; i < aObjList.size(); i++ )
            {
                if ( aObjList[ i ].equalsIgnoreAsciiCase( aCustomShape ) )
                {
                    FmFormModel aFormModel;
                    SfxItemPool& rPool = aFormModel.GetItemPool();
                    rPool.FreezeIdRanges();
                    if ( GalleryExplorer::GetSdrObj( GALLERY_THEME_POWERPOINT, i, &aFormModel ) )
                    {
                        const SdrPage* pPage = aFormModel.GetPage( 0 );
                        if ( pPage )
                        {
                            const SdrObject* pSourceObj = pPage->GetObj( 0 );
                            if( pSourceObj )
                            {
                                const SfxItemSet& rSource = pSourceObj->GetMergedItemSet();
                                SfxItemSet aDest(
                                    pObj->GetModel()->GetItemPool(),
                                    svl::Items<
                                        // Ranges from SdrAttrObj:
                                        SDRATTR_START, SDRATTR_SHADOW_LAST,
                                        SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                                        SDRATTR_TEXTDIRECTION,
                                            SDRATTR_TEXTDIRECTION,
                                        // Graphic attributes, 3D properties,
                                        // CustomShape properties:
                                        SDRATTR_GRAF_FIRST,
                                            SDRATTR_CUSTOMSHAPE_LAST,
                                        // Range from SdrTextObj:
                                        EE_ITEMS_START, EE_ITEMS_END>{});
                                aDest.Set( rSource );
                                pObj->SetMergedItemSet( aDest );
                                sal_Int32 nAngle = pSourceObj->GetRotateAngle();
                                if ( nAngle )
                                {
                                    double a = nAngle * F_PI18000;
                                    pObj->NbcRotate( pObj->GetSnapRect().Center(), nAngle, sin( a ), cos( a ) );
                                }
                                bAttributesAppliedFromGallery = true;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    if ( !bAttributesAppliedFromGallery )
    {
        pObj->SetMergedItem( SvxAdjustItem( SvxAdjust::Center, EE_PARA_JUST ) );
        pObj->SetMergedItem( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        pObj->SetMergedItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
        pObj->SetMergedItem( makeSdrTextAutoGrowHeightItem( false ) );
        static_cast<SdrObjCustomShape*>(pObj)->MergeDefaultAttributes( &aCustomShape );
    }
}

const OUString& FuConstructCustomShape::GetShapeType() const
{
    return aCustomShape;
}

SdrObject* FuConstructCustomShape::CreateDefaultObject(const sal_uInt16, const ::tools::Rectangle& rRectangle)
{
    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        mpView->GetCurrentObjInventor(), mpView->GetCurrentObjIdentifier(),
        nullptr, mpDoc);

    if( pObj )
    {
        ::tools::Rectangle aRect( rRectangle );
        if ( doConstructOrthogonal() )
            ImpForceQuadratic( aRect );
        pObj->SetLogicRect( aRect );
        SetAttributes( pObj );
        SfxItemSet aAttr(mpDoc->GetPool());
        SetStyleSheet(aAttr, pObj);
        pObj->SetMergedItemSet(aAttr);
    }
    return pObj;
}

// #i33136#
bool FuConstructCustomShape::doConstructOrthogonal() const
{
    return SdrObjCustomShape::doConstructOrthogonal(aCustomShape);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
