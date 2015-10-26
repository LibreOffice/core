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
#include <com/sun/star/uno/XComponentContext.hpp>

#include <toolkit/controls/stdtabcontrollermodel.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/helper/property.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <rtl/uuid.h>

#include <tools/debug.hxx>

#define UNOCONTROL_STREAMVERSION    (short)2


//  class UnoControlModelEntryList

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

    if ( (*it).bGroup )
        delete (*it).pGroup;
    else
        (*it).xControl.clear();

    maList.erase( it );
}

size_t  UnoControlModelEntryList::size() const {
    return maList.size();
}

void UnoControlModelEntryList::push_back( const UnoControlModelEntry& item ) {
    maList.push_back( item );
}

void UnoControlModelEntryList::insert( size_t i, const UnoControlModelEntry& item ) {
    if ( i < maList.size() ) {
        UnoControlModelEntryListBase::iterator it = maList.begin();
        ::std::advance( it, i );
        maList.insert( it, item );
    } else {
        maList.push_back( item );
    }
}

UnoControlModelEntry::UnoControlModelEntry( const UnoControlModelEntry& other)
    : bGroup(other.bGroup)
{
    if (bGroup)
        pGroup = other.pGroup;
    else
        xControl = other.xControl;
}

UnoControlModelEntry& UnoControlModelEntry::operator=( const UnoControlModelEntry& other)
{
    if (!bGroup)
        xControl.clear();
    bGroup = other.bGroup;
    if (bGroup)
        pGroup = other.pGroup;
    else
        xControl = other.xControl;
    return *this;
}

UnoControlModelEntry::~UnoControlModelEntry()
{
    if (!bGroup)
        xControl.clear();
}

//  class StdTabControllerModel

StdTabControllerModel::StdTabControllerModel()
{
    mbGroupControl = true;
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
        const UnoControlModelEntry& rEntry = rList[ n ];
        if ( rEntry.bGroup )
            nCount += ImplGetControlCount( *rEntry.pGroup );
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
        const UnoControlModelEntry& rEntry = rList[ n ];
        if ( rEntry.bGroup )
            ImplGetControlModels( ppRefs, *rEntry.pGroup );
        else
        {
            **ppRefs = rEntry.xControl;
            (*ppRefs)++;
        }
    }
}

void StdTabControllerModel::ImplSetControlModels( UnoControlModelEntryList& rList, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Controls )
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > * pRefs = Controls.getConstArray();
    sal_uInt32 nControls = Controls.getLength();
    for ( sal_uInt32 n = 0; n < nControls; n++ )
    {
        UnoControlModelEntry aNewEntry;
        aNewEntry.bGroup = false;
        aNewEntry.xControl = pRefs[n];
        rList.push_back( aNewEntry );
    }
}

sal_uInt32 StdTabControllerModel::ImplGetControlPos( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rCtrl, const UnoControlModelEntryList& rList )
{
    for ( size_t n = rList.size(); n; )
    {
        const UnoControlModelEntry& rEntry = rList[ --n ];
        if ( !rEntry.bGroup && ( rEntry.xControl == rCtrl ) )
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
::com::sun::star::uno::Any StdTabControllerModel::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
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
    cppu::UnoType<com::sun::star::awt::XTabControllerModel>::get(),
    cppu::UnoType<com::sun::star::lang::XServiceInfo>::get(),
    cppu::UnoType<com::sun::star::io::XPersistObject>::get()
IMPL_XTYPEPROVIDER_END

sal_Bool StdTabControllerModel::getGroupControl(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mbGroupControl;
}

void StdTabControllerModel::setGroupControl( sal_Bool GroupControl ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mbGroupControl = GroupControl;
}

void StdTabControllerModel::setControlModels( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Controls ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maControls.Reset();
    ImplSetControlModels( maControls, Controls );
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > StdTabControllerModel::getControlModels(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aSeq( ImplGetControlCount( maControls ) );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > * pRefs = aSeq.getArray();
    ImplGetControlModels( &pRefs, maControls );
    return aSeq;
}

void StdTabControllerModel::setGroup( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group, const OUString& GroupName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // The controls might occur as a flat list and will be grouped.
    // Nested groups are not possible.
    // The first element of a group determines its position.
    UnoControlModelEntry aNewEntry;
    aNewEntry.bGroup = true;
    aNewEntry.pGroup = new UnoControlModelEntryList;
    aNewEntry.pGroup->SetName( GroupName );
    ImplSetControlModels( *aNewEntry.pGroup, Group );

    bool bInserted = false;
    size_t nElements = aNewEntry.pGroup->size();
    for ( size_t n = 0; n < nElements; n++ )
    {
        UnoControlModelEntry& rEntry = (*aNewEntry.pGroup)[ n ];
        if ( !rEntry.bGroup )
        {
            sal_uInt32 nPos = ImplGetControlPos( rEntry.xControl, maControls );
            // At the beginning, all Controls should be in a flattened list
            DBG_ASSERT( nPos != CONTROLPOS_NOTFOUND, "setGroup - Element not found" );
            if ( nPos != CONTROLPOS_NOTFOUND )
            {
                maControls.DestroyEntry( nPos );
                if ( !bInserted )
                {
                    maControls.insert( nPos, aNewEntry );
                    bInserted = true;
                }
            }
        }
    }
    if ( !bInserted )
        maControls.push_back( aNewEntry );
}

sal_Int32 StdTabControllerModel::getGroupCount(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // Start with only one group layer, even though Model and Impl-methods
    // work recursively, this is not presented to the outside.

    sal_Int32 nGroups = 0;
    size_t nEntries = maControls.size();
    for ( size_t n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry& rEntry = maControls[ n ];
        if ( rEntry.bGroup )
            nGroups++;
    }
    return nGroups;
}

void StdTabControllerModel::getGroup( sal_Int32 nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& rGroup, OUString& rName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aSeq;
    sal_uInt32 nG = 0;
    size_t nEntries = maControls.size();
    for ( size_t n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry& rEntry = maControls[ n ];
        if ( rEntry.bGroup )
        {
            if ( nG == (sal_uInt32)nGroup )
            {
                sal_uInt32 nCount = ImplGetControlCount( *rEntry.pGroup );
                aSeq = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >( nCount );
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > * pRefs = aSeq.getArray();
                ImplGetControlModels( &pRefs, *rEntry.pGroup );
                rName = rEntry.pGroup->GetName();
                break;
            }
            nG++;
        }
    }
    rGroup = aSeq;
}

void StdTabControllerModel::getGroupByName( const OUString& rName, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& rGroup ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nGroup = 0;
    size_t nEntries = maControls.size();
    for ( size_t n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry& rEntry = maControls[ n ];
        if ( rEntry.bGroup )
        {
            if ( rEntry.pGroup->GetName() == rName )
            {
                OUString Dummy;
                getGroup( nGroup, rGroup, Dummy );
                break;
            }
            nGroup++;
        }
    }
}


// ::com::sun::star::io::XPersistObject
OUString StdTabControllerModel::getServiceName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return OUString::createFromAscii( szServiceName_TabControllerModel );
}

void StdTabControllerModel::write( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& OutStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
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
        OUString aGroupName;
        getGroup( n, aGroupCtrls, aGroupName );
        OutStream->writeUTF( aGroupName );
        ImplWriteControls( OutStream, aGroupCtrls );
    }
}

void StdTabControllerModel::read( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& InStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aSeq = ImplReadControls( InStream );
    setControlModels( aSeq );

    sal_uInt32 nGroups = InStream->readLong();
    for ( sal_uInt32 n = 0; n < nGroups; n++ )
    {
        OUString aGroupName = InStream->readUTF();
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aCtrlSeq = ImplReadControls( InStream );
        setGroup( aCtrlSeq, aGroupName );
    }
}

OUString StdTabControllerModel::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("stardiv.Toolkit.StdTabControllerModel");
}

sal_Bool StdTabControllerModel::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> StdTabControllerModel::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<OUString>{
        OUString::createFromAscii(szServiceName2_TabControllerModel),
        "stardiv.vcl.controlmodel.TabController"};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
stardiv_Toolkit_StdTabControllerModel_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new StdTabControllerModel());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
