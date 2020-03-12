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

#ifndef INCLUDED_PACKAGE_SOURCE_XSTOR_OCOMPINSTREAM_HXX
#define INCLUDED_PACKAGE_SOURCE_XSTOR_OCOMPINSTREAM_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/XExtendedStorageStream.hpp>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <comphelper/refcountedmutex.hxx>
#include <rtl/ref.hxx>
#include <memory>


struct OWriteStream_Impl;

class OInputCompStream : public cppu::WeakImplHelper < css::io::XInputStream
                                                        ,css::embed::XExtendedStorageStream
                                                        ,css::embed::XRelationshipAccess
                                                        ,css::beans::XPropertySet >
{
protected:
    OWriteStream_Impl* m_pImpl;
    rtl::Reference<comphelper::RefCountedMutex> m_xMutex;
    css::uno::Reference < css::io::XInputStream > m_xStream;
    std::unique_ptr<::comphelper::OInterfaceContainerHelper2> m_pInterfaceContainer;
    css::uno::Sequence < css::beans::PropertyValue > m_aProperties;
    bool m_bDisposed;
    sal_Int32 m_nStorageType;

public:
    OInputCompStream( OWriteStream_Impl& pImpl,
                      css::uno::Reference< css::io::XInputStream > const & xStream,
                      const css::uno::Sequence< css::beans::PropertyValue >& aProps,
                      sal_Int32 nStorageType );

    OInputCompStream( css::uno::Reference< css::io::XInputStream > const & xStream,
                      const css::uno::Sequence< css::beans::PropertyValue >& aProps,
                      sal_Int32 nStorageType );

    virtual ~OInputCompStream() override;

    void InternalDispose();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType ) override;

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override;
    virtual sal_Int32 SAL_CALL available(  ) override;
    virtual void SAL_CALL closeInput(  ) override;

    //XStream
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getInputStream(  ) override;
    virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL getOutputStream(  ) override;

    //XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    //XRelationshipAccess
    virtual sal_Bool SAL_CALL hasByID( const OUString& sID ) override;
    virtual OUString SAL_CALL getTargetByID( const OUString& sID ) override;
    virtual OUString SAL_CALL getTypeByID( const OUString& sID ) override;
    virtual css::uno::Sequence< css::beans::StringPair > SAL_CALL getRelationshipByID( const OUString& sID ) override;
    virtual css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > SAL_CALL getRelationshipsByType( const OUString& sType ) override;
    virtual css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > SAL_CALL getAllRelationships(  ) override;
    virtual void SAL_CALL insertRelationshipByID( const OUString& sID, const css::uno::Sequence< css::beans::StringPair >& aEntry, sal_Bool bReplace ) override;
    virtual void SAL_CALL removeRelationshipByID( const OUString& sID ) override;
    virtual void SAL_CALL insertRelationships( const css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > >& aEntries, sal_Bool bReplace ) override;
    virtual void SAL_CALL clearRelationships(  ) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
