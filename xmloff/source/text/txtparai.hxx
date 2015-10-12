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
#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_TXTPARAI_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_TXTPARAI_HXX


#include <com/sun/star/uno/Reference.h>
#include <xmloff/xmlictxt.hxx>

class XMLHints_Impl;
namespace com { namespace sun { namespace star {
namespace text {  class XTextRange; }
namespace xml { namespace sax { class XAttributeList; } }
} } }

#define CONV_FROM_STAR_BATS 1
#define CONV_FROM_STAR_MATH 2
#define CONV_STAR_FONT_FLAGS_VALID 4

class XMLParaContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextRange > xStart;
    OUString             sStyleName;
    OUString             m_sXmlId;
    OUString             m_sAbout;
    OUString             m_sProperty;
    OUString             m_sContent;
    OUString             m_sDatatype;
    bool                        m_bHaveAbout;
    sal_Int8                nOutlineLevel;
    XMLHints_Impl           *pHints;
    // Lost outline numbering in master document (#i73509#)
    bool                mbOutlineLevelAttrFound;
    bool                bIgnoreLeadingSpace;
    bool                bHeading;
    bool                bIsListHeader;
    bool                bIsRestart;
    sal_Int16               nStartValue;
    sal_uInt8               nStarFontsConvFlags;

public:

    TYPEINFO_OVERRIDE();

    XMLParaContext( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            bool bHeading );

    virtual ~XMLParaContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) override;

    virtual void Characters( const OUString& rChars ) override;

};

class XMLNumberedParaContext : public SvXMLImportContext
{
    /// text:list-level MINUS 1
    sal_Int16 m_Level;
    /// text:start-value
    sal_Int16 m_StartValue;
    /// xml:id
    OUString m_XmlId;
    /// text:list-id
    OUString m_ListId;
    /// text:style-name
    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace > m_xNumRules;

public:

    TYPEINFO_OVERRIDE();

    XMLNumberedParaContext( SvXMLImport& i_rImport,
            sal_uInt16 i_nPrefix,
            const OUString& i_rLocalName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & i_xAttrList );

    virtual ~XMLNumberedParaContext();

    virtual void EndElement() override;

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 i_nPrefix,
            const OUString& i_rLocalName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & i_xAttrList ) override;

    sal_Int16 GetLevel() const { return m_Level; }
    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace >& GetNumRules() const
        { return m_xNumRules; }
    const OUString& GetListId() const { return m_ListId; }
    sal_Int16 GetStartValue() const { return m_StartValue; }

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
