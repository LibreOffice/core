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

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif


#include <cppuhelper/implbase2.hxx>

#ifndef _SFXITEMPOOL_HXX
#include <bf_svtools/itempool.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <bf_svtools/itemset.hxx>
#endif

#ifndef _SFXLSTNER_HXX 
#include <bf_svtools/lstner.hxx>
#endif

#ifndef _SVX_XLNEDIT_HXX //autogen
#include <xlnedit.hxx>
#endif
#ifndef _SVX_XLNSTIT_HXX 
#include <xlnstit.hxx>
#endif
#include "svdmodel.hxx"
#include "xdef.hxx"
    
#include <vector>

#ifndef _VOS_MUTEX_HXX_ 
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX 
#include <vcl/svapp.hxx>
#endif


#include "unoapi.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::vos;

typedef std::vector< SfxItemSet* > ItemPoolVector;

class SvxUnoMarkerTable : public WeakImplHelper2< container::XNameContainer, lang::XServiceInfo >,
                          public SfxListener
{
private:
    SdrModel*		mpModel;
    SfxItemPool*	mpModelPool;

    ItemPoolVector maItemSetVector;

public:
    SvxUnoMarkerTable( SdrModel* pModel ) throw();
    virtual	~SvxUnoMarkerTable() throw();

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
void SvxUnoMarkerTable::Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw()
{
    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

    if( pSdrHint && HINT_MODELCLEARED == pSdrHint->GetKind() )
        dispose();
}

sal_Bool SAL_CALL SvxUnoMarkerTable::supportsService( const  OUString& ServiceName ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;

    return FALSE;
}

OUString SAL_CALL SvxUnoMarkerTable::getImplementationName() throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoMarkerTable") );
}

uno::Sequence< OUString > SAL_CALL SvxUnoMarkerTable::getSupportedServiceNames(  )
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.MarkerTable" ));
    return aSNS;
}

void SAL_CALL SvxUnoMarkerTable::ImplInsertByName( const OUString& aName, const uno::Any& aElement )
{
    SfxItemSet* mpInSet = new SfxItemSet( *mpModelPool, XATTR_LINESTART, XATTR_LINEEND );
    maItemSetVector.push_back( mpInSet );

    XLineEndItem aEndMarker;
    aEndMarker.SetName( String( aName ) );
    aEndMarker.PutValue( aElement );
    
    mpInSet->Put( aEndMarker, XATTR_LINEEND );
    
    XLineStartItem aStartMarker;
    aStartMarker.SetName( String( aName ) );
    aStartMarker.PutValue( aElement );

    mpInSet->Put( aStartMarker, XATTR_LINESTART );
}

// XNameContainer
void SAL_CALL SvxUnoMarkerTable::insertByName( const OUString& aApiName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( hasByName( aApiName ) )
        throw container::ElementExistException();

    String aName;
    SvxUnogetInternalNameForItem( XATTR_LINEEND, aApiName, aName );

    ImplInsertByName( aName, aElement );
}

void SAL_CALL SvxUnoMarkerTable::removeByName( const OUString& aApiName )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    String Name;
    SvxUnogetInternalNameForItem( XATTR_LINEEND, aApiName, Name );

    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    NameOrIndex *pItem;
    const String aSearchName( Name );

    while( aIter != aEnd )
    {
        pItem = (NameOrIndex *)&((*aIter)->Get( XATTR_LINEEND ) );
        if( pItem->GetName() == aSearchName )
        {
            delete (*aIter);
            maItemSetVector.erase( aIter );
            return;
        }
        aIter++;
    }

    if( !hasByName( Name ) )
        throw container::NoSuchElementException();
}

// XNameReplace
void SAL_CALL SvxUnoMarkerTable::replaceByName( const OUString& aApiName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    String aName;
    SvxUnogetInternalNameForItem( XATTR_LINEEND, aApiName, aName );

    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    NameOrIndex *pItem;
    const String aSearchName( aName );

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
        aIter++;
    }

    // if it is not in our own sets, modify the pool!
    sal_Bool bFound = sal_False;

    USHORT nSurrogate;
    const USHORT nStartCount = mpModelPool ? mpModelPool->GetItemCount( XATTR_LINESTART ) : 0;
    for( nSurrogate = 0; nSurrogate < nStartCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem( XATTR_LINESTART, nSurrogate);
        if( pItem && pItem->GetName() == aSearchName )
        {
            pItem->PutValue( aElement );
            bFound = sal_True;
            break;
        }
    }

    const USHORT nEndCount = mpModelPool ? mpModelPool->GetItemCount( XATTR_LINEEND ) : 0;
    for( nSurrogate = 0; nSurrogate < nEndCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem( XATTR_LINEEND, nSurrogate);
        if( pItem && pItem->GetName() == aSearchName )
        {
            pItem->PutValue( aElement );
            bFound = sal_True;
            break;
        }
    }

    if( bFound )
        ImplInsertByName( aName, aElement );
    else
        throw container::NoSuchElementException();
}

static sal_Bool getByNameFromPool( const String& rSearchName, SfxItemPool* pPool, USHORT nWhich, uno::Any& rAny )
{
    NameOrIndex *pItem;
    const sal_Int32 nSurrogateCount = pPool ? (sal_Int32)pPool->GetItemCount( nWhich ) : 0;
    for( sal_Int32 nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)pPool->GetItem( nWhich, (USHORT)nSurrogate );

        if( pItem && pItem->GetName() == rSearchName )
        {
            pItem->QueryValue( rAny, 0 );
            return sal_True;
        }
    }

    return sal_False;
}

// XNameAccess
uno::Any SAL_CALL SvxUnoMarkerTable::getByName( const OUString& aApiName )
    throw( container::NoSuchElementException,  lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    String aName;
    SvxUnogetInternalNameForItem( XATTR_LINEEND, aApiName, aName );

    uno::Any aAny;

    if( mpModelPool && aName.Len() != 0 )
    {
        do
        {
            const String aSearchName( aName );
            if( getByNameFromPool( aSearchName, mpModelPool, XATTR_LINESTART, aAny ) )
                break;

            if( getByNameFromPool( aSearchName, mpModelPool, XATTR_LINEEND, aAny ) )
                break;

            throw container::NoSuchElementException();
        }
        while(0);
    }

    return aAny;
}

static void createNamesForPool( SfxItemPool* pPool, USHORT nWhich, std::set< OUString, comphelper::UStringLess >& rNameSet )
{
    const sal_Int32 nSuroCount = pPool->GetItemCount( nWhich );
    sal_Int32 nSurrogate;

    NameOrIndex *pItem;
    OUString aName;

    for( nSurrogate = 0; nSurrogate < nSuroCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)pPool->GetItem( nWhich, (USHORT)nSurrogate );

        if( pItem == NULL || pItem->GetName().Len() == 0 )
            continue;

        SvxUnogetApiNameForItem( XATTR_LINEEND, pItem->GetName(), aName );
        rNameSet.insert( aName );
    }
}

uno::Sequence< OUString > SAL_CALL SvxUnoMarkerTable::getElementNames()
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

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
    OGuard aGuard( Application::GetSolarMutex() );

    if( aName.getLength() == 0 )
        return sal_False;

    String aSearchName;

    NameOrIndex *pItem;

    SvxUnogetInternalNameForItem( XATTR_LINESTART, aName, aSearchName );
    USHORT nStartCount = mpModelPool ? mpModelPool->GetItemCount( XATTR_LINESTART ) : 0;
    USHORT nSurrogate;
    for( nSurrogate = 0; nSurrogate < nStartCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem( XATTR_LINESTART, nSurrogate);
        if( pItem && pItem->GetName() == aSearchName )
            return sal_True;
    }

    SvxUnogetInternalNameForItem( XATTR_LINEEND, aName, aSearchName );
    USHORT nEndCount = mpModelPool ? mpModelPool->GetItemCount( XATTR_LINEEND ) : 0;
    for( nSurrogate = 0; nSurrogate < nEndCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem( XATTR_LINEEND, nSurrogate);
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
    OGuard aGuard( Application::GetSolarMutex() );

    NameOrIndex *pItem;

    const USHORT nStartCount = mpModelPool ? mpModelPool->GetItemCount( XATTR_LINESTART ) : 0;
    USHORT nSurrogate;
    for( nSurrogate = 0; nSurrogate < nStartCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem( XATTR_LINESTART, nSurrogate);
        if( pItem && pItem->GetName().Len() != 0 )
            return sal_True;
    }

    const USHORT nEndCount = mpModelPool ? mpModelPool->GetItemCount( XATTR_LINEEND ) : 0;
    for( nSurrogate = 0; nSurrogate < nEndCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem( XATTR_LINEEND, nSurrogate);
        if( pItem && pItem->GetName().Len() != 0 )
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



}
