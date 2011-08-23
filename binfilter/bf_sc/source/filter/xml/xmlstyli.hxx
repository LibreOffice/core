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

#ifndef _XMLSTYLI_HXX
#define _XMLSTYLI_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <bf_xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_XMLICTXT_HXX
#include <bf_xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include <bf_xmloff/maptype.hxx>
#endif
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include <bf_xmloff/prstylei.hxx>
#endif
#ifndef _XMLOFF_XMLIMPPR_HXX
#include <bf_xmloff/xmlimppr.hxx>
#endif
#ifndef _XMLTEXTMASTERPAGECONTEXT_HXX
#include <bf_xmloff/XMLTextMasterPageContext.hxx>
#endif
#ifndef _XMLTEXTMASTERSTYLESCONTEXT_HXX
#include <bf_xmloff/XMLTextMasterStylesContext.hxx>
#endif
#ifndef _COM_SUN_STAR_SHEET_CONDITIONOPERATOR_HPP_
#include <com/sun/star/sheet/ConditionOperator.hpp>
#endif
#include "xmlimprt.hxx"
namespace binfilter {

class ScXMLCellImportPropertyMapper : public SvXMLImportPropertyMapper
{
protected:

public:

    ScXMLCellImportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper,
            SvXMLImport& rImport);
    virtual ~ScXMLCellImportPropertyMapper();

    /** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
/*	virtual sal_Bool handleSpecialItem(
            XMLPropertyState& rProperty,
            ::std::vector< XMLPropertyState >& rProperties,
            const ::rtl::OUString& rValue,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap ) const;*/

    /** this method is called for every item that has the MID_FLAG_NO_ITEM_IMPORT flag set */
/*	virtual sal_Bool handleNoItem(
            sal_Int32 nIndex,
            ::std::vector< XMLPropertyState >& rProperties,
               const ::rtl::OUString& rValue,
               const SvXMLUnitConverter& rUnitConverter,
               const SvXMLNamespaceMap& rNamespaceMap ) const;*/

    /** This method is called when all attributes have been processed. It may be used to remove items that are incomplete */
    virtual void finished(
            ::std::vector< XMLPropertyState >& rProperties, sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const;
};

class ScXMLRowImportPropertyMapper : public SvXMLImportPropertyMapper
{
protected:

public:

    ScXMLRowImportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper,
            SvXMLImport& rImport);
    virtual ~ScXMLRowImportPropertyMapper();

    /** This method is called when all attributes have been processed. It may be used to remove items that are incomplete */
    virtual void finished(
            ::std::vector< XMLPropertyState >& rProperties, sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const;
};
struct ScXMLMapContent;

class XMLTableStyleContext : public XMLPropStyleContext
{
    ::rtl::OUString				sDataStyleName;
    ::rtl::OUString				sPageStyle;
    const ::rtl::OUString			sNumberFormat;
    SvXMLStylesContext*			pStyles;
    std::vector<ScXMLMapContent>	aMaps;
    ::com::sun::star::uno::Any	aConditionalFormat;
    sal_Int32					nNumberFormat;
    sal_Bool					bConditionalFormatCreated : 1;
    sal_Bool					bParentSet : 1;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

    void SetOperator(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aProps,
        const ::com::sun::star::sheet::ConditionOperator aOp) const;
    void SetBaseCellAddress(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aProps,
        const ::rtl::OUString& sBaseCell) const;
    void SetStyle(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aProps,
        const ::rtl::OUString& sApplyStyle) const;
    void SetFormula1(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aProps,
        const ::rtl::OUString& sFormula) const;
    void SetFormula2(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aProps,
        const ::rtl::OUString& sFormula) const;
    void SetFormulas(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aProps,
        const ::rtl::OUString& sFormulas) const;

    void GetConditionalFormat(
        ::com::sun::star::uno::Any& aAny, const ::rtl::OUString& sCondition,
        const ::rtl::OUString& sApplyStyle, const ::rtl::OUString& sBaseCell) const;
protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const ::rtl::OUString& rLocalName,
                               const ::rtl::OUString& rValue );

public:

    TYPEINFO();

    XMLTableStyleContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            SvXMLStylesContext& rStyles, sal_uInt16 nFamily, sal_Bool bDefaultStyle = sal_False );
    virtual ~XMLTableStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void FillPropertySet(const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & rPropSet );

    virtual void SetDefaults();

      void AddProperty(sal_Int16 nContextID, const ::com::sun::star::uno::Any& aValue);

    sal_Int32 GetNumberFormat() { return nNumberFormat; }
};

class XMLTableStylesContext : public SvXMLStylesContext
{
    ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer > xCellStyles;
    ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer > xColumnStyles;
    ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer > xRowStyles;
    ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer > xTableStyles;
    const ::rtl::OUString sCellStyleServiceName;
    const ::rtl::OUString sColumnStyleServiceName;
    const ::rtl::OUString sRowStyleServiceName;
    const ::rtl::OUString sTableStyleServiceName;
    sal_Int32 nNumberFormatIndex;
    sal_Int32 nConditionalFormatIndex;
    sal_Int32 nCellStyleIndex;
    sal_Int32 nMasterPageNameIndex;
    sal_Bool bAutoStyles : 1;

    UniReference < SvXMLImportPropertyMapper > xCellImpPropMapper;
    UniReference < SvXMLImportPropertyMapper > xColumnImpPropMapper;
    UniReference < SvXMLImportPropertyMapper > xRowImpPropMapper;
    UniReference < SvXMLImportPropertyMapper > xTableImpPropMapper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

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

//	virtual SvXMLImportPropertyMapper *GetImpPropMapper();

public:

    XMLTableStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx ,
            const ::rtl::OUString& rLName ,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const sal_Bool bAutoStyles );
    virtual ~XMLTableStylesContext();

    // Create child element.
/*	virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );*/

    virtual void EndElement();

    virtual UniReference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
                        sal_uInt16 nFamily ) const;
    virtual ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer >
        GetStylesContainer( sal_uInt16 nFamily ) const;
    virtual ::rtl::OUString GetServiceName( sal_uInt16 nFamily ) const;

    sal_Int32 GetIndex(const sal_Int16 nContextID);
};

class ScXMLMasterStylesContext : public SvXMLStylesContext
{
protected:
    virtual SvXMLStyleContext *CreateStyleChildContext( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual SvXMLStyleContext *CreateStyleStyleChildContext( sal_uInt16 nFamily,
        sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual sal_Bool InsertStyleFamily( sal_uInt16 nFamily ) const;

public:
    TYPEINFO();

    ScXMLMasterStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList);

    virtual ~ScXMLMasterStylesContext();
    virtual void EndElement();
};

namespace com { namespace sun { namespace star {
    namespace style { class XStyle; }
} } }

class ScMasterPageContext : public XMLTextMasterPageContext
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xPropSet;
    const ::rtl::OUString		sEmpty;
    sal_Bool				bContainsRightHeader : 1;
    sal_Bool				bContainsRightFooter : 1;

    void ClearContent(const ::rtl::OUString& rContent);
public:

    TYPEINFO();

    ScMasterPageContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            sal_Bool bOverwrite );
    virtual ~ScMasterPageContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual SvXMLImportContext *CreateHeaderFooterContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const sal_Bool bFooter,
            const sal_Bool bLeft );

    virtual void Finish( sal_Bool bOverwrite );
};

} //namespace binfilter
#endif
