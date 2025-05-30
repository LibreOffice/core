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
#include <com/sun/star/uno/Sequence.h>

namespace com::sun::star {
    namespace beans { class XPropertySet; }
}


/**
 * Import text sections.
 *
 * This context may *also* be used for index header sections. The
 * differentiates its behaviour based on GetLocalName().
 */
class XMLSectionImportContext final : public SvXMLImportContext
{
    /// TextSection (as XPropertySet) for passing down to data source elements
    css::uno::Reference<css::beans::XPropertySet> xSectionPropertySet;

    OUString sXmlId;
    OUString sStyleName;
    OUString sName;
    OUString sCond;
    css::uno::Sequence<sal_Int8> aSequence;
    bool bProtect;
    bool bCondOK;
    bool bIsVisible;
    bool bValid;
    bool bSequenceOK;
    bool bIsCurrentlyVisible;
    bool bIsCurrentlyVisibleOK;

    bool bHasContent;

public:

    XMLSectionImportContext( SvXMLImport& rImport );

    virtual ~XMLSectionImportContext() override;

private:

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    void ProcessAttributes(
        const css::uno::Reference<css::xml::sax::XFastAttributeList> & xAttrList );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
