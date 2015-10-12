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
#include <cppuhelper/interfacecontainer.h>

#include "mutexholder.hxx"

struct OWriteStream_Impl;

class OInputCompStream : public cppu::WeakImplHelper < ::com::sun::star::io::XInputStream
                                                        ,::com::sun::star::embed::XExtendedStorageStream
                                                        ,::com::sun::star::embed::XRelationshipAccess
                                                        ,::com::sun::star::beans::XPropertySet >
{
protected:
    OWriteStream_Impl* m_pImpl;

    rtl::Reference<SotMutexHolder> m_rMutexRef;

    ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > m_xStream;

    ::cppu::OInterfaceContainerHelper* m_pInterfaceContainer;

    ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > m_aProperties;

    bool m_bDisposed;

    sal_Int32 m_nStorageType;

public:
    OInputCompStream( OWriteStream_Impl& pImpl,
                      ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xStream,
                      const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps,
                      sal_Int32 nStorageType );

    OInputCompStream( ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xStream,
                      const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps,
                      sal_Int32 nStorageType );

    virtual ~OInputCompStream();

    void InternalDispose();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL available(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL closeInput(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XStream
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //XRelationshipAccess
    virtual sal_Bool SAL_CALL hasByID( const OUString& sID ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTargetByID( const OUString& sID ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTypeByID( const OUString& sID ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > SAL_CALL getRelationshipByID( const OUString& sID ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > SAL_CALL getRelationshipsByType( const OUString& sType ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > SAL_CALL getAllRelationships(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertRelationshipByID( const OUString& sID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aEntry, sal_Bool bReplace ) throw (::com::sun::star::container::ElementExistException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeRelationshipByID( const OUString& sID ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertRelationships( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > >& aEntries, sal_Bool bReplace ) throw (::com::sun::star::container::ElementExistException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearRelationships(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
