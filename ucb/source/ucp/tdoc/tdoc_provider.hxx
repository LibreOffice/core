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

#ifndef INCLUDED_TDOC_PROVIDER_HXX
#define INCLUDED_TDOC_PROVIDER_HXX

#include "rtl/ref.hxx"
#include <com/sun/star/frame/XTransientDocumentsDocumentContentFactory.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include "ucbhelper/providerhelper.hxx"
#include "tdoc_uri.hxx" // for TDOC_URL_SCHEME
#include "tdoc_docmgr.hxx"
#include "tdoc_storage.hxx" // for StorageAccessMode

namespace com { namespace sun { namespace star { namespace embed {
    class XStorage;
} } } }

namespace com { namespace sun { namespace star { namespace frame {
    class XModel;
} } } }

namespace tdoc_ucp {

//=========================================================================

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

//=========================================================================

class StorageElementFactory;

class ContentProvider :
    public ::ucbhelper::ContentProviderImplHelper,
    public com::sun::star::frame::XTransientDocumentsDocumentContentFactory,
    public OfficeDocumentsEventListener
{
public:
    ContentProvider( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XMultiServiceFactory >& rXSMgr );
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
    queryStorage( const rtl::OUString & rUri, StorageAccessMode eMode ) const;

    com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
    queryStorageClone( const rtl::OUString & rUri ) const;

    com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
    queryInputStream( const rtl::OUString & rUri,
                      const rtl::OUString & rPassword ) const
        throw ( com::sun::star::packages::WrongPasswordException );

    com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >
    queryOutputStream( const rtl::OUString & rUri,
                       const rtl::OUString & rPassword,
                       bool bTruncate ) const
        throw ( com::sun::star::packages::WrongPasswordException );

    com::sun::star::uno::Reference< com::sun::star::io::XStream >
    queryStream( const rtl::OUString & rUri,
                 const rtl::OUString & rPassword,
                 bool bTruncate ) const
        throw ( com::sun::star::packages::WrongPasswordException );

    bool queryNamesOfChildren(
        const rtl::OUString & rUri,
        com::sun::star::uno::Sequence< rtl::OUString > & rNames ) const;

    // storage properties
    rtl::OUString queryStorageTitle( const rtl::OUString & rUri ) const;

    com::sun::star::uno::Reference< com::sun::star::frame::XModel >
    queryDocumentModel( const rtl::OUString & rUri ) const;

    // interface OfficeDocumentsEventListener
    virtual void notifyDocumentOpened( const rtl::OUString & rDocId );
    virtual void notifyDocumentClosed( const rtl::OUString & rDocId );

private:
    rtl::Reference< OfficeDocumentsManager > m_xDocsMgr;
    rtl::Reference< StorageElementFactory >  m_xStgElemFac;
};

} // namespace tdoc_ucp

#endif /* !INCLUDED_TDOC_PROVIDER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
