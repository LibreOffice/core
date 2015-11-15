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

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <svl/itempool.hxx>
#include "svx/unoapi.hxx"
#include "editeng/xmlcnitm.hxx"

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
    class NamespaceMap : public WeakImplHelper< XNameAccess, XServiceInfo >
    {
    private:
        sal_uInt16* mpWhichIds;
        SfxItemPool* mpPool;

    public:
        NamespaceMap( sal_uInt16* pWhichIds, SfxItemPool* pPool );
        virtual ~NamespaceMap();

        // XNameAccess
        virtual Any SAL_CALL getByName( const OUString& aName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException, std::exception) override;
        virtual Sequence< OUString > SAL_CALL getElementNames(  ) throw (RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (RuntimeException, std::exception) override;

        // XElementAccess
        virtual Type SAL_CALL getElementType(  ) throw (RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException, std::exception) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException, std::exception) override;
    };

    Reference< XInterface > SAL_CALL NamespaceMap_createInstance( sal_uInt16* pWhichIds, SfxItemPool* pPool )
    {
        return static_cast<XWeak*>(new NamespaceMap( pWhichIds, pPool ));
    }

    Sequence< OUString > SAL_CALL NamespaceMap_getSupportedServiceNames()
        throw()
    {
        Sequence<OUString> aSupportedServiceNames { "com.sun.star.xml.NamespaceMap" };
        return aSupportedServiceNames;
    }

    OUString SAL_CALL NamespaceMap_getImplementationName()
        throw()
    {
        return OUString( "com.sun.star.comp.Svx.NamespaceMap" );
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

        bool next( OUString& rPrefix, OUString& rURL );
    };
}

using namespace ::svx;



NamespaceIteratorImpl::NamespaceIteratorImpl( sal_uInt16* pWhichIds, SfxItemPool* pPool )
{
    mpPool = pPool;
    mpCurrentAttr = nullptr;
    mnCurrentAttr = 0;

    mpWhichId = pWhichIds;

    mnItem = 0;
    mnItemCount = (mpWhichId && (0 != *mpWhichId) && mpPool) ? mpPool->GetItemCount2( *mpWhichId ) : 0;
}

bool NamespaceIteratorImpl::next( OUString& rPrefix, OUString& rURL )
{
    // we still need to process the current attribute
    if( mpCurrentAttr && (mnCurrentAttr != USHRT_MAX) )
    {
        rPrefix = mpCurrentAttr->GetPrefix( mnCurrentAttr );
        rURL = mpCurrentAttr->GetNamespace( mnCurrentAttr );

        mnCurrentAttr = mpCurrentAttr->GetNextNamespaceIndex( mnCurrentAttr );
        return true;
    }

    // we need the next namespace item
    mpCurrentAttr = nullptr;

    const SfxPoolItem* pItem = nullptr;
    // look for the next available item in the current pool
    while( (mnItem < mnItemCount) && ( nullptr == (pItem = mpPool->GetItem2( *mpWhichId, mnItem ) ) ) )
        mnItem++;

    // are we finished with the current whichid?
    if( mnItem == mnItemCount )
    {
        mpWhichId++;

        // are we finished with the current pool?
        if( 0 != *mpWhichId )
        {
            mnItem = 0;
            mnItemCount = mpPool ? mpPool->GetItemCount2( *mpWhichId ) : 0;
            return next( rPrefix, rURL );
        }

        pItem = nullptr;
    }

    if( pItem )
    {
        mnItem++;

        // get that item and see if there namespaces inside
        const SvXMLAttrContainerItem *pUnknown = static_cast<const SvXMLAttrContainerItem *>(pItem);
        if( (pUnknown->GetAttrCount() > 0) )
        {
            mpCurrentAttr = pUnknown;
            mnCurrentAttr = pUnknown->GetFirstNamespaceIndex();
        }
        return next( rPrefix, rURL );
    }

    return false;
}



NamespaceMap::NamespaceMap( sal_uInt16* pWhichIds, SfxItemPool* pPool )
: mpWhichIds( pWhichIds ), mpPool( pPool )
{
}

NamespaceMap::~NamespaceMap()
{
}

// XNameAccess
Any SAL_CALL NamespaceMap::getByName( const OUString& aName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    OUString aPrefix;
    OUString aURL;

    bool bFound;

    do
    {
        bFound = aIter.next( aPrefix, aURL );
    }
    while( bFound && (aPrefix != aName ) );

    if( !bFound )
        throw NoSuchElementException();

    return makeAny( aURL );
}

Sequence< OUString > SAL_CALL NamespaceMap::getElementNames() throw (RuntimeException, std::exception)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    OUString aPrefix;
    OUString aURL;

    std::set< OUString > aPrefixSet;

    while( aIter.next( aPrefix, aURL ) )
        aPrefixSet.insert( aPrefix );

    Sequence< OUString > aSeq( aPrefixSet.size() );
    OUString* pPrefixes = aSeq.getArray();

    std::set< OUString >::iterator aPrefixIter( aPrefixSet.begin() );
    const std::set< OUString >::iterator aEnd( aPrefixSet.end() );

    while( aPrefixIter != aEnd )
    {
        *pPrefixes++ = *aPrefixIter++;
    }

    return aSeq;
}

sal_Bool SAL_CALL NamespaceMap::hasByName( const OUString& aName ) throw (RuntimeException, std::exception)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    OUString aPrefix;
    OUString aURL;

    bool bFound;

    do
    {
        bFound = aIter.next( aPrefix, aURL );
    }
    while( bFound && (aPrefix != aName ) );

    return bFound;
}

// XElementAccess
Type SAL_CALL NamespaceMap::getElementType() throw (RuntimeException, std::exception)
{
    return ::cppu::UnoType<OUString>::get();
}

sal_Bool SAL_CALL NamespaceMap::hasElements() throw (RuntimeException, std::exception)
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    OUString aPrefix;
    OUString aURL;

    return aIter.next( aPrefix, aURL );
}

// XServiceInfo
OUString SAL_CALL NamespaceMap::getImplementationName(  )
    throw(RuntimeException, std::exception)
{
    return NamespaceMap_getImplementationName();
}

sal_Bool SAL_CALL NamespaceMap::supportsService( const OUString& serviceName )
    throw(RuntimeException, std::exception)
{
    return cppu::supportsService( this, serviceName );
}

Sequence< OUString > SAL_CALL NamespaceMap::getSupportedServiceNames(  )
    throw(RuntimeException, std::exception)
{
    return NamespaceMap_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
