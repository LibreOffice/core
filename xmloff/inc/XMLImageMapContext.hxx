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

#ifndef INCLUDED_XMLOFF_INC_XMLIMAGEMAPCONTEXT_HXX
#define INCLUDED_XMLOFF_INC_XMLIMAGEMAPCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>

namespace com { namespace sun { namespace star {
    namespace container { class XIndexContainer; }
    namespace beans { class XPropertySet; }
    namespace xml { namespace sax { class XAttributeList; } }
} } }


class XMLImageMapContext : public SvXMLImportContext
{
    const OUString sImageMap;

    /// the image map to be imported
    css::uno::Reference< css::container::XIndexContainer> xImageMap;

    /// the property set from which to get and where eventually to set the
    /// image map
    css::uno::Reference< css::beans::XPropertySet> xPropertySet;

public:

    XMLImageMapContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        css::uno::Reference< css::beans::XPropertySet> & rPropertySet);

    virtual ~XMLImageMapContext();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    virtual void EndElement() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
