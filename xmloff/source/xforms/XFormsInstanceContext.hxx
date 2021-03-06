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

#include "TokenContext.hxx"
#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star {
    namespace xml::sax { class XAttributeList; }
    namespace xml::dom { class XDocument; }
    namespace beans { class XPropertySet; }
    namespace xforms { class XModel2; }
}

class SvXMLImport;
class SvXMLImportContext;

/** import the xforms:instance element */
class XFormsInstanceContext : public TokenContext
{
    css::uno::Reference<css::xforms::XModel2> mxModel;
    css::uno::Reference<css::xml::dom::XDocument> mxInstance;
    OUString msId;
    OUString msURL;

public:
    XFormsInstanceContext( SvXMLImport& rImport,
                           const css::uno::Reference<css::xforms::XModel2> & xModel );

    // implement SvXMLImportContext & TokenContext methods:
    // We override CreateChildContext, because we want to read
    // arbitrary DOM elements. For the attributes, we use the
    // TokenContext mechanism.

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext(
        const OUString& Namespace, const OUString& Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
    virtual void SAL_CALL endUnknownElement(const OUString & Namespace, const OUString & Name) override;

protected:
    virtual void HandleAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter ) override;

    virtual SvXMLImportContext* HandleChild(
        sal_Int32 nElementToken,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
