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

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"
#include "PropType.hxx"
#include "DeepTContext.hxx"
#include "RenameElemTContext.hxx"
#include "ProcAttrTContext.hxx"
#include "ActionMapTypesOOo.hxx"
#include "MutableAttrList.hxx"
#include "TransformerActions.hxx"
#include "PropertyActionsOOo.hxx"
#include "TransformerBase.hxx"

#include "StyleOOoTContext.hxx"
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

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

static const XMLPropType aPropTypes[XML_FAMILY_TYPE_END][MAX_PROP_TYPES] =
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
    ENTRY1( END )                               // XML_FAMILY_TYPE_PRESENTATION_PAGE_LAYOUT,
};

static const XMLTokenEnum aPropTokens[XML_PROP_TYPE_END] =
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

static const sal_uInt16 aAttrActionMaps[XML_PROP_TYPE_END] =
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

static const sal_uInt16 aElemActionMaps[XML_PROP_TYPE_END] =
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

class XMLTypedPropertiesOOoTContext_Impl : public XMLPersElemContentTContext
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList > m_xAttrList;

public:

    TYPEINFO();

    XMLTypedPropertiesOOoTContext_Impl( XMLTransformerBase& rTransformer,
                           const OUString& rQName );

    virtual ~XMLTypedPropertiesOOoTContext_Impl();

    using XMLPersAttrListTContext::AddAttribute;
    void AddAttribute( const OUString &sName ,
                       const OUString &sValue );
    void AddAttribute( sal_uInt16 nPrefix, XMLTokenEnum eToken,
                       const OUString &sValue );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& rAttrList );

    virtual void Export();
};

TYPEINIT1( XMLTypedPropertiesOOoTContext_Impl, XMLPersElemContentTContext );

XMLTypedPropertiesOOoTContext_Impl::XMLTypedPropertiesOOoTContext_Impl(
    XMLTransformerBase& rImp,
    const OUString& rQName ) :
    XMLPersElemContentTContext( rImp, rQName ),
    m_xAttrList( new XMLMutableAttributeList() )
{
}

XMLTypedPropertiesOOoTContext_Impl::~XMLTypedPropertiesOOoTContext_Impl()
{
}

void XMLTypedPropertiesOOoTContext_Impl::AddAttribute(
    const OUString &sName ,
    const OUString &sValue )
{
    static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
            ->AddAttribute( sName, sValue );
}

void XMLTypedPropertiesOOoTContext_Impl::AddAttribute(
    sal_uInt16 nPrefix, XMLTokenEnum eToken,
    const OUString &sValue )
{
    OUString sName(
            GetTransformer().GetNamespaceMap().GetQNameByKey(
                        nPrefix, ::xmloff::token::GetXMLToken( eToken ) ) );
    static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
            ->AddAttribute( sName, sValue );
}

void XMLTypedPropertiesOOoTContext_Impl::StartElement(
        const Reference< XAttributeList >& )
{
    // empty, ignore even the attribute list
}

void XMLTypedPropertiesOOoTContext_Impl::Export()
{
    if( m_xAttrList->getLength() || HasElementContent() )
    {
        GetTransformer().GetDocHandler()->startElement( GetQName(), m_xAttrList );
        ExportContent();
        GetTransformer().GetDocHandler()->endElement( GetQName() );
    }
}

class XMLPropertiesOOoTContext_Impl : public XMLTransformerContext
{
    ::rtl::Reference < XMLTypedPropertiesOOoTContext_Impl >
        m_aPropContexts[MAX_PROP_TYPES];

    typedef XMLPropType XMLPropTypes[MAX_PROP_TYPES];

    XMLPropTypes m_aPropTypes;

    sal_Bool m_bPersistent;

    XMLTypedPropertiesOOoTContext_Impl *GetPropContextAndAction(
            TransformerAction_Impl& rAction,
            sal_uInt16 nPrefix, const OUString& rLocalName,
            sal_Bool bElem );

    XMLTypedPropertiesOOoTContext_Impl *GetPropContext(
            XMLPropType eType );

public:

    TYPEINFO();

    XMLPropertiesOOoTContext_Impl( XMLTransformerBase& rTransformer,
                           const OUString& rQName,
                               const XMLPropTypes& rTypes,
                               sal_Bool bPersistent );

    virtual ~XMLPropertiesOOoTContext_Impl();

    XMLTransformerContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const OUString& rQName,
            const Reference< XAttributeList >& rAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& rAttrList );

    virtual void EndElement();

    virtual void Characters( const OUString& rChars );

    virtual void Export();

    virtual sal_Bool IsPersistent() const;
};

TYPEINIT1( XMLPropertiesOOoTContext_Impl, XMLTransformerContext );

XMLTypedPropertiesOOoTContext_Impl
    *XMLPropertiesOOoTContext_Impl::GetPropContext(
            XMLPropType eType )
{
    sal_uInt16 nIndex = MAX_PROP_TYPES;
    for( sal_uInt16 i=0; i< MAX_PROP_TYPES; i++ )
    {
        if( m_aPropTypes[i] == eType )
        {
            nIndex = i;
            break;
        }
    }
    if( MAX_PROP_TYPES == nIndex )
        return 0;

    if( !m_aPropContexts[nIndex].is() )
    {
        m_aPropContexts[nIndex] =
            new XMLTypedPropertiesOOoTContext_Impl(
                    GetTransformer(),
                    GetTransformer().GetNamespaceMap().GetQNameByKey(
                        XML_NAMESPACE_STYLE,
                        ::xmloff::token::GetXMLToken(
                            aPropTokens[m_aPropTypes[nIndex]] ) ));
    }

    return m_aPropContexts[nIndex].get();
}

XMLTypedPropertiesOOoTContext_Impl
    *XMLPropertiesOOoTContext_Impl::GetPropContextAndAction(
            TransformerAction_Impl& rAction,
            sal_uInt16 nPrefix, const OUString& rLocalName,
            sal_Bool bElem )
{
    rAction.m_nActionType = XML_ATACTION_COPY;
    sal_uInt16 nIndex = 0;

    XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
    sal_uInt16 i=0;
    while( i < MAX_PROP_TYPES && XML_PROP_TYPE_END!=m_aPropTypes[i])
    {
        sal_uInt16 nActionMap =
            (bElem ? aElemActionMaps : aAttrActionMaps)[m_aPropTypes[i]];
        if( nActionMap < MAX_OOO_PROP_ACTIONS )
        {
            XMLTransformerActions *pActions =
                GetTransformer().GetUserDefinedActions( nActionMap );
            OSL_ENSURE( pActions, "go no actions" );
            if( pActions )
            {
                XMLTransformerActions::const_iterator aIter =
                    pActions->find( aKey );

                if( !(aIter == pActions->end()) )
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
    if( !( XML_NAMESPACE_NONE == nPrefix ||
                (XML_NAMESPACE_UNKNOWN_FLAG & nPrefix) ||
                XML_PROP_TYPE_END==m_aPropTypes[1] ||
                (i<MAX_PROP_TYPES && XML_PROP_TYPE_END!=m_aPropTypes[i]) ) )
    {
        OString aTmp("Didnt't find property: ");
        const OUString& rPrefix =
            GetTransformer().GetNamespaceMap().GetPrefixByKey( nPrefix );
        aTmp += OString( rPrefix.getStr(), rPrefix.getLength(),
                                RTL_TEXTENCODING_ASCII_US );
        aTmp += OString( ':' );
        aTmp += OString( rLocalName.getStr(), rLocalName.getLength(),
                                RTL_TEXTENCODING_ASCII_US );
        aTmp += OString(", assuming <style:");
        const OUString& rName =
            ::xmloff::token::GetXMLToken( aPropTokens[m_aPropTypes[0]] );
        aTmp += OString( rName.getStr(), rName.getLength(),
                                RTL_TEXTENCODING_ASCII_US );
        aTmp += OString( '>' );

        OSL_FAIL(aTmp.getStr());
    }
#endif

    if( !m_aPropContexts[nIndex].is() )
    {
        m_aPropContexts[nIndex] =
            new XMLTypedPropertiesOOoTContext_Impl(
                    GetTransformer(),
                    GetTransformer().GetNamespaceMap().GetQNameByKey(
                        XML_NAMESPACE_STYLE,
                        ::xmloff::token::GetXMLToken(
                            aPropTokens[m_aPropTypes[nIndex]] ) ));
    }

    return m_aPropContexts[nIndex].get();
}

XMLPropertiesOOoTContext_Impl::XMLPropertiesOOoTContext_Impl(
    XMLTransformerBase& rImp,
    const OUString& rQName,
    const XMLPropTypes& rTypes,
    sal_Bool bPersistent    ) :
    XMLTransformerContext( rImp, rQName ),
    m_bPersistent( bPersistent )
{
    for( sal_uInt16 i=0; i < MAX_PROP_TYPES; ++i )
    {
        // remember the types that belong to the attribute and element lists
        m_aPropTypes[i] = rTypes[i];
    }
}

XMLPropertiesOOoTContext_Impl::~XMLPropertiesOOoTContext_Impl()
{
}

XMLTransformerContext *XMLPropertiesOOoTContext_Impl::CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const OUString& rQName,
            const Reference< XAttributeList >& rAttrList )
{
    TransformerAction_Impl aAction;
    return GetPropContextAndAction( aAction, nPrefix, rLocalName, sal_True )
                ->CreateChildContext( nPrefix, rLocalName, rQName, rAttrList );
}

void XMLPropertiesOOoTContext_Impl::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    Reference< XAttributeList > xAttrList( rAttrList );

    XMLTypedPropertiesOOoTContext_Impl * pIntervalMinorDivisorContext = 0;
    double fIntervalMajor = 0.0;
    double fIntervalMinor = 0.0;
    sal_Bool bMoveProtect = sal_False;
    sal_Bool bSizeProtect = sal_False;
    OUString aProtectAttrValue;
    XMLTypedPropertiesOOoTContext_Impl * pProtectContext = 0;

    /* Attribute <style:mirror> has to be priority over attribute <style:draw>.
       The filter from OpenDocument file format to OpenOffice.org file format
       produces styles with both attributes. (#i49139#)
    */
    sal_Bool bExistStyleMirror( sal_False );
    OUString aStyleMirrorAttrValue;
    sal_Bool bExistDrawMirror( sal_False );
    OUString aDrawMirrorAttrValue;
    XMLTypedPropertiesOOoTContext_Impl* pMirrorContext( 0L );

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString sAttrName = xAttrList->getNameByIndex( i );
        const OUString sAttrValue = xAttrList->getValueByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( sAttrName,
                                                                 &aLocalName );
        TransformerAction_Impl aAction;
        XMLTypedPropertiesOOoTContext_Impl *pContext =
            GetPropContextAndAction( aAction, nPrefix, aLocalName, sal_False );
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
                    GetPropContext( (XMLPropType)aAction.m_nParam1 );
                if( pContext2 )
                    pContext2->AddAttribute( sAttrName, sAttrValue );
            }
            break;
        case XML_ATACTION_RENAME:
            {
                pContext->AddAttribute( aAction.GetQNamePrefixFromParam1(),
                                        aAction.GetQNameTokenFromParam1(),
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
                pContext->AddAttribute( aAction.GetQNamePrefixFromParam1(),
                                        aAction.GetQNameTokenFromParam1(),
                                        aAttrValue );
            }
            break;
        case XML_ATACTION_NEG_PERCENT:
            {
                OUString aAttrValue( sAttrValue );
                GetTransformer().NegPercent(aAttrValue);
                pContext->AddAttribute( sAttrName, aAttrValue );
            }
            break;
        case XML_ATACTION_RENAME_NEG_PERCENT:
            {
                OUString aAttrValue( sAttrValue );
                GetTransformer().NegPercent(aAttrValue);
                pContext->AddAttribute( aAction.GetQNamePrefixFromParam1(),
                                        aAction.GetQNameTokenFromParam1(),
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
                    GetPropContext( (XMLPropType)aAction.m_nParam1 );
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
                OUString aAttrValue( GetXMLToken(
                                        IsXMLToken( sAttrValue, XML_TRUE )
                                            ? XML_CONTINUOUS
                                            : XML_SKIP_WHITE_SPACE) );
                OUString aAttrQName(
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_STYLE,
                            GetXMLToken( XML_TEXT_UNDERLINE_MODE ) ) );
                pContext->AddAttribute( aAttrQName, aAttrValue );

                aAttrQName =
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_STYLE,
                            GetXMLToken( XML_TEXT_LINE_THROUGH_MODE ) );
                pContext->AddAttribute( aAttrQName, aAttrValue );
            }
            break;
        case XML_PTACTION_KEEP_WITH_NEXT:
            {
                OUString aAttrValue( GetXMLToken(
                                        IsXMLToken( sAttrValue, XML_TRUE )
                                            ? XML_ALWAYS
                                            : XML_AUTO) );
                pContext->AddAttribute( sAttrName, aAttrValue );
            }
            break;
        case XML_PTACTION_UNDERLINE:
            {
                XMLTokenEnum eToken = GetTransformer().GetToken( sAttrValue );
                sal_Bool bBold = sal_False, bDouble = sal_False;
                switch( eToken )
                {
                case XML_SINGLE:
                    eToken = XML_SOLID;
                    break;
                case XML_DOUBLE:
                    eToken = XML_SOLID;
                    bDouble = sal_True;
                    break;
                case XML_BOLD:
                    eToken = XML_SOLID;
                    bBold = sal_True;
                    break;
                case XML_BOLD_DOTTED:
                    eToken = XML_DOTTED;
                    bBold = sal_True;
                    break;
                case XML_BOLD_DASH:
                    eToken = XML_DASH;
                    bBold = sal_True;
                    break;
                case XML_BOLD_LONG_DASH:
                    eToken = XML_LONG_DASH;
                    bBold = sal_True;
                    break;
                case XML_BOLD_DOT_DASH:
                    eToken = XML_DOT_DASH;
                    bBold = sal_True;
                    break;
                case XML_BOLD_DOT_DOT_DASH:
                    eToken = XML_DOT_DOT_DASH;
                    bBold = sal_True;
                    break;
                case XML_BOLD_WAVE:
                    eToken = XML_WAVE;
                    bBold = sal_True;
                    break;
                case XML_DOUBLE_WAVE:
                    eToken = XML_WAVE;
                    bDouble = sal_True;
                    break;
                case XML_NONE:
                    eToken = XML_NONE;
                    bDouble = sal_False;
                    break;
                default:
                    OSL_FAIL( "xmloff::XMLPropertiesOOoTContext_Impl::StartElement(), unknown underline token!" );
                    break;
                }
                pContext->AddAttribute(
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_STYLE,
                            GetXMLToken( XML_TEXT_UNDERLINE_STYLE ) ),
                        eToken != XML_TOKEN_END ? GetXMLToken( eToken )
                                                   : sAttrValue );
                if( bDouble )
                    pContext->AddAttribute(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_STYLE,
                                GetXMLToken( XML_TEXT_UNDERLINE_TYPE ) ),
                            GetXMLToken( XML_DOUBLE ) );
                if( bBold )
                    pContext->AddAttribute(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_STYLE,
                                GetXMLToken( XML_TEXT_UNDERLINE_WIDTH ) ),
                            GetXMLToken( XML_BOLD ) );
            }
            break;
        case XML_PTACTION_LINETHROUGH:
            {
                XMLTokenEnum eToken = GetTransformer().GetToken( sAttrValue );
                sal_Bool bBold = sal_False, bDouble = sal_False;
                sal_Unicode c = 0;
                switch( eToken )
                {
                case XML_SINGLE_LINE:
                    eToken = XML_SOLID;
                    break;
                case XML_DOUBLE_LINE:
                    eToken = XML_SOLID;
                    bDouble = sal_True;
                    break;
                case XML_THICK_LINE:
                    eToken = XML_SOLID;
                    bBold = sal_True;
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
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_STYLE,
                            GetXMLToken( XML_TEXT_LINE_THROUGH_STYLE ) ),
                        eToken != XML_TOKEN_END ? GetXMLToken( eToken )
                                                   : sAttrValue );
                if( bDouble )
                    pContext->AddAttribute(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_STYLE,
                                GetXMLToken( XML_TEXT_LINE_THROUGH_TYPE ) ),
                            GetXMLToken( XML_DOUBLE ) );
                if( bBold )
                    pContext->AddAttribute(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_STYLE,
                                GetXMLToken( XML_TEXT_LINE_THROUGH_WIDTH ) ),
                            GetXMLToken( XML_BOLD ) );
                if( c )
                    pContext->AddAttribute(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_STYLE,
                                GetXMLToken( XML_TEXT_LINE_THROUGH_TEXT ) ),
                            OUString( c ) );
            }
            break;
        case XML_PTACTION_SPLINES:
            {
                sal_Int32 nSplineType = sAttrValue.toInt32();
                OUString aNewAttrName = GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_CHART, GetXMLToken( XML_INTERPOLATION ) );

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
                OUString aNewAttrName = GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_CHART, GetXMLToken( XML_SYMBOL_TYPE ) );

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
                        pContext->AddAttribute( GetTransformer().GetNamespaceMap().GetQNameByKey(
                                                    XML_NAMESPACE_CHART, GetXMLToken( XML_SYMBOL_NAME )),
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
                    GetTransformer(), GetTransformer().GetNamespaceMap().GetQNameByKey(
                        XML_NAMESPACE_CHART, GetXMLToken( XML_SYMBOL_IMAGE )));

                OUString aAttrValue( sAttrValue );
                if( GetTransformer().ConvertURIToOASIS( aAttrValue, sal_True ))
                {
                    pSymbolImageContext->AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, aAttrValue );
                    pContext->AddContent( pSymbolImageContext );
                }
            }
            break;

        // #i25616#
        case XML_PTACTION_TRANSPARENCY :
            {
                OUString aAttrValue( sAttrValue );
                GetTransformer().NegPercent(aAttrValue);
                pContext->AddAttribute( XML_NAMESPACE_DRAW,
                                        XML_OPACITY,
                                        aAttrValue );
                pContext->AddAttribute( XML_NAMESPACE_DRAW,
                                        XML_IMAGE_OPACITY,
                                        aAttrValue );
            }
            break;

        case XML_PTACTION_BREAK_INSIDE:
            {
                pContext->AddAttribute(
                    XML_NAMESPACE_FO, XML_KEEP_TOGETHER,
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
                bExistDrawMirror = sal_True;
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
                        aStyleMirrorAttrValue += OUString(" " );
                    }

                    if ( IsXMLToken( aToken, XML_HORIZONTAL_ON_LEFT_PAGES ) )
                    {
                        aStyleMirrorAttrValue += GetXMLToken( XML_HORIZONTAL_ON_EVEN );
                    }
                    else if ( IsXMLToken( aToken, XML_HORIZONTAL_ON_RIGHT_PAGES ) )
                    {
                        aStyleMirrorAttrValue += GetXMLToken( XML_HORIZONTAL_ON_ODD );
                    }
                    else
                    {
                        aStyleMirrorAttrValue += aToken;
                    }
                }
                bExistStyleMirror = sal_True;
                pMirrorContext = pContext;
            }
            break;
        case XML_ATACTION_GAMMA_OOO:        // converts double value to percentage
            {
                double fValue = sAttrValue.toDouble();
                sal_Int32 nValue = (sal_Int32)((fValue * 100.0) + ( fValue > 0 ? 0.5 : - 0.5 ) );

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
            OSL_ENSURE( !this, "unknown action" );
            break;
        }
    }

    // OpenDocument file format: attribute value of <style:mirror> wrong (#i49139#)
    if ( bExistStyleMirror )
    {
        pMirrorContext->AddAttribute(
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_STYLE, GetXMLToken( XML_MIRROR ) ),
                        aStyleMirrorAttrValue);
    }
    else if ( bExistDrawMirror )
    {
        pMirrorContext->AddAttribute(
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_STYLE, GetXMLToken( XML_MIRROR ) ),
                        aDrawMirrorAttrValue);
    }

    if( bMoveProtect || bSizeProtect || !aProtectAttrValue.isEmpty() )
    {
        if( (bMoveProtect ||bSizeProtect) && IsXMLToken( aProtectAttrValue, XML_NONE ) )
            aProtectAttrValue = OUString();

        const OUString& rPosition = GetXMLToken( XML_POSITION );
        if( bMoveProtect && -1 == aProtectAttrValue.indexOf( rPosition ) )
        {
            if( !aProtectAttrValue.isEmpty() )
                aProtectAttrValue += OUString( sal_Unicode( ' ' ) );
            aProtectAttrValue += rPosition;
        }

        const OUString& rSize = GetXMLToken( XML_SIZE );
        if( bSizeProtect && -1 == aProtectAttrValue.indexOf( rSize ) )
        {
            if( !aProtectAttrValue.isEmpty() )
                aProtectAttrValue += OUString( sal_Unicode( ' ' ) );
            aProtectAttrValue += rSize;
        }

        pProtectContext->AddAttribute( GetTransformer().GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_STYLE, GetXMLToken( XML_PROTECT ) ), aProtectAttrValue );
    }

    if( pIntervalMinorDivisorContext )
    {
        if( fIntervalMinor != 0.0 )
        {
            sal_Int32 nIntervalMinorDivisor = static_cast< sal_Int32 >(
                ::rtl::math::round( fIntervalMajor / fIntervalMinor ));

            OUStringBuffer aBuf;
            ::sax::Converter::convertNumber( aBuf, nIntervalMinorDivisor );
            pIntervalMinorDivisorContext->AddAttribute(
                GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_CHART,
                    GetXMLToken( XML_INTERVAL_MINOR_DIVISOR )),
                aBuf.makeStringAndClear());
        }
    }
}

void XMLPropertiesOOoTContext_Impl::EndElement()
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

    for( sal_uInt16 i=0; i < MAX_PROP_TYPES; i++ )
    {
        if( m_aPropContexts[i].is() )
            m_aPropContexts[i]->Export();
    }
}

sal_Bool XMLPropertiesOOoTContext_Impl::IsPersistent() const
{
    return m_bPersistent;
}

TYPEINIT1( XMLStyleOOoTContext, XMLPersElemContentTContext );

XMLStyleOOoTContext::XMLStyleOOoTContext( XMLTransformerBase& rImp,
                                            const OUString& rQName,
                                                XMLFamilyType eT,
                                             sal_Bool bPersistent ) :
    XMLPersElemContentTContext( rImp, rQName ),
    m_eFamily( eT ),
    m_bPersistent( bPersistent )
{
}

XMLStyleOOoTContext::XMLStyleOOoTContext(
        XMLTransformerBase& rImp,
          const OUString& rQName,
        XMLFamilyType eT,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
           sal_Bool bPersistent ) :
    XMLPersElemContentTContext( rImp, rQName, nPrefix, eToken ),
    m_eFamily( eT ),
    m_bPersistent( bPersistent )
{
}

XMLStyleOOoTContext::~XMLStyleOOoTContext()
{
}

XMLTransformerContext *XMLStyleOOoTContext::CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const OUString& rQName,
            const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        IsXMLToken( rLocalName, XML_PROPERTIES ) )
    {
        if( aPropTypes[m_eFamily][0] == XML_PROP_TYPE_END )
        {
            OSL_ENSURE( !this, "unexpected properties element" );
            pContext = m_bPersistent
                            ? XMLPersElemContentTContext::CreateChildContext(
                                    nPrefix, rLocalName, rQName, rAttrList )
                            : XMLTransformerContext::CreateChildContext(
                                    nPrefix, rLocalName, rQName, rAttrList );
        }
        else if( aPropTypes[m_eFamily][1] == XML_PROP_TYPE_END )
        {
            sal_uInt16 nActionMap =
                aAttrActionMaps[aPropTypes[m_eFamily][0]];
            if( nActionMap < MAX_OOO_PROP_ACTIONS )
            {
                pContext = new XMLPropertiesOOoTContext_Impl(
                                    GetTransformer(), rQName,
                                    aPropTypes[m_eFamily], m_bPersistent );
            }
            else
            {
                if( m_bPersistent )
                    pContext = new XMLPersElemContentTContext(
                                    GetTransformer(), rQName,
                                    XML_NAMESPACE_STYLE,
                                    aPropTokens[aPropTypes[m_eFamily][0]] );
                else
                    pContext = new XMLRenameElemTransformerContext(
                                    GetTransformer(), rQName,
                                    XML_NAMESPACE_STYLE,
                                    aPropTokens[aPropTypes[m_eFamily][0]] );
            }
        }
        else
        {
            pContext = new XMLPropertiesOOoTContext_Impl(
                                GetTransformer(), rQName,
                                aPropTypes[m_eFamily], m_bPersistent);
        }

        if( m_bPersistent )
            AddContent( pContext );
    }
    else
    {
        pContext = m_bPersistent
                        ? XMLPersElemContentTContext::CreateChildContext(
                                nPrefix, rLocalName, rQName, rAttrList )
                        : XMLTransformerContext::CreateChildContext(
                                nPrefix, rLocalName, rQName, rAttrList );
    }

    return pContext;
}

void XMLStyleOOoTContext::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OOO_STYLE_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    Reference< XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = 0;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( sAttrName,
                                                                 &aLocalName );
        XMLTransformerActions::key_type aKey( nPrefix, aLocalName );
        XMLTransformerActions::const_iterator aIter =
            pActions->find( aKey );
        if( !(aIter == pActions->end() ) )
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
                    sal_Bool bControl = sal_False;
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
                            bControl = sal_True;
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
                        OUString aNewAttrQName(
                            GetTransformer().GetNamespaceMap().
                                GetQNameByKey(
                                    nPrefix, ::xmloff::token::GetXMLToken(
                                        XML_DISPLAY_NAME ) ) );
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
                    if( GetTransformer().NegPercent(aAttrValue) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_URI_OOO:
                {
                    OUString aAttrValue( sAttrValue );
                    if( GetTransformer().ConvertURIToOASIS( aAttrValue,
                            static_cast< sal_Bool >((*aIter).second.m_nParam1)))
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            default:
                OSL_ENSURE( !this, "unknown action" );
                break;
            }
        }
    }
    if( XML_FAMILY_TYPE_END == m_eFamily )
        m_eFamily = XML_FAMILY_TYPE_TEXT;
    if( m_bPersistent )
        XMLPersElemContentTContext::StartElement( xAttrList );
    else
        GetTransformer().GetDocHandler()->startElement( GetExportQName(),
                                                        xAttrList );
}

void XMLStyleOOoTContext::EndElement()
{
    if( m_bPersistent )
        XMLPersElemContentTContext::EndElement();
    else
        GetTransformer().GetDocHandler()->endElement( GetExportQName() );
}

void XMLStyleOOoTContext::Characters( const OUString& )
{
    // element content only:
}

sal_Bool XMLStyleOOoTContext::IsPersistent() const
{
    return m_bPersistent;
}

XMLTransformerActions *XMLStyleOOoTContext::CreateTransformerActions(
        sal_uInt16 nType )
{
    XMLTransformerActionInit *pInit = 0;

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

    XMLTransformerActions *pActions = 0;
    if( pInit )
        pActions = new XMLTransformerActions( pInit );

    return pActions;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
