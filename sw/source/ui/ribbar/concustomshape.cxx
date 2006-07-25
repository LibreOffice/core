/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: concustomshape.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 12:41:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#pragma hdrstop


#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef sdtacitm_hxx //autogen
#include <svx/sdtacitm.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SDTAGITM_HXX //autogen
#include <svx/sdtagitm.hxx>
#endif
#ifndef SDTAKITM_HXX //autogen
#include <svx/sdtakitm.hxx>
#endif
#ifndef _SDTADITM_HXX //autogen
#include <svx/sdtaditm.hxx>
#endif
#ifndef _SDTAAITM_HXX //autogen
#include <svx/sdtaaitm.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SVDCAPT_HXX
#include <svx/svdocapt.hxx>
#endif
#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _DRAWBASE_HXX
#include <drawbase.hxx>
#endif
#ifndef _SW_CONCUSTOMSHAPE_HXX
#include <concustomshape.hxx>
#endif
#ifndef _GALLERY_HXX_
#include <svx/gallery.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _FM_FMMODEL_HXX
#include <svx/fmmodel.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDOASHP_HXX
#include <svx/svdoashp.hxx>
#endif
#ifndef _SDTAGITM_HXX
#include <svx/sdtagitm.hxx>
#endif
#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif


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

BOOL ConstCustomShape::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = SwDrawBase::MouseButtonDown(rMEvt);
    if ( bReturn )
    {
        SdrView *pSdrView = pSh->GetDrawView();
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

                SfxItemSet aAttr( pView->GetPool() );
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

BOOL ConstCustomShape::MouseButtonUp(const MouseEvent& rMEvt)
{
    return SwDrawBase::MouseButtonUp(rMEvt);
}

/*************************************************************************
|*
|* activate function
|*
\************************************************************************/

void ConstCustomShape::Activate(const USHORT nSlotId)
{
    pWin->SetDrawMode( OBJ_CUSTOMSHAPE );

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
        pObj->SetMergedItem( SvxAdjustItem( SVX_ADJUST_CENTER ) );
        pObj->SetMergedItem( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        pObj->SetMergedItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
        pObj->SetMergedItem( SdrTextAutoGrowHeightItem( sal_False ) );
        ((SdrObjCustomShape*)pObj)->MergeDefaultAttributes( &aCustomShape );
    }
}

void ConstCustomShape::CreateDefaultObject()
{
    SwDrawBase::CreateDefaultObject();
    SdrView *pSdrView = pSh->GetDrawView();
    if ( pSdrView )
    {
        const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
        sal_uInt32 nCount = rMarkList.GetMarkCount();
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

// eof
