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

#pragma once

#include <sal/config.h>

#include <string_view>

#include <rtl/ref.hxx>
#include <com/sun/star/frame/XTransientDocumentsDocumentContentFactory.hpp>
#include <com/sun/star/frame/XTransientDocumentsDocumentContentIdentifierFactory.hpp>
#include <ucbhelper/providerhelper.hxx>
#include "tdoc_uri.hxx"
#include "tdoc_docmgr.hxx"
#include "tdoc_storage.hxx"

namespace com::sun::star::embed {
    class XStorage;
}

namespace com::sun::star::frame {
    class XModel;
}

namespace com::sun::star::util {
    struct DateTime;
}

namespace tdoc_ucp {


inline constexpr OUString TDOC_ROOT_CONTENT_TYPE =
                u"application/" TDOC_URL_SCHEME "-root"_ustr;
inline constexpr OUString TDOC_DOCUMENT_CONTENT_TYPE =
                u"application/" TDOC_URL_SCHEME "-document"_ustr;
inline constexpr OUString TDOC_FOLDER_CONTENT_TYPE =
                u"application/" TDOC_URL_SCHEME "-folder"_ustr;
inline constexpr OUString TDOC_STREAM_CONTENT_TYPE =
                u"application/" TDOC_URL_SCHEME "-stream"_ustr;


typedef cppu::ImplInheritanceHelper<
    ::ucbhelper::ContentProviderImplHelper,
    css::frame::XTransientDocumentsDocumentContentIdentifierFactory,
    css::frame::XTransientDocumentsDocumentContentFactory > ContentProvider_Base;
class ContentProvider : public ContentProvider_Base
{
public:
    explicit ContentProvider( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~ContentProvider() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XContentProvider
    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
    queryContent( const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier ) override;

    // XTransientDocumentsDocumentContentIdentifierFactory
    virtual css::uno::Reference<css::ucb::XContentIdentifier> SAL_CALL
    createDocumentContentIdentifier(
        css::uno::Reference<css::frame::XModel> const& xModel) override;

    // XTransientDocumentsDocumentContentFactory
    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
    createDocumentContent( const css::uno::Reference<
                                css::frame::XModel >& Model ) override;

    // Non-UNO interfaces
    css::uno::Reference< css::embed::XStorage >
    queryStorage( const OUString & rUri, StorageAccessMode eMode ) const;

    css::uno::Reference< css::embed::XStorage >
    queryStorageClone( const OUString & rUri ) const;

    /// @throws css::packages::WrongPasswordException
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::io::XInputStream >
    queryInputStream( const OUString & rUri,
                      const OUString & rPassword ) const;

    /// @throws css::packages::WrongPasswordException
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::io::XOutputStream >
    queryOutputStream( const OUString & rUri,
                       const OUString & rPassword,
                       bool bTruncate ) const;

    /// @throws css::packages::WrongPasswordException
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::io::XStream >
    queryStream( const OUString & rUri,
                 const OUString & rPassword,
                 bool bTruncate ) const;

    bool queryNamesOfChildren(
        const OUString & rUri,
        css::uno::Sequence< OUString > & rNames ) const;

    // storage properties
    OUString queryStorageTitle( const OUString & rUri ) const;

    css::uno::Reference< css::frame::XModel >
    queryDocumentModel( const OUString & rUri ) const;

    css::util::DateTime queryStreamDateModified(OUString const & uri) const;

    // interface OfficeDocumentsEventListener
    void notifyDocumentOpened( std::u16string_view rDocId );
    void notifyDocumentClosed( std::u16string_view rDocId );

private:
    rtl::Reference< OfficeDocumentsManager > m_xDocsMgr;
    rtl::Reference< StorageElementFactory >  m_xStgElemFac;
};

} // namespace tdoc_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
