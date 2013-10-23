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
#include <cppuhelper/supportsservice.hxx>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <svl/style.hxx>

#include <svx/unoprov.hxx>

#include "../ui/inc/strings.hrc"
#include "stlfamily.hxx"
#include "stlsheet.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "glob.hxx"

#include <map>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;

// ----------------------------------------------------------

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

            const SfxStyles& rStyles = mxPool->GetStyles();
            for( SfxStyles::const_iterator iter( rStyles.begin() ); iter != rStyles.end(); ++iter )
            {
                SdStyleSheet* pStyle = static_cast< SdStyleSheet* >( (*iter).get() );
                if( pStyle && (pStyle->GetFamily() == SD_STYLE_FAMILY_MASTERPAGE) && pStyle->GetName().startsWith(aLayoutName) )
                    maStyleSheets[ pStyle->GetApiName() ] = rtl::Reference< SdStyleSheet >( pStyle );
            }
        }
    }

    return maStyleSheets;
}

// ----------------------------------------------------------

SdStyleFamily::SdStyleFamily( const rtl::Reference< SfxStyleSheetPool >& xPool, SfxStyleFamily nFamily )
: mnFamily( nFamily )
, mxPool( xPool )
, mpImpl( 0 )
{
}

// ----------------------------------------------------------

SdStyleFamily::SdStyleFamily( const rtl::Reference< SfxStyleSheetPool >& xPool, const SdPage* pMasterPage )
: mnFamily( SD_STYLE_FAMILY_MASTERPAGE )
, mxPool( xPool )
, mpImpl( new SdStyleFamilyImpl() )
{
    mpImpl->mxMasterPage.reset( const_cast< SdPage* >( pMasterPage ) );
    mpImpl->mxPool = xPool;
}

// ----------------------------------------------------------

SdStyleFamily::~SdStyleFamily()
{
    DBG_ASSERT( !mxPool.is(), "SdStyleFamily::~SdStyleFamily(), dispose me first!" );
    delete mpImpl;
}

// ----------------------------------------------------------

void SdStyleFamily::throwIfDisposed() const throw(RuntimeException)
{
    if( !mxPool.is() )
        throw DisposedException();
}

// ----------------------------------------------------------

SdStyleSheet* SdStyleFamily::GetValidNewSheet( const Any& rElement ) throw(IllegalArgumentException)
{
    Reference< XStyle > xStyle( rElement, UNO_QUERY );
    SdStyleSheet* pStyle = static_cast< SdStyleSheet* >( xStyle.get() );

    if( pStyle == 0 || (pStyle->GetFamily() != mnFamily) || (&pStyle->GetPool() != mxPool.get()) || (mxPool->Find( pStyle->GetName(), mnFamily) != 0) )
        throw IllegalArgumentException();

    return pStyle;
}

// ----------------------------------------------------------

SdStyleSheet* SdStyleFamily::GetSheetByName( const OUString& rName ) throw(NoSuchElementException, WrappedTargetException )
{
    SdStyleSheet* pRet = 0;
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
            const SfxStyles& rStyles = mxPool->GetStyles();
            for( SfxStyles::const_iterator iter( rStyles.begin() ); iter != rStyles.end(); ++iter )
            {
                SdStyleSheet* pStyle = static_cast< SdStyleSheet* >( (*iter).get() );
                if( pStyle && (pStyle->GetFamily() == mnFamily) && (pStyle->GetApiName() == rName) )
                {
                    pRet = pStyle;
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
OUString SAL_CALL SdStyleFamily::getImplementationName() throw(RuntimeException)
{
    return OUString( "SdStyleFamily" );
}

sal_Bool SAL_CALL SdStyleFamily::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL SdStyleFamily::getSupportedServiceNames() throw(RuntimeException)
{
    OUString aServiceName( "com.sun.star.style.StyleFamily" );
    Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

// XNamed
OUString SAL_CALL SdStyleFamily::getName() throw (RuntimeException)
{
    if( mnFamily == SD_STYLE_FAMILY_MASTERPAGE )
    {
        SdPage* pPage = static_cast< SdPage* >( mpImpl->mxMasterPage.get() );
        if( pPage == 0 )
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

// ----------------------------------------------------------

void SAL_CALL SdStyleFamily::setName( const OUString& ) throw (RuntimeException)
{
}

// ----------------------------------------------------------
// XNameAccess
// ----------------------------------------------------------

Any SAL_CALL SdStyleFamily::getByName( const OUString& rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    throwIfDisposed();
    return Any( Reference< XStyle >( static_cast<SfxUnoStyleSheet*>(GetSheetByName( rName )) ) );
}

// ----------------------------------------------------------

Sequence< OUString > SAL_CALL SdStyleFamily::getElementNames() throw(RuntimeException)
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
        const SfxStyles& rStyles = mxPool->GetStyles();
        for( SfxStyles::const_iterator iter( rStyles.begin() ); iter != rStyles.end(); ++iter )
        {
            SdStyleSheet* pStyle = static_cast< SdStyleSheet* >( (*iter).get() );
            if( pStyle && (pStyle->GetFamily() == mnFamily) )
                aNames.push_back( pStyle->GetApiName() );
        }
        return Sequence< OUString >( &(*aNames.begin()), aNames.size() );
    }
}

// ----------------------------------------------------------

sal_Bool SAL_CALL SdStyleFamily::hasByName( const OUString& aName ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if( !aName.isEmpty() )
    {
        if( mnFamily == SD_STYLE_FAMILY_MASTERPAGE )
        {
            PresStyleMap& rStyleSheets = mpImpl->getStyleSheets();
            PresStyleMap::iterator iter( rStyleSheets.find(aName) );
            return ( iter != rStyleSheets.end() ) ? sal_True : sal_False;
        }
        else
        {
            const SfxStyles& rStyles = mxPool->GetStyles();
            for( SfxStyles::const_iterator iter( rStyles.begin() ); iter != rStyles.end(); ++iter )
            {
                SdStyleSheet* pStyle = static_cast< SdStyleSheet* >( (*iter).get() );
                if( pStyle && (pStyle->GetFamily() == mnFamily) && ( pStyle->GetApiName() == aName ) )
                    return sal_True;
            }
        }
    }

    return sal_False;
}

// ----------------------------------------------------------
// XElementAccess
// ----------------------------------------------------------

Type SAL_CALL SdStyleFamily::getElementType() throw(RuntimeException)
{
    return XStyle::static_type();
}

// ----------------------------------------------------------

sal_Bool SAL_CALL SdStyleFamily::hasElements() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if( mnFamily == SD_STYLE_FAMILY_MASTERPAGE )
    {
        return sal_True;
    }
    else
    {
        const SfxStyles& rStyles = mxPool->GetStyles();
        for( SfxStyles::const_iterator iter( rStyles.begin() ); iter != rStyles.end(); ++iter )
        {
            SdStyleSheet* pStyle = static_cast< SdStyleSheet* >( (*iter).get() );
            if( pStyle && (pStyle->GetFamily() == mnFamily) )
                return sal_True;
        }
    }

    return sal_False;
}

// ----------------------------------------------------------
// XIndexAccess
// ----------------------------------------------------------

sal_Int32 SAL_CALL SdStyleFamily::getCount() throw(RuntimeException)
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
        const SfxStyles& rStyles = mxPool->GetStyles();
        for( SfxStyles::const_iterator iter( rStyles.begin() ); iter != rStyles.end(); ++iter )
        {
            SdStyleSheet* pStyle = static_cast< SdStyleSheet* >( (*iter).get() );
            if( pStyle && (pStyle->GetFamily() == mnFamily) )
                nCount++;
        }
    }

    return nCount;
}

// ----------------------------------------------------------

Any SAL_CALL SdStyleFamily::getByIndex( sal_Int32 Index ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
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
            const SfxStyles& rStyles = mxPool->GetStyles();
            for( SfxStyles::const_iterator iter( rStyles.begin() ); iter != rStyles.end(); ++iter )
            {
                SdStyleSheet* pStyle = static_cast< SdStyleSheet* >( (*iter).get() );
                if( pStyle && (pStyle->GetFamily() == mnFamily) )
                {
                    if( Index-- == 0 )
                        return Any( Reference< XStyle >( pStyle ) );
                }
            }
        }
    }

    throw IndexOutOfBoundsException();
}

// ----------------------------------------------------------
// XNameContainer
// ----------------------------------------------------------

void SAL_CALL SdStyleFamily::insertByName( const OUString& rName, const Any& rElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
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

// ----------------------------------------------------------

void SAL_CALL SdStyleFamily::removeByName( const OUString& rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    SdStyleSheet* pStyle = GetSheetByName( rName );

    if( !pStyle->IsUserDefined() )
        throw WrappedTargetException();

    mxPool->Remove( pStyle );
}

// ----------------------------------------------------------
// XNameReplace
// ----------------------------------------------------------

void SAL_CALL SdStyleFamily::replaceByName( const OUString& rName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    SdStyleSheet* pOldStyle = GetSheetByName( rName );
    SdStyleSheet* pNewStyle = GetValidNewSheet( aElement );

    mxPool->Remove( pOldStyle );
    mxPool->Insert( pNewStyle );
}

// ----------------------------------------------------------
// XSingleServiceFactory
// ----------------------------------------------------------

Reference< XInterface > SAL_CALL SdStyleFamily::createInstance() throw(Exception, RuntimeException)
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

// ----------------------------------------------------------

Reference< XInterface > SAL_CALL SdStyleFamily::createInstanceWithArguments( const Sequence< Any >&  ) throw(Exception, RuntimeException)
{
    return createInstance();
}

// ----------------------------------------------------------
// XComponent
// ----------------------------------------------------------

void SAL_CALL SdStyleFamily::dispose(  ) throw (RuntimeException)
{
    if( mxPool.is() )
        mxPool.clear();

    if( mpImpl )
    {
        delete mpImpl;
        mpImpl = 0;
    }
}

// ----------------------------------------------------------

void SAL_CALL SdStyleFamily::addEventListener( const Reference< XEventListener >&  ) throw (RuntimeException)
{
}

// ----------------------------------------------------------

void SAL_CALL SdStyleFamily::removeEventListener( const Reference< XEventListener >&  ) throw (RuntimeException)
{
}

// ----------------------------------------------------------
// XPropertySet
// ----------------------------------------------------------

Reference<XPropertySetInfo> SdStyleFamily::getPropertySetInfo() throw (RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
    return Reference<XPropertySetInfo>();
}

// ----------------------------------------------------------

void SdStyleFamily::setPropertyValue( const OUString& , const Any&  ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

// ----------------------------------------------------------

Any SdStyleFamily::getPropertyValue( const OUString& PropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
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

// ----------------------------------------------------------

void SdStyleFamily::addPropertyChangeListener( const OUString& , const Reference<XPropertyChangeListener>&  ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

// ----------------------------------------------------------

void SdStyleFamily::removePropertyChangeListener( const OUString& , const Reference<XPropertyChangeListener>&  ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

// ----------------------------------------------------------

void SdStyleFamily::addVetoableChangeListener( const OUString& , const Reference<XVetoableChangeListener>& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

// ----------------------------------------------------------

void SdStyleFamily::removeVetoableChangeListener( const OUString& , const Reference<XVetoableChangeListener>&  ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
