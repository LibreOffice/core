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

#ifndef INCLUDED_TDOC_PROVIDER_HXX
#define INCLUDED_TDOC_PROVIDER_HXX

#include "rtl/ref.hxx"
#include <com/sun/star/frame/XTransientDocumentsDocumentContentFactory.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include "ucbhelper/providerhelper.hxx"
#include "tdoc_uri.hxx"
#include "tdoc_docmgr.hxx"
#include "tdoc_storage.hxx"

namespace com { namespace sun { namespace star { namespace embed {
    class XStorage;
} } } }

namespace com { namespace sun { namespace star { namespace frame {
    class XModel;
} } } }

namespace tdoc_ucp {



#define TDOC_CONTENT_PROVIDER_SERVICE_NAME \
                "com.sun.star.ucb.TransientDocumentsContentProvider"
#define TDOC_CONTENT_PROVIDER_SERVICE_NAME_LENGTH   50

#define TDOC_ROOT_CONTENT_TYPE \
                "application/" TDOC_URL_SCHEME "-root"
#define TDOC_DOCUMENT_CONTENT_TYPE \
                "application/" TDOC_URL_SCHEME "-document"
#define TDOC_FOLDER_CONTENT_TYPE \
                "application/" TDOC_URL_SCHEME "-folder"
#define TDOC_STREAM_CONTENT_TYPE \
                "application/" TDOC_URL_SCHEME "-stream"



class StorageElementFactory;

class ContentProvider :
    public ::ucbhelper::ContentProviderImplHelper,
    public com::sun::star::frame::XTransientDocumentsDocumentContentFactory,
    public OfficeDocumentsEventListener
{
public:
    ContentProvider( const com::sun::star::uno::Reference<
                        com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~ContentProvider();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XContentProvider
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( com::sun::star::ucb::IllegalIdentifierException,
               com::sun::star::uno::RuntimeException );

    // XTransientDocumentsDocumentContentFactory
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContent > SAL_CALL
    createDocumentContent( const ::com::sun::star::uno::Reference<
                                com::sun::star::frame::XModel >& Model )
        throw ( com::sun::star::lang::IllegalArgumentException,
                com::sun::star::uno::RuntimeException );

    // Non-UNO interfaces
    com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
    queryStorage( const OUString & rUri, StorageAccessMode eMode ) const;

    com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
    queryStorageClone( const OUString & rUri ) const;

    com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    queryInputStream( const OUString & rUri,
                      const OUString & rPassword ) const
        throw ( com::sun::star::packages::WrongPasswordException );

    com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >
    queryOutputStream( const OUString & rUri,
                       const OUString & rPassword,
                       bool bTruncate ) const
        throw ( com::sun::star::packages::WrongPasswordException );

    com::sun::star::uno::Reference< com::sun::star::io::XStream >
    queryStream( const OUString & rUri,
                 const OUString & rPassword,
                 bool bTruncate ) const
        throw ( com::sun::star::packages::WrongPasswordException );

    bool queryNamesOfChildren(
        const OUString & rUri,
        com::sun::star::uno::Sequence< OUString > & rNames ) const;

    // storage properties
    OUString queryStorageTitle( const OUString & rUri ) const;

    com::sun::star::uno::Reference< com::sun::star::frame::XModel >
    queryDocumentModel( const OUString & rUri ) const;

    // interface OfficeDocumentsEventListener
    virtual void notifyDocumentOpened( const OUString & rDocId );
    virtual void notifyDocumentClosed( const OUString & rDocId );

private:
    rtl::Reference< OfficeDocumentsManager > m_xDocsMgr;
    rtl::Reference< StorageElementFactory >  m_xStgElemFac;
};

} // namespace tdoc_ucp

#endif /* !INCLUDED_TDOC_PROVIDER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
