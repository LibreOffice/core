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
#ifndef INCLUDED_XMLOFF_INC_XMLVERSION_HXX
#define INCLUDED_XMLOFF_INC_XMLVERSION_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/document/XDocumentRevisionListPersistence.hpp>
#include <com/sun/star/util/RevisionTag.hpp>
#include <com/sun/star/embed/XStorage.hpp>

#include <cppuhelper/implbase.hxx>
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
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rContext,
        const com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >& rVersions,
        const OUString &rFileName,
        com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > &rHandler );
    virtual     ~XMLVersionListExport() {}

    sal_uInt32  exportDoc( enum ::xmloff::token::XMLTokenEnum eClass ) override;
    void        _ExportAutoStyles() override {}
    void        _ExportMasterStyles () override {}
    void        _ExportContent() override {}
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
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) override;

public:

    XMLVersionListImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rContext,
        com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >& rVersions );
    virtual ~XMLVersionListImport() throw();

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

    virtual ~XMLVersionListContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) override;

};

class XMLVersionContext: public SvXMLImportContext
{
private:
    XMLVersionListImport&  rLocalRef;

    static bool         ParseISODateTimeString(
                                const OUString& rString,
                                com::sun::star::util::DateTime& rDateTime );

public:

    XMLVersionContext( XMLVersionListImport& rImport,
                          sal_uInt16 nPrefix,
                          const OUString& rLocalName,
                          const ::com::sun::star::uno::Reference<
                          ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual ~XMLVersionContext();
};

class XMLVersionListPersistence : public ::cppu::WeakImplHelper< ::com::sun::star::document::XDocumentRevisionListPersistence, css::lang::XServiceInfo >
{
public:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::util::RevisionTag > SAL_CALL load( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL store( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage, const ::com::sun::star::uno::Sequence< ::com::sun::star::util::RevisionTag >& List ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
