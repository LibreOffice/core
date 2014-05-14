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
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/XMLTextMasterPageContext.hxx>
#include <xmloff/XMLTextMasterStylesContext.hxx>
#include <xmloff/txtstyli.hxx>
#include <com/sun/star/sheet/ConditionOperator.hpp>
#include "xmlimprt.hxx"

class ScConditionalFormat;

class ScXMLCellImportPropertyMapper : public SvXMLImportPropertyMapper
{
protected:

public:

    ScXMLCellImportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper,
            SvXMLImport& rImport);
    virtual ~ScXMLCellImportPropertyMapper();

    /** This method is called when all attributes have been processed. It may be used to remove items that are incomplete */
    virtual void finished(
            ::std::vector< XMLPropertyState >& rProperties, sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const SAL_OVERRIDE;
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
            ::std::vector< XMLPropertyState >& rProperties, sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const SAL_OVERRIDE;
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

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue ) SAL_OVERRIDE;

public:

    TYPEINFO_OVERRIDE();

    XMLTableStyleContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            SvXMLStylesContext& rStyles, sal_uInt16 nFamily, bool bDefaultStyle = false );
    virtual ~XMLTableStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

    virtual void FillPropertySet(const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & rPropSet ) SAL_OVERRIDE;

    virtual void SetDefaults() SAL_OVERRIDE;

      void AddProperty(sal_Int16 nContextID, const com::sun::star::uno::Any& aValue);
    XMLPropertyState* FindProperty(const sal_Int16 nContextID);

    sal_Int32 GetNumberFormat();// { return nNumberFormat; }

    SCTAB GetLastSheet() const       { return nLastSheet; }
    void SetLastSheet(SCTAB nNew)    { nLastSheet = nNew; }

    void ApplyCondFormat( const com::sun::star::uno::Sequence<com::sun::star::table::CellRangeAddress>& xCellRanges );

private:
    using XMLPropStyleContext::SetStyle;
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
    const OUString sCellStyleServiceName;
    const OUString sColumnStyleServiceName;
    const OUString sRowStyleServiceName;
    const OUString sTableStyleServiceName;
    sal_Int32 nNumberFormatIndex;
    sal_Int32 nConditionalFormatIndex;
    sal_Int32 nCellStyleIndex;
    sal_Int32 nMasterPageNameIndex;
    bool bAutoStyles;

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
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

    virtual SvXMLStyleContext *CreateDefaultStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

public:

    XMLTableStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx ,
            const OUString& rLName ,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const bool bAutoStyles );
    virtual ~XMLTableStylesContext();

    virtual void EndElement() SAL_OVERRIDE;

    virtual UniReference < SvXMLImportPropertyMapper > GetImportPropertyMapper(
                        sal_uInt16 nFamily ) const SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference <
                    ::com::sun::star::container::XNameContainer >
        GetStylesContainer( sal_uInt16 nFamily ) const SAL_OVERRIDE;
    virtual OUString GetServiceName( sal_uInt16 nFamily ) const SAL_OVERRIDE;

    sal_Int32 GetIndex(const sal_Int16 nContextID);
};

class ScXMLMasterStylesContext : public SvXMLStylesContext
{
protected:
    virtual SvXMLStyleContext *CreateStyleChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

    virtual SvXMLStyleContext *CreateStyleStyleChildContext( sal_uInt16 nFamily,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

    virtual bool InsertStyleFamily( sal_uInt16 nFamily ) const SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();

    ScXMLMasterStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList);

    virtual ~ScXMLMasterStylesContext();
    virtual void EndElement() SAL_OVERRIDE;
};

namespace com { namespace sun { namespace star {
    namespace style { class XStyle; }
} } }

class ScMasterPageContext : public XMLTextMasterPageContext
{
    com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xPropSet;
    const OUString     sEmpty;
    bool                bContainsRightHeader;
    bool                bContainsRightFooter;

    void ClearContent(const OUString& rContent);
public:

    TYPEINFO_OVERRIDE();

    ScMasterPageContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            bool bOverwrite );
    virtual ~ScMasterPageContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

    virtual SvXMLImportContext *CreateHeaderFooterContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const bool bFooter,
            const bool bLeft,
            const bool bFirst ) SAL_OVERRIDE;

    virtual void Finish( bool bOverwrite ) SAL_OVERRIDE;
};

class ScCellTextStyleContext : public XMLTextStyleContext
{
    sal_Int32   nLastSheet;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScCellTextStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            SvXMLStylesContext& rStyles, sal_uInt16 nFamily,
            bool bDefaultStyle = false );
    virtual ~ScCellTextStyleContext();

    // overload FillPropertySet to store style information
    virtual void FillPropertySet(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & rPropSet ) SAL_OVERRIDE;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
