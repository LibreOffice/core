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

#include <fuconcustomshape.hxx>
#include <editeng/svxenum.hxx>
#include <svx/gallery.hxx>
#include <sfx2/request.hxx>
#include <svx/fmmodel.hxx>
#include <svl/itempool.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoashp.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdtagitm.hxx>
#include <tabvwsh.hxx>
#include <drawview.hxx>
#include <editeng/adjustitem.hxx>

#include <math.h>

using namespace com::sun::star;

FuConstCustomShape::FuConstCustomShape(ScTabViewShell& rViewSh, vcl::Window* pWin, ScDrawView* pViewP, SdrModel* pDoc, const SfxRequest& rReq )
    : FuConstruct(rViewSh, pWin, pViewP, pDoc, rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    if ( pArgs )
    {
        const SfxStringItem& rItm = static_cast<const SfxStringItem&>(pArgs->Get( rReq.GetSlot() ));
        aCustomShape = rItm.GetValue();
    }
}

FuConstCustomShape::~FuConstCustomShape()
{
}

bool FuConstCustomShape::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);
    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pWindow->CaptureMouse();
        pView->BegCreateObj(aPnt);

        SdrObject* pObj = pView->GetCreateObj();
        if ( pObj )
        {
            SetAttributes( pObj );
            bool bForceNoFillStyle = false;
            if ( static_cast<SdrObjCustomShape*>(pObj)->UseNoFillStyle() )
                bForceNoFillStyle = true;
            if ( bForceNoFillStyle )
                pObj->SetMergedItem( XFillStyleItem( drawing::FillStyle_NONE ) );
        }

        bReturn = true;
    }
    return bReturn;
}

bool FuConstCustomShape::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = false;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        pView->EndCreateObj(SdrCreateCmd::ForceEnd);
        bReturn = true;
    }
    return (FuConstruct::MouseButtonUp(rMEvt) || bReturn);
}

void FuConstCustomShape::Activate()
{
    pView->SetCurrentObj( OBJ_CUSTOMSHAPE );

    aNewPointer = PointerStyle::DrawRect;
    aOldPointer = pWindow->GetPointer();
    rViewShell.SetActivePointer( aNewPointer );

    SdrLayer* pLayer = pView->GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_CONTROLS);
    if (pLayer)
        pView->SetActiveLayer( pLayer->GetName() );

    FuConstruct::Activate();
}

void FuConstCustomShape::Deactivate()
{
    FuConstruct::Deactivate();

    SdrLayer* pLayer = pView->GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_FRONT);
    if (pLayer)
        pView->SetActiveLayer( pLayer->GetName() );

    rViewShell.SetActivePointer( aOldPointer );
}

// Create default drawing objects via keyboard
SdrObjectUniquePtr FuConstCustomShape::CreateDefaultObject(const sal_uInt16 /* nID */, const tools::Rectangle& rRectangle)
{
    SdrObjectUniquePtr pObj(SdrObjFactory::MakeNewObject(
        *pDrDoc,
        pView->GetCurrentObjInventor(),
        pView->GetCurrentObjIdentifier()));

    if( pObj )
    {
        tools::Rectangle aRectangle( rRectangle );
        SetAttributes( pObj.get() );
        if ( SdrObjCustomShape::doConstructOrthogonal( aCustomShape ) )
            ImpForceQuadratic( aRectangle );
        pObj->SetLogicRect( aRectangle );
    }

    return pObj;
}

void FuConstCustomShape::SetAttributes( SdrObject* pObj )
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
                    SfxItemPool& rPool(aFormModel.GetItemPool());
                    rPool.FreezeIdRanges();

                    if ( GalleryExplorer::GetSdrObj( GALLERY_THEME_POWERPOINT, i, &aFormModel ) )
                    {
                        const SdrObject* pSourceObj = aFormModel.GetPage( 0 )->GetObj( 0 );
                        if( pSourceObj )
                        {
                            const SfxItemSet& rSource = pSourceObj->GetMergedItemSet();
                            SfxItemSet aDest(
                                pObj->getSdrModelFromSdrObject().GetItemPool(),
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

// #i33136#
bool FuConstCustomShape::doConstructOrthogonal() const
{
    return SdrObjCustomShape::doConstructOrthogonal(aCustomShape);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
