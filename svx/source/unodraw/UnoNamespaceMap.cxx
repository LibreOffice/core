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


#include <climits>
#include <set>

#include <svx/UnoNamespaceMap.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svl/itempool.hxx>
#include <editeng/xmlcnitm.hxx>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

namespace svx
{
    namespace {

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

        // XNameAccess
        virtual Any SAL_CALL getByName( const OUString& aName ) override;
        virtual Sequence< OUString > SAL_CALL getElementNames(  ) override;
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

        // XElementAccess
        virtual Type SAL_CALL getElementType(  ) override;
        virtual sal_Bool SAL_CALL hasElements(  ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
    };

    }

    Reference< XInterface > NamespaceMap_createInstance( sal_uInt16* pWhichIds, SfxItemPool* pPool )
    {
        return static_cast<XWeak*>(new NamespaceMap( pWhichIds, pPool ));
    }

    static Sequence< OUString > NamespaceMap_getSupportedServiceNames()
        noexcept
    {
        Sequence<OUString> aSupportedServiceNames { "com.sun.star.xml.NamespaceMap" };
        return aSupportedServiceNames;
    }

    static OUString NamespaceMap_getImplementationName()
        noexcept
    {
        return "com.sun.star.comp.Svx.NamespaceMap";
    }

    namespace {

    class NamespaceIteratorImpl
    {
    private:
        SfxItemPool* mpPool;

        sal_uInt16* mpWhichId;

        std::vector<const SvXMLAttrContainerItem*> mvItems;
        sal_Int32 mnItem;

        const SvXMLAttrContainerItem* mpCurrentAttr;
        sal_uInt16 mnCurrentAttr;

    public:

        NamespaceIteratorImpl( sal_uInt16* pWhichIds, SfxItemPool* pPool );

        bool next( OUString& rPrefix, OUString& rURL );
    };

    }
}

using namespace ::svx;


NamespaceIteratorImpl::NamespaceIteratorImpl( sal_uInt16* pWhichIds, SfxItemPool* pPool )
{
    mpPool = pPool;
    mpCurrentAttr = nullptr;
    mnCurrentAttr = 0;

    mpWhichId = pWhichIds;

    mnItem = -1;
    if (mpWhichId && (0 != *mpWhichId) && mpPool)
    {
        mvItems.reserve(mpPool->GetItemCount2( *mpWhichId ));
        for (const SfxPoolItem* pItem : mpPool->GetItemSurrogates( *mpWhichId ))
            mvItems.push_back(static_cast<const SvXMLAttrContainerItem*>(pItem));
    }
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
    mnItem++;

    // are we finished with the current whichid?
    if( mnItem == static_cast<sal_Int32>(mvItems.size()) )
    {
        mpWhichId++;

        // are we finished with the current pool?
        if( 0 == *mpWhichId )
            return false;

        mnItem = -1;
        mvItems.clear();
        if (mpPool)
        {
            mvItems.reserve(mpPool->GetItemCount2( *mpWhichId ));
            for (const SfxPoolItem* pItem2 : mpPool->GetItemSurrogates( *mpWhichId ))
                mvItems.push_back(static_cast<const SvXMLAttrContainerItem*>(pItem2));
        }
        return next( rPrefix, rURL );
    }

    auto pItem = mvItems[mnItem];
    // get that item and see if there namespaces inside
    if( pItem->GetAttrCount() > 0 )
    {
        mpCurrentAttr = pItem;
        mnCurrentAttr = pItem->GetFirstNamespaceIndex();
    }
    return next( rPrefix, rURL );
}


NamespaceMap::NamespaceMap( sal_uInt16* pWhichIds, SfxItemPool* pPool )
: mpWhichIds( pWhichIds ), mpPool( pPool )
{
}

// XNameAccess
Any SAL_CALL NamespaceMap::getByName( const OUString& aName )
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

Sequence< OUString > SAL_CALL NamespaceMap::getElementNames()
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    OUString aPrefix;
    OUString aURL;

    std::set< OUString > aPrefixSet;

    while( aIter.next( aPrefix, aURL ) )
        aPrefixSet.insert( aPrefix );

    return comphelper::containerToSequence(aPrefixSet);
}

sal_Bool SAL_CALL NamespaceMap::hasByName( const OUString& aName )
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
Type SAL_CALL NamespaceMap::getElementType()
{
    return ::cppu::UnoType<OUString>::get();
}

sal_Bool SAL_CALL NamespaceMap::hasElements()
{
    NamespaceIteratorImpl aIter( mpWhichIds, mpPool );

    OUString aPrefix;
    OUString aURL;

    return aIter.next( aPrefix, aURL );
}

// XServiceInfo
OUString SAL_CALL NamespaceMap::getImplementationName(  )
{
    return NamespaceMap_getImplementationName();
}

sal_Bool SAL_CALL NamespaceMap::supportsService( const OUString& serviceName )
{
    return cppu::supportsService( this, serviceName );
}

Sequence< OUString > SAL_CALL NamespaceMap::getSupportedServiceNames(  )
{
    return NamespaceMap_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
