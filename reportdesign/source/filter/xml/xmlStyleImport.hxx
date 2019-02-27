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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLSTYLEIMPORT_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLSTYLEIMPORT_HXX

#include <rtl/ustring.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/XMLTextMasterPageContext.hxx>
#include <xmloff/XMLTextMasterStylesContext.hxx>
#include <xmloff/contextid.hxx>
#include <xmloff/controlpropertyhdl.hxx>
#include <vector>

namespace rptxml
{
    class ORptFilter;

    class OControlStyleContext : public XMLPropStyleContext
    {
        OUString             m_sDataStyleName;
        SvXMLStylesContext*         pStyles;
        //  std::vector<ScXMLMapContent>    aMaps;
        sal_Int32                   m_nNumberFormat;
        ORptFilter&                 m_rImport;

        ORptFilter& GetOwnImport() const { return m_rImport;}

        OControlStyleContext(const OControlStyleContext&) = delete;
        void operator =(const OControlStyleContext&) = delete;
    protected:

        virtual void SetAttribute( sal_uInt16 nPrefixKey,
                                const OUString& rLocalName,
                                const OUString& rValue ) override;

    public:

        OControlStyleContext( ORptFilter& rImport, sal_uInt16 nPrfx,
                const OUString& rLName,
                const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
                SvXMLStylesContext& rStyles, sal_uInt16 nFamily );

        virtual ~OControlStyleContext() override;


        virtual void FillPropertySet(const css::uno::Reference<
                    css::beans::XPropertySet > & rPropSet ) override;

        virtual void SetDefaults() override;

        void AddProperty(sal_Int16 nContextID, const css::uno::Any& aValue);
    };

    class OReportStylesContext : public SvXMLStylesContext
    {
        ORptFilter&           m_rImport;
        sal_Int32 m_nNumberFormatIndex;
        bool const bAutoStyles : 1;

        //mutable rtl::Reference < SvXMLImportPropertyMapper > m_xControlImpPropMapper;
        mutable rtl::Reference < SvXMLImportPropertyMapper > m_xCellImpPropMapper;
        mutable rtl::Reference < SvXMLImportPropertyMapper > m_xColumnImpPropMapper;
        mutable rtl::Reference < SvXMLImportPropertyMapper > m_xRowImpPropMapper;
        mutable rtl::Reference < SvXMLImportPropertyMapper > m_xTableImpPropMapper;

        mutable css::uno::Reference< css::container::XNameContainer > m_xCellStyles;
        mutable css::uno::Reference< css::container::XNameContainer > m_xColumnStyles;
        mutable css::uno::Reference< css::container::XNameContainer > m_xRowStyles;
        mutable css::uno::Reference< css::container::XNameContainer > m_xTableStyles;

        ORptFilter& GetOwnImport() const { return m_rImport;}

        OReportStylesContext(const OReportStylesContext&) = delete;
        void operator =(const OReportStylesContext&) = delete;
    protected:

        // Create a style context.
        virtual SvXMLStyleContext *CreateStyleStyleChildContext(
                sal_uInt16 nFamily,
                sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

        virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
                sal_uInt16 nFamily, sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    public:


        OReportStylesContext( ORptFilter& rImport, sal_uInt16 nPrfx ,
                const OUString& rLName ,
                const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
                const bool bAutoStyles );
        virtual ~OReportStylesContext() override;

        virtual void EndElement() override;

        virtual rtl::Reference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
                            sal_uInt16 nFamily ) const override;
        virtual css::uno::Reference< css::container::XNameContainer >
            GetStylesContainer( sal_uInt16 nFamily ) const override;
        virtual OUString GetServiceName( sal_uInt16 nFamily ) const override;

        sal_Int32 GetIndex(const sal_Int16 nContextID);
    };

} // rptxml

#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLSTYLEIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
