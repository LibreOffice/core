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

#include <sal/config.h>

#include <o3tl/ptr_container.hxx>
#include <svl/zforlist.hxx>
#include <sal/macros.h>

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/i18nmap.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlmetai.hxx>
#include <sfx2/objsh.hxx>
#include <xmloff/xmlnumfi.hxx>
#include <xmloff/xmlscripti.hxx>
#include <xmloff/XMLFontStylesContext.hxx>
#include <xmloff/DocumentSettingsContext.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/numehelp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/token/tokens.hxx>
#include <xmloff/xmlerror.hxx>

#include <sax/tools/converter.hxx>

#include <svl/zformat.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/editstat.hxx>

#include "appluno.hxx"
#include "xmlimprt.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "docuno.hxx"
#include "nameuno.hxx"
#include "xmlbodyi.hxx"
#include "xmlstyli.hxx"
#include "ViewSettingsSequenceDefines.hxx"

#include "patattr.hxx"

#include "XMLConverter.hxx"
#include "XMLDetectiveContext.hxx"
#include "XMLTableShapeImportHelper.hxx"
#include "XMLChangeTrackingImportHelper.hxx"
#include "chgviset.hxx"
#include "XMLStylesImportHelper.hxx"
#include "sheetdata.hxx"
#include "rangeutl.hxx"
#include "postit.hxx"
#include "formulaparserpool.hxx"
#include "externalrefmgr.hxx"
#include "editutil.hxx"
#include "editattributemap.hxx"
#include "documentimport.hxx"
#include "pivotsource.hxx"
#include <unonames.hxx>
#include <numformat.hxx>

#include <comphelper/extract.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/util/XMergeable.hpp>
#include <com/sun/star/sheet/CellInsertMode.hpp>
#include <com/sun/star/sheet/XCellRangeMovement.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>

#include <memory>
#include <utility>

#define SC_LOCALE           "Locale"
#define SC_STANDARDFORMAT   "StandardFormat"
#define SC_CURRENCYSYMBOL   "CurrencySymbol"
#define SC_REPEAT_ROW "repeat-row"
#define SC_FILTER "filter"
#define SC_PRINT_RANGE "print-range"

using namespace com::sun::star;
using namespace ::xmloff::token;
using namespace ::formula;
using namespace com::sun::star::xml::sax;
using namespace xmloff;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;

OUString SAL_CALL ScXMLImport_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisImporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLImport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLImport_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLImport(IMPORT_ALL);
    return static_cast<cppu::OWeakObject*>(new ScXMLImport( comphelper::getComponentContext(rSMgr), ScXMLImport_getImplementationName(), SvXMLImportFlags::ALL ));
}

OUString SAL_CALL ScXMLImport_Meta_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisMetaImporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLImport_Meta_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLImport_Meta_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Meta_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLImport(IMPORT_META);
    return static_cast<cppu::OWeakObject*>(new ScXMLImport( comphelper::getComponentContext(rSMgr), ScXMLImport_Meta_getImplementationName(), SvXMLImportFlags::META ));
}

OUString SAL_CALL ScXMLImport_Styles_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisStylesImporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLImport_Styles_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLImport_Styles_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Styles_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLImport(SvXMLImportFlagsSTYLES|SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::MASTERSTYLES|SvXMLImportFlags::FONTDECLS);
    return static_cast<cppu::OWeakObject*>(new ScXMLImport( comphelper::getComponentContext(rSMgr), ScXMLImport_Styles_getImplementationName(), SvXMLImportFlags::STYLES|SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::MASTERSTYLES|SvXMLImportFlags::FONTDECLS));
}

OUString SAL_CALL ScXMLImport_Content_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisContentImporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLImport_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLImport_Content_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Content_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLImport(SvXMLImportFlags::META|SvXMLImportFlags::STYLES|SvXMLImportFlags::MASTERSTYLES|SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::CONTENT|SvXMLImportFlags::SCRIPTS|SvXMLImportFlags::SETTINGS|SvXMLImportFlags::FONTDECLS);
    return static_cast<cppu::OWeakObject*>(new ScXMLImport( comphelper::getComponentContext(rSMgr), ScXMLImport_Content_getImplementationName(), SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::CONTENT|SvXMLImportFlags::SCRIPTS|SvXMLImportFlags::FONTDECLS));
}

OUString SAL_CALL ScXMLImport_Settings_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisSettingsImporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLImport_Settings_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLImport_Settings_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLImport_Settings_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLImport(SvXMLImportFlags::SETTINGS);
    return static_cast<cppu::OWeakObject*>(new ScXMLImport( comphelper::getComponentContext(rSMgr), ScXMLImport_Settings_getImplementationName(), SvXMLImportFlags::SETTINGS ));
}

const SvXMLTokenMap& ScXMLImport::GetTableRowCellAttrTokenMap()
{
    static const SvXMLTokenMapEntry aTableRowCellAttrTokenMap[] =
    {
        { XML_NAMESPACE_TABLE,  XML_STYLE_NAME,                     XML_TOK_TABLE_ROW_CELL_ATTR_STYLE_NAME,
            (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_style_name) },
        { XML_NAMESPACE_TABLE,  XML_CONTENT_VALIDATION_NAME,        XML_TOK_TABLE_ROW_CELL_ATTR_CONTENT_VALIDATION_NAME,
            (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_content_validation_name) },
        { XML_NAMESPACE_TABLE,  XML_NUMBER_ROWS_SPANNED,            XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_ROWS,
            (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_number_rows_spanned) },
        { XML_NAMESPACE_TABLE,  XML_NUMBER_COLUMNS_SPANNED,         XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_COLS,
            (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_number_columns_spanned) },
        { XML_NAMESPACE_TABLE,  XML_NUMBER_MATRIX_COLUMNS_SPANNED,  XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_COLS,
            (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_number_matrix_columns_spanned) },
        { XML_NAMESPACE_TABLE,  XML_NUMBER_MATRIX_ROWS_SPANNED,     XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_ROWS,
            (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_number_matrix_rows_spanned) },
        { XML_NAMESPACE_TABLE,  XML_NUMBER_COLUMNS_REPEATED,        XML_TOK_TABLE_ROW_CELL_ATTR_REPEATED,
            (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_number_columns_repeated) },
        { XML_NAMESPACE_OFFICE, XML_VALUE_TYPE,                     XML_TOK_TABLE_ROW_CELL_ATTR_VALUE_TYPE,
            (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_value_type) },
        { XML_NAMESPACE_CALC_EXT, XML_VALUE_TYPE,                   XML_TOK_TABLE_ROW_CELL_ATTR_NEW_VALUE_TYPE,
            (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_value_type) },
        { XML_NAMESPACE_OFFICE, XML_VALUE,                          XML_TOK_TABLE_ROW_CELL_ATTR_VALUE,
            (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_value) },
        { XML_NAMESPACE_OFFICE, XML_DATE_VALUE,                     XML_TOK_TABLE_ROW_CELL_ATTR_DATE_VALUE,
            (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_date_value) },
        { XML_NAMESPACE_OFFICE, XML_TIME_VALUE,                     XML_TOK_TABLE_ROW_CELL_ATTR_TIME_VALUE,
            (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_time_value) },
        { XML_NAMESPACE_OFFICE, XML_STRING_VALUE,                   XML_TOK_TABLE_ROW_CELL_ATTR_STRING_VALUE,
            (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_string_value) },
        { XML_NAMESPACE_OFFICE, XML_BOOLEAN_VALUE,                  XML_TOK_TABLE_ROW_CELL_ATTR_BOOLEAN_VALUE,
            (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_boolean_value) },
        { XML_NAMESPACE_TABLE,  XML_FORMULA,                        XML_TOK_TABLE_ROW_CELL_ATTR_FORMULA,
            (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_formula) },
        { XML_NAMESPACE_OFFICE, XML_CURRENCY,                       XML_TOK_TABLE_ROW_CELL_ATTR_CURRENCY,
            (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_currency) },
        XML_TOKEN_MAP_END
    };

    if ( !pTableRowCellAttrTokenMap )
        pTableRowCellAttrTokenMap = new SvXMLTokenMap( aTableRowCellAttrTokenMap );
    return *pTableRowCellAttrTokenMap;
}

// NB: virtually inherit so we can multiply inherit properly
//     in ScXMLFlatDocContext_Impl
class ScXMLDocContext_Impl : public virtual SvXMLImportContext
{
protected:
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLDocContext_Impl( ScXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const uno::Reference<xml::sax::XAttributeList>& xAttrList );
    virtual ~ScXMLDocContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference<xml::sax::XAttributeList>& xAttrList ) SAL_OVERRIDE;
};

ScXMLDocContext_Impl::ScXMLDocContext_Impl( ScXMLImport& rImport, sal_uInt16 nPrfx,
                                           const OUString& rLName,
                                           const uno::Reference<xml::sax::XAttributeList>& /* xAttrList */ ) :
SvXMLImportContext( rImport, nPrfx, rLName )
{

}

ScXMLDocContext_Impl::~ScXMLDocContext_Impl()
{
}

// context for flat file xml format
class ScXMLFlatDocContext_Impl
    : public ScXMLDocContext_Impl, public SvXMLMetaDocumentContext
{
public:
    ScXMLFlatDocContext_Impl( ScXMLImport& i_rImport,
        sal_uInt16 i_nPrefix, const OUString & i_rLName,
        const uno::Reference<xml::sax::XAttributeList>& i_xAttrList,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps);

    virtual ~ScXMLFlatDocContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 i_nPrefix, const OUString& i_rLocalName,
        const uno::Reference<xml::sax::XAttributeList>& i_xAttrList) SAL_OVERRIDE;
};

ScXMLFlatDocContext_Impl::ScXMLFlatDocContext_Impl( ScXMLImport& i_rImport,
                                                   sal_uInt16 i_nPrefix, const OUString & i_rLName,
                                                   const uno::Reference<xml::sax::XAttributeList>& i_xAttrList,
                                                   const uno::Reference<document::XDocumentProperties>& i_xDocProps) :
SvXMLImportContext(i_rImport, i_nPrefix, i_rLName),
ScXMLDocContext_Impl(i_rImport, i_nPrefix, i_rLName, i_xAttrList),
SvXMLMetaDocumentContext(i_rImport, i_nPrefix, i_rLName,
                         i_xDocProps)
{
}

ScXMLFlatDocContext_Impl::~ScXMLFlatDocContext_Impl() { }

SvXMLImportContext *ScXMLFlatDocContext_Impl::CreateChildContext(
    sal_uInt16 i_nPrefix, const OUString& i_rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& i_xAttrList)
{
    // behave like meta base class iff we encounter office:meta
    const SvXMLTokenMap& rTokenMap = GetScImport().GetDocElemTokenMap();
    if ( XML_TOK_DOC_META == rTokenMap.Get( i_nPrefix, i_rLocalName ) ) {
        return SvXMLMetaDocumentContext::CreateChildContext(
            i_nPrefix, i_rLocalName, i_xAttrList );
    } else {
        return ScXMLDocContext_Impl::CreateChildContext(
            i_nPrefix, i_rLocalName, i_xAttrList );
    }
}

class ScXMLBodyContext_Impl : public SvXMLImportContext
{
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLBodyContext_Impl( ScXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~ScXMLBodyContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;
};

ScXMLBodyContext_Impl::ScXMLBodyContext_Impl( ScXMLImport& rImport,
                                             sal_uInt16 nPrfx, const OUString& rLName,
                                             const uno::Reference< xml::sax::XAttributeList > & /* xAttrList */ ) :
SvXMLImportContext( rImport, nPrfx, rLName )
{
}

ScXMLBodyContext_Impl::~ScXMLBodyContext_Impl()
{
}

SvXMLImportContext *ScXMLBodyContext_Impl::CreateChildContext(
    sal_uInt16 /* nPrefix */,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    return GetScImport().CreateBodyContext( rLocalName, xAttrList );
}

SvXMLImportContext *ScXMLDocContext_Impl::CreateChildContext( sal_uInt16 nPrefix,
                                                             const OUString& rLocalName,
                                                             const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetDocElemTokenMap());
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_DOC_FONTDECLS:
        if (GetScImport().getImportFlags() & SvXMLImportFlags::FONTDECLS)
            pContext = GetScImport().CreateFontDeclsContext(nPrefix, rLocalName, xAttrList);
        break;
    case XML_TOK_DOC_STYLES:
        if (GetScImport().getImportFlags() & SvXMLImportFlags::STYLES)
            pContext = GetScImport().CreateStylesContext( rLocalName, xAttrList, false);
        break;
    case XML_TOK_DOC_AUTOSTYLES:
        if (GetScImport().getImportFlags() & SvXMLImportFlags::AUTOSTYLES)
            pContext = GetScImport().CreateStylesContext( rLocalName, xAttrList, true);
        break;
    case XML_TOK_DOC_MASTERSTYLES:
        if (GetScImport().getImportFlags() & SvXMLImportFlags::MASTERSTYLES)
            pContext = new ScXMLMasterStylesContext( GetImport(), nPrefix, rLocalName,
            xAttrList );
        break;
    case XML_TOK_DOC_META:
        DBG_WARNING("XML_TOK_DOC_META: should not have come here, maybe document is invalid?");
        break;
    case XML_TOK_DOC_SCRIPTS:
        if (GetScImport().getImportFlags() & SvXMLImportFlags::SCRIPTS)
            pContext = GetScImport().CreateScriptContext( rLocalName );
        break;
    case XML_TOK_DOC_BODY:
        if (GetScImport().getImportFlags() & SvXMLImportFlags::CONTENT)
            pContext = new ScXMLBodyContext_Impl( GetScImport(), nPrefix,
            rLocalName, xAttrList );
        break;
    case XML_TOK_DOC_SETTINGS:
        if (GetScImport().getImportFlags() & SvXMLImportFlags::SETTINGS)
            pContext = new XMLDocumentSettingsContext(GetScImport(), nPrefix, rLocalName, xAttrList );
        break;
    }

    if(!pContext)
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

const SvXMLTokenMap& ScXMLImport::GetDocElemTokenMap()
{
    if( !pDocElemTokenMap )
    {
        static const SvXMLTokenMapEntry aDocTokenMap[] =
        {
            { XML_NAMESPACE_OFFICE, XML_FONT_FACE_DECLS,    XML_TOK_DOC_FONTDECLS,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_font_face_decls) },
            { XML_NAMESPACE_OFFICE, XML_STYLES,             XML_TOK_DOC_STYLES,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_styles) },
            { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_STYLES,   XML_TOK_DOC_AUTOSTYLES,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_automatic_styles) },
            { XML_NAMESPACE_OFFICE, XML_MASTER_STYLES,      XML_TOK_DOC_MASTERSTYLES,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_master_styles) },
            { XML_NAMESPACE_OFFICE, XML_META,               XML_TOK_DOC_META,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_meta) },
            { XML_NAMESPACE_OFFICE, XML_SCRIPTS,            XML_TOK_DOC_SCRIPTS,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_scripts) },
            { XML_NAMESPACE_OFFICE, XML_BODY,               XML_TOK_DOC_BODY,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_body) },
            { XML_NAMESPACE_OFFICE, XML_SETTINGS,           XML_TOK_DOC_SETTINGS,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_settings) },
            XML_TOKEN_MAP_END
        };

        pDocElemTokenMap = new SvXMLTokenMap( aDocTokenMap );

    } // if( !pDocElemTokenMap )

    return *pDocElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetBodyElemTokenMap()
{
    if( !pBodyElemTokenMap )
    {
        static const SvXMLTokenMapEntry aBodyTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_TRACKED_CHANGES,         XML_TOK_BODY_TRACKED_CHANGES,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_tracked_changes) },
            { XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS,    XML_TOK_BODY_CALCULATION_SETTINGS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_calculation_settings) },
            { XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATIONS,     XML_TOK_BODY_CONTENT_VALIDATIONS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_content_validations) },
            { XML_NAMESPACE_TABLE, XML_LABEL_RANGES,            XML_TOK_BODY_LABEL_RANGES,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_label_ranges) },
            { XML_NAMESPACE_TABLE, XML_TABLE,                   XML_TOK_BODY_TABLE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table) },
            { XML_NAMESPACE_TABLE, XML_NAMED_EXPRESSIONS,       XML_TOK_BODY_NAMED_EXPRESSIONS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_named_expressions) },
            { XML_NAMESPACE_TABLE, XML_DATABASE_RANGES,         XML_TOK_BODY_DATABASE_RANGES,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_database_ranges) },
            { XML_NAMESPACE_TABLE, XML_DATABASE_RANGE,          XML_TOK_BODY_DATABASE_RANGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_database_range) },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLES,       XML_TOK_BODY_DATA_PILOT_TABLES,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_tables) },
            { XML_NAMESPACE_TABLE, XML_CONSOLIDATION,           XML_TOK_BODY_CONSOLIDATION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_consolidation) },
            { XML_NAMESPACE_TABLE, XML_DDE_LINKS,               XML_TOK_BODY_DDE_LINKS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_dde_links) },
            { XML_NAMESPACE_CALC_EXT, XML_DATA_STREAM_SOURCE,   XML_TOK_BODY_DATA_STREAM_SOURCE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_data_stream_source) },
            XML_TOKEN_MAP_END
        };

        pBodyElemTokenMap = new SvXMLTokenMap( aBodyTokenMap );
    } // if( !pBodyElemTokenMap )

    return *pBodyElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationsElemTokenMap()
{
    if( !pContentValidationsElemTokenMap )
    {
        static const SvXMLTokenMapEntry aContentValidationsElemTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION,  XML_TOK_CONTENT_VALIDATION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_content_validation) },
            XML_TOKEN_MAP_END
        };

        pContentValidationsElemTokenMap = new SvXMLTokenMap( aContentValidationsElemTokenMap );
    } // if( !pContentValidationsElemTokenMap )

    return *pContentValidationsElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationElemTokenMap()
{
    if( !pContentValidationElemTokenMap )
    {
        static const SvXMLTokenMapEntry aContentValidationElemTokenMap[] =
        {
            { XML_NAMESPACE_TABLE,  XML_HELP_MESSAGE,    XML_TOK_CONTENT_VALIDATION_ELEM_HELP_MESSAGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_help_message) },
            { XML_NAMESPACE_TABLE,  XML_ERROR_MESSAGE,   XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MESSAGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_error_message) },
            { XML_NAMESPACE_TABLE,  XML_ERROR_MACRO,     XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MACRO,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_error_macro) },
            { XML_NAMESPACE_OFFICE, XML_EVENT_LISTENERS, XML_TOK_CONTENT_VALIDATION_ELEM_EVENT_LISTENERS,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_event_listeners) },
            XML_TOKEN_MAP_END
        };

        pContentValidationElemTokenMap = new SvXMLTokenMap( aContentValidationElemTokenMap );
    } // if( !pContentValidationElemTokenMap )

    return *pContentValidationElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationAttrTokenMap()
{
    if( !pContentValidationAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aContentValidationAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_NAME,                XML_TOK_CONTENT_VALIDATION_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_name) },
            { XML_NAMESPACE_TABLE, XML_CONDITION,           XML_TOK_CONTENT_VALIDATION_CONDITION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_condition) },
            { XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS,   XML_TOK_CONTENT_VALIDATION_BASE_CELL_ADDRESS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_base_cell_address) },
            { XML_NAMESPACE_TABLE, XML_ALLOW_EMPTY_CELL,    XML_TOK_CONTENT_VALIDATION_ALLOW_EMPTY_CELL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_allow_empty_cell) },
            { XML_NAMESPACE_TABLE, XML_DISPLAY_LIST,        XML_TOK_CONTENT_VALIDATION_DISPLAY_LIST,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_display_list) },
            XML_TOKEN_MAP_END
        };

        pContentValidationAttrTokenMap = new SvXMLTokenMap( aContentValidationAttrTokenMap );
    } // if( !pContentValidationAttrTokenMap )

    return *pContentValidationAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationMessageElemTokenMap()
{
    if( !pContentValidationMessageElemTokenMap )
    {
        static const SvXMLTokenMapEntry aContentValidationMessageElemTokenMap[] =
        {
            { XML_NAMESPACE_TEXT, XML_P,    XML_TOK_P,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_p) },
            XML_TOKEN_MAP_END
        };

        pContentValidationMessageElemTokenMap = new SvXMLTokenMap( aContentValidationMessageElemTokenMap );
    } // if( !pContentValidationMessageElemTokenMap )

    return *pContentValidationMessageElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationHelpMessageAttrTokenMap()
{
    if( !pContentValidationHelpMessageAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aContentValidationHelpMessageAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_TITLE,   XML_TOK_HELP_MESSAGE_ATTR_TITLE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_title) },
            { XML_NAMESPACE_TABLE, XML_DISPLAY, XML_TOK_HELP_MESSAGE_ATTR_DISPLAY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_display) },
            XML_TOKEN_MAP_END
        };

        pContentValidationHelpMessageAttrTokenMap = new SvXMLTokenMap( aContentValidationHelpMessageAttrTokenMap );
    } // if( !pContentValidationHelpMessageAttrTokenMap )

    return *pContentValidationHelpMessageAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationErrorMessageAttrTokenMap()
{
    if( !pContentValidationErrorMessageAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aContentValidationErrorMessageAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_TITLE,           XML_TOK_ERROR_MESSAGE_ATTR_TITLE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_title) },
            { XML_NAMESPACE_TABLE, XML_DISPLAY,         XML_TOK_ERROR_MESSAGE_ATTR_DISPLAY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_display) },
            { XML_NAMESPACE_TABLE, XML_MESSAGE_TYPE,    XML_TOK_ERROR_MESSAGE_ATTR_MESSAGE_TYPE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_message_type) },
            XML_TOKEN_MAP_END
        };

        pContentValidationErrorMessageAttrTokenMap = new SvXMLTokenMap( aContentValidationErrorMessageAttrTokenMap );
    } // if( !pContentValidationErrorMessageAttrTokenMap )

    return *pContentValidationErrorMessageAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetContentValidationErrorMacroAttrTokenMap()
{
    if( !pContentValidationErrorMacroAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aContentValidationErrorMacroAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_NAME,    XML_TOK_ERROR_MACRO_ATTR_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_name) },
            { XML_NAMESPACE_TABLE, XML_EXECUTE, XML_TOK_ERROR_MACRO_ATTR_EXECUTE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_execute) },
            XML_TOKEN_MAP_END
        };

        pContentValidationErrorMacroAttrTokenMap = new SvXMLTokenMap( aContentValidationErrorMacroAttrTokenMap );
    } // if( !pContentValidationErrorMacroAttrTokenMap )

    return *pContentValidationErrorMacroAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetCondFormatsTokenMap()
{
    if( !pCondFormatsTokenMap )
    {
        static const SvXMLTokenMapEntry aCondFormatsElemTokenMap[] =
        {
            { XML_NAMESPACE_CALC_EXT, XML_CONDITIONAL_FORMAT, XML_TOK_CONDFORMATS_CONDFORMAT,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_conditional_format) },
            XML_TOKEN_MAP_END
        };

        pCondFormatsTokenMap = new SvXMLTokenMap( aCondFormatsElemTokenMap );
    }

    return *pCondFormatsTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetCondFormatTokenMap()
{
    if( !pCondFormatTokenMap )
    {
        static const SvXMLTokenMapEntry aCondFormatElemTokenMap[] =
        {
            { XML_NAMESPACE_CALC_EXT, XML_COLOR_SCALE, XML_TOK_CONDFORMAT_COLORSCALE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_color_scale) },
            { XML_NAMESPACE_CALC_EXT, XML_DATA_BAR, XML_TOK_CONDFORMAT_DATABAR,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_data_bar) },
            { XML_NAMESPACE_CALC_EXT, XML_CONDITION, XML_TOK_CONDFORMAT_CONDITION,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_condition) },
            { XML_NAMESPACE_CALC_EXT, XML_ICON_SET, XML_TOK_CONDFORMAT_ICONSET,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_icon_set) },
            { XML_NAMESPACE_CALC_EXT, XML_DATE_IS, XML_TOK_CONDFORMAT_DATE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_date_is) },
            XML_TOKEN_MAP_END
        };

        pCondFormatTokenMap = new SvXMLTokenMap( aCondFormatElemTokenMap );
    }

    return *pCondFormatTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetCondFormatAttrMap()
{
    if( !pCondFormatAttrMap )
    {
        static const SvXMLTokenMapEntry aCondFormatAttrTokenMap[] =
        {
            { XML_NAMESPACE_CALC_EXT, XML_TARGET_RANGE_ADDRESS, XML_TOK_CONDFORMAT_TARGET_RANGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_target_range_address) },
            XML_TOKEN_MAP_END
        };

        pCondFormatAttrMap = new SvXMLTokenMap( aCondFormatAttrTokenMap );
    }

    return *pCondFormatAttrMap;
}

const SvXMLTokenMap& ScXMLImport::GetCondDateAttrMap()
{
    if( !pCondDateAttrMap )
    {
        static const SvXMLTokenMapEntry aCondDateAttrTokenMap[] =
        {
            { XML_NAMESPACE_CALC_EXT, XML_DATE, XML_TOK_COND_DATE_VALUE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_date) },
            { XML_NAMESPACE_CALC_EXT, XML_STYLE, XML_TOK_COND_DATE_STYLE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_style) },
            XML_TOKEN_MAP_END
        };

        pCondDateAttrMap = new SvXMLTokenMap( aCondDateAttrTokenMap );
    }

    return *pCondDateAttrMap;
}

const SvXMLTokenMap& ScXMLImport::GetConditionAttrMap()
{
    if( !pConditionAttrMap )
    {
        static const SvXMLTokenMapEntry aConditionAttrTokenMap[] =
        {
            { XML_NAMESPACE_CALC_EXT, XML_VALUE, XML_TOK_CONDITION_VALUE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_value) },
            { XML_NAMESPACE_CALC_EXT, XML_APPLY_STYLE_NAME, XML_TOK_CONDITION_APPLY_STYLE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_apply_style_name) },
            { XML_NAMESPACE_CALC_EXT, XML_BASE_CELL_ADDRESS, XML_TOK_CONDITION_BASE_CELL_ADDRESS,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_base_cell_address) },
            XML_TOKEN_MAP_END
        };

        pConditionAttrMap = new SvXMLTokenMap( aConditionAttrTokenMap );
    }

    return *pConditionAttrMap;
}

const SvXMLTokenMap& ScXMLImport::GetColorScaleTokenMap()
{
    if( !pColorScaleTokenMap )
    {
        static const SvXMLTokenMapEntry aColorScaleElemTokenMap[] =
        {
            { XML_NAMESPACE_CALC_EXT, XML_COLOR_SCALE_ENTRY, XML_TOK_COLORSCALE_COLORSCALEENTRY,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_color_scale_entry) },
            XML_TOKEN_MAP_END
        };

        pColorScaleTokenMap = new SvXMLTokenMap( aColorScaleElemTokenMap );
    }

    return *pColorScaleTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetColorScaleEntryAttrMap()
{
    if( !pColorScaleEntryAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aColorScaleAttrTokenMap[] =
        {
            { XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_TOK_COLORSCALEENTRY_TYPE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_type) },
            { XML_NAMESPACE_CALC_EXT, XML_VALUE, XML_TOK_COLORSCALEENTRY_VALUE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_value) },
            { XML_NAMESPACE_CALC_EXT, XML_COLOR, XML_TOK_COLORSCALEENTRY_COLOR,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_color) },
            XML_TOKEN_MAP_END
        };

        pColorScaleEntryAttrTokenMap = new SvXMLTokenMap( aColorScaleAttrTokenMap );
    }

    return *pColorScaleEntryAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetFormattingTokenMap()
{
    if( !pDataBarTokenMap )
    {
        static const SvXMLTokenMapEntry aDataBarElemTokenMap[] =
        {
            { XML_NAMESPACE_CALC_EXT, XML_DATA_BAR_ENTRY, XML_TOK_DATABAR_DATABARENTRY,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_data_bar_entry) },
            { XML_NAMESPACE_CALC_EXT, XML_FORMATTING_ENTRY, XML_TOK_FORMATTING_ENTRY,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_formatting_entry) },
            XML_TOKEN_MAP_END
        };

        pDataBarTokenMap = new SvXMLTokenMap( aDataBarElemTokenMap );
    }

    return *pDataBarTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataBarAttrMap()
{
    if( !pDataBarAttrMap )
    {
        static const SvXMLTokenMapEntry aDataBarAttrTokenMap[] =
        {
            { XML_NAMESPACE_CALC_EXT, XML_POSITIVE_COLOR, XML_TOK_DATABAR_POSITIVE_COLOR,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_positive_color) },
            { XML_NAMESPACE_CALC_EXT, XML_NEGATIVE_COLOR, XML_TOK_DATABAR_NEGATIVE_COLOR,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_negative_color) },
            { XML_NAMESPACE_CALC_EXT, XML_GRADIENT, XML_TOK_DATABAR_GRADIENT,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_gradient) },
            { XML_NAMESPACE_CALC_EXT, XML_AXIS_POSITION, XML_TOK_DATABAR_AXISPOSITION,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_axis_position) },
            { XML_NAMESPACE_CALC_EXT, XML_SHOW_VALUE, XML_TOK_DATABAR_SHOWVALUE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_show_value) },
            { XML_NAMESPACE_CALC_EXT, XML_AXIS_COLOR, XML_TOK_DATABAR_AXISCOLOR,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_axis_color) },
            { XML_NAMESPACE_CALC_EXT, XML_MIN_LENGTH, XML_TOK_DATABAR_MINLENGTH,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_min_length) },
            { XML_NAMESPACE_CALC_EXT, XML_MAX_LENGTH, XML_TOK_DATABAR_MAXLENGTH,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_max_length) },
            XML_TOKEN_MAP_END
        };

        pDataBarAttrMap = new SvXMLTokenMap( aDataBarAttrTokenMap );
    }

    return *pDataBarAttrMap;
}

const SvXMLTokenMap& ScXMLImport::GetIconSetAttrMap()
{
    if( !pIconSetAttrMap )
    {
        static const SvXMLTokenMapEntry aIconSetAttrTokenMap[] =
        {
            { XML_NAMESPACE_CALC_EXT, XML_ICON_SET_TYPE, XML_TOK_ICONSET_TYPE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_icon_set_type) },
            { XML_NAMESPACE_CALC_EXT, XML_SHOW_VALUE, XML_TOK_ICONSET_SHOWVALUE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_show_value) },
            XML_TOKEN_MAP_END
        };

        pIconSetAttrMap = new SvXMLTokenMap( aIconSetAttrTokenMap );
    }

    return *pIconSetAttrMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataBarEntryAttrMap()
{
    if( !pFormattingEntryAttrMap )
    {
        static const SvXMLTokenMapEntry aDataBarAttrEntryTokenMap[] =
        {
            { XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_TOK_DATABARENTRY_TYPE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_type) },
            { XML_NAMESPACE_CALC_EXT, XML_VALUE, XML_TOK_DATABARENTRY_VALUE,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_value) },
            XML_TOKEN_MAP_END
        };

        pFormattingEntryAttrMap = new SvXMLTokenMap( aDataBarAttrEntryTokenMap );
    }

    return *pFormattingEntryAttrMap;
}

const SvXMLTokenMap& ScXMLImport::GetLabelRangesElemTokenMap()
{
    if( !pLabelRangesElemTokenMap )
    {
        static const SvXMLTokenMapEntry aLabelRangesElemTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_LABEL_RANGE, XML_TOK_LABEL_RANGE_ELEM,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_label_range) },
            XML_TOKEN_MAP_END
        };

        pLabelRangesElemTokenMap = new SvXMLTokenMap( aLabelRangesElemTokenMap );
    } // if( !pLabelRangesElemTokenMap )

    return *pLabelRangesElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetLabelRangeAttrTokenMap()
{
    if( !pLabelRangeAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aLabelRangeAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_LABEL_CELL_RANGE_ADDRESS,    XML_TOK_LABEL_RANGE_ATTR_LABEL_RANGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_label_cell_range_address) },
            { XML_NAMESPACE_TABLE, XML_DATA_CELL_RANGE_ADDRESS,     XML_TOK_LABEL_RANGE_ATTR_DATA_RANGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_cell_range_address) },
            { XML_NAMESPACE_TABLE, XML_ORIENTATION,                 XML_TOK_LABEL_RANGE_ATTR_ORIENTATION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_orientation) },
            XML_TOKEN_MAP_END
        };

        pLabelRangeAttrTokenMap = new SvXMLTokenMap( aLabelRangeAttrTokenMap );
    } // if( !pLabelRangeAttrTokenMap )

    return *pLabelRangeAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableElemTokenMap()
{
    if( !pTableElemTokenMap )
    {
        static const SvXMLTokenMapEntry aTableTokenMap[] =
        {
            { XML_NAMESPACE_TABLE,  XML_NAMED_EXPRESSIONS,    XML_TOK_TABLE_NAMED_EXPRESSIONS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_expression) },
            { XML_NAMESPACE_TABLE,  XML_TABLE_COLUMN_GROUP,   XML_TOK_TABLE_COL_GROUP,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_column_group) },
            { XML_NAMESPACE_TABLE,  XML_TABLE_HEADER_COLUMNS, XML_TOK_TABLE_HEADER_COLS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_header_columns) },
            { XML_NAMESPACE_TABLE,  XML_TABLE_COLUMNS,        XML_TOK_TABLE_COLS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_columns) },
            { XML_NAMESPACE_TABLE,  XML_TABLE_COLUMN,         XML_TOK_TABLE_COL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_column) },
            { XML_NAMESPACE_TABLE,  XML_TABLE_PROTECTION,     XML_TOK_TABLE_PROTECTION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_protection) },
            { XML_NAMESPACE_LO_EXT, XML_TABLE_PROTECTION,     XML_TOK_TABLE_PROTECTION_EXT,
                (FastToken::NAMESPACE | XML_NAMESPACE_LO_EXT | XML_table_protection) },
            { XML_NAMESPACE_OFFICE_EXT, XML_TABLE_PROTECTION, XML_TOK_TABLE_PROTECTION_EXT,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE_EXT | XML_table_protection) },
            { XML_NAMESPACE_TABLE,  XML_TABLE_ROW_GROUP,      XML_TOK_TABLE_ROW_GROUP,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_row_group) },
            { XML_NAMESPACE_TABLE,  XML_TABLE_HEADER_ROWS,    XML_TOK_TABLE_HEADER_ROWS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_header_rows) },
            { XML_NAMESPACE_TABLE,  XML_TABLE_ROWS,           XML_TOK_TABLE_ROWS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_rows) },
            { XML_NAMESPACE_TABLE,  XML_TABLE_ROW,            XML_TOK_TABLE_ROW,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_row) },
            { XML_NAMESPACE_TABLE,  XML_TABLE_SOURCE,         XML_TOK_TABLE_SOURCE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_source) },
            { XML_NAMESPACE_TABLE,  XML_SCENARIO,             XML_TOK_TABLE_SCENARIO,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_scenario) },
            { XML_NAMESPACE_TABLE,  XML_SHAPES,               XML_TOK_TABLE_SHAPES,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_shapes) },
            { XML_NAMESPACE_OFFICE, XML_FORMS,                XML_TOK_TABLE_FORMS,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_forms) },
            { XML_NAMESPACE_OFFICE, XML_EVENT_LISTENERS,      XML_TOK_TABLE_EVENT_LISTENERS,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_event_listeners) },
            { XML_NAMESPACE_OFFICE_EXT, XML_EVENT_LISTENERS,  XML_TOK_TABLE_EVENT_LISTENERS_EXT,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE_EXT | XML_event_listeners) },
            { XML_NAMESPACE_CALC_EXT, XML_CONDITIONAL_FORMATS, XML_TOK_TABLE_CONDFORMATS,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_conditional_formats) },
            XML_TOKEN_MAP_END
        };

        pTableElemTokenMap = new SvXMLTokenMap( aTableTokenMap );
    } // if( !pTableElemTokenMap )

    return *pTableElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableProtectionAttrTokenMap()
{
    if (!pTableProtectionElemTokenMap)
    {
        static const SvXMLTokenMapEntry aTableProtectionTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_SELECT_PROTECTED_CELLS,      XML_TOK_TABLE_SELECT_PROTECTED_CELLS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_select_protected_cells) },
            { XML_NAMESPACE_TABLE, XML_SELECT_UNPROTECTED_CELLS,    XML_TOK_TABLE_SELECT_UNPROTECTED_CELLS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_select_unprotected_cells) },
            { XML_NAMESPACE_OFFICE_EXT, XML_SELECT_PROTECTED_CELLS, XML_TOK_TABLE_SELECT_PROTECTED_CELLS_EXT,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE_EXT | XML_select_protected_cells) },
            { XML_NAMESPACE_LO_EXT, XML_SELECT_PROTECTED_CELLS, XML_TOK_TABLE_SELECT_PROTECTED_CELLS_EXT,
                (FastToken::NAMESPACE | XML_NAMESPACE_LO_EXT | XML_select_protected_cells) },
            { XML_NAMESPACE_OFFICE_EXT, XML_SELECT_UNPROTECTED_CELLS, XML_TOK_TABLE_SELECT_UNPROTECTED_CELLS_EXT,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE_EXT | XML_select_unprotected_cells) },
            { XML_NAMESPACE_LO_EXT, XML_SELECT_UNPROTECTED_CELLS, XML_TOK_TABLE_SELECT_UNPROTECTED_CELLS_EXT,
                (FastToken::NAMESPACE | XML_NAMESPACE_LO_EXT | XML_select_unprotected_cells) },
            XML_TOKEN_MAP_END
        };
        pTableProtectionElemTokenMap = new SvXMLTokenMap(aTableProtectionTokenMap);
    }

    return *pTableProtectionElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableRowsElemTokenMap()
{
    if( !pTableRowsElemTokenMap )
    {
        static const SvXMLTokenMapEntry aTableRowsElemTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_TABLE_ROW_GROUP,     XML_TOK_TABLE_ROWS_ROW_GROUP,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_row_group) },
            { XML_NAMESPACE_TABLE, XML_TABLE_HEADER_ROWS,   XML_TOK_TABLE_ROWS_HEADER_ROWS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_header_rows) },
            { XML_NAMESPACE_TABLE, XML_TABLE_ROWS,          XML_TOK_TABLE_ROWS_ROWS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_rows) },
            { XML_NAMESPACE_TABLE, XML_TABLE_ROW,           XML_TOK_TABLE_ROWS_ROW,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_row) },
            XML_TOKEN_MAP_END
        };

        pTableRowsElemTokenMap = new SvXMLTokenMap( aTableRowsElemTokenMap );
    } // if( !pTableRowsElemTokenMap )

    return *pTableRowsElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableColsElemTokenMap()
{
    if( !pTableColsElemTokenMap )
    {
        static const SvXMLTokenMapEntry aTableColsElemTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_TABLE_COLUMN_GROUP,      XML_TOK_TABLE_COLS_COL_GROUP,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_column_group) },
            { XML_NAMESPACE_TABLE, XML_TABLE_HEADER_COLUMNS,    XML_TOK_TABLE_COLS_HEADER_COLS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_header_columns) },
            { XML_NAMESPACE_TABLE, XML_TABLE_COLUMNS,           XML_TOK_TABLE_COLS_COLS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_columns) },
            { XML_NAMESPACE_TABLE, XML_TABLE_COLUMN,            XML_TOK_TABLE_COLS_COL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_column) },
            XML_TOKEN_MAP_END
        };

        pTableColsElemTokenMap = new SvXMLTokenMap( aTableColsElemTokenMap );
    } // if( !pTableColsElemTokenMap )

    return *pTableColsElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableAttrTokenMap()
{
    if( !pTableAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aTableAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE,     XML_NAME,           XML_TOK_TABLE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_name) },
            { XML_NAMESPACE_TABLE,     XML_STYLE_NAME,     XML_TOK_TABLE_STYLE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_style_name) },
            { XML_NAMESPACE_TABLE, XML_PROTECTED,                   XML_TOK_TABLE_PROTECTED,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_protected) },
            { XML_NAMESPACE_TABLE,     XML_PRINT_RANGES,   XML_TOK_TABLE_PRINT_RANGES,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_print_ranges) },
            { XML_NAMESPACE_TABLE,     XML_PROTECTION_KEY, XML_TOK_TABLE_PASSWORD,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_protection_key) },
            { XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM, XML_TOK_TABLE_PASSHASH,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_protection_key_digest_algorithm) },
            { XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM_2, XML_TOK_TABLE_PASSHASH_2,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_protection_key_digest_algorithm_2) },
            { XML_NAMESPACE_LO_EXT, XML_PROTECTION_KEY_DIGEST_ALGORITHM_2, XML_TOK_TABLE_PASSHASH_2,
                (FastToken::NAMESPACE | XML_NAMESPACE_LO_EXT | XML_protection_key_digest_algorithm_2) },
            { XML_NAMESPACE_TABLE,     XML_PRINT,          XML_TOK_TABLE_PRINT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_print) },
            XML_TOKEN_MAP_END
        };

        pTableAttrTokenMap = new SvXMLTokenMap( aTableAttrTokenMap );
    } // if( !pTableAttrTokenMap )

    return *pTableAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableScenarioAttrTokenMap()
{
    if( !pTableScenarioAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aTableScenarioAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_DISPLAY_BORDER,      XML_TOK_TABLE_SCENARIO_ATTR_DISPLAY_BORDER,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_display_border) },
            { XML_NAMESPACE_TABLE, XML_BORDER_COLOR,        XML_TOK_TABLE_SCENARIO_ATTR_BORDER_COLOR,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_border_color) },
            { XML_NAMESPACE_TABLE, XML_COPY_BACK,           XML_TOK_TABLE_SCENARIO_ATTR_COPY_BACK,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_copy_back) },
            { XML_NAMESPACE_TABLE, XML_COPY_STYLES,         XML_TOK_TABLE_SCENARIO_ATTR_COPY_STYLES,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_copy_styles) },
            { XML_NAMESPACE_TABLE, XML_COPY_FORMULAS,       XML_TOK_TABLE_SCENARIO_ATTR_COPY_FORMULAS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_copy_formulas) },
            { XML_NAMESPACE_TABLE, XML_IS_ACTIVE,           XML_TOK_TABLE_SCENARIO_ATTR_IS_ACTIVE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_is_active) },
            { XML_NAMESPACE_TABLE, XML_SCENARIO_RANGES,     XML_TOK_TABLE_SCENARIO_ATTR_SCENARIO_RANGES,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_scenario_ranges) },
            { XML_NAMESPACE_TABLE, XML_COMMENT,             XML_TOK_TABLE_SCENARIO_ATTR_COMMENT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_comment) },
            { XML_NAMESPACE_TABLE, XML_PROTECTED,           XML_TOK_TABLE_SCENARIO_ATTR_PROTECTED,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_protected) },
            XML_TOKEN_MAP_END
        };

        pTableScenarioAttrTokenMap = new SvXMLTokenMap( aTableScenarioAttrTokenMap );
    } // if( !pTableScenarioAttrTokenMap )

    return *pTableScenarioAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableColAttrTokenMap()
{
    if( !pTableColAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aTableColAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_STYLE_NAME,                  XML_TOK_TABLE_COL_ATTR_STYLE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_style_name) },
            { XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED,     XML_TOK_TABLE_COL_ATTR_REPEATED,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_number_columns_repeated) },
            { XML_NAMESPACE_TABLE, XML_VISIBILITY,                  XML_TOK_TABLE_COL_ATTR_VISIBILITY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_visibility) },
            { XML_NAMESPACE_TABLE, XML_DEFAULT_CELL_STYLE_NAME,    XML_TOK_TABLE_COL_ATTR_DEFAULT_CELL_STYLE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_default_cell_style_name) },
            XML_TOKEN_MAP_END
        };

        pTableColAttrTokenMap = new SvXMLTokenMap( aTableColAttrTokenMap );
    } // if( !pTableColAttrTokenMap )

    return *pTableColAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableRowElemTokenMap()
{
    if( !pTableRowElemTokenMap )
    {
        static const SvXMLTokenMapEntry aTableRowTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_TABLE_CELL,      XML_TOK_TABLE_ROW_CELL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_cell) },
            { XML_NAMESPACE_TABLE, XML_COVERED_TABLE_CELL,  XML_TOK_TABLE_ROW_COVERED_CELL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_covered_table_cell) },
            XML_TOKEN_MAP_END
        };

        pTableRowElemTokenMap = new SvXMLTokenMap( aTableRowTokenMap );
    } // if( !pTableRowElemTokenMap )

    return *pTableRowElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableRowAttrTokenMap()
{
    if( !pTableRowAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aTableRowAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_STYLE_NAME,                  XML_TOK_TABLE_ROW_ATTR_STYLE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_style_name) },
            { XML_NAMESPACE_TABLE, XML_VISIBILITY,                  XML_TOK_TABLE_ROW_ATTR_VISIBILITY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_visibility) },
            { XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_REPEATED,        XML_TOK_TABLE_ROW_ATTR_REPEATED,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_number_rows_repeated) },
            { XML_NAMESPACE_TABLE, XML_DEFAULT_CELL_STYLE_NAME,     XML_TOK_TABLE_ROW_ATTR_DEFAULT_CELL_STYLE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_default_cell_style_name) },
            //  { XML_NAMESPACE_TABLE, XML_USE_OPTIMAL_HEIGHT,          XML_TOK_TABLE_ROW_ATTR_USE_OPTIMAL_HEIGHT,
            //      (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_use_optimal_height) },
            XML_TOKEN_MAP_END
        };

        pTableRowAttrTokenMap = new SvXMLTokenMap( aTableRowAttrTokenMap );
    } // if( !pTableRowAttrTokenMap )

    return *pTableRowAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableRowCellElemTokenMap()
{
    if( !pTableRowCellElemTokenMap )
    {
        static const SvXMLTokenMapEntry aTableRowCellTokenMap[] =
        {
            { XML_NAMESPACE_TEXT,   XML_P,                  XML_TOK_TABLE_ROW_CELL_P,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_p) },
            { XML_NAMESPACE_TABLE,  XML_SUB_TABLE,          XML_TOK_TABLE_ROW_CELL_TABLE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_sub_table) },
            { XML_NAMESPACE_OFFICE, XML_ANNOTATION,         XML_TOK_TABLE_ROW_CELL_ANNOTATION,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_annotation) },
            { XML_NAMESPACE_TABLE,  XML_DETECTIVE,          XML_TOK_TABLE_ROW_CELL_DETECTIVE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_detective) },
            { XML_NAMESPACE_TABLE,  XML_CELL_RANGE_SOURCE,  XML_TOK_TABLE_ROW_CELL_CELL_RANGE_SOURCE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_cell_range_source) },
            XML_TOKEN_MAP_END
        };

        pTableRowCellElemTokenMap = new SvXMLTokenMap( aTableRowCellTokenMap );
    } // if( !pTableRowCellElemTokenMap )

    return *pTableRowCellElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableAnnotationAttrTokenMap()
{
    if( !pTableAnnotationAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aTableAnnotationAttrTokenMap[] =
        {
            { XML_NAMESPACE_OFFICE, XML_AUTHOR,                 XML_TOK_TABLE_ANNOTATION_ATTR_AUTHOR,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_author) },
            { XML_NAMESPACE_OFFICE, XML_CREATE_DATE,            XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_create_date) },
            { XML_NAMESPACE_OFFICE, XML_CREATE_DATE_STRING,     XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE_STRING,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_create_date_string) },
            { XML_NAMESPACE_OFFICE, XML_DISPLAY,                XML_TOK_TABLE_ANNOTATION_ATTR_DISPLAY,
                (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_display) },
            { XML_NAMESPACE_SVG,    XML_X,                      XML_TOK_TABLE_ANNOTATION_ATTR_X,
                (FastToken::NAMESPACE | XML_NAMESPACE_SVG | XML_x) },
            { XML_NAMESPACE_SVG,    XML_Y,                      XML_TOK_TABLE_ANNOTATION_ATTR_Y,
                (FastToken::NAMESPACE | XML_NAMESPACE_SVG | XML_y) },
            XML_TOKEN_MAP_END
        };

        pTableAnnotationAttrTokenMap = new SvXMLTokenMap( aTableAnnotationAttrTokenMap );
    } // if( !pTableAnnotationAttrTokenMap )

    return *pTableAnnotationAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDetectiveElemTokenMap()
{
    if( !pDetectiveElemTokenMap )
    {
        static const SvXMLTokenMapEntry aDetectiveElemTokenMap[]=
        {
            { XML_NAMESPACE_TABLE,  XML_HIGHLIGHTED_RANGE,  XML_TOK_DETECTIVE_ELEM_HIGHLIGHTED,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_highlighted_range) },
            { XML_NAMESPACE_TABLE,  XML_OPERATION,          XML_TOK_DETECTIVE_ELEM_OPERATION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_operation) },
            XML_TOKEN_MAP_END
        };

        pDetectiveElemTokenMap = new SvXMLTokenMap( aDetectiveElemTokenMap );
    } // if( !pDetectiveElemTokenMap )

    return *pDetectiveElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDetectiveHighlightedAttrTokenMap()
{
    if( !pDetectiveHighlightedAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDetectiveHighlightedAttrTokenMap[]=
        {
            { XML_NAMESPACE_TABLE,  XML_CELL_RANGE_ADDRESS,     XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_CELL_RANGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_cell_range_address) },
            { XML_NAMESPACE_TABLE,  XML_DIRECTION,              XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_DIRECTION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_direction) },
            { XML_NAMESPACE_TABLE,  XML_CONTAINS_ERROR,         XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_CONTAINS_ERROR,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_contains_error) },
            { XML_NAMESPACE_TABLE,  XML_MARKED_INVALID,         XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_MARKED_INVALID,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_marked_invalid) },
            XML_TOKEN_MAP_END
        };

        pDetectiveHighlightedAttrTokenMap = new SvXMLTokenMap( aDetectiveHighlightedAttrTokenMap );
    } // if( !pDetectiveHighlightedAttrTokenMap )

    return *pDetectiveHighlightedAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDetectiveOperationAttrTokenMap()
{
    if( !pDetectiveOperationAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDetectiveOperationAttrTokenMap[]=
        {
            { XML_NAMESPACE_TABLE,  XML_NAME,   XML_TOK_DETECTIVE_OPERATION_ATTR_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_name) },
            { XML_NAMESPACE_TABLE,  XML_INDEX,  XML_TOK_DETECTIVE_OPERATION_ATTR_INDEX,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_index) },
            XML_TOKEN_MAP_END
        };

        pDetectiveOperationAttrTokenMap = new SvXMLTokenMap( aDetectiveOperationAttrTokenMap );
    } // if( !pDetectiveOperationAttrTokenMap )

    return *pDetectiveOperationAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetTableCellRangeSourceAttrTokenMap()
{
    if( !pTableCellRangeSourceAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aTableCellRangeSourceAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE,  XML_NAME,                   XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_name) },
            { XML_NAMESPACE_XLINK,  XML_HREF,                   XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_HREF,
                (FastToken::NAMESPACE | XML_NAMESPACE_XLINK | XML_href) },
            { XML_NAMESPACE_TABLE,  XML_FILTER_NAME,            XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_filter_name) },
            { XML_NAMESPACE_TABLE,  XML_FILTER_OPTIONS,         XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_OPTIONS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_filter_options) },
            { XML_NAMESPACE_TABLE,  XML_LAST_COLUMN_SPANNED,    XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_COLUMN,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_last_column_spanned) },
            { XML_NAMESPACE_TABLE,  XML_LAST_ROW_SPANNED,       XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_ROW,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_last_row_spanned) },
            { XML_NAMESPACE_TABLE,  XML_REFRESH_DELAY,          XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_REFRESH_DELAY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_refresh_delay) },
            XML_TOKEN_MAP_END
        };

        pTableCellRangeSourceAttrTokenMap = new SvXMLTokenMap( aTableCellRangeSourceAttrTokenMap );
    } // if( !pTableCellRangeSourceAttrTokenMap )

    return *pTableCellRangeSourceAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetNamedExpressionsElemTokenMap()
{
    if( !pNamedExpressionsElemTokenMap )
    {
        static const SvXMLTokenMapEntry aNamedExpressionsTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_NAMED_RANGE,             XML_TOK_NAMED_EXPRESSIONS_NAMED_RANGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_named_range) },
            { XML_NAMESPACE_TABLE, XML_NAMED_EXPRESSION,        XML_TOK_NAMED_EXPRESSIONS_NAMED_EXPRESSION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_named_expression) },
            XML_TOKEN_MAP_END
        };

        pNamedExpressionsElemTokenMap = new SvXMLTokenMap( aNamedExpressionsTokenMap );
    } // if( !pNamedExpressionsElemTokenMap )

    return *pNamedExpressionsElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetNamedRangeAttrTokenMap()
{
    if( !pNamedRangeAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aNamedRangeAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_NAME,                XML_TOK_NAMED_RANGE_ATTR_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_name) },
            { XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS,  XML_TOK_NAMED_RANGE_ATTR_CELL_RANGE_ADDRESS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_cell_range_address) },
            { XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS,   XML_TOK_NAMED_RANGE_ATTR_BASE_CELL_ADDRESS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_base_cell_address) },
            { XML_NAMESPACE_TABLE, XML_RANGE_USABLE_AS,     XML_TOK_NAMED_RANGE_ATTR_RANGE_USABLE_AS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_range_usable_as) },
            XML_TOKEN_MAP_END
        };

        pNamedRangeAttrTokenMap = new SvXMLTokenMap( aNamedRangeAttrTokenMap );
    } // if( !pNamedRangeAttrTokenMap )

    return *pNamedRangeAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetNamedExpressionAttrTokenMap()
{
    if( !pNamedExpressionAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aNamedExpressionAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_NAME,                XML_TOK_NAMED_EXPRESSION_ATTR_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_name) },
            { XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS,   XML_TOK_NAMED_EXPRESSION_ATTR_BASE_CELL_ADDRESS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_base_cell_address) },
            { XML_NAMESPACE_TABLE, XML_EXPRESSION,          XML_TOK_NAMED_EXPRESSION_ATTR_EXPRESSION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_expression) },
            XML_TOKEN_MAP_END
        };

        pNamedExpressionAttrTokenMap = new SvXMLTokenMap( aNamedExpressionAttrTokenMap );
    } // if( !pNamedExpressionAttrTokenMap )

    return *pNamedExpressionAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangesElemTokenMap()
{
    if( !pDatabaseRangesElemTokenMap )
    {
        static const SvXMLTokenMapEntry aDatabaseRangesTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_DATABASE_RANGE,  XML_TOK_DATABASE_RANGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_database_range) },
            XML_TOKEN_MAP_END
        };

        pDatabaseRangesElemTokenMap = new SvXMLTokenMap( aDatabaseRangesTokenMap );
    } // if( !pDatabaseRangesElemTokenMap )

    return *pDatabaseRangesElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeElemTokenMap()
{
    if( !pDatabaseRangeElemTokenMap )
    {
        static const SvXMLTokenMapEntry aDatabaseRangeTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_SQL,     XML_TOK_DATABASE_RANGE_SOURCE_SQL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_database_source_sql) },
            { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_TABLE,   XML_TOK_DATABASE_RANGE_SOURCE_TABLE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_database_source_table) },
            { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_QUERY,   XML_TOK_DATABASE_RANGE_SOURCE_QUERY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_database_source_query) },
            { XML_NAMESPACE_TABLE, XML_FILTER,                  XML_TOK_FILTER,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_filter) },
            { XML_NAMESPACE_TABLE, XML_SORT,                    XML_TOK_SORT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_sort) },
            { XML_NAMESPACE_TABLE, XML_SUBTOTAL_RULES,          XML_TOK_DATABASE_RANGE_SUBTOTAL_RULES,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_subtotal_rules) },
            XML_TOKEN_MAP_END
        };

        pDatabaseRangeElemTokenMap = new SvXMLTokenMap( aDatabaseRangeTokenMap );
    } // if( !pDatabaseRangeElemTokenMap )

    return *pDatabaseRangeElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeAttrTokenMap()
{
    if( !pDatabaseRangeAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDatabaseRangeAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_NAME,                    XML_TOK_DATABASE_RANGE_ATTR_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_name) },
            { XML_NAMESPACE_TABLE, XML_IS_SELECTION,            XML_TOK_DATABASE_RANGE_ATTR_IS_SELECTION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_is_selection) },
            { XML_NAMESPACE_TABLE, XML_ON_UPDATE_KEEP_STYLES,   XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_STYLES,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_on_update_keep_styles) },
            { XML_NAMESPACE_TABLE, XML_ON_UPDATE_KEEP_SIZE,     XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_SIZE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_on_update_keep_size) },
            { XML_NAMESPACE_TABLE, XML_HAS_PERSISTENT_DATA,     XML_TOK_DATABASE_RANGE_ATTR_HAS_PERSISTENT_DATA,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_has_persistent_data) },
            { XML_NAMESPACE_TABLE, XML_ORIENTATION,         XML_TOK_DATABASE_RANGE_ATTR_ORIENTATION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_orientation) },
            { XML_NAMESPACE_TABLE, XML_CONTAINS_HEADER,     XML_TOK_DATABASE_RANGE_ATTR_CONTAINS_HEADER,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_contains_header) },
            { XML_NAMESPACE_TABLE, XML_DISPLAY_FILTER_BUTTONS,  XML_TOK_DATABASE_RANGE_ATTR_DISPLAY_FILTER_BUTTONS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_display_filter_buttons) },
            { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS,    XML_TOK_DATABASE_RANGE_ATTR_TARGET_RANGE_ADDRESS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_target_range_address) },
            { XML_NAMESPACE_TABLE, XML_REFRESH_DELAY,           XML_TOK_DATABASE_RANGE_ATTR_REFRESH_DELAY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_refresh_delay) },
            XML_TOKEN_MAP_END
        };

        pDatabaseRangeAttrTokenMap = new SvXMLTokenMap( aDatabaseRangeAttrTokenMap );
    } // if( !pDatabaseRangeAttrTokenMap )

    return *pDatabaseRangeAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeSourceSQLAttrTokenMap()
{
    if( !pDatabaseRangeSourceSQLAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDatabaseRangeSourceSQLAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_DATABASE_NAME,           XML_TOK_SOURCE_SQL_ATTR_DATABASE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_database_name) },
            { XML_NAMESPACE_XLINK, XML_HREF,                    XML_TOK_SOURCE_SQL_ATTR_HREF,
                (FastToken::NAMESPACE | XML_NAMESPACE_XLINK | XML_href) },
            { XML_NAMESPACE_TABLE, XML_CONNECTION_RESOURCE,     XML_TOK_SOURCE_SQL_ATTR_CONNECTION_RESOURCE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_connection_resource) },
            { XML_NAMESPACE_TABLE, XML_SQL_STATEMENT,           XML_TOK_SOURCE_SQL_ATTR_SQL_STATEMENT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_sql_statement) },
            { XML_NAMESPACE_TABLE, XML_PARSE_SQL_STATEMENT, XML_TOK_SOURCE_SQL_ATTR_PARSE_SQL_STATEMENT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_parse_sql_statement) },
            XML_TOKEN_MAP_END
        };

        pDatabaseRangeSourceSQLAttrTokenMap = new SvXMLTokenMap( aDatabaseRangeSourceSQLAttrTokenMap );
    } // if( !pDatabaseRangeSourceSQLAttrTokenMap )

    return *pDatabaseRangeSourceSQLAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeSourceTableAttrTokenMap()
{
    if( !pDatabaseRangeSourceTableAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDatabaseRangeSourceTableAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_DATABASE_NAME,           XML_TOK_SOURCE_TABLE_ATTR_DATABASE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_database_name) },
            { XML_NAMESPACE_XLINK, XML_HREF,                    XML_TOK_SOURCE_TABLE_ATTR_HREF,
                (FastToken::NAMESPACE | XML_NAMESPACE_XLINK | XML_href) },
            { XML_NAMESPACE_TABLE, XML_CONNECTION_RESOURCE,     XML_TOK_SOURCE_TABLE_ATTR_CONNECTION_RESOURCE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_connection_resource) },
            { XML_NAMESPACE_TABLE, XML_TABLE_NAME,              XML_TOK_SOURCE_TABLE_ATTR_TABLE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_table_name) },
            { XML_NAMESPACE_TABLE, XML_DATABASE_TABLE_NAME,     XML_TOK_SOURCE_TABLE_ATTR_TABLE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_database_table_name) }
            XML_TOKEN_MAP_END
        };

        pDatabaseRangeSourceTableAttrTokenMap = new SvXMLTokenMap( aDatabaseRangeSourceTableAttrTokenMap );
    } // if( !pDatabaseRangeSourceTableAttrTokenMap )

    return *pDatabaseRangeSourceTableAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeSourceQueryAttrTokenMap()
{
    if( !pDatabaseRangeSourceQueryAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDatabaseRangeSourceQueryAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_DATABASE_NAME,           XML_TOK_SOURCE_QUERY_ATTR_DATABASE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_database_name) },
            { XML_NAMESPACE_XLINK, XML_HREF,                    XML_TOK_SOURCE_QUERY_ATTR_HREF,
                (FastToken::NAMESPACE | XML_NAMESPACE_XLINK | XML_href) },
            { XML_NAMESPACE_TABLE, XML_CONNECTION_RESOURCE,     XML_TOK_SOURCE_QUERY_ATTR_CONNECTION_RESOURCE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_connection_resource) },
            { XML_NAMESPACE_TABLE, XML_QUERY_NAME,              XML_TOK_SOURCE_QUERY_ATTR_QUERY_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_query_name) },
            XML_TOKEN_MAP_END
        };

        pDatabaseRangeSourceQueryAttrTokenMap = new SvXMLTokenMap( aDatabaseRangeSourceQueryAttrTokenMap );
    } // if( !pDatabaseRangeSourceQueryAttrTokenMap )

    return *pDatabaseRangeSourceQueryAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetFilterElemTokenMap()
{
    if( !pFilterElemTokenMap )
    {
        static const SvXMLTokenMapEntry aFilterTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_FILTER_AND,          XML_TOK_FILTER_AND,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_filter_and) },
            { XML_NAMESPACE_TABLE, XML_FILTER_OR,           XML_TOK_FILTER_OR,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_filter_or) },
            { XML_NAMESPACE_TABLE, XML_FILTER_CONDITION,    XML_TOK_FILTER_CONDITION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_filter_condition) },
            XML_TOKEN_MAP_END
        };

        pFilterElemTokenMap = new SvXMLTokenMap( aFilterTokenMap );
    } // if( !pFilterElemTokenMap )

    return *pFilterElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetFilterAttrTokenMap()
{
    if( !pFilterAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aFilterAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS,            XML_TOK_FILTER_ATTR_TARGET_RANGE_ADDRESS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_target_range_address) },
            { XML_NAMESPACE_TABLE, XML_CONDITION_SOURCE_RANGE_ADDRESS,  XML_TOK_FILTER_ATTR_CONDITION_SOURCE_RANGE_ADDRESS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_condition_source_range_address) },
            { XML_NAMESPACE_TABLE, XML_CONDITION_SOURCE,                XML_TOK_FILTER_ATTR_CONDITION_SOURCE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_condition_source) },
            { XML_NAMESPACE_TABLE, XML_DISPLAY_DUPLICATES,              XML_TOK_FILTER_ATTR_DISPLAY_DUPLICATES,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_display_duplicates) },
            XML_TOKEN_MAP_END
        };

        pFilterAttrTokenMap = new SvXMLTokenMap( aFilterAttrTokenMap );
    } // if( !pFilterAttrTokenMap )

    return *pFilterAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetFilterConditionElemTokenMap()
{
    if( !pFilterConditionElemTokenMap )
    {
        static const SvXMLTokenMapEntry aTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_FILTER_SET_ITEM, XML_TOK_CONDITION_FILTER_SET_ITEM,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_filter_set_item) },
            XML_TOKEN_MAP_END
        };

        pFilterConditionElemTokenMap = new SvXMLTokenMap( aTokenMap );
    }

    return *pFilterConditionElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetFilterConditionAttrTokenMap()
{
    if( !pFilterConditionAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aFilterConditionAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_FIELD_NUMBER,    XML_TOK_CONDITION_ATTR_FIELD_NUMBER,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_field_number) },
            { XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE,  XML_TOK_CONDITION_ATTR_CASE_SENSITIVE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_case_sensitive) },
            { XML_NAMESPACE_TABLE, XML_DATA_TYPE,       XML_TOK_CONDITION_ATTR_DATA_TYPE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_type) },
            { XML_NAMESPACE_TABLE, XML_VALUE,           XML_TOK_CONDITION_ATTR_VALUE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_value) },
            { XML_NAMESPACE_TABLE, XML_OPERATOR,        XML_TOK_CONDITION_ATTR_OPERATOR,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_operator) },
            XML_TOKEN_MAP_END
        };

        pFilterConditionAttrTokenMap = new SvXMLTokenMap( aFilterConditionAttrTokenMap );
    } // if( !pFilterConditionAttrTokenMap )

    return *pFilterConditionAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetFilterSetItemAttrTokenMap()
{
    if( !pFilterSetItemAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_VALUE, XML_TOK_FILTER_SET_ITEM_ATTR_VALUE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_value) },
            XML_TOKEN_MAP_END
        };

        pFilterSetItemAttrTokenMap = new SvXMLTokenMap( aTokenMap );
    }

    return *pFilterSetItemAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSortElemTokenMap()
{
    if( !pSortElemTokenMap )
    {
        static const SvXMLTokenMapEntry aSortTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_SORT_BY, XML_TOK_SORT_SORT_BY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_sort_by) },
            XML_TOKEN_MAP_END
        };

        pSortElemTokenMap = new SvXMLTokenMap( aSortTokenMap );
    } // if( !pSortElemTokenMap )

    return *pSortElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSortAttrTokenMap()
{
    if( !pSortAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aSortAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_BIND_STYLES_TO_CONTENT,  XML_TOK_SORT_ATTR_BIND_STYLES_TO_CONTENT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_bind_styles_to_content) },
            { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS,    XML_TOK_SORT_ATTR_TARGET_RANGE_ADDRESS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_target_range_address) },
            { XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE,          XML_TOK_SORT_ATTR_CASE_SENSITIVE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_case_sensitive) },
            { XML_NAMESPACE_TABLE, XML_RFC_LANGUAGE_TAG,        XML_TOK_SORT_ATTR_RFC_LANGUAGE_TAG,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_rfc_language_tag) },
            { XML_NAMESPACE_TABLE, XML_LANGUAGE,                XML_TOK_SORT_ATTR_LANGUAGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_language) },
            { XML_NAMESPACE_TABLE, XML_SCRIPT,                  XML_TOK_SORT_ATTR_SCRIPT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_script) },
            { XML_NAMESPACE_TABLE, XML_COUNTRY,                 XML_TOK_SORT_ATTR_COUNTRY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_country) },
            { XML_NAMESPACE_TABLE, XML_ALGORITHM,               XML_TOK_SORT_ATTR_ALGORITHM,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_algorithm) },
            XML_TOKEN_MAP_END
        };

        pSortAttrTokenMap = new SvXMLTokenMap( aSortAttrTokenMap );
    } // if( !pSortAttrTokenMap )

    return *pSortAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSortSortByAttrTokenMap()
{
    if( !pSortSortByAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aSortSortByAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_FIELD_NUMBER,    XML_TOK_SORT_BY_ATTR_FIELD_NUMBER,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_field_number) },
            { XML_NAMESPACE_TABLE, XML_DATA_TYPE,       XML_TOK_SORT_BY_ATTR_DATA_TYPE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_type) },
            { XML_NAMESPACE_TABLE, XML_ORDER,           XML_TOK_SORT_BY_ATTR_ORDER,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_order) },
            XML_TOKEN_MAP_END
        };

        pSortSortByAttrTokenMap = new SvXMLTokenMap( aSortSortByAttrTokenMap );
    } // if( !pSortSortByAttrTokenMap )

    return *pSortSortByAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeSubTotalRulesElemTokenMap()
{
    if( !pDatabaseRangeSubTotalRulesElemTokenMap )
    {
        static const SvXMLTokenMapEntry aDatabaseRangeSubTotalRulesTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_SORT_GROUPS,     XML_TOK_SUBTOTAL_RULES_SORT_GROUPS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_sort_groups) },
            { XML_NAMESPACE_TABLE, XML_SUBTOTAL_RULE,   XML_TOK_SUBTOTAL_RULES_SUBTOTAL_RULE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_subtotal_rule) },
            XML_TOKEN_MAP_END
        };

        pDatabaseRangeSubTotalRulesElemTokenMap = new SvXMLTokenMap( aDatabaseRangeSubTotalRulesTokenMap );
    } // if( !pDatabaseRangeSubTotalRulesElemTokenMap )

    return *pDatabaseRangeSubTotalRulesElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDatabaseRangeSubTotalRulesAttrTokenMap()
{
    if( !pDatabaseRangeSubTotalRulesAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDatabaseRangeSubTotalRulesAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_BIND_STYLES_TO_CONTENT,          XML_TOK_SUBTOTAL_RULES_ATTR_BIND_STYLES_TO_CONTENT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_bind_styles_to_content) },
            { XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE,                  XML_TOK_SUBTOTAL_RULES_ATTR_CASE_SENSITIVE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_case_sensitive) },
            { XML_NAMESPACE_TABLE, XML_PAGE_BREAKS_ON_GROUP_CHANGE, XML_TOK_SUBTOTAL_RULES_ATTR_PAGE_BREAKS_ON_GROUP_CHANGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_page_breaks_on_group_change) },
            XML_TOKEN_MAP_END
        };

        pDatabaseRangeSubTotalRulesAttrTokenMap = new SvXMLTokenMap( aDatabaseRangeSubTotalRulesAttrTokenMap );
    } // if( !pDatabaseRangeSubTotalRulesAttrTokenMap )

    return *pDatabaseRangeSubTotalRulesAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSubTotalRulesSortGroupsAttrTokenMap()
{
    if( !pSubTotalRulesSortGroupsAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aSubTotalRulesSortGroupsAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_DATA_TYPE,   XML_TOK_SORT_GROUPS_ATTR_DATA_TYPE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_type) },
            { XML_NAMESPACE_TABLE, XML_ORDER,       XML_TOK_SORT_GROUPS_ATTR_ORDER,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_order) },
            XML_TOKEN_MAP_END
        };

        pSubTotalRulesSortGroupsAttrTokenMap = new SvXMLTokenMap( aSubTotalRulesSortGroupsAttrTokenMap );
    } // if( !pSubTotalRulesSortGroupsAttrTokenMap )

    return *pSubTotalRulesSortGroupsAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSubTotalRulesSubTotalRuleElemTokenMap()
{
    if( !pSubTotalRulesSubTotalRuleElemTokenMap )
    {
        static const SvXMLTokenMapEntry aSubTotalRulesSubTotalRuleTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_SUBTOTAL_FIELD,  XML_TOK_SUBTOTAL_RULE_SUBTOTAL_FIELD,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_subtotal_field) },
            XML_TOKEN_MAP_END
        };

        pSubTotalRulesSubTotalRuleElemTokenMap = new SvXMLTokenMap( aSubTotalRulesSubTotalRuleTokenMap );
    } // if( !pSubTotalRulesSubTotalRuleElemTokenMap )

    return *pSubTotalRulesSubTotalRuleElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSubTotalRulesSubTotalRuleAttrTokenMap()
{
    if( !pSubTotalRulesSubTotalRuleAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aSubTotalRulesSubTotalRuleAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_GROUP_BY_FIELD_NUMBER,   XML_TOK_SUBTOTAL_RULE_ATTR_GROUP_BY_FIELD_NUMBER,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_group_by_field_number) },
            XML_TOKEN_MAP_END
        };

        pSubTotalRulesSubTotalRuleAttrTokenMap = new SvXMLTokenMap( aSubTotalRulesSubTotalRuleAttrTokenMap );
    } // if( !pSubTotalRulesSubTotalRuleAttrTokenMap )

    return *pSubTotalRulesSubTotalRuleAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetSubTotalRuleSubTotalFieldAttrTokenMap()
{
    if( !pSubTotalRuleSubTotalFieldAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aSubTotalRuleSubTotalFieldAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_FIELD_NUMBER,    XML_TOK_SUBTOTAL_FIELD_ATTR_FIELD_NUMBER,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_field_number) },
            { XML_NAMESPACE_TABLE, XML_FUNCTION,        XML_TOK_SUBTOTAL_FIELD_ATTR_FUNCTION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_function) },
            XML_TOKEN_MAP_END
        };

        pSubTotalRuleSubTotalFieldAttrTokenMap = new SvXMLTokenMap( aSubTotalRuleSubTotalFieldAttrTokenMap );
    } // if( !pSubTotalRuleSubTotalFieldAttrTokenMap )

    return *pSubTotalRuleSubTotalFieldAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotTablesElemTokenMap()
{
    if( !pDataPilotTablesElemTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotTablesElemTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLE,    XML_TOK_DATA_PILOT_TABLE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_table) },
            XML_TOKEN_MAP_END
        };

        pDataPilotTablesElemTokenMap = new SvXMLTokenMap( aDataPilotTablesElemTokenMap );
    } // if( !pDataPilotTablesElemTokenMap )

    return *pDataPilotTablesElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotTableAttrTokenMap()
{
    if( !pDataPilotTableAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotTableAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_NAME,                    XML_TOK_DATA_PILOT_TABLE_ATTR_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_name) },
            { XML_NAMESPACE_TABLE, XML_APPLICATION_DATA,        XML_TOK_DATA_PILOT_TABLE_ATTR_APPLICATION_DATA,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_application_data) },
            { XML_NAMESPACE_TABLE, XML_GRAND_TOTAL,             XML_TOK_DATA_PILOT_TABLE_ATTR_GRAND_TOTAL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_grand_total) },
            { XML_NAMESPACE_TABLE, XML_IGNORE_EMPTY_ROWS,       XML_TOK_DATA_PILOT_TABLE_ATTR_IGNORE_EMPTY_ROWS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_ignore_empty_rows) },
            { XML_NAMESPACE_TABLE, XML_IDENTIFY_CATEGORIES,     XML_TOK_DATA_PILOT_TABLE_ATTR_IDENTIFY_CATEGORIES,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_identify_categories) },
            { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS,    XML_TOK_DATA_PILOT_TABLE_ATTR_TARGET_RANGE_ADDRESS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_target_range_address) },
            { XML_NAMESPACE_TABLE, XML_BUTTONS,                 XML_TOK_DATA_PILOT_TABLE_ATTR_BUTTONS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_buttons) },
            { XML_NAMESPACE_TABLE, XML_SHOW_FILTER_BUTTON,      XML_TOK_DATA_PILOT_TABLE_ATTR_SHOW_FILTER_BUTTON,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_show_filter_button) },
            { XML_NAMESPACE_TABLE, XML_DRILL_DOWN_ON_DOUBLE_CLICK, XML_TOK_DATA_PILOT_TABLE_ATTR_DRILL_DOWN,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_drill_down_on_double_click) },
            { XML_NAMESPACE_TABLE, XML_HEADER_GRID_LAYOUT,      XML_TOK_DATA_PILOT_TABLE_ATTR_HEADER_GRID_LAYOUT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_header_grid_layout) },
            XML_TOKEN_MAP_END
        };

        pDataPilotTableAttrTokenMap = new SvXMLTokenMap( aDataPilotTableAttrTokenMap );
    } // if( !pDataPilotTableAttrTokenMap )

    return *pDataPilotTableAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotTableElemTokenMap()
{
    if( !pDataPilotTableElemTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotTableElemTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_SQL, XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SQL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_database_source_sql) },
            { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_TABLE,   XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_TABLE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_database_source_table) },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_GRAND_TOTAL,  XML_TOK_DATA_PILOT_TABLE_ELEM_GRAND_TOTAL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_grand_total) },
            { XML_NAMESPACE_TABLE_EXT, XML_DATA_PILOT_GRAND_TOTAL, XML_TOK_DATA_PILOT_TABLE_ELEM_GRAND_TOTAL_EXT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE_EXT | XML_data_pilot_grand_total) },
            { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_QUERY,   XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_QUERY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_database_source_query) },
            { XML_NAMESPACE_TABLE, XML_SOURCE_SERVICE,          XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SERVICE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_source_service) },
            { XML_NAMESPACE_TABLE, XML_SOURCE_CELL_RANGE,       XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_CELL_RANGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_source_cell_range) },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_FIELD,        XML_TOK_DATA_PILOT_TABLE_ELEM_DATA_PILOT_FIELD,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_field) },
            XML_TOKEN_MAP_END
        };

        pDataPilotTableElemTokenMap = new SvXMLTokenMap( aDataPilotTableElemTokenMap );
    } // if( !pDataPilotTableElemTokenMap )

    return *pDataPilotTableElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotTableSourceServiceAttrTokenMap()
{
    if( !pDataPilotTableSourceServiceAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotTableSourceServiceAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_NAME,                    XML_TOK_SOURCE_SERVICE_ATTR_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_name) },
            { XML_NAMESPACE_TABLE, XML_SOURCE_NAME,             XML_TOK_SOURCE_SERVICE_ATTR_SOURCE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_source_name) },
            { XML_NAMESPACE_TABLE, XML_OBJECT_NAME,             XML_TOK_SOURCE_SERVICE_ATTR_OBJECT_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_object_name) },
            { XML_NAMESPACE_TABLE, XML_USER_NAME,               XML_TOK_SOURCE_SERVICE_ATTR_USER_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_user_name) },
            { XML_NAMESPACE_TABLE, XML_PASSWORD,                XML_TOK_SOURCE_SERVICE_ATTR_PASSWORD,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_password) },
            XML_TOKEN_MAP_END
        };

        pDataPilotTableSourceServiceAttrTokenMap = new SvXMLTokenMap( aDataPilotTableSourceServiceAttrTokenMap );
    } // if( !pDataPilotTableSourceServiceAttrTokenMap )

    return *pDataPilotTableSourceServiceAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotGrandTotalAttrTokenMap()
{
    if (!pDataPilotGrandTotalAttrTokenMap)
    {
        static const SvXMLTokenMapEntry aDataPilotGrandTotalAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE,     XML_DISPLAY,      XML_TOK_DATA_PILOT_GRAND_TOTAL_ATTR_DISPLAY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_display) },
            { XML_NAMESPACE_TABLE,     XML_ORIENTATION,  XML_TOK_DATA_PILOT_GRAND_TOTAL_ATTR_ORIENTATION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_orientation) },
            { XML_NAMESPACE_TABLE,     XML_DISPLAY_NAME, XML_TOK_DATA_PILOT_GRAND_TOTAL_ATTR_DISPLAY_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_display_name) },
            { XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, XML_TOK_DATA_PILOT_GRAND_TOTAL_ATTR_DISPLAY_NAME_EXT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE_EXT | XML_display_name) },
            XML_TOKEN_MAP_END
        };

        pDataPilotGrandTotalAttrTokenMap = new SvXMLTokenMap( aDataPilotGrandTotalAttrTokenMap );
    }

    return *pDataPilotGrandTotalAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotTableSourceCellRangeAttrTokenMap()
{
    if( !pDataPilotTableSourceCellRangeAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotTableSourceCellRangeAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, XML_TOK_SOURCE_CELL_RANGE_ATTR_CELL_RANGE_ADDRESS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_cell_range_address) },
            { XML_NAMESPACE_TABLE, XML_NAME, XML_TOK_SOURCE_CELL_RANGE_ATTR_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_name) },
            XML_TOKEN_MAP_END
        };

        pDataPilotTableSourceCellRangeAttrTokenMap = new SvXMLTokenMap( aDataPilotTableSourceCellRangeAttrTokenMap );
    } // if( !pDataPilotTableSourceCellRangeAttrTokenMap )

    return *pDataPilotTableSourceCellRangeAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotTableSourceCellRangeElemTokenMap()
{
    if( !pDataPilotTableSourceCellRangeElemTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotTableSourceCellRangeElemTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_FILTER,      XML_TOK_SOURCE_CELL_RANGE_ELEM_FILTER,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_filter) },
            XML_TOKEN_MAP_END
        };

        pDataPilotTableSourceCellRangeElemTokenMap = new SvXMLTokenMap( aDataPilotTableSourceCellRangeElemTokenMap );
    } // if( !pDataPilotTableSourceCellRangeElemTokenMap )

    return *pDataPilotTableSourceCellRangeElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotFieldAttrTokenMap()
{
    if( !pDataPilotFieldAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotFieldAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE,     XML_SOURCE_FIELD_NAME,    XML_TOK_DATA_PILOT_FIELD_ATTR_SOURCE_FIELD_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_source_field_name) },
            { XML_NAMESPACE_TABLE,     XML_DISPLAY_NAME,         XML_TOK_DATA_PILOT_FIELD_ATTR_DISPLAY_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_display_name) },
            { XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME,         XML_TOK_DATA_PILOT_FIELD_ATTR_DISPLAY_NAME_EXT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE_EXT | XML_display_name) },
            { XML_NAMESPACE_TABLE,     XML_IS_DATA_LAYOUT_FIELD, XML_TOK_DATA_PILOT_FIELD_ATTR_IS_DATA_LAYOUT_FIELD,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_is_data_layout_field) },
            { XML_NAMESPACE_TABLE,     XML_FUNCTION,             XML_TOK_DATA_PILOT_FIELD_ATTR_FUNCTION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_function) },
            { XML_NAMESPACE_TABLE,     XML_ORIENTATION,          XML_TOK_DATA_PILOT_FIELD_ATTR_ORIENTATION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_orientation) },
            { XML_NAMESPACE_TABLE,     XML_SELECTED_PAGE,        XML_TOK_DATA_PILOT_FIELD_ATTR_SELECTED_PAGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_selected_page) },
            { XML_NAMESPACE_LO_EXT,    XML_IGNORE_SELECTED_PAGE, XML_TOK_DATA_PILOT_FIELD_ATTR_IGNORE_SELECTED_PAGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_LO_EXT | XML_ignore_selected_page) },
            { XML_NAMESPACE_TABLE,     XML_USED_HIERARCHY,       XML_TOK_DATA_PILOT_FIELD_ATTR_USED_HIERARCHY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_used_hierarchy) },
            XML_TOKEN_MAP_END
        };

        pDataPilotFieldAttrTokenMap = new SvXMLTokenMap( aDataPilotFieldAttrTokenMap );
    } // if( !pDataPilotFieldAttrTokenMap )

    return *pDataPilotFieldAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotFieldElemTokenMap()
{
    if( !pDataPilotFieldElemTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotFieldElemTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_LEVEL,        XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_LEVEL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_level) },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_FIELD_REFERENCE, XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_REFERENCE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_field_reference) },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_GROUPS,       XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_GROUPS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_groups) },
            XML_TOKEN_MAP_END
        };

        pDataPilotFieldElemTokenMap = new SvXMLTokenMap( aDataPilotFieldElemTokenMap );
    } // if( !pDataPilotFieldElemTokenMap )

    return *pDataPilotFieldElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotLevelAttrTokenMap()
{
    if( !pDataPilotLevelAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotLevelAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_SHOW_EMPTY,              XML_TOK_DATA_PILOT_LEVEL_ATTR_SHOW_EMPTY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_show_empty) },
            { XML_NAMESPACE_CLAC_EXT, XML_REPEAT_ITEM_LABELS, XML_TOK_DATA_PILOT_LEVEL_ATTR_REPEAT_ITEM_LABELS,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_repeat_item_labels) },
            XML_TOKEN_MAP_END
        };

        pDataPilotLevelAttrTokenMap = new SvXMLTokenMap( aDataPilotLevelAttrTokenMap );
    } // if( !pDataPilotLevelAttrTokenMap )

    return *pDataPilotLevelAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotLevelElemTokenMap()
{
    if( !pDataPilotLevelElemTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotLevelElemTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTALS,    XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_SUBTOTALS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_subtotals) },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBERS,      XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_MEMBERS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_members) },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_DISPLAY_INFO, XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_DISPLAY_INFO,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_display_info) },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_SORT_INFO,    XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_SORT_INFO,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_sort_info) },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_LAYOUT_INFO,  XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_LAYOUT_INFO,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_layout_info) },
            XML_TOKEN_MAP_END
        };

        pDataPilotLevelElemTokenMap = new SvXMLTokenMap( aDataPilotLevelElemTokenMap );
    } // if( !pDataPilotLevelElemTokenMap )

    return *pDataPilotLevelElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotSubTotalsElemTokenMap()
{
    if( !pDataPilotSubTotalsElemTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotSubTotalsElemTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTAL, XML_TOK_DATA_PILOT_SUBTOTALS_ELEM_DATA_PILOT_SUBTOTAL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_subtotal) },
            XML_TOKEN_MAP_END
        };

        pDataPilotSubTotalsElemTokenMap = new SvXMLTokenMap( aDataPilotSubTotalsElemTokenMap );
    } // if( !pDataPilotSubTotalsElemTokenMap )

    return *pDataPilotSubTotalsElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotSubTotalAttrTokenMap()
{
    if( !pDataPilotSubTotalAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotSubTotalAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE,     XML_FUNCTION,     XML_TOK_DATA_PILOT_SUBTOTAL_ATTR_FUNCTION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_function) },
            { XML_NAMESPACE_TABLE,     XML_DISPLAY_NAME, XML_TOK_DATA_PILOT_SUBTOTAL_ATTR_DISPLAY_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_display_name) },
            { XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, XML_TOK_DATA_PILOT_SUBTOTAL_ATTR_DISPLAY_NAME_EXT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE_EXT | XML_display_name) },
            XML_TOKEN_MAP_END
        };

        pDataPilotSubTotalAttrTokenMap = new SvXMLTokenMap( aDataPilotSubTotalAttrTokenMap );
    } // if( !pDataPilotSubTotalAttrTokenMap )

    return *pDataPilotSubTotalAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotMembersElemTokenMap()
{
    if( !pDataPilotMembersElemTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotMembersElemTokenMap[] =
        {
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBER,       XML_TOK_DATA_PILOT_MEMBERS_ELEM_DATA_PILOT_MEMBER,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_data_pilot_member) },
            XML_TOKEN_MAP_END
        };

        pDataPilotMembersElemTokenMap = new SvXMLTokenMap( aDataPilotMembersElemTokenMap );
    } // if( !pDataPilotMembersElemTokenMap )

    return *pDataPilotMembersElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataPilotMemberAttrTokenMap()
{
    if( !pDataPilotMemberAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aDataPilotMemberAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE,     XML_NAME,         XML_TOK_DATA_PILOT_MEMBER_ATTR_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_name) },
            { XML_NAMESPACE_TABLE,     XML_DISPLAY_NAME, XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_display_name) },
            { XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY_NAME_EXT,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE_EXT | XML_display_name) },
            { XML_NAMESPACE_TABLE,     XML_DISPLAY,      XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_display) },
            { XML_NAMESPACE_TABLE,     XML_SHOW_DETAILS, XML_TOK_DATA_PILOT_MEMBER_ATTR_SHOW_DETAILS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_show_details) },
            XML_TOKEN_MAP_END
        };

        pDataPilotMemberAttrTokenMap = new SvXMLTokenMap( aDataPilotMemberAttrTokenMap );
    } // if( !pDataPilotMemberAttrTokenMap )

    return *pDataPilotMemberAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetConsolidationAttrTokenMap()
{
    if( !pConsolidationAttrTokenMap )
    {
        static const SvXMLTokenMapEntry aConsolidationAttrTokenMap[] =
        {
            { XML_NAMESPACE_TABLE,  XML_FUNCTION,                       XML_TOK_CONSOLIDATION_ATTR_FUNCTION,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_function) },
            { XML_NAMESPACE_TABLE,  XML_SOURCE_CELL_RANGE_ADDRESSES,    XML_TOK_CONSOLIDATION_ATTR_SOURCE_RANGES,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_source_cell_range_addresses) },
            { XML_NAMESPACE_TABLE,  XML_TARGET_CELL_ADDRESS,            XML_TOK_CONSOLIDATION_ATTR_TARGET_ADDRESS,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_target_cell_address) },
            { XML_NAMESPACE_TABLE,  XML_USE_LABEL,                      XML_TOK_CONSOLIDATION_ATTR_USE_LABEL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_use_label) },
            { XML_NAMESPACE_TABLE,  XML_LINK_TO_SOURCE_DATA,            XML_TOK_CONSOLIDATION_ATTR_LINK_TO_SOURCE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_link_to_source_data) },
            XML_TOKEN_MAP_END
        };

        pConsolidationAttrTokenMap = new SvXMLTokenMap( aConsolidationAttrTokenMap );
    } // if( !pConsolidationAttrTokenMap )

    return *pConsolidationAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetCellTextParaElemTokenMap()
{
    if (!pCellTextParaElemTokenMap)
    {
        static const SvXMLTokenMapEntry aMap[] =
        {
            { XML_NAMESPACE_TEXT, XML_S, XML_TOK_CELL_TEXT_S,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_s) },
            { XML_NAMESPACE_TEXT, XML_SPAN, XML_TOK_CELL_TEXT_SPAN,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_span) },
            { XML_NAMESPACE_TEXT, XML_SHEET_NAME, XML_TOK_CELL_TEXT_SHEET_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_sheet_name) },
            { XML_NAMESPACE_TEXT, XML_DATE, XML_TOK_CELL_TEXT_DATE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_date) },
            { XML_NAMESPACE_TEXT, XML_TITLE, XML_TOK_CELL_TEXT_TITLE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_title) },
            { XML_NAMESPACE_TEXT, XML_A, XML_TOK_CELL_TEXT_URL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_a) },
            XML_TOKEN_MAP_END
        };

        pCellTextParaElemTokenMap = new SvXMLTokenMap(aMap);
    }
    return *pCellTextParaElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetCellTextSpanElemTokenMap()
{
    if (!pCellTextSpanElemTokenMap)
    {
        static const SvXMLTokenMapEntry aMap[] =
        {
            { XML_NAMESPACE_TEXT, XML_SHEET_NAME, XML_TOK_CELL_TEXT_SPAN_ELEM_SHEET_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_sheet_name) },
            { XML_NAMESPACE_TEXT, XML_DATE, XML_TOK_CELL_TEXT_SPAN_ELEM_DATE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_date) },
            { XML_NAMESPACE_TEXT, XML_TITLE, XML_TOK_CELL_TEXT_SPAN_ELEM_TITLE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_title) },
            { XML_NAMESPACE_TEXT, XML_A, XML_TOK_CELL_TEXT_SPAN_ELEM_URL,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_a) },
            { XML_NAMESPACE_TEXT, XML_S, XML_TOK_CELL_TEXT_SPAN_ELEM_S,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_s) },
            XML_TOKEN_MAP_END
        };

        pCellTextSpanElemTokenMap = new SvXMLTokenMap(aMap);
    }
    return *pCellTextSpanElemTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetCellTextSpanAttrTokenMap()
{
    if (!pCellTextSpanAttrTokenMap)
    {
        static const SvXMLTokenMapEntry aMap[] =
        {
            { XML_NAMESPACE_TEXT, XML_STYLE_NAME, XML_TOK_CELL_TEXT_SPAN_ATTR_STYLE_NAME,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_style_name) },
            XML_TOKEN_MAP_END
        };

        pCellTextSpanAttrTokenMap = new SvXMLTokenMap(aMap);
    }
    return *pCellTextSpanAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetCellTextURLAttrTokenMap()
{
    if (!pCellTextURLAttrTokenMap)
    {
        static const SvXMLTokenMapEntry aMap[] =
        {
            { XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_CELL_TEXT_URL_ATTR_UREF,
                (FastToken::NAMESPACE | XML_NAMESPACE_XLINK | XML_href) },
            { XML_NAMESPACE_XLINK, XML_TYPE, XML_TOK_CELL_TEXT_URL_ATTR_TYPE,
                (FastToken::NAMESPACE | XML_NAMESPACE_XLINK | XML_type) },
            XML_TOKEN_MAP_END
        };

        pCellTextURLAttrTokenMap = new SvXMLTokenMap(aMap);
    }
    return *pCellTextURLAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetCellTextSAttrTokenMap()
{
    if (!pCellTextSAttrTokenMap)
    {
        static const SvXMLTokenMapEntry aMap[] =
        {
            { XML_NAMESPACE_TEXT, XML_C, XML_TOK_CELL_TEXT_S_ATTR_C,
                (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_c) },
            XML_TOKEN_MAP_END
        };

        pCellTextSAttrTokenMap = new SvXMLTokenMap(aMap);
    }
    return *pCellTextSAttrTokenMap;
}

const SvXMLTokenMap& ScXMLImport::GetDataStreamAttrTokenMap()
{
    if (!pDataStreamAttrTokenMap)
    {
        static const SvXMLTokenMapEntry aMap[] =
        {
            { XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_DATA_STREAM_ATTR_URL,
                (FastToken::NAMESPACE | XML_NAMESPACE_XLINK | XML_href) },
            { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS, XML_TOK_DATA_STREAM_ATTR_RANGE,
                (FastToken::NAMESPACE | XML_NAMESPACE_TABLE | XML_target_range_address) },
            { XML_NAMESPACE_CALC_EXT, XML_EMPTY_LINE_REFRESH, XML_TOK_DATA_STREAM_ATTR_EMPTY_LINE_REFRESH,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_empty_line_refresh) },
            { XML_NAMESPACE_CALC_EXT, XML_INSERTION_POSITION, XML_TOK_DATA_STREAM_ATTR_INSERTION_POSITION,
                (FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT | XML_insertion_position) },
            XML_TOKEN_MAP_END
        };
        pDataStreamAttrTokenMap = new SvXMLTokenMap(aMap);
    }
    return *pDataStreamAttrTokenMap;
}

void ScXMLImport::SetPostProcessData( sc::ImportPostProcessData* p )
{
    mpPostProcessData = p;
}

sc::PivotTableSources& ScXMLImport::GetPivotTableSources()
{
    if (!mpPivotSources)
        mpPivotSources.reset(new sc::PivotTableSources);

    return *mpPivotSources;
}

SvXMLImportContext *ScXMLImport::CreateContext( sal_uInt16 nPrefix,
                                               const OUString& rLocalName,
                                               const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( (XML_NAMESPACE_OFFICE == nPrefix) &&
        ( IsXMLToken(rLocalName, XML_DOCUMENT_STYLES) ||
        IsXMLToken(rLocalName, XML_DOCUMENT_CONTENT) ||
        IsXMLToken(rLocalName, XML_DOCUMENT_SETTINGS) )) {
            pContext = new ScXMLDocContext_Impl( *this, nPrefix, rLocalName,
                xAttrList );
    } else if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
        ( IsXMLToken(rLocalName, XML_DOCUMENT_META)) ) {
            pContext = CreateMetaContext(rLocalName);
    } else if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
        ( IsXMLToken(rLocalName, XML_DOCUMENT)) ) {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                GetModel(), uno::UNO_QUERY_THROW);
            // flat OpenDocument file format
            pContext = new ScXMLFlatDocContext_Impl( *this, nPrefix, rLocalName,
                xAttrList, xDPS->getDocumentProperties());
    }
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

ScXMLImport::ScXMLImport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rContext,
    OUString const & implementationName, SvXMLImportFlags nImportFlag)
:   SvXMLImport( rContext, implementationName, nImportFlag ),
    pDoc( NULL ),
    pChangeTrackingImportHelper(NULL),
    pStylesImportHelper(NULL),
    sNumberFormat(SC_UNONAME_NUMFMT),
    sLocale(SC_LOCALE),
    sCellStyle(SC_UNONAME_CELLSTYL),
    sStandardFormat(SC_STANDARDFORMAT),
    sType(SC_UNONAME_TYPE),
    pDocElemTokenMap( 0 ),
    pStylesElemTokenMap( 0 ),
    pStylesAttrTokenMap( 0 ),
    pStyleElemTokenMap( 0 ),
    pBodyElemTokenMap( 0 ),
    pContentValidationsElemTokenMap( 0 ),
    pContentValidationElemTokenMap( 0 ),
    pContentValidationAttrTokenMap( 0 ),
    pContentValidationMessageElemTokenMap( 0 ),
    pContentValidationHelpMessageAttrTokenMap( 0 ),
    pContentValidationErrorMessageAttrTokenMap( 0 ),
    pContentValidationErrorMacroAttrTokenMap( 0 ),
    pCondFormatsTokenMap( 0 ),
    pCondFormatTokenMap( 0 ),
    pCondFormatAttrMap( 0 ),
    pCondDateAttrMap( 0 ),
    pConditionAttrMap( 0 ),
    pColorScaleTokenMap( 0 ),
    pColorScaleEntryAttrTokenMap( 0 ),
    pDataBarTokenMap( 0 ),
    pDataBarAttrMap( 0 ),
    pFormattingEntryAttrMap( 0 ),
    pIconSetAttrMap( 0 ),
    pLabelRangesElemTokenMap( 0 ),
    pLabelRangeAttrTokenMap( 0 ),
    pTableElemTokenMap( 0 ),
    pTableProtectionElemTokenMap(NULL),
    pTableRowsElemTokenMap( 0 ),
    pTableColsElemTokenMap( 0 ),
    pTableScenarioAttrTokenMap( 0 ),
    pTableAttrTokenMap( 0 ),
    pTableColAttrTokenMap( 0 ),
    pTableRowElemTokenMap( 0 ),
    pTableRowAttrTokenMap( 0 ),
    pTableRowCellElemTokenMap( 0 ),
    pTableRowCellAttrTokenMap( 0 ),
    pTableAnnotationAttrTokenMap( 0 ),
    pDetectiveElemTokenMap( 0 ),
    pDetectiveHighlightedAttrTokenMap( 0 ),
    pDetectiveOperationAttrTokenMap( 0 ),
    pTableCellRangeSourceAttrTokenMap( 0 ),
    pNamedExpressionsElemTokenMap( 0 ),
    pNamedRangeAttrTokenMap( 0 ),
    pNamedExpressionAttrTokenMap( 0 ),
    pDatabaseRangesElemTokenMap( 0 ),
    pDatabaseRangeElemTokenMap( 0 ),
    pDatabaseRangeAttrTokenMap( 0 ),
    pDatabaseRangeSourceSQLAttrTokenMap( 0 ),
    pDatabaseRangeSourceTableAttrTokenMap( 0 ),
    pDatabaseRangeSourceQueryAttrTokenMap( 0 ),
    pFilterElemTokenMap( 0 ),
    pFilterAttrTokenMap( 0 ),
    pFilterConditionElemTokenMap( 0 ),
    pFilterConditionAttrTokenMap( 0 ),
    pFilterSetItemAttrTokenMap( 0 ),
    pSortElemTokenMap( 0 ),
    pSortAttrTokenMap( 0 ),
    pSortSortByAttrTokenMap( 0 ),
    pDatabaseRangeSubTotalRulesElemTokenMap( 0 ),
    pDatabaseRangeSubTotalRulesAttrTokenMap( 0 ),
    pSubTotalRulesSortGroupsAttrTokenMap( 0 ),
    pSubTotalRulesSubTotalRuleElemTokenMap( 0 ),
    pSubTotalRulesSubTotalRuleAttrTokenMap( 0 ),
    pSubTotalRuleSubTotalFieldAttrTokenMap( 0 ),
    pDataPilotTablesElemTokenMap( 0 ),
    pDataPilotTableAttrTokenMap( 0 ),
    pDataPilotTableElemTokenMap( 0 ),
    pDataPilotTableSourceServiceAttrTokenMap( 0 ),
    pDataPilotGrandTotalAttrTokenMap(NULL),
    pDataPilotTableSourceCellRangeElemTokenMap( 0 ),
    pDataPilotTableSourceCellRangeAttrTokenMap( 0 ),
    pDataPilotFieldAttrTokenMap( 0 ),
    pDataPilotFieldElemTokenMap( 0 ),
    pDataPilotLevelAttrTokenMap( 0 ),
    pDataPilotLevelElemTokenMap( 0 ),
    pDataPilotSubTotalsElemTokenMap( 0 ),
    pDataPilotSubTotalAttrTokenMap( 0 ),
    pDataPilotMembersElemTokenMap( 0 ),
    pDataPilotMemberAttrTokenMap( 0 ),
    pConsolidationAttrTokenMap( 0 ),
    pCellTextParaElemTokenMap(NULL),
    pCellTextSpanElemTokenMap(NULL),
    pCellTextSpanAttrTokenMap(NULL),
    pCellTextURLAttrTokenMap(NULL),
    pCellTextSAttrTokenMap(NULL),
    pDataStreamAttrTokenMap(NULL),
    mpPostProcessData(NULL),
    aTables(*this),
    pMyNamedExpressions(NULL),
    pMyLabelRanges(NULL),
    pValidations(NULL),
    pDetectiveOpArray(NULL),
    pSolarMutexGuard(NULL),
    pNumberFormatAttributesExportHelper(NULL),
    pStyleNumberFormats(NULL),
    sPrevStyleName(),
    sPrevCurrency(),
    nSolarMutexLocked(0),
    nProgressCount(0),
    nStyleFamilyMask( 0 ),
    nPrevCellType(0),
    bLoadDoc( true ),
    bRemoveLastChar(false),
    bNullDateSetted(false),
    bSelfImportingXMLSet(false),
    mbLockSolarMutex(true),
    mbImportStyles(true),
    mbHasNewCondFormatData(false)
{
    pStylesImportHelper = new ScMyStylesImportHelper(*this);

    xScPropHdlFactory = new XMLScPropHdlFactory;
    xCellStylesPropertySetMapper = new XMLPropertySetMapper(aXMLScCellStylesProperties, xScPropHdlFactory, false);
    xColumnStylesPropertySetMapper = new XMLPropertySetMapper(aXMLScColumnStylesProperties, xScPropHdlFactory, false);
    xRowStylesPropertySetMapper = new XMLPropertySetMapper(aXMLScRowStylesImportProperties, xScPropHdlFactory, false);
    xTableStylesPropertySetMapper = new XMLPropertySetMapper(aXMLScTableStylesImportProperties, xScPropHdlFactory, false);

    // #i66550# needed for 'presentation:event-listener' element for URLs in shapes
    GetNamespaceMap().Add(
        GetXMLToken( XML_NP_PRESENTATION ),
        GetXMLToken( XML_N_PRESENTATION ),
        XML_NAMESPACE_PRESENTATION );

    // initialize cell type map.
    const struct { XMLTokenEnum  _token; sal_Int16 _type; } aCellTypePairs[] =
    {
        { XML_FLOAT,        util::NumberFormat::NUMBER },
        { XML_STRING,       util::NumberFormat::TEXT },
        { XML_TIME,         util::NumberFormat::TIME },
        { XML_DATE,         util::NumberFormat::DATETIME },
        { XML_PERCENTAGE,   util::NumberFormat::PERCENT },
        { XML_CURRENCY,     util::NumberFormat::CURRENCY },
        { XML_BOOLEAN,      util::NumberFormat::LOGICAL }
    };
    size_t n = sizeof(aCellTypePairs)/sizeof(aCellTypePairs[0]);
    for (size_t i = 0; i < n; ++i)
    {
        aCellTypeMap.insert(
            CellTypeMap::value_type(
                GetXMLToken(aCellTypePairs[i]._token), aCellTypePairs[i]._type));
    }
}

ScXMLImport::~ScXMLImport() throw()
{
    //  delete pI18NMap;
    delete pDocElemTokenMap;
    delete pStylesElemTokenMap;
    delete pStylesAttrTokenMap;
    delete pStyleElemTokenMap;
    delete pBodyElemTokenMap;
    delete pContentValidationsElemTokenMap;
    delete pContentValidationElemTokenMap;
    delete pContentValidationAttrTokenMap;
    delete pContentValidationMessageElemTokenMap;
    delete pContentValidationHelpMessageAttrTokenMap;
    delete pContentValidationErrorMessageAttrTokenMap;
    delete pContentValidationErrorMacroAttrTokenMap;
    delete pCondFormatsTokenMap;
    delete pCondFormatTokenMap;
    delete pCondFormatAttrMap;
    delete pCondDateAttrMap;
    delete pConditionAttrMap;
    delete pColorScaleTokenMap;
    delete pColorScaleEntryAttrTokenMap;
    delete pDataBarTokenMap;
    delete pDataBarAttrMap;
    delete pFormattingEntryAttrMap;
    delete pLabelRangesElemTokenMap;
    delete pLabelRangeAttrTokenMap;
    delete pTableElemTokenMap;
    delete pTableProtectionElemTokenMap;
    delete pTableRowsElemTokenMap;
    delete pTableColsElemTokenMap;
    delete pTableAttrTokenMap;
    delete pTableScenarioAttrTokenMap;
    delete pTableColAttrTokenMap;
    delete pTableRowElemTokenMap;
    delete pTableRowAttrTokenMap;
    delete pTableRowCellElemTokenMap;
    delete pTableRowCellAttrTokenMap;
    delete pTableAnnotationAttrTokenMap;
    delete pDetectiveElemTokenMap;
    delete pDetectiveHighlightedAttrTokenMap;
    delete pDetectiveOperationAttrTokenMap;
    delete pTableCellRangeSourceAttrTokenMap;
    delete pNamedExpressionsElemTokenMap;
    delete pNamedRangeAttrTokenMap;
    delete pNamedExpressionAttrTokenMap;
    delete pDatabaseRangesElemTokenMap;
    delete pDatabaseRangeElemTokenMap;
    delete pDatabaseRangeAttrTokenMap;
    delete pDatabaseRangeSourceSQLAttrTokenMap;
    delete pDatabaseRangeSourceTableAttrTokenMap;
    delete pDatabaseRangeSourceQueryAttrTokenMap;
    delete pFilterElemTokenMap;
    delete pFilterAttrTokenMap;
    delete pFilterConditionElemTokenMap;
    delete pFilterConditionAttrTokenMap;
    delete pFilterSetItemAttrTokenMap;
    delete pSortElemTokenMap;
    delete pSortAttrTokenMap;
    delete pSortSortByAttrTokenMap;
    delete pDatabaseRangeSubTotalRulesElemTokenMap;
    delete pDatabaseRangeSubTotalRulesAttrTokenMap;
    delete pSubTotalRulesSortGroupsAttrTokenMap;
    delete pSubTotalRulesSubTotalRuleElemTokenMap;
    delete pSubTotalRulesSubTotalRuleAttrTokenMap;
    delete pSubTotalRuleSubTotalFieldAttrTokenMap;
    delete pDataPilotTablesElemTokenMap;
    delete pDataPilotTableAttrTokenMap;
    delete pDataPilotTableElemTokenMap;
    delete pDataPilotTableSourceServiceAttrTokenMap;
    delete pDataPilotTableSourceCellRangeAttrTokenMap;
    delete pDataPilotTableSourceCellRangeElemTokenMap;
    delete pDataPilotFieldAttrTokenMap;
    delete pDataPilotFieldElemTokenMap;
    delete pDataPilotLevelAttrTokenMap;
    delete pDataPilotLevelElemTokenMap;
    delete pDataPilotSubTotalsElemTokenMap;
    delete pDataPilotSubTotalAttrTokenMap;
    delete pDataPilotMembersElemTokenMap;
    delete pDataPilotMemberAttrTokenMap;
    delete pConsolidationAttrTokenMap;
    delete pCellTextParaElemTokenMap;
    delete pCellTextSpanElemTokenMap;
    delete pCellTextSpanAttrTokenMap;
    delete pCellTextURLAttrTokenMap;
    delete pCellTextSAttrTokenMap;
    delete pDataStreamAttrTokenMap;

    delete pChangeTrackingImportHelper;
    delete pNumberFormatAttributesExportHelper;
    delete pStyleNumberFormats;
    delete pStylesImportHelper;

    delete pSolarMutexGuard;

    delete pMyNamedExpressions;
    delete pMyLabelRanges;
    delete pValidations;
    delete pDetectiveOpArray;
}

void ScXMLImport::initialize( const css::uno::Sequence<css::uno::Any>& aArguments )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    SvXMLImport::initialize(aArguments);

    uno::Reference<beans::XPropertySet> xInfoSet = getImportInfo();
    if (!xInfoSet.is())
        return;

    uno::Reference<beans::XPropertySetInfo> xInfoSetInfo = xInfoSet->getPropertySetInfo();
    if (!xInfoSetInfo.is())
        return;

    if (xInfoSetInfo->hasPropertyByName(SC_UNO_ODS_LOCK_SOLAR_MUTEX))
        xInfoSet->getPropertyValue(SC_UNO_ODS_LOCK_SOLAR_MUTEX) >>= mbLockSolarMutex;

    if (xInfoSetInfo->hasPropertyByName(SC_UNO_ODS_IMPORT_STYLES))
        xInfoSet->getPropertyValue(SC_UNO_ODS_IMPORT_STYLES) >>= mbImportStyles;
}

SvXMLImportContext *ScXMLImport::CreateFontDeclsContext(const sal_uInt16 nPrefix, const OUString& rLocalName,
                                                        const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    XMLFontStylesContext *pFSContext = new XMLFontStylesContext(
        *this, nPrefix, rLocalName, xAttrList, osl_getThreadTextEncoding());
    SetFontDecls(pFSContext);
    SvXMLImportContext* pContext = pFSContext;
    return pContext;
}

SvXMLImportContext *ScXMLImport::CreateStylesContext(const OUString& rLocalName,
                                                     const uno::Reference<xml::sax::XAttributeList>& xAttrList, bool bIsAutoStyle )
{
    SvXMLImportContext* pContext = new XMLTableStylesContext(
        *this, XML_NAMESPACE_OFFICE, rLocalName, xAttrList, bIsAutoStyle);

    if (bIsAutoStyle)
        SetAutoStyles(static_cast<SvXMLStylesContext*>(pContext));
    else
        SetStyles(static_cast<SvXMLStylesContext*>(pContext));

    return pContext;
}

SvXMLImportContext *ScXMLImport::CreateBodyContext(const OUString& rLocalName,
                                                   const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    return new ScXMLBodyContext(*this, XML_NAMESPACE_OFFICE, rLocalName, xAttrList);
}

SvXMLImportContext *ScXMLImport::CreateMetaContext(
    const OUString& rLocalName )
{
    SvXMLImportContext* pContext = NULL;

    if (getImportFlags() & SvXMLImportFlags::META)
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> const xDocProps(
            (IsStylesOnlyMode()) ? 0 : xDPS->getDocumentProperties());
        pContext = new SvXMLMetaDocumentContext(*this,
            XML_NAMESPACE_OFFICE, rLocalName,
            xDocProps);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( *this,
        XML_NAMESPACE_OFFICE, rLocalName );

    return pContext;
}

SvXMLImportContext *ScXMLImport::CreateScriptContext(
    const OUString& rLocalName )
{
    SvXMLImportContext* pContext = NULL;

    if( !(IsStylesOnlyMode()) )
    {
        pContext = new XMLScriptContext( *this,
            XML_NAMESPACE_OFFICE, rLocalName,
            GetModel() );
    }

    if( !pContext )
        pContext = new SvXMLImportContext( *this, XML_NAMESPACE_OFFICE,
        rLocalName );

    return pContext;
}

void ScXMLImport::SetStatistics(
                                const uno::Sequence<beans::NamedValue> & i_rStats)
{
    static const char* s_stats[] =
    { "TableCount", "CellCount", "ObjectCount", 0 };

    SvXMLImport::SetStatistics(i_rStats);

    sal_uInt32 nCount(0);
    for (sal_Int32 i = 0; i < i_rStats.getLength(); ++i) {
        for (const char** pStat = s_stats; *pStat != 0; ++pStat) {
            if (i_rStats[i].Name.equalsAscii(*pStat)) {
                sal_Int32 val = 0;
                if (i_rStats[i].Value >>= val) {
                    nCount += val;
                } else {
                    OSL_FAIL("ScXMLImport::SetStatistics: invalid entry");
                }
            }
        }
    }

    if (nCount)
    {
        GetProgressBarHelper()->SetReference(nCount);
        GetProgressBarHelper()->SetValue(0);
    }
}

ScDocumentImport& ScXMLImport::GetDoc()
{
    return *mpDocImport;
}

sal_Int16 ScXMLImport::GetCellType(const OUString& rStrValue) const
{
    CellTypeMap::const_iterator itr = aCellTypeMap.find(rStrValue);
    if (itr != aCellTypeMap.end())
        return itr->second;

    return util::NumberFormat::UNDEFINED;
}

XMLShapeImportHelper* ScXMLImport::CreateShapeImport()
{
    return new XMLTableShapeImportHelper(*this);
}

bool ScXMLImport::GetValidation(const OUString& sName, ScMyImportValidation& aValidation)
{
    if (pValidations)
    {
        bool bFound(false);
        ScMyImportValidations::iterator aItr(pValidations->begin());
        ScMyImportValidations::iterator aEndItr(pValidations->end());
        while(aItr != aEndItr && !bFound)
        {
            if (aItr->sName == sName)
            {
                // source position must be set as string,
                // so sBaseCellAddress no longer has to be converted here

                bFound = true;
            }
            else
                ++aItr;
        }
        if (bFound)
            aValidation = *aItr;
        return bFound;
    }
    return false;
}

void ScXMLImport::AddNamedExpression(SCTAB nTab, ScMyNamedExpression* pNamedExp)
{
    ::std::unique_ptr<ScMyNamedExpression> p(pNamedExp);
    SheetNamedExpMap::iterator itr = maSheetNamedExpressions.find(nTab);
    if (itr == maSheetNamedExpressions.end())
    {
        // No chain exists for this sheet.  Create one.
        ::std::unique_ptr<ScMyNamedExpressions> pNew(new ScMyNamedExpressions);
        ::std::pair<SheetNamedExpMap::iterator, bool> r = o3tl::ptr_container::insert(maSheetNamedExpressions, nTab, std::move(pNew));
        if (!r.second)
            // insertion failed.
            return;

        itr = r.first;
    }
    ScMyNamedExpressions& r = *itr->second;
    o3tl::ptr_container::push_back(r, std::move(p));
}

ScXMLChangeTrackingImportHelper* ScXMLImport::GetChangeTrackingImportHelper()
{
    if (!pChangeTrackingImportHelper)
        pChangeTrackingImportHelper = new ScXMLChangeTrackingImportHelper();
    return pChangeTrackingImportHelper;
}

void ScXMLImport::InsertStyles()
{
    GetStyles()->CopyStylesToDoc(true);

    // if content is going to be loaded with the same import, set bLatinDefaultStyle flag now
    if ( getImportFlags() & SvXMLImportFlags::CONTENT )
        ExamineDefaultStyle();
}

void ScXMLImport::ExamineDefaultStyle()
{
    if (pDoc)
    {
        // #i62435# after inserting the styles, check if the default style has a latin-script-only
        // number format (then, value cells can be pre-initialized with western script type)

        const ScPatternAttr* pDefPattern = pDoc->GetDefPattern();
        if (pDefPattern && sc::NumFmtUtil::isLatinScript(*pDefPattern, *pDoc))
            mpDocImport->setDefaultNumericScript(SvtScriptType::LATIN);
    }
}

void ScXMLImport::SetChangeTrackingViewSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rChangeProps)
{
    if (pDoc)
    {
        sal_Int32 nCount(rChangeProps.getLength());
        if (nCount)
        {
            ScXMLImport::MutexGuard aGuard(*this);
            sal_Int16 nTemp16(0);
            boost::scoped_ptr<ScChangeViewSettings> pViewSettings(new ScChangeViewSettings());
            for (sal_Int32 i = 0; i < nCount; ++i)
            {
                OUString sName(rChangeProps[i].Name);
                if (sName == "ShowChanges")
                    pViewSettings->SetShowChanges(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName == "ShowAcceptedChanges")
                    pViewSettings->SetShowAccepted(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName == "ShowRejectedChanges")
                    pViewSettings->SetShowRejected(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName == "ShowChangesByDatetime")
                    pViewSettings->SetHasDate(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName == "ShowChangesByDatetimeMode")
                {
                    if (rChangeProps[i].Value >>= nTemp16)
                        pViewSettings->SetTheDateMode(static_cast<SvxRedlinDateMode>(nTemp16));
                }
                else if (sName == "ShowChangesByDatetimeFirstDatetime")
                {
                    util::DateTime aDateTime;
                    if (rChangeProps[i].Value >>= aDateTime)
                    {
                        pViewSettings->SetTheFirstDateTime(::DateTime(aDateTime));
                    }
                }
                else if (sName == "ShowChangesByDatetimeSecondDatetime")
                {
                    util::DateTime aDateTime;
                    if (rChangeProps[i].Value >>= aDateTime)
                    {
                        pViewSettings->SetTheLastDateTime(::DateTime(aDateTime));
                    }
                }
                else if (sName == "ShowChangesByAuthor")
                    pViewSettings->SetHasAuthor(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName == "ShowChangesByAuthorName")
                {
                    OUString sOUName;
                    if (rChangeProps[i].Value >>= sOUName)
                    {
                        OUString sAuthorName(sOUName);
                        pViewSettings->SetTheAuthorToShow(sAuthorName);
                    }
                }
                else if (sName == "ShowChangesByComment")
                    pViewSettings->SetHasComment(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName == "ShowChangesByCommentText")
                {
                    OUString sOUComment;
                    if (rChangeProps[i].Value >>= sOUComment)
                    {
                        OUString sComment(sOUComment);
                        pViewSettings->SetTheComment(sComment);
                    }
                }
                else if (sName == "ShowChangesByRanges")
                    pViewSettings->SetHasRange(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName == "ShowChangesByRangesList")
                {
                    OUString sRanges;
                    if ((rChangeProps[i].Value >>= sRanges) && !sRanges.isEmpty())
                    {
                        ScRangeList aRangeList;
                        ScRangeStringConverter::GetRangeListFromString(
                            aRangeList, sRanges, GetDocument(), FormulaGrammar::CONV_OOO);
                        pViewSettings->SetTheRangeList(aRangeList);
                    }
                }
            }
            pDoc->SetChangeViewSettings(*pViewSettings);
        }
    }
}

void ScXMLImport::SetViewSettings(const uno::Sequence<beans::PropertyValue>& aViewProps)
{
    sal_Int32 nCount(aViewProps.getLength());
    sal_Int32 nHeight(0);
    sal_Int32 nLeft(0);
    sal_Int32 nTop(0);
    sal_Int32 nWidth(0);
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        OUString sName(aViewProps[i].Name);
        if (sName == "VisibleAreaHeight")
            aViewProps[i].Value >>= nHeight;
        else if (sName == "VisibleAreaLeft")
            aViewProps[i].Value >>= nLeft;
        else if (sName == "VisibleAreaTop")
            aViewProps[i].Value >>= nTop;
        else if (sName == "VisibleAreaWidth")
            aViewProps[i].Value >>= nWidth;
        else if (sName == "TrackedChangesViewSettings")
        {
            uno::Sequence<beans::PropertyValue> aChangeProps;
            if(aViewProps[i].Value >>= aChangeProps)
                SetChangeTrackingViewSettings(aChangeProps);
        }
    }
    if (nHeight && nWidth)
    {
        if (GetModel().is())
        {
            ScModelObj* pDocObj(ScModelObj::getImplementation( GetModel() ));
            if (pDocObj)
            {
                SfxObjectShell* pEmbeddedObj = pDocObj->GetEmbeddedObject();
                if (pEmbeddedObj)
                {
                    Rectangle aRect;
                    aRect.setX( nLeft );
                    aRect.setY( nTop );
                    aRect.setWidth( nWidth );
                    aRect.setHeight( nHeight );
                    pEmbeddedObj->SetVisArea(aRect);
                }
            }
        }
    }
}

void ScXMLImport::SetConfigurationSettings(const uno::Sequence<beans::PropertyValue>& aConfigProps)
{
    if (GetModel().is())
    {
        uno::Reference <lang::XMultiServiceFactory> xMultiServiceFactory(GetModel(), uno::UNO_QUERY);
        if (xMultiServiceFactory.is())
        {
            sal_Int32 nCount(aConfigProps.getLength());
            OUString sCTName("TrackedChangesProtectionKey");
            OUString sVBName("VBACompatibilityMode");
            OUString sSCName("ScriptConfiguration");
            css::uno::Sequence<css::beans::PropertyValue> aFilteredProps(
                aConfigProps.getLength());
            sal_Int32 nFilteredPropsLen = 0;
            for (sal_Int32 i = nCount - 1; i >= 0; --i)
            {
                if (aConfigProps[i].Name == sCTName)
                {
                    OUString sKey;
                    if (aConfigProps[i].Value >>= sKey)
                    {
                        uno::Sequence<sal_Int8> aPass;
                        ::sax::Converter::decodeBase64(aPass, sKey);
                        if (aPass.getLength())
                        {
                            if (pDoc->GetChangeTrack())
                                pDoc->GetChangeTrack()->SetProtection(aPass);
                            else
                            {
                                std::set<OUString> aUsers;
                                ScChangeTrack* pTrack = new ScChangeTrack(pDoc, aUsers);
                                pTrack->SetProtection(aPass);
                                pDoc->SetChangeTrack(pTrack);
                            }
                        }
                    }
                }
                // store the following items for later use (after document is loaded)
                else if ((aConfigProps[i].Name == sVBName) || (aConfigProps[i].Name == sSCName))
                {
                    uno::Reference< beans::XPropertySet > xImportInfo = getImportInfo();
                    if (xImportInfo.is())
                    {
                        uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = xImportInfo->getPropertySetInfo();
                        if (xPropertySetInfo.is() && xPropertySetInfo->hasPropertyByName(aConfigProps[i].Name))
                            xImportInfo->setPropertyValue( aConfigProps[i].Name, aConfigProps[i].Value );
                    }
                }
                if (aConfigProps[i].Name != "LinkUpdateMode")
                {
                    aFilteredProps[nFilteredPropsLen++] = aConfigProps[i];
                }
            }
            aFilteredProps.realloc(nFilteredPropsLen);
            uno::Reference <uno::XInterface> xInterface = xMultiServiceFactory->createInstance("com.sun.star.comp.SpreadsheetSettings");
            uno::Reference <beans::XPropertySet> xProperties(xInterface, uno::UNO_QUERY);
            if (xProperties.is())
                SvXMLUnitConverter::convertPropertySet(xProperties, aFilteredProps);
        }
    }
}

sal_Int32 ScXMLImport::SetCurrencySymbol(const sal_Int32 nKey, const OUString& rCurrency)
{
    uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier(GetNumberFormatsSupplier());
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xLocalNumberFormats(xNumberFormatsSupplier->getNumberFormats());
        if (xLocalNumberFormats.is())
        {
            OUString sFormatString;
            try
            {
                uno::Reference <beans::XPropertySet> xProperties(xLocalNumberFormats->getByKey(nKey));
                if (xProperties.is())
                {
                    lang::Locale aLocale;
                    if (GetDocument() && (xProperties->getPropertyValue(sLocale) >>= aLocale))
                    {
                        {
                            ScXMLImport::MutexGuard aGuard(*this);
                            LocaleDataWrapper aLocaleData( comphelper::getProcessComponentContext(), LanguageTag( aLocale) );
                            OUStringBuffer aBuffer(15);
                            aBuffer.appendAscii("#");
                            aBuffer.append( aLocaleData.getNumThousandSep() );
                            aBuffer.appendAscii("##0");
                            aBuffer.append( aLocaleData.getNumDecimalSep() );
                            aBuffer.appendAscii("00 [$");
                            aBuffer.append(rCurrency);
                            aBuffer.appendAscii("]");
                            sFormatString = aBuffer.makeStringAndClear();
                        }
                        sal_Int32 nNewKey = xLocalNumberFormats->queryKey(sFormatString, aLocale, true);
                        if (nNewKey == -1)
                            nNewKey = xLocalNumberFormats->addNew(sFormatString, aLocale);
                        return nNewKey;
                    }
                }
            }
            catch ( const util::MalformedNumberFormatException& rException )
            {
                OUString sErrorMessage("Fehler im Formatstring ");
                sErrorMessage += sFormatString;
                sErrorMessage += " an Position ";
                sErrorMessage += OUString::number(rException.CheckPos);
                uno::Sequence<OUString> aSeq(1);
                aSeq[0] = sErrorMessage;
                uno::Reference<xml::sax::XLocator> xLocator;
                SetError(XMLERROR_API | XMLERROR_FLAG_ERROR, aSeq, rException.Message, xLocator);
            }
        }
    }
    return nKey;
}

bool ScXMLImport::IsCurrencySymbol(const sal_Int32 nNumberFormat, const OUString& sCurrentCurrency, const OUString& sBankSymbol)
{
    uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier(GetNumberFormatsSupplier());
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xLocalNumberFormats(xNumberFormatsSupplier->getNumberFormats());
        if (xLocalNumberFormats.is())
        {
            try
            {
                uno::Reference <beans::XPropertySet> xNumberPropertySet(xLocalNumberFormats->getByKey(nNumberFormat));
                if (xNumberPropertySet.is())
                {
                    OUString sTemp;
                    if ( xNumberPropertySet->getPropertyValue(OUString(SC_CURRENCYSYMBOL)) >>= sTemp)
                    {
                        if (sCurrentCurrency.equals(sTemp))
                            return true;
                        // #i61657# This may be a legacy currency symbol that changed in the meantime.
                        if (SvNumberFormatter::GetLegacyOnlyCurrencyEntry( sCurrentCurrency, sBankSymbol) != NULL)
                            return true;
                        // In the rare case that sCurrentCurrency is not the
                        // currency symbol, but a matching ISO code
                        // abbreviation instead that was obtained through
                        // XMLNumberFormatAttributesExportHelper::GetCellType(),
                        // check with the number format's symbol. This happens,
                        // for example, in the es_BO locale, where a legacy
                        // B$,BOB matched B$->BOP, which leads to
                        // sCurrentCurrency being BOP, and the previous call
                        // with BOP,BOB didn't find an entry, but B$,BOB will.
                        return SvNumberFormatter::GetLegacyOnlyCurrencyEntry( sTemp, sBankSymbol) != NULL;
                    }
                }
            }
            catch ( uno::Exception& )
            {
                OSL_FAIL("Numberformat not found");
            }
        }
    }
    return false;
}

void ScXMLImport::SetType(uno::Reference <beans::XPropertySet>& rProperties,
                          sal_Int32& rNumberFormat,
                          const sal_Int16 nCellType,
                          const OUString& rCurrency)
{
    if (!mbImportStyles)
        return;

    if ((nCellType != util::NumberFormat::TEXT) && (nCellType != util::NumberFormat::UNDEFINED))
    {
        if (rNumberFormat == -1)
            rProperties->getPropertyValue( sNumberFormat ) >>= rNumberFormat;
        OSL_ENSURE(rNumberFormat != -1, "no NumberFormat");
        bool bIsStandard;
        // sCurrentCurrency may be the ISO code abbreviation if the currency
        // symbol matches such, or if no match found the symbol itself!
        OUString sCurrentCurrency;
        sal_Int32 nCurrentCellType(
            GetNumberFormatAttributesExportHelper()->GetCellType(
                rNumberFormat, sCurrentCurrency, bIsStandard) & ~util::NumberFormat::DEFINED);
        if ((nCellType != nCurrentCellType) && !((nCellType == util::NumberFormat::NUMBER &&
            ((nCurrentCellType == util::NumberFormat::SCIENTIFIC) ||
            (nCurrentCellType == util::NumberFormat::FRACTION) ||
            (nCurrentCellType == util::NumberFormat::LOGICAL) ||
            (nCurrentCellType == 0))) || (nCurrentCellType == util::NumberFormat::TEXT)) && !((nCellType == util::NumberFormat::DATETIME) &&
            (nCurrentCellType == util::NumberFormat::DATE)))
        {
            if (!xNumberFormats.is())
            {
                uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier(GetNumberFormatsSupplier());
                if (xNumberFormatsSupplier.is())
                    xNumberFormats.set(xNumberFormatsSupplier->getNumberFormats());
            }
            if (xNumberFormats.is())
            {
                try
                {
                    uno::Reference < beans::XPropertySet> xNumberFormatProperties(xNumberFormats->getByKey(rNumberFormat));
                    if (xNumberFormatProperties.is())
                    {
                        if (nCellType != util::NumberFormat::CURRENCY)
                        {
                            lang::Locale aLocale;
                            if ( xNumberFormatProperties->getPropertyValue(sLocale) >>= aLocale )
                            {
                                if (!xNumberFormatTypes.is())
                                    xNumberFormatTypes.set(uno::Reference <util::XNumberFormatTypes>(xNumberFormats, uno::UNO_QUERY));
                                rProperties->setPropertyValue( sNumberFormat, uno::makeAny(xNumberFormatTypes->getStandardFormat(nCellType, aLocale)) );
                            }
                        }
                        else if (!rCurrency.isEmpty() && !sCurrentCurrency.isEmpty())
                        {
                            if (!sCurrentCurrency.equals(rCurrency))
                                if (!IsCurrencySymbol(rNumberFormat, sCurrentCurrency, rCurrency))
                                    rProperties->setPropertyValue( sNumberFormat, uno::makeAny(SetCurrencySymbol(rNumberFormat, rCurrency)));
                        }
                    }
                }
                catch ( uno::Exception& )
                {
                    OSL_FAIL("Numberformat not found");
                }
            }
        }
        else
        {
            if ((nCellType == util::NumberFormat::CURRENCY) && !rCurrency.isEmpty() && !sCurrentCurrency.isEmpty() &&
                !sCurrentCurrency.equals(rCurrency) && !IsCurrencySymbol(rNumberFormat, sCurrentCurrency, rCurrency))
                rProperties->setPropertyValue( sNumberFormat, uno::makeAny(SetCurrencySymbol(rNumberFormat, rCurrency)));
        }
    }
}

void ScXMLImport::AddStyleRange(const table::CellRangeAddress& rCellRange)
{
    if (!mbImportStyles)
        return;

    if (!xSheetCellRanges.is() && GetModel().is())
    {
        uno::Reference <lang::XMultiServiceFactory> xMultiServiceFactory(GetModel(), uno::UNO_QUERY);
        if (xMultiServiceFactory.is())
            xSheetCellRanges.set(uno::Reference <sheet::XSheetCellRangeContainer>(xMultiServiceFactory->createInstance("com.sun.star.sheet.SheetCellRanges"), uno::UNO_QUERY));
        OSL_ENSURE(xSheetCellRanges.is(), "didn't get SheetCellRanges");

    }
    xSheetCellRanges->addRangeAddress(rCellRange, false);
}

void ScXMLImport::SetStyleToRanges()
{
    if (!mbImportStyles)
        return;

    if (!sPrevStyleName.isEmpty())
    {
        uno::Reference <beans::XPropertySet> xProperties (xSheetCellRanges, uno::UNO_QUERY);
        if (xProperties.is())
        {
            XMLTableStylesContext *pStyles(static_cast<XMLTableStylesContext *>(GetAutoStyles()));
            XMLTableStyleContext* pStyle = NULL;
            if ( pStyles )
                pStyle = const_cast<XMLTableStyleContext*>(static_cast<const XMLTableStyleContext *>(pStyles->FindStyleChildContext(
                        XML_STYLE_FAMILY_TABLE_CELL, sPrevStyleName, true)));
            if (pStyle)
            {
                pStyle->FillPropertySet(xProperties);
                // here needs to be the cond format import method
                sal_Int32 nNumberFormat(pStyle->GetNumberFormat());
                SetType(xProperties, nNumberFormat, nPrevCellType, sPrevCurrency);

                // store first cell of first range for each style, once per sheet
                uno::Sequence<table::CellRangeAddress> aAddresses(xSheetCellRanges->getRangeAddresses());
                pStyle->ApplyCondFormat(aAddresses);
                if ( aAddresses.getLength() > 0 )
                {
                    const table::CellRangeAddress& rRange = aAddresses[0];
                    if ( rRange.Sheet != pStyle->GetLastSheet() )
                    {
                        ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetModel())->GetSheetSaveData();
                        pSheetData->AddCellStyle( sPrevStyleName,
                            ScAddress( (SCCOL)rRange.StartColumn, (SCROW)rRange.StartRow, (SCTAB)rRange.Sheet ) );
                        pStyle->SetLastSheet(rRange.Sheet);
                    }
                }
            }
            else
            {
                xProperties->setPropertyValue(sCellStyle, uno::makeAny(GetStyleDisplayName( XML_STYLE_FAMILY_TABLE_CELL, sPrevStyleName )));
                sal_Int32 nNumberFormat(GetStyleNumberFormats()->GetStyleNumberFormat(sPrevStyleName));
                bool bInsert(nNumberFormat == -1);
                SetType(xProperties, nNumberFormat, nPrevCellType, sPrevCurrency);
                if (bInsert)
                    GetStyleNumberFormats()->AddStyleNumberFormat(sPrevStyleName, nNumberFormat);
            }
        }
    }
    if (GetModel().is())
    {
        uno::Reference <lang::XMultiServiceFactory> xMultiServiceFactory(GetModel(), uno::UNO_QUERY);
        if (xMultiServiceFactory.is())
            xSheetCellRanges.set(uno::Reference <sheet::XSheetCellRangeContainer>(
            xMultiServiceFactory->createInstance(
            OUString("com.sun.star.sheet.SheetCellRanges")),
            uno::UNO_QUERY));
    }
    OSL_ENSURE(xSheetCellRanges.is(), "didn't get SheetCellRanges");
}

void ScXMLImport::SetStyleToRange(const ScRange& rRange, const OUString* pStyleName,
                                  const sal_Int16 nCellType, const OUString* pCurrency)
{
    if (!mbImportStyles)
        return;

    if (sPrevStyleName.isEmpty())
    {
        nPrevCellType = nCellType;
        if (pStyleName)
            sPrevStyleName = *pStyleName;
        if (pCurrency)
            sPrevCurrency = *pCurrency;
        else if (!sPrevCurrency.isEmpty())
            sPrevCurrency = sEmpty;
    }
    else if ((nCellType != nPrevCellType) ||
        ((pStyleName && !pStyleName->equals(sPrevStyleName)) ||
        (!pStyleName && !sPrevStyleName.isEmpty())) ||
        ((pCurrency && !pCurrency->equals(sPrevCurrency)) ||
        (!pCurrency && !sPrevCurrency.isEmpty())))
    {
        SetStyleToRanges();
        nPrevCellType = nCellType;
        if (pStyleName)
            sPrevStyleName = *pStyleName;
        else if(!sPrevStyleName.isEmpty())
            sPrevStyleName = sEmpty;
        if (pCurrency)
            sPrevCurrency = *pCurrency;
        else if(!sPrevCurrency.isEmpty())
            sPrevCurrency = sEmpty;
    }
    table::CellRangeAddress aCellRange;
    aCellRange.StartColumn = rRange.aStart.Col();
    aCellRange.StartRow = rRange.aStart.Row();
    aCellRange.Sheet = rRange.aStart.Tab();
    aCellRange.EndColumn = rRange.aEnd.Col();
    aCellRange.EndRow = rRange.aEnd.Row();
    AddStyleRange(aCellRange);
}

bool ScXMLImport::SetNullDateOnUnitConverter()
{
    if (!bNullDateSetted)
        bNullDateSetted = GetMM100UnitConverter().setNullDate(GetModel());
    OSL_ENSURE(bNullDateSetted, "could not set the null date");
    return bNullDateSetted;
}

XMLNumberFormatAttributesExportHelper* ScXMLImport::GetNumberFormatAttributesExportHelper()
{
    if (!pNumberFormatAttributesExportHelper)
        pNumberFormatAttributesExportHelper = new XMLNumberFormatAttributesExportHelper(GetNumberFormatsSupplier());
    return pNumberFormatAttributesExportHelper;
}

ScMyStyleNumberFormats* ScXMLImport::GetStyleNumberFormats()
{
    if (!pStyleNumberFormats)
        pStyleNumberFormats = new ScMyStyleNumberFormats();
    return pStyleNumberFormats;
}

void ScXMLImport::SetStylesToRangesFinished()
{
    SetStyleToRanges();
    sPrevStyleName = sEmpty;
}

// XImporter
void SAL_CALL ScXMLImport::setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    ScXMLImport::MutexGuard aGuard(*this);
    SvXMLImport::setTargetDocument( xDoc );

    uno::Reference<frame::XModel> xModel(xDoc, uno::UNO_QUERY);
    pDoc = ScXMLConverter::GetScDocument( xModel );
    OSL_ENSURE( pDoc, "ScXMLImport::setTargetDocument - no ScDocument!" );
    if (!pDoc)
        throw lang::IllegalArgumentException();

    mpDocImport.reset(new ScDocumentImport(*pDoc));
    mpComp.reset(new ScCompiler(pDoc, ScAddress()));
    mpComp->SetGrammar(formula::FormulaGrammar::GRAM_ODFF);

    uno::Reference<document::XActionLockable> xActionLockable(xDoc, uno::UNO_QUERY);
    if (xActionLockable.is())
        xActionLockable->addActionLock();
}

// ::com::sun::star::xml::sax::XDocumentHandler
void SAL_CALL ScXMLImport::startDocument()
throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception )
{
    ScXMLImport::MutexGuard aGuard(*this);
    SvXMLImport::startDocument();
    if (pDoc && !pDoc->IsImportingXML())
    {
        ScModelObj::getImplementation(GetModel())->BeforeXMLLoading();
        bSelfImportingXMLSet = true;
    }

    // if content and styles are loaded with separate imports,
    // set bLatinDefaultStyle flag at the start of the content import
    SvXMLImportFlags nFlags = getImportFlags();
    if ( ( nFlags & SvXMLImportFlags::CONTENT ) && !( nFlags & SvXMLImportFlags::STYLES ) )
        ExamineDefaultStyle();

    if (getImportFlags() & SvXMLImportFlags::CONTENT)
    {
        if (GetModel().is())
        {
            // store initial namespaces, to find the ones that were added from the file later
            ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetModel())->GetSheetSaveData();
            const SvXMLNamespaceMap& rNamespaces = GetNamespaceMap();
            pSheetData->StoreInitialNamespaces(rNamespaces);
        }
    }

    uno::Reference< beans::XPropertySet > const xImportInfo( getImportInfo() );
    uno::Reference< beans::XPropertySetInfo > const xPropertySetInfo(
            xImportInfo.is() ? xImportInfo->getPropertySetInfo() : 0);
    if (xPropertySetInfo.is())
    {
        OUString const sOrganizerMode(
            "OrganizerMode");
        if (xPropertySetInfo->hasPropertyByName(sOrganizerMode))
        {
            bool bStyleOnly(false);
            if (xImportInfo->getPropertyValue(sOrganizerMode) >>= bStyleOnly)
            {
                bLoadDoc = !bStyleOnly;
            }
        }
    }

    UnlockSolarMutex();
}

sal_Int32 ScXMLImport::GetRangeType(const OUString& sRangeType)
{
    sal_Int32 nRangeType(0);
    OUStringBuffer sBuffer;
    sal_Int16 i = 0;
    while (i <= sRangeType.getLength())
    {
        if ((i == sRangeType.getLength()) || (sRangeType[i] == ' '))
        {
            OUString sTemp = sBuffer.makeStringAndClear();
            if (sTemp == "repeat-column")
                nRangeType |= sheet::NamedRangeFlag::COLUMN_HEADER;
            else if (sTemp == SC_REPEAT_ROW)
                nRangeType |= sheet::NamedRangeFlag::ROW_HEADER;
            else if (sTemp == SC_FILTER)
                nRangeType |= sheet::NamedRangeFlag::FILTER_CRITERIA;
            else if (sTemp == SC_PRINT_RANGE)
                nRangeType |= sheet::NamedRangeFlag::PRINT_AREA;
        }
        else if (i < sRangeType.getLength())
            sBuffer.append(sRangeType[i]);
        ++i;
    }
    return nRangeType;
}

void ScXMLImport::SetLabelRanges()
{
    ScMyLabelRanges* pLabelRanges = GetLabelRanges();
    if (pLabelRanges)
    {
        uno::Reference <beans::XPropertySet> xPropertySet (GetModel(), uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            uno::Any aColAny = xPropertySet->getPropertyValue(OUString(SC_UNO_COLLABELRNG));
            uno::Any aRowAny = xPropertySet->getPropertyValue(OUString(SC_UNO_ROWLABELRNG));

            uno::Reference< sheet::XLabelRanges > xColRanges;
            uno::Reference< sheet::XLabelRanges > xRowRanges;

            if ( ( aColAny >>= xColRanges ) && ( aRowAny >>= xRowRanges ) )
            {
                table::CellRangeAddress aLabelRange;
                table::CellRangeAddress aDataRange;

                ScMyLabelRanges::iterator aItr = pLabelRanges->begin();
                while (aItr != pLabelRanges->end())
                {
                    sal_Int32 nOffset1(0);
                    sal_Int32 nOffset2(0);
                    FormulaGrammar::AddressConvention eConv = FormulaGrammar::CONV_OOO;

                    if (ScRangeStringConverter::GetRangeFromString( aLabelRange, (*aItr)->sLabelRangeStr, GetDocument(), eConv, nOffset1 ) &&
                        ScRangeStringConverter::GetRangeFromString( aDataRange, (*aItr)->sDataRangeStr, GetDocument(), eConv, nOffset2 ))
                    {
                        if ( (*aItr)->bColumnOrientation )
                            xColRanges->addNew( aLabelRange, aDataRange );
                        else
                            xRowRanges->addNew( aLabelRange, aDataRange );
                    }

                    delete *aItr;
                    aItr = pLabelRanges->erase(aItr);
                }
            }
        }
    }
}

namespace {

class RangeNameInserter : public ::std::unary_function<ScMyNamedExpression, void>
{
    ScDocument* mpDoc;
    ScRangeName& mrRangeName;
    ScXMLImport& mrXmlImport;

public:
    RangeNameInserter(ScDocument* pDoc, ScRangeName& rRangeName, ScXMLImport& rXmlImport) :
        mpDoc(pDoc), mrRangeName(rRangeName), mrXmlImport(rXmlImport) {}

    void operator() (const ScMyNamedExpression& r) const
    {
        using namespace formula;

        const OUString& aType = r.sRangeType;
        sal_uInt32 nUnoType = ScXMLImport::GetRangeType(aType);

        sal_uInt16 nNewType = RT_NAME;
        if ( nUnoType & sheet::NamedRangeFlag::FILTER_CRITERIA )    nNewType |= RT_CRITERIA;
        if ( nUnoType & sheet::NamedRangeFlag::PRINT_AREA )         nNewType |= RT_PRINTAREA;
        if ( nUnoType & sheet::NamedRangeFlag::COLUMN_HEADER )      nNewType |= RT_COLHEADER;
        if ( nUnoType & sheet::NamedRangeFlag::ROW_HEADER )         nNewType |= RT_ROWHEADER;

        if (mpDoc)
        {
            // Insert a new name.
            ScAddress aPos;
            sal_Int32 nOffset = 0;
            bool bSuccess = ScRangeStringConverter::GetAddressFromString(
                aPos, r.sBaseCellAddress, mpDoc, FormulaGrammar::CONV_OOO, nOffset);

            if (bSuccess)
            {
                OUString aContent = r.sContent;
                if (!r.bIsExpression)
                    ScXMLConverter::ParseFormula(aContent, false);

                ScRangeData* pData = new ScRangeData(
                    mpDoc, r.sName, aContent, aPos, nNewType, r.eGrammar);
                mrRangeName.insert(pData);
            }
        }
    }
};

}

void ScXMLImport::SetNamedRanges()
{
    ScMyNamedExpressions* pNamedExpressions = GetNamedExpressions();
    if (!pNamedExpressions)
        return;

    if (!pDoc)
        return;

    // Insert the namedRanges
    ScRangeName* pRangeNames = pDoc->GetRangeName();
    ::std::for_each(pNamedExpressions->begin(), pNamedExpressions->end(), RangeNameInserter(pDoc, *pRangeNames, *this));
}

void ScXMLImport::SetSheetNamedRanges()
{
    if (!pDoc)
        return;

    SheetNamedExpMap::const_iterator itr = maSheetNamedExpressions.begin(), itrEnd = maSheetNamedExpressions.end();
    for (; itr != itrEnd; ++itr)
    {
        SCTAB nTab = itr->first;
        ScRangeName* pRangeNames = pDoc->GetRangeName(nTab);
        if (!pRangeNames)
            continue;

        const ScMyNamedExpressions& rNames = *itr->second;
        ::std::for_each(rNames.begin(), rNames.end(), RangeNameInserter(pDoc, *pRangeNames, *this));
    }
}

void SAL_CALL ScXMLImport::endDocument()
    throw(::com::sun::star::xml::sax::SAXException,
          ::com::sun::star::uno::RuntimeException,
          std::exception)
{
    ScXMLImport::MutexGuard aGuard(*this);
    if (getImportFlags() & SvXMLImportFlags::CONTENT)
    {
        if (GetModel().is())
        {
            mpDocImport->finalize();

            uno::Reference<document::XViewDataSupplier> xViewDataSupplier(GetModel(), uno::UNO_QUERY);
            if (xViewDataSupplier.is())
            {
                uno::Reference<container::XIndexAccess> xIndexAccess(xViewDataSupplier->getViewData());
                if (xIndexAccess.is() && xIndexAccess->getCount() > 0)
                {
                    uno::Sequence< beans::PropertyValue > aSeq;
                    if (xIndexAccess->getByIndex(0) >>= aSeq)
                    {
                        sal_Int32 nCount (aSeq.getLength());
                        for (sal_Int32 i = 0; i < nCount; ++i)
                        {
                            OUString sName(aSeq[i].Name);
                            if (sName == SC_ACTIVETABLE)
                            {
                                OUString sValue;
                                if(aSeq[i].Value >>= sValue)
                                {
                                    OUString sTabName(sValue);
                                    SCTAB nTab(0);
                                    if (pDoc->GetTable(sTabName, nTab))
                                    {
                                        pDoc->SetVisibleTab(nTab);
                                        i = nCount;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            SetLabelRanges();
            SetNamedRanges();
            SetSheetNamedRanges();
            if (mpPivotSources)
                // Process pivot table sources after the named ranges have been set.
                mpPivotSources->process();
        }
        GetProgressBarHelper()->End();  // make room for subsequent SfxProgressBars
        if (pDoc)
        {
            pDoc->CompileXML();

            // After CompileXML, links must be completely changed to the new URLs.
            // Otherwise, hasExternalFile for API wouldn't work (#i116940#),
            // and typing a new formula would create a second link with the same "real" file name.
            if (pDoc->HasExternalRefManager())
                pDoc->GetExternalRefManager()->updateAbsAfterLoad();
        }

        // If the stream contains cells outside of the current limits, the styles can't be re-created,
        // so stream copying is disabled then.
        if (pDoc && GetModel().is() && !pDoc->HasRangeOverflow())
        {
            // set "valid stream" flags after loading (before UpdateRowHeights, so changed formula results
            // in UpdateRowHeights can already clear the flags again)
            ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetModel())->GetSheetSaveData();

            SCTAB nTabCount = pDoc->GetTableCount();
            for (SCTAB nTab=0; nTab<nTabCount; ++nTab)
            {
                pDoc->SetDrawPageSize(nTab);
                if (!pSheetData->IsSheetBlocked( nTab ))
                    pDoc->SetStreamValid( nTab, true );
            }
        }
        aTables.FixupOLEs();
    }
    if (GetModel().is())
    {
        uno::Reference<document::XActionLockable> xActionLockable(GetModel(), uno::UNO_QUERY);
        if (xActionLockable.is())
            xActionLockable->removeActionLock();
    }
    SvXMLImport::endDocument();

    if(pDoc && bSelfImportingXMLSet)
        ScModelObj::getImplementation(GetModel())->AfterXMLLoading(true);
}

// XEventListener
void ScXMLImport::DisposingModel()
{
    SvXMLImport::DisposingModel();
    pDoc = NULL;
}

ScXMLImport::MutexGuard::MutexGuard(ScXMLImport& rImport) :
    mrImport(rImport)
{
    mrImport.LockSolarMutex();
}

ScXMLImport::MutexGuard::~MutexGuard()
{
    mrImport.UnlockSolarMutex();
}

void ScXMLImport::LockSolarMutex()
{
    // #i62677# When called from DocShell/Wrapper, the SolarMutex is already locked,
    // so there's no need to allocate (and later delete) the SolarMutexGuard.
    if (!mbLockSolarMutex)
    {
        DBG_TESTSOLARMUTEX();
        return;
    }

    if (nSolarMutexLocked == 0)
    {
        OSL_ENSURE(!pSolarMutexGuard, "Solar Mutex is locked");
        pSolarMutexGuard = new SolarMutexGuard();
    }
    ++nSolarMutexLocked;
}

void ScXMLImport::UnlockSolarMutex()
{
    if (nSolarMutexLocked > 0)
    {
        nSolarMutexLocked--;
        if (nSolarMutexLocked == 0)
        {
            OSL_ENSURE(pSolarMutexGuard, "Solar Mutex is always unlocked");
            delete pSolarMutexGuard;
            pSolarMutexGuard = NULL;
        }
    }
}

sal_Int32 ScXMLImport::GetByteOffset()
{
    sal_Int32 nOffset = -1;
    uno::Reference<xml::sax::XLocator> xLocator = GetLocator();
    uno::Reference<io::XSeekable> xSeek( xLocator, uno::UNO_QUERY );        //! should use different interface
    if ( xSeek.is() )
        nOffset = (sal_Int32)xSeek->getPosition();
    return nOffset;
}

void ScXMLImport::SetRangeOverflowType(sal_uInt32 nType)
{
    //  #i31130# Overflow is stored in the document, because the ScXMLImport object
    //  isn't available in ScXMLImportWrapper::ImportFromComponent when using the
    //  OOo->Oasis transformation.

    if ( pDoc )
        pDoc->SetRangeOverflowType( nType );
}

void ScXMLImport::ProgressBarIncrement(bool bEditCell, sal_Int32 nInc)
{
    nProgressCount += nInc;
    if (bEditCell || nProgressCount > 100)
    {
        GetProgressBarHelper()->Increment(nProgressCount);
        nProgressCount = 0;
    }
}

void ScXMLImport::ExtractFormulaNamespaceGrammar(
        OUString& rFormula, OUString& rFormulaNmsp, FormulaGrammar::Grammar& reGrammar,
        const OUString& rAttrValue, bool bRestrictToExternalNmsp ) const
{
    // parse the attribute value, extract namespace ID, literal namespace, and formula string
    rFormulaNmsp.clear();
    sal_uInt16 nNsId = GetNamespaceMap()._GetKeyByAttrName( rAttrValue, 0, &rFormula, &rFormulaNmsp, false );

    // check if we have an ODF formula namespace
    if( !bRestrictToExternalNmsp ) switch( nNsId )
    {
        case XML_NAMESPACE_OOOC:
            rFormulaNmsp.clear();  // remove namespace string for built-in grammar
            reGrammar = FormulaGrammar::GRAM_PODF;
            return;
        case XML_NAMESPACE_OF:
            rFormulaNmsp.clear();  // remove namespace string for built-in grammar
            reGrammar = FormulaGrammar::GRAM_ODFF;
            return;
    }

    /*  Find default grammar for formulas without namespace. There may be
        documents in the wild that stored no namespace in ODF 1.0/1.1. Use
        GRAM_PODF then (old style ODF 1.0/1.1 formulas). The default for ODF
        1.2 and later without namespace is GRAM_ODFF (OpenFormula). */
    FormulaGrammar::Grammar eDefaultGrammar =
        (GetDocument()->GetStorageGrammar() == FormulaGrammar::GRAM_PODF) ?
            FormulaGrammar::GRAM_PODF : FormulaGrammar::GRAM_ODFF;

    /*  Check if we have no namespace at all. The value XML_NAMESPACE_NONE
        indicates that there is no colon. If the first character of the
        attribute value is the equality sign, the value XML_NAMESPACE_UNKNOWN
        indicates that there is a colon somewhere in the formula string. */
    if( (nNsId == XML_NAMESPACE_NONE) || ((nNsId == XML_NAMESPACE_UNKNOWN) && (rAttrValue.toChar() == '=')) )
    {
        rFormula = rAttrValue;          // return entire string as formula
        reGrammar = eDefaultGrammar;
        return;
    }

    /*  Check if a namespace URL could be resolved from the attribute value.
        Use that namespace only, if the Calc document knows an associated
        external formula parser. This prevents that the range operator in
        conjunction with defined names is confused as namespaces prefix, e.g.
        in the expression 'table:A1' where 'table' is a named reference. */
    if( ((nNsId & XML_NAMESPACE_UNKNOWN_FLAG) != 0) && !rFormulaNmsp.isEmpty() &&
        GetDocument()->GetFormulaParserPool().hasFormulaParser( rFormulaNmsp ) )
    {
        reGrammar = FormulaGrammar::GRAM_EXTERNAL;
        return;
    }

    /*  All attempts failed (e.g. no namespace and no leading equality sign, or
        an invalid namespace prefix), continue with the entire attribute value. */
    rFormula = rAttrValue;
    rFormulaNmsp.clear();  // remove any namespace string
    reGrammar = eDefaultGrammar;
}

bool ScXMLImport::IsFormulaErrorConstant( const OUString& rStr ) const
{
    if (!mpComp)
        return false;

    return mpComp->GetErrorConstant(rStr) > 0;
}

ScEditEngineDefaulter* ScXMLImport::GetEditEngine()
{
    if (!mpEditEngine)
    {
        mpEditEngine.reset(new ScEditEngineDefaulter(pDoc->GetEnginePool()));
        mpEditEngine->SetRefMapMode(MAP_100TH_MM);
        mpEditEngine->SetEditTextObjectPool(pDoc->GetEditPool());
        mpEditEngine->SetUpdateMode(false);
        mpEditEngine->EnableUndo(false);
        mpEditEngine->SetControlWord(mpEditEngine->GetControlWord() & ~EEControlBits::ALLOWBIGOBJS);
    }
    return mpEditEngine.get();
}

const ScXMLEditAttributeMap& ScXMLImport::GetEditAttributeMap() const
{
    if (!mpEditAttrMap)
        mpEditAttrMap.reset(new ScXMLEditAttributeMap);
    return *mpEditAttrMap;
}

void ScXMLImport::NotifyEmbeddedFontRead()
{
    if ( pDoc )
        pDoc->SetIsUsingEmbededFonts( true );
}

ScMyImpDetectiveOpArray* ScXMLImport::GetDetectiveOpArray()
{
    if (!pDetectiveOpArray)
        pDetectiveOpArray = new ScMyImpDetectiveOpArray();
    return pDetectiveOpArray;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
