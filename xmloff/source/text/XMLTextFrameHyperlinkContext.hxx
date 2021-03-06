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

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <xmloff/xmlictxt.hxx>

namespace com::sun::star {
    namespace text { class XTextCursor; class XTextFrame; }
    namespace beans { class XPropertySet; }
}

class XMLTextFrameHyperlinkContext : public SvXMLImportContext
{
    OUString              sHRef;
    OUString              sName;
    OUString              sTargetFrameName;
    css::text::TextContentAnchorType eDefaultAnchorType;
    SvXMLImportContextRef   xFrameContext;
    bool                    bMap;

public:

    XMLTextFrameHyperlinkContext( SvXMLImport& rImport,
            sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList,
            css::text::TextContentAnchorType eDefaultAnchorType );
    virtual ~XMLTextFrameHyperlinkContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    css::text::TextContentAnchorType GetAnchorType() const;

    css::uno::Reference < css::text::XTextContent > GetTextContent() const;

    // Frame "to character": anchor moves from first to last char after saving (#i33242#)
    css::uno::Reference < css::drawing::XShape > GetShape() const;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
