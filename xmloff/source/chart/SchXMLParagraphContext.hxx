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
#ifndef INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLPARAGRAPHCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLPARAGRAPHCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

namespace com { namespace sun { namespace star { namespace xml { namespace sax {
        class XAttributeList;
}}}}}

class SchXMLParagraphContext : public SvXMLImportContext
{
private:
    OUString& mrText;
    OUString* mpId;
    OUStringBuffer maBuffer;

public:
    SchXMLParagraphContext( SvXMLImport& rImport,
                            const OUString& rLocalName,
                            OUString& rText,
                            OUString * pOutId = nullptr );
    virtual ~SchXMLParagraphContext();
    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    virtual void Characters( const OUString& rChars ) override;
};

#endif // INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLPARAGRAPHCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
