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
#ifndef DBAUI_COLUMNMODEL_HXX
#define DBAUI_COLUMNMODEL_HXX

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/compbase4.hxx>
#include "apitools.hxx"

namespace dbaui
{

// OColumnControlModel
typedef ::cppu::WeakAggComponentImplHelper4 <   ::com::sun::star::awt::XControlModel
                                        ,   ::com::sun::star::lang::XServiceInfo
                                        ,   ::com::sun::star::util::XCloneable
                                        ,   ::com::sun::star::io::XPersistObject
                                        >   OColumnControlModel_BASE;

class OColumnControlModel;

class OColumnControlModel :  public ::comphelper::OMutexAndBroadcastHelper
                            ,public ::comphelper::OPropertyContainer
                            ,public ::comphelper::OPropertyArrayUsageHelper< OColumnControlModel >
                            ,public OColumnControlModel_BASE
{

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xORB;
// [properties]
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>          m_xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xColumn;
    OUString                 m_sDefaultControl;
    ::com::sun::star::uno::Any      m_aTabStop;
    sal_Bool                        m_bEnable;
    sal_Int16                       m_nBorder;
    sal_Int32                       m_nWidth;
// [properties]

    inline ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> getORB() const { return m_xORB; }
    void registerProperties();
protected:

    virtual ~OColumnControlModel();
    OColumnControlModel(const OColumnControlModel* _pSource
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
public:
    OColumnControlModel(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

// UNO Anbindung
    DECLARE_XINTERFACE( )

// ::com::sun::star::lang::XServiceInfo
    DECLARE_SERVICE_INFO_STATIC();

// XTypeProvider
    DECLARE_TYPEPROVIDER( );

// com::sun::star::uno::XAggregation
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);

//  ::com::sun::star::io::XPersistObject
    virtual OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// OPropertyArrayUsageHelper
    DECLARE_PROPERTYCONTAINER_DEFAULTS( );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException);
};

}   // namespace dbaui
#endif // DBAUI_COLUMNMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
