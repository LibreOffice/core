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

#ifndef _XMLOFF_XMLBASICI_HXX
#define _XMLOFF_XMLBASICI_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XXMLOasisBasicImporter.hpp>
#include <xmloff/xmlictxt.hxx>

// class XMLBasicImportContext

class XMLBasicImportContext : public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                 m_xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XXMLOasisBasicImporter >    m_xHandler;

public:
    XMLBasicImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxModel );

    virtual ~XMLBasicImportContext();

    virtual SvXMLImportContext* CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& rxAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& rxAttrList );

    virtual void EndElement();

    virtual void Characters( const OUString& rChars );
};

// class XMLBasicImportChildContext

class XMLBasicImportChildContext : public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xHandler;

public:
    XMLBasicImportChildContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >& rxHandler );

    virtual ~XMLBasicImportChildContext();

    virtual SvXMLImportContext* CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();

    virtual void Characters( const OUString& rChars );
};

#endif // _XMLOFF_XMLBASICI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
