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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include "unolayer.hxx"

#include <comphelper/extract.hxx>
#include <svx/svdpagv.hxx>
#include <svx/unoshape.hxx>
#include <svx/svdobj.hxx>
#include <cppuhelper/supportsservice.hxx>

// following ones for InsertSdPage()
#include <svx/svdlayer.hxx>

#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <unomodel.hxx>
#include <unoprnms.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <svx/svdpool.hxx>
#include <FrameView.hxx>
#include <DrawViewShell.hxx>
#include <View.hxx>
#include <ViewShell.hxx>
#include <app.hrc>
#include <strings.hrc>
#include <sdresid.hxx>

#include <unokywds.hxx>
#include "unowcntr.hxx"
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

// class SdLayer
#define WID_LAYER_LOCKED    1
#define WID_LAYER_PRINTABLE 2
#define WID_LAYER_VISIBLE   3
#define WID_LAYER_NAME      4
#define WID_LAYER_TITLE     5
#define WID_LAYER_DESC      6

static const SvxItemPropertySet* ImplGetSdLayerPropertySet()
{
    static const SfxItemPropertyMapEntry aSdLayerPropertyMap_Impl[] =
    {
        { OUString(UNO_NAME_LAYER_LOCKED),      WID_LAYER_LOCKED,   cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UNO_NAME_LAYER_PRINTABLE),   WID_LAYER_PRINTABLE,cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UNO_NAME_LAYER_VISIBLE),     WID_LAYER_VISIBLE,  cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UNO_NAME_LAYER_NAME),        WID_LAYER_NAME,     ::cppu::UnoType<OUString>::get(), 0, 0 },
        { OUString("Title"),                    WID_LAYER_TITLE,    ::cppu::UnoType<OUString>::get(), 0, 0 },
        { OUString("Description"),              WID_LAYER_DESC,     ::cppu::UnoType<OUString>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SvxItemPropertySet aSDLayerPropertySet_Impl( aSdLayerPropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
    return &aSDLayerPropertySet_Impl;
}

SdLayer::SdLayer(SdLayerManager* pLayerManager_, SdrLayer* pSdrLayer_)
: mxLayerManager(pLayerManager_)
, pLayer(pSdrLayer_)
, pPropSet(ImplGetSdLayerPropertySet())
{
    // no defaults possible yet, a "set" would overwrite existing information
    // in view, which is currently needed for saving, because pLayer is not updated
    // from view.
}

SdLayer::~SdLayer() throw()
{
}

// uno helper
UNO3_GETIMPLEMENTATION_IMPL( SdLayer );

// XServiceInfo
OUString SAL_CALL SdLayer::getImplementationName()
{
    return OUString("SdUnoLayer");
}

sal_Bool SAL_CALL SdLayer::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SdLayer::getSupportedServiceNames()
{
    OUString aServiceName("com.sun.star.drawing.Layer");
    uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

// beans::XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SdLayer::getPropertySetInfo(  )
{
    SolarMutexGuard aGuard;
    return pPropSet->getPropertySetInfo();
}

void SAL_CALL SdLayer::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    if(pLayer == nullptr || mxLayerManager == nullptr)
        throw lang::DisposedException();

    const SfxItemPropertySimpleEntry* pEntry = pPropSet->getPropertyMapEntry(aPropertyName);

    switch( pEntry ? pEntry->nWID : -1 )
    {
    case WID_LAYER_LOCKED:
    {
        pLayer->SetLockedODF( cppu::any2bool(aValue) );
        set(LOCKED, cppu::any2bool(aValue)); // changes the View, if any exists
        break;
    }
    case WID_LAYER_PRINTABLE:
    {
        pLayer->SetPrintableODF( cppu::any2bool(aValue) );
        set(PRINTABLE, cppu::any2bool(aValue)); // changes the View, if any exists
        break;
    }
    case WID_LAYER_VISIBLE:
    {
        pLayer->SetVisibleODF( cppu::any2bool(aValue) );
        set(VISIBLE, cppu::any2bool(aValue)); // changes the View, if any exists
        break;
    }
    case WID_LAYER_NAME:
    {
        OUString aName;
        if(!(aValue >>= aName))
            throw lang::IllegalArgumentException();

        pLayer->SetName(aName);
        mxLayerManager->UpdateLayerView();
        break;
    }

    case WID_LAYER_TITLE:
    {
        OUString sTitle;
        if(!(aValue >>= sTitle))
            throw lang::IllegalArgumentException();

        pLayer->SetTitle(sTitle);
        break;
    }

    case WID_LAYER_DESC:
    {
        OUString sDescription;
        if(!(aValue >>= sDescription))
            throw lang::IllegalArgumentException();

        pLayer->SetDescription(sDescription);
        break;
    }

    default:
        throw beans::UnknownPropertyException( aPropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    if( mxLayerManager->GetDocShell() )
        mxLayerManager->GetDocShell()->SetModified();
}

uno::Any SAL_CALL SdLayer::getPropertyValue( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    if(pLayer == nullptr || mxLayerManager == nullptr)
        throw lang::DisposedException();

    const SfxItemPropertySimpleEntry* pEntry = pPropSet->getPropertyMapEntry(PropertyName);

    uno::Any aValue;

    switch( pEntry ? pEntry->nWID : -1 )
    {
    case WID_LAYER_LOCKED:
        aValue <<= get( LOCKED );
        break;
    case WID_LAYER_PRINTABLE:
        aValue <<= get( PRINTABLE );
        break;
    case WID_LAYER_VISIBLE:
        aValue <<= get( VISIBLE );
        break;
    case WID_LAYER_NAME:
    {
        OUString aRet(pLayer->GetName());
        aValue <<= aRet;
        break;
    }
    case WID_LAYER_TITLE:
        aValue <<= pLayer->GetTitle();
        break;
    case WID_LAYER_DESC:
        aValue <<= pLayer->GetDescription();
        break;
    default:
        throw beans::UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    return aValue;
}

void SAL_CALL SdLayer::addPropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >& ) {}
void SAL_CALL SdLayer::removePropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >& ) {}
void SAL_CALL SdLayer::addVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >& ) {}
void SAL_CALL SdLayer::removeVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >& ) {}

bool SdLayer::get( LayerAttribute what ) throw()
{
    if(pLayer && mxLayerManager.is())
    {
        // Try 1. is an arbitrary page open?
        ::sd::View *pView = mxLayerManager->GetView();
        SdrPageView* pSdrPageView = nullptr;
        if(pView)
            pSdrPageView = pView->GetSdrPageView();

        if(pSdrPageView)
        {
            OUString aLayerName = pLayer->GetName();
            switch(what)
            {
            case VISIBLE:   return pSdrPageView->IsLayerVisible(aLayerName);
            case PRINTABLE: return pSdrPageView->IsLayerPrintable(aLayerName);
            case LOCKED:    return pSdrPageView->IsLayerLocked(aLayerName);
            }
        }

        // Try 2. get info from FrameView
        if(mxLayerManager->GetDocShell())
        {
            ::sd::FrameView *pFrameView = mxLayerManager->GetDocShell()->GetFrameView();
            if(pFrameView)
                switch(what)
                {
                case VISIBLE:   return pFrameView->GetVisibleLayers().IsSet(pLayer->GetID());
                case PRINTABLE: return pFrameView->GetPrintableLayers().IsSet(pLayer->GetID());
                case LOCKED:    return pFrameView->GetLockedLayers().IsSet(pLayer->GetID());
                }
        }
    }
    return false; //TODO: uno::Exception?
}

void SdLayer::set( LayerAttribute what, bool flag ) throw()
{
    if(!(pLayer && mxLayerManager.is()))
        return;

    // Try 1. is an arbitrary page open?
    ::sd::View *pView = mxLayerManager->GetView();
    SdrPageView* pSdrPageView = nullptr;
    if(pView)
        pSdrPageView = pView->GetSdrPageView();

    if(pSdrPageView)
    {
        OUString aLayerName(pLayer->GetName());
        switch(what)
        {
        case VISIBLE:   pSdrPageView->SetLayerVisible(aLayerName,flag);
                        break;
        case PRINTABLE: pSdrPageView->SetLayerPrintable(aLayerName,flag);
                        break;
        case LOCKED:    pSdrPageView->SetLayerLocked(aLayerName,flag);
                        break;
        }
    }

    // Try 2. get info from FrameView
    if(!mxLayerManager->GetDocShell())
        return;

    ::sd::FrameView *pFrameView = mxLayerManager->GetDocShell()->GetFrameView();

    if(!pFrameView)
        return;

    SdrLayerIDSet aNewLayers;
    switch(what)
    {
    case VISIBLE:   aNewLayers = pFrameView->GetVisibleLayers();
                    break;
    case PRINTABLE: aNewLayers = pFrameView->GetPrintableLayers();
                    break;
    case LOCKED:    aNewLayers = pFrameView->GetLockedLayers();
                    break;
    }

    aNewLayers.Set(pLayer->GetID(),flag);

    switch(what)
    {
    case VISIBLE:   pFrameView->SetVisibleLayers(aNewLayers);
                    break;
    case PRINTABLE: pFrameView->SetPrintableLayers(aNewLayers);
                    break;
    case LOCKED:    pFrameView->SetLockedLayers(aNewLayers);
                    break;
    }
    return;
    //TODO: uno::Exception?
}

// css::container::XChild
uno::Reference<uno::XInterface> SAL_CALL SdLayer::getParent()
{
    SolarMutexGuard aGuard;

    if( !mxLayerManager.is() )
        throw lang::DisposedException();

    return uno::Reference<uno::XInterface> (static_cast<cppu::OWeakObject*>(mxLayerManager.get()), uno::UNO_QUERY);
}

void SAL_CALL SdLayer::setParent (const uno::Reference<uno::XInterface >& )
{
    throw lang::NoSupportException ();
}

// XComponent
void SAL_CALL SdLayer::dispose(  )
{
    mxLayerManager.clear();
    pLayer = nullptr;
}

void SAL_CALL SdLayer::addEventListener( const uno::Reference< lang::XEventListener >& )
{
    OSL_FAIL("not implemented!");
}

void SAL_CALL SdLayer::removeEventListener( const uno::Reference< lang::XEventListener >& )
{
    OSL_FAIL("not implemented!");
}

// class SdLayerManager
SdLayerManager::SdLayerManager( SdXImpressDocument& rMyModel ) throw()
:mpModel( &rMyModel)
{
    mpLayers.reset(new SvUnoWeakContainer);
}

SdLayerManager::~SdLayerManager() throw()
{
    dispose();
}

// uno helper
UNO3_GETIMPLEMENTATION_IMPL( SdLayerManager );

// XComponent
void SAL_CALL SdLayerManager::dispose(  )
{
    mpModel = nullptr;
    if( mpLayers )
    {
        mpLayers->dispose();
        mpLayers.reset();
    }
}

void SAL_CALL SdLayerManager::addEventListener( const uno::Reference< lang::XEventListener >& )
{
    OSL_FAIL("not implemented!");
}

void SAL_CALL SdLayerManager::removeEventListener( const uno::Reference< lang::XEventListener >& )
{
    OSL_FAIL("not implemented!");
}

// XServiceInfo
OUString SAL_CALL SdLayerManager::getImplementationName()
{
    return OUString("SdUnoLayerManager");
}

sal_Bool SAL_CALL SdLayerManager::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SdLayerManager::getSupportedServiceNames()
{
    uno::Sequence< OUString > aSeq { "com.sun.star.drawing.LayerManager" };
    return aSeq;
}

// XLayerManager
uno::Reference< drawing::XLayer > SAL_CALL SdLayerManager::insertNewByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;

    if( mpModel == nullptr )
        throw lang::DisposedException();

    uno::Reference< drawing::XLayer > xLayer;

    if( mpModel->mpDoc )
    {
        SdrLayerAdmin& rLayerAdmin = mpModel->mpDoc->GetLayerAdmin();
        sal_uInt16 nLayerCnt = rLayerAdmin.GetLayerCount();
        sal_Int32 nLayer = nLayerCnt - 2 + 1;
        OUString aLayerName;

        // Test for existing names
        while( aLayerName.isEmpty() || rLayerAdmin.GetLayer( aLayerName ) )
        {
            aLayerName = SdResId(STR_LAYER) + OUString::number(nLayer);
            ++nLayer;
        }

        SdrLayerAdmin& rLA=mpModel->mpDoc->GetLayerAdmin();
        const sal_Int32 nMax=rLA.GetLayerCount();
        if (nIndex>nMax) nIndex=nMax;
        xLayer = GetLayer (rLA.NewLayer(aLayerName,static_cast<sal_uInt16>(nIndex)));
        mpModel->SetModified();
    }
    return xLayer;
}

void SAL_CALL SdLayerManager::remove( const uno::Reference< drawing::XLayer >& xLayer )
{
    SolarMutexGuard aGuard;

    if( mpModel == nullptr )
        throw lang::DisposedException();

    SdLayer* pSdLayer = SdLayer::getImplementation(xLayer);

    if(pSdLayer && GetView())
    {
        const SdrLayer* pSdrLayer = pSdLayer->GetSdrLayer();
        GetView()->DeleteLayer( pSdrLayer->GetName() );

        UpdateLayerView();
    }

    mpModel->SetModified();
}

void SAL_CALL SdLayerManager::attachShapeToLayer( const uno::Reference< drawing::XShape >& xShape, const uno::Reference< drawing::XLayer >& xLayer )
{
    SolarMutexGuard aGuard;

    if( mpModel == nullptr )
        throw lang::DisposedException();

    SdLayer* pSdLayer = SdLayer::getImplementation(xLayer);
    SdrLayer* pSdrLayer = pSdLayer?pSdLayer->GetSdrLayer():nullptr;
    if(pSdrLayer==nullptr)
        return;

    SvxShape* pShape = SvxShape::getImplementation( xShape );
    SdrObject* pSdrObject = pShape?pShape->GetSdrObject():nullptr;

    if(pSdrObject)
        pSdrObject->SetLayer(pSdrLayer->GetID());

    mpModel->SetModified();
}

uno::Reference< drawing::XLayer > SAL_CALL SdLayerManager::getLayerForShape( const uno::Reference< drawing::XShape >& xShape )
{
    SolarMutexGuard aGuard;

    if( mpModel == nullptr )
        throw lang::DisposedException();

    uno::Reference< drawing::XLayer >  xLayer;

    if(mpModel->mpDoc)
    {
        SvxShape* pShape = SvxShape::getImplementation( xShape );
        SdrObject* pObj = pShape?pShape->GetSdrObject():nullptr;
        if(pObj)
        {
            SdrLayerID aId = pObj->GetLayer();
            SdrLayerAdmin& rLayerAdmin = mpModel->mpDoc->GetLayerAdmin();
            xLayer = GetLayer (rLayerAdmin.GetLayerPerID(aId));
        }
    }
    return xLayer;
}

// XIndexAccess
sal_Int32 SAL_CALL SdLayerManager::getCount()
{
    SolarMutexGuard aGuard;

    if( mpModel == nullptr )
        throw lang::DisposedException();

    if( mpModel->mpDoc )
    {
        SdrLayerAdmin& rLayerAdmin = mpModel->mpDoc->GetLayerAdmin();
        return rLayerAdmin.GetLayerCount();
    }

    return 0;
}

uno::Any SAL_CALL SdLayerManager::getByIndex( sal_Int32 nLayer )
{
    SolarMutexGuard aGuard;

    if( mpModel == nullptr )
        throw lang::DisposedException();

    if( nLayer >= getCount() || nLayer < 0 )
        throw lang::IndexOutOfBoundsException();

    uno::Any aAny;

    if( mpModel->mpDoc )
    {
        SdrLayerAdmin& rLayerAdmin = mpModel->mpDoc->GetLayerAdmin();
        uno::Reference<drawing::XLayer> xLayer (GetLayer (rLayerAdmin.GetLayer(static_cast<sal_uInt16>(nLayer))));
        aAny <<= xLayer;
    }
    return aAny;
}

// XNameAccess
uno::Any SAL_CALL SdLayerManager::getByName( const OUString& aName )
{
    SolarMutexGuard aGuard;

    if( (mpModel == nullptr) || (mpModel->mpDoc == nullptr ) )
        throw lang::DisposedException();

    SdrLayerAdmin& rLayerAdmin = mpModel->mpDoc->GetLayerAdmin();
    SdrLayer* pLayer = rLayerAdmin.GetLayer(aName);
    if( pLayer == nullptr )
        throw container::NoSuchElementException();

    return uno::Any( GetLayer (pLayer) );
}

uno::Sequence< OUString > SAL_CALL SdLayerManager::getElementNames()
{
    SolarMutexGuard aGuard;

    if( mpModel == nullptr )
        throw lang::DisposedException();

    SdrLayerAdmin& rLayerAdmin = mpModel->mpDoc->GetLayerAdmin();
    const sal_uInt16 nLayerCount = rLayerAdmin.GetLayerCount();

    uno::Sequence< OUString > aSeq( nLayerCount );

    OUString* pStrings = aSeq.getArray();

    for( sal_uInt16 nLayer = 0; nLayer < nLayerCount; nLayer++ )
    {
        SdrLayer* pLayer = rLayerAdmin.GetLayer( nLayer );
        if( pLayer )
            *pStrings++ = pLayer->GetName();
    }

    return aSeq;
}

sal_Bool SAL_CALL SdLayerManager::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;

    if( mpModel == nullptr )
        throw lang::DisposedException();

    SdrLayerAdmin& rLayerAdmin = mpModel->mpDoc->GetLayerAdmin();

    return nullptr != rLayerAdmin.GetLayer(aName);
}

// XElementAccess
uno::Type SAL_CALL SdLayerManager::getElementType()
{
    return cppu::UnoType<drawing::XLayer>::get();
}

sal_Bool SAL_CALL SdLayerManager::hasElements()
{
    return getCount() > 0;
}

/**
 * If something was changed at the layers, this methods takes care that the
 * changes are made visible in sdbcx::View.
 */
void SdLayerManager::UpdateLayerView() const throw()
{
    if(!mpModel->mpDocShell)
        return;

    ::sd::DrawViewShell* pDrViewSh = dynamic_cast< ::sd::DrawViewShell* >( mpModel->mpDocShell->GetViewShell());

    if(pDrViewSh)
    {
        bool bLayerMode = pDrViewSh->IsLayerModeActive();
        pDrViewSh->ChangeEditMode(pDrViewSh->GetEditMode(), !bLayerMode);
        pDrViewSh->ChangeEditMode(pDrViewSh->GetEditMode(), bLayerMode);
    }

    mpModel->mpDoc->SetChanged();
}

/** */
::sd::View* SdLayerManager::GetView() const throw()
{
    if( mpModel->mpDocShell )
    {
        ::sd::ViewShell* pViewSh = mpModel->mpDocShell->GetViewShell();
        if(pViewSh)
            return pViewSh->GetView();
    }
    return nullptr;
}

namespace
{
/** Compare two pointers to <type>SdrLayer</type> objects.
    @param xRef
        The implementing SdLayer class provides the first pointer by the
        <member>SdLayer::GetSdrLayer</member> method.
    @param pSearchData
        This void pointer is the second pointer to an <type>SdrLayer</type>
        object.
    @return
        Return </True> if both pointers point to the same object.
*/
bool compare_layers (const uno::WeakReference<uno::XInterface>& xRef, void const * pSearchData)
{
    uno::Reference<uno::XInterface> xLayer (xRef);
    if (xLayer.is())
    {
        SdLayer* pSdLayer = SdLayer::getImplementation (xRef);
        if (pSdLayer != nullptr)
        {
            SdrLayer* pSdrLayer = pSdLayer->GetSdrLayer ();
            if (pSdrLayer == static_cast<SdrLayer const *>(pSearchData))
                return true;
        }
    }
    return false;
}
}

/** Use the <member>mpLayers</member> container of weak references to either
    retrieve and return a previously created <type>XLayer</type> object for
    the given <type>SdrLayer</type> object or create and remember a new one.
*/
uno::Reference<drawing::XLayer> SdLayerManager::GetLayer (SdrLayer* pLayer)
{
    uno::WeakReference<uno::XInterface> xRef;
    uno::Reference<drawing::XLayer>  xLayer;

    // Search existing xLayer for the given pLayer.
    if (mpLayers->findRef (xRef, static_cast<void*>(pLayer), compare_layers))
        xLayer.set(xRef, uno::UNO_QUERY);

    // Create the xLayer if necessary.
    if ( ! xLayer.is())
    {
        xLayer = new SdLayer (this, pLayer);

        // Remember the new xLayer for future calls.
        uno::WeakReference<uno::XInterface> wRef(xLayer);
        mpLayers->insert(wRef);
    }

    return xLayer;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
