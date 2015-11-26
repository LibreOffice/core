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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLTEXTFRAMEHYPERLINKCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLTEXTFRAMEHYPERLINKCONTEXT_HXX

#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <xmloff/xmlictxt.hxx>

namespace com { namespace sun { namespace star {
    namespace text { class XTextCursor; class XTextFrame; }
    namespace beans { class XPropertySet; }
} } }

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
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            css::text::TextContentAnchorType eDefaultAnchorType );
    virtual ~XMLTextFrameHyperlinkContext();

    virtual void EndElement() override;

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                 const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    css::text::TextContentAnchorType GetAnchorType() const;

    css::uno::Reference < css::text::XTextContent > GetTextContent() const;

    // Frame "to character": anchor moves from first to last char after saving (#i33242#)
    css::uno::Reference < css::drawing::XShape > GetShape() const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
