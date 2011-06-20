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
