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
#ifndef _XMLOFF_TEXTPARAI_HXX_
#define _XMLOFF_TEXTPARAI_HXX_


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
        ::com::sun::star::text::XTextRange > xStart;    // xub_StrLen nStart;
    ::rtl::OUString             sStyleName;
    ::rtl::OUString             m_sXmlId;
    ::rtl::OUString             m_sAbout;
    ::rtl::OUString             m_sProperty;
    ::rtl::OUString             m_sContent;
    ::rtl::OUString             m_sDatatype;
    bool                        m_bHaveAbout;
    sal_Int8                nOutlineLevel;
    XMLHints_Impl           *pHints;
    // Lost outline numbering in master document (#i73509#)
    sal_Bool                mbOutlineLevelAttrFound;
    sal_Bool                bIgnoreLeadingSpace;
    sal_Bool                bHeading;
    sal_Bool                bIsListHeader;
    sal_Bool                bIsRestart;
    sal_Int16               nStartValue;
    sal_uInt8               nStarFontsConvFlags;

public:

    TYPEINFO();

    XMLParaContext( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            sal_Bool bHeading );

    virtual ~XMLParaContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void Characters( const ::rtl::OUString& rChars );

};

class XMLNumberedParaContext : public SvXMLImportContext
{
    /// text:list-level MINUS 1
    sal_Int16 m_Level;
    /// text:start-value
    sal_Int16 m_StartValue;
    /// xml:id
    ::rtl::OUString m_XmlId;
    /// text:list-id
    ::rtl::OUString m_ListId;
    /// text:style-name
    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace > m_xNumRules;

public:

    TYPEINFO();

    XMLNumberedParaContext( SvXMLImport& i_rImport,
            sal_uInt16 i_nPrefix,
            const ::rtl::OUString& i_rLocalName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & i_xAttrList );

    virtual ~XMLNumberedParaContext();

    virtual void EndElement();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 i_nPrefix,
            const ::rtl::OUString& i_rLocalName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & i_xAttrList );

    sal_Int16 GetLevel() const { return m_Level; }
    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace >& GetNumRules() const
        { return m_xNumRules; }
    const ::rtl::OUString& GetListId() const { return m_ListId; }
    sal_Int16 GetStartValue() const { return m_StartValue; }

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
