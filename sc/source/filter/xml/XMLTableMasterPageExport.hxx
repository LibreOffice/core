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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLTABLEMASTERPAGEEXPORT_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLTABLEMASTERPAGEEXPORT_HXX

#include <rtl/ustring.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLTextMasterPageExport.hxx>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>

#include "xmlexprt.hxx"

namespace com { namespace sun { namespace star {
    namespace text { class XText; }
} } }

class XMLTableMasterPageExport : public XMLTextMasterPageExport
{
    void exportHeaderFooter(const css::uno::Reference < css::sheet::XHeaderFooterContent >& xHeaderFooter,
                            const xmloff::token::XMLTokenEnum aName,
                            const bool bDisplay);

protected:
    virtual void exportHeaderFooterContent(
            const css::uno::Reference< css::text::XText >& rText,
            bool bAutoStyles, bool bProgress = true ) override;

    virtual void exportMasterPageContent(
                const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
                 bool bAutoStyles ) override;

public:
    explicit XMLTableMasterPageExport( ScXMLExport& rExp );
    virtual ~XMLTableMasterPageExport() override;
};

#endif  //  _XMLOFF_XMLTABLEMASTERPAGEEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
