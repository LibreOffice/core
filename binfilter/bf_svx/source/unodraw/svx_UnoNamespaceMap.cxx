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

#include <set>

#include <com/sun/star/container/XNameAccess.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase2.hxx>

#include <osl/diagnose.h>


#include <comphelper/stl_types.hxx>

#include <bf_svtools/itempool.hxx>

#include "unoapi.hxx"

#include "xmlcnitm.hxx"
namespace binfilter {

using namespace ::comphelper;
using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

namespace svx
{
    /**	implements a component to export namespaces of all SvXMLAttrContainerItem inside
        one or two pools with a variable count of which ids.
    */
    class NamespaceMap : public WeakImplHelper2< XNameAccess, XServiceInfo >
    {
    private:
        sal_uInt16* mpWhichIds;
        SfxItemPool* mpPool;

    public:
        NamespaceMap( sal_uInt16* pWhichIds, SfxItemPool* pPool );
        virtual ~NamespaceMap();

        // XNameAccess
        virtual Any SAL_CALL getByName( const OUString& aName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException);
        virtual Sequence< OUString > SAL_CALL getElementNames(  ) throw (RuntimeException);
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (RuntimeException);

        // XElementAccess
        virtual Type SAL_CALL getElementType(  ) throw (RuntimeException);
        virtual sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    };

    Reference< XInterface > SAL_CALL NamespaceMap_createInstance( sal_uInt16* pWhichIds, SfxItemPool* pPool )
        throw( Exception )
    {
        return (XWeak*)new NamespaceMap( pWhichIds, pPool );
    }

    Sequence< OUString > SAL_CALL NamespaceMap_getSupportedServiceNames()
        throw()
    {
        Sequence< OUString > aSupportedServiceNames( 1 );
        aSupportedServiceNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.NamespaceMap" ) );
        return aSupportedServiceNames;
    }

    OUString SAL_CALL NamespaceMap_getImplementationName()
        throw()
    {
        return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Svx.NamespaceMap" ) );
    }



    class NamespaceIteratorImpl
    {
    private:
        SfxItemPool* mpPool;

        sal_uInt16* mpWhichId;

        sal_uInt16 mnItemCount;
        sal_uInt16 mnItem;

        const SvXMLAttrContainerItem* mpCurrentAttr;
        sal_uInt16 mnCurrentAttr;

    public:

        NamespaceIteratorImpl( sal_uInt16* pWhichIds, SfxItemPool* pPool );

        sal_Bool next( OUString& rPrefix, OUString& rURL );
    };
}

using namespace ::binfilter ::svx;

// -------------

NamespaceIteratorImpl::NamespaceIteratorImpl( sal_uInt16* pWhichIds, SfxItemPool* pPool )
{
    mpPool = pPool;
    mpCurrentAttr = NULL;
    mnCurrentAttr = 0;

    mpWhichId = pWhichIds;

    mnItem = 0;
    mnItemCount = (mpWhichId && (0 != *mpWhichId) && mpPool) ? mpPool->GetItemCount( *mpWhichId ) : 0;
}

sal_Bool NamespaceIteratorImpl::next( OUString& rPrefix, OUString& rURL )
{
    // we still need to process the current attribute
    if( mpCurrentAttr && (mnCurrentAttr != USHRT_MAX) )
    {
        rPrefix = mpCurrentAttr->GetPrefix( mnCurrentAttr );
        rURL = mpCurrentAttr->GetNamespace( mnCurrentAttr );

        mnCurrentAttr = mpCurrentAttr->GetNextNamespaceIndex( mnCurrentAttr );
        return sal_True;
    }

    // we need the next namespace item
    mpCurrentAttr = NULL;

    const SfxPoolItem* pItem;
    // look for the next available item in the current pool
    while( (mnItem < mnItemCount) && ( NULL == (pItem = mpPool->GetItem( *mpWhichId, mnItem ) ) ) )
        mnItem++;

    // are we finished with the current whichid?
    if( mnItem == mnItemCount )
    {
        mpWhichId++;

        // are we finished with the current pool?
        if( 0 != *mpWhichId )
        {
            mnItem = 0;
            mnItemCount = (mpWhichId && (0 != *mpWhichId) && mpPool) ? mpPool->GetItemCount( *mpWhichId ) : 0;
            return next( rPrefix, rURL );
        }

        pItem = NULL;
    }

    if( pItem )
    {
        mnItem++;

        // get that item and see if there namespaces inside
        const SvXMLAttrContainerItem *pUnknown = (const SvXMLAttrContainerItem *)pItem;
        if( (pUnknown->GetAttrCount() > 0) )
        {
            mpCurrentAttr = pUnknown;
            mnCurrentAttr = pUnknown->GetFirstNamespaceIndex();
        }
        return next( rPrefix, rURL );
    }

    return false;
}

// -------------

NamespaceMap::NamespaceMap( sal_uInt16* pWhichIds, SfxItemPool* pPool )
: mpWhichIds( pWhichIds ), mpPool( pPool )
{
}

NamespaceMap::~NamespaceMap()
{
}

// XNameAccess
Any SAL_CALL NamespaceMap::getByName( const OUString& aName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    OUString aPrefix;
    OUString aURL;

    sal_Bool bFound;

    do
    {
        bFound = aIter.next( aPrefix, aURL );
    }
    while( bFound && (aPrefix != aName ) );

    if( !bFound )
        throw NoSuchElementException();

    return makeAny( aURL );
}

Sequence< OUString > SAL_CALL NamespaceMap::getElementNames() throw (RuntimeException)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    OUString aPrefix;
    OUString aURL;

    std::set< OUString, comphelper::UStringLess > aPrefixSet;

    while( aIter.next( aPrefix, aURL ) )
        aPrefixSet.insert( aPrefix );

    Sequence< OUString > aSeq( aPrefixSet.size() );
    OUString* pPrefixes = aSeq.getArray();

    std::set< OUString, comphelper::UStringLess >::iterator aPrefixIter( aPrefixSet.begin() );
    const std::set< OUString, comphelper::UStringLess >::iterator aEnd( aPrefixSet.end() );

    while( aPrefixIter != aEnd )
    {
        *pPrefixes++ = *aPrefixIter++;
    }

    return aSeq;
}

sal_Bool SAL_CALL NamespaceMap::hasByName( const OUString& aName ) throw (RuntimeException)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    OUString aPrefix;
    OUString aURL;

    sal_Bool bFound;

    do
    {
        bFound = aIter.next( aPrefix, aURL );
    }
    while( bFound && (aPrefix != aName ) );

    return bFound;
}

// XElementAccess
Type SAL_CALL NamespaceMap::getElementType() throw (RuntimeException)
{
    return ::getCppuType( (const OUString*) 0 );
}

sal_Bool SAL_CALL NamespaceMap::hasElements() throw (RuntimeException)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    OUString aPrefix;
    OUString aURL;

    return aIter.next( aPrefix, aURL );
}

// XServiceInfo
OUString SAL_CALL NamespaceMap::getImplementationName(  )
    throw(RuntimeException)
{
    return NamespaceMap_getImplementationName();
}

sal_Bool SAL_CALL NamespaceMap::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    return sal_True;
}

Sequence< OUString > SAL_CALL NamespaceMap::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return NamespaceMap_getSupportedServiceNames();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
