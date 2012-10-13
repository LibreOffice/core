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

#ifndef ODMA_PROVIDER_HXX
#define ODMA_PROVIDER_HXX

#include <ucbhelper/providerhelper.hxx>
#include "odma_lib.hxx"

#include "rtl/ref.hxx"

#include <map>
#include "odma_contentprops.hxx"

namespace odma {

//=========================================================================

// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.
#define ODMA_CONTENT_PROVIDER_SERVICE_NAME \
                "com.sun.star.ucb.ODMAContentProvider"
//  #define ODMA_CONTENT_PROVIDER_SERVICE_NAME_LENGTH   34

// URL scheme. This is the scheme the provider will be able to create
// contents for. The UCB will select the provider ( i.e. in order to create
// contents ) according to this scheme.
#define ODMA_URL_ODMAID         "::ODMA"
#define ODMA_URL_SCHEME         "vnd.sun.star.odma"
#define ODMA_URL_SCHEME_SHORT   "odma"
#define ODMA_URL_SHORT          ":"
#define ODMA_URL_SHORT_LGTH     5
#define ODMA_URL_LGTH           18
#define ODMA_URL_ODMAID_LGTH    6

// UCB Content Type.
#define ODMA_CONTENT_TYPE       "application/" ODMA_URL_SCHEME "-content"
#define ODMA_ODMA_REGNAME       "sodma"
#define ODM_NAME_MAX            64      // Max length of a name document including
                                        // the terminating NULL character.

//=========================================================================
class ContentProperties;
class ContentProvider : public ::ucbhelper::ContentProviderImplHelper
{
    typedef ::std::map< ::rtl::OString, ::rtl::Reference<ContentProperties> > ContentsMap;
    ContentsMap      m_aContents;  // contains all ContentProperties
    static ODMHANDLE m_aOdmHandle; // the one and only ODMA handle to our DMS

    /** fillDocumentProperties fills the given _rProp with ODMA properties
        @param  _rProp  the ContentProperties
    */
    void fillDocumentProperties(const ::rtl::Reference<ContentProperties>& _rProp);

    /**
    */
    ::rtl::Reference<ContentProperties> getContentProperty(const ::rtl::OUString& _sName,
                                                       const ContentPropertiesMemberFunctor& _aFunctor) const;
public:
    ContentProvider( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr );
    virtual ~ContentProvider();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XContentProvider
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( ::com::sun::star::ucb::IllegalIdentifierException,
               ::com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Additional interfaces
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////
    static ODMHANDLE getHandle();

    /** append add an entry to the internal map
        @param  _rProp  the content properties
    */
    void append(const ::rtl::Reference<ContentProperties>& _rProp);

    /** closeDocument closes the document
        @param  _sDocumentId    the id of the document
    */
    void closeDocument(const ::rtl::OString& _sDocumentId);

    /** saveDocument saves the document in DMS
        @param  _sDocumentId    the id of the document
    */
    void saveDocument(const ::rtl::OString& _sDocumentId);

    /** queryContentProperty query in the DMS for a content which document name is equal to _sDocumentName
        @param  _sDocumentName  the document to query for

        @return the content properties for this content or an empty refernce
    */
    ::rtl::Reference<ContentProperties> queryContentProperty(const ::rtl::OUString& _sDocumentName);

    /** getContentPropertyWithTitle returns the ContentProperties for the first content with that title
        @param  _sTitle the title of the document

        @return the content properties
    */
    ::rtl::Reference<ContentProperties> getContentPropertyWithTitle(const ::rtl::OUString& _sTitle) const;

    /** getContentPropertyWithDocumentId returns the ContentProperties for the first content with that title
        @param  _sTitle the title of the document

        @return the content properties
    */
    ::rtl::Reference<ContentProperties> getContentPropertyWithDocumentId(const ::rtl::OUString& _sDocumentId) const;

    /** getContentPropertyWithSavedAsName returns the ContentProperties for the first content with that SavedAsName
        @param  _sSaveAsName    the SavedAsName of the document

        @return the content properties
    */
    ::rtl::Reference<ContentProperties> getContentPropertyWithSavedAsName(const ::rtl::OUString& _sSaveAsName) const;

    /** openDoc returns the URL for the temporary file for the specific Content and opens it
        @param  _rProp  used for check if already open, the member m_sFileURL will be set if is wan't opened yet

        @return the URL of the temporary file
    */
    static ::rtl::OUString openDoc(const ::rtl::Reference<ContentProperties>& _rProp) throw (::com::sun::star::uno::Exception);

    /** convertURL converts a normal URL into an ODMA understandable name
        @param  _sCanonicURL    the URL from ContentIndentifier

        @return the ODMA name
    */
    static ::rtl::OUString convertURL(const ::rtl::OUString& _sCanonicURL);

    /** deleteDocument deletes the document inside the DMS and remove the content properties from inside list
        @param  _rProp  the ContentProperties

        @return true when successful
    */
    sal_Bool deleteDocument(const ::rtl::Reference<ContentProperties>& _rProp);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
