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
#include <rtl/ustring.hxx>


namespace com::sun::star {
    namespace xml::sax { class XAttributeList; }
    namespace beans { class XPropertySet; }
}

/**
 * Superclass for index source elements
 */
class XMLIndexSourceBaseContext : public SvXMLImportContext
{
    bool bUseLevelFormats;
    bool bChapterIndex;     /// chapter-wise or document index?
    bool bRelativeTabs;     /// tab stops relative to margin or indent?

protected:

    /// property set of index; must be accessible to subclasses
    css::uno::Reference<css::beans::XPropertySet> & rIndexPropertySet;

public:


    XMLIndexSourceBaseContext(
        SvXMLImport& rImport,
        css::uno::Reference<css::beans::XPropertySet> & rPropSet,
        bool bLevelFormats);

    virtual ~XMLIndexSourceBaseContext() override;

protected:

    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter);

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
