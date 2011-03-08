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
#ifndef _SFX_APPUNO_HXX
#define _SFX_APPUNO_HXX

//____________________________________________________________________________________________________________________________________
//  generated header
//____________________________________________________________________________________________________________________________________

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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//____________________________________________________________________________________________________________________________________
//  fix uno header
//____________________________________________________________________________________________________________________________________
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase4.hxx>

//____________________________________________________________________________________________________________________________________
//  something else header
//____________________________________________________________________________________________________________________________________
#include <tools/errcode.hxx>
#include <svl/svarray.hxx>
#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>
#include <sfx2/sfxuno.hxx>

//____________________________________________________________________________________________________________________________________
//  forwards
//____________________________________________________________________________________________________________________________________

//____________________________________________________________________________________________________________________________________
//  declarations
//____________________________________________________________________________________________________________________________________
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
