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
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>

namespace com::sun::star {
    namespace container { class XIndexContainer; }
    namespace beans { class XPropertySet; }
    namespace xml::sax { class XAttributeList; }
}


class XMLImageMapContext final : public SvXMLImportContext
{
    /// the image map to be imported
    css::uno::Reference< css::container::XIndexContainer> xImageMap;

    /// the property set from which to get and where eventually to set the
    /// image map
    css::uno::Reference< css::beans::XPropertySet> xPropertySet;

public:

    XMLImageMapContext(
        SvXMLImport& rImport,
        css::uno::Reference< css::beans::XPropertySet> const & rPropertySet);

    virtual ~XMLImageMapContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
