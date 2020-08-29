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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXTOCCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXTOCCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>


namespace com::sun::star {
    namespace xml::sax { class XAttributeList; }
    namespace beans { class XPropertySet; }
}


enum IndexTypeEnum
{
    TEXT_INDEX_TOC,
    TEXT_INDEX_ALPHABETICAL,
    TEXT_INDEX_TABLE,
    TEXT_INDEX_OBJECT,
    TEXT_INDEX_BIBLIOGRAPHY,
    TEXT_INDEX_USER,
    TEXT_INDEX_ILLUSTRATION,

    TEXT_INDEX_UNKNOWN
};

class XMLIndexBodyContext;
/**
 * Import all indices.
 *
 * Originally, this class would import only the TOC (table of
 * content), but now it's role has been expanded to handle all
 * indices, and hence is named inappropriately. Depending on the
 * element name it decides which index source element context to create.
 */
class XMLIndexTOCContext : public SvXMLImportContext
{
    /** XPropertySet of the index */
    css::uno::Reference<css::beans::XPropertySet> xTOCPropertySet;

    enum IndexTypeEnum eIndexType;

    bool bValid;

    rtl::Reference<XMLIndexBodyContext> xBodyContextRef;

public:


    XMLIndexTOCContext( SvXMLImport& rImport, sal_Int32 nElement );

    virtual ~XMLIndexTOCContext() override;

protected:

    virtual void SAL_CALL startFastElement( sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual SvXMLImportContextRef CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
