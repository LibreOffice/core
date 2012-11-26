/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef DBA_XMLSTYLEIMPORT_HXX
#define DBA_XMLSTYLEIMPORT_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include <xmloff/maptype.hxx>
#endif
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include <xmloff/prstylei.hxx>
#endif
#ifndef _XMLOFF_XMLIMPPR_HXX
#include <xmloff/xmlimppr.hxx>
#endif
#ifndef _XMLTEXTMASTERPAGECONTEXT_HXX
#include <xmloff/XMLTextMasterPageContext.hxx>
#endif
#ifndef _XMLTEXTMASTERSTYLESCONTEXT_HXX
#include <xmloff/XMLTextMasterStylesContext.hxx>
#endif
#ifndef _XMLOFF_CONTEXTID_HXX_
#include <xmloff/contextid.hxx>
#endif
#ifndef _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_
#include <xmloff/controlpropertyhdl.hxx>
#endif
#include <vector>

namespace dbaxml
{
    class ODBFilter;

    class OTableStyleContext : public XMLPropStyleContext
    {
        ::rtl::OUString             m_sDataStyleName;
        ::rtl::OUString             sPageStyle;
        const rtl::OUString         sNumberFormat;
        SvXMLStylesContext*         pStyles;
        //  std::vector<ScXMLMapContent>    aMaps;
        com::sun::star::uno::Any    aConditionalFormat;
        sal_Int32                   m_nNumberFormat;
        sal_Bool                    bConditionalFormatCreated : 1;
        sal_Bool                    bParentSet : 1;

        ODBFilter& GetOwnImport();

    protected:

        virtual void SetAttribute( sal_uInt16 nPrefixKey,
                                const ::rtl::OUString& rLocalName,
                                const ::rtl::OUString& rValue );

    public:
        OTableStyleContext( ODBFilter& rImport, sal_uInt16 nPrfx,
                const ::rtl::OUString& rLName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                SvXMLStylesContext& rStyles, sal_uInt16 nFamily, sal_Bool bDefaultStyle = sal_False );

        virtual ~OTableStyleContext();


        virtual void FillPropertySet(const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > & rPropSet );

        virtual void SetDefaults();

          void AddProperty(sal_Int16 nContextID, const com::sun::star::uno::Any& aValue);

        sal_Int32 GetNumberFormat() { return m_nNumberFormat; }
    };

    class OTableStylesContext : public SvXMLStylesContext
    {
        const ::rtl::OUString sTableStyleServiceName;
        const ::rtl::OUString sColumnStyleServiceName;
        const ::rtl::OUString sCellStyleServiceName;
        sal_Int32 m_nNumberFormatIndex;
        sal_Int32 nMasterPageNameIndex;
        sal_Bool bAutoStyles : 1;

        mutable UniReference < SvXMLImportPropertyMapper > m_xTableImpPropMapper;
        mutable UniReference < SvXMLImportPropertyMapper > m_xColumnImpPropMapper;
        mutable UniReference < SvXMLImportPropertyMapper > m_xCellImpPropMapper;

        ODBFilter& GetOwnImport();

    protected:

        // Create a style context.
        virtual SvXMLStyleContext *CreateStyleStyleChildContext(
                sal_uInt16 nFamily,
                sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    public:
        OTableStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx ,
                const ::rtl::OUString& rLName ,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                const sal_Bool bAutoStyles );
        virtual ~OTableStylesContext();

        virtual void EndElement();

        virtual UniReference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
                            sal_uInt16 nFamily ) const;
        virtual ::com::sun::star::uno::Reference <
                        ::com::sun::star::container::XNameContainer >
            GetStylesContainer( sal_uInt16 nFamily ) const;
        virtual ::rtl::OUString GetServiceName( sal_uInt16 nFamily ) const;

        sal_Int32 GetIndex(const sal_Int16 nContextID);
    };
// -----------------------------------------------------------------------------
} // dbaxml
// -----------------------------------------------------------------------------
#endif // DBA_XMLSTYLEIMPORT_HXX
