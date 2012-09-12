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

#ifndef XMLOFF_XMLMETAI_HXX
#define XMLOFF_XMLMETAI_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <xmloff/xmlictxt.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/xml/dom/XSAXDocumentBuilder2.hpp>


/// handles the top-level office:document-meta element of meta.xml documents
// NB: virtual inheritance is needed so that the context that handles the
//     flat xml file format can multiply inherit properly
class XMLOFF_DLLPUBLIC SvXMLMetaDocumentContext
    : public virtual SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> mxDocProps;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::dom::XSAXDocumentBuilder2> mxDocBuilder;

public:
    SvXMLMetaDocumentContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XDocumentProperties>& xDocProps);

    virtual ~SvXMLMetaDocumentContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference<
             ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();

public:
    static void setBuildId(const ::rtl::OUString & rGenerator,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xImportInfo );
};

#endif // _XMLOFF_XMLMETAI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
