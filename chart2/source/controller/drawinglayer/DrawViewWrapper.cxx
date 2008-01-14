/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DrawViewWrapper.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:57:18 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "DrawViewWrapper.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "ConfigurationAccess.hxx"

// header for class SdrPage
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
//header for class SdrPageView
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif
// header for class SdrModel
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
// header for class E3dScene
#ifndef _E3D_SCENE3D_HXX
#include <svx/scene3d.hxx>
#endif

#ifndef _SVDETC_HXX
#include <svx/svdetc.hxx>
#endif

#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif

// header for class SvxForbiddenCharactersTable
#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <svx/forbiddencharacterstable.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

// header for class SvxShape
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <sfx2/objsh.hxx>

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
    catch( uno::Exception& )
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

    /*
void lcl_initOutliner( SdrOutliner* pTargetOutliner, SdrOutliner* pSourceOutliner )
{
    //just an unsuccessful try to initialize the text edit outliner correctly
    //if( bInit )
    {
        pTargetOutliner->EraseVirtualDevice();
        pTargetOutliner->SetUpdateMode(FALSE);
        pTargetOutliner->SetEditTextObjectPool( pSourceOutliner->GetEditTextObjectPool() );
        pTargetOutliner->SetDefTab( pSourceOutliner->GetDefTab() );
    }

    pTargetOutliner->SetRefDevice( pSourceOutliner->GetRefDevice() );
    pTargetOutliner->SetForbiddenCharsTable( pSourceOutliner->GetForbiddenCharsTable() );
    pTargetOutliner->SetAsianCompressionMode( pSourceOutliner->GetAsianCompressionMode() );
    pTargetOutliner->SetKernAsianPunctuation( pSourceOutliner->IsKernAsianPunctuation() );
    pTargetOutliner->SetStyleSheetPool( pSourceOutliner->GetStyleSheetPool() );
    pTargetOutliner->SetRefMapMode( pSourceOutliner->GetRefMapMode() );
    pTargetOutliner->SetDefaultLanguage( pSourceOutliner->GetDefaultLanguage() );
    pTargetOutliner->SetHyphenator( pSourceOutliner->GetHyphenator() );

    USHORT nX, nY;
    pSourceOutliner->GetGlobalCharStretching( nX, nY );
    pTargetOutliner->SetGlobalCharStretching( nX, nY );

    *//*
    if ( !GetRefDevice() )
    {
        MapMode aMapMode(eObjUnit, Point(0,0), aObjUnit, aObjUnit);
        pTargetOutliner->SetRefMapMode(aMapMode);
    }
    *//*
}
*/

DrawViewWrapper::DrawViewWrapper( SdrModel* pSdrModel, OutputDevice* pOut, bool bPaintPageForEditMode)
            : E3dView(pSdrModel, pOut)
            , m_pMarkHandleProvider(NULL)
            , m_apOutliner( SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, pSdrModel ) )
            , m_bRestoreMapMode( false )
{
    // #114898#
    SetBufferedOutputAllowed(true);
    SetBufferedOverlayAllowed(true);

    SetPagePaintingAllowed(bPaintPageForEditMode);

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
    //this->SetResizeAtCenter(true);//for interactive resize-dragging: keep the object center fix

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

//virtual
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
    //ULONG nOptions =SDRSEARCH_DEEP|SDRSEARCH_PASS2BOUND|SDRSEARCH_PASS3NEAREST;
    ULONG nOptions = SDRSEARCH_DEEP | SDRSEARCH_TESTMARKABLE;

    SdrPageView* pSdrPageView = this->GetPageView();
    this->SdrView::PickObj(rPnt, lcl_getHitTolerance( this->GetFirstOutputDevice() ), pRet, pSdrPageView, nOptions);

    if( pRet )
    {
        //3d objects need a special treatment
        //because the simple PickObj method is not accurate in this case for performance reasons
        E3dObject* pE3d = dynamic_cast< E3dObject* >(pRet);
        if( pE3d )
        {
            E3dScene* pScene = pE3d->GetScene();
            if( pScene )
            {
                ::std::vector< SdrObject* > aHitList;
                sal_uInt32 nHitCount = pScene->HitTest( rPnt, aHitList );
                if( nHitCount )
                    pRet = aHitList[0];
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

    this->SetFrameDragSingles(bFrameDragSingles);//decide wether each single object should get handles
    this->SdrView::MarkObj( pObj, this->GetPageView() );
    this->showMarkHandles();
}


void DrawViewWrapper::setMarkHandleProvider( MarkHandleProvider* pMarkHandleProvider )
{
    m_pMarkHandleProvider = pMarkHandleProvider;
}

void DrawViewWrapper::CompleteRedraw(
    OutputDevice* pOut, const Region& rReg, USHORT /* nPaintMode */,
    ::sdr::contact::ViewObjectContactRedirector* /* pRedirector */ )
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
//    lcl_initOutliner( m_apOutliner.get(), &GetModel()->GetDrawOutliner() );
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
    aFullSet.Put( SfxUInt16Item(SID_ATTR_METRIC,static_cast< sal_uInt16 >( ConfigurationAccess::getConfigurationAccess()->getFieldUnit())));
    return aFullSet;
}

SdrObject* DrawViewWrapper::getNamedSdrObject( const rtl::OUString& rName ) const
{
    if(rName.getLength()==0)
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

void DrawViewWrapper::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
{
    //prevent wrong reselection of objects
    SdrModel* pSdrModel( this->GetModel() );
    if( pSdrModel && pSdrModel->isLocked() )
        return;

    E3dView::SFX_NOTIFY(rBC, rBCType, rHint, rHintType);

    const SdrHint* pSdrHint = dynamic_cast< const SdrHint* >( &rHint );
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

//static
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
