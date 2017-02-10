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
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <svl/style.hxx>

#include <svx/unoprov.hxx>

#include "strings.hrc"
#include "stlfamily.hxx"
#include "stlsheet.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "glob.hxx"

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
    SdrPageWeakRef mxMasterPage;
    OUString maLayoutName;

    PresStyleMap& getStyleSheets();
    rtl::Reference< SfxStyleSheetPool > mxPool;

private:
    PresStyleMap maStyleSheets;
};

PresStyleMap& SdStyleFamilyImpl::getStyleSheets()
{
    if( mxMasterPage.is() && (mxMasterPage->GetLayoutName() != maLayoutName) )
    {
        maLayoutName = mxMasterPage->GetLayoutName();

        OUString aLayoutName( maLayoutName );
        const sal_Int32 nLen = aLayoutName.indexOf(SD_LT_SEPARATOR ) + 4;
        aLayoutName = aLayoutName.copy(0, nLen );

        if( (maStyleSheets.empty()) || !(*maStyleSheets.begin()).second->GetName().startsWith( aLayoutName) )
        {
            maStyleSheets.clear();

            // The iterator will return only style sheets of family master page
            std::shared_ptr<SfxStyleSheetIterator> aSSSIterator = std::make_shared<SfxStyleSheetIterator>(mxPool.get(), SD_STYLE_FAMILY_MASTERPAGE);
            for ( SfxStyleSheetBase* pStyle = aSSSIterator->First(); pStyle;
                                     pStyle = aSSSIterator->Next() )
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
: mnFamily( nFamily )
, mxPool( xPool )
{
}

SdStyleFamily::SdStyleFamily( const rtl::Reference< SfxStyleSheetPool >& xPool, const SdPage* pMasterPage )
: mnFamily( SD_STYLE_FAMILY_MASTERPAGE )
, mxPool( xPool )
, mpImpl( new SdStyleFamilyImpl() )
{
    mpImpl->mxMasterPage.reset( const_cast< SdPage* >( pMasterPage ) );
    mpImpl->mxPool = xPool;
}

SdStyleFamily::~SdStyleFamily()
{
    DBG_ASSERT( !mxPool.is(), "SdStyleFamily::~SdStyleFamily(), dispose me first!" );
}

void SdStyleFamily::throwIfDisposed() const
{
    if( !mxPool.is() )
        throw DisposedException();
}

SdStyleSheet* SdStyleFamily::GetValidNewSheet( const Any& rElement )
{
    Reference< XStyle > xStyle( rElement, UNO_QUERY );
    SdStyleSheet* pStyle = static_cast< SdStyleSheet* >( xStyle.get() );

    if( pStyle == nullptr || (pStyle->GetFamily() != mnFamily) || (&pStyle->GetPool() != mxPool.get()) || (mxPool->Find( pStyle->GetName(), mnFamily) != nullptr) )
        throw IllegalArgumentException();

    return pStyle;
}

SdStyleSheet* SdStyleFamily::GetSheetByName( const OUString& rName )
{
    SdStyleSheet* pRet = nullptr;
    if( !rName.isEmpty() )
    {
        if( mnFamily == SD_STYLE_FAMILY_MASTERPAGE )
        {
            PresStyleMap& rStyleMap = mpImpl->getStyleSheets();
            PresStyleMap::iterator iter( rStyleMap.find(rName) );
            if( iter != rStyleMap.end() )
                pRet = (*iter).second.get();
        }
        else
        {
            std::shared_ptr<SfxStyleSheetIterator> aSSSIterator = std::make_shared<SfxStyleSheetIterator>(mxPool.get(), mnFamily);
            for ( SfxStyleSheetBase* pStyle = aSSSIterator->First(); pStyle;
                                     pStyle = aSSSIterator->Next() )
            {
                // we assume that we have only SdStyleSheets
                SdStyleSheet* pSdStyle = static_cast< SdStyleSheet* >( pStyle );
                if( pSdStyle && pSdStyle->GetApiName() == rName)
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
    return OUString( "SdStyleFamily" );
}

sal_Bool SAL_CALL SdStyleFamily::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL SdStyleFamily::getSupportedServiceNames()
{
    OUString aServiceName( "com.sun.star.style.StyleFamily" );
    Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

// XNamed
OUString SAL_CALL SdStyleFamily::getName()
{
    if( mnFamily == SD_STYLE_FAMILY_MASTERPAGE )
    {
        SdPage* pPage = static_cast< SdPage* >( mpImpl->mxMasterPage.get() );
        if( pPage == nullptr )
            throw DisposedException();

        OUString aLayoutName( pPage->GetLayoutName() );
        const OUString aSep( SD_LT_SEPARATOR );
        sal_Int32 nIndex = aLayoutName.indexOf(aSep);
        if( nIndex != -1 )
            aLayoutName = aLayoutName.copy(0, nIndex);

        return OUString( aLayoutName );
    }
    else
    {
        return SdStyleSheet::GetFamilyString( mnFamily );
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

    if( mnFamily == SD_STYLE_FAMILY_MASTERPAGE )
    {
        PresStyleMap& rStyleMap = mpImpl->getStyleSheets();
        Sequence< OUString > aNames( rStyleMap.size() );

        PresStyleMap::iterator iter( rStyleMap.begin() );
        OUString* pNames = aNames.getArray();
        while( iter != rStyleMap.end() )
        {
            rtl::Reference< SdStyleSheet > xStyle( (*iter++).second );
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
        std::shared_ptr<SfxStyleSheetIterator> aSSSIterator = std::make_shared<SfxStyleSheetIterator>(mxPool.get(), mnFamily);
        for ( SfxStyleSheetBase* pStyle = aSSSIterator->First(); pStyle;
                                 pStyle = aSSSIterator->Next() )
        {
            // we assume that we have only SdStyleSheets
            SdStyleSheet* pSdStyle = static_cast< SdStyleSheet* >( pStyle );
            if( pSdStyle )
            {
                aNames.push_back( pSdStyle->GetApiName() );
            }
        }
        return Sequence< OUString >( &(*aNames.begin()), aNames.size() );
    }
}

sal_Bool SAL_CALL SdStyleFamily::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if( !aName.isEmpty() )
    {
        if( mnFamily == SD_STYLE_FAMILY_MASTERPAGE )
        {
            PresStyleMap& rStyleSheets = mpImpl->getStyleSheets();
            PresStyleMap::iterator iter( rStyleSheets.find(aName) );
            return iter != rStyleSheets.end();
        }
        else
        {
            std::shared_ptr<SfxStyleSheetIterator> aSSSIterator = std::make_shared<SfxStyleSheetIterator>(mxPool.get(), mnFamily);
            for ( SfxStyleSheetBase* pStyle = aSSSIterator->First(); pStyle;
                                     pStyle = aSSSIterator->Next() )
            {
                // we assume that we have only SdStyleSheets
                SdStyleSheet* pSdStyle = static_cast< SdStyleSheet* >( pStyle );
                if( pSdStyle )
                {
                    if (pSdStyle->GetApiName() == aName)
                    {
                        return true;
                    }
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

    if( mnFamily == SD_STYLE_FAMILY_MASTERPAGE )
    {
        return true;
    }
    else
    {
        std::shared_ptr<SfxStyleSheetIterator> aSSSIterator = std::make_shared<SfxStyleSheetIterator>(mxPool.get(), mnFamily);
        if (aSSSIterator->First())
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
    if( mnFamily == SD_STYLE_FAMILY_MASTERPAGE )
    {
        return mpImpl->getStyleSheets().size();
    }
    else
    {
        std::shared_ptr<SfxStyleSheetIterator> aSSSIterator = std::make_shared<SfxStyleSheetIterator>(mxPool.get(), mnFamily);
        for ( SfxStyleSheetBase* pStyle = aSSSIterator->First(); pStyle;
                                 pStyle = aSSSIterator->Next() )
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
        if( mnFamily == SD_STYLE_FAMILY_MASTERPAGE )
        {
            PresStyleMap& rStyleSheets = mpImpl->getStyleSheets();
            if( !rStyleSheets.empty() )
            {
                PresStyleMap::iterator iter( rStyleSheets.begin() );
                while( Index-- && (iter != rStyleSheets.end()) )
                    ++iter;

                if( (Index==-1) && (iter != rStyleSheets.end()) )
                    return Any( Reference< XStyle >( (*iter).second.get() ) );
            }
        }
        else
        {
            std::shared_ptr<SfxStyleSheetIterator> aSSSIterator = std::make_shared<SfxStyleSheetIterator>(mxPool.get(), mnFamily);
            for ( SfxStyleSheetBase* pStyle = aSSSIterator->First(); pStyle;
                                     pStyle = aSSSIterator->Next() )
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
    mxPool->Insert( pStyle );
}

void SAL_CALL SdStyleFamily::removeByName( const OUString& rName )
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    SdStyleSheet* pStyle = GetSheetByName( rName );

    if( !pStyle->IsUserDefined() )
        throw WrappedTargetException();

    mxPool->Remove( pStyle );
}

// XNameReplace

void SAL_CALL SdStyleFamily::replaceByName( const OUString& rName, const Any& aElement )
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    SdStyleSheet* pOldStyle = GetSheetByName( rName );
    SdStyleSheet* pNewStyle = GetValidNewSheet( aElement );

    mxPool->Remove( pOldStyle );
    mxPool->Insert( pNewStyle );
}

// XSingleServiceFactory

Reference< XInterface > SAL_CALL SdStyleFamily::createInstance()
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if( mnFamily == SD_STYLE_FAMILY_MASTERPAGE )
    {
        throw IllegalAccessException();
    }
    else
    {
        return Reference< XInterface >( static_cast< XStyle* >( SdStyleSheet::CreateEmptyUserStyle( *mxPool.get(), mnFamily ) ) );
    }
}

Reference< XInterface > SAL_CALL SdStyleFamily::createInstanceWithArguments( const Sequence< Any >&  )
{
    return createInstance();
}

// XComponent

void SAL_CALL SdStyleFamily::dispose(  )
{
    if( mxPool.is() )
        mxPool.clear();

    if( mpImpl )
    {
        mpImpl.reset();
    }
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
    if ( PropertyName == "DisplayName" )
    {
        SolarMutexGuard aGuard;
        OUString sDisplayName;
        switch( mnFamily )
        {
            case SD_STYLE_FAMILY_MASTERPAGE:    sDisplayName = getName(); break;
            case SD_STYLE_FAMILY_CELL:          sDisplayName = SD_RESSTR(STR_CELL_STYLE_FAMILY); break;
            default:                            sDisplayName = SD_RESSTR(STR_GRAPHICS_STYLE_FAMILY); break;
        }
        return Any( sDisplayName );
    }
    else
    {
        throw UnknownPropertyException( "unknown property: " + PropertyName, static_cast<OWeakObject *>(this) );
    }
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
