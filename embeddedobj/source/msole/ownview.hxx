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

#ifndef INCLUDED_EMBEDDEDOBJ_SOURCE_MSOLE_OWNVIEW_HXX
#define INCLUDED_EMBEDDEDOBJ_SOURCE_MSOLE_OWNVIEW_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase.hxx>

#include <osl/mutex.hxx>

class OwnView_Impl : public ::cppu::WeakImplHelper < css::util::XCloseListener,
                                                      css::document::XEventListener >
{
    ::osl::Mutex m_aMutex;

    css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;
    css::uno::Reference< css::frame::XModel > m_xModel;

    OUString m_aTempFileURL;
    OUString m_aNativeTempURL;

    OUString m_aFilterName;

    bool m_bBusy;

    bool m_bUseNative;

private:
    bool CreateModelFromURL( const OUString& aFileURL );

    bool CreateModel( bool bUseNative );

    bool ReadContentsAndGenerateTempFile( const css::uno::Reference< css::io::XInputStream >& xStream, bool bParseHeader );

    void CreateNative();

public:
    static OUString GetFilterNameFromExtentionAndInStream(
                                const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory,
                                const OUString& aNameWithExtention,
                                const css::uno::Reference< css::io::XInputStream >& xInputStream );

    OwnView_Impl( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory,
                  const css::uno::Reference< css::io::XInputStream >& xStream );
    virtual ~OwnView_Impl();

    bool Open();

    void Close();

    virtual void SAL_CALL notifyEvent( const css::document::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL queryClosing( const css::lang::EventObject& Source, sal_Bool GetsOwnership ) throw (css::util::CloseVetoException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL notifyClosing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
