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
#include <editeng/adjustitem.hxx>

#include <math.h>



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

bool FuConstCustomShape::MouseButtonDown(const MouseEvent& rMEvt)
{
    
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);
    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        
        
        
        
        Point aGridOff = CurrentGridSyncOffsetAndPos( aPnt );

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
            pObj->SetGridOffset( aGridOff );
        }

        bReturn = true;
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuConstCustomShape::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuConstCustomShape::MouseButtonUp(const MouseEvent& rMEvt)
{
    
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = false;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        pView->EndCreateObj(SDRCREATE_FORCEEND);
        bReturn = true;
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

bool FuConstCustomShape::KeyInput(const KeyEvent& rKEvt)
{
    return FuConstruct::KeyInput(rKEvt);
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
        std::vector< OUString > aObjList;
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
                            SfxItemSet aDest( pObj->GetModel()->GetItemPool(),              
                            SDRATTR_START, SDRATTR_SHADOW_LAST,
                            SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                            SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
                            
                            SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST,
                            
                            SDRATTR_3D_FIRST, SDRATTR_3D_LAST,
                            
                            SDRATTR_CUSTOMSHAPE_FIRST, SDRATTR_CUSTOMSHAPE_LAST,
                            
                            EE_ITEMS_START, EE_ITEMS_END,
                            
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


bool FuConstCustomShape::doConstructOrthogonal() const
{
    return SdrObjCustomShape::doConstructOrthogonal(aCustomShape);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
