/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <sfx2/bindings.hxx>
#include <svx/htmlmode.hxx>
#include <svx/svdobj.hxx>
#include <svx/sdtakitm.hxx>
#include <svx/sdtaditm.hxx>
#include <svx/sdtaaitm.hxx>
#include <svx/svdview.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/outlobj.hxx>
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <edtwin.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#ifndef _DRAWBASE_HXX
#include <drawbase.hxx>
#endif
#include <concustomshape.hxx>
#include <svx/gallery.hxx>
#include <sfx2/request.hxx>
#ifndef _FM_FMMODEL_HXX
#include <svx/fmmodel.hxx>
#endif
#include <svl/itempool.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoashp.hxx>
#include <editeng/adjitem.hxx>
#include <svx/svdlegacy.hxx>

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

/*************************************************************************
|*
\************************************************************************/

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
                sal_Bool bForceFillStyle = sal_True;
                sal_Bool bForceNoFillStyle = sal_False;
                if ( ((SdrObjCustomShape*)pObj)->UseNoFillStyle() )
                {
                    bForceFillStyle = sal_False;
                    bForceNoFillStyle = sal_True;
                }

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
    m_pWin->setSdrObjectCreationInfo(SdrObjectCreationInfo(OBJ_CUSTOMSHAPE));

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
                            SfxItemSet aDest( pObj->GetObjectItemPool(),                // ranges from SdrAttrObj
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

                            const long aOldRotation(sdr::legacy::GetRotateAngle(*pSourceObj));
                            if ( aOldRotation )
                            {
                                sdr::legacy::RotateSdrObject(*pObj, sdr::legacy::GetSnapRect(*pObj).Center(), aOldRotation);
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
        pObj->SetMergedItem( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, sal_False ) );
        ((SdrObjCustomShape*)pObj)->MergeDefaultAttributes( &aCustomShape );
    }
}

void ConstCustomShape::CreateDefaultObject()
{
    SwDrawBase::CreateDefaultObject();
    SdrView *pSdrView = m_pSh->GetDrawView();
    if ( pSdrView )
    {
        SdrObject* pObj = pSdrView->getSelectedIfSingle();

        if ( pObj && dynamic_cast< SdrObjCustomShape* >(pObj) )
                SetAttributes( pObj );
        }
    }

// #i33136#
bool ConstCustomShape::doConstructOrthogonal() const
{
    return SdrObjCustomShape::doConstructOrthogonal(aCustomShape);
}

// eof
