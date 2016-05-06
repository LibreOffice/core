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

#include <memory>
#include <utility>
#include <o3tl/make_unique.hxx>
#define SC_LOCALE           "Locale"
#define SC_CURRENCYSYMBOL   "CurrencySymbol"
#define SC_REPEAT_ROW "repeat-row"
#define SC_FILTER "filter"
#define SC_PRINT_RANGE "print-range"

using namespace com::sun::star;
using namespace ::xmloff::token;
using namespace ::formula;

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
    // return (cppu::OWeakObject*)new ScXMLImport(SvXMLImportFlags::SETTINGS);
    return static_cast<cppu::OWeakObject*>(new ScXMLImport( comphelper::getComponentContext(rSMgr), ScXMLImport_Settings_getImplementationName(), SvXMLImportFlags::SETTINGS ));
}

const SvXMLTokenMap& ScXMLImport::GetTableRowCellAttrTokenMap()
{
    static const SvXMLTokenMapEntry aTableRowCellAttrTokenMap[] =
    {
        { XML_NAMESPACE_TABLE,  XML_STYLE_NAME,                     XML_TOK_TABLE_ROW_CELL_ATTR_STYLE_NAME              },
        { XML_NAMESPACE_TABLE,  XML_CONTENT_VALIDATION_NAME,        XML_TOK_TABLE_ROW_CELL_ATTR_CONTENT_VALIDATION_NAME },
        { XML_NAMESPACE_TABLE,  XML_NUMBER_ROWS_SPANNED,            XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_ROWS            },
        { XML_NAMESPACE_TABLE,  XML_NUMBER_COLUMNS_SPANNED,         XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_COLS            },
        { XML_NAMESPACE_TABLE,  XML_NUMBER_MATRIX_COLUMNS_SPANNED,  XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_COLS     },
        { XML_NAMESPACE_TABLE,  XML_NUMBER_MATRIX_ROWS_SPANNED,     XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_ROWS     },
        { XML_NAMESPACE_TABLE,  XML_NUMBER_COLUMNS_REPEATED,        XML_TOK_TABLE_ROW_CELL_ATTR_REPEATED                },
        { XML_NAMESPACE_OFFICE, XML_VALUE_TYPE,                     XML_TOK_TABLE_ROW_CELL_ATTR_VALUE_TYPE              },
        { XML_NAMESPACE_CALC_EXT, XML_VALUE_TYPE,                   XML_TOK_TABLE_ROW_CELL_ATTR_NEW_VALUE_TYPE          },
        { XML_NAMESPACE_OFFICE, XML_VALUE,                          XML_TOK_TABLE_ROW_CELL_ATTR_VALUE                   },
        { XML_NAMESPACE_OFFICE, XML_DATE_VALUE,                     XML_TOK_TABLE_ROW_CELL_ATTR_DATE_VALUE              },
        { XML_NAMESPACE_OFFICE, XML_TIME_VALUE,                     XML_TOK_TABLE_ROW_CELL_ATTR_TIME_VALUE              },
        { XML_NAMESPACE_OFFICE, XML_STRING_VALUE,                   XML_TOK_TABLE_ROW_CELL_ATTR_STRING_VALUE            },
        { XML_NAMESPACE_OFFICE, XML_BOOLEAN_VALUE,                  XML_TOK_TABLE_ROW_CELL_ATTR_BOOLEAN_VALUE           },
        { XML_NAMESPACE_TABLE,  XML_FORMULA,                        XML_TOK_TABLE_ROW_CELL_ATTR_FORMULA                 },
        { XML_NAMESPACE_OFFICE, XML_CURRENCY,                       XML_TOK_TABLE_ROW_CELL_ATTR_CURRENCY                },
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
        const uno::Reference<xml::sax::XAttributeList>& xAttrList ) override;
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
        const uno::Reference<xml::sax::XAttributeList>& i_xAttrList) override;
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
        const uno::Reference< xml::sax::XAttributeList > & xAttrList ) override;
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
    SvXMLImportContext *pContext(nullptr);

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
        SAL_INFO("sc", "XML_TOK_DOC_META: should not have come here, maybe document is invalid?");
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
            { XML_NAMESPACE_OFFICE, XML_FONT_FACE_DECLS,    XML_TOK_DOC_FONTDECLS           },
            { XML_NAMESPACE_OFFICE, XML_STYLES,             XML_TOK_DOC_STYLES              },
            { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_STYLES,   XML_TOK_DOC_AUTOSTYLES          },
            { XML_NAMESPACE_OFFICE, XML_MASTER_STYLES,      XML_TOK_DOC_MASTERSTYLES        },
            { XML_NAMESPACE_OFFICE, XML_META,               XML_TOK_DOC_META                },
            { XML_NAMESPACE_OFFICE, XML_SCRIPTS,            XML_TOK_DOC_SCRIPTS             },
            { XML_NAMESPACE_OFFICE, XML_BODY,               XML_TOK_DOC_BODY                },
            { XML_NAMESPACE_OFFICE, XML_SETTINGS,           XML_TOK_DOC_SETTINGS            },
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
            { XML_NAMESPACE_TABLE, XML_TRACKED_CHANGES,         XML_TOK_BODY_TRACKED_CHANGES        },
            { XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS,    XML_TOK_BODY_CALCULATION_SETTINGS   },
            { XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATIONS,     XML_TOK_BODY_CONTENT_VALIDATIONS    },
            { XML_NAMESPACE_TABLE, XML_LABEL_RANGES,            XML_TOK_BODY_LABEL_RANGES           },
            { XML_NAMESPACE_TABLE, XML_TABLE,                   XML_TOK_BODY_TABLE                  },
            { XML_NAMESPACE_TABLE, XML_NAMED_EXPRESSIONS,       XML_TOK_BODY_NAMED_EXPRESSIONS      },
            { XML_NAMESPACE_TABLE, XML_DATABASE_RANGES,         XML_TOK_BODY_DATABASE_RANGES        },
            { XML_NAMESPACE_TABLE, XML_DATABASE_RANGE,          XML_TOK_BODY_DATABASE_RANGE         },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLES,       XML_TOK_BODY_DATA_PILOT_TABLES      },
            { XML_NAMESPACE_TABLE, XML_CONSOLIDATION,           XML_TOK_BODY_CONSOLIDATION          },
            { XML_NAMESPACE_TABLE, XML_DDE_LINKS,               XML_TOK_BODY_DDE_LINKS              },
            { XML_NAMESPACE_CALC_EXT, XML_DATA_STREAM_SOURCE,   XML_TOK_BODY_DATA_STREAM_SOURCE     },
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
            { XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION,  XML_TOK_CONTENT_VALIDATION  },
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
            { XML_NAMESPACE_TABLE,  XML_HELP_MESSAGE,    XML_TOK_CONTENT_VALIDATION_ELEM_HELP_MESSAGE    },
            { XML_NAMESPACE_TABLE,  XML_ERROR_MESSAGE,   XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MESSAGE   },
            { XML_NAMESPACE_TABLE,  XML_ERROR_MACRO,     XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MACRO     },
            { XML_NAMESPACE_OFFICE, XML_EVENT_LISTENERS, XML_TOK_CONTENT_VALIDATION_ELEM_EVENT_LISTENERS },
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
            { XML_NAMESPACE_TABLE, XML_NAME,                XML_TOK_CONTENT_VALIDATION_NAME                 },
            { XML_NAMESPACE_TABLE, XML_CONDITION,           XML_TOK_CONTENT_VALIDATION_CONDITION            },
            { XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS,   XML_TOK_CONTENT_VALIDATION_BASE_CELL_ADDRESS    },
            { XML_NAMESPACE_TABLE, XML_ALLOW_EMPTY_CELL,    XML_TOK_CONTENT_VALIDATION_ALLOW_EMPTY_CELL     },
            { XML_NAMESPACE_TABLE, XML_DISPLAY_LIST,        XML_TOK_CONTENT_VALIDATION_DISPLAY_LIST         },
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
            { XML_NAMESPACE_TEXT, XML_P,    XML_TOK_P   },
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
            { XML_NAMESPACE_TABLE, XML_TITLE,   XML_TOK_HELP_MESSAGE_ATTR_TITLE     },
            { XML_NAMESPACE_TABLE, XML_DISPLAY, XML_TOK_HELP_MESSAGE_ATTR_DISPLAY   },
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
            { XML_NAMESPACE_TABLE, XML_TITLE,           XML_TOK_ERROR_MESSAGE_ATTR_TITLE        },
            { XML_NAMESPACE_TABLE, XML_DISPLAY,         XML_TOK_ERROR_MESSAGE_ATTR_DISPLAY      },
            { XML_NAMESPACE_TABLE, XML_MESSAGE_TYPE,    XML_TOK_ERROR_MESSAGE_ATTR_MESSAGE_TYPE },
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
            { XML_NAMESPACE_TABLE, XML_NAME,    XML_TOK_ERROR_MACRO_ATTR_NAME       },
            { XML_NAMESPACE_TABLE, XML_EXECUTE, XML_TOK_ERROR_MACRO_ATTR_EXECUTE    },
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
            { XML_NAMESPACE_CALC_EXT, XML_CONDITIONAL_FORMAT, XML_TOK_CONDFORMATS_CONDFORMAT },
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
            { XML_NAMESPACE_CALC_EXT, XML_COLOR_SCALE, XML_TOK_CONDFORMAT_COLORSCALE },
            { XML_NAMESPACE_CALC_EXT, XML_DATA_BAR, XML_TOK_CONDFORMAT_DATABAR },
            { XML_NAMESPACE_CALC_EXT, XML_CONDITION, XML_TOK_CONDFORMAT_CONDITION },
            { XML_NAMESPACE_CALC_EXT, XML_ICON_SET, XML_TOK_CONDFORMAT_ICONSET },
            { XML_NAMESPACE_CALC_EXT, XML_DATE_IS, XML_TOK_CONDFORMAT_DATE },
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
            { XML_NAMESPACE_CALC_EXT, XML_TARGET_RANGE_ADDRESS, XML_TOK_CONDFORMAT_TARGET_RANGE },
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
            { XML_NAMESPACE_CALC_EXT, XML_DATE, XML_TOK_COND_DATE_VALUE },
            { XML_NAMESPACE_CALC_EXT, XML_STYLE, XML_TOK_COND_DATE_STYLE },
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
            { XML_NAMESPACE_CALC_EXT, XML_VALUE, XML_TOK_CONDITION_VALUE },
            { XML_NAMESPACE_CALC_EXT, XML_APPLY_STYLE_NAME, XML_TOK_CONDITION_APPLY_STYLE_NAME },
            { XML_NAMESPACE_CALC_EXT, XML_BASE_CELL_ADDRESS, XML_TOK_CONDITION_BASE_CELL_ADDRESS },
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
            { XML_NAMESPACE_CALC_EXT, XML_COLOR_SCALE_ENTRY, XML_TOK_COLORSCALE_COLORSCALEENTRY },
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
            { XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_TOK_COLORSCALEENTRY_TYPE },
            { XML_NAMESPACE_CALC_EXT, XML_VALUE, XML_TOK_COLORSCALEENTRY_VALUE },
            { XML_NAMESPACE_CALC_EXT, XML_COLOR, XML_TOK_COLORSCALEENTRY_COLOR },
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
            { XML_NAMESPACE_CALC_EXT, XML_DATA_BAR_ENTRY, XML_TOK_DATABAR_DATABARENTRY },
            { XML_NAMESPACE_CALC_EXT, XML_FORMATTING_ENTRY, XML_TOK_FORMATTING_ENTRY },
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
            { XML_NAMESPACE_CALC_EXT, XML_POSITIVE_COLOR, XML_TOK_DATABAR_POSITIVE_COLOR },
            { XML_NAMESPACE_CALC_EXT, XML_NEGATIVE_COLOR, XML_TOK_DATABAR_NEGATIVE_COLOR },
            { XML_NAMESPACE_CALC_EXT, XML_GRADIENT, XML_TOK_DATABAR_GRADIENT },
            { XML_NAMESPACE_CALC_EXT, XML_AXIS_POSITION, XML_TOK_DATABAR_AXISPOSITION },
            { XML_NAMESPACE_CALC_EXT, XML_SHOW_VALUE, XML_TOK_DATABAR_SHOWVALUE },
            { XML_NAMESPACE_CALC_EXT, XML_AXIS_COLOR, XML_TOK_DATABAR_AXISCOLOR },
            { XML_NAMESPACE_CALC_EXT, XML_MIN_LENGTH, XML_TOK_DATABAR_MINLENGTH },
            { XML_NAMESPACE_CALC_EXT, XML_MAX_LENGTH, XML_TOK_DATABAR_MAXLENGTH },
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
            { XML_NAMESPACE_CALC_EXT, XML_ICON_SET_TYPE, XML_TOK_ICONSET_TYPE },
            { XML_NAMESPACE_CALC_EXT, XML_SHOW_VALUE, XML_TOK_ICONSET_SHOWVALUE },
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
            { XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_TOK_DATABARENTRY_TYPE },
            { XML_NAMESPACE_CALC_EXT, XML_VALUE, XML_TOK_DATABARENTRY_VALUE },
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
            { XML_NAMESPACE_TABLE, XML_LABEL_RANGE, XML_TOK_LABEL_RANGE_ELEM    },
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
            { XML_NAMESPACE_TABLE, XML_LABEL_CELL_RANGE_ADDRESS,    XML_TOK_LABEL_RANGE_ATTR_LABEL_RANGE    },
            { XML_NAMESPACE_TABLE, XML_DATA_CELL_RANGE_ADDRESS,     XML_TOK_LABEL_RANGE_ATTR_DATA_RANGE     },
            { XML_NAMESPACE_TABLE, XML_ORIENTATION,                 XML_TOK_LABEL_RANGE_ATTR_ORIENTATION    },
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
            { XML_NAMESPACE_TABLE,  XML_NAMED_EXPRESSIONS,    XML_TOK_TABLE_NAMED_EXPRESSIONS },
            { XML_NAMESPACE_TABLE,  XML_TABLE_COLUMN_GROUP,   XML_TOK_TABLE_COL_GROUP       },
            { XML_NAMESPACE_TABLE,  XML_TABLE_HEADER_COLUMNS, XML_TOK_TABLE_HEADER_COLS },
            { XML_NAMESPACE_TABLE,  XML_TABLE_COLUMNS,        XML_TOK_TABLE_COLS            },
            { XML_NAMESPACE_TABLE,  XML_TABLE_COLUMN,         XML_TOK_TABLE_COL         },
            { XML_NAMESPACE_TABLE,  XML_TABLE_PROTECTION,     XML_TOK_TABLE_PROTECTION    },
            { XML_NAMESPACE_LO_EXT, XML_TABLE_PROTECTION,     XML_TOK_TABLE_PROTECTION_EXT  },
            { XML_NAMESPACE_OFFICE_EXT, XML_TABLE_PROTECTION, XML_TOK_TABLE_PROTECTION_EXT  },
            { XML_NAMESPACE_TABLE,  XML_TABLE_ROW_GROUP,      XML_TOK_TABLE_ROW_GROUP       },
            { XML_NAMESPACE_TABLE,  XML_TABLE_HEADER_ROWS,    XML_TOK_TABLE_HEADER_ROWS },
            { XML_NAMESPACE_TABLE,  XML_TABLE_ROWS,           XML_TOK_TABLE_ROWS            },
            { XML_NAMESPACE_TABLE,  XML_TABLE_ROW,            XML_TOK_TABLE_ROW         },
            { XML_NAMESPACE_TABLE,  XML_TABLE_SOURCE,         XML_TOK_TABLE_SOURCE      },
            { XML_NAMESPACE_TABLE,  XML_SCENARIO,             XML_TOK_TABLE_SCENARIO        },
            { XML_NAMESPACE_TABLE,  XML_SHAPES,               XML_TOK_TABLE_SHAPES      },
            { XML_NAMESPACE_OFFICE, XML_FORMS,                XML_TOK_TABLE_FORMS           },
            { XML_NAMESPACE_OFFICE, XML_EVENT_LISTENERS,      XML_TOK_TABLE_EVENT_LISTENERS },
            { XML_NAMESPACE_OFFICE_EXT, XML_EVENT_LISTENERS,  XML_TOK_TABLE_EVENT_LISTENERS_EXT },
            { XML_NAMESPACE_CALC_EXT, XML_CONDITIONAL_FORMATS, XML_TOK_TABLE_CONDFORMATS },
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
            { XML_NAMESPACE_TABLE, XML_SELECT_PROTECTED_CELLS,      XML_TOK_TABLE_SELECT_PROTECTED_CELLS    },
            { XML_NAMESPACE_TABLE, XML_SELECT_UNPROTECTED_CELLS,    XML_TOK_TABLE_SELECT_UNPROTECTED_CELLS  },
            { XML_NAMESPACE_OFFICE_EXT, XML_SELECT_PROTECTED_CELLS, XML_TOK_TABLE_SELECT_PROTECTED_CELLS_EXT    },
            { XML_NAMESPACE_LO_EXT, XML_SELECT_PROTECTED_CELLS, XML_TOK_TABLE_SELECT_PROTECTED_CELLS_EXT    },
            { XML_NAMESPACE_OFFICE_EXT, XML_SELECT_UNPROTECTED_CELLS, XML_TOK_TABLE_SELECT_UNPROTECTED_CELLS_EXT  },
            { XML_NAMESPACE_LO_EXT, XML_SELECT_UNPROTECTED_CELLS, XML_TOK_TABLE_SELECT_UNPROTECTED_CELLS_EXT  },
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
            { XML_NAMESPACE_TABLE, XML_TABLE_ROW_GROUP,     XML_TOK_TABLE_ROWS_ROW_GROUP    },
            { XML_NAMESPACE_TABLE, XML_TABLE_HEADER_ROWS,   XML_TOK_TABLE_ROWS_HEADER_ROWS  },
            { XML_NAMESPACE_TABLE, XML_TABLE_ROWS,          XML_TOK_TABLE_ROWS_ROWS         },
            { XML_NAMESPACE_TABLE, XML_TABLE_ROW,           XML_TOK_TABLE_ROWS_ROW          },
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
            { XML_NAMESPACE_TABLE, XML_TABLE_COLUMN_GROUP,      XML_TOK_TABLE_COLS_COL_GROUP    },
            { XML_NAMESPACE_TABLE, XML_TABLE_HEADER_COLUMNS,    XML_TOK_TABLE_COLS_HEADER_COLS  },
            { XML_NAMESPACE_TABLE, XML_TABLE_COLUMNS,           XML_TOK_TABLE_COLS_COLS         },
            { XML_NAMESPACE_TABLE, XML_TABLE_COLUMN,            XML_TOK_TABLE_COLS_COL          },
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
            { XML_NAMESPACE_TABLE,     XML_NAME,           XML_TOK_TABLE_NAME          },
            { XML_NAMESPACE_TABLE,     XML_STYLE_NAME,     XML_TOK_TABLE_STYLE_NAME    },
            { XML_NAMESPACE_TABLE, XML_PROTECTED,                   XML_TOK_TABLE_PROTECTED         },
            { XML_NAMESPACE_TABLE,     XML_PRINT_RANGES,   XML_TOK_TABLE_PRINT_RANGES  },
            { XML_NAMESPACE_TABLE,     XML_PROTECTION_KEY, XML_TOK_TABLE_PASSWORD      },
            { XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM, XML_TOK_TABLE_PASSHASH      },
            { XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM_2, XML_TOK_TABLE_PASSHASH_2  },
            { XML_NAMESPACE_LO_EXT, XML_PROTECTION_KEY_DIGEST_ALGORITHM_2, XML_TOK_TABLE_PASSHASH_2  },
            { XML_NAMESPACE_TABLE,     XML_PRINT,          XML_TOK_TABLE_PRINT         },
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
            { XML_NAMESPACE_TABLE, XML_DISPLAY_BORDER,      XML_TOK_TABLE_SCENARIO_ATTR_DISPLAY_BORDER  },
            { XML_NAMESPACE_TABLE, XML_BORDER_COLOR,        XML_TOK_TABLE_SCENARIO_ATTR_BORDER_COLOR    },
            { XML_NAMESPACE_TABLE, XML_COPY_BACK,           XML_TOK_TABLE_SCENARIO_ATTR_COPY_BACK       },
            { XML_NAMESPACE_TABLE, XML_COPY_STYLES,         XML_TOK_TABLE_SCENARIO_ATTR_COPY_STYLES     },
            { XML_NAMESPACE_TABLE, XML_COPY_FORMULAS,       XML_TOK_TABLE_SCENARIO_ATTR_COPY_FORMULAS   },
            { XML_NAMESPACE_TABLE, XML_IS_ACTIVE,           XML_TOK_TABLE_SCENARIO_ATTR_IS_ACTIVE       },
            { XML_NAMESPACE_TABLE, XML_SCENARIO_RANGES,     XML_TOK_TABLE_SCENARIO_ATTR_SCENARIO_RANGES },
            { XML_NAMESPACE_TABLE, XML_COMMENT,             XML_TOK_TABLE_SCENARIO_ATTR_COMMENT         },
            { XML_NAMESPACE_TABLE, XML_PROTECTED,           XML_TOK_TABLE_SCENARIO_ATTR_PROTECTED       },
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
            { XML_NAMESPACE_TABLE, XML_STYLE_NAME,                  XML_TOK_TABLE_COL_ATTR_STYLE_NAME       },
            { XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED,     XML_TOK_TABLE_COL_ATTR_REPEATED         },
            { XML_NAMESPACE_TABLE, XML_VISIBILITY,                  XML_TOK_TABLE_COL_ATTR_VISIBILITY       },
            { XML_NAMESPACE_TABLE, XML_DEFAULT_CELL_STYLE_NAME,    XML_TOK_TABLE_COL_ATTR_DEFAULT_CELL_STYLE_NAME },
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
            { XML_NAMESPACE_TABLE, XML_TABLE_CELL,      XML_TOK_TABLE_ROW_CELL              },
            { XML_NAMESPACE_TABLE, XML_COVERED_TABLE_CELL,  XML_TOK_TABLE_ROW_COVERED_CELL      },
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
            { XML_NAMESPACE_TABLE, XML_STYLE_NAME,                  XML_TOK_TABLE_ROW_ATTR_STYLE_NAME           },
            { XML_NAMESPACE_TABLE, XML_VISIBILITY,                  XML_TOK_TABLE_ROW_ATTR_VISIBILITY           },
            { XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_REPEATED,        XML_TOK_TABLE_ROW_ATTR_REPEATED             },
            { XML_NAMESPACE_TABLE, XML_DEFAULT_CELL_STYLE_NAME,     XML_TOK_TABLE_ROW_ATTR_DEFAULT_CELL_STYLE_NAME },
            //  { XML_NAMESPACE_TABLE, XML_USE_OPTIMAL_HEIGHT,          XML_TOK_TABLE_ROW_ATTR_USE_OPTIMAL_HEIGHT   },
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
            { XML_NAMESPACE_TEXT,   XML_P,                  XML_TOK_TABLE_ROW_CELL_P                    },
            { XML_NAMESPACE_TABLE,  XML_SUB_TABLE,          XML_TOK_TABLE_ROW_CELL_TABLE                },
            { XML_NAMESPACE_OFFICE, XML_ANNOTATION,         XML_TOK_TABLE_ROW_CELL_ANNOTATION           },
            { XML_NAMESPACE_TABLE,  XML_DETECTIVE,          XML_TOK_TABLE_ROW_CELL_DETECTIVE            },
            { XML_NAMESPACE_TABLE,  XML_CELL_RANGE_SOURCE,  XML_TOK_TABLE_ROW_CELL_CELL_RANGE_SOURCE    },
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
            { XML_NAMESPACE_OFFICE, XML_AUTHOR,                 XML_TOK_TABLE_ANNOTATION_ATTR_AUTHOR                },
            { XML_NAMESPACE_OFFICE, XML_CREATE_DATE,            XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE           },
            { XML_NAMESPACE_OFFICE, XML_CREATE_DATE_STRING,     XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE_STRING    },
            { XML_NAMESPACE_OFFICE, XML_DISPLAY,                XML_TOK_TABLE_ANNOTATION_ATTR_DISPLAY               },
            { XML_NAMESPACE_SVG,    XML_X,                      XML_TOK_TABLE_ANNOTATION_ATTR_X                     },
            { XML_NAMESPACE_SVG,    XML_Y,                      XML_TOK_TABLE_ANNOTATION_ATTR_Y                     },
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
            { XML_NAMESPACE_TABLE,  XML_HIGHLIGHTED_RANGE,  XML_TOK_DETECTIVE_ELEM_HIGHLIGHTED  },
            { XML_NAMESPACE_TABLE,  XML_OPERATION,          XML_TOK_DETECTIVE_ELEM_OPERATION    },
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
            { XML_NAMESPACE_TABLE,  XML_CELL_RANGE_ADDRESS,     XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_CELL_RANGE       },
            { XML_NAMESPACE_TABLE,  XML_DIRECTION,              XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_DIRECTION        },
            { XML_NAMESPACE_TABLE,  XML_CONTAINS_ERROR,         XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_CONTAINS_ERROR   },
            { XML_NAMESPACE_TABLE,  XML_MARKED_INVALID,         XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_MARKED_INVALID   },
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
            { XML_NAMESPACE_TABLE,  XML_NAME,   XML_TOK_DETECTIVE_OPERATION_ATTR_NAME   },
            { XML_NAMESPACE_TABLE,  XML_INDEX,  XML_TOK_DETECTIVE_OPERATION_ATTR_INDEX  },
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
            { XML_NAMESPACE_TABLE,  XML_NAME,                   XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_NAME           },
            { XML_NAMESPACE_XLINK,  XML_HREF,                   XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_HREF           },
            { XML_NAMESPACE_TABLE,  XML_FILTER_NAME,            XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_NAME    },
            { XML_NAMESPACE_TABLE,  XML_FILTER_OPTIONS,         XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_OPTIONS },
            { XML_NAMESPACE_TABLE,  XML_LAST_COLUMN_SPANNED,    XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_COLUMN    },
            { XML_NAMESPACE_TABLE,  XML_LAST_ROW_SPANNED,       XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_ROW       },
            { XML_NAMESPACE_TABLE,  XML_REFRESH_DELAY,          XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_REFRESH_DELAY  },
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
            { XML_NAMESPACE_TABLE, XML_NAMED_RANGE,             XML_TOK_NAMED_EXPRESSIONS_NAMED_RANGE           },
            { XML_NAMESPACE_TABLE, XML_NAMED_EXPRESSION,        XML_TOK_NAMED_EXPRESSIONS_NAMED_EXPRESSION  },
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
            { XML_NAMESPACE_TABLE, XML_NAME,                XML_TOK_NAMED_RANGE_ATTR_NAME               },
            { XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS,  XML_TOK_NAMED_RANGE_ATTR_CELL_RANGE_ADDRESS },
            { XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS,   XML_TOK_NAMED_RANGE_ATTR_BASE_CELL_ADDRESS  },
            { XML_NAMESPACE_TABLE, XML_RANGE_USABLE_AS,     XML_TOK_NAMED_RANGE_ATTR_RANGE_USABLE_AS    },
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
            { XML_NAMESPACE_TABLE, XML_NAME,                XML_TOK_NAMED_EXPRESSION_ATTR_NAME              },
            { XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS,   XML_TOK_NAMED_EXPRESSION_ATTR_BASE_CELL_ADDRESS },
            { XML_NAMESPACE_TABLE, XML_EXPRESSION,          XML_TOK_NAMED_EXPRESSION_ATTR_EXPRESSION        },
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
            { XML_NAMESPACE_TABLE, XML_DATABASE_RANGE,  XML_TOK_DATABASE_RANGE      },
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
            { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_SQL,     XML_TOK_DATABASE_RANGE_SOURCE_SQL       },
            { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_TABLE,   XML_TOK_DATABASE_RANGE_SOURCE_TABLE     },
            { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_QUERY,   XML_TOK_DATABASE_RANGE_SOURCE_QUERY     },
            { XML_NAMESPACE_TABLE, XML_FILTER,                  XML_TOK_FILTER                          },
            { XML_NAMESPACE_TABLE, XML_SORT,                    XML_TOK_SORT                            },
            { XML_NAMESPACE_TABLE, XML_SUBTOTAL_RULES,          XML_TOK_DATABASE_RANGE_SUBTOTAL_RULES   },
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
            { XML_NAMESPACE_TABLE, XML_NAME,                    XML_TOK_DATABASE_RANGE_ATTR_NAME                    },
            { XML_NAMESPACE_TABLE, XML_IS_SELECTION,            XML_TOK_DATABASE_RANGE_ATTR_IS_SELECTION            },
            { XML_NAMESPACE_TABLE, XML_ON_UPDATE_KEEP_STYLES,   XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_STYLES   },
            { XML_NAMESPACE_TABLE, XML_ON_UPDATE_KEEP_SIZE,     XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_SIZE     },
            { XML_NAMESPACE_TABLE, XML_HAS_PERSISTENT_DATA,     XML_TOK_DATABASE_RANGE_ATTR_HAS_PERSISTENT_DATA     },
            { XML_NAMESPACE_TABLE, XML_ORIENTATION,         XML_TOK_DATABASE_RANGE_ATTR_ORIENTATION             },
            { XML_NAMESPACE_TABLE, XML_CONTAINS_HEADER,     XML_TOK_DATABASE_RANGE_ATTR_CONTAINS_HEADER         },
            { XML_NAMESPACE_TABLE, XML_DISPLAY_FILTER_BUTTONS,  XML_TOK_DATABASE_RANGE_ATTR_DISPLAY_FILTER_BUTTONS  },
            { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS,    XML_TOK_DATABASE_RANGE_ATTR_TARGET_RANGE_ADDRESS    },
            { XML_NAMESPACE_TABLE, XML_REFRESH_DELAY,           XML_TOK_DATABASE_RANGE_ATTR_REFRESH_DELAY           },
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
            { XML_NAMESPACE_TABLE, XML_DATABASE_NAME,           XML_TOK_SOURCE_SQL_ATTR_DATABASE_NAME       },
            { XML_NAMESPACE_XLINK, XML_HREF,                    XML_TOK_SOURCE_SQL_ATTR_HREF                },
            { XML_NAMESPACE_TABLE, XML_CONNECTION_RESOURCE,     XML_TOK_SOURCE_SQL_ATTR_CONNECTION_RESOURCE},
            { XML_NAMESPACE_TABLE, XML_SQL_STATEMENT,           XML_TOK_SOURCE_SQL_ATTR_SQL_STATEMENT       },
            { XML_NAMESPACE_TABLE, XML_PARSE_SQL_STATEMENT, XML_TOK_SOURCE_SQL_ATTR_PARSE_SQL_STATEMENT },
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
            { XML_NAMESPACE_TABLE, XML_DATABASE_NAME,           XML_TOK_SOURCE_TABLE_ATTR_DATABASE_NAME         },
            { XML_NAMESPACE_XLINK, XML_HREF,                    XML_TOK_SOURCE_TABLE_ATTR_HREF                  },
            { XML_NAMESPACE_TABLE, XML_CONNECTION_RESOURCE,     XML_TOK_SOURCE_TABLE_ATTR_CONNECTION_RESOURCE  },
            { XML_NAMESPACE_TABLE, XML_TABLE_NAME,              XML_TOK_SOURCE_TABLE_ATTR_TABLE_NAME            },
            { XML_NAMESPACE_TABLE, XML_DATABASE_TABLE_NAME,     XML_TOK_SOURCE_TABLE_ATTR_TABLE_NAME            },
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
            { XML_NAMESPACE_TABLE, XML_DATABASE_NAME,           XML_TOK_SOURCE_QUERY_ATTR_DATABASE_NAME         },
            { XML_NAMESPACE_XLINK, XML_HREF,                    XML_TOK_SOURCE_QUERY_ATTR_HREF                  },
            { XML_NAMESPACE_TABLE, XML_CONNECTION_RESOURCE,     XML_TOK_SOURCE_QUERY_ATTR_CONNECTION_RESOURCE  },
            { XML_NAMESPACE_TABLE, XML_QUERY_NAME,              XML_TOK_SOURCE_QUERY_ATTR_QUERY_NAME            },
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
            { XML_NAMESPACE_TABLE, XML_FILTER_AND,          XML_TOK_FILTER_AND          },
            { XML_NAMESPACE_TABLE, XML_FILTER_OR,           XML_TOK_FILTER_OR           },
            { XML_NAMESPACE_TABLE, XML_FILTER_CONDITION,    XML_TOK_FILTER_CONDITION    },
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
            { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS,            XML_TOK_FILTER_ATTR_TARGET_RANGE_ADDRESS            },
            { XML_NAMESPACE_TABLE, XML_CONDITION_SOURCE_RANGE_ADDRESS,  XML_TOK_FILTER_ATTR_CONDITION_SOURCE_RANGE_ADDRESS  },
            { XML_NAMESPACE_TABLE, XML_CONDITION_SOURCE,                XML_TOK_FILTER_ATTR_CONDITION_SOURCE                },
            { XML_NAMESPACE_TABLE, XML_DISPLAY_DUPLICATES,              XML_TOK_FILTER_ATTR_DISPLAY_DUPLICATES              },
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
            { XML_NAMESPACE_TABLE, XML_FILTER_SET_ITEM, XML_TOK_CONDITION_FILTER_SET_ITEM },
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
            { XML_NAMESPACE_TABLE, XML_FIELD_NUMBER,    XML_TOK_CONDITION_ATTR_FIELD_NUMBER     },
            { XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE,  XML_TOK_CONDITION_ATTR_CASE_SENSITIVE   },
            { XML_NAMESPACE_TABLE, XML_DATA_TYPE,       XML_TOK_CONDITION_ATTR_DATA_TYPE        },
            { XML_NAMESPACE_TABLE, XML_VALUE,           XML_TOK_CONDITION_ATTR_VALUE            },
            { XML_NAMESPACE_TABLE, XML_OPERATOR,        XML_TOK_CONDITION_ATTR_OPERATOR         },
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
            { XML_NAMESPACE_TABLE, XML_VALUE, XML_TOK_FILTER_SET_ITEM_ATTR_VALUE },
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
            { XML_NAMESPACE_TABLE, XML_SORT_BY, XML_TOK_SORT_SORT_BY    },
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
            { XML_NAMESPACE_TABLE, XML_BIND_STYLES_TO_CONTENT,  XML_TOK_SORT_ATTR_BIND_STYLES_TO_CONTENT    },
            { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS,    XML_TOK_SORT_ATTR_TARGET_RANGE_ADDRESS      },
            { XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE,          XML_TOK_SORT_ATTR_CASE_SENSITIVE            },
            { XML_NAMESPACE_TABLE, XML_RFC_LANGUAGE_TAG,        XML_TOK_SORT_ATTR_RFC_LANGUAGE_TAG          },
            { XML_NAMESPACE_TABLE, XML_LANGUAGE,                XML_TOK_SORT_ATTR_LANGUAGE                  },
            { XML_NAMESPACE_TABLE, XML_SCRIPT,                  XML_TOK_SORT_ATTR_SCRIPT                    },
            { XML_NAMESPACE_TABLE, XML_COUNTRY,                 XML_TOK_SORT_ATTR_COUNTRY                   },
            { XML_NAMESPACE_TABLE, XML_ALGORITHM,               XML_TOK_SORT_ATTR_ALGORITHM                 },
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
            { XML_NAMESPACE_TABLE, XML_FIELD_NUMBER,    XML_TOK_SORT_BY_ATTR_FIELD_NUMBER   },
            { XML_NAMESPACE_TABLE, XML_DATA_TYPE,       XML_TOK_SORT_BY_ATTR_DATA_TYPE      },
            { XML_NAMESPACE_TABLE, XML_ORDER,           XML_TOK_SORT_BY_ATTR_ORDER          },
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
            { XML_NAMESPACE_TABLE, XML_SORT_GROUPS,     XML_TOK_SUBTOTAL_RULES_SORT_GROUPS      },
            { XML_NAMESPACE_TABLE, XML_SUBTOTAL_RULE,   XML_TOK_SUBTOTAL_RULES_SUBTOTAL_RULE    },
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
            { XML_NAMESPACE_TABLE, XML_BIND_STYLES_TO_CONTENT,          XML_TOK_SUBTOTAL_RULES_ATTR_BIND_STYLES_TO_CONTENT      },
            { XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE,                  XML_TOK_SUBTOTAL_RULES_ATTR_CASE_SENSITIVE              },
            { XML_NAMESPACE_TABLE, XML_PAGE_BREAKS_ON_GROUP_CHANGE, XML_TOK_SUBTOTAL_RULES_ATTR_PAGE_BREAKS_ON_GROUP_CHANGE },
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
            { XML_NAMESPACE_TABLE, XML_DATA_TYPE,   XML_TOK_SORT_GROUPS_ATTR_DATA_TYPE  },
            { XML_NAMESPACE_TABLE, XML_ORDER,       XML_TOK_SORT_GROUPS_ATTR_ORDER      },
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
            { XML_NAMESPACE_TABLE, XML_SUBTOTAL_FIELD,  XML_TOK_SUBTOTAL_RULE_SUBTOTAL_FIELD    },
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
            { XML_NAMESPACE_TABLE, XML_GROUP_BY_FIELD_NUMBER,   XML_TOK_SUBTOTAL_RULE_ATTR_GROUP_BY_FIELD_NUMBER    },
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
            { XML_NAMESPACE_TABLE, XML_FIELD_NUMBER,    XML_TOK_SUBTOTAL_FIELD_ATTR_FIELD_NUMBER    },
            { XML_NAMESPACE_TABLE, XML_FUNCTION,        XML_TOK_SUBTOTAL_FIELD_ATTR_FUNCTION        },
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
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLE,    XML_TOK_DATA_PILOT_TABLE    },
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
            { XML_NAMESPACE_TABLE, XML_NAME,                    XML_TOK_DATA_PILOT_TABLE_ATTR_NAME                  },
            { XML_NAMESPACE_TABLE, XML_APPLICATION_DATA,        XML_TOK_DATA_PILOT_TABLE_ATTR_APPLICATION_DATA      },
            { XML_NAMESPACE_TABLE, XML_GRAND_TOTAL,             XML_TOK_DATA_PILOT_TABLE_ATTR_GRAND_TOTAL           },
            { XML_NAMESPACE_TABLE, XML_IGNORE_EMPTY_ROWS,       XML_TOK_DATA_PILOT_TABLE_ATTR_IGNORE_EMPTY_ROWS     },
            { XML_NAMESPACE_TABLE, XML_IDENTIFY_CATEGORIES,     XML_TOK_DATA_PILOT_TABLE_ATTR_IDENTIFY_CATEGORIES   },
            { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS,    XML_TOK_DATA_PILOT_TABLE_ATTR_TARGET_RANGE_ADDRESS  },
            { XML_NAMESPACE_TABLE, XML_BUTTONS,                 XML_TOK_DATA_PILOT_TABLE_ATTR_BUTTONS               },
            { XML_NAMESPACE_TABLE, XML_SHOW_FILTER_BUTTON,      XML_TOK_DATA_PILOT_TABLE_ATTR_SHOW_FILTER_BUTTON    },
            { XML_NAMESPACE_TABLE, XML_DRILL_DOWN_ON_DOUBLE_CLICK, XML_TOK_DATA_PILOT_TABLE_ATTR_DRILL_DOWN         },
    { XML_NAMESPACE_TABLE, XML_HEADER_GRID_LAYOUT,      XML_TOK_DATA_PILOT_TABLE_ATTR_HEADER_GRID_LAYOUT    },
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
            { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_SQL, XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SQL        },
            { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_TABLE,   XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_TABLE      },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_GRAND_TOTAL,  XML_TOK_DATA_PILOT_TABLE_ELEM_GRAND_TOTAL       },
            { XML_NAMESPACE_TABLE_EXT, XML_DATA_PILOT_GRAND_TOTAL, XML_TOK_DATA_PILOT_TABLE_ELEM_GRAND_TOTAL_EXT },
            { XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_QUERY,   XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_QUERY      },
            { XML_NAMESPACE_TABLE, XML_SOURCE_SERVICE,          XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SERVICE    },
            { XML_NAMESPACE_TABLE, XML_SOURCE_CELL_RANGE,       XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_CELL_RANGE },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_FIELD,        XML_TOK_DATA_PILOT_TABLE_ELEM_DATA_PILOT_FIELD  },
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
            { XML_NAMESPACE_TABLE, XML_NAME,                    XML_TOK_SOURCE_SERVICE_ATTR_NAME                },
            { XML_NAMESPACE_TABLE, XML_SOURCE_NAME,             XML_TOK_SOURCE_SERVICE_ATTR_SOURCE_NAME         },
            { XML_NAMESPACE_TABLE, XML_OBJECT_NAME,             XML_TOK_SOURCE_SERVICE_ATTR_OBJECT_NAME         },
            { XML_NAMESPACE_TABLE, XML_USER_NAME,               XML_TOK_SOURCE_SERVICE_ATTR_USER_NAME           },
            { XML_NAMESPACE_TABLE, XML_PASSWORD,                XML_TOK_SOURCE_SERVICE_ATTR_PASSWORD            },
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
            { XML_NAMESPACE_TABLE,     XML_DISPLAY,      XML_TOK_DATA_PILOT_GRAND_TOTAL_ATTR_DISPLAY          },
            { XML_NAMESPACE_TABLE,     XML_ORIENTATION,  XML_TOK_DATA_PILOT_GRAND_TOTAL_ATTR_ORIENTATION      },
            { XML_NAMESPACE_TABLE,     XML_DISPLAY_NAME, XML_TOK_DATA_PILOT_GRAND_TOTAL_ATTR_DISPLAY_NAME     },
            { XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, XML_TOK_DATA_PILOT_GRAND_TOTAL_ATTR_DISPLAY_NAME_EXT },
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
            { XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, XML_TOK_SOURCE_CELL_RANGE_ATTR_CELL_RANGE_ADDRESS },
            { XML_NAMESPACE_TABLE, XML_NAME, XML_TOK_SOURCE_CELL_RANGE_ATTR_NAME },
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
            { XML_NAMESPACE_TABLE, XML_FILTER,      XML_TOK_SOURCE_CELL_RANGE_ELEM_FILTER},
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
            { XML_NAMESPACE_TABLE,     XML_SOURCE_FIELD_NAME,    XML_TOK_DATA_PILOT_FIELD_ATTR_SOURCE_FIELD_NAME    },
            { XML_NAMESPACE_TABLE,     XML_DISPLAY_NAME,         XML_TOK_DATA_PILOT_FIELD_ATTR_DISPLAY_NAME         },
            { XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME,         XML_TOK_DATA_PILOT_FIELD_ATTR_DISPLAY_NAME_EXT     },
            { XML_NAMESPACE_TABLE,     XML_IS_DATA_LAYOUT_FIELD, XML_TOK_DATA_PILOT_FIELD_ATTR_IS_DATA_LAYOUT_FIELD },
            { XML_NAMESPACE_TABLE,     XML_FUNCTION,             XML_TOK_DATA_PILOT_FIELD_ATTR_FUNCTION             },
            { XML_NAMESPACE_TABLE,     XML_ORIENTATION,          XML_TOK_DATA_PILOT_FIELD_ATTR_ORIENTATION          },
            { XML_NAMESPACE_TABLE,     XML_SELECTED_PAGE,        XML_TOK_DATA_PILOT_FIELD_ATTR_SELECTED_PAGE        },
            { XML_NAMESPACE_LO_EXT,    XML_IGNORE_SELECTED_PAGE, XML_TOK_DATA_PILOT_FIELD_ATTR_IGNORE_SELECTED_PAGE },
            { XML_NAMESPACE_TABLE,     XML_USED_HIERARCHY,       XML_TOK_DATA_PILOT_FIELD_ATTR_USED_HIERARCHY       },
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
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_LEVEL,        XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_LEVEL      },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_FIELD_REFERENCE, XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_REFERENCE },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_GROUPS,       XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_GROUPS },
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
            { XML_NAMESPACE_TABLE, XML_SHOW_EMPTY,              XML_TOK_DATA_PILOT_LEVEL_ATTR_SHOW_EMPTY            },
            { XML_NAMESPACE_CALC_EXT, XML_REPEAT_ITEM_LABELS, XML_TOK_DATA_PILOT_LEVEL_ATTR_REPEAT_ITEM_LABELS    },
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
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTALS,    XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_SUBTOTALS  },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBERS,      XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_MEMBERS    },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_DISPLAY_INFO, XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_DISPLAY_INFO },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_SORT_INFO,    XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_SORT_INFO  },
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_LAYOUT_INFO,  XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_LAYOUT_INFO },
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
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTAL, XML_TOK_DATA_PILOT_SUBTOTALS_ELEM_DATA_PILOT_SUBTOTAL   },
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
            { XML_NAMESPACE_TABLE,     XML_FUNCTION,     XML_TOK_DATA_PILOT_SUBTOTAL_ATTR_FUNCTION         },
            { XML_NAMESPACE_TABLE,     XML_DISPLAY_NAME, XML_TOK_DATA_PILOT_SUBTOTAL_ATTR_DISPLAY_NAME     },
            { XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, XML_TOK_DATA_PILOT_SUBTOTAL_ATTR_DISPLAY_NAME_EXT },
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
            { XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBER,       XML_TOK_DATA_PILOT_MEMBERS_ELEM_DATA_PILOT_MEMBER   },
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
            { XML_NAMESPACE_TABLE,     XML_NAME,         XML_TOK_DATA_PILOT_MEMBER_ATTR_NAME             },
            { XML_NAMESPACE_TABLE,     XML_DISPLAY_NAME, XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY_NAME     },
            { XML_NAMESPACE_TABLE_EXT, XML_DISPLAY_NAME, XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY_NAME_EXT },
            { XML_NAMESPACE_TABLE,     XML_DISPLAY,      XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY          },
            { XML_NAMESPACE_TABLE,     XML_SHOW_DETAILS, XML_TOK_DATA_PILOT_MEMBER_ATTR_SHOW_DETAILS     },
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
            { XML_NAMESPACE_TABLE,  XML_FUNCTION,                       XML_TOK_CONSOLIDATION_ATTR_FUNCTION         },
            { XML_NAMESPACE_TABLE,  XML_SOURCE_CELL_RANGE_ADDRESSES,    XML_TOK_CONSOLIDATION_ATTR_SOURCE_RANGES    },
            { XML_NAMESPACE_TABLE,  XML_TARGET_CELL_ADDRESS,            XML_TOK_CONSOLIDATION_ATTR_TARGET_ADDRESS   },
            { XML_NAMESPACE_TABLE,  XML_USE_LABEL,                      XML_TOK_CONSOLIDATION_ATTR_USE_LABEL        },
            { XML_NAMESPACE_TABLE,  XML_LINK_TO_SOURCE_DATA,            XML_TOK_CONSOLIDATION_ATTR_LINK_TO_SOURCE   },
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
            { XML_NAMESPACE_TEXT, XML_S, XML_TOK_CELL_TEXT_S },
            { XML_NAMESPACE_TEXT, XML_SPAN, XML_TOK_CELL_TEXT_SPAN },
            { XML_NAMESPACE_TEXT, XML_SHEET_NAME, XML_TOK_CELL_TEXT_SHEET_NAME },
            { XML_NAMESPACE_TEXT, XML_DATE, XML_TOK_CELL_TEXT_DATE },
            { XML_NAMESPACE_TEXT, XML_TITLE, XML_TOK_CELL_TEXT_TITLE },
            { XML_NAMESPACE_TEXT, XML_A, XML_TOK_CELL_TEXT_URL },
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
            { XML_NAMESPACE_TEXT, XML_SHEET_NAME, XML_TOK_CELL_TEXT_SPAN_ELEM_SHEET_NAME },
            { XML_NAMESPACE_TEXT, XML_DATE, XML_TOK_CELL_TEXT_SPAN_ELEM_DATE },
            { XML_NAMESPACE_TEXT, XML_TITLE, XML_TOK_CELL_TEXT_SPAN_ELEM_TITLE },
            { XML_NAMESPACE_TEXT, XML_A, XML_TOK_CELL_TEXT_SPAN_ELEM_URL },
            { XML_NAMESPACE_TEXT, XML_S, XML_TOK_CELL_TEXT_SPAN_ELEM_S },
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
            { XML_NAMESPACE_TEXT, XML_STYLE_NAME, XML_TOK_CELL_TEXT_SPAN_ATTR_STYLE_NAME },
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
            { XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_CELL_TEXT_URL_ATTR_UREF },
            { XML_NAMESPACE_XLINK, XML_TYPE, XML_TOK_CELL_TEXT_URL_ATTR_TYPE },
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
            { XML_NAMESPACE_TEXT, XML_C, XML_TOK_CELL_TEXT_S_ATTR_C },
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
            { XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_DATA_STREAM_ATTR_URL },
            { XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS, XML_TOK_DATA_STREAM_ATTR_RANGE },
            { XML_NAMESPACE_CALC_EXT, XML_EMPTY_LINE_REFRESH, XML_TOK_DATA_STREAM_ATTR_EMPTY_LINE_REFRESH },
            { XML_NAMESPACE_CALC_EXT, XML_INSERTION_POSITION, XML_TOK_DATA_STREAM_ATTR_INSERTION_POSITION },
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
    SvXMLImportContext *pContext = nullptr;

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
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    OUString const & implementationName, SvXMLImportFlags nImportFlag)
:   SvXMLImport( rContext, implementationName, nImportFlag ),
    pDoc( nullptr ),
    pChangeTrackingImportHelper(nullptr),
    pStylesImportHelper(nullptr),
    sNumberFormat(SC_UNONAME_NUMFMT),
    sLocale(SC_LOCALE),
    sCellStyle(SC_UNONAME_CELLSTYL),
    pDocElemTokenMap( nullptr ),
    pBodyElemTokenMap( nullptr ),
    pContentValidationsElemTokenMap( nullptr ),
    pContentValidationElemTokenMap( nullptr ),
    pContentValidationAttrTokenMap( nullptr ),
    pContentValidationMessageElemTokenMap( nullptr ),
    pContentValidationHelpMessageAttrTokenMap( nullptr ),
    pContentValidationErrorMessageAttrTokenMap( nullptr ),
    pContentValidationErrorMacroAttrTokenMap( nullptr ),
    pCondFormatsTokenMap( nullptr ),
    pCondFormatTokenMap( nullptr ),
    pCondFormatAttrMap( nullptr ),
    pCondDateAttrMap( nullptr ),
    pConditionAttrMap( nullptr ),
    pColorScaleTokenMap( nullptr ),
    pColorScaleEntryAttrTokenMap( nullptr ),
    pDataBarTokenMap( nullptr ),
    pDataBarAttrMap( nullptr ),
    pFormattingEntryAttrMap( nullptr ),
    pIconSetAttrMap( nullptr ),
    pLabelRangesElemTokenMap( nullptr ),
    pLabelRangeAttrTokenMap( nullptr ),
    pTableElemTokenMap( nullptr ),
    pTableProtectionElemTokenMap(nullptr),
    pTableRowsElemTokenMap( nullptr ),
    pTableColsElemTokenMap( nullptr ),
    pTableScenarioAttrTokenMap( nullptr ),
    pTableAttrTokenMap( nullptr ),
    pTableColAttrTokenMap( nullptr ),
    pTableRowElemTokenMap( nullptr ),
    pTableRowAttrTokenMap( nullptr ),
    pTableRowCellElemTokenMap( nullptr ),
    pTableRowCellAttrTokenMap( nullptr ),
    pTableAnnotationAttrTokenMap( nullptr ),
    pDetectiveElemTokenMap( nullptr ),
    pDetectiveHighlightedAttrTokenMap( nullptr ),
    pDetectiveOperationAttrTokenMap( nullptr ),
    pTableCellRangeSourceAttrTokenMap( nullptr ),
    pNamedExpressionsElemTokenMap( nullptr ),
    pNamedRangeAttrTokenMap( nullptr ),
    pNamedExpressionAttrTokenMap( nullptr ),
    pDatabaseRangesElemTokenMap( nullptr ),
    pDatabaseRangeElemTokenMap( nullptr ),
    pDatabaseRangeAttrTokenMap( nullptr ),
    pDatabaseRangeSourceSQLAttrTokenMap( nullptr ),
    pDatabaseRangeSourceTableAttrTokenMap( nullptr ),
    pDatabaseRangeSourceQueryAttrTokenMap( nullptr ),
    pFilterElemTokenMap( nullptr ),
    pFilterAttrTokenMap( nullptr ),
    pFilterConditionElemTokenMap( nullptr ),
    pFilterConditionAttrTokenMap( nullptr ),
    pFilterSetItemAttrTokenMap( nullptr ),
    pSortElemTokenMap( nullptr ),
    pSortAttrTokenMap( nullptr ),
    pSortSortByAttrTokenMap( nullptr ),
    pDatabaseRangeSubTotalRulesElemTokenMap( nullptr ),
    pDatabaseRangeSubTotalRulesAttrTokenMap( nullptr ),
    pSubTotalRulesSortGroupsAttrTokenMap( nullptr ),
    pSubTotalRulesSubTotalRuleElemTokenMap( nullptr ),
    pSubTotalRulesSubTotalRuleAttrTokenMap( nullptr ),
    pSubTotalRuleSubTotalFieldAttrTokenMap( nullptr ),
    pDataPilotTablesElemTokenMap( nullptr ),
    pDataPilotTableAttrTokenMap( nullptr ),
    pDataPilotTableElemTokenMap( nullptr ),
    pDataPilotTableSourceServiceAttrTokenMap( nullptr ),
    pDataPilotGrandTotalAttrTokenMap(nullptr),
    pDataPilotTableSourceCellRangeElemTokenMap( nullptr ),
    pDataPilotTableSourceCellRangeAttrTokenMap( nullptr ),
    pDataPilotFieldAttrTokenMap( nullptr ),
    pDataPilotFieldElemTokenMap( nullptr ),
    pDataPilotLevelAttrTokenMap( nullptr ),
    pDataPilotLevelElemTokenMap( nullptr ),
    pDataPilotSubTotalsElemTokenMap( nullptr ),
    pDataPilotSubTotalAttrTokenMap( nullptr ),
    pDataPilotMembersElemTokenMap( nullptr ),
    pDataPilotMemberAttrTokenMap( nullptr ),
    pConsolidationAttrTokenMap( nullptr ),
    pCellTextParaElemTokenMap(nullptr),
    pCellTextSpanElemTokenMap(nullptr),
    pCellTextSpanAttrTokenMap(nullptr),
    pCellTextURLAttrTokenMap(nullptr),
    pCellTextSAttrTokenMap(nullptr),
    pDataStreamAttrTokenMap(nullptr),
    mpPostProcessData(nullptr),
    aTables(*this),
    m_pMyNamedExpressions(nullptr),
    pMyLabelRanges(nullptr),
    pValidations(nullptr),
    pDetectiveOpArray(nullptr),
    pSolarMutexGuard(nullptr),
    pNumberFormatAttributesExportHelper(nullptr),
    pStyleNumberFormats(nullptr),
    sPrevStyleName(),
    sPrevCurrency(),
    nSolarMutexLocked(0),
    nProgressCount(0),
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
    for (const auto & aCellTypePair : aCellTypePairs)
    {
        aCellTypeMap.insert(
            CellTypeMap::value_type(
                GetXMLToken(aCellTypePair._token), aCellTypePair._type));
    }
}

ScXMLImport::~ScXMLImport() throw()
{
    //  delete pI18NMap;
    delete pDocElemTokenMap;
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

    delete m_pMyNamedExpressions;
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
    SvXMLImportContext* pContext = nullptr;

    if (getImportFlags() & SvXMLImportFlags::META)
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> const xDocProps(
            (IsStylesOnlyMode()) ? nullptr : xDPS->getDocumentProperties());
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
    SvXMLImportContext* pContext = nullptr;

    if( !(IsStylesOnlyMode()) )
    {
        pContext = new XMLScriptContext( *this, rLocalName, GetModel() );
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
    { "TableCount", "CellCount", "ObjectCount", nullptr };

    SvXMLImport::SetStatistics(i_rStats);

    sal_uInt32 nCount(0);
    for (sal_Int32 i = 0; i < i_rStats.getLength(); ++i) {
        for (const char** pStat = s_stats; *pStat != nullptr; ++pStat) {
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
    SheetNamedExpMap::iterator itr = m_SheetNamedExpressions.find(nTab);
    if (itr == m_SheetNamedExpressions.end())
    {
        // No chain exists for this sheet.  Create one.
        ::std::pair<SheetNamedExpMap::iterator, bool> r =
            m_SheetNamedExpressions.insert(std::make_pair(nTab, o3tl::make_unique<ScMyNamedExpressions>()));
        if (!r.second)
            // insertion failed.
            return;

        itr = r.first;
    }
    ScMyNamedExpressions& r = *itr->second;
    r.push_back(std::move(p));
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

void ScXMLImport::SetChangeTrackingViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& rChangeProps)
{
    if (pDoc)
    {
        sal_Int32 nCount(rChangeProps.getLength());
        if (nCount)
        {
            ScXMLImport::MutexGuard aGuard(*this);
            sal_Int16 nTemp16(0);
            std::unique_ptr<ScChangeViewSettings> pViewSettings(new ScChangeViewSettings());
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
                        pViewSettings->SetTheAuthorToShow(sOUName);
                    }
                }
                else if (sName == "ShowChangesByComment")
                    pViewSettings->SetHasComment(::cppu::any2bool(rChangeProps[i].Value));
                else if (sName == "ShowChangesByCommentText")
                {
                    OUString sOUComment;
                    if (rChangeProps[i].Value >>= sOUComment)
                    {
                        pViewSettings->SetTheComment(sOUComment);
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
                            aBuffer.append("#");
                            aBuffer.append( aLocaleData.getNumThousandSep() );
                            aBuffer.append("##0");
                            aBuffer.append( aLocaleData.getNumDecimalSep() );
                            aBuffer.append("00 [$");
                            aBuffer.append(rCurrency);
                            aBuffer.append("]");
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
                uno::Sequence<OUString> aSeq { sErrorMessage };
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
                    if ( xNumberPropertySet->getPropertyValue(SC_CURRENCYSYMBOL) >>= sTemp)
                    {
                        if (sCurrentCurrency.equals(sTemp))
                            return true;
                        // #i61657# This may be a legacy currency symbol that changed in the meantime.
                        if (SvNumberFormatter::GetLegacyOnlyCurrencyEntry( sCurrentCurrency, sBankSymbol) != nullptr)
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
                        return SvNumberFormatter::GetLegacyOnlyCurrencyEntry( sTemp, sBankSymbol) != nullptr;
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
            XMLTableStyleContext* pStyle = nullptr;
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
            xMultiServiceFactory->createInstance("com.sun.star.sheet.SheetCellRanges"),
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
            sPrevCurrency.clear();
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
            sPrevStyleName.clear();
        if (pCurrency)
            sPrevCurrency = *pCurrency;
        else if(!sPrevCurrency.isEmpty())
            sPrevCurrency.clear();
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
    sPrevStyleName.clear();
}

// XImporter
void SAL_CALL ScXMLImport::setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc )
throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
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

// css::xml::sax::XDocumentHandler
void SAL_CALL ScXMLImport::startDocument()
throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception )
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
            xImportInfo.is() ? xImportInfo->getPropertySetInfo() : nullptr);
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
            uno::Any aColAny = xPropertySet->getPropertyValue(SC_UNO_COLLABELRNG);
            uno::Any aRowAny = xPropertySet->getPropertyValue(SC_UNO_ROWLABELRNG);

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

public:
    RangeNameInserter(ScDocument* pDoc, ScRangeName& rRangeName) :
        mpDoc(pDoc), mrRangeName(rRangeName) {}

    void operator() (const std::unique_ptr<ScMyNamedExpression>& p) const
    {
        using namespace formula;

        const OUString& aType = p->sRangeType;
        sal_uInt32 nUnoType = ScXMLImport::GetRangeType(aType);

        ScRangeData::Type nNewType = ScRangeData::Type::Name;
        if ( nUnoType & sheet::NamedRangeFlag::FILTER_CRITERIA )    nNewType |= ScRangeData::Type::Criteria;
        if ( nUnoType & sheet::NamedRangeFlag::PRINT_AREA )         nNewType |= ScRangeData::Type::PrintArea;
        if ( nUnoType & sheet::NamedRangeFlag::COLUMN_HEADER )      nNewType |= ScRangeData::Type::ColHeader;
        if ( nUnoType & sheet::NamedRangeFlag::ROW_HEADER )         nNewType |= ScRangeData::Type::RowHeader;

        if (mpDoc)
        {
            // Insert a new name.
            ScAddress aPos;
            sal_Int32 nOffset = 0;
            bool bSuccess = ScRangeStringConverter::GetAddressFromString(
                aPos, p->sBaseCellAddress, mpDoc, FormulaGrammar::CONV_OOO, nOffset);

            if (bSuccess)
            {
                OUString aContent = p->sContent;
                if (!p->bIsExpression)
                    ScXMLConverter::ConvertCellRangeAddress(aContent);

                ScRangeData* pData = new ScRangeData(
                    mpDoc, p->sName, aContent, aPos, nNewType, p->eGrammar);
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
    ::std::for_each(pNamedExpressions->begin(), pNamedExpressions->end(), RangeNameInserter(pDoc, *pRangeNames));
}

void ScXMLImport::SetSheetNamedRanges()
{
    if (!pDoc)
        return;

    for (auto const& itr : m_SheetNamedExpressions)
    {
        const SCTAB nTab = itr.first;
        ScRangeName* pRangeNames = pDoc->GetRangeName(nTab);
        if (!pRangeNames)
            continue;

        const ScMyNamedExpressions& rNames = *itr.second;
        ::std::for_each(rNames.begin(), rNames.end(), RangeNameInserter(pDoc, *pRangeNames));
    }
}

void ScXMLImport::SetStringRefSyntaxIfMissing()
{
    if (!pDoc)
        return;

    ScCalcConfig aCalcConfig = pDoc->GetCalcConfig();

    // Has any string ref syntax been imported?
    // If not, we need to take action
    if ( !aCalcConfig.mbHasStringRefSyntax )
    {
        aCalcConfig.meStringRefAddressSyntax = formula::FormulaGrammar::CONV_A1_XL_A1;
        pDoc->SetCalcConfig(aCalcConfig);
    }
}

void SAL_CALL ScXMLImport::endDocument()
    throw(css::xml::sax::SAXException,
          css::uno::RuntimeException,
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
                                OUString sTabName;
                                if(aSeq[i].Value >>= sTabName)
                                {
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
            SetStringRefSyntaxIfMissing();
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
        ScModelObj::getImplementation(GetModel())->AfterXMLLoading();
}

// XEventListener
void ScXMLImport::DisposingModel()
{
    SvXMLImport::DisposingModel();
    pDoc = nullptr;
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
            pSolarMutexGuard = nullptr;
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

void ScXMLImport::ProgressBarIncrement()
{
    nProgressCount++;
    if (nProgressCount > 100)
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
    sal_uInt16 nNsId = GetNamespaceMap().GetKeyByAttrName_( rAttrValue, nullptr, &rFormula, &rFormulaNmsp, false );

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
