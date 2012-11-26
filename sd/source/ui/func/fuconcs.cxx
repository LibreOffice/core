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
#include "precompiled_sd.hxx"

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
#include <svx/sxekitm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdocirc.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <editeng/adjitem.hxx>
#include <svx/xtable.hxx>
#include <svx/sdasitm.hxx>
#include <svx/tbxcustomshapes.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdomeas.hxx>
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"
#include <editeng/writingmodeitem.hxx>
#include <svx/gallery.hxx>
#include <svl/itempool.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <svx/svdlegacy.hxx>

#include "sdresid.hxx"
#include "View.hxx"
#include "sdpage.hxx"
#include "Window.hxx"
#include "stlpool.hxx"
#include "drawdoc.hxx"
#include "res_bmp.hrc"
#include "glob.hrc"

namespace sd {

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

bool FuConstructCustomShape::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        const basegfx::B2DPoint aLogicPos(mpWindow->GetInverseViewTransformation() * aPixelPos);

        mpWindow->CaptureMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        mpView->BegCreateObj(aLogicPos, nDrgLog);

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
            SfxItemSet aAttr(mpDoc->GetItemPool());
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

bool FuConstructCustomShape::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuConstructCustomShape::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn(false);

    if(mpView->GetCreateObj() && rMEvt.IsLeft())
    {
        SdrObject* pObj = mpView->GetCreateObj();
        if( pObj && mpView->EndCreateObj( SDRCREATE_FORCEEND ) )
        {
            bReturn = true;
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
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert true, andernfalls
|* false.
|*
\************************************************************************/

bool FuConstructCustomShape::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = FuConstruct::KeyInput(rKEvt);

    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstructCustomShape::Activate()
{
    mpView->setSdrObjectCreationInfo(SdrObjectCreationInfo(static_cast< sal_uInt16 >(OBJ_CUSTOMSHAPE)));
    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Attribute fuer das zu erzeugende Objekt setzen
|*
\************************************************************************/

void FuConstructCustomShape::SetAttributes( SdrObject* pObj )
{
    OSL_ENSURE(pObj, "FuConstructCustomShape::SetAttributes without SdrObject (!)");
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

                                const long nAngle(sdr::legacy::GetRotateAngle(*pSourceObj));
                                if ( nAngle )
                                {
                                    const Rectangle aOldObjSnapRect(sdr::legacy::GetSnapRect(*pObj));
                                    sdr::legacy::RotateSdrObject(*pObj, aOldObjSnapRect.Center(), nAngle);
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
        pObj->SetMergedItem( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, sal_False ) );
        ((SdrObjCustomShape*)pObj)->MergeDefaultAttributes( &aCustomShape );
    }
}

// #97016#
SdrObject* FuConstructCustomShape::CreateDefaultObject(const sal_uInt16, const basegfx::B2DRange& rRange)
{
    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        mpView->getSdrModelFromSdrView(),
        mpView->getSdrObjectCreationInfo());

    if( pObj )
    {
        basegfx::B2DRange aRange( rRange );

        if ( doConstructOrthogonal() )
        {
            ImpForceQuadratic( aRange );
        }

        sdr::legacy::SetLogicRange(*pObj, aRange );
        SetAttributes( pObj );
        SfxItemSet aAttr(pObj->GetObjectItemPool());
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
