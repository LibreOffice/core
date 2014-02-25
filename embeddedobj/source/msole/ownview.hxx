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

#ifndef __OWNVIEW_HXX_
#define __OWNVIEW_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase2.hxx>

#include <osl/mutex.hxx>

class OwnView_Impl : public ::cppu::WeakImplHelper2 < ::com::sun::star::util::XCloseListener,
                                                      ::com::sun::star::document::XEventListener >
{
    ::osl::Mutex m_aMutex;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > m_xModel;

    OUString m_aTempFileURL;
    OUString m_aNativeTempURL;

    OUString m_aFilterName;

    sal_Bool m_bBusy;

    sal_Bool m_bUseNative;

private:
    sal_Bool CreateModelFromURL( const OUString& aFileURL );

    sal_Bool CreateModel( sal_Bool bUseNative );

    sal_Bool ReadContentsAndGenerateTempFile( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xStream, sal_Bool bParseHeader );

    void CreateNative();

public:
    static OUString GetFilterNameFromExtentionAndInStream(
                                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                                const OUString& aNameWithExtention,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInputStream );

    OwnView_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xStream );
    virtual ~OwnView_Impl();

    sal_Bool Open();

    void Close();

    virtual void SAL_CALL notifyEvent( const ::com::sun::star::document::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual void SAL_CALL queryClosing( const ::com::sun::star::lang::EventObject& Source, sal_Bool GetsOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL notifyClosing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
