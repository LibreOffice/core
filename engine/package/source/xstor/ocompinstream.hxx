/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#ifndef INCLUDED_PACKAGE_SOURCE_XSTOR_OCOMPINSTREAM_HXX
#define INCLUDED_PACKAGE_SOURCE_XSTOR_OCOMPINSTREAM_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/XExtendedStorageStream.hpp>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <comphelper/refcountedmutex.hxx>
#include <comphelper/bytereader.hxx>
#include <rtl/ref.hxx>
#include <memory>


struct OWriteStream_Impl;

class OInputCompStream : public cppu::WeakImplHelper < css::io::XInputStream
                                                        ,css::embed::XExtendedStorageStream
                                                        ,css::embed::XRelationshipAccess
                                                        ,css::beans::XPropertySet >,
                         public comphelper::ByteReader
{
protected:
    OWriteStream_Impl* m_pImpl;
    rtl::Reference<comphelper::RefCountedMutex> m_xMutex;
    cpo::uno::Reference < css::io::XInputStream > m_xStream;
    comphelper::ByteReader* m_pByteReader;
    std::unique_ptr<::comphelper::OInterfaceContainerHelper3<css::lang::XEventListener>> m_pInterfaceContainer;
    cpo::uno::Sequence < css::beans::PropertyValue > m_aProperties;
    bool m_bDisposed;
    sal_Int32 m_nStorageType;

public:
    OInputCompStream( OWriteStream_Impl& pImpl,
                      cpo::uno::Reference< css::io::XInputStream > xStream,
                      const cpo::uno::Sequence< css::beans::PropertyValue >& aProps,
                      sal_Int32 nStorageType );

    OInputCompStream( cpo::uno::Reference< css::io::XInputStream > xStream,
                      const cpo::uno::Sequence< css::beans::PropertyValue >& aProps,
                      sal_Int32 nStorageType );

    virtual ~OInputCompStream() override;

    void InternalDispose();

    // XInterface
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type& rType ) override;

    // XInputStream
    virtual sal_Int32 readBytes( cpo::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) override;
    virtual sal_Int32 readSomeBytes( cpo::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) override;
    virtual void skipBytes( sal_Int32 nBytesToSkip ) override;
    virtual sal_Int32 available(  ) override;
    virtual void closeInput(  ) override;

    //XStream
    virtual cpo::uno::Reference< css::io::XInputStream > getInputStream(  ) override;
    virtual cpo::uno::Reference< css::io::XOutputStream > getOutputStream(  ) override;

    //XComponent
    virtual void dispose(  ) override;
    virtual void addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;

    //XRelationshipAccess
    virtual bool hasByID( const OUString& sID ) override;
    virtual OUString getTargetByID( const OUString& sID ) override;
    virtual OUString getTypeByID( const OUString& sID ) override;
    virtual cpo::uno::Sequence< css::beans::StringPair > getRelationshipByID( const OUString& sID ) override;
    virtual cpo::uno::Sequence< cpo::uno::Sequence< css::beans::StringPair > > getRelationshipsByType( const OUString& sType ) override;
    virtual cpo::uno::Sequence< cpo::uno::Sequence< css::beans::StringPair > > getAllRelationships(  ) override;
    virtual void insertRelationshipByID( const OUString& sID, const cpo::uno::Sequence< css::beans::StringPair >& aEntry, bool bReplace ) override;
    virtual void removeRelationshipByID( const OUString& sID ) override;
    virtual void insertRelationships( const cpo::uno::Sequence< cpo::uno::Sequence< css::beans::StringPair > >& aEntries, bool bReplace ) override;
    virtual void clearRelationships(  ) override;

    //XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() override;
    virtual void setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    virtual void addPropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void removePropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void addVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void removeVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // comphelper::ByteReader
    virtual sal_Int32 readSomeBytes(sal_Int8* aData, sal_Int32 nBytesToRead) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
