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

#include "ocompinstream.hxx"
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <utility>

#include "owriteablestream.hxx"

using namespace ::com::sun::star;

OInputCompStream::OInputCompStream( OWriteStream_Impl& aImpl,
                                    uno::Reference < io::XInputStream > xStream,
                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                    sal_Int32 nStorageType )
: m_pImpl( &aImpl )
, m_xMutex( m_pImpl->m_xMutex )
, m_xStream(std::move( xStream ))
, m_aProperties( aProps )
, m_bDisposed( false )
, m_nStorageType( nStorageType )
{
    OSL_ENSURE( m_pImpl->m_xMutex.is(), "No mutex is provided!" );
    if ( !m_pImpl->m_xMutex.is() )
        throw uno::RuntimeException(); // just a disaster

    assert(m_xStream.is());
}

OInputCompStream::OInputCompStream( uno::Reference < io::XInputStream > xStream,
                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                    sal_Int32 nStorageType )
: m_pImpl( nullptr )
, m_xMutex( new comphelper::RefCountedMutex )
, m_xStream(std::move( xStream ))
, m_aProperties( aProps )
, m_bDisposed( false )
, m_nStorageType( nStorageType )
{
    assert(m_xStream.is());
}

OInputCompStream::~OInputCompStream()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( !m_bDisposed )
    {
        osl_atomic_increment(&m_refCount);
        dispose();
    }
}

uno::Any SAL_CALL OInputCompStream::queryInterface( const uno::Type& rType )
{
    // common interfaces
    uno::Any aReturn = ::cppu::queryInterface
                (   rType
                    ,   static_cast<io::XInputStream*> ( this )
                    ,   static_cast<io::XStream*> ( this )
                    ,   static_cast<lang::XComponent*> ( this )
                    ,   static_cast<beans::XPropertySet*> ( this )
                    ,   static_cast<embed::XExtendedStorageStream*> ( this ) );

    if ( aReturn.hasValue() )
        return aReturn ;

    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        aReturn = ::cppu::queryInterface
                    (   rType
                        ,   static_cast<embed::XRelationshipAccess*> ( this ) );

        if ( aReturn.hasValue() )
            return aReturn ;
    }

    return OWeakObject::queryInterface( rType );
}

sal_Int32 SAL_CALL OInputCompStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );
    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    return m_xStream->readBytes( aData, nBytesToRead );
}

sal_Int32 SAL_CALL OInputCompStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );
    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    return m_xStream->readSomeBytes( aData, nMaxBytesToRead );

}

void SAL_CALL OInputCompStream::skipBytes( sal_Int32 nBytesToSkip )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );
    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    m_xStream->skipBytes( nBytesToSkip );

}

sal_Int32 SAL_CALL OInputCompStream::available(  )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );
    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    return m_xStream->available();

}

void SAL_CALL OInputCompStream::closeInput(  )
{
    dispose();
}

uno::Reference< io::XInputStream > SAL_CALL OInputCompStream::getInputStream()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );
    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    return this;
}

uno::Reference< io::XOutputStream > SAL_CALL OInputCompStream::getOutputStream()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );
    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    return uno::Reference< io::XOutputStream >();
}

void OInputCompStream::InternalDispose()
{
    // can be called only by OWriteStream_Impl
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );
    if ( m_bDisposed )
        return;

    // the source object is also a kind of locker for the current object
    // since the listeners could dispose the object while being notified
    lang::EventObject aSource( getXWeak() );

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->disposeAndClear( aSource );

    try
    {
        m_xStream->closeInput();
    }
    catch( uno::Exception& )
    {}

    m_pImpl = nullptr;
    m_bDisposed = true;
}

void SAL_CALL OInputCompStream::dispose(  )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );
    if ( m_bDisposed )
        return;

    if ( m_pInterfaceContainer )
    {
        lang::EventObject aSource( getXWeak() );
        m_pInterfaceContainer->disposeAndClear( aSource );
    }

    m_xStream->closeInput();

    if ( m_pImpl )
    {
        m_pImpl->InputStreamDisposed( this );
        m_pImpl = nullptr;
    }

    m_bDisposed = true;
}

void SAL_CALL OInputCompStream::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );
    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer.reset( new ::comphelper::OInterfaceContainerHelper3<css::lang::XEventListener>( m_xMutex->GetMutex() ) );

    m_pInterfaceContainer->addInterface( xListener );
}

void SAL_CALL OInputCompStream::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );
    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( xListener );
}

sal_Bool SAL_CALL OInputCompStream::hasByID(  const OUString& sID )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    try
    {
        getRelationshipByID( sID );
        return true;
    }
    catch( container::NoSuchElementException& )
    {}

    return false;
}

namespace
{

const beans::StringPair* lcl_findPairByName(const uno::Sequence<beans::StringPair>& rSeq, const OUString& rName)
{
    return std::find_if(rSeq.begin(), rSeq.end(),
        [&rName](const beans::StringPair& rPair) { return rPair.First == rName; });
}

}

OUString SAL_CALL OInputCompStream::getTargetByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    const uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    auto pRel = lcl_findPairByName(aSeq, u"Target"_ustr);
    if (pRel != aSeq.end())
        return pRel->Second;

    return OUString();
}

OUString SAL_CALL OInputCompStream::getTypeByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    const uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    auto pRel = lcl_findPairByName(aSeq, u"Type"_ustr);
    if (pRel != aSeq.end())
        return pRel->Second;

    return OUString();
}

uno::Sequence< beans::StringPair > SAL_CALL OInputCompStream::getRelationshipByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    // TODO/LATER: in future the unification of the ID could be checked
    const uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    const beans::StringPair aIDRel(u"Id"_ustr, sID);
    auto pRel = std::find_if(aSeq.begin(), aSeq.end(),
        [&aIDRel](const uno::Sequence<beans::StringPair>& rRel){
            return std::find(rRel.begin(), rRel.end(), aIDRel) != rRel.end(); });
    if (pRel != aSeq.end())
        return *pRel;

    throw container::NoSuchElementException();
}

uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OInputCompStream::getRelationshipsByType(  const OUString& sType  )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    // TODO/LATER: in future the unification of the ID could be checked
    const uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    const beans::StringPair aTypeRel(u"Type"_ustr, sType);
    std::vector< uno::Sequence<beans::StringPair> > aResult;
    aResult.reserve(aSeq.getLength());

    std::copy_if(aSeq.begin(), aSeq.end(), std::back_inserter(aResult),
        [&aTypeRel](const uno::Sequence<beans::StringPair>& rRel) {
            return std::find(rRel.begin(), rRel.end(), aTypeRel) != rRel.end(); });

    return comphelper::containerToSequence(aResult);
}

uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OInputCompStream::getAllRelationships()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    // TODO/LATER: in future the information could be taken directly from m_pImpl when possible
    auto pProp = std::find_if(std::cbegin(m_aProperties), std::cend(m_aProperties),
        [](const beans::PropertyValue& rProp) { return rProp.Name == "RelationsInfo"; });
    if (pProp != std::cend(m_aProperties))
    {
        uno::Sequence< uno::Sequence< beans::StringPair > > aResult;
        if (pProp->Value >>= aResult)
            return aResult;
    }

    throw io::IOException(u"relations info could not be read"_ustr); // the relations info could not be read
}

void SAL_CALL OInputCompStream::insertRelationshipByID(  const OUString& /*sID*/, const uno::Sequence< beans::StringPair >& /*aEntry*/, sal_Bool /*bReplace*/  )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    throw io::IOException(); // TODO: Access denied
}

void SAL_CALL OInputCompStream::removeRelationshipByID(  const OUString& /*sID*/  )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    throw io::IOException(); // TODO: Access denied
}

void SAL_CALL OInputCompStream::insertRelationships(  const uno::Sequence< uno::Sequence< beans::StringPair > >& /*aEntries*/, sal_Bool /*bReplace*/  )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    throw io::IOException(); // TODO: Access denied
}

void SAL_CALL OInputCompStream::clearRelationships()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    throw io::IOException(); // TODO: Access denied
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL OInputCompStream::getPropertySetInfo()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    //TODO:
    return uno::Reference< beans::XPropertySetInfo >();
}

void SAL_CALL OInputCompStream::setPropertyValue( const OUString& aPropertyName, const uno::Any& /*aValue*/ )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    // all the provided properties are accessible
    if (std::any_of(std::cbegin(m_aProperties), std::cend(m_aProperties),
            [&aPropertyName](const beans::PropertyValue& rProp) { return rProp.Name == aPropertyName; }))
        throw beans::PropertyVetoException(); // TODO

    throw beans::UnknownPropertyException(aPropertyName); // TODO
}

uno::Any SAL_CALL OInputCompStream::getPropertyValue( const OUString& aProp )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    OUString aPropertyName;
    if ( aProp == "IsEncrypted" )
        aPropertyName = "Encrypted";
    else
        aPropertyName = aProp;

    if ( aPropertyName == "RelationsInfo" )
        throw beans::UnknownPropertyException(aPropertyName); // TODO

    // all the provided properties are accessible
    auto pProp = std::find_if(std::cbegin(m_aProperties), std::cend(m_aProperties),
        [&aPropertyName](const beans::PropertyValue& rProp) { return rProp.Name == aPropertyName; });
    if (pProp != std::cend(m_aProperties))
        return pProp->Value;

    throw beans::UnknownPropertyException(aPropertyName); // TODO
}

void SAL_CALL OInputCompStream::addPropertyChangeListener(
    const OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    //TODO:
}

void SAL_CALL OInputCompStream::removePropertyChangeListener(
    const OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    //TODO:
}

void SAL_CALL OInputCompStream::addVetoableChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    //TODO:
}

void SAL_CALL OInputCompStream::removeVetoableChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bDisposed )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    //TODO:
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
