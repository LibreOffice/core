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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLTABLESOURCECONTEXT_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLTABLESOURCECONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/sheet/SheetLinkMode.hpp>
#include "xmlimprt.hxx"
#include "importcontext.hxx"

class ScXMLTableSourceContext : public ScXMLImportContext
{
    OUString                           sLink;
    OUString                           sTableName;
    OUString                           sFilterName;
    OUString                           sFilterOptions;
    sal_Int32                          nRefresh;
    css::sheet::SheetLinkMode          nMode;

public:
    ScXMLTableSourceContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList );

    virtual ~ScXMLTableSourceContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
