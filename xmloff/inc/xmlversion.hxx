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
#ifndef _XMLOFF_XMLVERSION_HXX
#define _XMLOFF_XMLVERSION_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/document/XDocumentRevisionListPersistence.hpp>
#include <com/sun/star/util/RevisionTag.hpp>
#include <com/sun/star/embed/XStorage.hpp>

#include <cppuhelper/implbase1.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>

class XMLVersionListExport : public SvXMLExport
{
private:
    const com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >& maVersions;
public:
    XMLVersionListExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
        const com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >& rVersions,
        const OUString &rFileName,
        com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > &rHandler );
    virtual     ~XMLVersionListExport() {}

    sal_uInt32  exportDoc( enum ::xmloff::token::XMLTokenEnum eClass );
    void        _ExportAutoStyles() {}
    void        _ExportMasterStyles () {}
    void        _ExportContent() {}
};

class XMLVersionListImport : public SvXMLImport
{
private:
    com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >& maVersions;

protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                    const OUString& rLocalName,
                    const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

public:

    XMLVersionListImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
        com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >& rVersions );
    ~XMLVersionListImport() throw();

    com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >&
        GetList() { return maVersions; }
};

class XMLVersionListContext : public SvXMLImportContext
{
private:
    XMLVersionListImport & rLocalRef;

public:

    XMLVersionListContext( XMLVersionListImport& rImport,
                           sal_uInt16 nPrefix,
                           const OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    ~XMLVersionListContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

};

class XMLVersionContext: public SvXMLImportContext
{
private:
    XMLVersionListImport&  rLocalRef;

    static sal_Bool         ParseISODateTimeString(
                                const OUString& rString,
                                com::sun::star::util::DateTime& rDateTime );

public:

    XMLVersionContext( XMLVersionListImport& rImport,
                          sal_uInt16 nPrefix,
                          const OUString& rLocalName,
                          const ::com::sun::star::uno::Reference<
                          ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    ~XMLVersionContext();
};

class XMLVersionListPersistence : public ::cppu::WeakImplHelper1< ::com::sun::star::document::XDocumentRevisionListPersistence >
{
public:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::util::RevisionTag > SAL_CALL load( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL store( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage, const ::com::sun::star::uno::Sequence< ::com::sun::star::util::RevisionTag >& List ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
};

::com::sun::star::uno::Sequence< OUString > SAL_CALL
    XMLVersionListPersistence_getSupportedServiceNames()
    throw();

OUString SAL_CALL XMLVersionPersistence_getImplementationName()
    throw();

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    XMLVersionListPersistence_createInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & )
    throw( ::com::sun::star::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
