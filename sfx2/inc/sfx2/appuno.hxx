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
#ifndef _SFX_APPUNO_HXX
#define _SFX_APPUNO_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XSynchronousDispatch.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase4.hxx>

#include <tools/errcode.hxx>
#include <sfx2/sfxuno.hxx>

class SfxObjectShell;
class SfxMacroLoader  :     public ::com::sun::star::frame::XDispatchProvider,
                            public ::com::sun::star::frame::XNotifyingDispatch,
                            public ::com::sun::star::frame::XSynchronousDispatch,
                            public ::com::sun::star::lang::XTypeProvider,
                            public ::com::sun::star::lang::XServiceInfo,
                            public ::com::sun::star::lang::XInitialization,
                            public ::cppu::OWeakObject
{
    ::com::sun::star::uno::WeakReference < ::com::sun::star::frame::XFrame > m_xFrame;

    SfxObjectShell*             GetObjectShell_Impl();

public:
    // XInterface, XTypeProvider, XServiceInfo
    SFX_DECL_XINTERFACE_XTYPEPROVIDER_XSERVICEINFO

    SfxMacroLoader( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& )
    {}

    static ErrCode loadMacro( const ::rtl::OUString& aURL, ::com::sun::star::uno::Any& rRetval, SfxObjectShell* pDoc=NULL ) throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatch > SAL_CALL
                    queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& sTargetFrameName,
                    FrameSearchFlags eSearchFlags ) throw( ::com::sun::star::uno::RuntimeException ) ;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatch > > SAL_CALL
                    queryDispatches( const ::com::sun::star::uno::Sequence < ::com::sun::star::frame::DispatchDescriptor >& seqDescriptor )
                        throw( ::com::sun::star::uno::RuntimeException ) ;
    virtual void SAL_CALL dispatchWithNotification( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArgs, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArgs ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL dispatchWithReturnValue( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArgs ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
};

class SfxAppDispatchProvider : public ::cppu::WeakImplHelper4< ::com::sun::star::frame::XDispatchProvider,
                                                               ::com::sun::star::lang::XServiceInfo,
                                                               ::com::sun::star::lang::XInitialization,
                                                               ::com::sun::star::frame::XDispatchInformationProvider >
{
    ::com::sun::star::uno::WeakReference < ::com::sun::star::frame::XFrame > m_xFrame;
public:
                    SfxAppDispatchProvider( const com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& )
                    {}

    SFX_DECL_XSERVICEINFO
    virtual ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatch > SAL_CALL
                    queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& sTargetFrameName,
                    FrameSearchFlags eSearchFlags ) throw( ::com::sun::star::uno::RuntimeException ) ;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatch > > SAL_CALL
                    queryDispatches( const ::com::sun::star::uno::Sequence < ::com::sun::star::frame::DispatchDescriptor >& seqDescriptor )
                        throw( ::com::sun::star::uno::RuntimeException ) ;
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedCommandGroups() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( sal_Int16 ) throw (::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
