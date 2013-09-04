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
#include <comphelper/stl_types.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <svl/style.hxx>

#include <cppuhelper/implbase2.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/lstner.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xdef.hxx>
#include <svx/xflhtit.hxx>

#include <vector>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>


#include "svx/unofill.hxx"

#include "svx/unoapi.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

typedef std::vector< SfxItemSet* > ItemPoolVector;

class SvxUnoMarkerTable : public WeakImplHelper2< container::XNameContainer, lang::XServiceInfo >,
                          public SfxListener
{
private:
    SdrModel*       mpModel;
    SfxItemPool*    mpModelPool;

    ItemPoolVector maItemSetVector;

public:
    SvxUnoMarkerTable( SdrModel* pModel ) throw();
    virtual ~SvxUnoMarkerTable() throw();

    void dispose();

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw ();

    void SAL_CALL ImplInsertByName( const OUString& aName, const uno::Any& aElement );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) throw( uno::RuntimeException);
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeByName( const  OUString& Name ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const  OUString& aName ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);
    virtual uno::Sequence<  OUString > SAL_CALL getElementNames(  ) throw( uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) throw( uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw( uno::RuntimeException);
};

SvxUnoMarkerTable::SvxUnoMarkerTable( SdrModel* pModel ) throw()
: mpModel( pModel ),
  mpModelPool( pModel ? &pModel->GetItemPool() : (SfxItemPool*)NULL )
{
    if( pModel )
        StartListening( *pModel );
}

SvxUnoMarkerTable::~SvxUnoMarkerTable() throw()
{
    if( mpModel )
        EndListening( *mpModel );
    dispose();
}

void SvxUnoMarkerTable::dispose()
{
    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    while( aIter != aEnd )
    {
        delete (*aIter++);
    }

    maItemSetVector.clear();
}

// SfxListener
void SvxUnoMarkerTable::Notify( SfxBroadcaster&, const SfxHint& rHint ) throw()
{
    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

    if( pSdrHint && HINT_MODELCLEARED == pSdrHint->GetKind() )
        dispose();
}

sal_Bool SAL_CALL SvxUnoMarkerTable::supportsService( const  OUString& ServiceName ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

OUString SAL_CALL SvxUnoMarkerTable::getImplementationName() throw( uno::RuntimeException )
{
    return OUString("SvxUnoMarkerTable");
}

uno::Sequence< OUString > SAL_CALL SvxUnoMarkerTable::getSupportedServiceNames(  )
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString("com.sun.star.drawing.MarkerTable" );
    return aSNS;
}

void SAL_CALL SvxUnoMarkerTable::ImplInsertByName( const OUString& aName, const uno::Any& aElement )
{
    SfxItemSet* mpInSet = new SfxItemSet( *mpModelPool, XATTR_LINESTART, XATTR_LINEEND );
    maItemSetVector.push_back( mpInSet );

    XLineEndItem aEndMarker;
    aEndMarker.SetName( aName );
    aEndMarker.PutValue( aElement );

    mpInSet->Put( aEndMarker, XATTR_LINEEND );

    XLineStartItem aStartMarker;
    aStartMarker.SetName( aName );
    aStartMarker.PutValue( aElement );

    mpInSet->Put( aStartMarker, XATTR_LINESTART );
}

// XNameContainer
void SAL_CALL SvxUnoMarkerTable::insertByName( const OUString& aApiName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if( hasByName( aApiName ) )
        throw container::ElementExistException();

    OUString aName = SvxUnogetInternalNameForItem(XATTR_LINEEND, aApiName);

    ImplInsertByName( aName, aElement );
}

void SAL_CALL SvxUnoMarkerTable::removeByName( const OUString& aApiName )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // a little quickfix for 2.0 to let applications clear api
    // created items that are not used
    if ( aApiName == "~clear~" )
    {
        dispose();
        return;
    }

    OUString Name = SvxUnogetInternalNameForItem(XATTR_LINEEND, aApiName);

    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    NameOrIndex *pItem;
    const OUString aSearchName( Name );

    while( aIter != aEnd )
    {
        pItem = (NameOrIndex *)&((*aIter)->Get( XATTR_LINEEND ) );
        if( pItem->GetName() == aSearchName )
        {
            delete (*aIter);
            maItemSetVector.erase( aIter );
            return;
        }
        ++aIter;
    }

    if( !hasByName( Name ) )
        throw container::NoSuchElementException();
}

// XNameReplace
void SAL_CALL SvxUnoMarkerTable::replaceByName( const OUString& aApiName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    OUString aName = SvxUnogetInternalNameForItem(XATTR_LINEEND, aApiName);

    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    NameOrIndex *pItem;
    const OUString aSearchName( aName );

    while( aIter != aEnd )
    {
        pItem = (NameOrIndex *)&((*aIter)->Get( XATTR_LINEEND ) );
        if( pItem->GetName() == aSearchName )
        {
            XLineEndItem aEndMarker;
            aEndMarker.SetName( aSearchName );
            if( !aEndMarker.PutValue( aElement ) )
                throw lang::IllegalArgumentException();

            (*aIter)->Put( aEndMarker, XATTR_LINEEND );

            XLineStartItem aStartMarker;
            aStartMarker.SetName( aSearchName );
            aStartMarker.PutValue( aElement );

            (*aIter)->Put( aStartMarker, XATTR_LINESTART );
            return;
        }
        ++aIter;
    }

    // if it is not in our own sets, modify the pool!
    bool bFound = false;

    sal_uInt32 nSurrogate;
    const sal_uInt32 nStartCount = mpModelPool ? mpModelPool->GetItemCount2( XATTR_LINESTART ) : 0;
    for( nSurrogate = 0; nSurrogate < nStartCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem2( XATTR_LINESTART, nSurrogate);
        if( pItem && pItem->GetName() == aSearchName )
        {
            pItem->PutValue( aElement );
            bFound = true;
            break;
        }
    }

    const sal_uInt32 nEndCount = mpModelPool ? mpModelPool->GetItemCount2( XATTR_LINEEND ) : 0;
    for( nSurrogate = 0; nSurrogate < nEndCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem2( XATTR_LINEEND, nSurrogate);
        if( pItem && pItem->GetName() == aSearchName )
        {
            pItem->PutValue( aElement );
            bFound = true;
            break;
        }
    }

    if( bFound )
        ImplInsertByName( aName, aElement );
    else
        throw container::NoSuchElementException();
}

static bool getByNameFromPool( const OUString& rSearchName, SfxItemPool* pPool, sal_uInt16 nWhich, uno::Any& rAny )
{
    NameOrIndex *pItem;
    const sal_uInt32 nSurrogateCount = pPool ? pPool->GetItemCount2( nWhich ) : 0;
    for( sal_uInt32 nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)pPool->GetItem2( nWhich, nSurrogate );

        if( pItem && pItem->GetName() == rSearchName )
        {
            pItem->QueryValue( rAny, 0 );
            return true;
        }
    }

    return false;
}

// XNameAccess
uno::Any SAL_CALL SvxUnoMarkerTable::getByName( const OUString& aApiName )
    throw( container::NoSuchElementException,  lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    OUString aName = SvxUnogetInternalNameForItem(XATTR_LINEEND, aApiName);

    uno::Any aAny;

    if (mpModelPool && !aName.isEmpty())
    {
        do
        {
            if (getByNameFromPool(aName, mpModelPool, XATTR_LINESTART, aAny))
                break;

            if (getByNameFromPool(aName, mpModelPool, XATTR_LINEEND, aAny))
                break;

            throw container::NoSuchElementException();
        }
        while(0);
    }

    return aAny;
}

static void createNamesForPool( SfxItemPool* pPool, sal_uInt16 nWhich, std::set< OUString, comphelper::UStringLess >& rNameSet )
{
    const sal_uInt32 nSuroCount = pPool->GetItemCount2( nWhich );

    for(sal_uInt32 nSurrogate = 0; nSurrogate < nSuroCount; ++nSurrogate)
    {
        NameOrIndex* pItem = (NameOrIndex*)pPool->GetItem2( nWhich, nSurrogate );

        if( pItem == NULL || pItem->GetName().isEmpty() )
            continue;

        OUString aName = SvxUnogetApiNameForItem(XATTR_LINEEND, pItem->GetName());
        rNameSet.insert( aName );
    }
}

uno::Sequence< OUString > SAL_CALL SvxUnoMarkerTable::getElementNames()
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    std::set< OUString, comphelper::UStringLess > aNameSet;

    // search model pool for line starts
    createNamesForPool( mpModelPool, XATTR_LINESTART, aNameSet );

    // search model pool for line ends
    createNamesForPool( mpModelPool, XATTR_LINEEND, aNameSet );

    uno::Sequence< OUString > aSeq( aNameSet.size() );
    OUString* pNames = aSeq.getArray();

    std::set< OUString, comphelper::UStringLess >::iterator aIter( aNameSet.begin() );
    const std::set< OUString, comphelper::UStringLess >::iterator aEnd( aNameSet.end() );

    while( aIter != aEnd )
    {
        *pNames++ = *aIter++;
    }

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoMarkerTable::hasByName( const OUString& aName )
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if( aName.isEmpty() )
        return sal_False;

    OUString aSearchName;

    NameOrIndex *pItem;

    aSearchName = SvxUnogetInternalNameForItem(XATTR_LINESTART, aName);
    sal_uInt32 nStartCount = mpModelPool ? mpModelPool->GetItemCount2( XATTR_LINESTART ) : 0;
    sal_uInt32 nSurrogate;
    for( nSurrogate = 0; nSurrogate < nStartCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem2( XATTR_LINESTART, nSurrogate);
        if( pItem && pItem->GetName() == aSearchName )
            return sal_True;
    }

    aSearchName = SvxUnogetInternalNameForItem(XATTR_LINEEND, aName);
    sal_uInt32 nEndCount = mpModelPool ? mpModelPool->GetItemCount2( XATTR_LINEEND ) : 0;
    for( nSurrogate = 0; nSurrogate < nEndCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem2( XATTR_LINEEND, nSurrogate);
        if( pItem && pItem->GetName() == aSearchName )
            return sal_True;
    }

    return sal_False;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoMarkerTable::getElementType(  )
    throw( uno::RuntimeException )
{
    return ::getCppuType((const drawing::PointSequence*)0);
}

sal_Bool SAL_CALL SvxUnoMarkerTable::hasElements(  )
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    NameOrIndex *pItem;

    const sal_uInt32 nStartCount = mpModelPool ? mpModelPool->GetItemCount2( XATTR_LINESTART ) : 0;
    sal_uInt32 nSurrogate;
    for( nSurrogate = 0; nSurrogate < nStartCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem2( XATTR_LINESTART, nSurrogate);
        if( pItem && !pItem->GetName().isEmpty() )
            return sal_True;
    }

    const sal_uInt32 nEndCount = mpModelPool ? mpModelPool->GetItemCount2( XATTR_LINEEND ) : 0;
    for( nSurrogate = 0; nSurrogate < nEndCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem2( XATTR_LINEEND, nSurrogate);
        if( pItem && !pItem->GetName().isEmpty() )
            return sal_True;
    }

    return sal_False;
}

/**
 * Create a hatchtable
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoMarkerTable_createInstance( SdrModel* pModel )
{
    return *new SvxUnoMarkerTable(pModel);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
