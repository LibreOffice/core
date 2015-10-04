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

#ifndef INCLUDED_EXAMPLES_COMPLEXTOOLBARCONTROLS_MYPROTOCOLHANDLER_H
#define INCLUDED_EXAMPLES_COMPLEXTOOLBARCONTROLS_MYPROTOCOLHANDLER_H

#include <com/sun/star/awt/XToolkit2.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>
#include <cppuhelper/implbase.hxx>

#define MYPROTOCOLHANDLER_IMPLEMENTATIONNAME   "vnd.demo.Impl.ProtocolHandler"
#define MYPROTOCOLHANDLER_SERVICENAME          "vnd.demo.ProtocolHandler"

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace frame
            {
                class XModel;
                class XFrame;
            }
            namespace uno { class XComponentContext; }
        }
    }
}


class MyProtocolHandler : public cppu::WeakImplHelper
<
    com::sun::star::frame::XDispatchProvider,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;

public:
    MyProtocolHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext)
        : mxContext( rxContext ) {}

    // XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
            SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL,
                const ::rtl::OUString& sTargetFrameName, sal_Int32 nSearchFlags )
                throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence < ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > >
        SAL_CALL queryDispatches(
            const ::com::sun::star::uno::Sequence < ::com::sun::star::frame::DispatchDescriptor >& seqDescriptor )
            throw( ::com::sun::star::uno::RuntimeException );

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException);
};

::rtl::OUString MyProtocolHandler_getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL MyProtocolHandler_supportsService( const ::rtl::OUString& ServiceName )
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL MyProtocolHandler_getSupportedServiceNames(  )
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL MyProtocolHandler_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & rContext)
    throw ( ::com::sun::star::uno::Exception );

class BaseDispatch : public cppu::WeakImplHelper
<
    ::com::sun::star::frame::XDispatch,
    ::com::sun::star::frame::XControlNotificationListener
>
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit2 > mxToolkit;
    ::rtl::OUString msDocService;
    ::rtl::OUString maComboBoxText;
    sal_Bool        mbButtonEnabled;

public:
    BaseDispatch( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame, const ::rtl::OUString& rServiceName );

    virtual ~BaseDispatch();

    void ShowMessageBox( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& aTitle, const ::rtl::OUString& aMsgText );
    void SendCommand( const com::sun::star::util::URL& aURL, const ::rtl::OUString& rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rArgs, sal_Bool bEnabled );
    void SendCommandTo( const com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener >& xControl, const com::sun::star::util::URL& aURL, const ::rtl::OUString& rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rArgs, sal_Bool bEnabled );

    // XDispatch
    virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& aURL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArgs )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl,
        const ::com::sun::star::util::URL& aURL ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl,
        const ::com::sun::star::util::URL& aURL ) throw (::com::sun::star::uno::RuntimeException);

    // XControlNotificationListener
    virtual void SAL_CALL controlEvent( const ::com::sun::star::frame::ControlEvent& Event )
        throw (::com::sun::star::uno::RuntimeException);
};

class WriterDispatch : public BaseDispatch
{
public:
    WriterDispatch( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame )
        : BaseDispatch( rxContext, xFrame, rtl::OUString( "com.sun.star.text.TextDocument" ) )
    {}
};

class CalcDispatch : public BaseDispatch
{
public:
    CalcDispatch( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame )
        : BaseDispatch( rxContext, xFrame, rtl::OUString( "com.sun.star.sheet.SpreadSheetDocument" ) )
    {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
