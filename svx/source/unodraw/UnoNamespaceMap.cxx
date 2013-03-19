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


#include <set>

#include "svx/UnoNamespaceMap.hxx"
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase2.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <comphelper/stl_types.hxx>
#include <svl/itempool.hxx>
#include "svx/unoapi.hxx"
#include "editeng/xmlcnitm.hxx"


using namespace ::comphelper;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

namespace svx
{
    /** implements a component to export namespaces of all SvXMLAttrContainerItem inside
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
        virtual Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (RuntimeException);
        virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (RuntimeException);

        // XElementAccess
        virtual Type SAL_CALL getElementType(  ) throw (RuntimeException);
        virtual sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    };

    Reference< XInterface > SAL_CALL NamespaceMap_createInstance( sal_uInt16* pWhichIds, SfxItemPool* pPool )
    {
        return (XWeak*)new NamespaceMap( pWhichIds, pPool );
    }

    Sequence< ::rtl::OUString > SAL_CALL NamespaceMap_getSupportedServiceNames()
        throw()
    {
        Sequence< ::rtl::OUString > aSupportedServiceNames( 1 );
        aSupportedServiceNames[0] = ::rtl::OUString( "com.sun.star.xml.NamespaceMap" );
        return aSupportedServiceNames;
    }

    ::rtl::OUString SAL_CALL NamespaceMap_getImplementationName()
        throw()
    {
        return ::rtl::OUString( "com.sun.star.comp.Svx.NamespaceMap" );
    }



    class NamespaceIteratorImpl
    {
    private:
        SfxItemPool* mpPool;

        sal_uInt16* mpWhichId;

        sal_uInt32 mnItemCount;
        sal_uInt32 mnItem;

        const SvXMLAttrContainerItem* mpCurrentAttr;
        sal_uInt16 mnCurrentAttr;

    public:

        NamespaceIteratorImpl( sal_uInt16* pWhichIds, SfxItemPool* pPool );

        sal_Bool next( ::rtl::OUString& rPrefix, ::rtl::OUString& rURL );
    };
}

using namespace ::svx;

// -------------

NamespaceIteratorImpl::NamespaceIteratorImpl( sal_uInt16* pWhichIds, SfxItemPool* pPool )
{
    mpPool = pPool;
    mpCurrentAttr = NULL;
    mnCurrentAttr = 0;

    mpWhichId = pWhichIds;

    mnItem = 0;
    mnItemCount = (mpWhichId && (0 != *mpWhichId) && mpPool) ? mpPool->GetItemCount2( *mpWhichId ) : 0;
}

sal_Bool NamespaceIteratorImpl::next( ::rtl::OUString& rPrefix, ::rtl::OUString& rURL )
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

    const SfxPoolItem* pItem = 0;
    // look for the next available item in the current pool
    while( (mnItem < mnItemCount) && ( NULL == (pItem = mpPool->GetItem2( *mpWhichId, mnItem ) ) ) )
        mnItem++;

    // are we finished with the current whichid?
    if( mnItem == mnItemCount )
    {
        mpWhichId++;

        // are we finished with the current pool?
        if( 0 != *mpWhichId )
        {
            mnItem = 0;
            mnItemCount = (mpWhichId && (0 != *mpWhichId) && mpPool) ? mpPool->GetItemCount2( *mpWhichId ) : 0;
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
Any SAL_CALL NamespaceMap::getByName( const ::rtl::OUString& aName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    ::rtl::OUString aPrefix;
    ::rtl::OUString aURL;

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

Sequence< ::rtl::OUString > SAL_CALL NamespaceMap::getElementNames() throw (RuntimeException)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    ::rtl::OUString aPrefix;
    ::rtl::OUString aURL;

    std::set< ::rtl::OUString, comphelper::UStringLess > aPrefixSet;

    while( aIter.next( aPrefix, aURL ) )
        aPrefixSet.insert( aPrefix );

    Sequence< ::rtl::OUString > aSeq( aPrefixSet.size() );
    ::rtl::OUString* pPrefixes = aSeq.getArray();

    std::set< ::rtl::OUString, comphelper::UStringLess >::iterator aPrefixIter( aPrefixSet.begin() );
    const std::set< ::rtl::OUString, comphelper::UStringLess >::iterator aEnd( aPrefixSet.end() );

    while( aPrefixIter != aEnd )
    {
        *pPrefixes++ = *aPrefixIter++;
    }

    return aSeq;
}

sal_Bool SAL_CALL NamespaceMap::hasByName( const ::rtl::OUString& aName ) throw (RuntimeException)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    ::rtl::OUString aPrefix;
    ::rtl::OUString aURL;

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
    return ::getCppuType( (const ::rtl::OUString*) 0 );
}

sal_Bool SAL_CALL NamespaceMap::hasElements() throw (RuntimeException)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    ::rtl::OUString aPrefix;
    ::rtl::OUString aURL;

    return aIter.next( aPrefix, aURL );
}

// XServiceInfo
::rtl::OUString SAL_CALL NamespaceMap::getImplementationName(  )
    throw(RuntimeException)
{
    return NamespaceMap_getImplementationName();
}

sal_Bool SAL_CALL NamespaceMap::supportsService( const ::rtl::OUString& )
    throw(RuntimeException)
{
    return sal_True;
}

Sequence< ::rtl::OUString > SAL_CALL NamespaceMap::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return NamespaceMap_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
