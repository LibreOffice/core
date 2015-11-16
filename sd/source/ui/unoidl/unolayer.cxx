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

#include "unolayer.hxx"
#include <osl/mutex.hxx>

#include <comphelper/extract.hxx>
#include <svx/svdpagv.hxx>
#include <svx/unoshape.hxx>
#include <svx/svdobj.hxx>
#include <cppuhelper/supportsservice.hxx>

// following ones for InsertSdPage()
#include <svx/svdlayer.hxx>

#include "DrawDocShell.hxx"
#include <drawdoc.hxx>
#include <unomodel.hxx>
#include "unoprnms.hxx"
#include <com/sun/star/lang/NoSupportException.hpp>
#include <svx/svdpool.hxx>
#include "unohelp.hxx"
#include "FrameView.hxx"
#include "DrawViewShell.hxx"
#include "View.hxx"
#include "ViewShell.hxx"
#include "app.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include "glob.hrc"

#include "unokywds.hxx"
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

const SvxItemPropertySet* ImplGetSdLayerPropertySet()
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

OUString SdLayer::convertToInternalName( const OUString& rName )
{
    if ( rName == sUNO_LayerName_background )
    {
        return SD_RESSTR( STR_LAYER_BCKGRND );
    }
    else if ( rName == sUNO_LayerName_background_objects )
    {
        return  SD_RESSTR( STR_LAYER_BCKGRNDOBJ );
    }
    else if ( rName == sUNO_LayerName_layout )
    {
        return  SD_RESSTR( STR_LAYER_LAYOUT );
    }
    else if ( rName == sUNO_LayerName_controls )
    {
        return  SD_RESSTR( STR_LAYER_CONTROLS );
    }
    else if ( rName == sUNO_LayerName_measurelines )
    {
        return  SD_RESSTR( STR_LAYER_MEASURELINES );
    }
    else
    {
        return rName;
    }
}

OUString SdLayer::convertToExternalName( const OUString& rName )
{
    if( rName == SD_RESSTR( STR_LAYER_BCKGRND ) )
    {
        return OUString( sUNO_LayerName_background );
    }
    else if( rName == SD_RESSTR( STR_LAYER_BCKGRNDOBJ ) )
    {
        return OUString( sUNO_LayerName_background_objects );
    }
    else if( rName == SD_RESSTR( STR_LAYER_LAYOUT ) )
    {
        return OUString( sUNO_LayerName_layout );
    }
    else if( rName == SD_RESSTR( STR_LAYER_CONTROLS ) )
    {
        return OUString( sUNO_LayerName_controls );
    }
    else if( rName == SD_RESSTR( STR_LAYER_MEASURELINES ) )
    {
        return OUString( sUNO_LayerName_measurelines );
    }
    else
    {
        return rName;
    }
}

SdLayer::SdLayer( SdLayerManager* pLayerManager_, SdrLayer* pSdrLayer_ ) throw()
: pLayerManager(pLayerManager_)
, mxLayerManager(pLayerManager_)
, pLayer(pSdrLayer_)
, pPropSet(ImplGetSdLayerPropertySet())
{
}

SdLayer::~SdLayer() throw()
{
}

// uno helper
UNO3_GETIMPLEMENTATION_IMPL( SdLayer );

// XServiceInfo
OUString SAL_CALL SdLayer::getImplementationName()
    throw(uno::RuntimeException, std::exception)
{
    return OUString("SdUnoLayer");
}

sal_Bool SAL_CALL SdLayer::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SdLayer::getSupportedServiceNames()
    throw(uno::RuntimeException, std::exception)
{
    OUString aServiceName(UNO_PREFIX "drawing.Layer");
    uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

// beans::XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SdLayer::getPropertySetInfo(  )
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return pPropSet->getPropertySetInfo();
}

void SAL_CALL SdLayer::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if(pLayer == nullptr || pLayerManager == nullptr)
        throw lang::DisposedException();

    const SfxItemPropertySimpleEntry* pEntry = pPropSet->getPropertyMapEntry(aPropertyName);

    switch( pEntry ? pEntry->nWID : -1 )
    {
    case WID_LAYER_LOCKED:
    {
        set(LOCKED, cppu::any2bool(aValue));
        break;
    }
    case WID_LAYER_PRINTABLE:
    {
        set(PRINTABLE, cppu::any2bool(aValue));
        break;
    }
    case WID_LAYER_VISIBLE:
    {
        set(VISIBLE, cppu::any2bool(aValue));
        break;
    }
    case WID_LAYER_NAME:
    {
        OUString aName;
        if(!(aValue >>= aName))
            throw lang::IllegalArgumentException();

        pLayer->SetName(SdLayer::convertToInternalName( aName ) );
        pLayerManager->UpdateLayerView();
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
        throw beans::UnknownPropertyException();
    }

    if( pLayerManager->GetDocShell() )
        pLayerManager->GetDocShell()->SetModified();
}

uno::Any SAL_CALL SdLayer::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if(pLayer == nullptr || pLayerManager == nullptr)
        throw lang::DisposedException();

    const SfxItemPropertySimpleEntry* pEntry = pPropSet->getPropertyMapEntry(PropertyName);

    uno::Any aValue;

    switch( pEntry ? pEntry->nWID : -1 )
    {
    case WID_LAYER_LOCKED:
        sd::bool2any( get( LOCKED ), aValue );
        break;
    case WID_LAYER_PRINTABLE:
        sd::bool2any( get( PRINTABLE ), aValue );
        break;
    case WID_LAYER_VISIBLE:
        sd::bool2any( get( VISIBLE ), aValue );
        break;
    case WID_LAYER_NAME:
    {
        OUString aRet( SdLayer::convertToExternalName( pLayer->GetName() ) );
        aValue <<= aRet;
        break;
    }
    case WID_LAYER_TITLE:
        aValue <<= OUString( pLayer->GetTitle() );
        break;
    case WID_LAYER_DESC:
        aValue <<= OUString( pLayer->GetDescription() );
        break;
    default:
        throw beans::UnknownPropertyException();
    }

    return aValue;
}

void SAL_CALL SdLayer::addPropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >& ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception) {}
void SAL_CALL SdLayer::removePropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >& ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception) {}
void SAL_CALL SdLayer::addVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >& ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception) {}
void SAL_CALL SdLayer::removeVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >& ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception) {}

bool SdLayer::get( LayerAttribute what ) throw()
{
    if(pLayer&&pLayerManager)
    {
        // Try 1. is an arbitrary page open?
        ::sd::View *pView = pLayerManager->GetView();
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
        if(pLayerManager->GetDocShell())
        {
            ::sd::FrameView *pFrameView = pLayerManager->GetDocShell()->GetFrameView();
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
    if(pLayer&&pLayerManager)
    {
        // Try 1. is an arbitrary page open?
        ::sd::View *pView = pLayerManager->GetView();
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
        if(pLayerManager->GetDocShell())
        {
            ::sd::FrameView *pFrameView = pLayerManager->GetDocShell()->GetFrameView();

            if(pFrameView)
            {
                SetOfByte aNewLayers;
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
            }
        }
    }
    //TODO: uno::Exception?
}

// css::container::XChild
uno::Reference<uno::XInterface> SAL_CALL SdLayer::getParent()
    throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if( pLayerManager == nullptr )
        throw lang::DisposedException();

    return uno::Reference<uno::XInterface> (mxLayerManager, uno::UNO_QUERY);
}

void SAL_CALL SdLayer::setParent (const uno::Reference<uno::XInterface >& )
    throw (css::lang::NoSupportException,
        css::uno::RuntimeException, std::exception)
{
    throw lang::NoSupportException ();
}

// XComponent
void SAL_CALL SdLayer::dispose(  ) throw (uno::RuntimeException, std::exception)
{
    pLayerManager = nullptr;
    mxLayerManager = nullptr;
    pLayer = nullptr;
}

void SAL_CALL SdLayer::addEventListener( const uno::Reference< lang::XEventListener >& ) throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented!");
}

void SAL_CALL SdLayer::removeEventListener( const uno::Reference< lang::XEventListener >& ) throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented!");
}

// class SdLayerManager
SdLayerManager::SdLayerManager( SdXImpressDocument& rMyModel ) throw()
:mpModel( &rMyModel)
{
    mpLayers = new SvUnoWeakContainer;
}

SdLayerManager::~SdLayerManager() throw()
{
    dispose();
}

// uno helper
UNO3_GETIMPLEMENTATION_IMPL( SdLayerManager );

// XComponent
void SAL_CALL SdLayerManager::dispose(  ) throw (uno::RuntimeException, std::exception)
{
    mpModel = nullptr;
    if( mpLayers )
    {
        mpLayers->dispose();

        delete mpLayers;
        mpLayers = nullptr;
    }
}

void SAL_CALL SdLayerManager::addEventListener( const uno::Reference< lang::XEventListener >& ) throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented!");
}

void SAL_CALL SdLayerManager::removeEventListener( const uno::Reference< lang::XEventListener >& ) throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented!");
}

// XServiceInfo
OUString SAL_CALL SdLayerManager::getImplementationName()
    throw(uno::RuntimeException, std::exception)
{
    return OUString("SdUnoLayerManager");
}

sal_Bool SAL_CALL SdLayerManager::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SdLayerManager::getSupportedServiceNames()
    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aSeq { UNO_PREFIX "drawing.LayerManager" };
    return aSeq;
}

// XLayerManager
uno::Reference< drawing::XLayer > SAL_CALL SdLayerManager::insertNewByIndex( sal_Int32 nIndex )
    throw(uno::RuntimeException, std::exception)
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
        while( aLayerName.isEmpty() || rLayerAdmin.GetLayer( aLayerName, false) )
        {
            aLayerName = SD_RESSTR(STR_LAYER);
            aLayerName += OUString::number(nLayer);
            ++nLayer;
        }

        SdrLayerAdmin& rLA=mpModel->mpDoc->GetLayerAdmin();
        const sal_Int32 nMax=rLA.GetLayerCount();
        if (nIndex>nMax) nIndex=nMax;
        xLayer = GetLayer (rLA.NewLayer(aLayerName,(sal_uInt16)nIndex));
        mpModel->SetModified();
    }
    return xLayer;
}

void SAL_CALL SdLayerManager::remove( const uno::Reference< drawing::XLayer >& xLayer )
    throw(container::NoSuchElementException, uno::RuntimeException, std::exception)
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
    throw(uno::RuntimeException, std::exception)
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

    if(pSdrObject && pSdrLayer )
        pSdrObject->SetLayer(pSdrLayer->GetID());

    mpModel->SetModified();
}

uno::Reference< drawing::XLayer > SAL_CALL SdLayerManager::getLayerForShape( const uno::Reference< drawing::XShape >& xShape ) throw(uno::RuntimeException, std::exception)
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
    throw(uno::RuntimeException, std::exception)
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
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
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
        uno::Reference<drawing::XLayer> xLayer (GetLayer (rLayerAdmin.GetLayer((sal_uInt16)nLayer)));
        aAny <<= xLayer;
    }
    return aAny;
}

// XNameAccess
uno::Any SAL_CALL SdLayerManager::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if( (mpModel == nullptr) || (mpModel->mpDoc == nullptr ) )
        throw lang::DisposedException();

    SdrLayerAdmin& rLayerAdmin = mpModel->mpDoc->GetLayerAdmin();
    SdrLayer* pLayer = rLayerAdmin.GetLayer( SdLayer::convertToInternalName( aName ), false );
    if( pLayer == nullptr )
        throw container::NoSuchElementException();

    return uno::Any( GetLayer (pLayer) );
}

uno::Sequence< OUString > SAL_CALL SdLayerManager::getElementNames()
    throw(uno::RuntimeException, std::exception)
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
            *pStrings++ = SdLayer::convertToExternalName( pLayer->GetName() );
    }

    return aSeq;
}

sal_Bool SAL_CALL SdLayerManager::hasByName( const OUString& aName ) throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if( mpModel == nullptr )
        throw lang::DisposedException();

    SdrLayerAdmin& rLayerAdmin = mpModel->mpDoc->GetLayerAdmin();

    return nullptr != rLayerAdmin.GetLayer( SdLayer::convertToInternalName( aName ), false );
}

// XElementAccess
uno::Type SAL_CALL SdLayerManager::getElementType()
    throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<drawing::XLayer>::get();
}

sal_Bool SAL_CALL SdLayerManager::hasElements() throw(uno::RuntimeException, std::exception)
{
    return getCount() > 0;
}

/**
 * If something was changed at the layers, this methods takes care that the
 * changes are made visible in sdbcx::View.
 */
void SdLayerManager::UpdateLayerView( bool modify ) const throw()
{
    if(mpModel->mpDocShell)
    {
        ::sd::DrawViewShell* pDrViewSh = dynamic_cast< ::sd::DrawViewShell* >( mpModel->mpDocShell->GetViewShell());

        if(pDrViewSh)
        {
            bool bLayerMode = pDrViewSh->IsLayerModeActive();
            pDrViewSh->ChangeEditMode(pDrViewSh->GetEditMode(), !bLayerMode);
            pDrViewSh->ChangeEditMode(pDrViewSh->GetEditMode(), bLayerMode);
        }

        if(modify)
            mpModel->mpDoc->SetChanged();
    }
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
bool compare_layers (uno::WeakReference<uno::XInterface> xRef, void* pSearchData)
{
    uno::Reference<uno::XInterface> xLayer (xRef);
    if (xLayer.is())
    {
        SdLayer* pSdLayer = SdLayer::getImplementation (xRef);
        if (pSdLayer != nullptr)
        {
            SdrLayer* pSdrLayer = pSdLayer->GetSdrLayer ();
            if (pSdrLayer == static_cast<SdrLayer*>(pSearchData))
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
