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

#include <comphelper/processfactory.hxx>
#include <cppuhelper/proptypehlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdpagv.hxx>
#include <svx/unoshape.hxx>
#include <sfx2/zoomitem.hxx>
#include <com/sun/star/drawing/DrawViewMode.hpp>
#include <com/sun/star/drawing/ShapeCollection.hpp>
#include <com/sun/star/drawing/XLayerManager.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>

#include <vector>

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

bool SdUnoDrawView::getMasterPageMode() const throw()
{
    return (mrDrawViewShell.GetEditMode() == EM_MASTERPAGE);
}

void SdUnoDrawView::setMasterPageMode (bool bMasterPageMode) throw()
{
    if ((mrDrawViewShell.GetEditMode() == EM_MASTERPAGE) != bMasterPageMode)
    {
        mrDrawViewShell.ChangeEditMode (
            bMasterPageMode ? EM_MASTERPAGE : EM_PAGE,
            mrDrawViewShell.IsLayerModeActive());
    }
}

bool SdUnoDrawView::getLayerMode() const throw()
{
    return mrDrawViewShell.IsLayerModeActive();
}

void SdUnoDrawView::setLayerMode (bool bLayerMode) throw()
{
    if (mrDrawViewShell.IsLayerModeActive() != bLayerMode)
    {
        mrDrawViewShell.ChangeEditMode (
            mrDrawViewShell.GetEditMode(),
            bLayerMode);
    }
}

Reference<drawing::XLayer> SdUnoDrawView::getActiveLayer() throw ()
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
        SdrLayer* pLayer = rLayerAdmin.GetLayer (mrView.GetActiveLayer(), true);
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

void SdUnoDrawView::setActiveLayer (const Reference<drawing::XLayer>& rxLayer)
    throw (uno::RuntimeException, std::exception)
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
    throw (lang::IllegalArgumentException,
           RuntimeException,
           std::exception)
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

Any SAL_CALL SdUnoDrawView::getSelection()
    throw (RuntimeException, std::exception)
{
    Any aAny;

    if( mrView.IsTextEdit() )
        mrView.getTextSelection( aAny );

    if( !aAny.hasValue() )
    {
        const SdrMarkList& rMarkList = mrView.GetMarkedObjectList();
        const size_t nCount = rMarkList.GetMarkCount();
        if( nCount )
        {
            Reference< drawing::XShapes > xShapes( drawing::ShapeCollection::create(
                        comphelper::getProcessComponentContext()), UNO_QUERY );
            for( size_t nNum = 0; nNum < nCount; ++nNum)
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
    throw(css::uno::RuntimeException, std::exception)
{
    (void)rxListener;
}

void SAL_CALL SdUnoDrawView::removeSelectionChangeListener (
    const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
    throw(css::uno::RuntimeException, std::exception)
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
        css::uno::RuntimeException, std::exception)
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
                bool bValue = false;
                rValue >>= bValue;
                setMasterPageMode( bValue );
            }
            break;

        case DrawController::PROPERTY_LAYERMODE:
            {
                bool bValue = false;
                rValue >>= bValue;
                setLayerMode( bValue );
            }
            break;
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
        css::uno::RuntimeException, std::exception)
{
    Any aValue;
    switch( nHandle )
    {
        case DrawController::PROPERTY_CURRENTPAGE:
            aValue <<= getCurrentPage();
            break;

        case DrawController::PROPERTY_MASTERPAGEMODE:
            aValue <<= getMasterPageMode();
            break;

        case DrawController::PROPERTY_LAYERMODE:
            aValue <<= getLayerMode();
            break;

        case DrawController::PROPERTY_ACTIVE_LAYER:
            aValue <<= getActiveLayer();
            break;

        case DrawController::PROPERTY_ZOOMVALUE:
            aValue <<= GetZoom();
            break;
        case DrawController::PROPERTY_ZOOMTYPE:
            aValue <<= (sal_Int16)css::view::DocumentZoomType::BY_VALUE;
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
    throw (RuntimeException, std::exception)
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

Reference< drawing::XDrawPage > SAL_CALL SdUnoDrawView::getCurrentPage()
    throw(RuntimeException, std::exception)
{
    Reference< drawing::XDrawPage >  xPage;

    SdrPageView *pPV = mrView.GetSdrPageView();
    SdrPage* pPage = pPV ? pPV->GetPage() : NULL;

    if(pPage)
        xPage = Reference< drawing::XDrawPage >::query( pPage->getUnoPage() );

    return xPage;
}

sal_Int16 SdUnoDrawView::GetZoom() const
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
    SvxZoomItem aZoomItem( SvxZoomType::PERCENT, nZoom );

    SfxViewFrame* pViewFrame = mrDrawViewShell.GetViewFrame();
    if( pViewFrame )
    {
        SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();
        if( pDispatcher )
        {
            pDispatcher->Execute(SID_ATTR_ZOOM,SfxCallMode::SYNCHRON,&aZoomItem, 0L);
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
                case css::view::DocumentZoomType::OPTIMAL:
                    eZoomType = SvxZoomType::OPTIMAL;
                    break;

                case css::view::DocumentZoomType::PAGE_WIDTH:
                case css::view::DocumentZoomType::PAGE_WIDTH_EXACT:
                    eZoomType = SvxZoomType::PAGEWIDTH;
                    break;

                case css::view::DocumentZoomType::ENTIRE_PAGE:
                    eZoomType = SvxZoomType::WHOLEPAGE;
                    break;

                default:
                    return;
            }
            SvxZoomItem aZoomItem( eZoomType );
            pDispatcher->Execute(SID_ATTR_ZOOM,SfxCallMode::SYNCHRON,&aZoomItem, 0L);
        }
    }
}

SdXImpressDocument* SdUnoDrawView::GetModel() const throw()
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
OUString SAL_CALL SdUnoDrawView::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return OUString( "com.sun.star.comp.sd.SdUnoDrawView") ;
}

sal_Bool SAL_CALL SdUnoDrawView::supportsService( const OUString& ServiceName ) throw (RuntimeException, std::exception)
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL SdUnoDrawView::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    OUString aSN("com.sun.star.drawing.DrawingDocumentDrawView");
    uno::Sequence< OUString > aSeq( &aSN, 1 );
    return aSeq;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
