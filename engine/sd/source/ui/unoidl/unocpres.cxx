/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <algorithm>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <o3tl/safeint.hxx>
#include <vcl/svapp.hxx>
#include <svx/svdpage.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <createunocustomshow.hxx>
#include <unomodel.hxx>
#include <drawdoc.hxx>
#include "unocpres.hxx"
#include <cusshow.hxx>
#include <unopage.hxx>
#include <customshowlist.hxx>

using namespace ::com::sun::star;
using namespace ::cpo;

uno::Reference< cpo::uno::XInterface > createUnoCustomShow( SdCustomShow* pShow )
{
    return static_cast<cppu::OWeakObject*>(new SdXCustomPresentation( pShow ));
}

SdXCustomPresentation::SdXCustomPresentation() noexcept
:   mpSdCustomShow(nullptr), mpModel(nullptr),
    bDisposing( false )
{
}

SdXCustomPresentation::SdXCustomPresentation( SdCustomShow* pShow) noexcept
:   mpSdCustomShow(pShow), mpModel(nullptr),
    bDisposing( false )
{
}

SdXCustomPresentation::~SdXCustomPresentation() noexcept
{
}

// XServiceInfo
OUString SdXCustomPresentation::getImplementationName()
{
    return u"SdXCustomPresentation"_ustr ;
}

bool SdXCustomPresentation::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

cpo::uno::Sequence< OUString > SdXCustomPresentation::getSupportedServiceNames()
{
    return { u"com.sun.star.presentation.CustomPresentation"_ustr };
}

// XIndexContainer
void SdXCustomPresentation::insertByIndex( sal_Int32 Index, const cpo::uno::Any& Element )
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        throw lang::DisposedException();

    if( Index < 0 || o3tl::make_unsigned(Index) > ( mpSdCustomShow ? mpSdCustomShow->PagesVector().size() : 0 ) )
        throw lang::IndexOutOfBoundsException();

    uno::Reference< drawing::XDrawPage > xPage;
    Element >>= xPage;

    if(!xPage.is())
        throw lang::IllegalArgumentException();

    SdDrawPage* pPage = comphelper::getFromUnoTunnel<SdDrawPage>( xPage );

    if(pPage)
    {
        if (!mpModel)
            mpModel = pPage->GetModel();

        if (!mpSdCustomShow)
            mpSdCustomShow = new SdCustomShow;

        mpSdCustomShow->PagesVector().insert(mpSdCustomShow->PagesVector().begin() + Index,
            static_cast<SdPage*>(pPage->GetSdrPage()));
    }

    if( mpModel )
        mpModel->SetModified();
}

void SdXCustomPresentation::removeByIndex( sal_Int32 Index )
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        throw lang::DisposedException();

    if(mpSdCustomShow)
    {
        uno::Reference< drawing::XDrawPage > xPage;
        getByIndex( Index ) >>= xPage;

        if( xPage.is() )
        {
            SvxDrawPage* pPage = comphelper::getFromUnoTunnel<SvxDrawPage>( xPage );
            if(pPage)
            {
                SdCustomShow::PageVec::iterator it = std::find(
                    mpSdCustomShow->PagesVector().begin(),
                    mpSdCustomShow->PagesVector().end(),
                    pPage->GetSdrPage());
                if (it != mpSdCustomShow->PagesVector().end())
                    mpSdCustomShow->PagesVector().erase(it);
            }
        }
    }

    if( mpModel )
        mpModel->SetModified();
}

// XIndexReplace
void SdXCustomPresentation::replaceByIndex( sal_Int32 Index, const cpo::uno::Any& Element )
{
    removeByIndex( Index );
    insertByIndex( Index, Element );
}

// XElementAccess
cpo::uno::Type SdXCustomPresentation::getElementType()
{
    return cppu::UnoType<drawing::XDrawPage>::get();
}

bool SdXCustomPresentation::hasElements()
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        throw lang::DisposedException();

    return getCount() > 0;
}

// XIndexAccess
sal_Int32 SdXCustomPresentation::getCount()
{
    SolarMutexGuard aGuard;
    if( bDisposing )
        throw lang::DisposedException();

    return mpSdCustomShow ? mpSdCustomShow->PagesVector().size() : 0;
}

cpo::uno::Any SdXCustomPresentation::getByIndex( sal_Int32 Index )
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        throw lang::DisposedException();

    if (Index < 0 || !mpSdCustomShow || o3tl::make_unsigned(Index) >= mpSdCustomShow->PagesVector().size())
        throw lang::IndexOutOfBoundsException();

    cpo::uno::Any aAny;
    SdrPage * pPage = const_cast<SdPage *>(mpSdCustomShow->PagesVector()[Index]);

    if( pPage )
    {
        uno::Reference< drawing::XDrawPage > xRef( pPage->getUnoPage() );
        aAny <<= xRef;
    }

    return aAny;
}

// XNamed
OUString SdXCustomPresentation::getName()
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        throw lang::DisposedException();

    if(mpSdCustomShow)
        return mpSdCustomShow->GetName();

    return OUString();
}

void SdXCustomPresentation::setName( const OUString& aName )
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        throw lang::DisposedException();

    if(mpSdCustomShow)
        mpSdCustomShow->SetName( aName );
}

// XComponent
void SdXCustomPresentation::dispose()
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        return; // caught a recursion

    bDisposing = true;

    uno::Reference< cpo::uno::XInterface > xSource( static_cast<cppu::OWeakObject*>(this) );

    std::unique_lock aGuard2(aDisposeContainerMutex);
    lang::EventObject aEvt;
    aEvt.Source = std::move(xSource);
    aDisposeListeners.disposeAndClear(aGuard2, aEvt);

    mpSdCustomShow = nullptr;
}

void SdXCustomPresentation::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    if( bDisposing )
        throw lang::DisposedException();

    std::unique_lock aGuard(aDisposeContainerMutex);
    aDisposeListeners.addInterface(aGuard, xListener);
}

void SdXCustomPresentation::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
{
    if( !bDisposing )
    {
        std::unique_lock aGuard(aDisposeContainerMutex);
        aDisposeListeners.removeInterface(aGuard, aListener);
    }
}

/*===========================================================================*
 *  class SdXCustomPresentationAccess : public XCustomPresentationAccess,    *
 *                                      public UsrObject                     *
 *===========================================================================*/

SdXCustomPresentationAccess::SdXCustomPresentationAccess(SdXImpressDocument& rMyModel) noexcept
: mrModel(rMyModel)
{
}

SdXCustomPresentationAccess::~SdXCustomPresentationAccess() noexcept
{
}

// XServiceInfo
OUString SdXCustomPresentationAccess::getImplementationName()
{
    return u"SdXCustomPresentationAccess"_ustr;
}

bool SdXCustomPresentationAccess::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

cpo::uno::Sequence< OUString > SdXCustomPresentationAccess::getSupportedServiceNames()
{
    return { u"com.sun.star.presentation.CustomPresentationAccess"_ustr };
}

// XSingleServiceFactory
uno::Reference< cpo::uno::XInterface > SdXCustomPresentationAccess::createInstance()
{
    uno::Reference< cpo::uno::XInterface >  xRef( static_cast<cppu::OWeakObject*>(new SdXCustomPresentation()) );
    return xRef;
}

uno::Reference< cpo::uno::XInterface > SdXCustomPresentationAccess::createInstanceWithArguments( const cpo::uno::Sequence< cpo::uno::Any >& )
{
    return createInstance();
}

// XNameContainer
void SdXCustomPresentationAccess::insertByName( const OUString& aName, const cpo::uno::Any& aElement )
{
    SolarMutexGuard aGuard;

    // get the documents custom show list
    SdCustomShowList* pList = nullptr;
    if(mrModel.GetDoc())
        pList = mrModel.GetDoc()->GetCustomShowList(true);

    // no list, no cookies
    if( nullptr == pList)
        throw cpo::uno::RuntimeException();

    // do we have a container::XIndexContainer?
    SdXCustomPresentation* pXShow = nullptr;

    uno::Reference< container::XIndexContainer > xContainer;
    if( (aElement >>= xContainer) && xContainer.is() )
        pXShow = dynamic_cast<SdXCustomPresentation*>(xContainer.get());

    if( nullptr == pXShow )
        throw lang::IllegalArgumentException();

    // get the internal custom show from the api wrapper
    SdCustomShow* pShow = pXShow->GetSdCustomShow();
    if( nullptr == pShow )
    {
        pShow = new SdCustomShow( xContainer );
        pXShow->SetSdCustomShow( pShow );
    }
    else
    {
        if( nullptr == pXShow->GetModel() || *pXShow->GetModel() != mrModel )
            throw lang::IllegalArgumentException();
    }

    // give it a name
    pShow->SetName( aName);

    // check if this or another customshow with the same name already exists
    for( SdCustomShow* pCompare = pList->First();
         pCompare;
         pCompare = pList->Next() )
    {
        if( pCompare == pShow || pCompare->GetName() == pShow->GetName() )
            throw container::ElementExistException();
    }

    pList->push_back(std::unique_ptr<SdCustomShow>(pShow));

    mrModel.SetModified();
}

void SdXCustomPresentationAccess::removeByName( const OUString& Name )
{
    SolarMutexGuard aGuard;

    SdCustomShow* pShow = getSdCustomShow(Name);

    SdCustomShowList* pList = GetCustomShowList();
    if(!pList || !pShow)
        throw container::NoSuchElementException();

    pList->erase( pShow );

    mrModel.SetModified();
}

// XNameReplace
void SdXCustomPresentationAccess::replaceByName( const OUString& aName, const cpo::uno::Any& aElement )
{
    removeByName( aName );
    insertByName( aName, aElement );
}

// XNameAccess
cpo::uno::Any SdXCustomPresentationAccess::getByName( const OUString& aName )
{
    SolarMutexGuard aGuard;

    SdCustomShow* pShow = getSdCustomShow(aName);
    if(!pShow)
    {
        throw container::NoSuchElementException();
    }

    uno::Reference< container::XIndexContainer >  xRef( pShow->getUnoCustomShow(), uno::UNO_QUERY );
    return cpo::uno::Any(xRef);
}

cpo::uno::Sequence< OUString > SdXCustomPresentationAccess::getElementNames()
{
    SolarMutexGuard aGuard;

    SdCustomShowList* pList = GetCustomShowList();
    const sal_uInt32 nCount = pList ? pList->size() : 0;

    cpo::uno::Sequence< OUString > aSequence( nCount );
    OUString* pStringList = aSequence.getArray();

    sal_uInt32 nIdx = 0;
    while( nIdx < nCount )
    {
        const SdCustomShow* pShow = (*pList)[nIdx].get();
        pStringList[nIdx] = pShow->GetName();
        nIdx++;
    }

    return aSequence;
}

bool SdXCustomPresentationAccess::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    return getSdCustomShow(aName) != nullptr;
}

// XElementAccess
cpo::uno::Type SdXCustomPresentationAccess::getElementType()
{
    return cppu::UnoType<container::XIndexContainer>::get();
}

bool SdXCustomPresentationAccess::hasElements()
{
    SolarMutexGuard aGuard;

    SdCustomShowList* pList = GetCustomShowList();
    return pList && !pList->empty();
}

SdCustomShow * SdXCustomPresentationAccess::getSdCustomShow( std::u16string_view rName ) const noexcept
{
    sal_uInt32 nIdx = 0;

    SdCustomShowList* pList = GetCustomShowList();
    const sal_uInt32 nCount = pList ? pList->size() : 0;

    while( nIdx < nCount )
    {
        SdCustomShow* pShow = (*pList)[nIdx].get();
        if( pShow->GetName() == rName )
            return pShow;
        nIdx++;
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
