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


#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLCHANGEELEMENTIMPORTCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLCHANGEELEMENTIMPORTCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax {
        class XAttributeList;
    } }
} } }
class XMLChangedRegionImportContext;



/**
 * Import <text:deletion> and <text:insertion> elements contained in a
 * <text:changed-region> element.
 */
class XMLChangeElementImportContext : public SvXMLImportContext
{
    /**
     * accept text content (paragraphs) in element as redline content?
     *
     * From the "5.5.4 <text:deletion>" section of the ODF 1.2 standard :
     *   The <text:deletion> element may also contain content that was
     *   deleted while change tracking was enabled.
     *
     * No other section in the "5.5 Change Tracking" chapter contain
     * this sentence.
     *
     * So if bAcceptContent is true, we are importing a <text:deletion> element
     */
    bool bAcceptContent;

    /// context of enclosing <text:changed-region> element
    XMLChangedRegionImportContext& rChangedRegion;

public:

    TYPEINFO_OVERRIDE();

    XMLChangeElementImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        /// accept text content (paragraphs) in element as redline content?
        bool bAcceptContent,
        /// context of enclosing <text:changed-region> element
        XMLChangedRegionImportContext& rParent);

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList) SAL_OVERRIDE;

    // Start- and EndElement are needed here to set the inside_deleted_section
    // flag at the corresponding TextImportHelper
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;

    virtual void EndElement() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
