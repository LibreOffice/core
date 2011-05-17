/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

//.........................................................................
namespace dbaui
{
//.........................................................................

//==================================================================
// OColumnControlModel
//==================================================================
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
    ::rtl::OUString                 m_sDefaultControl;
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
    virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);


// OPropertyArrayUsageHelper
    DECLARE_PROPERTYCONTAINER_DEFAULTS( );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................
#endif // DBAUI_COLUMNMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
