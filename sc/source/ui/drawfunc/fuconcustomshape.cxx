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


#include "fuconcustomshape.hxx"
#include <editeng/svxenum.hxx>
#include <svx/gallery.hxx>
#include <sfx2/request.hxx>
#include <svx/fmmodel.hxx>
#include <svl/itempool.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoashp.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdtagitm.hxx>
#include "fuconuno.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"
#include "drawview.hxx"
#include <editeng/adjitem.hxx>

#include <math.h>

//------------------------------------------------------------------------

FuConstCustomShape::FuConstCustomShape( ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP, SdrModel* pDoc, SfxRequest& rReq )
    : FuConstruct( pViewSh, pWin, pViewP, pDoc, rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    if ( pArgs )
    {
        const SfxStringItem& rItm = (const SfxStringItem&)pArgs->Get( rReq.GetSlot() );
        aCustomShape = rItm.GetValue();
    }
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstCustomShape::~FuConstCustomShape()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool FuConstCustomShape::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = FuConstruct::MouseButtonDown(rMEvt);
    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pWindow->CaptureMouse();
        pView->BegCreateObj(aPnt);

        SdrObject* pObj = pView->GetCreateObj();
        if ( pObj )
        {
            SetAttributes( pObj );
            sal_Bool bForceNoFillStyle = false;
            if ( ((SdrObjCustomShape*)pObj)->UseNoFillStyle() )
                bForceNoFillStyle = sal_True;
            if ( bForceNoFillStyle )
                pObj->SetMergedItem( XFillStyleItem( XFILL_NONE ) );
        }

        bReturn = sal_True;
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuConstCustomShape::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool FuConstCustomShape::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = false;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        pView->EndCreateObj(SDRCREATE_FORCEEND);
        bReturn = sal_True;
    }
    return (FuConstruct::MouseButtonUp(rMEvt) || bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

sal_Bool FuConstCustomShape::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstCustomShape::Activate()
{
    pView->SetCurrentObj( OBJ_CUSTOMSHAPE, SdrInventor );

    aNewPointer = Pointer( POINTER_DRAW_RECT );
    aOldPointer = pWindow->GetPointer();
    pViewShell->SetActivePointer( aNewPointer );

    SdrLayer* pLayer = pView->GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_CONTROLS);
    if (pLayer)
        pView->SetActiveLayer( pLayer->GetName() );

    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstCustomShape::Deactivate()
{
    FuConstruct::Deactivate();

    SdrLayer* pLayer = pView->GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_FRONT);
    if (pLayer)
        pView->SetActiveLayer( pLayer->GetName() );

    pViewShell->SetActivePointer( aOldPointer );
}

// Create default drawing objects via keyboard
SdrObject* FuConstCustomShape::CreateDefaultObject(const sal_uInt16 /* nID */, const Rectangle& rRectangle)
{
    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->GetCurrentObjInventor(), pView->GetCurrentObjIdentifier(),
        0L, pDrDoc);
    if( pObj )
    {
        Rectangle aRectangle( rRectangle );
        SetAttributes( pObj );
        if ( SdrObjCustomShape::doConstructOrthogonal( aCustomShape ) )
            ImpForceQuadratic( aRectangle );
        pObj->SetLogicRect( aRectangle );
    }
    return pObj;
}

/*************************************************************************
|*
|* applying attributes
|*
\************************************************************************/

void FuConstCustomShape::SetAttributes( SdrObject* pObj )
{
    sal_Bool bAttributesAppliedFromGallery = false;

    if ( GalleryExplorer::GetSdrObjCount( GALLERY_THEME_POWERPOINT ) )
    {
        std::vector< rtl::OUString > aObjList;
        if ( GalleryExplorer::FillObjListTitle( GALLERY_THEME_POWERPOINT, aObjList ) )
        {
            sal_uInt16 i;
            for ( i = 0; i < aObjList.size(); i++ )
            {
                if ( aObjList[ i ].equalsIgnoreAsciiCase( aCustomShape ) )
                {
                    FmFormModel aFormModel;
                    SfxItemPool& rPool = aFormModel.GetItemPool();
                    rPool.FreezeIdRanges();
                    if ( GalleryExplorer::GetSdrObj( GALLERY_THEME_POWERPOINT, i, &aFormModel ) )
                    {
                        const SdrObject* pSourceObj = aFormModel.GetPage( 0 )->GetObj( 0 );
                        if( pSourceObj )
                        {
                            const SfxItemSet& rSource = pSourceObj->GetMergedItemSet();
                            SfxItemSet aDest( pObj->GetModel()->GetItemPool(),              // ranges from SdrAttrObj
                            SDRATTR_START, SDRATTR_SHADOW_LAST,
                            SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                            SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
                            // Graphic Attributes
                            SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST,
                            // 3d Properties
                            SDRATTR_3D_FIRST, SDRATTR_3D_LAST,
                            // CustomShape properties
                            SDRATTR_CUSTOMSHAPE_FIRST, SDRATTR_CUSTOMSHAPE_LAST,
                            // range from SdrTextObj
                            EE_ITEMS_START, EE_ITEMS_END,
                            // end
                            0, 0);
                            aDest.Set( rSource );
                            pObj->SetMergedItemSet( aDest );
                            sal_Int32 nAngle = pSourceObj->GetRotateAngle();
                            if ( nAngle )
                            {
                                double a = nAngle * F_PI18000;
                                pObj->NbcRotate( pObj->GetSnapRect().Center(), nAngle, sin( a ), cos( a ) );
                            }
                            bAttributesAppliedFromGallery = sal_True;
                        }
                    }
                    break;
                }
            }
        }
    }
    if ( !bAttributesAppliedFromGallery )
    {
        pObj->SetMergedItem( SvxAdjustItem( SVX_ADJUST_CENTER, 0 ) );
        pObj->SetMergedItem( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        pObj->SetMergedItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
        pObj->SetMergedItem( SdrTextAutoGrowHeightItem( false ) );
        ((SdrObjCustomShape*)pObj)->MergeDefaultAttributes( &aCustomShape );
    }
}

// #i33136#
bool FuConstCustomShape::doConstructOrthogonal() const
{
    return SdrObjCustomShape::doConstructOrthogonal(aCustomShape);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
