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

#include <vector>

#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>

class XMLHints_Impl;

/// Imports <loext:content-control>.
class XMLContentControlContext : public SvXMLImportContext
{
    XMLHints_Impl& m_rHints;

    bool& m_rIgnoreLeadingSpace;

    css::uno::Reference<css::text::XTextRange> m_xStart;

    bool m_bShowingPlaceHolder = false;

    bool m_bCheckbox = false;
    bool m_bChecked = false;
    OUString m_aCheckedState;
    OUString m_aUncheckedState;
    std::vector<css::beans::PropertyValues> m_aListItems;
    bool m_bPicture = false;
    bool m_bDate = false;
    OUString m_aDateFormat;
    OUString m_aDateLanguage;
    OUString m_aCurrentDate;

public:
    XMLContentControlContext(SvXMLImport& rImport, sal_Int32 nElement, XMLHints_Impl& rHints,
                             bool& rIgnoreLeadingSpace);

    void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;

    void SAL_CALL endFastElement(sal_Int32 nElement) override;

    css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& rAttrList) override;

    void SAL_CALL characters(const OUString& rChars) override;

    void AppendListItem(const css::beans::PropertyValues& rListItem);
};

/// Imports <loext:list-item> inside <loext:content-control>.
class XMLListItemContext : public SvXMLImportContext
{
    XMLContentControlContext& m_rContentControl;

public:
    XMLListItemContext(SvXMLImport& rImport, XMLContentControlContext& rContentControl);

    void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
