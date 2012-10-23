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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess2.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>

#include "ohierarchyholder.hxx"

using namespace ::com::sun::star;

//===============================================
// OHierarchyHolder_Impl
//===============================================

//-----------------------------------------------
uno::Reference< embed::XExtendedStorageStream > OHierarchyHolder_Impl::GetStreamHierarchically( sal_Int32 nStorageMode, OStringList_Impl& aListPath, sal_Int32 nStreamMode, const ::comphelper::SequenceAsHashMap& aEncryptionData )
{
    uno::Reference< embed::XStorage > xOwnStor( m_xWeakOwnStorage.get(), uno::UNO_QUERY_THROW );

    if ( !( nStorageMode & embed::ElementModes::WRITE ) && ( nStreamMode & embed::ElementModes::WRITE ) )
        throw io::IOException();

    uno::Reference< embed::XExtendedStorageStream > xResult =
        m_xChild->GetStreamHierarchically( nStorageMode, aListPath, nStreamMode, aEncryptionData );
    if ( !xResult.is() )
        throw uno::RuntimeException();

    return xResult;
}

//-----------------------------------------------
void OHierarchyHolder_Impl::RemoveStreamHierarchically( OStringList_Impl& aListPath )
{
    uno::Reference< embed::XStorage > xOwnStor( m_xWeakOwnStorage.get(), uno::UNO_QUERY_THROW );

    m_xChild->RemoveStreamHierarchically( aListPath );
}

//-----------------------------------------------
// static
OStringList_Impl OHierarchyHolder_Impl::GetListPathFromString( const ::rtl::OUString& aPath )
{
    OStringList_Impl aResult;
    sal_Int32 nIndex = 0;
    do
    {
        ::rtl::OUString aName = aPath.getToken( 0, '/', nIndex );
        if ( aName.isEmpty() )
            throw lang::IllegalArgumentException();

        aResult.push_back( aName );
    }
    while ( nIndex >= 0 );

    return aResult;
}

//===============================================
// OHierarchyElement_Impl
//===============================================

//-----------------------------------------------
uno::Reference< embed::XExtendedStorageStream > OHierarchyElement_Impl::GetStreamHierarchically( sal_Int32 nStorageMode, OStringList_Impl& aListPath, sal_Int32 nStreamMode, const ::comphelper::SequenceAsHashMap& aEncryptionData )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !( nStorageMode & embed::ElementModes::WRITE ) && ( nStreamMode & embed::ElementModes::WRITE ) )
        throw io::IOException();

    if ( !aListPath.size() )
        throw uno::RuntimeException();

    ::rtl::OUString aNextName = *(aListPath.begin());
    aListPath.erase( aListPath.begin() );

    uno::Reference< embed::XExtendedStorageStream > xResult;

    uno::Reference< embed::XStorage > xOwnStor = m_xOwnStorage.is() ? m_xOwnStorage
                : uno::Reference< embed::XStorage >( m_xWeakOwnStorage.get(), uno::UNO_QUERY );
    if ( !xOwnStor.is() )
        throw uno::RuntimeException();

    if ( !aListPath.size() )
    {
        if ( !aEncryptionData.size() )
        {
            uno::Reference< embed::XHierarchicalStorageAccess > xHStorage( xOwnStor, uno::UNO_QUERY_THROW );
            xResult = xHStorage->openStreamElementByHierarchicalName( aNextName, nStreamMode );
        }
        else
        {
            uno::Reference< embed::XHierarchicalStorageAccess2 > xHStorage( xOwnStor, uno::UNO_QUERY_THROW );
            xResult = xHStorage->openEncryptedStreamByHierarchicalName( aNextName, nStreamMode, aEncryptionData.getAsConstNamedValueList() );
        }

        uno::Reference< embed::XTransactedObject > xTransact( xResult, uno::UNO_QUERY );
        if ( xTransact.is() )
        {
            // the existance of the transacted object means that the stream is opened for writing also
            // so the whole chain must be commited
            uno::Reference< embed::XTransactionBroadcaster > xTrBroadcast( xTransact, uno::UNO_QUERY_THROW );
            xTrBroadcast->addTransactionListener( static_cast< embed::XTransactionListener* >( this ) );
        }
        else
        {
            uno::Reference< lang::XComponent > xStreamComp( xResult, uno::UNO_QUERY_THROW );
            xStreamComp->addEventListener( static_cast< lang::XEventListener* >( this ) );
        }

        m_aOpenStreams.push_back( uno::WeakReference< embed::XExtendedStorageStream >( xResult ) );
    }
    else
    {
        sal_Bool bNewElement = sal_False;
        ::rtl::Reference< OHierarchyElement_Impl > aElement;
        OHierarchyElementList_Impl::iterator aIter = m_aChildren.find( aNextName );
        if ( aIter != m_aChildren.end() )
            aElement = aIter->second;

        if ( !aElement.is() )
        {
            bNewElement = sal_True;
            uno::Reference< embed::XStorage > xChildStorage = xOwnStor->openStorageElement( aNextName, nStorageMode );
            if ( !xChildStorage.is() )
                throw uno::RuntimeException();

            aElement = new OHierarchyElement_Impl( NULL, xChildStorage );
        }

        xResult = aElement->GetStreamHierarchically( nStorageMode, aListPath, nStreamMode, aEncryptionData );
        if ( !xResult.is() )
            throw uno::RuntimeException();

        if ( bNewElement )
        {
            m_aChildren[aNextName] = aElement;
            aElement->SetParent( this );
        }
    }

    // the subelement was opened successfuly, remember the storage to let it be locked
    m_xOwnStorage = xOwnStor;

    return xResult;
}

//-----------------------------------------------
void OHierarchyElement_Impl::RemoveStreamHierarchically( OStringList_Impl& aListPath )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !aListPath.size() )
        throw uno::RuntimeException();

    ::rtl::OUString aNextName = *(aListPath.begin());
    aListPath.erase( aListPath.begin() );

    uno::Reference< embed::XExtendedStorageStream > xResult;

    uno::Reference< embed::XStorage > xOwnStor = m_xOwnStorage.is() ? m_xOwnStorage
                : uno::Reference< embed::XStorage >( m_xWeakOwnStorage.get(), uno::UNO_QUERY );
    if ( !xOwnStor.is() )
        throw uno::RuntimeException();

    if ( !aListPath.size() )
    {
        xOwnStor->removeElement( aNextName );
    }
    else
    {
        ::rtl::Reference< OHierarchyElement_Impl > aElement;
        OHierarchyElementList_Impl::iterator aIter = m_aChildren.find( aNextName );
        if ( aIter != m_aChildren.end() )
            aElement = aIter->second;

        if ( !aElement.is() )
        {
            uno::Reference< embed::XStorage > xChildStorage = xOwnStor->openStorageElement( aNextName,
                                                                                            embed::ElementModes::READWRITE );
            if ( !xChildStorage.is() )
                throw uno::RuntimeException();

            aElement = new OHierarchyElement_Impl( NULL, xChildStorage );
        }

        aElement->RemoveStreamHierarchically( aListPath );
    }

    uno::Reference< embed::XTransactedObject > xTransact( xOwnStor, uno::UNO_QUERY );
    if ( xTransact.is() )
        xTransact->commit();

    TestForClosing();
}

//-----------------------------------------------
void OHierarchyElement_Impl::Commit()
{
    ::rtl::Reference< OHierarchyElement_Impl > aLocker( this );
    ::rtl::Reference< OHierarchyElement_Impl > aParent;
    uno::Reference< embed::XStorage > xOwnStor;

    {
        ::osl::MutexGuard aGuard( m_aMutex );
        aParent = m_rParent;
        xOwnStor = m_xOwnStorage;
    }

    if ( xOwnStor.is() )
    {
        uno::Reference< embed::XTransactedObject > xTransact( xOwnStor, uno::UNO_QUERY_THROW );
        xTransact->commit();
        if ( aParent.is() )
            aParent->Commit();
    }
}

//-----------------------------------------------
void OHierarchyElement_Impl::TestForClosing()
{
    ::rtl::Reference< OHierarchyElement_Impl > aLocker( this );
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !m_aOpenStreams.size() && !m_aChildren.size() )
        {
            if ( m_rParent.is() )
            {
                // only the root storage should not be disposed, other storages can be disposed
                if ( m_xOwnStorage.is() )
                {
                    try
                    {
                        m_xOwnStorage->dispose();
                    }
                    catch( uno::Exception& )
                    {}
                }

                m_rParent->RemoveElement( this );
            }

            m_xOwnStorage = uno::Reference< embed::XStorage >();
        }
    }
}

//-----------------------------------------------
void SAL_CALL OHierarchyElement_Impl::disposing( const lang::EventObject& Source )
        throw ( uno::RuntimeException )
{
    uno::Sequence< embed::XStorage > aStoragesToCommit;

    try
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        uno::Reference< embed::XExtendedStorageStream > xStream( Source.Source, uno::UNO_QUERY );

        for ( OWeakStorRefList_Impl::iterator pStorageIter = m_aOpenStreams.begin();
              pStorageIter != m_aOpenStreams.end(); )
        {
            OWeakStorRefList_Impl::iterator pTmp = pStorageIter++;
            if ( !pTmp->get().is() || pTmp->get() == xStream )
                m_aOpenStreams.erase( pTmp );
        }

        aGuard.clear();

        TestForClosing();
    }
    catch( uno::Exception& )
    {
        throw uno::RuntimeException(); // no exception must happen here, usually an exception means disaster
    }
}

//-----------------------------------------------
void OHierarchyElement_Impl::RemoveElement( const ::rtl::Reference< OHierarchyElement_Impl >& aRef )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        for ( OHierarchyElementList_Impl::iterator aIter = m_aChildren.begin();
              aIter != m_aChildren.end(); /* increment is done in body */)
        {
            OHierarchyElementList_Impl::iterator aTmpIter = aIter;
            ++aIter;

            if ( aTmpIter->second == aRef )
                m_aChildren.erase( aTmpIter );
        }
    }

    TestForClosing();
}

// XTransactionListener
//-----------------------------------------------
void SAL_CALL OHierarchyElement_Impl::preCommit( const ::com::sun::star::lang::EventObject& /*aEvent*/ )
    throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
}

//-----------------------------------------------
void SAL_CALL OHierarchyElement_Impl::commited( const ::com::sun::star::lang::EventObject& /*aEvent*/ )
    throw (::com::sun::star::uno::RuntimeException)
{
    try
    {
        Commit();
    }
    catch( const uno::Exception& e )
    {
        throw lang::WrappedTargetRuntimeException(
                            "Can not commit storage sequence!",
                            uno::Reference< uno::XInterface >(),
                            uno::makeAny( e ) );
    }
}

//-----------------------------------------------
void SAL_CALL OHierarchyElement_Impl::preRevert( const ::com::sun::star::lang::EventObject& /*aEvent*/ )
    throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
}

//-----------------------------------------------
void SAL_CALL OHierarchyElement_Impl::reverted( const ::com::sun::star::lang::EventObject& /*aEvent*/ )
    throw (::com::sun::star::uno::RuntimeException)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
