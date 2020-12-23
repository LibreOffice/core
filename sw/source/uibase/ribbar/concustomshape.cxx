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

#include <svx/svdobj.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/svdview.hxx>
#include <editeng/eeitem.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <wrtsh.hxx>
#include <drawbase.hxx>
#include <concustomshape.hxx>
#include <svx/gallery.hxx>
#include <sfx2/request.hxx>
#include <svx/fmmodel.hxx>
#include <svl/itempool.hxx>
#include <svl/stritem.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoashp.hxx>
#include <svx/xfillit0.hxx>
#include <editeng/adjustitem.hxx>

#include <math.h>

using namespace com::sun::star;

ConstCustomShape::ConstCustomShape( SwWrtShell* pWrtShell, SwEditWin* pEditWin, SwView* pSwView, SfxRequest const & rReq )
    : SwDrawBase( pWrtShell, pEditWin, pSwView )
{
    aCustomShape = ConstCustomShape::GetShapeTypeFromRequest( rReq );
}

const OUString& ConstCustomShape::GetShapeType() const
{
    return aCustomShape;
}

OUString ConstCustomShape::GetShapeTypeFromRequest( SfxRequest const & rReq )
{
    OUString aRet;
    const SfxItemSet* pArgs = rReq.GetArgs();
    if ( pArgs )
    {
        const SfxStringItem& rItm = static_cast<const SfxStringItem&>(pArgs->Get( rReq.GetSlot() ));
        aRet = rItm.GetValue();
    }
    return aRet;
}

bool ConstCustomShape::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = SwDrawBase::MouseButtonDown(rMEvt);
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
                if ( static_cast<SdrObjCustomShape*>(pObj)->UseNoFillStyle() )
                    bForceNoFillStyle = true;

                SfxItemSet aAttr( m_pView->GetPool() );
                if ( bForceNoFillStyle )
                    aAttr.Put( XFillStyleItem( drawing::FillStyle_NONE ) );
                pObj->SetMergedItemSet(aAttr);
            }
        }
    }
    return bReturn;
}

void ConstCustomShape::Activate(const sal_uInt16 nSlotId)
{
    m_pWin->SetSdrDrawMode( OBJ_CUSTOMSHAPE );

    SwDrawBase::Activate(nSlotId);
}

// applying attributes

void ConstCustomShape::SetAttributes( SdrObject* pObj )
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
                                pObj->NbcRotate( pObj->GetSnapRect().Center(), nAngle );
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
        pObj->SetMergedItem( SvxAdjustItem( SvxAdjust::Center, RES_PARATR_ADJUST ) );
        pObj->SetMergedItem( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        pObj->SetMergedItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
        pObj->SetMergedItem( makeSdrTextAutoGrowHeightItem( false ) );
        static_cast<SdrObjCustomShape*>(pObj)->MergeDefaultAttributes( &aCustomShape );
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
            if ( dynamic_cast< const SdrObjCustomShape *>( pObj ) )
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
