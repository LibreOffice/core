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
#include <svx/tbxcustomshapes.hxx>
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
#include <com/sun/star/uno/Any.hxx>

#include "sdresid.hxx"
#include "View.hxx"
#include "sdpage.hxx"
#include "Window.hxx"
#include "stlpool.hxx"
#include "drawdoc.hxx"
#include "res_bmp.hrc"
#include "glob.hrc"

namespace sd {

TYPEINIT1( FuConstructCustomShape, FuConstruct );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstructCustomShape::FuConstructCustomShape (
        ViewShell*          pViewSh,
        ::sd::Window*       pWin,
        ::sd::View*         pView,
        SdDrawDocument*     pDoc,
        SfxRequest&         rReq ) :
    FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuConstructCustomShape::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuConstructCustomShape* pFunc;
    FunctionReference xFunc( pFunc = new FuConstructCustomShape( pViewSh, pWin, pView, pDoc, rReq ) );
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
        const SfxStringItem& rItm = (const SfxStringItem&)pArgs->Get( rReq.GetSlot() );
        aCustomShape = rItm.GetValue();
    }

    mpViewShell->GetViewShellBase().GetToolBarManager()->SetToolBar(
        ToolBarManager::TBG_FUNCTION,
        ToolBarManager::msDrawingObjectToolBar);
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool FuConstructCustomShape::MouseButtonDown(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

        mpWindow->CaptureMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        mpView->BegCreateObj(aPnt, (OutputDevice*) NULL, nDrgLog);

        SdrObject* pObj = mpView->GetCreateObj();
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
            SfxItemSet aAttr(mpDoc->GetPool());
            SetStyleSheet( aAttr, pObj, bForceFillStyle, bForceNoFillStyle );
            pObj->SetMergedItemSet(aAttr);
        }
    }

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuConstructCustomShape::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool FuConstructCustomShape::MouseButtonUp(const MouseEvent& rMEvt)
{
    sal_Bool bReturn(sal_False);

    if(mpView->IsCreateObj() && rMEvt.IsLeft())
    {
        SdrObject* pObj = mpView->GetCreateObj();
        if( pObj && mpView->EndCreateObj( SDRCREATE_FORCEEND ) )
        {
            bReturn = sal_True;
        }
    }
    bReturn = FuConstruct::MouseButtonUp (rMEvt) || bReturn;

    if (!bPermanent)
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

    return bReturn;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* sal_False.
|*
\************************************************************************/

sal_Bool FuConstructCustomShape::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstructCustomShape::Activate()
{
    mpView->SetCurrentObj( OBJ_CUSTOMSHAPE );
    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Attribute fuer das zu erzeugende Objekt setzen
|*
\************************************************************************/

void FuConstructCustomShape::SetAttributes( SdrObject* pObj )
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
                        const SdrPage* pPage = aFormModel.GetPage( 0 );
                        if ( pPage )
                        {
                            const SdrObject* pSourceObj = pPage->GetObj( 0 );
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


    /*
                                com::sun::star::uno::Any aAny;
                                if ( ((SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY )).QueryValue( aAny ) )
                                {
                                    aGeometryItem.PutValue( aAny );
                                    pObj->SetMergedItem( aGeometryItem );
                                    bAttributesAppliedFromGallery = sal_True;
                                }
    */
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
        pObj->SetMergedItem( SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ) );
        pObj->SetMergedItem( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        pObj->SetMergedItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
        pObj->SetMergedItem( SdrTextAutoGrowHeightItem( sal_False ) );
        ((SdrObjCustomShape*)pObj)->MergeDefaultAttributes( &aCustomShape );
    }
}

SdrObject* FuConstructCustomShape::CreateDefaultObject(const sal_uInt16, const Rectangle& rRectangle)
{
    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        mpView->GetCurrentObjInventor(), mpView->GetCurrentObjIdentifier(),
        0L, mpDoc);

    if( pObj )
    {
        Rectangle aRect( rRectangle );
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
