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
#include <sal/log.hxx>

#include <svl/zforlist.hxx>

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlmetai.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/streamwrap.hxx>
#include <xmloff/xmlscripti.hxx>
#include <xmloff/XMLFontStylesContext.hxx>
#include <xmloff/DocumentSettingsContext.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/numehelp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/ProgressBarHelper.hxx>

#include <svl/languageoptions.hxx>
#include <editeng/editstat.hxx>
#include <formula/errorcodes.hxx>

#include <appluno.hxx>
#include "xmlimprt.hxx"
#include "importcontext.hxx"
#include <document.hxx>
#include <docsh.hxx>
#include <docuno.hxx>
#include "xmlbodyi.hxx"
#include "xmlstyli.hxx"
#include <ViewSettingsSequenceDefines.hxx>

#include <compiler.hxx>

#include "XMLConverter.hxx"
#include "XMLDetectiveContext.hxx"
#include "XMLTableShapeImportHelper.hxx"
#include "XMLChangeTrackingImportHelper.hxx"
#include <chgviset.hxx>
#include "XMLStylesImportHelper.hxx"
#include <sheetdata.hxx>
#include <rangeutl.hxx>
#include <formulaparserpool.hxx>
#include <externalrefmgr.hxx>
#include <editutil.hxx>
#include "editattributemap.hxx"
#include <documentimport.hxx>
#include "pivotsource.hxx"
#include <unonames.hxx>
#include <numformat.hxx>
#include <sizedev.hxx>
#include <scdll.hxx>
#include "xmlstyle.hxx"

#include <comphelper/base64.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/util/MalformedNumberFormatException.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>

#include <memory>
#include <utility>

#define SC_LOCALE           "Locale"
#define SC_CURRENCYSYMBOL   "CurrencySymbol"
#define SC_REPEAT_ROW "repeat-row"
#define SC_FILTER "filter"
#define SC_PRINT_RANGE "print-range"

using namespace com::sun::star;
using namespace ::xmloff::token;
using namespace ::formula;

OUString ScXMLImport_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisImporter" );
}

uno::Sequence< OUString > ScXMLImport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLImport_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > ScXMLImport_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr )
{
    // return (cppu::OWeakObject*)new ScXMLImport(IMPORT_ALL);
    return static_cast<cppu::OWeakObject*>(new ScXMLImport( comphelper::getComponentContext(rSMgr), ScXMLImport_getImplementationName(), SvXMLImportFlags::ALL ));
}

OUString ScXMLImport_Meta_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisMetaImporter" );
}

uno::Sequence< OUString > ScXMLImport_Meta_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLImport_Meta_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > ScXMLImport_Meta_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr )
{
    // return (cppu::OWeakObject*)new ScXMLImport(IMPORT_META);
    return static_cast<cppu::OWeakObject*>(new ScXMLImport( comphelper::getComponentContext(rSMgr), ScXMLImport_Meta_getImplementationName(), SvXMLImportFlags::META ));
}

OUString ScXMLImport_Styles_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisStylesImporter" );
}

uno::Sequence< OUString > ScXMLImport_Styles_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLImport_Styles_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > ScXMLImport_Styles_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr )
{
    // return (cppu::OWeakObject*)new ScXMLImport(SvXMLImportFlagsSTYLES|SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::MASTERSTYLES|SvXMLImportFlags::FONTDECLS);
    return static_cast<cppu::OWeakObject*>(new ScXMLImport( comphelper::getComponentContext(rSMgr), ScXMLImport_Styles_getImplementationName(), SvXMLImportFlags::STYLES|SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::MASTERSTYLES|SvXMLImportFlags::FONTDECLS));
}

OUString ScXMLImport_Content_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisContentImporter" );
}

uno::Sequence< OUString > ScXMLImport_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLImport_Content_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > ScXMLImport_Content_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr )
{
    // return (cppu::OWeakObject*)new ScXMLImport(SvXMLImportFlags::META|SvXMLImportFlags::STYLES|SvXMLImportFlags::MASTERSTYLES|SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::CONTENT|SvXMLImportFlags::SCRIPTS|SvXMLImportFlags::SETTINGS|SvXMLImportFlags::FONTDECLS);
    return static_cast<cppu::OWeakObject*>(new ScXMLImport( comphelper::getComponentContext(rSMgr), ScXMLImport_Content_getImplementationName(), SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::CONTENT|SvXMLImportFlags::SCRIPTS|SvXMLImportFlags::FONTDECLS));
}

OUString ScXMLImport_Settings_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisSettingsImporter" );
}

uno::Sequence< OUString > ScXMLImport_Settings_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLImport_Settings_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > ScXMLImport_Settings_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr )
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
        pTableRowCellAttrTokenMap.reset(new SvXMLTokenMap( aTableRowCellAttrTokenMap ));
    return *pTableRowCellAttrTokenMap;
}

// NB: virtually inherit so we can multiply inherit properly
//     in ScXMLFlatDocContext_Impl
class ScXMLDocContext_Impl : public virtual SvXMLImportContext
{
protected:
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLDocContext_Impl( ScXMLImport& rImport );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference<xml::sax::XAttributeList>& xAttrList ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
        createFastChildContext( sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList ) override;

    virtual void SAL_CALL startFastElement (sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList) override;

    virtual void SAL_CALL characters(const OUString & aChars) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

ScXMLDocContext_Impl::ScXMLDocContext_Impl( ScXMLImport& rImport ) :
SvXMLImportContext( rImport )
{
}

// context for flat file xml format
class ScXMLFlatDocContext_Impl
    : public ScXMLDocContext_Impl, public SvXMLMetaDocumentContext
{
public:

    ScXMLFlatDocContext_Impl( ScXMLImport& i_rImport,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps);

    virtual void SAL_CALL startFastElement (sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual void SAL_CALL characters( const OUString& aChars ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
        createFastChildContext( sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList ) override;
};

ScXMLFlatDocContext_Impl::ScXMLFlatDocContext_Impl( ScXMLImport& i_rImport,
                                                   const uno::Reference<document::XDocumentProperties>& i_xDocProps) :
SvXMLImportContext(i_rImport),
ScXMLDocContext_Impl(i_rImport),
SvXMLMetaDocumentContext(i_rImport, i_xDocProps)
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    ScXMLFlatDocContext_Impl::createFastChildContext( sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    if ( nElement == XML_ELEMENT( OFFICE, XML_META ) )
        return SvXMLMetaDocumentContext::createFastChildContext( nElement, xAttrList );
    else
        return ScXMLDocContext_Impl::createFastChildContext( nElement, xAttrList );
}

void SAL_CALL ScXMLFlatDocContext_Impl::startFastElement(sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList > & xAttrList)
{
    SvXMLMetaDocumentContext::startFastElement( nElement, xAttrList );
}

void SAL_CALL ScXMLFlatDocContext_Impl::endFastElement(sal_Int32 nElement)
{
    SvXMLMetaDocumentContext::endFastElement( nElement );
}

void SAL_CALL ScXMLFlatDocContext_Impl::characters(const OUString& rChars)
{
    SvXMLMetaDocumentContext::characters(rChars);
}

class ScXMLBodyContext_Impl : public ScXMLImportContext
{
public:
    ScXMLBodyContext_Impl( ScXMLImport& rImport );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
        createFastChildContext( sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList ) override;
};

ScXMLBodyContext_Impl::ScXMLBodyContext_Impl( ScXMLImport& rImport ) :
ScXMLImportContext( rImport )
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    ScXMLBodyContext_Impl::createFastChildContext( sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );
    return GetScImport().CreateBodyContext( pAttribList );
}

SvXMLImportContextRef ScXMLDocContext_Impl::CreateChildContext( sal_uInt16 nPrefix,
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
    case XML_TOK_DOC_SETTINGS:
        if (GetScImport().getImportFlags() & SvXMLImportFlags::SETTINGS)
            pContext = new XMLDocumentSettingsContext(GetScImport(), nPrefix, rLocalName, xAttrList );
        break;
    }

    if(!pContext)
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    ScXMLDocContext_Impl::createFastChildContext( sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    SvXMLImportContext *pContext(nullptr);

    switch( nElement )
    {
        case XML_ELEMENT( OFFICE, XML_BODY ):
        if (GetScImport().getImportFlags() & SvXMLImportFlags::CONTENT)
            pContext = new ScXMLBodyContext_Impl( GetScImport() );
        break;

        //TODO: handle all other cases
        default:
           pContext = new SvXMLImportContext( GetImport() );
    }

    if(!pContext)
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLDocContext_Impl::startFastElement(sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList > & /*xAttrList*/)
{
}

void SAL_CALL ScXMLDocContext_Impl::endFastElement(sal_Int32 /*nElement*/)
{
}

void SAL_CALL ScXMLDocContext_Impl::characters(const OUString &)
{
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

        pDocElemTokenMap.reset(new SvXMLTokenMap( aDocTokenMap ));

    } // if( !pDocElemTokenMap )

    return *pDocElemTokenMap;
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

        pContentValidationElemTokenMap.reset(new SvXMLTokenMap( aContentValidationElemTokenMap ));
    } // if( !pContentValidationElemTokenMap )

    return *pContentValidationElemTokenMap;
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

        pContentValidationMessageElemTokenMap.reset(new SvXMLTokenMap( aContentValidationMessageElemTokenMap ));
    } // if( !pContentValidationMessageElemTokenMap )

    return *pContentValidationMessageElemTokenMap;
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

        pTableElemTokenMap.reset(new SvXMLTokenMap( aTableTokenMap ));
    } // if( !pTableElemTokenMap )

    return *pTableElemTokenMap;
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

        pTableRowsElemTokenMap.reset(new SvXMLTokenMap( aTableRowsElemTokenMap ));
    } // if( !pTableRowsElemTokenMap )

    return *pTableRowsElemTokenMap;
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

        pTableRowElemTokenMap.reset(new SvXMLTokenMap( aTableRowTokenMap ));
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

        pTableRowAttrTokenMap.reset(new SvXMLTokenMap( aTableRowAttrTokenMap ));
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

        pTableRowCellElemTokenMap.reset(new SvXMLTokenMap( aTableRowCellTokenMap ));
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

        pTableAnnotationAttrTokenMap.reset(new SvXMLTokenMap( aTableAnnotationAttrTokenMap ));
    } // if( !pTableAnnotationAttrTokenMap )

    return *pTableAnnotationAttrTokenMap;
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

SvXMLImportContext *ScXMLImport::CreateFastContext( sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    SvXMLImportContext *pContext = nullptr;

    switch( nElement )
    {
    case XML_ELEMENT( OFFICE, XML_DOCUMENT_STYLES ):
    case XML_ELEMENT( OFFICE, XML_DOCUMENT_CONTENT ):
    case XML_ELEMENT( OFFICE, XML_DOCUMENT_SETTINGS ):
        pContext = new ScXMLDocContext_Impl( *this );
        break;

    case XML_ELEMENT( OFFICE, XML_DOCUMENT_META ):
        pContext = CreateMetaContext(nElement);
        break;

    case XML_ELEMENT( OFFICE, XML_DOCUMENT ):
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            GetModel(), uno::UNO_QUERY_THROW);
        // flat OpenDocument file format
        pContext = new ScXMLFlatDocContext_Impl( *this,
            xDPS->getDocumentProperties());
        break;
    }

    default:
        pContext = new SvXMLImportContext( *this );
    }

    return pContext;
}

static constexpr OUStringLiteral gsNumberFormat(SC_UNONAME_NUMFMT);
static constexpr OUStringLiteral gsLocale(SC_LOCALE);
static constexpr OUStringLiteral gsCellStyle(SC_UNONAME_CELLSTYL);

ScXMLImport::ScXMLImport(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    OUString const & implementationName, SvXMLImportFlags nImportFlag)
:   SvXMLImport( rContext, implementationName, nImportFlag ),
    pDoc( nullptr ),
    mpPostProcessData(nullptr),
    aTables(*this),
    sPrevStyleName(),
    sPrevCurrency(),
    nSolarMutexLocked(0),
    nProgressCount(0),
    nPrevCellType(0),
    bLoadDoc( true ),
    bNullDateSetted(false),
    bSelfImportingXMLSet(false),
    mbLockSolarMutex(true),
    mbImportStyles(true),
    mbHasNewCondFormatData(false)
{
    pStylesImportHelper.reset(new ScMyStylesImportHelper(*this));

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
}

ScXMLImport::~ScXMLImport() throw()
{
    //  delete pI18NMap;
    pDocElemTokenMap.reset();
    pContentValidationElemTokenMap.reset();
    pContentValidationMessageElemTokenMap.reset();
    pTableElemTokenMap.reset();
    pTableRowsElemTokenMap.reset();
    pTableRowElemTokenMap.reset();
    pTableRowAttrTokenMap.reset();
    pTableRowCellElemTokenMap.reset();
    pTableRowCellAttrTokenMap.reset();
    pTableAnnotationAttrTokenMap.reset();

    pChangeTrackingImportHelper.reset();
    pNumberFormatAttributesExportHelper.reset();
    pStyleNumberFormats.reset();
    pStylesImportHelper.reset();

    m_pMyNamedExpressions.reset();
    pMyLabelRanges.reset();
    pValidations.reset();
    pDetectiveOpArray.reset();

    //call SvXMLImport dtor contents before deleting pSolarMutexGuard
    cleanup();

    pSolarMutexGuard.reset();
}

void ScXMLImport::initialize( const css::uno::Sequence<css::uno::Any>& aArguments )
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

SvXMLImportContext *ScXMLImport::CreateBodyContext(const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
{
    return new ScXMLBodyContext(*this, rAttrList);
}

SvXMLImportContext *ScXMLImport::CreateMetaContext(
    const sal_Int32 /*nElement*/ )
{
    SvXMLImportContext* pContext = nullptr;

    if (getImportFlags() & SvXMLImportFlags::META)
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> const xDocProps(
            (IsStylesOnlyMode()) ? nullptr : xDPS->getDocumentProperties());
        pContext = new SvXMLMetaDocumentContext(*this, xDocProps);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( *this );

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

sal_Int16 ScXMLImport::GetCellType(const char* rStrValue, const sal_Int32 nStrLength)
{
    sal_Int16 nCellType = util::NumberFormat::UNDEFINED;
    if (rStrValue != nullptr)
    {
        switch (rStrValue[0])
        {
            case 'b':
                if (nStrLength == 7 && !strcmp(rStrValue, "boolean"))
                    nCellType = util::NumberFormat::LOGICAL;
            break;
            case 'c':
                if (nStrLength == 8 && !strcmp(rStrValue, "currency"))
                    nCellType = util::NumberFormat::CURRENCY;
            break;
            case 'd':
                if (nStrLength == 4 && !strcmp(rStrValue, "date"))
                    nCellType = util::NumberFormat::DATETIME;
            break;
            case 'f':
                if (nStrLength == 5 && !strcmp(rStrValue, "float"))
                    nCellType = util::NumberFormat::NUMBER;
            break;
            case 'p':
                if (nStrLength == 10 && !strcmp(rStrValue, "percentage"))
                    nCellType = util::NumberFormat::PERCENT;
            break;
            case 's':
                if (nStrLength == 6 && !strcmp(rStrValue, "string"))
                    nCellType = util::NumberFormat::TEXT;
            break;
            case 't':
                if (nStrLength == 4 && !strcmp(rStrValue, "time"))
                    nCellType = util::NumberFormat::TIME;
            break;
        }
    }

    return nCellType;
}

XMLShapeImportHelper* ScXMLImport::CreateShapeImport()
{
    return new XMLTableShapeImportHelper(*this);
}

bool ScXMLImport::GetValidation(const OUString& sName, ScMyImportValidation& aValidation)
{
    if (pValidations)
    {
        auto aItr = std::find_if(pValidations->begin(), pValidations->end(),
            [&sName](const ScMyImportValidation& rValidation) { return rValidation.sName == sName; });
        if (aItr != pValidations->end())
        {
            // source position must be set as string,
            // so sBaseCellAddress no longer has to be converted here
            aValidation = *aItr;
            return true;
        }
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
            m_SheetNamedExpressions.insert(std::make_pair(nTab, std::make_unique<ScMyNamedExpressions>()));
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
        pChangeTrackingImportHelper.reset(new ScXMLChangeTrackingImportHelper());
    return pChangeTrackingImportHelper.get();
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
    if (nHeight && nWidth && GetModel().is())
    {
        ScModelObj* pDocObj(ScModelObj::getImplementation( GetModel() ));
        if (pDocObj)
        {
            SfxObjectShell* pEmbeddedObj = pDocObj->GetEmbeddedObject();
            if (pEmbeddedObj)
            {
                tools::Rectangle aRect;
                aRect.setX( nLeft );
                aRect.setY( nTop );
                aRect.setWidth( nWidth );
                aRect.setHeight( nHeight );
                pEmbeddedObj->SetVisArea(aRect);
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
            css::uno::Sequence<css::beans::PropertyValue> aFilteredProps(
                aConfigProps.getLength());
            sal_Int32 nFilteredPropsLen = 0;
            for (sal_Int32 i = nCount - 1; i >= 0; --i)
            {
                if (aConfigProps[i].Name == "TrackedChangesProtectionKey")
                {
                    OUString sKey;
                    if (aConfigProps[i].Value >>= sKey)
                    {
                        uno::Sequence<sal_Int8> aPass;
                        ::comphelper::Base64::decode(aPass, sKey);
                        if (aPass.getLength())
                        {
                            if (pDoc->GetChangeTrack())
                                pDoc->GetChangeTrack()->SetProtection(aPass);
                            else
                            {
                                std::set<OUString> aUsers;
                                std::unique_ptr<ScChangeTrack> pTrack( new ScChangeTrack(pDoc, aUsers) );
                                pTrack->SetProtection(aPass);
                                pDoc->SetChangeTrack(std::move(pTrack));
                            }
                        }
                    }
                }
                // store the following items for later use (after document is loaded)
                else if ((aConfigProps[i].Name == "VBACompatibilityMode") || (aConfigProps[i].Name == "ScriptConfiguration"))
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
                    if (GetDocument() && (xProperties->getPropertyValue(gsLocale) >>= aLocale))
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
                OUString sErrorMessage("Error in Formatstring ");
                sErrorMessage += sFormatString;
                sErrorMessage += " at position ";
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
                        if (sCurrentCurrency == sTemp)
                            return true;
                        // A release that saved an unknown currency may have
                        // saved the currency symbol of the number format
                        // instead of an ISO code bank symbol. In another
                        // release we may have a match for that. In this case
                        // sCurrentCurrency is the ISO code obtained through
                        // XMLNumberFormatAttributesExportHelper::GetCellType()
                        // and sBankSymbol is the currency symbol.
                        if (sCurrentCurrency.getLength() == 3 && sBankSymbol == sTemp)
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

void ScXMLImport::SetType(const uno::Reference <beans::XPropertySet>& rProperties,
                          sal_Int32& rNumberFormat,
                          const sal_Int16 nCellType,
                          const OUString& rCurrency)
{
    if (!mbImportStyles)
        return;

    if ((nCellType != util::NumberFormat::TEXT) && (nCellType != util::NumberFormat::UNDEFINED))
    {
        if (rNumberFormat == -1)
            rProperties->getPropertyValue( gsNumberFormat ) >>= rNumberFormat;
        OSL_ENSURE(rNumberFormat != -1, "no NumberFormat");
        bool bIsStandard;
        // sCurrentCurrency may be the ISO code abbreviation if the currency
        // symbol matches such, or if no match found the symbol itself!
        OUString sCurrentCurrency;
        sal_Int32 nCurrentCellType(
            GetNumberFormatAttributesExportHelper()->GetCellType(
                rNumberFormat, sCurrentCurrency, bIsStandard) & ~util::NumberFormat::DEFINED);
        // If the (numeric) cell type (number, currency, date, time, boolean)
        // is different from the format type then for some combinations we may
        // have to apply a format, e.g. in case the generator deduced format
        // from type and did not apply a format but we don't keep a dedicated
        // type internally. Specifically this is necessary if the cell type is
        // not number but the format type is (i.e. General). Currency cells
        // need extra attention, see calls of ScXMLImport::IsCurrencySymbol()
        // and description within there and ScXMLImport::SetCurrencySymbol().
        if ((nCellType != nCurrentCellType) &&
                (nCellType != util::NumberFormat::NUMBER) &&
                (bIsStandard || (nCellType == util::NumberFormat::CURRENCY)))
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
                            if ( xNumberFormatProperties->getPropertyValue(gsLocale) >>= aLocale )
                            {
                                if (!xNumberFormatTypes.is())
                                    xNumberFormatTypes.set(uno::Reference <util::XNumberFormatTypes>(xNumberFormats, uno::UNO_QUERY));
                                rProperties->setPropertyValue( gsNumberFormat, uno::makeAny(xNumberFormatTypes->getStandardFormat(nCellType, aLocale)) );
                            }
                        }
                        else if (!rCurrency.isEmpty() && !sCurrentCurrency.isEmpty())
                        {
                            if (sCurrentCurrency != rCurrency)
                                if (!IsCurrencySymbol(rNumberFormat, sCurrentCurrency, rCurrency))
                                    rProperties->setPropertyValue( gsNumberFormat, uno::makeAny(SetCurrencySymbol(rNumberFormat, rCurrency)));
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
                sCurrentCurrency != rCurrency && !IsCurrencySymbol(rNumberFormat, sCurrentCurrency, rCurrency))
                rProperties->setPropertyValue( gsNumberFormat, uno::makeAny(SetCurrencySymbol(rNumberFormat, rCurrency)));
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

                css::uno::Any aAny = xProperties->getPropertyValue("FormatID");
                sal_uInt64 nKey = 0;
                if ((aAny >>= nKey) && nKey)
                {
                    ScFormatSaveData* pFormatSaveData = ScModelObj::getImplementation(GetModel())->GetFormatSaveData();
                    pFormatSaveData->maIDToName.insert(std::pair<sal_uInt64, OUString>(nKey, sPrevStyleName));
                }

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
                            ScAddress( static_cast<SCCOL>(rRange.StartColumn), static_cast<SCROW>(rRange.StartRow), static_cast<SCTAB>(rRange.Sheet) ) );
                        pStyle->SetLastSheet(rRange.Sheet);
                    }
                }
            }
            else
            {
                xProperties->setPropertyValue(gsCellStyle, uno::makeAny(GetStyleDisplayName( XML_STYLE_FAMILY_TABLE_CELL, sPrevStyleName )));
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
        ((pStyleName && *pStyleName != sPrevStyleName) ||
        (!pStyleName && !sPrevStyleName.isEmpty())) ||
        ((pCurrency && *pCurrency != sPrevCurrency) ||
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
        pNumberFormatAttributesExportHelper.reset(new XMLNumberFormatAttributesExportHelper(GetNumberFormatsSupplier()));
    return pNumberFormatAttributesExportHelper.get();
}

ScMyStyleNumberFormats* ScXMLImport::GetStyleNumberFormats()
{
    if (!pStyleNumberFormats)
        pStyleNumberFormats.reset(new ScMyStyleNumberFormats);
    return pStyleNumberFormats.get();
}

void ScXMLImport::SetStylesToRangesFinished()
{
    SetStyleToRanges();
    sPrevStyleName.clear();
}

// XImporter
void SAL_CALL ScXMLImport::setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc )
{
    ScXMLImport::MutexGuard aGuard(*this);
    SvXMLImport::setTargetDocument( xDoc );

    uno::Reference<frame::XModel> xModel(xDoc, uno::UNO_QUERY);
    pDoc = ScXMLConverter::GetScDocument( xModel );
    OSL_ENSURE( pDoc, "ScXMLImport::setTargetDocument - no ScDocument!" );
    if (!pDoc)
        throw lang::IllegalArgumentException();

    mpDocImport.reset(new ScDocumentImport(*pDoc));
    mpComp.reset(new ScCompiler(pDoc, ScAddress(), formula::FormulaGrammar::GRAM_ODFF));

    uno::Reference<document::XActionLockable> xActionLockable(xDoc, uno::UNO_QUERY);
    if (xActionLockable.is())
        xActionLockable->addActionLock();
}

// css::xml::sax::XDocumentHandler
void SAL_CALL ScXMLImport::startDocument()
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
    sal_Int32 i = 0;
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
    if (pMyLabelRanges)
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

                for (const auto& rxLabelRange : *pMyLabelRanges)
                {
                    sal_Int32 nOffset1(0);
                    sal_Int32 nOffset2(0);
                    FormulaGrammar::AddressConvention eConv = FormulaGrammar::CONV_OOO;

                    if (ScRangeStringConverter::GetRangeFromString( aLabelRange, rxLabelRange->sLabelRangeStr, GetDocument(), eConv, nOffset1 ) &&
                        ScRangeStringConverter::GetRangeFromString( aDataRange, rxLabelRange->sDataRangeStr, GetDocument(), eConv, nOffset2 ))
                    {
                        if ( rxLabelRange->bColumnOrientation )
                            xColRanges->addNew( aLabelRange, aDataRange );
                        else
                            xRowRanges->addNew( aLabelRange, aDataRange );
                    }
                }

                pMyLabelRanges->clear();
            }
        }
    }
}

namespace {

class RangeNameInserter
{
    ScDocument* const mpDoc;
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
    if (!m_pMyNamedExpressions)
        return;

    if (!pDoc)
        return;

    // Insert the namedRanges
    ScRangeName* pRangeNames = pDoc->GetRangeName();
    ::std::for_each(m_pMyNamedExpressions->begin(), m_pMyNamedExpressions->end(), RangeNameInserter(pDoc, *pRangeNames));
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

        // There are rows with optimal height which need to be updated
        if (pDoc && !maRecalcRowRanges.empty())
        {
            bool bLockHeight = pDoc->IsAdjustHeightLocked();
            if (bLockHeight)
            {
                pDoc->UnlockAdjustHeight();
            }

            ScSizeDeviceProvider aProv(static_cast<ScDocShell*>(pDoc->GetDocumentShell()));
            ScDocRowHeightUpdater aUpdater(*pDoc, aProv.GetDevice(), aProv.GetPPTX(), aProv.GetPPTY(), &maRecalcRowRanges);
            aUpdater.update();

            if (bLockHeight)
            {
                pDoc->LockAdjustHeight();
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

    if (pDoc)
    {
        pDoc->BroadcastUno(SfxHint(SfxHintId::ScClearCache));
    }

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
        pSolarMutexGuard.reset(new SolarMutexGuard());
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
            pSolarMutexGuard.reset();
        }
    }
}

sal_Int32 ScXMLImport::GetByteOffset()
{
    sal_Int32 nOffset = -1;
    uno::Reference<xml::sax::XLocator> xLocator = GetLocator();
    uno::Reference<io::XSeekable> xSeek( xLocator, uno::UNO_QUERY );        //! should use different interface
    if ( xSeek.is() )
        nOffset = static_cast<sal_Int32>(xSeek->getPosition());
    return nOffset;
}

void ScXMLImport::SetRangeOverflowType(ErrCode nType)
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

FormulaError ScXMLImport::GetFormulaErrorConstant( const OUString& rStr ) const
{
    if (!mpComp)
        return FormulaError::NONE;

    return mpComp->GetErrorConstant(rStr);
}

ScEditEngineDefaulter* ScXMLImport::GetEditEngine()
{
    if (!mpEditEngine)
    {
        mpEditEngine.reset(new ScEditEngineDefaulter(pDoc->GetEnginePool()));
        mpEditEngine->SetRefMapMode(MapMode(MapUnit::Map100thMM));
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
    if (pDoc)
        pDoc->SetEmbedFonts(true);
}

ScMyImpDetectiveOpArray* ScXMLImport::GetDetectiveOpArray()
{
    if (!pDetectiveOpArray)
        pDetectiveOpArray.reset(new ScMyImpDetectiveOpArray());
    return pDetectiveOpArray.get();
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportFODS(SvStream &rStream)
{
    ScDLL::Init();

    SfxObjectShellLock xDocSh(new ScDocShell);
    xDocSh->DoInitNew();
    uno::Reference<frame::XModel> xModel(xDocSh->GetModel());

    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<io::XInputStream> xStream(new ::utl::OSeekableInputStreamWrapper(rStream));
    uno::Reference<uno::XInterface> xInterface(xMultiServiceFactory->createInstance("com.sun.star.comp.Writer.XmlFilterAdaptor"), uno::UNO_QUERY_THROW);

    css::uno::Sequence<OUString> aUserData(7);
    aUserData[0] = "com.sun.star.comp.filter.OdfFlatXml";
    aUserData[2] = "com.sun.star.comp.Calc.XMLOasisImporter";
    aUserData[3] = "com.sun.star.comp.Calc.XMLOasisExporter";
    aUserData[6] = "true";
    uno::Sequence<beans::PropertyValue> aAdaptorArgs(comphelper::InitPropertySequence(
    {
        { "UserData", uno::Any(aUserData) },
    }));
    css::uno::Sequence<uno::Any> aOuterArgs(1);
    aOuterArgs[0] <<= aAdaptorArgs;

    uno::Reference<lang::XInitialization> xInit(xInterface, uno::UNO_QUERY_THROW);
    xInit->initialize(aOuterArgs);

    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "InputStream", uno::Any(xStream) },
        { "URL", uno::Any(OUString("private:stream")) },
    }));
    xImporter->setTargetDocument(xModel);

    uno::Reference<document::XFilter> xFilter(xInterface, uno::UNO_QUERY_THROW);
    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while its considered uninitialized,
    //setting a property will inform the document its modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    xDocSh->SetLoading(SfxLoadedFlags::NONE);
    bool ret = xFilter->filter(aArgs);
    xDocSh->SetLoading(SfxLoadedFlags::ALL);

    xDocSh->DoClose();

    return ret;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportXLSX(SvStream &rStream)
{
    ScDLL::Init();

    SfxObjectShellLock xDocSh(new ScDocShell);
    xDocSh->DoInitNew();
    uno::Reference<frame::XModel> xModel(xDocSh->GetModel());

    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<io::XInputStream> xStream(new utl::OSeekableInputStreamWrapper(rStream));

    uno::Reference<document::XFilter> xFilter(xMultiServiceFactory->createInstance("com.sun.star.comp.oox.xls.ExcelFilter"), uno::UNO_QUERY_THROW);

    uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "InputStream", uno::makeAny(xStream) },
        { "InputMode", uno::makeAny(true) },
    }));
    xImporter->setTargetDocument(xModel);

    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while its considered uninitialized,
    //setting a property will inform the document its modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    xDocSh->SetLoading(SfxLoadedFlags::NONE);
    bool ret = false;
    try
    {
        ret = xFilter->filter(aArgs);
    }
    catch (const css::io::IOException&)
    {
    }
    catch (const css::lang::WrappedTargetRuntimeException&)
    {
    }
    xDocSh->SetLoading(SfxLoadedFlags::ALL);

    xDocSh->DoClose();

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
