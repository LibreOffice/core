/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <set>

#include "svx/UnoNamespaceMap.hxx"
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase2.hxx>
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
    class NamespaceMap : public WeakImplHelper2< XNameAccess, XServiceInfo >
    {
    private:
        sal_uInt16* mpWhichIds;
        SfxItemPool* mpPool;

    public:
        NamespaceMap( sal_uInt16* pWhichIds, SfxItemPool* pPool );
        virtual ~NamespaceMap();

        
        virtual Any SAL_CALL getByName( const OUString& aName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException);
        virtual Sequence< OUString > SAL_CALL getElementNames(  ) throw (RuntimeException);
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (RuntimeException);

        
        virtual Type SAL_CALL getElementType(  ) throw (RuntimeException);
        virtual sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException);

        
        virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    };

    Reference< XInterface > SAL_CALL NamespaceMap_createInstance( sal_uInt16* pWhichIds, SfxItemPool* pPool )
    {
        return (XWeak*)new NamespaceMap( pWhichIds, pPool );
    }

    Sequence< OUString > SAL_CALL NamespaceMap_getSupportedServiceNames()
        throw()
    {
        Sequence< OUString > aSupportedServiceNames( 1 );
        aSupportedServiceNames[0] = "com.sun.star.xml.NamespaceMap";
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

        sal_Bool next( OUString& rPrefix, OUString& rURL );
    };
}

using namespace ::svx;



NamespaceIteratorImpl::NamespaceIteratorImpl( sal_uInt16* pWhichIds, SfxItemPool* pPool )
{
    mpPool = pPool;
    mpCurrentAttr = NULL;
    mnCurrentAttr = 0;

    mpWhichId = pWhichIds;

    mnItem = 0;
    mnItemCount = (mpWhichId && (0 != *mpWhichId) && mpPool) ? mpPool->GetItemCount2( *mpWhichId ) : 0;
}

sal_Bool NamespaceIteratorImpl::next( OUString& rPrefix, OUString& rURL )
{
    
    if( mpCurrentAttr && (mnCurrentAttr != USHRT_MAX) )
    {
        rPrefix = mpCurrentAttr->GetPrefix( mnCurrentAttr );
        rURL = mpCurrentAttr->GetNamespace( mnCurrentAttr );

        mnCurrentAttr = mpCurrentAttr->GetNextNamespaceIndex( mnCurrentAttr );
        return sal_True;
    }

    
    mpCurrentAttr = NULL;

    const SfxPoolItem* pItem = 0;
    
    while( (mnItem < mnItemCount) && ( NULL == (pItem = mpPool->GetItem2( *mpWhichId, mnItem ) ) ) )
        mnItem++;

    
    if( mnItem == mnItemCount )
    {
        mpWhichId++;

        
        if( 0 != *mpWhichId )
        {
            mnItem = 0;
            mnItemCount = mpPool ? mpPool->GetItemCount2( *mpWhichId ) : 0;
            return next( rPrefix, rURL );
        }

        pItem = NULL;
    }

    if( pItem )
    {
        mnItem++;

        
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



NamespaceMap::NamespaceMap( sal_uInt16* pWhichIds, SfxItemPool* pPool )
: mpWhichIds( pWhichIds ), mpPool( pPool )
{
}

NamespaceMap::~NamespaceMap()
{
}


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


OUString SAL_CALL NamespaceMap::getImplementationName(  )
    throw(RuntimeException)
{
    return NamespaceMap_getImplementationName();
}

sal_Bool SAL_CALL NamespaceMap::supportsService( const OUString& serviceName )
    throw(RuntimeException)
{
    return cppu::supportsService( this, serviceName );
}

Sequence< OUString > SAL_CALL NamespaceMap::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return NamespaceMap_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
