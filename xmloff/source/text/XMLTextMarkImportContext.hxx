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


#pragma once

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>


namespace com::sun::star {
    namespace text {
        class XTextRange;
        class XTextContent;
    }
    namespace xml::sax {
        class XAttributeList;
    }
}
class XMLTextImportHelper;

class XMLFieldParamImportContext : public SvXMLImportContext
{
    XMLTextImportHelper& rHelper;
public:
    XMLFieldParamImportContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rHlp );

    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList> & xAttrList) override;
};


/**
 * import bookmarks and reference marks
 * ( <bookmark>, <bookmark-start>, <bookmark-end>,
 *   <reference>, <reference-start>, <reference-end> )
 *
 * All elements are handled by the same class due to their similarities.
 */
class XMLTextMarkImportContext final : public SvXMLImportContext
{
private:
    XMLTextImportHelper & m_rHelper;

    css::uno::Reference<css::uno::XInterface> & m_rxCrossRefHeadingBookmark;

    OUString m_sBookmarkName;
    OUString m_sFieldName;
    OUString m_sXmlId;
    // RDFa
    bool m_bHaveAbout;
    OUString m_sAbout;
    OUString m_sProperty;
    OUString m_sContent;
    OUString m_sDatatype;

public:

    XMLTextMarkImportContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rHlp,
        css::uno::Reference<css::uno::XInterface> & io_rxCrossRefHeadingBookmark );

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

public:
    static css::uno::Reference< css::text::XTextContent > CreateAndInsertMark(
        SvXMLImport& rImport,
        const OUString& sServiceName,
        const OUString& sMarkName,
        const css::uno::Reference<css::text::XTextRange> & rRange,
        const OUString& i_rXmlId = OUString());

    bool FindName(
        const css::uno::Reference<css::xml::sax::XFastAttributeList> & xAttrList);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
