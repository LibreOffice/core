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

#include "DrawViewWrapper.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "ConfigurationAccess.hxx"
#include "macros.hxx"

#include <unotools/lingucfg.hxx>
#include <editeng/langitem.hxx>
// header for class SdrPage
#include <svx/svdpage.hxx>
//header for class SdrPageView
#include <svx/svdpagv.hxx>
// header for class SdrModel
#include <svx/svdmodel.hxx>
// header for class E3dScene
#include <svx/scene3d.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoutl.hxx>

// header for class SvxForbiddenCharactersTable
#include <editeng/forbiddencharacterstable.hxx>

#include <svx/svxids.hrc>

// header for class SvxShape
#include <svx/unoshape.hxx>
#include <editeng/fhgtitem.hxx>

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <sfx2/objsh.hxx>
#include <svx/helperhittest3d.hxx>

using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................

namespace
{
    short lcl_getHitTolerance( OutputDevice* pOutDev )
    {
        const short HITPIX=2; //hit-tolerance in pixel
        short nHitTolerance = 50;
        if(pOutDev)
            nHitTolerance = static_cast<short>(pOutDev->PixelToLogic(Size(HITPIX,0)).Width());
        return nHitTolerance;
    }

// this code is copied from sfx2/source/doc/objembed.cxx
SfxObjectShell * lcl_GetParentObjectShell( const uno::Reference< frame::XModel > & xModel )
{
    SfxObjectShell* pResult = NULL;

    try
    {
        uno::Reference< container::XChild > xChildModel( xModel, uno::UNO_QUERY );
        if ( xChildModel.is() )
        {
            uno::Reference< lang::XUnoTunnel > xParentTunnel( xChildModel->getParent(), uno::UNO_QUERY );
            if ( xParentTunnel.is() )
            {
                SvGlobalName aSfxIdent( SFX_GLOBAL_CLASSID );
                pResult = reinterpret_cast< SfxObjectShell * >(
                    xParentTunnel->getSomething( uno::Sequence< sal_Int8 >( aSfxIdent.GetByteSequence() ) ) );
            }
        }
    }
    catch( const uno::Exception& )
    {
        // TODO: error handling
    }

    return pResult;
}

// this code is copied from sfx2/source/doc/objembed.cxx.  It is a workaround to
// get the reference device (e.g. printer) fromthe parent document
OutputDevice * lcl_GetParentRefDevice( const uno::Reference< frame::XModel > & xModel )
{
    SfxObjectShell * pParent = lcl_GetParentObjectShell( xModel );
    if ( pParent )
        return pParent->GetDocumentRefDev();
    return NULL;
}

}

DrawViewWrapper::DrawViewWrapper( SdrModel* pSdrModel, OutputDevice* pOut, bool bPaintPageForEditMode)
            : E3dView(pSdrModel, pOut)
            , m_pMarkHandleProvider(NULL)
            , m_apOutliner( SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, pSdrModel ) )
            , m_bRestoreMapMode( false )
{
    SetBufferedOutputAllowed(true);
    SetBufferedOverlayAllowed(true);

    SetPagePaintingAllowed(bPaintPageForEditMode);

    // #i12587# support for shapes in chart
    SdrOutliner* pOutliner = getOutliner();
    SfxItemPool* pOutlinerPool = ( pOutliner ? pOutliner->GetEditTextObjectPool() : NULL );
    if ( pOutlinerPool )
    {
        SvtLinguConfig aLinguConfig;
        SvtLinguOptions aLinguOptions;
        if ( aLinguConfig.GetOptions( aLinguOptions ) )
        {
            pOutlinerPool->SetPoolDefaultItem( SvxLanguageItem( aLinguOptions.nDefaultLanguage, EE_CHAR_LANGUAGE ) );
            pOutlinerPool->SetPoolDefaultItem( SvxLanguageItem( aLinguOptions.nDefaultLanguage_CJK, EE_CHAR_LANGUAGE_CJK ) );
            pOutlinerPool->SetPoolDefaultItem( SvxLanguageItem( aLinguOptions.nDefaultLanguage_CTL, EE_CHAR_LANGUAGE_CTL ) );
        }

        // set font height without changing SdrEngineDefaults
        pOutlinerPool->SetPoolDefaultItem( SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT ) );  // 12pt
    }

    ReInit();
}

void DrawViewWrapper::ReInit()
{
    OutputDevice* pOutDev = this->GetFirstOutputDevice();
    Size aOutputSize(100,100);
    if(pOutDev)
        aOutputSize = pOutDev->GetOutputSize();

    bPageVisible = false;
    bPageBorderVisible = false;
    bBordVisible = false;
    bGridVisible = false;
    bHlplVisible = false;

    this->SetNoDragXorPolys(true);//for interactive 3D resize-dragging: paint only a single rectangle (not a simulated 3D object)

    //a correct work area is at least necessary for correct values in the position and  size dialog
    Rectangle aRect(Point(0,0), aOutputSize);
    this->SetWorkArea(aRect);

    this->ShowSdrPage(this->GetModel()->GetPage(0));
}

DrawViewWrapper::~DrawViewWrapper()
{
    aComeBackTimer.Stop();//@todo this should be done in destructor of base class
    UnmarkAllObj();//necessary to aavoid a paint call during the destructor hierarchy
}

SdrPageView* DrawViewWrapper::GetPageView() const
{
    SdrPageView* pSdrPageView = this->GetSdrPageView();
    return pSdrPageView;
};

void DrawViewWrapper::SetMarkHandles()
{
    if( m_pMarkHandleProvider && m_pMarkHandleProvider->getMarkHandles( aHdl ) )
        return;
    else
        SdrView::SetMarkHandles();
}

SdrObject* DrawViewWrapper::getHitObject( const Point& rPnt ) const
{
    SdrObject* pRet = NULL;
    sal_uLong nOptions = SDRSEARCH_DEEP | SDRSEARCH_TESTMARKABLE;

    SdrPageView* pSdrPageView = this->GetPageView();
    this->SdrView::PickObj(rPnt, lcl_getHitTolerance( this->GetFirstOutputDevice() ), pRet, pSdrPageView, nOptions);

    if( pRet )
    {
        //ignore some special shapes
        OUString aShapeName = pRet->GetName();
        if( aShapeName.match("PlotAreaIncludingAxes") || aShapeName.match("PlotAreaExcludingAxes") )
        {
            pRet->SetMarkProtect( true );
            return getHitObject( rPnt );
        }

        //3d objects need a special treatment
        //because the simple PickObj method is not accurate in this case for performance reasons
        E3dObject* pE3d = dynamic_cast< E3dObject* >(pRet);
        if( pE3d )
        {
            E3dScene* pScene = pE3d->GetScene();
            if( pScene )
            {
                // prepare result vector and call helper
                ::std::vector< const E3dCompoundObject* > aHitList;
                const basegfx::B2DPoint aHitPoint(rPnt.X(), rPnt.Y());
                getAllHit3DObjectsSortedFrontToBack(aHitPoint, *pScene, aHitList);

                if(!aHitList.empty())
                {
                    // choose the frontmost hit 3D object of the scene
                    pRet = const_cast< E3dCompoundObject* >(aHitList[0]);
                }
            }
        }
    }
    return pRet;
}

void DrawViewWrapper::MarkObject( SdrObject* pObj )
{
    bool bFrameDragSingles = true;//true == green == surrounding handles
    if(pObj)
        pObj->SetMarkProtect(false);
    if( m_pMarkHandleProvider )
        bFrameDragSingles = m_pMarkHandleProvider->getFrameDragSingles();

    this->SetFrameDragSingles(bFrameDragSingles);//decide whether each single object should get handles
    this->SdrView::MarkObj( pObj, this->GetPageView() );
    this->showMarkHandles();
}


void DrawViewWrapper::setMarkHandleProvider( MarkHandleProvider* pMarkHandleProvider )
{
    m_pMarkHandleProvider = pMarkHandleProvider;
}

void DrawViewWrapper::CompleteRedraw(OutputDevice* pOut, const Region& rReg, sdr::contact::ViewObjectContactRedirector* /* pRedirector */)
{
    svtools::ColorConfig aColorConfig;
    Color aFillColor = Color( aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor );
    this->SetApplicationBackgroundColor(aFillColor);
    this->E3dView::CompleteRedraw( pOut, rReg );
}

SdrObject* DrawViewWrapper::getSelectedObject() const
{
    SdrObject* pObj(NULL);
    const SdrMarkList& rMarkList = this->GetMarkedObjectList();
    if(rMarkList.GetMarkCount() == 1)
    {
        SdrMark* pMark = rMarkList.GetMark(0);
        pObj = pMark->GetMarkedSdrObj();
    }
    return pObj;
}

SdrObject* DrawViewWrapper::getTextEditObject() const
{
    SdrObject* pObj = this->getSelectedObject();
    SdrObject* pTextObj = NULL;
    if( pObj && pObj->HasTextEdit())
        pTextObj = (SdrTextObj*)pObj;
    return pTextObj;
}

void DrawViewWrapper::attachParentReferenceDevice( const uno::Reference< frame::XModel > & xChartModel )
{
    OutputDevice * pParentRefDev( lcl_GetParentRefDevice( xChartModel ));
    SdrOutliner * pOutliner( getOutliner());
    if( pParentRefDev && pOutliner )
    {
        pOutliner->SetRefDevice( pParentRefDev );
    }
}

SdrOutliner* DrawViewWrapper::getOutliner() const
{
    return m_apOutliner.get();
}

SfxItemSet DrawViewWrapper::getPositionAndSizeItemSetFromMarkedObject() const
{
    SfxItemSet aFullSet( GetModel()->GetItemPool(),
                    SID_ATTR_TRANSFORM_POS_X,SID_ATTR_TRANSFORM_ANGLE,
                    SID_ATTR_TRANSFORM_PROTECT_POS,SID_ATTR_TRANSFORM_AUTOHEIGHT,
                    SDRATTR_ECKENRADIUS,SDRATTR_ECKENRADIUS,
                    SID_ATTR_METRIC,SID_ATTR_METRIC,
                    0);
    SfxItemSet aGeoSet( E3dView::GetGeoAttrFromMarked() );
    aFullSet.Put( aGeoSet );
    aFullSet.Put( SfxUInt16Item(SID_ATTR_METRIC,static_cast< sal_uInt16 >( ConfigurationAccess::getFieldUnit())));
    return aFullSet;
}

SdrObject* DrawViewWrapper::getNamedSdrObject( const OUString& rName ) const
{
    if(rName.isEmpty())
        return 0;
    SdrPageView* pSdrPageView = this->GetPageView();
    if( pSdrPageView )
    {
        return DrawModelWrapper::getNamedSdrObject( rName, pSdrPageView->GetObjList() );
    }
    return 0;
}

bool DrawViewWrapper::IsObjectHit( SdrObject* pObj, const Point& rPnt ) const
{
    if(pObj)
    {
        Rectangle aRect(pObj->GetCurrentBoundRect());
        return aRect.IsInside(rPnt);
    }
    return false;
}

void DrawViewWrapper::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    //prevent wrong reselection of objects
    SdrModel* pSdrModel( this->GetModel() );
    if( pSdrModel && pSdrModel->isLocked() )
        return;

    const SdrHint* pSdrHint = dynamic_cast< const SdrHint* >( &rHint );

    //#i76053# do nothing when only changes on the hidden draw page were made ( e.g. when the symbols for the dialogs are created )
    SdrPageView* pSdrPageView = this->GetPageView();
    if( pSdrHint && pSdrPageView )
    {
        if( pSdrPageView->GetPage() != pSdrHint->GetPage() )
            return;
    }

    E3dView::Notify(rBC, rHint);

    if( pSdrHint != 0 )
    {
        SdrHintKind eKind = pSdrHint->GetKind();
        if( eKind == HINT_BEGEDIT )
        {
            // #i79965# remember map mode
            OSL_ASSERT( ! m_bRestoreMapMode );
            OutputDevice* pOutDev = this->GetFirstOutputDevice();
            if( pOutDev )
            {
                m_aMapModeToRestore = pOutDev->GetMapMode();
                m_bRestoreMapMode = true;
            }
        }
        else if( eKind == HINT_ENDEDIT )
        {
            // #i79965# scroll back view when ending text edit
            OSL_ASSERT( m_bRestoreMapMode );
            if( m_bRestoreMapMode )
            {
                OutputDevice* pOutDev = this->GetFirstOutputDevice();
                if( pOutDev )
                {
                    pOutDev->SetMapMode( m_aMapModeToRestore );
                    m_bRestoreMapMode = false;
                }
            }
        }
    }
}

SdrObject* DrawViewWrapper::getSdrObject( const uno::Reference<
                    drawing::XShape >& xShape )
{
    SdrObject* pRet = 0;
    uno::Reference< lang::XUnoTunnel > xUnoTunnel( xShape, uno::UNO_QUERY );
    uno::Reference< lang::XTypeProvider > xTypeProvider( xShape, uno::UNO_QUERY );
    if(xUnoTunnel.is()&&xTypeProvider.is())
    {
        SvxShape* pSvxShape = reinterpret_cast<SvxShape*>(xUnoTunnel->getSomething( SvxShape::getUnoTunnelId() ));
        if(pSvxShape)
            pRet = pSvxShape->GetSdrObject();
    }
    return pRet;
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
