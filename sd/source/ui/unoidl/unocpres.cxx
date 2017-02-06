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

#include <algorithm>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svx/svdpage.hxx>
#include <comphelper/extract.hxx>
#include <cppuhelper/supportsservice.hxx>

#include "createunocustomshow.hxx"
#include "unomodel.hxx"
#include "drawdoc.hxx"
#include "unocpres.hxx"
#include "cusshow.hxx"
#include "unopage.hxx"
#include "customshowlist.hxx"

using namespace ::com::sun::star;

uno::Reference< uno::XInterface > createUnoCustomShow( SdCustomShow* pShow )
{
    return static_cast<cppu::OWeakObject*>(new SdXCustomPresentation( pShow ));
}

SdXCustomPresentation::SdXCustomPresentation() throw()
:   mpSdCustomShow(nullptr), mpModel(nullptr),
    aDisposeListeners( aDisposeContainerMutex ),
    bDisposing( false )
{
}

SdXCustomPresentation::SdXCustomPresentation( SdCustomShow* pShow) throw()
:   mpSdCustomShow(pShow), mpModel(nullptr),
    aDisposeListeners( aDisposeContainerMutex ),
    bDisposing( false )
{
}

SdXCustomPresentation::~SdXCustomPresentation() throw()
{
}

UNO3_GETIMPLEMENTATION_IMPL( SdXCustomPresentation );

// XServiceInfo
OUString SAL_CALL SdXCustomPresentation::getImplementationName()
{
    return OUString( "SdXCustomPresentation" ) ;
}

sal_Bool SAL_CALL SdXCustomPresentation::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SdXCustomPresentation::getSupportedServiceNames()
{
    OUString aSN( "com.sun.star.presentation.CustomPresentation" );
    uno::Sequence< OUString > aSeq( &aSN, 1 );
    return aSeq;
}

// XIndexContainer
void SAL_CALL SdXCustomPresentation::insertByIndex( sal_Int32 Index, const uno::Any& Element )
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        throw lang::DisposedException();

    if( Index < 0 || Index > (sal_Int32)( mpSdCustomShow ? mpSdCustomShow->PagesVector().size() : 0 ) )
        throw lang::IndexOutOfBoundsException();

    uno::Reference< drawing::XDrawPage > xPage;
    Element >>= xPage;

    if(!xPage.is())
        throw lang::IllegalArgumentException();

    SdDrawPage* pPage = SdDrawPage::getImplementation( xPage );

    if(pPage)
    {
        if( nullptr == mpModel )
            mpModel = pPage->GetModel();

        if( nullptr != mpModel && nullptr == mpSdCustomShow && mpModel->GetDoc() )
            mpSdCustomShow = new SdCustomShow;

        mpSdCustomShow->PagesVector().insert(mpSdCustomShow->PagesVector().begin() + Index,
            static_cast<SdPage*>(pPage->GetSdrPage()));
    }

    if( mpModel )
        mpModel->SetModified();
}

void SAL_CALL SdXCustomPresentation::removeByIndex( sal_Int32 Index )
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
            SvxDrawPage* pPage = SvxDrawPage::getImplementation( xPage );
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
void SAL_CALL SdXCustomPresentation::replaceByIndex( sal_Int32 Index, const uno::Any& Element )
{
    removeByIndex( Index );
    insertByIndex( Index, Element );
}

// XElementAccess
uno::Type SAL_CALL SdXCustomPresentation::getElementType()
{
    return cppu::UnoType<drawing::XDrawPage>::get();
}

sal_Bool SAL_CALL SdXCustomPresentation::hasElements()
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        throw lang::DisposedException();

    return getCount() > 0;
}

// XIndexAccess
sal_Int32 SAL_CALL SdXCustomPresentation::getCount()
{
    SolarMutexGuard aGuard;
    if( bDisposing )
        throw lang::DisposedException();

    return mpSdCustomShow ? mpSdCustomShow->PagesVector().size() : 0;
}

uno::Any SAL_CALL SdXCustomPresentation::getByIndex( sal_Int32 Index )
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        throw lang::DisposedException();

    if (Index < 0 || !mpSdCustomShow || Index >= (sal_Int32)mpSdCustomShow->PagesVector().size())
        throw lang::IndexOutOfBoundsException();

    uno::Any aAny;
    SdrPage * pPage = static_cast<SdrPage*>(const_cast<SdPage *>(mpSdCustomShow->PagesVector()[Index]));

    if( pPage )
    {
        uno::Reference< drawing::XDrawPage > xRef( pPage->getUnoPage(), uno::UNO_QUERY );
        aAny <<= xRef;
    }

    return aAny;
}

// XNamed
OUString SAL_CALL SdXCustomPresentation::getName()
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        throw lang::DisposedException();

    if(mpSdCustomShow)
        return mpSdCustomShow->GetName();

    return OUString();
}

void SAL_CALL SdXCustomPresentation::setName( const OUString& aName )
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        throw lang::DisposedException();

    if(mpSdCustomShow)
        mpSdCustomShow->SetName( aName );
}

// XComponent
void SAL_CALL SdXCustomPresentation::dispose()
{
    SolarMutexGuard aGuard;

    if( bDisposing )
        return; // catched a recursion

    bDisposing = true;

    uno::Reference< uno::XInterface > xSource( static_cast<cppu::OWeakObject*>(this) );

    lang::EventObject aEvt;
    aEvt.Source = xSource;
    aDisposeListeners.disposeAndClear(aEvt);

    mpSdCustomShow = nullptr;
}

void SAL_CALL SdXCustomPresentation::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    if( bDisposing )
        throw lang::DisposedException();

    aDisposeListeners.addInterface(xListener);
}

void SAL_CALL SdXCustomPresentation::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
{
    if( !bDisposing )
        aDisposeListeners.removeInterface(aListener);
}

/*===========================================================================*
 *  class SdXCustomPresentationAccess : public XCustomPresentationAccess,    *
 *                                      public UsrObject                     *
 *===========================================================================*/

SdXCustomPresentationAccess::SdXCustomPresentationAccess(SdXImpressDocument& rMyModel) throw()
: mrModel(rMyModel)
{
}

SdXCustomPresentationAccess::~SdXCustomPresentationAccess() throw()
{
}

// XServiceInfo
OUString SAL_CALL SdXCustomPresentationAccess::getImplementationName()
{
    return OUString( "SdXCustomPresentationAccess" );
}

sal_Bool SAL_CALL SdXCustomPresentationAccess::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SdXCustomPresentationAccess::getSupportedServiceNames()
{
    const OUString aNS( "com.sun.star.presentation.CustomPresentationAccess" );
    uno::Sequence< OUString > aSeq( &aNS, 1 );
    return aSeq;
}

// XSingleServiceFactory
uno::Reference< uno::XInterface > SAL_CALL SdXCustomPresentationAccess::createInstance()
{
    uno::Reference< uno::XInterface >  xRef( static_cast<cppu::OWeakObject*>(new SdXCustomPresentation()) );
    return xRef;
}

uno::Reference< uno::XInterface > SAL_CALL SdXCustomPresentationAccess::createInstanceWithArguments( const uno::Sequence< uno::Any >& )
{
    return createInstance();
}

// XNameContainer
void SAL_CALL SdXCustomPresentationAccess::insertByName( const OUString& aName, const uno::Any& aElement )
{
    SolarMutexGuard aGuard;

    // get the documents custom show list
    SdCustomShowList* pList = nullptr;
    if(mrModel.GetDoc())
        pList = mrModel.GetDoc()->GetCustomShowList(true);

    // no list, no cookies
    if( nullptr == pList)
        throw uno::RuntimeException();

    // do we have an container::XIndexContainer?
    SdXCustomPresentation* pXShow = nullptr;

    uno::Reference< container::XIndexContainer > xContainer;
    if( (aElement >>= xContainer) && xContainer.is() )
        pXShow = SdXCustomPresentation::getImplementation(xContainer);

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

    pList->push_back(pShow);

    mrModel.SetModified();
}

void SAL_CALL SdXCustomPresentationAccess::removeByName( const OUString& Name )
{
    SolarMutexGuard aGuard;

    SdCustomShow* pShow = getSdCustomShow(Name);

    SdCustomShowList* pList = GetCustomShowList();
    if(pList && pShow)
        delete pList->Remove( pShow );
    else
        throw container::NoSuchElementException();

    mrModel.SetModified();
}

// XNameReplace
void SAL_CALL SdXCustomPresentationAccess::replaceByName( const OUString& aName, const uno::Any& aElement )
{
    removeByName( aName );
    insertByName( aName, aElement );
}

// XNameAccess
uno::Any SAL_CALL SdXCustomPresentationAccess::getByName( const OUString& aName )
{
    SolarMutexGuard aGuard;

    uno::Any aAny;

    SdCustomShow* pShow = getSdCustomShow(aName);
    if(pShow)
    {
        uno::Reference< container::XIndexContainer >  xRef( pShow->getUnoCustomShow(), uno::UNO_QUERY );
        aAny <<= xRef;
    }
    else
    {
        throw container::NoSuchElementException();
    }

    return aAny;
}

uno::Sequence< OUString > SAL_CALL SdXCustomPresentationAccess::getElementNames()
{
    SolarMutexGuard aGuard;

    SdCustomShowList* pList = GetCustomShowList();
    const sal_uInt32 nCount = pList ? pList->size() : 0;

    uno::Sequence< OUString > aSequence( nCount );
    OUString* pStringList = aSequence.getArray();

    sal_uInt32 nIdx = 0;
    while( nIdx < nCount )
    {
        const SdCustomShow* pShow = (*pList)[nIdx];
        pStringList[nIdx] = pShow->GetName();
        nIdx++;
    }

    return aSequence;
}

sal_Bool SAL_CALL SdXCustomPresentationAccess::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    return getSdCustomShow(aName) != nullptr;
}

// XElementAccess
uno::Type SAL_CALL SdXCustomPresentationAccess::getElementType()
{
    return cppu::UnoType<container::XIndexContainer>::get();
}

sal_Bool SAL_CALL SdXCustomPresentationAccess::hasElements()
{
    SolarMutexGuard aGuard;

    SdCustomShowList* pList = GetCustomShowList();
    return pList && !pList->empty();
}

SdCustomShow * SdXCustomPresentationAccess::getSdCustomShow( const OUString& rName ) const throw()
{
    sal_uInt32 nIdx = 0;

    SdCustomShowList* pList = GetCustomShowList();
    const sal_uInt32 nCount = pList ? pList->size() : 0;

    while( nIdx < nCount )
    {
        SdCustomShow* pShow = (*pList)[nIdx];
        if( pShow->GetName() == rName )
            return pShow;
        nIdx++;
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
