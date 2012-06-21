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

#ifndef RPT_XMLSTYLEIMPORT_HXX
#define RPT_XMLSTYLEIMPORT_HXX

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
        ::rtl::OUString             m_sDataStyleName;
        ::rtl::OUString             sPageStyle;
        const rtl::OUString         sNumberFormat;
        SvXMLStylesContext*         pStyles;
        //  std::vector<ScXMLMapContent>    aMaps;
        com::sun::star::uno::Any    aConditionalFormat;
        sal_Int32                   m_nNumberFormat;
        ORptFilter&                 m_rImport;
        sal_Bool                    bConditionalFormatCreated : 1;
        sal_Bool                    bParentSet : 1;

        ORptFilter& GetOwnImport() const;

        OControlStyleContext(const OControlStyleContext&);
        void operator =(const OControlStyleContext&);
    protected:

        virtual void SetAttribute( sal_uInt16 nPrefixKey,
                                const ::rtl::OUString& rLocalName,
                                const ::rtl::OUString& rValue );

    public:

        TYPEINFO();

        OControlStyleContext( ORptFilter& rImport, sal_uInt16 nPrfx,
                const ::rtl::OUString& rLName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                SvXMLStylesContext& rStyles, sal_uInt16 nFamily, sal_Bool bDefaultStyle = sal_False );

        virtual ~OControlStyleContext();


        virtual void FillPropertySet(const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > & rPropSet );

        virtual void SetDefaults();

          void AddProperty(sal_Int16 nContextID, const com::sun::star::uno::Any& aValue);

        sal_Int32 GetNumberFormat() { return m_nNumberFormat; }
    };

    class OReportStylesContext : public SvXMLStylesContext
    {
        const ::rtl::OUString m_sTableStyleFamilyName;
        const ::rtl::OUString m_sColumnStyleFamilyName;
        const ::rtl::OUString m_sRowStyleFamilyName;
        const ::rtl::OUString m_sCellStyleFamilyName;
        ORptFilter&           m_rImport;
        sal_Int32 m_nNumberFormatIndex;
        sal_Int32 nMasterPageNameIndex;
        sal_Bool bAutoStyles : 1;

        //mutable UniReference < SvXMLImportPropertyMapper > m_xControlImpPropMapper;
        mutable UniReference < SvXMLImportPropertyMapper > m_xCellImpPropMapper;
        mutable UniReference < SvXMLImportPropertyMapper > m_xColumnImpPropMapper;
        mutable UniReference < SvXMLImportPropertyMapper > m_xRowImpPropMapper;
        mutable UniReference < SvXMLImportPropertyMapper > m_xTableImpPropMapper;

        mutable ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer > m_xCellStyles;
        mutable ::com::sun::star::uno::Reference <
                        ::com::sun::star::container::XNameContainer > m_xColumnStyles;
        mutable ::com::sun::star::uno::Reference <
                        ::com::sun::star::container::XNameContainer > m_xRowStyles;
        mutable ::com::sun::star::uno::Reference <
                        ::com::sun::star::container::XNameContainer > m_xTableStyles;

        ORptFilter& GetOwnImport() const;

        OReportStylesContext(const OReportStylesContext&);
        void operator =(const OReportStylesContext&);
    protected:

        // Create a style context.
        virtual SvXMLStyleContext *CreateStyleStyleChildContext(
                sal_uInt16 nFamily,
                sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

        virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
                sal_uInt16 nFamily, sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    public:

        TYPEINFO();

        OReportStylesContext( ORptFilter& rImport, sal_uInt16 nPrfx ,
                const ::rtl::OUString& rLName ,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                const sal_Bool bAutoStyles );
        virtual ~OReportStylesContext();

        virtual void EndElement();

        virtual UniReference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
                            sal_uInt16 nFamily ) const;
        virtual ::com::sun::star::uno::Reference <
                        ::com::sun::star::container::XNameContainer >
            GetStylesContainer( sal_uInt16 nFamily ) const;
        virtual ::rtl::OUString GetServiceName( sal_uInt16 nFamily ) const;
        virtual sal_uInt16 GetFamily( const ::rtl::OUString& rFamily ) const;

        sal_Int32 GetIndex(const sal_Int16 nContextID);
    };
// -----------------------------------------------------------------------------
} // rptxml
// -----------------------------------------------------------------------------
#endif // RPT_XMLSTYLEIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
