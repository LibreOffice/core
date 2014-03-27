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
        OUString             m_sDataStyleName;
        OUString             sPageStyle;
        const OUString         sNumberFormat;
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
                                const OUString& rLocalName,
                                const OUString& rValue ) SAL_OVERRIDE;

    public:

        TYPEINFO_OVERRIDE();

        OControlStyleContext( ORptFilter& rImport, sal_uInt16 nPrfx,
                const OUString& rLName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                SvXMLStylesContext& rStyles, sal_uInt16 nFamily, sal_Bool bDefaultStyle = sal_False );

        virtual ~OControlStyleContext();


        virtual void FillPropertySet(const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > & rPropSet ) SAL_OVERRIDE;

        virtual void SetDefaults() SAL_OVERRIDE;

          void AddProperty(sal_Int16 nContextID, const com::sun::star::uno::Any& aValue);

        sal_Int32 GetNumberFormat() { return m_nNumberFormat; }
    };

    class OReportStylesContext : public SvXMLStylesContext
    {
        const OUString m_sTableStyleFamilyName;
        const OUString m_sColumnStyleFamilyName;
        const OUString m_sRowStyleFamilyName;
        const OUString m_sCellStyleFamilyName;
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
                const OUString& rLocalName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

        virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
                sal_uInt16 nFamily, sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

    public:

        TYPEINFO_OVERRIDE();

        OReportStylesContext( ORptFilter& rImport, sal_uInt16 nPrfx ,
                const OUString& rLName ,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                const sal_Bool bAutoStyles );
        virtual ~OReportStylesContext();

        virtual void EndElement() SAL_OVERRIDE;

        virtual UniReference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
                            sal_uInt16 nFamily ) const SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference <
                        ::com::sun::star::container::XNameContainer >
            GetStylesContainer( sal_uInt16 nFamily ) const SAL_OVERRIDE;
        virtual OUString GetServiceName( sal_uInt16 nFamily ) const SAL_OVERRIDE;
        virtual sal_uInt16 GetFamily( const OUString& rFamily ) const SAL_OVERRIDE;

        sal_Int32 GetIndex(const sal_Int16 nContextID);
    };

} // rptxml

#endif // RPT_XMLSTYLEIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
