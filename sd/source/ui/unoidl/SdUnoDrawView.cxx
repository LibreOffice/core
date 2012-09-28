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


#include "SdUnoDrawView.hxx"

#include "DrawController.hxx"
#include "DrawDocShell.hxx"
#include "DrawViewShell.hxx"
#include "drawdoc.hxx"
#include "unolayer.hxx"
#include "unomodel.hxx"
#include "unopage.hxx"
#include "Window.hxx"
#include "pres.hxx"

#include <cppuhelper/proptypehlp.hxx>
#include <comphelper/serviceinfohelper.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdpagv.hxx>
#include <svx/unoshape.hxx>
#include <svx/unoshcol.hxx>
#include <sfx2/zoomitem.hxx>
#include <com/sun/star/drawing/DrawViewMode.hpp>
#include <com/sun/star/drawing/XLayerManager.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>

#include <vector>

using rtl::OUString;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;


namespace sd {

SdUnoDrawView::SdUnoDrawView(
    DrawViewShell& rViewShell,
    View& rView) throw()
    :   DrawSubControllerInterfaceBase(m_aMutex),
        mrDrawViewShell(rViewShell),
        mrView(rView)
{
}




SdUnoDrawView::~SdUnoDrawView() throw()
{
}




sal_Bool SdUnoDrawView::getMasterPageMode(void) const throw()
{
    return (mrDrawViewShell.GetEditMode() == EM_MASTERPAGE);
}




void SdUnoDrawView::setMasterPageMode (sal_Bool bMasterPageMode) throw()
{
    if ((mrDrawViewShell.GetEditMode() == EM_MASTERPAGE) != bMasterPageMode)
    {
        mrDrawViewShell.ChangeEditMode (
            bMasterPageMode ? EM_MASTERPAGE : EM_PAGE,
            mrDrawViewShell.IsLayerModeActive());
    }
}




sal_Bool SdUnoDrawView::getLayerMode(void) const throw()
{
    return mrDrawViewShell.IsLayerModeActive();
}




void SdUnoDrawView::setLayerMode (sal_Bool bLayerMode) throw()
{
    if (mrDrawViewShell.IsLayerModeActive() != (bLayerMode==sal_True))
    {
        mrDrawViewShell.ChangeEditMode (
            mrDrawViewShell.GetEditMode(),
            bLayerMode);
    }
}




Reference<drawing::XLayer> SdUnoDrawView::getActiveLayer (void) throw ()
{
    Reference<drawing::XLayer> xCurrentLayer;

    do
    {
        // Retrieve the layer manager from the model.
        SdXImpressDocument* pModel = GetModel();
        if (pModel == NULL)
            break;

        SdDrawDocument* pSdModel = pModel->GetDoc();
        if (pSdModel == NULL)
            break;

        // From the model get the current SdrLayer object via the layer admin.
        SdrLayerAdmin& rLayerAdmin = pSdModel->GetLayerAdmin ();
        SdrLayer* pLayer = rLayerAdmin.GetLayer (mrView.GetActiveLayer(), sal_True);
        if (pLayer == NULL)
            break;

        // Get the corresponding XLayer object from the implementation
        // object of the layer manager.
        Reference<drawing::XLayerManager> xManager (pModel->getLayerManager(), uno::UNO_QUERY);
        SdLayerManager* pManager = SdLayerManager::getImplementation (xManager);
        if (pManager != NULL)
            xCurrentLayer = pManager->GetLayer (pLayer);
    }
    while (false);

    return xCurrentLayer;
}




void SdUnoDrawView::setActiveLayer (const Reference<drawing::XLayer>& rxLayer) throw ()
{
    // Get the SdrLayer object corresponding to the given reference.
    if ( ! rxLayer.is())
        return;

    SdLayer* pLayer = SdLayer::getImplementation (rxLayer);
    if (pLayer == NULL)
        return;

    SdrLayer* pSdrLayer = pLayer->GetSdrLayer();
    if (pSdrLayer == NULL)
        return;

    // Set the new active layer and make the change visible.
    mrView.SetActiveLayer (pSdrLayer->GetName());
    mrDrawViewShell.ResetActualLayer ();
}




// XSelectionSupplier


sal_Bool SAL_CALL SdUnoDrawView::select( const Any& aSelection )
    throw(lang::IllegalArgumentException, RuntimeException)
{
    bool bOk = true;

    ::std::vector<SdrObject*> aObjects;

    SdrPage* pSdrPage = NULL;

    Reference< drawing::XShape > xShape;
    aSelection >>= xShape;

    if(xShape.is())
    {
        SvxShape* pShape = SvxShape::getImplementation( xShape );
        if( pShape && (pShape->GetSdrObject() != NULL) )
        {
            SdrObject* pObj = pShape->GetSdrObject();
            pSdrPage = pObj->GetPage();
            aObjects.push_back( pObj );
        }
        else
        {
            bOk = false;
        }
    }
    else
    {
        Reference< drawing::XShapes > xShapes;
        aSelection >>= xShapes;
        if( xShapes.is() )
        {
            const sal_uInt32 nCount = xShapes->getCount();
            for( sal_uInt32 i = 0; i < nCount; i++ )
            {
                xShapes->getByIndex(i) >>= xShape;
                if( xShape.is() )
                {
                    SvxShape* pShape = SvxShape::getImplementation(xShape);
                    if( (pShape == NULL) || (pShape->GetSdrObject() == NULL) )
                    {
                        bOk = false;
                        break;
                    }

                    SdrObject* pObj = pShape->GetSdrObject();

                    if( pSdrPage == NULL )
                    {
                        pSdrPage = pObj->GetPage();
                    }
                    else if( pSdrPage != pObj->GetPage() )
                    {
                        bOk = false;
                        break;
                    }

                    aObjects.push_back( pObj );
                }
            }
        }
    }

    if( bOk )
    {
        if( pSdrPage )
        {
            setMasterPageMode( pSdrPage->IsMasterPage() );
            mrDrawViewShell.SwitchPage( (pSdrPage->GetPageNum() - 1) >> 1 );
            mrDrawViewShell.WriteFrameViewData();
        }

        SdrPageView *pPV = mrView.GetSdrPageView();

        if(pPV)
        {
            // first deselect all
            mrView.UnmarkAllObj( pPV );

            ::std::vector<SdrObject*>::iterator aIter( aObjects.begin() );
            const ::std::vector<SdrObject*>::iterator aEnd( aObjects.end() );
            while( aIter != aEnd )
            {
                SdrObject* pObj = (*aIter++);
                mrView.MarkObj( pObj, pPV );
            }
        }
        else
        {
            bOk = false;
        }
    }

    return bOk;
}

//----------------------------------------------------------------------

Any SAL_CALL SdUnoDrawView::getSelection()
    throw(RuntimeException)
{
    Any aAny;

    if( mrView.IsTextEdit() )
        mrView.getTextSelection( aAny );


    if( !aAny.hasValue() )
    {
        const SdrMarkList& rMarkList = mrView.GetMarkedObjectList();
        sal_uInt32 nCount = rMarkList.GetMarkCount();
        if( nCount )
        {
            Reference< drawing::XShapes > xShapes( SvxShapeCollection_NewInstance(), UNO_QUERY );
            for( sal_uInt32 nNum = 0; nNum < nCount; nNum++)
            {
                SdrMark *pMark = rMarkList.GetMark(nNum);
                if(pMark==NULL)
                    continue;

                SdrObject *pObj = pMark->GetMarkedSdrObj();
                if(pObj==NULL || pObj->GetPage() == NULL)
                    continue;

                Reference< drawing::XDrawPage > xPage( pObj->GetPage()->getUnoPage(), UNO_QUERY);

                if(!xPage.is())
                    continue;

                SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation( xPage );

                if(pDrawPage==NULL)
                    continue;

                Reference< drawing::XShape > xShape( pObj->getUnoShape(), UNO_QUERY );

                if(xShape.is())
                    xShapes->add(xShape);
            }
            aAny <<= xShapes;
        }
    }

    return aAny;
}




void SAL_CALL SdUnoDrawView::addSelectionChangeListener (
    const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
    throw(css::uno::RuntimeException)
{
    (void)rxListener;
}




void SAL_CALL SdUnoDrawView::removeSelectionChangeListener (
    const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
    throw(css::uno::RuntimeException)
{
    (void)rxListener;
}




void SdUnoDrawView::setFastPropertyValue (
    sal_Int32 nHandle,
        const Any& rValue)
    throw(css::beans::UnknownPropertyException,
        css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    switch( nHandle )
    {
        case DrawController::PROPERTY_CURRENTPAGE:
            {
                Reference< drawing::XDrawPage > xPage;
                rValue >>= xPage;
                setCurrentPage( xPage );
            }
            break;

        case DrawController::PROPERTY_MASTERPAGEMODE:
            {
                sal_Bool bValue = sal_False;
                rValue >>= bValue;
                setMasterPageMode( bValue );
            }
            break;

        case DrawController::PROPERTY_LAYERMODE:
            {
                sal_Bool bValue = sal_False;
                rValue >>= bValue;
                setLayerMode( bValue );
            }

        case DrawController::PROPERTY_ACTIVE_LAYER:
            {
                Reference<drawing::XLayer> xLayer;
                rValue >>= xLayer;
                setActiveLayer (xLayer);
            }
            break;
        case DrawController::PROPERTY_ZOOMVALUE:
            {
                sal_Int16 nZoom = 0;
                rValue >>= nZoom;
                SetZoom( nZoom );
            }
            break;
        case DrawController::PROPERTY_ZOOMTYPE:
            {
                sal_Int16 nType = 0;
                rValue >>= nType;
                SetZoomType( nType );
            }
            break;
        case DrawController::PROPERTY_VIEWOFFSET:
            {
                awt::Point aOffset;
                rValue >>= aOffset;
                SetViewOffset( aOffset );
            }
            break;
        default:
            throw beans::UnknownPropertyException();
    }
}




Any SAL_CALL SdUnoDrawView::getFastPropertyValue (
    sal_Int32 nHandle)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    Any aValue;
    switch( nHandle )
    {
        case DrawController::PROPERTY_CURRENTPAGE:
            aValue <<= (const_cast<SdUnoDrawView*>(this))->getCurrentPage();
            break;

        case DrawController::PROPERTY_MASTERPAGEMODE:
            aValue <<= getMasterPageMode();
            break;

        case DrawController::PROPERTY_LAYERMODE:
            aValue <<= getLayerMode();
            break;

        case DrawController::PROPERTY_ACTIVE_LAYER:
            aValue <<= (const_cast<SdUnoDrawView*>(this))->getActiveLayer();
            break;

        case DrawController::PROPERTY_ZOOMVALUE:
            aValue <<= GetZoom();
            break;
        case DrawController::PROPERTY_ZOOMTYPE:
            aValue <<= (sal_Int16)com::sun::star::view::DocumentZoomType::BY_VALUE;
            break;
        case DrawController::PROPERTY_VIEWOFFSET:
            aValue <<= GetViewOffset();
            break;

        case DrawController::PROPERTY_DRAWVIEWMODE:
            aValue = getDrawViewMode();
            break;

        default:
            throw beans::UnknownPropertyException();
    }

    return aValue;
}




// XDrawView


void SAL_CALL SdUnoDrawView::setCurrentPage (
    const Reference< drawing::XDrawPage >& xPage )
    throw(RuntimeException)
{
    SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation( xPage );
    SdrPage *pSdrPage = pDrawPage ? pDrawPage->GetSdrPage() : NULL;

    if(pSdrPage)
    {
        // End editing of text.  Otherwise the edited text object would
        // still be visible on the new page.
        mrDrawViewShell.GetView()->SdrEndTextEdit();

        setMasterPageMode( pSdrPage->IsMasterPage() );
        mrDrawViewShell.SwitchPage( (pSdrPage->GetPageNum() - 1) >> 1 );
        mrDrawViewShell.WriteFrameViewData();
    }
}

//----------------------------------------------------------------------

Reference< drawing::XDrawPage > SAL_CALL SdUnoDrawView::getCurrentPage()
    throw(RuntimeException)
{
    Reference< drawing::XDrawPage >  xPage;

    SdrPageView *pPV = mrView.GetSdrPageView();
    SdrPage* pPage = pPV ? pPV->GetPage() : NULL;

    if(pPage)
        xPage = Reference< drawing::XDrawPage >::query( pPage->getUnoPage() );

    return xPage;
}


sal_Int16 SdUnoDrawView::GetZoom(void) const
{
    if (mrDrawViewShell.GetActiveWindow() )
    {
        return (sal_Int16)mrDrawViewShell.GetActiveWindow()->GetZoom();
    }
    else
    {
        return 0;
    }
}

void SdUnoDrawView::SetZoom( sal_Int16 nZoom )
{
    SvxZoomItem aZoomItem( SVX_ZOOM_PERCENT, nZoom );

    SfxViewFrame* pViewFrame = mrDrawViewShell.GetViewFrame();
    if( pViewFrame )
    {
        SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();
        if( pDispatcher )
        {
            pDispatcher->Execute(SID_ATTR_ZOOM,SFX_CALLMODE_SYNCHRON,&aZoomItem, 0L);
        }
    }
}


void SdUnoDrawView::SetViewOffset(const awt::Point& rWinPos )
{
    Point aWinPos( rWinPos.X, rWinPos.Y );
    aWinPos += mrDrawViewShell.GetViewOrigin();
    mrDrawViewShell.SetWinViewPos( aWinPos, true );
}

awt::Point SdUnoDrawView::GetViewOffset() const
{
    Point aRet;

    aRet = mrDrawViewShell.GetWinViewPos();
    aRet -= mrDrawViewShell.GetViewOrigin();

    return awt::Point( aRet.X(), aRet.Y() );
}

void SdUnoDrawView::SetZoomType ( sal_Int16 nType )
{
    SfxViewFrame* pViewFrame = mrDrawViewShell.GetViewFrame();
    if( pViewFrame )
    {
        SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();
        if( pDispatcher )
        {
            SvxZoomType eZoomType;
            switch( nType )
            {
                case com::sun::star::view::DocumentZoomType::OPTIMAL:
                    eZoomType = SVX_ZOOM_OPTIMAL;
                    break;

                case com::sun::star::view::DocumentZoomType::PAGE_WIDTH:
                case com::sun::star::view::DocumentZoomType::PAGE_WIDTH_EXACT:
                    eZoomType = SVX_ZOOM_PAGEWIDTH;
                    break;

                case com::sun::star::view::DocumentZoomType::ENTIRE_PAGE:
                    eZoomType = SVX_ZOOM_WHOLEPAGE;
                    break;

                default:
                    return;
            }
            SvxZoomItem aZoomItem( eZoomType );
            pDispatcher->Execute(SID_ATTR_ZOOM,SFX_CALLMODE_SYNCHRON,&aZoomItem, 0L);
        }
    }
}




SdXImpressDocument* SdUnoDrawView::GetModel (void) const throw()
{
    if (mrView.GetDocSh()!=NULL)
    {
        Reference<frame::XModel> xModel (mrView.GetDocSh()->GetModel());
        return SdXImpressDocument::getImplementation(xModel);
    }
    else
        return NULL;
}

Any SdUnoDrawView::getDrawViewMode() const
{
    Any aRet;
    switch( mrDrawViewShell.GetPageKind() )
    {
    case PK_NOTES:  aRet <<= DrawViewMode_NOTES; break;
    case PK_HANDOUT: aRet <<= DrawViewMode_HANDOUT; break;
    case PK_STANDARD: aRet <<= DrawViewMode_DRAW; break;
    }
    return aRet;
}

// XServiceInfo
OUString SAL_CALL SdUnoDrawView::getImplementationName(  ) throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.sd.SdUnoDrawView") ;
}

sal_Bool SAL_CALL SdUnoDrawView::supportsService( const OUString& ServiceName ) throw (RuntimeException)
{
    return comphelper::ServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

Sequence< OUString > SAL_CALL SdUnoDrawView::getSupportedServiceNames(  ) throw (RuntimeException)
{
    OUString aSN("com.sun.star.drawing.DrawingDocumentDrawView");
    uno::Sequence< OUString > aSeq( &aSN, 1 );
    return aSeq;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
