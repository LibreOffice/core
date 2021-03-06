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

#include <memory>

#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <xmloff/xmlictxt.hxx>
#include <xmlmultiimagehelper.hxx>

namespace com::sun::star {
    namespace text { class XTextCursor; class XTextContent; }
}

class XMLTextFrameContextHyperlink_Impl;

class XMLTextFrameContext : public SvXMLImportContext, public MultiImageImportHelper
{
    rtl::Reference< sax_fastparser::FastAttributeList > m_xAttrList;

    SvXMLImportContextRef m_xImplContext;
    SvXMLImportContextRef m_xReplImplContext;

    std::unique_ptr<XMLTextFrameContextHyperlink_Impl> m_pHyperlink;
    // Implement Title/Description Elements UI (#i73249#)
    OUString m_sTitle;
    OUString m_sDesc;

    css::text::TextContentAnchorType   m_eDefaultAnchorType;

    /* The <draw:name> can longer be used to distinguish Writer graphic/text box
       objects and Draw graphic/text box objects.
       The new distinguish attribute is the parent style of the automatic style
       of the object. All Draw objects have an automatic style without a parent style.
       (#i51726#)
    */
    bool m_HasAutomaticStyleWithoutParentStyle;
    bool m_bSupportsReplacement;

    bool CreateIfNotThere( css::uno::Reference < css::beans::XPropertySet >& rPropSet );

protected:
    /// helper to get the created xShape instance, needs to be overridden
    void removeGraphicFromImportContext(const SvXMLImportContext& rContext) override;
    OUString getGraphicPackageURLFromImportContext(const SvXMLImportContext& rContext) const override;
    css::uno::Reference<css::graphic::XGraphic> getGraphicFromImportContext(const SvXMLImportContext& rContext) const override;

public:


    XMLTextFrameContext( SvXMLImport& rImport,
            const css::uno::Reference<
                css::xml::sax::XFastAttributeList > & xAttrList,
            css::text::TextContentAnchorType eDfltAnchorType );

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    void SetHyperlink( const OUString& rHRef,
                       const OUString& rName,
                       const OUString& rTargetFrameName,
                       bool bMap );

    css::text::TextContentAnchorType GetAnchorType() const;

    css::uno::Reference < css::text::XTextContent > GetTextContent() const;

    // Frame "to character": anchor moves from first to last char after saving (#i33242#)
    css::uno::Reference < css::drawing::XShape > GetShape() const;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
