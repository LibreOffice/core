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
#include <editeng/adjitem.hxx>

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

rtl::OUString ConstCustomShape::GetShapeType() const
{
    return aCustomShape;
}

//static
rtl::OUString ConstCustomShape::GetShapeTypeFromRequest( SfxRequest& rReq )
{
    rtl::OUString aRet;
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
                sal_Bool bForceNoFillStyle = sal_False;
                if ( ((SdrObjCustomShape*)pObj)->UseNoFillStyle() )
                    bForceNoFillStyle = sal_True;

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
    sal_Bool bAttributesAppliedFromGallery = sal_False;

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
