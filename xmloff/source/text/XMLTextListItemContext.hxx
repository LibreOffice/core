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

#include <com/sun/star/container/XIndexReplace.hpp>

#include <xmloff/xmlictxt.hxx>

class XMLTextImportHelper;

class XMLTextListItemContext : public SvXMLImportContext
{
    XMLTextImportHelper& rTxtImport;

    sal_Int16                   nStartValue;

    // quantity of <text:list> child elements
    sal_Int16 mnSubListCount;
    // list style instance for text::style-override property
    css::uno::Reference< css::container::XIndexReplace > mxNumRulesOverride;

public:


    XMLTextListItemContext(
            SvXMLImport& rImport,
            XMLTextImportHelper& rTxtImp,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList,
            const bool bIsHeader );
    virtual ~XMLTextListItemContext() override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    bool HasStartValue() const { return -1 != nStartValue; }
    sal_Int16 GetStartValue() const { return nStartValue; }

    bool HasNumRulesOverride() const
    {
        return mxNumRulesOverride.is();
    }
    const css::uno::Reference < css::container::XIndexReplace >& GetNumRulesOverride() const
    {
        return mxNumRulesOverride;
    }
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
