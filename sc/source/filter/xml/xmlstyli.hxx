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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLSTYLI_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLSTYLI_HXX

#include <rtl/ustring.hxx>
#include <vector>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/XMLTextMasterPageContext.hxx>
#include <xmloff/txtstyli.hxx>
#include "xmlimprt.hxx"

class ScConditionalFormat;

class ScXMLCellImportPropertyMapper : public SvXMLImportPropertyMapper
{
protected:

public:

    ScXMLCellImportPropertyMapper(
            const rtl::Reference< XMLPropertySetMapper >& rMapper,
            SvXMLImport& rImport);
    virtual ~ScXMLCellImportPropertyMapper() override;

    /** This method is called when all attributes have been processed. It may be used to remove items that are incomplete */
    virtual void finished(
            ::std::vector< XMLPropertyState >& rProperties, sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const override;
};

class ScXMLRowImportPropertyMapper : public SvXMLImportPropertyMapper
{
protected:

public:

    ScXMLRowImportPropertyMapper(
            const rtl::Reference< XMLPropertySetMapper >& rMapper,
            SvXMLImport& rImport);
    virtual ~ScXMLRowImportPropertyMapper() override;

    /** This method is called when all attributes have been processed. It may be used to remove items that are incomplete */
    virtual void finished(
            ::std::vector< XMLPropertyState >& rProperties, sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const override;
};

class XMLTableStyleContext : public XMLPropStyleContext
{
    OUString             sDataStyleName;
    OUString               sPageStyle;
    SvXMLStylesContext*         pStyles;
    sal_Int32                   nNumberFormat;
    SCTAB                       nLastSheet;
    bool                        bParentSet;
    ScConditionalFormat*        mpCondFormat;
    bool                        mbDeleteCondFormat;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue ) override;

public:


    XMLTableStyleContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            SvXMLStylesContext& rStyles, XmlStyleFamily nFamily, bool bDefaultStyle = false );
    virtual ~XMLTableStyleContext() override;

    virtual SvXMLImportContextRef CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    virtual void FillPropertySet(const css::uno::Reference<
                css::beans::XPropertySet > & rPropSet ) override;

    virtual void SetDefaults() override;

      void AddProperty(sal_Int16 nContextID, const css::uno::Any& aValue);
    XMLPropertyState* FindProperty(const sal_Int16 nContextID);

    sal_Int32 GetNumberFormat();// { return nNumberFormat; }

    SCTAB GetLastSheet() const       { return nLastSheet; }
    void SetLastSheet(SCTAB nNew)    { nLastSheet = nNew; }

    void ApplyCondFormat( const css::uno::Sequence<css::table::CellRangeAddress>& xCellRanges );

private:
    using XMLPropStyleContext::SetStyle;
};

class XMLTableStylesContext : public SvXMLStylesContext
{
    css::uno::Reference< css::container::XNameContainer > xCellStyles;
    css::uno::Reference< css::container::XNameContainer > xColumnStyles;
    css::uno::Reference< css::container::XNameContainer > xRowStyles;
    css::uno::Reference< css::container::XNameContainer > xTableStyles;
    sal_Int32 nNumberFormatIndex;
    sal_Int32 nConditionalFormatIndex;
    sal_Int32 nCellStyleIndex;
    sal_Int32 nMasterPageNameIndex;
    bool bAutoStyles;

    rtl::Reference < SvXMLImportPropertyMapper > xCellImpPropMapper;
    rtl::Reference < SvXMLImportPropertyMapper > xColumnImpPropMapper;
    rtl::Reference < SvXMLImportPropertyMapper > xRowImpPropMapper;
    rtl::Reference < SvXMLImportPropertyMapper > xTableImpPropMapper;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

protected:

    // Create a style context.
    using SvXMLStylesContext::CreateStyleStyleChildContext;
    virtual SvXMLStyleContext *CreateStyleStyleChildContext(
            XmlStyleFamily nFamily,
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    using SvXMLStylesContext::CreateDefaultStyleStyleChildContext;
    virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
        XmlStyleFamily nFamily, sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

public:

    XMLTableStylesContext( SvXMLImport& rImport, bool bAutoStyles );
    virtual ~XMLTableStylesContext() override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual rtl::Reference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
                        XmlStyleFamily nFamily ) const override;
    virtual css::uno::Reference< css::container::XNameContainer >
        GetStylesContainer( XmlStyleFamily nFamily ) const override;
    virtual OUString GetServiceName( XmlStyleFamily nFamily ) const override;

    sal_Int32 GetIndex(const sal_Int16 nContextID);
};

class ScXMLMasterStylesContext : public SvXMLStylesContext
{
protected:
    using SvXMLStylesContext::CreateStyleChildContext;
    virtual SvXMLStyleContext *CreateStyleChildContext( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;

    using SvXMLStylesContext::CreateStyleStyleChildContext;
    virtual SvXMLStyleContext *CreateStyleStyleChildContext( XmlStyleFamily nFamily,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    virtual bool InsertStyleFamily( XmlStyleFamily nFamily ) const override;

public:

    ScXMLMasterStylesContext( SvXMLImport& rImport );

    virtual ~ScXMLMasterStylesContext() override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class ScMasterPageContext : public XMLTextMasterPageContext
{
    css::uno::Reference<css::beans::XPropertySet> xPropSet;
    bool                bContainsRightHeader;
    bool                bContainsRightFooter;

    void ClearContent(const OUString& rContent);
public:


    ScMasterPageContext( SvXMLImport& rImport, sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList,
            bool bOverwrite );
    virtual ~ScMasterPageContext() override;

    virtual SvXMLImportContext *CreateHeaderFooterContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            const bool bFooter,
            const bool bLeft,
            const bool bFirst ) override;

    virtual void Finish( bool bOverwrite ) override;
};

class ScCellTextStyleContext : public XMLTextStyleContext
{
    sal_Int32   nLastSheet;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScCellTextStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            SvXMLStylesContext& rStyles, XmlStyleFamily nFamily );
    virtual ~ScCellTextStyleContext() override;

    // override FillPropertySet to store style information
    virtual void FillPropertySet(
            const css::uno::Reference< css::beans::XPropertySet > & rPropSet ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
