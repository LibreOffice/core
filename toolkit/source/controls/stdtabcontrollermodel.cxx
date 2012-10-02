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

#include <com/sun/star/io/XMarkableStream.hpp>

#include <toolkit/controls/stdtabcontrollermodel.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/helper/property.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uuid.h>

#include <tools/debug.hxx>

#define UNOCONTROL_STREAMVERSION    (short)2

//  ----------------------------------------------------
//  class UnoControlModelEntryList
//  ----------------------------------------------------
UnoControlModelEntryList::UnoControlModelEntryList()
{
}

UnoControlModelEntryList::~UnoControlModelEntryList()
{
    Reset();
}

void UnoControlModelEntryList::Reset()
{
    for ( size_t n = maList.size(); n; )
        DestroyEntry( --n );
}

void UnoControlModelEntryList::DestroyEntry( size_t nEntry )
{
    UnoControlModelEntryListBase::iterator it = maList.begin();
    ::std::advance( it, nEntry );

    if ( (*it)->bGroup )
        delete (*it)->pGroup;
    else
        delete (*it)->pxControl;

    delete *it;
    maList.erase( it );
}

size_t  UnoControlModelEntryList::size() const {
    return maList.size();
}

UnoControlModelEntry* UnoControlModelEntryList::operator[]( size_t i ) const {
    return ( i < maList.size() ) ? maList[ i ] : NULL;
}

void UnoControlModelEntryList::push_back( UnoControlModelEntry* item ) {
    maList.push_back( item );
}

void UnoControlModelEntryList::insert( size_t i, UnoControlModelEntry* item ) {
    if ( i < maList.size() ) {
        UnoControlModelEntryListBase::iterator it = maList.begin();
        ::std::advance( it, i );
        maList.insert( it, item );
    } else {
        maList.push_back( item );
    }
}


//  ----------------------------------------------------
//  class StdTabControllerModel
//  ----------------------------------------------------
StdTabControllerModel::StdTabControllerModel()
{
    mbGroupControl = sal_True;
}

StdTabControllerModel::~StdTabControllerModel()
{
}

sal_uInt32 StdTabControllerModel::ImplGetControlCount( const UnoControlModelEntryList& rList ) const
{
    sal_uInt32 nCount = 0;
    size_t nEntries = rList.size();
    for ( size_t n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry* pEntry = rList[ n ];
        if ( pEntry->bGroup )
            nCount += ImplGetControlCount( *pEntry->pGroup );
        else
            nCount++;
    }
    return nCount;
}

void StdTabControllerModel::ImplGetControlModels( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > ** ppRefs, const UnoControlModelEntryList& rList ) const
{
    size_t nEntries = rList.size();
    for ( size_t n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry* pEntry = rList[ n ];
        if ( pEntry->bGroup )
            ImplGetControlModels( ppRefs, *pEntry->pGroup );
        else
        {
            **ppRefs = *pEntry->pxControl;
            (*ppRefs)++;
        }
    }
}

void StdTabControllerModel::ImplSetControlModels( UnoControlModelEntryList& rList, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Controls ) const
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > * pRefs = Controls.getConstArray();
    sal_uInt32 nControls = Controls.getLength();
    for ( sal_uInt32 n = 0; n < nControls; n++ )
    {
        UnoControlModelEntry* pNewEntry = new UnoControlModelEntry;
        pNewEntry->bGroup = sal_False;
        pNewEntry->pxControl = new ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > ;
        *pNewEntry->pxControl = pRefs[n];
        rList.push_back( pNewEntry );
    }
}

sal_uInt32 StdTabControllerModel::ImplGetControlPos( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  xCtrl, const UnoControlModelEntryList& rList ) const
{
    for ( size_t n = rList.size(); n; )
    {
        UnoControlModelEntry* pEntry = rList[ --n ];
        if ( !pEntry->bGroup && ( *pEntry->pxControl == xCtrl ) )
            return n;
    }
    return CONTROLPOS_NOTFOUND;
}

void ImplWriteControls( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream > & OutStream, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& rCtrls )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XMarkableStream >  xMark( OutStream, ::com::sun::star::uno::UNO_QUERY );
    DBG_ASSERT( xMark.is(), "write: no XMarkableStream!" );

    sal_uInt32 nStoredControls = 0;
    sal_Int32 nDataBeginMark = xMark->createMark();

    OutStream->writeLong( 0L ); // DataLen
    OutStream->writeLong( 0L ); // nStoredControls

    sal_uInt32 nCtrls = rCtrls.getLength();
    for ( sal_uInt32 n = 0; n < nCtrls; n++ )
    {
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  xI = rCtrls.getConstArray()[n];
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject >  xPO( xI, ::com::sun::star::uno::UNO_QUERY );
        DBG_ASSERT( xPO.is(), "write: Control doesn't support XPersistObject" );
        if ( xPO.is() )
        {
            OutStream->writeObject( xPO );
            nStoredControls++;
        }
    }
    sal_Int32 nDataLen = xMark->offsetToMark( nDataBeginMark );
    xMark->jumpToMark( nDataBeginMark );
    OutStream->writeLong( nDataLen );
    OutStream->writeLong( nStoredControls );
    xMark->jumpToFurthest();
    xMark->deleteMark(nDataBeginMark);
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > ImplReadControls( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream > & InStream )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XMarkableStream >  xMark( InStream, ::com::sun::star::uno::UNO_QUERY );
    DBG_ASSERT( xMark.is(), "write: no XMarkableStream!" );

    sal_Int32 nDataBeginMark = xMark->createMark();

    sal_Int32 nDataLen = InStream->readLong();
    sal_uInt32 nCtrls = InStream->readLong();

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aSeq( nCtrls );
    for ( sal_uInt32 n = 0; n < nCtrls; n++ )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject >  xObj = InStream->readObject();
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  xI( xObj, ::com::sun::star::uno::UNO_QUERY );
        aSeq.getArray()[n] = xI;
    }

    // Skip remainder if more data exists than this version recognizes
    xMark->jumpToMark( nDataBeginMark );
    InStream->skipBytes( nDataLen );
    xMark->deleteMark(nDataBeginMark);
    return aSeq;
}


// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any StdTabControllerModel::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XTabControllerModel* >(this)),
                                        (static_cast< ::com::sun::star::lang::XServiceInfo* >(this)),
                                        (static_cast< ::com::sun::star::io::XPersistObject* >(this)),
                                        (static_cast< ::com::sun::star::lang::XTypeProvider* >(this)) );
    return (aRet.hasValue() ? aRet : OWeakAggObject::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( StdTabControllerModel )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject>* ) NULL )
IMPL_XTYPEPROVIDER_END

sal_Bool StdTabControllerModel::getGroupControl(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mbGroupControl;
}

void StdTabControllerModel::setGroupControl( sal_Bool GroupControl ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mbGroupControl = GroupControl;
}

void StdTabControllerModel::setControlModels( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Controls ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maControls.Reset();
    ImplSetControlModels( maControls, Controls );
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > StdTabControllerModel::getControlModels(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aSeq( ImplGetControlCount( maControls ) );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > * pRefs = aSeq.getArray();
    ImplGetControlModels( &pRefs, maControls );
    return aSeq;
}

void StdTabControllerModel::setGroup( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group, const ::rtl::OUString& GroupName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // The controls might occur as a flat list and will be grouped.
    // Nested groups are not possible.
    // The first element of a group determines its position.
    UnoControlModelEntry* pNewEntry = new UnoControlModelEntry;
    pNewEntry->bGroup = sal_True;
    pNewEntry->pGroup = new UnoControlModelEntryList;
    pNewEntry->pGroup->SetName( GroupName );
    ImplSetControlModels( *pNewEntry->pGroup, Group );

    sal_Bool bInserted = sal_False;
    size_t nElements = pNewEntry->pGroup->size();
    for ( size_t n = 0; n < nElements; n++ )
    {
        UnoControlModelEntry* pEntry = (*pNewEntry->pGroup)[ n ];
        if ( !pEntry->bGroup )
        {
            sal_uInt32 nPos = ImplGetControlPos( *pEntry->pxControl, maControls );
            // At the beginning, all Controls should be in a flattened list
            DBG_ASSERT( nPos != CONTROLPOS_NOTFOUND, "setGroup - Element not found" );
            if ( nPos != CONTROLPOS_NOTFOUND )
            {
                maControls.DestroyEntry( nPos );
                if ( !bInserted )
                {
                    maControls.insert( nPos, pNewEntry );
                    bInserted = sal_True;
                }
            }
        }
    }
    if ( !bInserted )
        maControls.push_back( pNewEntry );
}

sal_Int32 StdTabControllerModel::getGroupCount(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // Start with only one group layer, even though Model and Impl-methods
    // work recursively, this is not presented to the outside.

    sal_Int32 nGroups = 0;
    size_t nEntries = maControls.size();
    for ( size_t n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry* pEntry = maControls[ n ];
        if ( pEntry->bGroup )
            nGroups++;
    }
    return nGroups;
}

void StdTabControllerModel::getGroup( sal_Int32 nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& rGroup, ::rtl::OUString& rName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aSeq;
    sal_uInt32 nG = 0;
    size_t nEntries = maControls.size();
    for ( size_t n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry* pEntry = maControls[ n ];
        if ( pEntry->bGroup )
        {
            if ( nG == (sal_uInt32)nGroup )
            {
                sal_uInt32 nCount = ImplGetControlCount( *pEntry->pGroup );
                aSeq = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >( nCount );
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > * pRefs = aSeq.getArray();
                ImplGetControlModels( &pRefs, *pEntry->pGroup );
                rName = pEntry->pGroup->GetName();
                break;
            }
            nG++;
        }
    }
    rGroup = aSeq;
}

void StdTabControllerModel::getGroupByName( const ::rtl::OUString& rName, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& rGroup ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nGroup = 0;
    size_t nEntries = maControls.size();
    for ( size_t n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry* pEntry = maControls[ n ];
        if ( pEntry->bGroup )
        {
            if ( pEntry->pGroup->GetName() == rName )
            {
                ::rtl::OUString Dummy;
                getGroup( nGroup, rGroup, Dummy );
                break;
            }
            nGroup++;
        }
    }
}


// ::com::sun::star::io::XPersistObject
::rtl::OUString StdTabControllerModel::getServiceName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_TabControllerModel );
}

void StdTabControllerModel::write( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& OutStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XMarkableStream >  xMark( OutStream, ::com::sun::star::uno::UNO_QUERY );
    DBG_ASSERT( xMark.is(), "write: no XMarkableStream!" );

    OutStream->writeShort( UNOCONTROL_STREAMVERSION );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aCtrls = getControlModels();
    ImplWriteControls( OutStream, aCtrls );

    sal_uInt32 nGroups = getGroupCount();
    OutStream->writeLong( nGroups );
    for ( sal_uInt32 n = 0; n < nGroups; n++ )
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aGroupCtrls;
        ::rtl::OUString aGroupName;
        getGroup( n, aGroupCtrls, aGroupName );
        OutStream->writeUTF( aGroupName );
        ImplWriteControls( OutStream, aGroupCtrls );
    }
}

void StdTabControllerModel::read( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& InStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aSeq = ImplReadControls( InStream );
    setControlModels( aSeq );

    sal_uInt32 nGroups = InStream->readLong();
    for ( sal_uInt32 n = 0; n < nGroups; n++ )
    {
        ::rtl::OUString aGroupName = InStream->readUTF();
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aCtrlSeq = ImplReadControls( InStream );
        setGroup( aCtrlSeq, aGroupName );
    }
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
