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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <utility>
#include <vcl/svapp.hxx>

#include <svl/style.hxx>

#include <tools/debug.hxx>
#include <unotools/weakref.hxx>

#include <strings.hrc>
#include <stlfamily.hxx>
#include <stlsheet.hxx>
#include <sdresid.hxx>
#include <sdpage.hxx>
#include <glob.hxx>

#include <map>
#include <memory>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;

typedef std::map< OUString, rtl::Reference< SdStyleSheet > > PresStyleMap;

struct SdStyleFamilyImpl
{
    SfxStyleFamily mnFamily;
    rtl::Reference< SfxStyleSheetPool > mxPool;
    unotools::WeakReference<SdPage> mxMasterPage;
    OUString maLayoutName;

    PresStyleMap& getStyleSheets();

private:
    PresStyleMap maStyleSheets;
};

PresStyleMap& SdStyleFamilyImpl::getStyleSheets()
{
    auto pMasterPage = mxMasterPage.get();
    if (!pMasterPage)
        return maStyleSheets;

    if (pMasterPage->GetLayoutName() != maLayoutName )
    {
        maLayoutName = pMasterPage->GetLayoutName();

        OUString aLayoutName( maLayoutName );
        const sal_Int32 nLen = aLayoutName.indexOf(SD_LT_SEPARATOR ) + 4;
        aLayoutName = aLayoutName.copy(0, nLen );

        if( (maStyleSheets.empty()) || !(*maStyleSheets.begin()).second->GetName().startsWith( aLayoutName) )
        {
            maStyleSheets.clear();

            // The iterator will return only style sheets of family master page
            SfxStyleSheetIterator aSSSIterator(mxPool.get(), SfxStyleFamily::Page);
            for ( SfxStyleSheetBase* pStyle = aSSSIterator.First(); pStyle;
                                     pStyle = aSSSIterator.Next() )
            {
                // we assume that we have only SdStyleSheets
                SdStyleSheet* pSdStyle = static_cast< SdStyleSheet* >( pStyle );
                if (pSdStyle->GetName().startsWith(aLayoutName))
                {
                    maStyleSheets[ pSdStyle->GetApiName() ].set( pSdStyle );
                }
            }
        }
    }

    return maStyleSheets;
}

SdStyleFamily::SdStyleFamily( const rtl::Reference< SfxStyleSheetPool >& xPool, SfxStyleFamily nFamily )
: mpImpl( new SdStyleFamilyImpl )
{
    mpImpl->mnFamily = nFamily;
    mpImpl->mxPool = xPool;
}

SdStyleFamily::SdStyleFamily( const rtl::Reference< SfxStyleSheetPool >& xPool, const SdPage* pMasterPage )
: mpImpl( new SdStyleFamilyImpl )
{
    mpImpl->mnFamily = SfxStyleFamily::Page;
    mpImpl->mxPool = xPool;
    mpImpl->mxMasterPage = const_cast< SdPage* >( pMasterPage );
}

SdStyleFamily::~SdStyleFamily()
{
    DBG_ASSERT( !mpImpl->mxPool.is(), "SdStyleFamily::~SdStyleFamily(), dispose me first!" );
}

void SdStyleFamily::throwIfDisposed() const
{
    if( !mpImpl->mxPool.is() )
        throw DisposedException();
}

SdStyleSheet* SdStyleFamily::GetValidNewSheet( const Any& rElement )
{
    Reference< XStyle > xStyle( rElement, UNO_QUERY );
    SdStyleSheet* pStyle = static_cast< SdStyleSheet* >( xStyle.get() );

    if( pStyle == nullptr
       || (pStyle->GetFamily() != mpImpl->mnFamily)
       || (pStyle->GetPool() != mpImpl->mxPool.get())
       || (mpImpl->mxPool->Find( pStyle->GetName(), mpImpl->mnFamily) != nullptr) )
        throw IllegalArgumentException();

    return pStyle;
}

SdStyleSheet* SdStyleFamily::GetSheetByName( const OUString& rName )
{
    SdStyleSheet* pRet = nullptr;
    if( !rName.isEmpty() )
    {
        if( mpImpl->mnFamily == SfxStyleFamily::Page )
        {
            PresStyleMap& rStyleMap = mpImpl->getStyleSheets();
            PresStyleMap::iterator iter( rStyleMap.find(rName) );
            if( iter != rStyleMap.end() )
                pRet = (*iter).second.get();
        }
        else
        {
            SfxStyleSheetIterator aSSSIterator(mpImpl->mxPool.get(), mpImpl->mnFamily);
            for ( SfxStyleSheetBase* pStyle = aSSSIterator.First(); pStyle;
                                     pStyle = aSSSIterator.Next() )
            {
                // we assume that we have only SdStyleSheets
                SdStyleSheet* pSdStyle = static_cast< SdStyleSheet* >( pStyle );
                if (pSdStyle->GetApiName() == rName)
                {
                    pRet = pSdStyle;
                    break;
                }
            }
        }
    }
    if( pRet )
        return pRet;

    throw NoSuchElementException();
}

// XServiceInfo
OUString SAL_CALL SdStyleFamily::getImplementationName()
{
    return u"SdStyleFamily"_ustr;
}

sal_Bool SAL_CALL SdStyleFamily::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL SdStyleFamily::getSupportedServiceNames()
{
    return { u"com.sun.star.style.StyleFamily"_ustr };
}

// XNamed
OUString SAL_CALL SdStyleFamily::getName()
{
    if( mpImpl->mnFamily == SfxStyleFamily::Page )
    {
        rtl::Reference<SdPage> pPage = mpImpl->mxMasterPage.get();
        if( pPage == nullptr )
            throw DisposedException();

        const OUString& rLayoutName = pPage->GetLayoutName();
        sal_Int32 nIndex = rLayoutName.indexOf(SD_LT_SEPARATOR);
        if (nIndex != -1)
            return rLayoutName.copy(0, nIndex);
        return rLayoutName;
    }
    else
    {
        return SdStyleSheet::GetFamilyString( mpImpl->mnFamily );
    }
}

void SAL_CALL SdStyleFamily::setName( const OUString& )
{
}

// XNameAccess

Any SAL_CALL SdStyleFamily::getByName( const OUString& rName )
{
    SolarMutexGuard aGuard;
    throwIfDisposed();
    return Any( Reference< XStyle >( static_cast<SfxUnoStyleSheet*>(GetSheetByName( rName )) ) );
}

Sequence< OUString > SAL_CALL SdStyleFamily::getElementNames()
{
    SolarMutexGuard aGuard;

    throwIfDisposed();

    if( mpImpl->mnFamily == SfxStyleFamily::Page )
    {
        PresStyleMap& rStyleMap = mpImpl->getStyleSheets();
        Sequence< OUString > aNames( rStyleMap.size() );

        OUString* pNames = aNames.getArray();
        for( const auto& rEntry : rStyleMap )
        {
            rtl::Reference< SdStyleSheet > xStyle( rEntry.second );
            if( xStyle.is() )
            {
                *pNames++ = xStyle->GetApiName();
            }
        }

        return aNames;
    }
    else
    {
        std::vector< OUString > aNames;
        SfxStyleSheetIterator aSSSIterator(mpImpl->mxPool.get(), mpImpl->mnFamily);
        for ( SfxStyleSheetBase* pStyle = aSSSIterator.First(); pStyle;
                                 pStyle = aSSSIterator.Next() )
        {
            // we assume that we have only SdStyleSheets
            SdStyleSheet* pSdStyle = static_cast< SdStyleSheet* >( pStyle );
            aNames.push_back(pSdStyle->GetApiName());
        }
        return Sequence< OUString >( aNames.data(), aNames.size() );
    }
}

sal_Bool SAL_CALL SdStyleFamily::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if( !aName.isEmpty() )
    {
        if( mpImpl->mnFamily == SfxStyleFamily::Page )
        {
            PresStyleMap& rStyleSheets = mpImpl->getStyleSheets();
            PresStyleMap::iterator iter( rStyleSheets.find(aName) );
            return iter != rStyleSheets.end();
        }
        else
        {
            SfxStyleSheetIterator aSSSIterator(mpImpl->mxPool.get(), mpImpl->mnFamily);
            for ( SfxStyleSheetBase* pStyle = aSSSIterator.First(); pStyle;
                                     pStyle = aSSSIterator.Next() )
            {
                // we assume that we have only SdStyleSheets
                SdStyleSheet* pSdStyle = static_cast< SdStyleSheet* >( pStyle );
                if (pSdStyle->GetApiName() == aName)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

// XElementAccess

Type SAL_CALL SdStyleFamily::getElementType()
{
    return cppu::UnoType<XStyle>::get();
}

sal_Bool SAL_CALL SdStyleFamily::hasElements()
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if( mpImpl->mnFamily == SfxStyleFamily::Page )
    {
        return true;
    }
    else
    {
        SfxStyleSheetIterator aSSSIterator(mpImpl->mxPool.get(), mpImpl->mnFamily);
        if (aSSSIterator.First())
        {
            return true;
        }
    }

    return false;
}

// XIndexAccess

sal_Int32 SAL_CALL SdStyleFamily::getCount()
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    sal_Int32 nCount = 0;
    if( mpImpl->mnFamily == SfxStyleFamily::Page )
    {
        return mpImpl->getStyleSheets().size();
    }
    else
    {
        SfxStyleSheetIterator aSSSIterator(mpImpl->mxPool.get(), mpImpl->mnFamily);
        for ( SfxStyleSheetBase* pStyle = aSSSIterator.First(); pStyle;
                                 pStyle = aSSSIterator.Next() )
        {
            nCount++;
        }
    }

    return nCount;
}

Any SAL_CALL SdStyleFamily::getByIndex( sal_Int32 Index )
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if( Index >= 0 )
    {
        if( mpImpl->mnFamily == SfxStyleFamily::Page )
        {
            PresStyleMap& rStyleSheets = mpImpl->getStyleSheets();
            if( Index < static_cast<sal_Int32>(rStyleSheets.size()) )
            {
                PresStyleMap::iterator iter( rStyleSheets.begin() );
                std::advance(iter, Index);
                return Any( Reference< XStyle >( (*iter).second ) );
            }
        }
        else
        {
            SfxStyleSheetIterator aSSSIterator(mpImpl->mxPool.get(), mpImpl->mnFamily);
            for ( SfxStyleSheetBase* pStyle = aSSSIterator.First(); pStyle;
                                     pStyle = aSSSIterator.Next() )
            {
                // we assume that we have only SdStyleSheets
                SdStyleSheet* pSdStyle = static_cast< SdStyleSheet* >( pStyle );
                if( Index-- == 0 )
                {
                    return Any( Reference< XStyle >( pSdStyle ) );
                }
            }
        }
    }

    throw IndexOutOfBoundsException();
}

// XNameContainer

void SAL_CALL SdStyleFamily::insertByName( const OUString& rName, const Any& rElement )
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if(rName.isEmpty())
        throw IllegalArgumentException();

    SdStyleSheet* pStyle = GetValidNewSheet( rElement );
    if( !pStyle->SetName( rName ) )
        throw ElementExistException();

    pStyle->SetApiName( rName );
    mpImpl->mxPool->Insert( pStyle );
}

void SAL_CALL SdStyleFamily::removeByName( const OUString& rName )
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    SdStyleSheet* pStyle = GetSheetByName( rName );

    if( !pStyle->IsUserDefined() )
        throw WrappedTargetException();

    mpImpl->mxPool->Remove( pStyle );
}

// XNameReplace

void SAL_CALL SdStyleFamily::replaceByName( const OUString& rName, const Any& aElement )
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    SdStyleSheet* pOldStyle = GetSheetByName( rName );
    SdStyleSheet* pNewStyle = GetValidNewSheet( aElement );

    mpImpl->mxPool->Remove( pOldStyle );
    mpImpl->mxPool->Insert( pNewStyle );
}

// XSingleServiceFactory

Reference< XInterface > SAL_CALL SdStyleFamily::createInstance()
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if( mpImpl->mnFamily == SfxStyleFamily::Page )
    {
        throw IllegalAccessException();
    }
    return Reference<XInterface>(
        static_cast<XStyle*>(SdStyleSheet::CreateEmptyUserStyle(*mpImpl->mxPool, mpImpl->mnFamily).get()));
}

Reference< XInterface > SAL_CALL SdStyleFamily::createInstanceWithArguments( const Sequence< Any >&  )
{
    return createInstance();
}

// XComponent

void SAL_CALL SdStyleFamily::dispose(  )
{
    if( mpImpl->mxPool.is() )
        mpImpl->mxPool.clear();
}

void SAL_CALL SdStyleFamily::addEventListener( const Reference< XEventListener >&  )
{
}

void SAL_CALL SdStyleFamily::removeEventListener( const Reference< XEventListener >&  )
{
}

// XPropertySet

Reference<XPropertySetInfo> SdStyleFamily::getPropertySetInfo()
{
    OSL_FAIL( "###unexpected!" );
    return Reference<XPropertySetInfo>();
}

void SdStyleFamily::setPropertyValue( const OUString& , const Any&  )
{
    OSL_FAIL( "###unexpected!" );
}

Any SdStyleFamily::getPropertyValue( const OUString& PropertyName )
{
    if ( PropertyName != "DisplayName" )
    {
        throw UnknownPropertyException( "unknown property: " + PropertyName, static_cast<OWeakObject *>(this) );
    }

    SolarMutexGuard aGuard;
    OUString sDisplayName;
    switch( mpImpl->mnFamily )
    {
        case SfxStyleFamily::Page:    sDisplayName = getName(); break;
        case SfxStyleFamily::Frame:          sDisplayName = SdResId(STR_CELL_STYLE_FAMILY); break;
        default:                            sDisplayName = SdResId(STR_GRAPHICS_STYLE_FAMILY); break;
    }
    return Any( sDisplayName );
}

void SdStyleFamily::addPropertyChangeListener( const OUString& , const Reference<XPropertyChangeListener>&  )
{
    OSL_FAIL( "###unexpected!" );
}

void SdStyleFamily::removePropertyChangeListener( const OUString& , const Reference<XPropertyChangeListener>&  )
{
    OSL_FAIL( "###unexpected!" );
}

void SdStyleFamily::addVetoableChangeListener( const OUString& , const Reference<XVetoableChangeListener>& )
{
    OSL_FAIL( "###unexpected!" );
}

void SdStyleFamily::removeVetoableChangeListener( const OUString& , const Reference<XVetoableChangeListener>&  )
{
    OSL_FAIL( "###unexpected!" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
