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

#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/namespacemap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include "PropType.hxx"
#include "DeepTContext.hxx"
#include "RenameElemTContext.hxx"
#include "ActionMapTypesOOo.hxx"
#include "MutableAttrList.hxx"
#include "TransformerActions.hxx"
#include "PropertyActionsOOo.hxx"
#include "TransformerBase.hxx"

#include "StyleOOoTContext.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlimp.hxx>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <array>

using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

const sal_uInt16 MAX_PROP_TYPES = 4;
#define ENTRY4(a,b,c,d) \
    { XML_PROP_TYPE_##a, XML_PROP_TYPE_##b, \
        XML_PROP_TYPE_##c, XML_PROP_TYPE_##d  }
#define ENTRY3(a,b,c) \
    { XML_PROP_TYPE_##a, XML_PROP_TYPE_##b,  \
        XML_PROP_TYPE_##c, XML_PROP_TYPE_END }
#define ENTRY2(a,b) \
    { XML_PROP_TYPE_##a, XML_PROP_TYPE_##b, \
        XML_PROP_TYPE_END, XML_PROP_TYPE_END }
#define ENTRY1(a) \
    { XML_PROP_TYPE_##a, XML_PROP_TYPE_##END, \
        XML_PROP_TYPE_END, XML_PROP_TYPE_END }

const std::array<XMLPropType,MAX_PROP_TYPES> aPropTypes[XML_FAMILY_TYPE_END] =
{
    ENTRY3( GRAPHIC, PARAGRAPH, TEXT ),         // XML_FAMILY_TYPE_GRAPHIC,
    ENTRY3( GRAPHIC, PARAGRAPH, TEXT ),         // XML_FAMILY_TYPE_PRESENTATION,
    ENTRY1( DRAWING_PAGE ),                     // XML_FAMILY_TYPE_DRAWING_PAGE,
    ENTRY1( END ),                              // XML_FAMILY_TYPE_MASTER_PAGE
    ENTRY1( PAGE_LAYOUT ),                      // XML_FAMILY_TYPE_PAGE_LAYOUT,
    ENTRY1( HEADER_FOOTER ),                    // XML_FAMILY_TYPE_HEADER_FOOTER
    ENTRY1( TEXT ),                             // XML_FAMILY_TYPE_TEXT,
    ENTRY2( PARAGRAPH, TEXT ),                  // XML_FAMILY_TYPE_PARAGRAPH,
    ENTRY1( RUBY ),                             //XML_FAMILY_TYPE_RUBY,
    ENTRY1( SECTION ),                          // XML_FAMILY_TYPE_SECTION,
    ENTRY1( TABLE ),                            // XML_FAMILY_TYPE_TABLE,
    ENTRY1( TABLE_COLUMN ),                     // XML_FAMILY_TYPE_TABLE_COLUMN,
    ENTRY1( TABLE_ROW ),                        // XML_FAMILY_TYPE_TABLE_ROW,
    ENTRY3( TABLE_CELL, PARAGRAPH, TEXT ),      // XML_FAMILY_TYPE_TABLE_CELL,
    ENTRY1( LIST_LEVEL ),                       // XML_FAMILY_TYPE_LIST,
    ENTRY4( CHART, GRAPHIC, PARAGRAPH, TEXT ),  // XML_FAMILY_TYPE_CHART,
    ENTRY1( TEXT ),                             // XML_FAMILY_TYPE_DATA,
    ENTRY1( END ),                              // XML_FAMILY_TYPE_GRADIENT,
    ENTRY1( END ),                              // XML_FAMILY_TYPE_HATCH,
    ENTRY1( END ),                              // XML_FAMILY_TYPE_FILL_IMAGE,
    ENTRY1( END ),                              // XML_FAMILY_TYPE_STROKE_DASH,
    ENTRY1( END ),                              // XML_FAMILY_TYPE_MARKER,
};

const XMLTokenEnum aPropTokens[XML_PROP_TYPE_END] =
{
    XML_GRAPHIC_PROPERTIES,
    XML_DRAWING_PAGE_PROPERTIES,
    XML_PAGE_LAYOUT_PROPERTIES,
    XML_HEADER_FOOTER_PROPERTIES,
    XML_TEXT_PROPERTIES,
    XML_PARAGRAPH_PROPERTIES,
    XML_RUBY_PROPERTIES,
    XML_SECTION_PROPERTIES,
    XML_TABLE_PROPERTIES,
    XML_TABLE_COLUMN_PROPERTIES,
    XML_TABLE_ROW_PROPERTIES,
    XML_TABLE_CELL_PROPERTIES,
    XML_LIST_LEVEL_PROPERTIES,
    XML_CHART_PROPERTIES
};

const sal_uInt16 aAttrActionMaps[XML_PROP_TYPE_END] =
{
    PROP_OOO_GRAPHIC_ATTR_ACTIONS,
    PROP_OOO_DRAWING_PAGE_ATTR_ACTIONS,     // DRAWING_PAGE
    PROP_OOO_PAGE_LAYOUT_ATTR_ACTIONS,
    PROP_OOO_HEADER_FOOTER_ATTR_ACTIONS,
    PROP_OOO_TEXT_ATTR_ACTIONS,
    PROP_OOO_PARAGRAPH_ATTR_ACTIONS,
    MAX_OOO_PROP_ACTIONS,       // RUBY
    PROP_OOO_SECTION_ATTR_ACTIONS,
    PROP_OOO_TABLE_ATTR_ACTIONS,
    PROP_OOO_TABLE_COLUMN_ATTR_ACTIONS,
    PROP_OOO_TABLE_ROW_ATTR_ACTIONS,
    PROP_OOO_TABLE_CELL_ATTR_ACTIONS,
    PROP_OOO_LIST_LEVEL_ATTR_ACTIONS,
    PROP_OOO_CHART_ATTR_ACTIONS
};

const sal_uInt16 aElemActionMaps[XML_PROP_TYPE_END] =
{
    PROP_OOO_GRAPHIC_ELEM_ACTIONS,
    MAX_OOO_PROP_ACTIONS,
    MAX_OOO_PROP_ACTIONS,
    MAX_OOO_PROP_ACTIONS,
    PROP_OOO_TEXT_ELEM_ACTIONS,
    PROP_OOO_PARAGRAPH_ELEM_ACTIONS,
    MAX_OOO_PROP_ACTIONS,
    MAX_OOO_PROP_ACTIONS,
    MAX_OOO_PROP_ACTIONS,
    MAX_OOO_PROP_ACTIONS,
    MAX_OOO_PROP_ACTIONS,
    PROP_OOO_TABLE_CELL_ELEM_ACTIONS,
    MAX_OOO_PROP_ACTIONS,
    PROP_OOO_CHART_ELEM_ACTIONS
};

namespace {

class XMLTypedPropertiesOOoTContext_Impl : public XMLPersElemContentTContext
{
    css::uno::Reference< css::xml::sax::XFastAttributeList > m_xAttrList;

public:
    XMLTypedPropertiesOOoTContext_Impl( XMLTransformerBase& rTransformer,
                           sal_Int32 rQName );

    using XMLPersAttrListTContext::AddAttribute;
    void AddAttribute( sal_Int32 sName ,
                       const OUString &sValue );

    virtual void startFastElement(sal_Int32 nElement,
                    const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttribs) override;

    virtual void Export() override;
};

}

XMLTypedPropertiesOOoTContext_Impl::XMLTypedPropertiesOOoTContext_Impl(
    XMLTransformerBase& rImp,
    sal_Int32 rQName ) :
    XMLPersElemContentTContext( rImp, rQName ),
    m_xAttrList( new XMLMutableAttributeList() )
{
}

void XMLTypedPropertiesOOoTContext_Impl::AddAttribute(
    sal_Int32 nAttributeToken,
    const OUString &sValue )
{
    static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
            ->AddAttribute( nAttributeToken, sValue );
}

void XMLTypedPropertiesOOoTContext_Impl::startFastElement(sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList > & /*rAttrList*/)
{
    // empty, ignore even the attribute list
}

void XMLTypedPropertiesOOoTContext_Impl::Export()
{
    if( m_xAttrList->getLength() || HasElementContent() )
    {
        GetTransformer().GetDocHandler()->startFastElement( GetQName(), m_xAttrList );
        ExportContent();
        GetTransformer().GetDocHandler()->endFastElement( GetQName() );
    }
}

namespace {

class XMLPropertiesOOoTContext_Impl : public XMLTransformerContext
{
    ::rtl::Reference < XMLTypedPropertiesOOoTContext_Impl >
        m_aPropContexts[MAX_PROP_TYPES];

    using XMLPropTypes = std::array<XMLPropType, MAX_PROP_TYPES>;

    XMLPropTypes const & m_rPropTypes;

    bool const m_bPersistent;

    XMLTypedPropertiesOOoTContext_Impl *GetPropContextAndAction(
            TransformerAction_Impl& rAction,
            sal_Int32 rQName,
            bool bElem );

    XMLTypedPropertiesOOoTContext_Impl *GetPropContext(
            XMLPropType eType );

public:
    XMLPropertiesOOoTContext_Impl( XMLTransformerBase& rTransformer,
                           sal_Int32 rQName,
                               const XMLPropTypes& rTypes,
                               bool bPersistent );

    rtl::Reference<XMLTransformerContext> createFastChildContext(
            sal_Int32 nElement,
            const Reference< XFastAttributeList >& rAttrList ) override;

    virtual void startFastElement(sal_Int32 nElement,
                    const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttribs) override;

    virtual void endFastElement(sal_Int32 nElement) override;

    virtual void Characters( const OUString& rChars ) override;

    virtual void Export() override;

    virtual bool IsPersistent() const override;
};

}

XMLTypedPropertiesOOoTContext_Impl
    *XMLPropertiesOOoTContext_Impl::GetPropContext(
            XMLPropType eType )
{
    sal_uInt16 nIndex = MAX_PROP_TYPES;
    for( sal_uInt16 i=0; i< MAX_PROP_TYPES; i++ )
    {
        if( m_rPropTypes[i] == eType )
        {
            nIndex = i;
            break;
        }
    }
    if( MAX_PROP_TYPES == nIndex )
        return nullptr;

    if( !m_aPropContexts[nIndex].is() )
    {
        m_aPropContexts[nIndex] =
            new XMLTypedPropertiesOOoTContext_Impl(
                    GetTransformer(),
                    XML_ELEMENT(STYLE, aPropTokens[m_rPropTypes[nIndex]] ) );
    }

    return m_aPropContexts[nIndex].get();
}

XMLTypedPropertiesOOoTContext_Impl
    *XMLPropertiesOOoTContext_Impl::GetPropContextAndAction(
            TransformerAction_Impl& rAction,
            sal_Int32 nElement,
            bool bElem )
{
    rAction.m_nActionType = XML_ATACTION_COPY;
    sal_uInt16 nIndex = 0;

    XMLTransformerActions::key_type aKey( nElement );
    sal_uInt16 i=0;
    while( i < MAX_PROP_TYPES && XML_PROP_TYPE_END!=m_rPropTypes[i])
    {
        sal_uInt16 nActionMap =
            (bElem ? aElemActionMaps : aAttrActionMaps)[m_rPropTypes[i]];
        if( nActionMap < MAX_OOO_PROP_ACTIONS )
        {
            XMLTransformerActions *pActions =
                GetTransformer().GetUserDefinedActions( nActionMap );
            OSL_ENSURE( pActions, "go no actions" );
            if( pActions )
            {
                XMLTransformerActions::const_iterator aIter =
                    pActions->find( aKey );

                if( aIter != pActions->end() )
                {
                    rAction = (*aIter).second;
                    nIndex = i;
                    break;
                }
            }
        }
        ++i;
    }

#ifdef DBG_UTIL
    if( !( IsTokenInNamespace(nElement, XML_NAMESPACE_NONE) ||
                XML_PROP_TYPE_END==m_rPropTypes[1] ||
                (i<MAX_PROP_TYPES && XML_PROP_TYPE_END!=m_rPropTypes[i]) ) )
    {
        SAL_WARN("xmloff", "Didn't find property: "
                << SvXMLImport::getPrefixAndNameFromToken(nElement)
                << ", assuming <style:"
                << ::xmloff::token::GetXMLToken( aPropTokens[m_rPropTypes[0]] )
                << ">" );
    }
#endif

    if( !m_aPropContexts[nIndex].is() )
    {
        m_aPropContexts[nIndex] =
            new XMLTypedPropertiesOOoTContext_Impl(
                    GetTransformer(),
                    XML_ELEMENT(STYLE, aPropTokens[m_rPropTypes[nIndex]] ) );
    }

    return m_aPropContexts[nIndex].get();
}

XMLPropertiesOOoTContext_Impl::XMLPropertiesOOoTContext_Impl(
    XMLTransformerBase& rImp,
    sal_Int32 rQName,
    const XMLPropTypes& rTypes,
    bool bPersistent    ) :
    XMLTransformerContext( rImp, rQName ),
    // remember the types that belong to the attribute and element lists
    m_rPropTypes(rTypes),
    m_bPersistent( bPersistent )
{
}

rtl::Reference<XMLTransformerContext> XMLPropertiesOOoTContext_Impl::createFastChildContext(
            sal_Int32 nElement,
            const Reference< XFastAttributeList >& rAttrList )
{
    TransformerAction_Impl aAction;
    return GetPropContextAndAction( aAction, nElement, true )
                ->createFastChildContext( nElement, rAttrList );
}

void XMLPropertiesOOoTContext_Impl::startFastElement(sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList > & rAttrList)
{
    XMLTypedPropertiesOOoTContext_Impl * pIntervalMinorDivisorContext = nullptr;
    double fIntervalMajor = 0.0;
    double fIntervalMinor = 0.0;
    bool bMoveProtect = false;
    bool bSizeProtect = false;
    OUString aProtectAttrValue;
    XMLTypedPropertiesOOoTContext_Impl * pProtectContext = nullptr;

    /* Attribute <style:mirror> has to be priority over attribute <style:draw>.
       The filter from OpenDocument file format to OpenOffice.org file format
       produces styles with both attributes. (#i49139#)
    */
    bool bExistStyleMirror( false );
    OUStringBuffer aStyleMirrorAttrValue;
    bool bExistDrawMirror( false );
    OUString aDrawMirrorAttrValue;
    XMLTypedPropertiesOOoTContext_Impl* pMirrorContext( nullptr );

    sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        sal_Int32 sAttrName = rAttrList->getTokenByIndex( i );
        const OUString sAttrValue = rAttrList->getValueByIndex( i );
        TransformerAction_Impl aAction;
        XMLTypedPropertiesOOoTContext_Impl *pContext =
            GetPropContextAndAction( aAction, sAttrName, false );
        switch( aAction.m_nActionType )
        {
        case XML_ATACTION_REMOVE:
            break;
        case XML_ATACTION_COPY:
            pContext->AddAttribute( sAttrName, sAttrValue );
            break;
        case XML_ATACTION_COPY_DUPLICATE:
            {
                pContext->AddAttribute( sAttrName, sAttrValue );
                XMLTypedPropertiesOOoTContext_Impl *pContext2 =
                    GetPropContext( static_cast<XMLPropType>(aAction.m_nParam1) );
                if( pContext2 )
                    pContext2->AddAttribute( sAttrName, sAttrValue );
            }
            break;
        case XML_ATACTION_RENAME:
            {
                pContext->AddAttribute( aAction.GetTokenFromParam1(),
                                        sAttrValue );
            }
            break;
        case XML_ATACTION_ENCODE_STYLE_NAME_REF:
            {
                OUString aAttrValue( sAttrValue );
                GetTransformer().EncodeStyleName(aAttrValue);
                pContext->AddAttribute( sAttrName, aAttrValue );
            }
            break;
        case XML_ATACTION_RENAME_ENCODE_STYLE_NAME_REF:
            {
                OUString aAttrValue( sAttrValue );
                GetTransformer().EncodeStyleName(aAttrValue);
                pContext->AddAttribute( aAction.GetTokenFromParam1(),
                                        aAttrValue );
            }
            break;
        case XML_ATACTION_NEG_PERCENT:
            {
                OUString aAttrValue( sAttrValue );
                XMLTransformerBase::NegPercent(aAttrValue);
                pContext->AddAttribute( sAttrName, aAttrValue );
            }
            break;
        case XML_ATACTION_RENAME_NEG_PERCENT:
            {
                OUString aAttrValue( sAttrValue );
                XMLTransformerBase::NegPercent(aAttrValue);
                pContext->AddAttribute( aAction.GetTokenFromParam1(),
                                        aAttrValue );
            }
            break;
        case XML_ATACTION_INCH2IN:
            {
                OUString aAttrValue( sAttrValue );
                XMLTransformerBase::ReplaceSingleInchWithIn( aAttrValue );
                pContext->AddAttribute( sAttrName, aAttrValue );
            }
            break;
        case XML_ATACTION_INCH2IN_DUPLICATE:
            {
                OUString aAttrValue( sAttrValue );
                XMLTransformerBase::ReplaceSingleInchWithIn( aAttrValue );
                pContext->AddAttribute( sAttrName, aAttrValue );
                XMLTypedPropertiesOOoTContext_Impl *pContext2 =
                    GetPropContext( static_cast<XMLPropType>(aAction.m_nParam1) );
                if( pContext2 )
                    pContext2->AddAttribute( sAttrName, aAttrValue );
            }
            break;
        case XML_ATACTION_INCHS2INS:
            {
                OUString aAttrValue( sAttrValue );
                XMLTransformerBase::ReplaceInchWithIn( aAttrValue );
                pContext->AddAttribute( sAttrName, aAttrValue );
            }
            break;
        case XML_PTACTION_LINE_MODE:
            {
                const OUString& aAttrValue( GetXMLToken(
                                        IsXMLToken( sAttrValue, XML_TRUE )
                                            ? XML_CONTINUOUS
                                            : XML_SKIP_WHITE_SPACE) );
                sal_Int32 aAttrQName = XML_ELEMENT(STYLE, XML_TEXT_UNDERLINE_MODE);
                pContext->AddAttribute( aAttrQName, aAttrValue );

                aAttrQName = XML_ELEMENT(STYLE, XML_TEXT_LINE_THROUGH_MODE);
                pContext->AddAttribute( aAttrQName, aAttrValue );
            }
            break;
        case XML_PTACTION_KEEP_WITH_NEXT:
            {
                const OUString& aAttrValue( GetXMLToken(
                                        IsXMLToken( sAttrValue, XML_TRUE )
                                            ? XML_ALWAYS
                                            : XML_AUTO) );
                pContext->AddAttribute( sAttrName, aAttrValue );
            }
            break;
        case XML_PTACTION_UNDERLINE:
            {
                XMLTokenEnum eToken = GetTransformer().GetToken( sAttrValue );
                bool bBold = false, bDouble = false;
                switch( eToken )
                {
                case XML_SINGLE:
                    eToken = XML_SOLID;
                    break;
                case XML_DOUBLE:
                    eToken = XML_SOLID;
                    bDouble = true;
                    break;
                case XML_BOLD:
                    eToken = XML_SOLID;
                    bBold = true;
                    break;
                case XML_BOLD_DOTTED:
                    eToken = XML_DOTTED;
                    bBold = true;
                    break;
                case XML_BOLD_DASH:
                    eToken = XML_DASH;
                    bBold = true;
                    break;
                case XML_BOLD_LONG_DASH:
                    eToken = XML_LONG_DASH;
                    bBold = true;
                    break;
                case XML_BOLD_DOT_DASH:
                    eToken = XML_DOT_DASH;
                    bBold = true;
                    break;
                case XML_BOLD_DOT_DOT_DASH:
                    eToken = XML_DOT_DOT_DASH;
                    bBold = true;
                    break;
                case XML_BOLD_WAVE:
                    eToken = XML_WAVE;
                    bBold = true;
                    break;
                case XML_DOUBLE_WAVE:
                    eToken = XML_WAVE;
                    bDouble = true;
                    break;
                case XML_NONE:
                    eToken = XML_NONE;
                    bDouble = false;
                    break;
                default:
                    OSL_FAIL( "xmloff::XMLPropertiesOOoTContext_Impl::startFastElement(), unknown underline token!" );
                    break;
                }
                pContext->AddAttribute(
                        XML_ELEMENT(STYLE, XML_TEXT_UNDERLINE_STYLE),
                        eToken != XML_TOKEN_END ? GetXMLToken( eToken )
                                                   : sAttrValue );
                if( bDouble )
                    pContext->AddAttribute(
                            XML_ELEMENT(STYLE, XML_TEXT_UNDERLINE_TYPE),
                            GetXMLToken( XML_DOUBLE ) );
                if( bBold )
                    pContext->AddAttribute(
                            XML_ELEMENT(STYLE, XML_TEXT_UNDERLINE_WIDTH),
                            GetXMLToken( XML_BOLD ) );
            }
            break;
        case XML_PTACTION_LINETHROUGH:
            {
                XMLTokenEnum eToken = GetTransformer().GetToken( sAttrValue );
                bool bBold = false, bDouble = false;
                sal_Unicode c = 0;
                switch( eToken )
                {
                case XML_SINGLE_LINE:
                    eToken = XML_SOLID;
                    break;
                case XML_DOUBLE_LINE:
                    eToken = XML_SOLID;
                    bDouble = true;
                    break;
                case XML_THICK_LINE:
                    eToken = XML_SOLID;
                    bBold = true;
                    break;
                case XML_SLASH:
                    eToken = XML_SOLID;
                    c = '/';
                    break;
                case XML_uX:
                    eToken = XML_SOLID;
                    c = 'X';
                    break;
                default:
                    break;
                }
                pContext->AddAttribute(
                        XML_ELEMENT(STYLE, XML_TEXT_LINE_THROUGH_STYLE),
                        eToken != XML_TOKEN_END ? GetXMLToken( eToken )
                                                   : sAttrValue );
                if( bDouble )
                    pContext->AddAttribute(
                            XML_ELEMENT(STYLE, XML_TEXT_LINE_THROUGH_TYPE),
                            GetXMLToken( XML_DOUBLE ) );
                if( bBold )
                    pContext->AddAttribute(
                            XML_ELEMENT(STYLE, XML_TEXT_LINE_THROUGH_WIDTH),
                            GetXMLToken( XML_BOLD ) );
                if( c )
                    pContext->AddAttribute(
                            XML_ELEMENT(STYLE, XML_TEXT_LINE_THROUGH_TEXT),
                            OUString( c ) );
            }
            break;
        case XML_PTACTION_SPLINES:
            {
                sal_Int32 nSplineType = sAttrValue.toInt32();
                sal_Int32 aNewAttrName = XML_ELEMENT(CHART, XML_INTERPOLATION);

                switch( nSplineType )
                {
                    // straight lines
                    case 0:
                        pContext->AddAttribute(
                            aNewAttrName, GetXMLToken( XML_NONE ));
                        break;
                    // cubic spline
                    case 1:
                        pContext->AddAttribute(
                            aNewAttrName, GetXMLToken( XML_CUBIC_SPLINE ));
                        break;
                    // B-spline
                    case 2:
                        pContext->AddAttribute(
                            aNewAttrName, GetXMLToken( XML_B_SPLINE ));
                        break;

                    default:
                        OSL_FAIL( "invalid spline type" );
                        pContext->AddAttribute(
                            aNewAttrName, GetXMLToken( XML_NONE ));
                        break;
                }
            }
            break;
        case XML_PTACTION_INTERVAL_MAJOR:
            pContext->AddAttribute( sAttrName, sAttrValue );
            ::sax::Converter::convertDouble( fIntervalMajor, sAttrValue );
            break;
        case XML_PTACTION_INTERVAL_MINOR:
            ::sax::Converter::convertDouble( fIntervalMinor, sAttrValue );
            pIntervalMinorDivisorContext = pContext;
            break;
        case XML_PTACTION_SYMBOL:
            {
                sal_Int32 nSymbolType = sAttrValue.toInt32();
                sal_Int32 aNewAttrName = XML_ELEMENT(CHART, XML_SYMBOL_TYPE);

                if( nSymbolType >= 0 )
                {
                    pContext->AddAttribute( aNewAttrName, GetXMLToken( XML_NAMED_SYMBOL ));
                    enum XMLTokenEnum eToken = XML_TOKEN_INVALID;
                    switch( nSymbolType )
                    {
                        // SYMBOL0
                        case 0:
                            // "square" has an awkward token name
                            eToken = XML_GRADIENTSTYLE_SQUARE;
                            break;
                        // SYMBOL1
                        case 1:
                            eToken = XML_DIAMOND;
                            break;
                        // SYMBOL2
                        case 2:
                            eToken = XML_ARROW_DOWN;
                            break;
                        // SYMBOL3
                        case 3:
                            eToken = XML_ARROW_UP;
                            break;
                        // SYMBOL4
                        case 4:
                            eToken = XML_ARROW_RIGHT;
                            break;
                        // SYMBOL5
                        case 5:
                            eToken = XML_ARROW_LEFT;
                            break;
                        // SYMBOL6
                        case 6:
                            eToken = XML_BOW_TIE;
                            break;
                        // SYMBOL7
                        case 7:
                            eToken = XML_HOURGLASS;
                            break;
                        case 8:
                            eToken = XML_CIRCLE;
                            break;
                        case 9:
                            eToken = XML_STAR;
                            break;
                        case 10:
                            eToken = XML_X;
                            break;
                        case 11:
                            eToken = XML_PLUS;
                            break;
                        case 12:
                            eToken = XML_ASTERISK;
                            break;
                        case 13:
                            eToken = XML_HORIZONTAL_BAR;
                            break;
                        case 14:
                            eToken = XML_VERTICAL_BAR;
                            break;
                        default:
                            OSL_FAIL( "invalid named symbol" );
                            break;
                    }

                    if( eToken != XML_TOKEN_INVALID )
                    {
                        pContext->AddAttribute( XML_ELEMENT(CHART, XML_SYMBOL_NAME),
                                                GetXMLToken( eToken ));
                    }
                }
                else
                {
                    switch( nSymbolType )
                    {
                        // NONE
                        case -3:
                            pContext->AddAttribute(
                                aNewAttrName, GetXMLToken( XML_NONE ));
                            break;
                            // AUTO
                        case -2:
                            pContext->AddAttribute(
                                aNewAttrName, GetXMLToken( XML_AUTOMATIC ));
                            break;
                            // BITMAPURL
                        case -1:
                            pContext->AddAttribute(
                                aNewAttrName, GetXMLToken( XML_IMAGE ));
                            break;
                        default:
                            OSL_FAIL( "invalid symbol type" );
                            pContext->AddAttribute(
                                aNewAttrName, GetXMLToken( XML_NONE ));
                            break;
                    }
                }
            }
            break;
        case XML_PTACTION_SYMBOL_IMAGE_NAME:
            {
                // create an xlink:href element for URI attribute
                XMLPersAttrListTContext *pSymbolImageContext = new XMLPersAttrListTContext(
                    GetTransformer(), XML_ELEMENT(CHART, XML_SYMBOL_IMAGE) );
                rtl::Reference<XMLTransformerContext> xSymbolImageContext(pSymbolImageContext);

                OUString aAttrValue( sAttrValue );
                if( GetTransformer().ConvertURIToOASIS( aAttrValue, true ))
                {
                    pSymbolImageContext->AddAttribute( XML_ELEMENT(XLINK, XML_HREF), aAttrValue );
                    pContext->AddContent(xSymbolImageContext);
                }
            }
            break;

        // #i25616#
        case XML_PTACTION_TRANSPARENCY :
            {
                OUString aAttrValue( sAttrValue );
                XMLTransformerBase::NegPercent(aAttrValue);
                pContext->AddAttribute( XML_ELEMENT(DRAW, XML_OPACITY),
                                        aAttrValue );
                pContext->AddAttribute( XML_ELEMENT(DRAW, XML_IMAGE_OPACITY),
                                        aAttrValue );
            }
            break;

        case XML_PTACTION_BREAK_INSIDE:
            {
                pContext->AddAttribute(
                    XML_ELEMENT(FO, XML_KEEP_TOGETHER),
                    GetXMLToken(
                        IsXMLToken( sAttrValue, XML_COLUMNSPLIT_AVOID )
                        ? XML_ALWAYS
                        : XML_AUTO ) );
            }
            break;
        case XML_ATACTION_CAPTION_ESCAPE_OOO:
            {
                OUString aAttrValue( sAttrValue );
                if( aAttrValue.indexOf( '%' ) != -1 )
                {
                    sal_Int32 nValue = 0;
                    ::sax::Converter::convertPercent( nValue, sAttrValue );
                    if( nValue )
                    {
                        nValue /= 100;
                        OUStringBuffer aOut;
                        ::sax::Converter::convertPercent( aOut, nValue );
                        aAttrValue = aOut.makeStringAndClear();
                    }
                }
                else
                {
                    XMLTransformerBase::ReplaceSingleInchWithIn( aAttrValue );
                }

                pContext->AddAttribute( sAttrName, aAttrValue );
            }
            break;
        case XML_ATACTION_MOVE_PROTECT:
            bMoveProtect = IsXMLToken( sAttrValue, XML_TRUE );
            pProtectContext = pContext;
            break;
        case XML_ATACTION_SIZE_PROTECT:
            bSizeProtect = IsXMLToken( sAttrValue, XML_TRUE );
            pProtectContext = pContext;
            break;
        case XML_ATACTION_PROTECT:
            aProtectAttrValue = sAttrValue;
            pProtectContext = pContext;
            break;
        case XML_ATACTION_DRAW_MIRROR_OOO:   // renames draw:mirror to style:mirror and adapts values
            {
                // OpenDocument file format: attribute value of <style:mirror> wrong (#i49139#)
                aDrawMirrorAttrValue =
                                GetXMLToken( IsXMLToken( sAttrValue, XML_TRUE )
                                             ? XML_HORIZONTAL : XML_NONE );
                bExistDrawMirror = true;
                pMirrorContext = pContext;
            }
            break;
        // OpenDocument file format: attribute value of <style:mirror> wrong (#i49139#)
        case XML_ATACTION_STYLE_MIRROR_OOO:   // adapts style:mirror values
            {
                SvXMLTokenEnumerator aTokenEnum( sAttrValue );
                OUString aToken;
                while( aTokenEnum.getNextToken( aToken ) )
                {
                    if ( !aStyleMirrorAttrValue.isEmpty() )
                    {
                        aStyleMirrorAttrValue.append(" ");
                    }

                    if ( IsXMLToken( aToken, XML_HORIZONTAL_ON_LEFT_PAGES ) )
                    {
                        aStyleMirrorAttrValue.append(GetXMLToken( XML_HORIZONTAL_ON_EVEN ));
                    }
                    else if ( IsXMLToken( aToken, XML_HORIZONTAL_ON_RIGHT_PAGES ) )
                    {
                        aStyleMirrorAttrValue.append(GetXMLToken( XML_HORIZONTAL_ON_ODD ));
                    }
                    else
                    {
                        aStyleMirrorAttrValue.append(aToken);
                    }
                }
                bExistStyleMirror = true;
                pMirrorContext = pContext;
            }
            break;
        case XML_ATACTION_GAMMA_OOO:        // converts double value to percentage
            {
                double fValue = sAttrValue.toDouble();
                sal_Int32 nValue = static_cast<sal_Int32>((fValue * 100.0) + ( fValue > 0 ? 0.5 : - 0.5 ) );

                OUStringBuffer aOut;
                ::sax::Converter::convertPercent( aOut, nValue );
                OUString aAttrValue( aOut.makeStringAndClear() );
                pContext->AddAttribute( sAttrName, aAttrValue );
            }
            break;
        case XML_ATACTION_OPACITY_FIX:
            {
                sal_Int32 nValue;
                if( sAttrValue.indexOf( '%' ) != -1 )
                {
                    ::sax::Converter::convertPercent( nValue, sAttrValue );
                }
                else
                {
                    nValue = sal_Int32( sAttrValue.toDouble() * 100.0 );
                }
                nValue = 100 - nValue;

                OUStringBuffer aOut;
                ::sax::Converter::convertPercent( aOut, nValue );
                pContext->AddAttribute( sAttrName, aOut.makeStringAndClear() );
            }
            break;
        default:
            OSL_ENSURE( false, "unknown action" );
            break;
        }
    }

    // OpenDocument file format: attribute value of <style:mirror> wrong (#i49139#)
    if ( bExistStyleMirror )
    {
        pMirrorContext->AddAttribute(
                        XML_ELEMENT(STYLE, XML_MIRROR),
                        aStyleMirrorAttrValue.makeStringAndClear());
    }
    else if ( bExistDrawMirror )
    {
        pMirrorContext->AddAttribute(
                        XML_ELEMENT(STYLE, XML_MIRROR),
                        aDrawMirrorAttrValue);
    }

    if (bMoveProtect || bSizeProtect || !aProtectAttrValue.isEmpty())
    {
        if( (bMoveProtect ||bSizeProtect) && IsXMLToken( aProtectAttrValue, XML_NONE ) )
            aProtectAttrValue.clear();

        const OUString& rPosition = GetXMLToken( XML_POSITION );
        if( bMoveProtect && -1 == aProtectAttrValue.indexOf( rPosition ) )
        {
            if( !aProtectAttrValue.isEmpty() )
                aProtectAttrValue += " ";
            aProtectAttrValue += rPosition;
        }

        const OUString& rSize = GetXMLToken( XML_SIZE );
        if( bSizeProtect && -1 == aProtectAttrValue.indexOf( rSize ) )
        {
            if( !aProtectAttrValue.isEmpty() )
                aProtectAttrValue += " ";
            aProtectAttrValue += rSize;
        }

        assert(pProtectContext && "coverity[var_deref_model] - pProtectContext should be assigned in a superset of the enclosing if condition entry logic");
        pProtectContext->AddAttribute( XML_ELEMENT(STYLE, XML_PROTECT), aProtectAttrValue );
    }

    if( !pIntervalMinorDivisorContext )
        return;

    if( fIntervalMinor != 0.0 )
    {
        sal_Int32 nIntervalMinorDivisor = static_cast< sal_Int32 >(
            ::rtl::math::round( fIntervalMajor / fIntervalMinor ));

        pIntervalMinorDivisorContext->AddAttribute(
            XML_ELEMENT(CHART, XML_INTERVAL_MINOR_DIVISOR),
            OUString::number( nIntervalMinorDivisor ));
    }
}

void XMLPropertiesOOoTContext_Impl::endFastElement(sal_Int32 )
{
    if( !m_bPersistent )
        Export();
}

void XMLPropertiesOOoTContext_Impl::Characters( const OUString& )
{
    // ignore them
}

void XMLPropertiesOOoTContext_Impl::Export()
{

    for(rtl::Reference<XMLTypedPropertiesOOoTContext_Impl> & rPropContext : m_aPropContexts)
    {
        if( rPropContext.is() )
            rPropContext->Export();
    }
}

bool XMLPropertiesOOoTContext_Impl::IsPersistent() const
{
    return m_bPersistent;
}

XMLStyleOOoTContext::XMLStyleOOoTContext( XMLTransformerBase& rImp,
                                            sal_Int32 rQName,
                                                XMLFamilyType eT,
                                             bool bPersistent ) :
    XMLPersElemContentTContext( rImp, rQName ),
    m_eFamily( eT ),
    m_bPersistent( bPersistent )
{
}

XMLStyleOOoTContext::XMLStyleOOoTContext(
        XMLTransformerBase& rImp,
        sal_Int32 rQName,
        XMLFamilyType eT,
        sal_Int32 rQName2,
           bool bPersistent ) :
    XMLPersElemContentTContext( rImp, rQName, rQName2 ),
    m_eFamily( eT ),
    m_bPersistent( bPersistent )
{
}

XMLStyleOOoTContext::~XMLStyleOOoTContext()
{
}

rtl::Reference<XMLTransformerContext> XMLStyleOOoTContext::createFastChildContext(
            sal_Int32 nElement,
            const Reference< XFastAttributeList >& rAttrList )
{
    rtl::Reference<XMLTransformerContext> pContext;

    if( nElement == XML_ELEMENT(STYLE, XML_PROPERTIES) )
    {
        if( aPropTypes[m_eFamily][0] == XML_PROP_TYPE_END )
        {
            OSL_ENSURE( false, "unexpected properties element" );
            pContext = m_bPersistent
                            ? XMLPersElemContentTContext::createFastChildContext(
                                    nElement, rAttrList )
                            : XMLTransformerContext::createFastChildContext(
                                    nElement, rAttrList );
        }
        else if( aPropTypes[m_eFamily][1] == XML_PROP_TYPE_END )
        {
            sal_uInt16 nActionMap =
                aAttrActionMaps[aPropTypes[m_eFamily][0]];
            if( nActionMap < MAX_OOO_PROP_ACTIONS )
            {
                pContext.set(new XMLPropertiesOOoTContext_Impl(
                                    GetTransformer(), nElement,
                                    aPropTypes[m_eFamily], m_bPersistent ));
            }
            else
            {
                if( m_bPersistent )
                    pContext.set(new XMLPersElemContentTContext(
                                    GetTransformer(), nElement,
                                    XML_NAMESPACE_STYLE,
                                    aPropTokens[aPropTypes[m_eFamily][0]] ));
                else
                    pContext.set(new XMLRenameElemTransformerContext(
                                    GetTransformer(), nElement,
                                    XML_ELEMENT(STYLE, aPropTokens[aPropTypes[m_eFamily][0]]) ));
            }
        }
        else
        {
            pContext.set(new XMLPropertiesOOoTContext_Impl(
                                GetTransformer(), nElement,
                                aPropTypes[m_eFamily], m_bPersistent));
        }

        if( m_bPersistent )
            AddContent( pContext );
    }
    else
    {
        pContext = m_bPersistent
                        ? XMLPersElemContentTContext::createFastChildContext(
                                nElement, rAttrList )
                        : XMLTransformerContext::createFastChildContext(
                                nElement, rAttrList );
    }

    return pContext;
}

void XMLStyleOOoTContext::startFastElement(sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList > & rAttrList)
{
    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OOO_STYLE_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    Reference< XFastAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = nullptr;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        sal_Int32 sAttrName = xAttrList->getTokenByIndex( i );
        XMLTransformerActions::key_type aKey( sAttrName );
        XMLTransformerActions::const_iterator aIter =
            pActions->find( aKey );
        if( aIter != pActions->end() )
        {
            if( !pMutableAttrList )
            {
                pMutableAttrList =
                    new XMLMutableAttributeList( xAttrList );
                xAttrList = pMutableAttrList;
            }
            const OUString sAttrValue = xAttrList->getValueByIndex( i );
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_STYLE_FAMILY:
                {
                    bool bControl = false;
                    if( XML_FAMILY_TYPE_END == m_eFamily )
                    {
                        if( IsXMLToken( sAttrValue, XML_GRAPHICS ) )
                            m_eFamily = XML_FAMILY_TYPE_GRAPHIC;
                        else if( IsXMLToken( sAttrValue, XML_PRESENTATION ) )
                            m_eFamily = XML_FAMILY_TYPE_PRESENTATION;
                        else if( IsXMLToken( sAttrValue, XML_DRAWING_PAGE ) )
                            m_eFamily = XML_FAMILY_TYPE_DRAWING_PAGE;
                        else if( IsXMLToken( sAttrValue, XML_TEXT) )
                            m_eFamily = XML_FAMILY_TYPE_TEXT;
                        else if( IsXMLToken( sAttrValue, XML_PARAGRAPH) )
                            m_eFamily = XML_FAMILY_TYPE_PARAGRAPH;
                        else if( IsXMLToken( sAttrValue, XML_RUBY) )
                            m_eFamily = XML_FAMILY_TYPE_RUBY;
                        else if( IsXMLToken( sAttrValue, XML_SECTION) )
                            m_eFamily = XML_FAMILY_TYPE_SECTION;
                        else if( IsXMLToken( sAttrValue, XML_TABLE) )
                            m_eFamily = XML_FAMILY_TYPE_TABLE;
                        else if( IsXMLToken( sAttrValue, XML_TABLE_COLUMN) )
                            m_eFamily = XML_FAMILY_TYPE_TABLE_COLUMN;
                        else if( IsXMLToken( sAttrValue, XML_TABLE_ROW) )
                            m_eFamily = XML_FAMILY_TYPE_TABLE_ROW;
                        else if( IsXMLToken( sAttrValue, XML_TABLE_CELL) )
                            m_eFamily = XML_FAMILY_TYPE_TABLE_CELL;
                        else if( IsXMLToken( sAttrValue, XML_CHART) )
                            m_eFamily = XML_FAMILY_TYPE_CHART;
                        else if( IsXMLToken( sAttrValue, XML_CONTROL) )
                        {
                            m_eFamily = XML_FAMILY_TYPE_PARAGRAPH;
                            bControl = true;
                        }
                    }
                    if( XML_FAMILY_TYPE_GRAPHIC == m_eFamily )
                    {
                        pMutableAttrList->SetValueByIndex( i,
                                                GetXMLToken( XML_GRAPHIC ) );
                    }
                    else if( bControl )
                    {
                        pMutableAttrList->SetValueByIndex( i,
                                                GetXMLToken( XML_PARAGRAPH ) );
                    }
                }
                break;
            case XML_ATACTION_INCH2IN:
                {
                    OUString aAttrValue( sAttrValue );
                    if( XMLTransformerBase::ReplaceSingleInchWithIn(
                                aAttrValue ) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_ENCODE_STYLE_NAME:
                {
                    OUString aAttrValue( sAttrValue );
                    if( GetTransformer().EncodeStyleName(aAttrValue) )
                    {
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                        sal_Int32 aNewAttrQName = (sAttrName & NMSP_MASK) | XML_DISPLAY_NAME;
                        pMutableAttrList->AddAttribute( aNewAttrQName,
                                                        sAttrValue );
                    }
                }
                break;
            case XML_ATACTION_ENCODE_STYLE_NAME_REF:
                {
                    OUString aAttrValue( sAttrValue );
                    if( GetTransformer().EncodeStyleName(aAttrValue) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_NEG_PERCENT:
                {
                    OUString aAttrValue( sAttrValue );
                    if( XMLTransformerBase::NegPercent(aAttrValue) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_URI_OOO:
                {
                    OUString aAttrValue( sAttrValue );
                    if( GetTransformer().ConvertURIToOASIS( aAttrValue,
                            static_cast< bool >((*aIter).second.m_nParam1)))
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            default:
                OSL_ENSURE( false, "unknown action" );
                break;
            }
        }
    }
    if( XML_FAMILY_TYPE_END == m_eFamily )
        m_eFamily = XML_FAMILY_TYPE_TEXT;
    if( m_bPersistent )
        XMLPersElemContentTContext::startFastElement( nElement, xAttrList );
    else
        GetTransformer().GetDocHandler()->startFastElement( GetExportQName(), xAttrList );
}

void XMLStyleOOoTContext::endFastElement(sal_Int32 nElement)
{
    if( m_bPersistent )
        XMLPersElemContentTContext::endFastElement(nElement);
    else
        GetTransformer().GetDocHandler()->endFastElement( GetExportQName() );
}

void XMLStyleOOoTContext::Characters( const OUString& )
{
    // element content only:
}

bool XMLStyleOOoTContext::IsPersistent() const
{
    return m_bPersistent;
}

XMLTransformerActions *XMLStyleOOoTContext::CreateTransformerActions(
        sal_uInt16 nType )
{
    XMLTransformerActionInit const *pInit = nullptr;

    switch( nType )
    {
    case PROP_OOO_GRAPHIC_ATTR_ACTIONS:
        pInit = aGraphicPropertyOOoAttrActionTable;
        break;
    case PROP_OOO_GRAPHIC_ELEM_ACTIONS:
        pInit = aGraphicPropertyOOoElemActionTable;
        break;
    case PROP_OOO_DRAWING_PAGE_ATTR_ACTIONS:
        pInit = aDrawingPagePropertyOOoAttrActionTable;
        break;
    case PROP_OOO_PAGE_LAYOUT_ATTR_ACTIONS:
        pInit = aPageLayoutPropertyOOoAttrActionTable;
        break;
    case PROP_OOO_HEADER_FOOTER_ATTR_ACTIONS:
        pInit = aHeaderFooterPropertyOOoAttrActionTable;
        break;
    case PROP_OOO_TEXT_ATTR_ACTIONS:
        pInit = aTextPropertyOOoAttrActionTable;
        break;
    case PROP_OOO_TEXT_ELEM_ACTIONS:
        pInit = aTextPropertyOOoElemActionTable;
        break;
    case PROP_OOO_PARAGRAPH_ATTR_ACTIONS:
        pInit = aParagraphPropertyOOoAttrActionTable;
        break;
    case PROP_OOO_PARAGRAPH_ELEM_ACTIONS:
        pInit = aParagraphPropertyOOoElemActionTable;
        break;
    case PROP_OOO_SECTION_ATTR_ACTIONS:
        pInit = aSectionPropertyOOoAttrActionTable;
        break;
    case PROP_OOO_TABLE_ATTR_ACTIONS:
        pInit = aTablePropertyOOoAttrActionTable;
        break;
    case PROP_OOO_TABLE_COLUMN_ATTR_ACTIONS:
        pInit = aTableColumnPropertyOOoAttrActionTable;
        break;
    case PROP_OOO_TABLE_ROW_ATTR_ACTIONS:
        pInit = aTableRowPropertyOOoAttrActionTable;
        break;
    case PROP_OOO_TABLE_CELL_ATTR_ACTIONS:
        pInit = aTableCellPropertyOOoAttrActionTable;
        break;
    case PROP_OOO_TABLE_CELL_ELEM_ACTIONS:
        pInit = aTableCellPropertyOOoElemActionTable;
        break;
    case PROP_OOO_LIST_LEVEL_ATTR_ACTIONS:
        pInit = aListLevelPropertyOOoAttrActionTable;
        break;
    case PROP_OOO_CHART_ATTR_ACTIONS:
        pInit = aChartPropertyOOoAttrActionTable;
        break;
    case PROP_OOO_CHART_ELEM_ACTIONS:
        pInit = aChartPropertyOOoElemActionTable;
        break;
    }

    XMLTransformerActions *pActions = nullptr;
    if( pInit )
        pActions = new XMLTransformerActions( pInit );

    return pActions;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
