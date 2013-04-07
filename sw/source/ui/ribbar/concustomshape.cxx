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

#include <sfx2/bindings.hxx>
#include <svx/htmlmode.hxx>
#include <svx/sdtacitm.hxx>
#include <svx/svdobj.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtakitm.hxx>
#include <svx/sdtaditm.hxx>
#include <svx/sdtaaitm.hxx>
#include <svx/svdview.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/outlobj.hxx>
#include <cmdid.h>
#include <view.hxx>
#include <edtwin.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <drawbase.hxx>
#include <concustomshape.hxx>
#include <svx/gallery.hxx>
#include <sfx2/request.hxx>
#include <svx/fmmodel.hxx>
#include <svl/itempool.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoashp.hxx>
#include <editeng/adjustitem.hxx>

#include <math.h>

/*************************************************************************
|*
|* C'Tor
|*
\************************************************************************/
ConstCustomShape::ConstCustomShape( SwWrtShell* pWrtShell, SwEditWin* pEditWin, SwView* pSwView, SfxRequest& rReq )
    : SwDrawBase( pWrtShell, pEditWin, pSwView )
{
    aCustomShape = ConstCustomShape::GetShapeTypeFromRequest( rReq );
}

OUString ConstCustomShape::GetShapeType() const
{
    return aCustomShape;
}

//static
OUString ConstCustomShape::GetShapeTypeFromRequest( SfxRequest& rReq )
{
    OUString aRet;
    const SfxItemSet* pArgs = rReq.GetArgs();
    if ( pArgs )
    {
        const SfxStringItem& rItm = (const SfxStringItem&)pArgs->Get( rReq.GetSlot() );
        aRet = rItm.GetValue();
    }
    return aRet;
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool ConstCustomShape::MouseButtonDown(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = SwDrawBase::MouseButtonDown(rMEvt);
    if ( bReturn )
    {
        SdrView *pSdrView = m_pSh->GetDrawView();
        if ( pSdrView )
        {
            SdrObject* pObj = pSdrView->GetCreateObj();
            if ( pObj )
            {
                SetAttributes( pObj );
                bool bForceNoFillStyle = false;
                if ( ((SdrObjCustomShape*)pObj)->UseNoFillStyle() )
                    bForceNoFillStyle = true;

                SfxItemSet aAttr( m_pView->GetPool() );
                if ( bForceNoFillStyle )
                    aAttr.Put( XFillStyleItem( XFILL_NONE ) );
                pObj->SetMergedItemSet(aAttr);
            }
        }
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool ConstCustomShape::MouseButtonUp(const MouseEvent& rMEvt)
{
    return SwDrawBase::MouseButtonUp(rMEvt);
}

/*************************************************************************
|*
|* activate function
|*
\************************************************************************/

void ConstCustomShape::Activate(const sal_uInt16 nSlotId)
{
    m_pWin->SetSdrDrawMode( OBJ_CUSTOMSHAPE );

    SwDrawBase::Activate(nSlotId);
}

/*************************************************************************
|*
|* applying attributes
|*
\************************************************************************/

void ConstCustomShape::SetAttributes( SdrObject* pObj )
{
    bool bAttributesAppliedFromGallery = false;

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
        pObj->SetMergedItem( SvxAdjustItem( SVX_ADJUST_CENTER, RES_PARATR_ADJUST ) );
        pObj->SetMergedItem( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        pObj->SetMergedItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
        pObj->SetMergedItem( SdrTextAutoGrowHeightItem( sal_False ) );
        ((SdrObjCustomShape*)pObj)->MergeDefaultAttributes( &aCustomShape );
    }
}

void ConstCustomShape::CreateDefaultObject()
{
    SwDrawBase::CreateDefaultObject();
    SdrView *pSdrView = m_pSh->GetDrawView();
    if ( pSdrView )
    {
        const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
        if ( rMarkList.GetMarkCount() == 1 )
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            if ( pObj && pObj->ISA( SdrObjCustomShape ) )
                SetAttributes( pObj );
        }
    }
}

// #i33136#
bool ConstCustomShape::doConstructOrthogonal() const
{
    return SdrObjCustomShape::doConstructOrthogonal(aCustomShape);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
