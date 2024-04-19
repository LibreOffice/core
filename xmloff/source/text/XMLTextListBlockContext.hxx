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

class XMLTextListBlockContext : public SvXMLImportContext
{
    XMLTextImportHelper&    mrTxtImport;

    css::uno::Reference< css::container::XIndexReplace > mxNumRules;

    // text:style-name property of <list> element
    OUString         msListStyleName;

    rtl::Reference<XMLTextListBlockContext> mxParentListBlock;

    sal_Int16               mnLevel;
    bool                mbRestartNumbering;
    bool                mbSetDefaults;

    // text:id property of <list> element, only valid for root <list> element
    OUString msListId;
    // text:continue-list property of <list> element, only valid for root <list> element
    OUString msContinueListId;

public:


    // add optional parameter <bRestartNumberingAtSubList>
    XMLTextListBlockContext(
                SvXMLImport& rImport,
                XMLTextImportHelper& rTxtImp,
                const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList,
                const bool bRestartNumberingAtSubList = false );
    virtual ~XMLTextListBlockContext() override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    sal_Int16 GetLevel() const { return mnLevel; }
    bool IsRestartNumbering() const { return mbRestartNumbering; }
    void ResetRestartNumbering() { mbRestartNumbering = false; }

    /// does this list have (possibly inherited from parent) list-style-name?
    const css::uno::Reference < css::container::XIndexReplace >& GetNumRules() const
        { return mxNumRules; }

    const OUString& GetListId() const { return msListId;}
    const OUString& GetContinueListId() const { return msContinueListId;}
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
