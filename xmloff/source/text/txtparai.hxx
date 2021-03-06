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


#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/uno/Reference.h>
#include <xmloff/xmlictxt.hxx>

class XMLHints_Impl;
namespace com::sun::star {
    namespace text {  class XTextRange; }
    namespace xml::sax { class XAttributeList; }
}

#define CONV_FROM_STAR_BATS 1
#define CONV_FROM_STAR_MATH 2
#define CONV_STAR_FONT_FLAGS_VALID 4

class XMLParaContext : public SvXMLImportContext
{
    css::uno::Reference < css::text::XTextRange > xStart;
    OUString             sStyleName;
    OUString             m_sXmlId;
    OUString             m_sAbout;
    OUString             m_sProperty;
    OUString             m_sContent;
    OUString             m_sDatatype;
    bool                 m_bHaveAbout;
    sal_Int8             nOutlineLevel;
    std::unique_ptr<XMLHints_Impl> m_xHints;
    // Lost outline numbering in master document (#i73509#)
    bool                 mbOutlineLevelAttrFound;
    bool                 mbOutlineContentVisible;
    bool                 bIgnoreLeadingSpace;
    bool                 bHeading;
    bool                 bIsListHeader;
    bool                 bIsRestart;
    sal_Int16            nStartValue;
    sal_uInt8            nStarFontsConvFlags;

public:


    XMLParaContext( SvXMLImport& rImport,
            sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList );

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL characters( const OUString& rChars ) override;

};

class XMLNumberedParaContext : public SvXMLImportContext
{
    /// text:list-level MINUS 1
    sal_Int16 m_Level;
    /// text:start-value
    sal_Int16 m_StartValue;
    /// text:list-id
    OUString m_ListId;
    /// text:style-name
    css::uno::Reference< css::container::XIndexReplace > m_xNumRules;

public:


    XMLNumberedParaContext( SvXMLImport& i_rImport,
            sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & i_xAttrList );

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    sal_Int16 GetLevel() const { return m_Level; }
    const css::uno::Reference< css::container::XIndexReplace >& GetNumRules() const
        { return m_xNumRules; }
    const OUString& GetListId() const { return m_ListId; }
    sal_Int16 GetStartValue() const { return m_StartValue; }

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
