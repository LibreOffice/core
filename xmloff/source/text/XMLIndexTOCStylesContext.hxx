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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXTOCSTYLESCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXTOCSTYLESCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>

#include <vector>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }


/**
 * Import <test:index-source-styles> elements and their children
 *
 * (Small hackery here: Because there's only one type of child
 * elements with only one interesting attribute, we completely handle
 * them inside the CreateChildContext method, rather than creating a
 * new import class for them. This must be changed if children become
 * more complex in future versions.)
 */
class XMLIndexTOCStylesContext : public SvXMLImportContext
{
    const OUString sLevelParagraphStyles;

    /// XPropertySet of the index
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & rTOCPropertySet;

    /// style names for this level
    ::std::vector< OUString > aStyleNames;

    /// outline level
    sal_Int32 nOutlineLevel;

public:

    TYPEINFO_OVERRIDE();

    XMLIndexTOCStylesContext(
        SvXMLImport& rImport,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        sal_uInt16 nPrfx,
        const OUString& rLocalName );

    virtual ~XMLIndexTOCStylesContext();

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
