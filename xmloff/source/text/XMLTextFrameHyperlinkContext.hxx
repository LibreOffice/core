/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLTEXTFRAMEHYPERLINKCONTEXT_HXX
#define _XMLTEXTFRAMEHYPERLINKCONTEXT_HXX

#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <xmloff/xmlictxt.hxx>

namespace com { namespace sun { namespace star {
    namespace text { class XTextCursor; class XTextFrame; }
    namespace beans { class XPropertySet; }
} } }

// OD 2004-04-21 #i26791#
class XMLTextFrameHint_Impl;

class XMLTextFrameHyperlinkContext : public SvXMLImportContext
{
    ::rtl::OUString              sHRef;
    ::rtl::OUString              sName;
    ::rtl::OUString              sTargetFrameName;
    ::com::sun::star::text::TextContentAnchorType eDefaultAnchorType;
    SvXMLImportContextRef       xFrameContext;
    sal_Bool                    bMap;

public:

    TYPEINFO();

    XMLTextFrameHyperlinkContext( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            ::com::sun::star::text::TextContentAnchorType eDefaultAnchorType );
    virtual ~XMLTextFrameHyperlinkContext();

    virtual void EndElement();

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    ::com::sun::star::text::TextContentAnchorType GetAnchorType() const;

    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextContent > GetTextContent() const;

    // Frame "to character": anchor moves from first to last char after saving (#i33242#)
    ::com::sun::star::uno::Reference <
        ::com::sun::star::drawing::XShape > GetShape() const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
