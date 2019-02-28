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

#define UNOCONTROL_STREAMVERSION    short(2)


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
    return ( i < maList.size() ) ? maList[ i ] : nullptr;
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
        UnoControlModelEntry* pEntry = rList[ n ];
        if ( pEntry->bGroup )
            nCount += ImplGetControlCount( *pEntry->pGroup );
        else
            nCount++;
    }
    return nCount;
}

void StdTabControllerModel::ImplGetControlModels( css::uno::Reference< css::awt::XControlModel > ** ppRefs, const UnoControlModelEntryList& rList ) const
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

void StdTabControllerModel::ImplSetControlModels( UnoControlModelEntryList& rList, const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Controls )
{
    const css::uno::Reference< css::awt::XControlModel > * pRefs = Controls.getConstArray();
    sal_uInt32 nControls = Controls.getLength();
    for ( sal_uInt32 n = 0; n < nControls; n++ )
    {
        UnoControlModelEntry* pNewEntry = new UnoControlModelEntry;
        pNewEntry->bGroup = false;
        pNewEntry->pxControl = new css::uno::Reference< css::awt::XControlModel > ;
        *pNewEntry->pxControl = pRefs[n];
        rList.push_back( pNewEntry );
    }
}

sal_uInt32 StdTabControllerModel::ImplGetControlPos( const css::uno::Reference< css::awt::XControlModel >& rCtrl, const UnoControlModelEntryList& rList )
{
    for ( size_t n = rList.size(); n; )
    {
        UnoControlModelEntry* pEntry = rList[ --n ];
        if ( !pEntry->bGroup && ( *pEntry->pxControl == rCtrl ) )
            return n;
    }
    return CONTROLPOS_NOTFOUND;
}

static void ImplWriteControls( const css::uno::Reference< css::io::XObjectOutputStream > & OutStream, const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& rCtrls )
{
    css::uno::Reference< css::io::XMarkableStream >  xMark( OutStream, css::uno::UNO_QUERY );
    DBG_ASSERT( xMark.is(), "write: no XMarkableStream!" );

    sal_uInt32 nStoredControls = 0;
    sal_Int32 nDataBeginMark = xMark->createMark();

    OutStream->writeLong( 0 ); // DataLen
    OutStream->writeLong( 0 ); // nStoredControls

    sal_uInt32 nCtrls = rCtrls.getLength();
    for ( sal_uInt32 n = 0; n < nCtrls; n++ )
    {
        const css::uno::Reference< css::awt::XControlModel >  xI = rCtrls.getConstArray()[n];
        css::uno::Reference< css::io::XPersistObject >  xPO( xI, css::uno::UNO_QUERY );
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

static css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > ImplReadControls( const css::uno::Reference< css::io::XObjectInputStream > & InStream )
{
    css::uno::Reference< css::io::XMarkableStream >  xMark( InStream, css::uno::UNO_QUERY );
    DBG_ASSERT( xMark.is(), "write: no XMarkableStream!" );

    sal_Int32 nDataBeginMark = xMark->createMark();

    sal_Int32 nDataLen = InStream->readLong();
    sal_uInt32 nCtrls = InStream->readLong();

    css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > aSeq( nCtrls );
    for ( sal_uInt32 n = 0; n < nCtrls; n++ )
    {
        css::uno::Reference< css::io::XPersistObject >  xObj = InStream->readObject();
        css::uno::Reference< css::awt::XControlModel >  xI( xObj, css::uno::UNO_QUERY );
        aSeq.getArray()[n] = xI;
    }

    // Skip remainder if more data exists than this version recognizes
    xMark->jumpToMark( nDataBeginMark );
    InStream->skipBytes( nDataLen );
    xMark->deleteMark(nDataBeginMark);
    return aSeq;
}


// css::uno::XInterface
css::uno::Any StdTabControllerModel::queryAggregation( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XTabControllerModel* >(this),
                                        static_cast< css::lang::XServiceInfo* >(this),
                                        static_cast< css::io::XPersistObject* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : OWeakAggObject::queryAggregation( rType ));
}

IMPL_IMPLEMENTATION_ID( StdTabControllerModel )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > StdTabControllerModel::getTypes()
{
    static const css::uno::Sequence< css::uno::Type > aTypeList {
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XTabControllerModel>::get(),
        cppu::UnoType<css::lang::XServiceInfo>::get(),
        cppu::UnoType<css::io::XPersistObject>::get()
    };
    return aTypeList;
}

sal_Bool StdTabControllerModel::getGroupControl(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mbGroupControl;
}

void StdTabControllerModel::setGroupControl( sal_Bool GroupControl )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mbGroupControl = GroupControl;
}

void StdTabControllerModel::setControlModels( const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Controls )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maControls.Reset();
    ImplSetControlModels( maControls, Controls );
}

css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > StdTabControllerModel::getControlModels(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > aSeq( ImplGetControlCount( maControls ) );
    css::uno::Reference< css::awt::XControlModel > * pRefs = aSeq.getArray();
    ImplGetControlModels( &pRefs, maControls );
    return aSeq;
}

void StdTabControllerModel::setGroup( const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Group, const OUString& GroupName )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // The controls might occur as a flat list and will be grouped.
    // Nested groups are not possible.
    // The first element of a group determines its position.
    UnoControlModelEntry* pNewEntry = new UnoControlModelEntry;
    pNewEntry->bGroup = true;
    pNewEntry->pGroup = new UnoControlModelEntryList;
    pNewEntry->pGroup->SetName( GroupName );
    ImplSetControlModels( *pNewEntry->pGroup, Group );

    bool bInserted = false;
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
                    bInserted = true;
                }
            }
        }
    }
    if ( !bInserted )
        maControls.push_back( pNewEntry );
}

sal_Int32 StdTabControllerModel::getGroupCount(  )
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

void StdTabControllerModel::getGroup( sal_Int32 nGroup, css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& rGroup, OUString& rName )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > aSeq;
    sal_uInt32 nG = 0;
    size_t nEntries = maControls.size();
    for ( size_t n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry* pEntry = maControls[ n ];
        if ( pEntry->bGroup )
        {
            if ( nG == static_cast<sal_uInt32>(nGroup) )
            {
                sal_uInt32 nCount = ImplGetControlCount( *pEntry->pGroup );
                aSeq = css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >( nCount );
                css::uno::Reference< css::awt::XControlModel > * pRefs = aSeq.getArray();
                ImplGetControlModels( &pRefs, *pEntry->pGroup );
                rName = pEntry->pGroup->GetName();
                break;
            }
            nG++;
        }
    }
    rGroup = aSeq;
}

void StdTabControllerModel::getGroupByName( const OUString& rName, css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& rGroup )
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
                OUString Dummy;
                getGroup( nGroup, rGroup, Dummy );
                break;
            }
            nGroup++;
        }
    }
}


// css::io::XPersistObject
OUString StdTabControllerModel::getServiceName(  )
{
    return OUString::createFromAscii( szServiceName_TabControllerModel );
}

void StdTabControllerModel::write( const css::uno::Reference< css::io::XObjectOutputStream >& OutStream )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::uno::Reference< css::io::XMarkableStream >  xMark( OutStream, css::uno::UNO_QUERY );
    DBG_ASSERT( xMark.is(), "write: no XMarkableStream!" );

    OutStream->writeShort( UNOCONTROL_STREAMVERSION );

    css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > aCtrls = getControlModels();
    ImplWriteControls( OutStream, aCtrls );

    sal_uInt32 nGroups = getGroupCount();
    OutStream->writeLong( nGroups );
    for ( sal_uInt32 n = 0; n < nGroups; n++ )
    {
        css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > aGroupCtrls;
        OUString aGroupName;
        getGroup( n, aGroupCtrls, aGroupName );
        OutStream->writeUTF( aGroupName );
        ImplWriteControls( OutStream, aGroupCtrls );
    }
}

void StdTabControllerModel::read( const css::uno::Reference< css::io::XObjectInputStream >& InStream )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > aSeq = ImplReadControls( InStream );
    setControlModels( aSeq );

    sal_uInt32 nGroups = InStream->readLong();
    for ( sal_uInt32 n = 0; n < nGroups; n++ )
    {
        OUString aGroupName = InStream->readUTF();
        css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > aCtrlSeq = ImplReadControls( InStream );
        setGroup( aCtrlSeq, aGroupName );
    }
}

OUString StdTabControllerModel::getImplementationName()
{
    return OUString("stardiv.Toolkit.StdTabControllerModel");
}

sal_Bool StdTabControllerModel::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> StdTabControllerModel::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{
        OUString::createFromAscii(szServiceName2_TabControllerModel),
        "stardiv.vcl.controlmodel.TabController"};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_StdTabControllerModel_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new StdTabControllerModel());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
