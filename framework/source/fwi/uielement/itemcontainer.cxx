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

#include <uielement/itemcontainer.hxx>
#include <uielement/constitemcontainer.hxx>
#include <threadhelp/resetableguard.hxx>
#include <comphelper/servicehelper.hxx>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

const char WRONG_TYPE_EXCEPTION[] = "Type must be com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >";

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************

ItemContainer::ItemContainer( const ShareableMutex& rMutex ) :
    m_aShareMutex( rMutex )
{
}


ItemContainer::ItemContainer( const ConstItemContainer& rConstItemContainer, const ShareableMutex& rMutex ) : m_aShareMutex( rMutex )
{
    copyItemContainer( rConstItemContainer.m_aItemVector, rMutex );
}

ItemContainer::ItemContainer( const Reference< XIndexAccess >& rSourceContainer, const ShareableMutex& rMutex ) :
    m_aShareMutex( rMutex )
{
    if ( rSourceContainer.is() )
    {
        sal_Int32 nCount = rSourceContainer->getCount();
        try
        {
            for ( sal_Int32 i = 0; i < nCount; i++ )
            {
                Sequence< PropertyValue > aPropSeq;
                if ( rSourceContainer->getByIndex( i ) >>= aPropSeq )
                {
                    sal_Int32 nContainerIndex = -1;
                    Reference< XIndexAccess > xIndexAccess;
                    for ( sal_Int32 j = 0; j < aPropSeq.getLength(); j++ )
                    {
                        if ( aPropSeq[j].Name == "ItemDescriptorContainer" )
                        {
                            aPropSeq[j].Value >>= xIndexAccess;
                            nContainerIndex = j;
                            break;
                        }
                    }

                    if ( xIndexAccess.is() && nContainerIndex >= 0 )
                        aPropSeq[nContainerIndex].Value <<= deepCopyContainer( xIndexAccess, rMutex );

                    m_aItemVector.push_back( aPropSeq );
                }
            }
        }
        catch ( const IndexOutOfBoundsException& )
        {
        }
    }
}

ItemContainer::~ItemContainer()
{
}

// private
void ItemContainer::copyItemContainer( const std::vector< Sequence< PropertyValue > >& rSourceVector, const ShareableMutex& rMutex )
{
    const sal_uInt32 nCount = rSourceVector.size();
    for ( sal_uInt32 i = 0; i < nCount; ++i )
    {
        sal_Int32 nContainerIndex = -1;
        Sequence< PropertyValue > aPropSeq( rSourceVector[i] );
        Reference< XIndexAccess > xIndexAccess;
        for ( sal_Int32 j = 0; j < aPropSeq.getLength(); j++ )
        {
            if ( aPropSeq[j].Name == "ItemDescriptorContainer" )
            {
                aPropSeq[j].Value >>= xIndexAccess;
                nContainerIndex = j;
                break;
            }
        }

        if ( xIndexAccess.is() && nContainerIndex >= 0 )
            aPropSeq[nContainerIndex].Value <<= deepCopyContainer( xIndexAccess, rMutex );

        m_aItemVector.push_back( aPropSeq );
    }
}

Reference< XIndexAccess > ItemContainer::deepCopyContainer( const Reference< XIndexAccess >& rSubContainer, const ShareableMutex& rMutex )
{
    Reference< XIndexAccess > xReturn;
    if ( rSubContainer.is() )
    {
        ConstItemContainer* pSource = ConstItemContainer::GetImplementation( rSubContainer );
        ItemContainer* pSubContainer( 0 );
        if ( pSource )
            pSubContainer = new ItemContainer( *pSource, rMutex );
        else
            pSubContainer = new ItemContainer( rSubContainer, rMutex );
        xReturn = Reference< XIndexAccess >( static_cast< OWeakObject* >( pSubContainer ), UNO_QUERY );
    }

    return xReturn;
}

namespace
{
    class theItemContainerUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theItemContainerUnoTunnelId > {};
}

const Sequence< sal_Int8 >& ItemContainer::GetUnoTunnelId() throw()
{
    return theItemContainerUnoTunnelId::get().getSeq();
}

ItemContainer* ItemContainer::GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( rxIFace, ::com::sun::star::uno::UNO_QUERY );
    return xUT.is() ? reinterpret_cast< ItemContainer* >(sal::static_int_cast< sal_IntPtr >(
                          xUT->getSomething( ItemContainer::GetUnoTunnelId() ))) : NULL;
}

// XElementAccess
sal_Bool SAL_CALL ItemContainer::hasElements()
throw ( RuntimeException )
{
    ShareGuard aLock( m_aShareMutex );
    return ( !m_aItemVector.empty() );
}

// XIndexAccess
sal_Int32 SAL_CALL ItemContainer::getCount()
throw ( RuntimeException )
{
    ShareGuard aLock( m_aShareMutex );
    return m_aItemVector.size();
}

Any SAL_CALL ItemContainer::getByIndex( sal_Int32 Index )
throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ShareGuard aLock( m_aShareMutex );
    if ( sal_Int32( m_aItemVector.size()) > Index )
        return makeAny( m_aItemVector[Index] );
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

// XIndexContainer
void SAL_CALL ItemContainer::insertByIndex( sal_Int32 Index, const Any& aItem )
throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    Sequence< PropertyValue > aSeq;
    if ( aItem >>= aSeq )
    {
        ShareGuard aLock( m_aShareMutex );
        if ( sal_Int32( m_aItemVector.size()) == Index )
            m_aItemVector.push_back( aSeq );
        else if ( sal_Int32( m_aItemVector.size()) >Index )
        {
            std::vector< Sequence< PropertyValue > >::iterator aIter = m_aItemVector.begin();
            aIter += Index;
            m_aItemVector.insert( aIter, aSeq );
        }
        else
            throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
    }
    else
        throw IllegalArgumentException( OUString( WRONG_TYPE_EXCEPTION ),
                                        (OWeakObject *)this, 2 );
}

void SAL_CALL ItemContainer::removeByIndex( sal_Int32 nIndex )
throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ShareGuard aLock( m_aShareMutex );
    if ( (sal_Int32)m_aItemVector.size() > nIndex )
    {
        m_aItemVector.erase(m_aItemVector.begin() + nIndex);
    }
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

void SAL_CALL ItemContainer::replaceByIndex( sal_Int32 Index, const Any& aItem )
throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    Sequence< PropertyValue > aSeq;
    if ( aItem >>= aSeq )
    {
        ShareGuard aLock( m_aShareMutex );
        if ( sal_Int32( m_aItemVector.size()) > Index )
            m_aItemVector[Index] = aSeq;
        else
            throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
    }
    else
        throw IllegalArgumentException( OUString( WRONG_TYPE_EXCEPTION ),
                                        (OWeakObject *)this, 2 );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
