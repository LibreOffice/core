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


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }


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
    const OUString sIsProtected;
    const OUString sName;

    /** XPropertySet of the index */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> xTOCPropertySet;

    enum IndexTypeEnum eIndexType;

    bool bValid;

    SvXMLImportContextRef xBodyContextRef;

public:


    XMLIndexTOCContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName );

    virtual ~XMLIndexTOCContext();

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList) override;

    virtual void EndElement() override;

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
