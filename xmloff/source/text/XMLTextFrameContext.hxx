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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLTEXTFRAMECONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLTEXTFRAMECONTEXT_HXX

#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlmultiimagehelper.hxx>

namespace com { namespace sun { namespace star {
    namespace text { class XTextCursor; class XTextContent; }
} } }

class XMLTextFrameContextHyperlink_Impl;

class XMLTextFrameContext : public SvXMLImportContext, public MultiImageImportHelper
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList > m_xAttrList;

    SvXMLImportContextRef m_xImplContext;
    SvXMLImportContextRef m_xReplImplContext;

    XMLTextFrameContextHyperlink_Impl   *m_pHyperlink;
    // Implement Title/Description Elements UI (#i73249#)
    OUString m_sTitle;
    OUString m_sDesc;

    ::com::sun::star::text::TextContentAnchorType   m_eDefaultAnchorType;

    /* The <draw:name> can longer be used to distinguish Writer graphic/text box
       objects and Draw graphic/text box objects.
       The new distinguish attribute is the parent style of the automatic style
       of the object. All Draw objects have an automatic style without a parent style.
       (#i51726#)
    */
    bool m_HasAutomaticStyleWithoutParentStyle;
    bool m_bSupportsReplacement;

    bool CreateIfNotThere( ::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet >& rPropSet );

protected:
    /// helper to get the created xShape instance, needs to be overridden
    virtual OUString getGraphicURLFromImportContext(const SvXMLImportContext& rContext) const override;
    virtual void removeGraphicFromImportContext(const SvXMLImportContext& rContext) const override;

public:

    TYPEINFO_OVERRIDE();

    XMLTextFrameContext( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            ::com::sun::star::text::TextContentAnchorType eDfltAnchorType );
    virtual ~XMLTextFrameContext();

    virtual void EndElement() override;

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) override;

    void SetHyperlink( const OUString& rHRef,
                       const OUString& rName,
                       const OUString& rTargetFrameName,
                       bool bMap );

    ::com::sun::star::text::TextContentAnchorType GetAnchorType() const;

    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextContent > GetTextContent() const;

    // Frame "to character": anchor moves from first to last char after saving (#i33242#)
    ::com::sun::star::uno::Reference <
        ::com::sun::star::drawing::XShape > GetShape() const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
