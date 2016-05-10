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
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <sax/tools/converter.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include "PropType.hxx"
#include "DeepTContext.hxx"
#include "ProcAttrTContext.hxx"
#include "TransformerBase.hxx"
#include "TransformerActions.hxx"
#include "ActionMapTypesOASIS.hxx"
#include "MutableAttrList.hxx"
#include "PropertyActionsOASIS.hxx"
#include "StyleOASISTContext.hxx"
#include <xmloff/xmluconv.hxx>

using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

static const sal_uInt16 aAttrActionMaps[XML_PROP_TYPE_END] =
{
    PROP_OASIS_GRAPHIC_ATTR_ACTIONS,
    PROP_OASIS_DRAWING_PAGE_ATTR_ACTIONS,               // DRAWING_PAGE
    PROP_OASIS_PAGE_LAYOUT_ATTR_ACTIONS,
    PROP_OASIS_HEADER_FOOTER_ATTR_ACTIONS,
    PROP_OASIS_TEXT_ATTR_ACTIONS,
    PROP_OASIS_PARAGRAPH_ATTR_ACTIONS,
    MAX_OASIS_PROP_ACTIONS,             // RUBY
    PROP_OASIS_SECTION_ATTR_ACTIONS,
    PROP_OASIS_TABLE_ATTR_ACTIONS,
    PROP_OASIS_TABLE_COLUMN_ATTR_ACTIONS,
    PROP_OASIS_TABLE_ROW_ATTR_ACTIONS,
    PROP_OASIS_TABLE_CELL_ATTR_ACTIONS,
    PROP_OASIS_LIST_LEVEL_ATTR_ACTIONS,
    PROP_OASIS_CHART_ATTR_ACTIONS
};

class XMLPropertiesTContext_Impl : public XMLPersElemContentTContext
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList > m_xAttrList;

    XMLPropType m_ePropType;
    bool        m_bControlStyle;
    OUString m_aStyleFamily;

public:

    void SetQNameAndPropType( const OUString& rQName,
                                 XMLPropType ePropType  )
    {
        m_ePropType = ePropType;
        XMLTransformerContext::SetQName( rQName );
    };

    XMLPropertiesTContext_Impl( XMLTransformerBase& rTransformer,
                           const OUString& rQName,
                           XMLPropType eP,
                           const OUString& rStyleFamily,
                           bool _bControlStyle = false );

    virtual ~XMLPropertiesTContext_Impl();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) override;

    virtual void Export() override;

    static XMLPropType GetPropType( const OUString& rLocalName );

    static OUString MergeUnderline( XMLTokenEnum eUnderline,
                                           bool bBold, bool bDouble );
    static OUString MergeLineThrough( XMLTokenEnum eLineThrough,
                                        bool bBold, bool bDouble,
                                           sal_Unicode c );
};

XMLPropertiesTContext_Impl::XMLPropertiesTContext_Impl(
    XMLTransformerBase& rImp, const OUString& rQName, XMLPropType eP,
        const OUString& rStyleFamily, bool _bControlStyle ) :
    XMLPersElemContentTContext( rImp, rQName, XML_NAMESPACE_STYLE,
                                XML_PROPERTIES),
    m_ePropType( eP ),
    m_bControlStyle( _bControlStyle ),
    m_aStyleFamily( rStyleFamily )
{
}

XMLPropertiesTContext_Impl::~XMLPropertiesTContext_Impl()
{
}

void XMLPropertiesTContext_Impl::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerActions *pActions =  nullptr;
    sal_uInt16 nActionMap = aAttrActionMaps[m_ePropType];
    if( nActionMap < MAX_OASIS_PROP_ACTIONS )
    {
        pActions = GetTransformer().GetUserDefinedActions( nActionMap );
        OSL_ENSURE( pActions, "go no actions" );
    }

    if( pActions )
    {
        XMLMutableAttributeList *pAttrList = nullptr;
        if( !m_xAttrList.is() )
        {
            pAttrList = new XMLMutableAttributeList();
            m_xAttrList = pAttrList;
        }
        else
        {
            pAttrList =
                static_cast< XMLMutableAttributeList * >( m_xAttrList.get() );
        }

        XMLTokenEnum eUnderline = XML_TOKEN_END;
        bool bBoldUnderline = false, bDoubleUnderline = false;
        XMLTokenEnum eLineThrough = XML_TOKEN_END;
        bool bBoldLineThrough = false, bDoubleLineThrough = false;
        sal_Unicode cLineThroughChar = 0;

        bool bIntervalMinorFound = false;
        double fIntervalMajor = 0.0;
        sal_Int32 nIntervalMinorDivisor = 0;

        // #i25616#
        OUString aOpacityValueRemember;
        OUString aImageOpacityValueRemember;

        sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            const OUString& rAttrName = rAttrList->getNameByIndex( i );
            const OUString& rAttrValue = rAttrList->getValueByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix =
                GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                 &aLocalName );

            XMLTransformerActions::key_type aKey( nPrefix, aLocalName );
            XMLTransformerActions::const_iterator aIter =
                pActions->find( aKey );
            if( !(aIter == pActions->end() ) )
            {
                switch( (*aIter).second.m_nActionType )
                {
                case XML_ATACTION_REMOVE:
                    break;
                case XML_ATACTION_COPY:
                    pAttrList->AddAttribute( rAttrName, rAttrValue );
                    break;
                case XML_ATACTION_RENAME:
                    {
                        OUString aNewAttrQName(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                (*aIter).second.GetQNamePrefixFromParam1(),
                                ::xmloff::token::GetXMLToken(
                                (*aIter).second.GetQNameTokenFromParam1()) ) );
                        pAttrList->AddAttribute( aNewAttrQName, rAttrValue );
                    }
                    break;
                case XML_ATACTION_IN2INCH:
                    {
                        OUString aAttrValue( rAttrValue );
                        XMLTransformerBase::ReplaceSingleInWithInch(
                                aAttrValue );
                        pAttrList->AddAttribute( rAttrName, aAttrValue );
                    }
                    break;
                case XML_ATACTION_INS2INCHS:
                    {
                        OUString aAttrValue( rAttrValue );
                        XMLTransformerBase::ReplaceInWithInch(
                                aAttrValue );
                        pAttrList->AddAttribute( rAttrName, aAttrValue );
                    }
                    break;
                case XML_ATACTION_DECODE_STYLE_NAME_REF:
                    {
                        OUString aAttrValue( rAttrValue );
                        XMLTransformerBase::DecodeStyleName(aAttrValue);
                        pAttrList->AddAttribute( rAttrName, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RENAME_DECODE_STYLE_NAME_REF:
                    {
                        OUString aNewAttrQName(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                (*aIter).second.GetQNamePrefixFromParam1(),
                                ::xmloff::token::GetXMLToken(
                                (*aIter).second.GetQNameTokenFromParam1()) ) );
                        OUString aAttrValue( rAttrValue );
                        XMLTransformerBase::DecodeStyleName(aAttrValue);
                        pAttrList->AddAttribute( aNewAttrQName, aAttrValue );
                    }
                    break;
                case XML_ATACTION_NEG_PERCENT:
                    {
                        OUString aAttrValue( rAttrValue );
                        XMLTransformerBase::NegPercent(aAttrValue);
                        pAttrList->AddAttribute( rAttrName, aAttrValue );
                    }
                    break;
                case XML_ATACTION_RENAME_NEG_PERCENT:
                    {
                        OUString aNewAttrQName(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                (*aIter).second.GetQNamePrefixFromParam1(),
                                ::xmloff::token::GetXMLToken(
                                (*aIter).second.GetQNameTokenFromParam1()) ) );
                        OUString aAttrValue( rAttrValue );
                        XMLTransformerBase::NegPercent(aAttrValue);
                        pAttrList->AddAttribute( aNewAttrQName, aAttrValue );
                    }
                    break;
                case XML_OPTACTION_LINE_MODE:
                    {
                        bool bWordMode =
                            IsXMLToken( rAttrValue, XML_SKIP_WHITE_SPACE );
                        OUString aAttrQName(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_FO,
                                GetXMLToken( XML_SCORE_SPACES ) ) );
                        sal_Int16 nIndex =
                            pAttrList->GetIndexByName( aAttrQName );
                        if( -1 != nIndex )
                        {
                            if( bWordMode )
                            {
                                const OUString& rOldValue =
                                    pAttrList->getValueByIndex( nIndex );
                                if( !IsXMLToken( rOldValue, XML_TRUE ) )
                                {
                                    pAttrList->SetValueByIndex( nIndex,
                                            GetXMLToken( XML_TRUE ) );
                                }
                            }
                        }
                        else
                        {
                            OUString aAttrValue( GetXMLToken( bWordMode
                                        ? XML_FALSE
                                        : XML_TRUE ) );
                            pAttrList->AddAttribute( aAttrQName, aAttrValue );
                        }
                    }
                    break;
                case XML_OPTACTION_KEEP_WITH_NEXT:
                    {
                        OUString aAttrValue( GetXMLToken(
                                        IsXMLToken( rAttrValue, XML_ALWAYS )
                                                    ? XML_TRUE
                                                    : XML_FALSE) );
                        pAttrList->AddAttribute( rAttrName, aAttrValue );
                    }
                    break;
                case XML_OPTACTION_UNDERLINE_WIDTH:
                    if( IsXMLToken( rAttrValue, XML_BOLD ) )
                        bBoldUnderline = true;
                    break;
                case XML_OPTACTION_UNDERLINE_TYPE:
                    if( IsXMLToken( rAttrValue, XML_DOUBLE ) )
                        bDoubleUnderline = true;
                    break;
                case XML_OPTACTION_UNDERLINE_STYLE:
                    eUnderline = GetTransformer().GetToken( rAttrValue );
                    break;
                case XML_OPTACTION_LINETHROUGH_WIDTH:
                    if( IsXMLToken( rAttrValue, XML_BOLD ) )
                        bBoldLineThrough = true;
                    break;
                case XML_OPTACTION_LINETHROUGH_TYPE:
                    if( IsXMLToken( rAttrValue, XML_DOUBLE ) )
                        bDoubleLineThrough = true;
                    break;
                case XML_OPTACTION_LINETHROUGH_STYLE:
                    eLineThrough = GetTransformer().GetToken( rAttrValue );
                    break;
                case XML_OPTACTION_LINETHROUGH_TEXT:
                    if( !rAttrValue.isEmpty() )
                        cLineThroughChar = rAttrValue[0];
                    break;
                case XML_OPTACTION_INTERPOLATION:
                    {
                        // 0: none (default)
                        sal_Int32 nSplineType = 0;
                        if( IsXMLToken( rAttrValue, XML_CUBIC_SPLINE ))
                            nSplineType = 1;
                        else if( IsXMLToken( rAttrValue, XML_B_SPLINE ))
                            nSplineType = 2;

                        pAttrList->AddAttribute(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_CHART,
                                GetXMLToken( XML_SPLINES )),
                            OUString::number( nSplineType ));
                    }
                    break;
                case XML_OPTACTION_INTERVAL_MAJOR:
                    pAttrList->AddAttribute( rAttrName, rAttrValue );
                    ::sax::Converter::convertDouble(fIntervalMajor, rAttrValue);
                    break;
                case XML_OPTACTION_INTERVAL_MINOR_DIVISOR:
                    ::sax::Converter::convertNumber(nIntervalMinorDivisor, rAttrValue);
                    bIntervalMinorFound = true;
                    break;
                case XML_OPTACTION_SYMBOL_TYPE:
                    {
                        // if symbol_type is "named-symbol" the "symbol"
                        // property is set in the action XML_OPTACTION_SYMBOL_NAME
                        sal_Int32 nSymbolType = 0;
                        if( IsXMLToken( rAttrValue, XML_NONE ))
                            nSymbolType = -3;
                        else if( IsXMLToken( rAttrValue, XML_AUTOMATIC ))
                            nSymbolType = -2;
                        else if( IsXMLToken( rAttrValue, XML_IMAGE ))
                            nSymbolType = -1;

                        if( nSymbolType < 0 )
                            pAttrList->AddAttribute(
                                GetTransformer().GetNamespaceMap().GetQNameByKey(
                                    XML_NAMESPACE_CHART,
                                    GetXMLToken( XML_SYMBOL )),
                                OUString::number( nSymbolType ));
                    }
                    break;
                case XML_OPTACTION_SYMBOL_NAME:
                    {
                        // assume "symbol-type" == "named-symbol"
                        sal_Int32 nSymbolType = -3; // NONE
                        // "square" just has an awkward token-name
                        if( IsXMLToken( rAttrValue, XML_GRADIENTSTYLE_SQUARE ))
                            nSymbolType = 0;
                        else if( IsXMLToken( rAttrValue, XML_DIAMOND ))
                            nSymbolType = 1;
                        else if( IsXMLToken( rAttrValue, XML_ARROW_DOWN ))
                            nSymbolType = 2;
                        else if( IsXMLToken( rAttrValue, XML_ARROW_UP ))
                            nSymbolType = 3;
                        else if( IsXMLToken( rAttrValue, XML_ARROW_RIGHT ))
                            nSymbolType = 4;
                        else if( IsXMLToken( rAttrValue, XML_ARROW_LEFT ))
                            nSymbolType = 5;
                        else if( IsXMLToken( rAttrValue, XML_BOW_TIE ))
                            nSymbolType = 6;
                        else if( IsXMLToken( rAttrValue, XML_HOURGLASS ))
                            nSymbolType = 7;
                        else if( IsXMLToken( rAttrValue, XML_CIRCLE ))
                            nSymbolType = 8;
                        else if( IsXMLToken( rAttrValue, XML_STAR ))
                            nSymbolType = 9;
                        else if( IsXMLToken( rAttrValue, XML_X ))
                            nSymbolType = 10;
                        else if( IsXMLToken( rAttrValue, XML_PLUS ))
                            nSymbolType = 11;
                        else if( IsXMLToken( rAttrValue, XML_ASTERISK ))
                            nSymbolType = 12;
                        else if( IsXMLToken( rAttrValue, XML_HORIZONTAL_BAR ))
                            nSymbolType = 13;
                        else if( IsXMLToken( rAttrValue, XML_VERTICAL_BAR ))
                            nSymbolType = 14;

                        if( nSymbolType >= 0 )
                            pAttrList->AddAttribute(
                                GetTransformer().GetNamespaceMap().GetQNameByKey(
                                    XML_NAMESPACE_CHART,
                                    GetXMLToken( XML_SYMBOL )),
                                OUString::number( nSymbolType ));
                    }
                    break;
                // #i25616#
                case XML_OPTACTION_OPACITY:
                    aOpacityValueRemember = rAttrValue;
                    XMLTransformerBase::NegPercent(aOpacityValueRemember);
                    break;

                // #i25616#
                case XML_OPTACTION_IMAGE_OPACITY:
                    aImageOpacityValueRemember = rAttrValue;
                    XMLTransformerBase::NegPercent(aImageOpacityValueRemember);
                    break;

                case XML_OPTACTION_KEEP_TOGETHER:
                    pAttrList->AddAttribute(
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_STYLE,GetXMLToken(XML_BREAK_INSIDE)),
                        GetXMLToken(
                            IsXMLToken( rAttrValue, XML_ALWAYS )
                            ? XML_COLUMNSPLIT_AVOID
                            : XML_COLUMNSPLIT_AUTO ) );
                    break;

                case XML_OPTACTION_CONTROL_TEXT_ALIGN:
                    if ( m_bControlStyle )
                    {
                        OUString aNewAttrQName(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_STYLE,
                                ::xmloff::token::GetXMLToken(
                                XML_TEXT_ALIGN ) ) );
                        pAttrList->AddAttribute( aNewAttrQName, rAttrValue );
                    }
                    else
                    {
                        OUString aNewAttrQName(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_FO,
                                ::xmloff::token::GetXMLToken(
                                XML_TEXT_ALIGN ) ) );
                        pAttrList->AddAttribute( aNewAttrQName, rAttrValue );
                    }
                    break;

                case XML_OPTACTION_DRAW_WRITING_MODE:
                    if( IsXMLToken( m_aStyleFamily, XML_GRAPHICS ) )
                    {
                        pAttrList->AddAttribute(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                    XML_NAMESPACE_DRAW,
                                    GetXMLToken( XML_WRITING_MODE ) ), rAttrValue );
                    }
                    pAttrList->AddAttribute( rAttrName, rAttrValue );
                    break;

                case XML_ATACTION_CAPTION_ESCAPE_OASIS:
                    {
                        OUString aAttrValue( rAttrValue );
                        if( aAttrValue.indexOf( '%' ) != -1 )
                        {
                            sal_Int32 nValue = 0;
                            ::sax::Converter::convertPercent(nValue, rAttrValue);
                            if( nValue )
                            {
                                nValue *= 100;
                                OUStringBuffer aOut;
                                ::sax::Converter::convertPercent(aOut, nValue);
                                aAttrValue = aOut.makeStringAndClear();
                            }
                        }
                        else
                        {
                            XMLTransformerBase::ReplaceSingleInWithInch( aAttrValue );
                        }

                        pAttrList->AddAttribute( rAttrName, aAttrValue );
                    }
                    break;

                case XML_ATACTION_DECODE_PROTECT:
                    {
                        pAttrList->AddAttribute( rAttrName, rAttrValue );

                        if( rAttrValue.indexOf( GetXMLToken( XML_SIZE ) ) != -1 )
                            pAttrList->AddAttribute( GetTransformer().GetNamespaceMap().GetQNameByKey(
                                    XML_NAMESPACE_DRAW,
                                    GetXMLToken( XML_SIZE_PROTECT )), GetXMLToken( XML_TRUE ) );

                        if( rAttrValue.indexOf( GetXMLToken( XML_POSITION ) ) != -1 )
                            pAttrList->AddAttribute( GetTransformer().GetNamespaceMap().GetQNameByKey(
                                    XML_NAMESPACE_DRAW,
                                    GetXMLToken( XML_MOVE_PROTECT )), GetXMLToken( XML_TRUE ) );
                    }
                    break;

                case XML_ATACTION_DRAW_MIRROR_OASIS: // renames style:mirror to draw:mirror and adapts values
                    {
                        // keep original for writer graphic objects
                        // Adapts attribute values (#i49139#)
                        OUString aNewAttrValue;
                        SvXMLTokenEnumerator aTokenEnum( rAttrValue );
                        OUString aToken;
                        while( aTokenEnum.getNextToken( aToken ) )
                        {
                            if ( !aNewAttrValue.isEmpty() )
                            {
                                aNewAttrValue += " ";
                            }

                            if ( IsXMLToken( aToken, XML_HORIZONTAL_ON_EVEN ) )
                            {
                                aNewAttrValue += GetXMLToken( XML_HORIZONTAL_ON_LEFT_PAGES );
                            }
                            else if ( IsXMLToken( aToken, XML_HORIZONTAL_ON_ODD ) )
                            {
                                aNewAttrValue += GetXMLToken( XML_HORIZONTAL_ON_RIGHT_PAGES );
                            }
                            else
                            {
                                aNewAttrValue += aToken;
                            }
                        }
                        pAttrList->AddAttribute( rAttrName, aNewAttrValue );

                        // create old draw:mirror for drawing graphic objects
                        OUString aAttrValue( GetXMLToken( IsXMLToken( rAttrValue, XML_HORIZONTAL ) ? XML_TRUE : XML_FALSE ) );
                        pAttrList->AddAttribute( GetTransformer().GetNamespaceMap().GetQNameByKey(
                                    XML_NAMESPACE_DRAW,
                                    GetXMLToken( XML_MIRROR )), aAttrValue );
                    }
                    break;
                case XML_ATACTION_GAMMA_OASIS:       // converts percentage value to double
                    {
                        sal_Int32 nValue;
                        ::sax::Converter::convertPercent( nValue, rAttrValue );
                        const double fValue = ((double)nValue) / 100.0;
                        pAttrList->AddAttribute( rAttrName, OUString::number( fValue ) );
                    }
                    break;
                case XML_ATACTION_OPACITY_FIX:
                    {
                        sal_Int32 nValue;
                        if( rAttrValue.indexOf( '%' ) != -1 )
                        {
                            ::sax::Converter::convertPercent(nValue, rAttrValue);
                        }
                        else
                        {
                            nValue = sal_Int32( rAttrValue.toDouble() * 100.0 );
                        }
                        nValue = 100 - nValue;

                        OUStringBuffer aOut;
                        ::sax::Converter::convertPercent(aOut, nValue);
                        pAttrList->AddAttribute( rAttrName, aOut.makeStringAndClear() );
                    }
                    break;
                default:
                    OSL_ENSURE( false, "unknown action" );
                    break;
                }
            }
            else
            {
                pAttrList->AddAttribute( rAttrName, rAttrValue );
            }
        }
        if( XML_TOKEN_END != eUnderline )
            pAttrList->AddAttribute(
                    GetTransformer().GetNamespaceMap().GetQNameByKey(
                        XML_NAMESPACE_STYLE,
                        GetXMLToken( XML_TEXT_UNDERLINE ) ),
                    MergeUnderline( eUnderline, bBoldUnderline,
                                    bDoubleUnderline ) );
        if( XML_TOKEN_END != eLineThrough )
            pAttrList->AddAttribute(
                    GetTransformer().GetNamespaceMap().GetQNameByKey(
                        XML_NAMESPACE_STYLE,
                        GetXMLToken( XML_TEXT_CROSSING_OUT ) ),
                    MergeLineThrough( eLineThrough, bBoldLineThrough,
                                    bDoubleLineThrough, cLineThroughChar ) );
        if( bIntervalMinorFound )
        {
            double fIntervalMinor = 0.0;
            if( nIntervalMinorDivisor != 0)
                fIntervalMinor = fIntervalMajor / static_cast< double >( nIntervalMinorDivisor );

            OUStringBuffer aBuf;
            ::sax::Converter::convertDouble( aBuf, fIntervalMinor );
            pAttrList->AddAttribute(
                GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_CHART,
                    GetXMLToken( XML_INTERVAL_MINOR )),
                aBuf.makeStringAndClear());
        }

        // #i25616#
        if(!aOpacityValueRemember.isEmpty() || !aImageOpacityValueRemember.isEmpty())
        {
            pAttrList->AddAttribute(
                    GetTransformer().GetNamespaceMap().GetQNameByKey(
                        XML_NAMESPACE_DRAW,
                        GetXMLToken( XML_TRANSPARENCY ) ),
                    !aImageOpacityValueRemember.isEmpty()
                    ? aImageOpacityValueRemember : aOpacityValueRemember );
        }
    }
    else
    {
        if( !m_xAttrList.is() )
        {
            m_xAttrList = new XMLMutableAttributeList( rAttrList, true );
        }
        else
        {
            static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                ->AppendAttributeList( rAttrList );
        }
    }
}

void XMLPropertiesTContext_Impl::Export()
{
    GetTransformer().GetDocHandler()->startElement( GetExportQName(),
                                                    m_xAttrList );
    ExportContent();
    GetTransformer().GetDocHandler()->endElement( GetExportQName() );
}

XMLPropType XMLPropertiesTContext_Impl::GetPropType( const OUString& rLocalName )
{
    XMLPropType eProp = XML_PROP_TYPE_END;
    if( IsXMLToken( rLocalName, XML_GRAPHIC_PROPERTIES )  )
        eProp = XML_PROP_TYPE_GRAPHIC;
    else if( IsXMLToken( rLocalName, XML_DRAWING_PAGE_PROPERTIES ) )
        eProp = XML_PROP_TYPE_DRAWING_PAGE;
    else if( IsXMLToken( rLocalName, XML_PAGE_LAYOUT_PROPERTIES ) )
        eProp = XML_PROP_TYPE_PAGE_LAYOUT;
    else if( IsXMLToken( rLocalName, XML_HEADER_FOOTER_PROPERTIES ) )
        eProp = XML_PROP_TYPE_HEADER_FOOTER;
    else if( IsXMLToken( rLocalName, XML_TEXT_PROPERTIES ) )
        eProp = XML_PROP_TYPE_TEXT;
    else if( IsXMLToken( rLocalName, XML_PARAGRAPH_PROPERTIES ) )
        eProp = XML_PROP_TYPE_PARAGRAPH;
    else if( IsXMLToken( rLocalName, XML_RUBY_PROPERTIES ) )
        eProp = XML_PROP_TYPE_RUBY;
    else if( IsXMLToken( rLocalName, XML_SECTION_PROPERTIES ) )
        eProp = XML_PROP_TYPE_SECTION;
    else if( IsXMLToken( rLocalName, XML_TABLE_PROPERTIES ) )
        eProp = XML_PROP_TYPE_TABLE;
    else if( IsXMLToken( rLocalName, XML_TABLE_COLUMN_PROPERTIES ) )
        eProp = XML_PROP_TYPE_TABLE_COLUMN;
    else if( IsXMLToken( rLocalName, XML_TABLE_ROW_PROPERTIES ) )
        eProp = XML_PROP_TYPE_TABLE_ROW;
    else if( IsXMLToken( rLocalName, XML_TABLE_CELL_PROPERTIES ) )
        eProp = XML_PROP_TYPE_TABLE_CELL;
    else if( IsXMLToken( rLocalName, XML_LIST_LEVEL_PROPERTIES ) )
        eProp = XML_PROP_TYPE_LIST_LEVEL;
    else if( IsXMLToken( rLocalName, XML_CHART_PROPERTIES ) )
        eProp = XML_PROP_TYPE_CHART;

    return eProp;
}

OUString XMLPropertiesTContext_Impl::MergeUnderline(
            XMLTokenEnum eUnderline, bool bBold, bool bDouble )
{
    if( bDouble )
    {
        switch( eUnderline )
        {
        case XML_WAVE:
            eUnderline = XML_DOUBLE_WAVE;
            break;
        default:
            eUnderline = XML_DOUBLE;
            break;
        }
    }
    else if( bBold )
    {
        switch( eUnderline )
        {
        case XML_NONE:
        case XML_SOLID:
            eUnderline = XML_BOLD;
            break;
        case XML_DOTTED:
            eUnderline = XML_BOLD_DOTTED;
            break;
        case XML_DASH:
            eUnderline = XML_BOLD_DASH;
            break;
        case XML_LONG_DASH:
            eUnderline = XML_BOLD_LONG_DASH;
            break;
        case XML_DOT_DASH:
            eUnderline = XML_BOLD_DOT_DASH;
            break;
        case XML_DOT_DOT_DASH:
            eUnderline = XML_BOLD_DOT_DOT_DASH;
            break;
        case XML_WAVE:
            eUnderline = XML_BOLD_WAVE;
            break;
        default:
            OSL_FAIL( "xmloff::XMLPropertiesTContext_Impl::MergeUnderline(), missing underline case!" );
            break;
        }
    }
    else
    {
        switch( eUnderline )
        {
        case XML_SOLID:
            eUnderline = XML_SINGLE;
            break;
        case XML_NONE:
            eUnderline = XML_NONE;
            break;
        default:
            OSL_FAIL( "xmloff::XMLPropertiesTContext_Impl::MergeUnderline(), missing underline case!" );
            break;
        }
    }

    return GetXMLToken( eUnderline );
}

OUString XMLPropertiesTContext_Impl::MergeLineThrough(
            XMLTokenEnum eLineThrough, bool bBold, bool bDouble,
               sal_Unicode c )
{
    if( c )
        eLineThrough = c=='/' ? XML_SLASH : XML_uX;
    else if( bDouble )
        eLineThrough = XML_DOUBLE_LINE;
    else if( bBold )
        eLineThrough = XML_THICK_LINE;
    else if( XML_NONE != eLineThrough )
        eLineThrough = XML_SINGLE_LINE;

    return GetXMLToken( eLineThrough );
}

XMLStyleOASISTContext::XMLStyleOASISTContext(XMLTransformerBase& rImp,
    const OUString& rQName, bool bPersistent)
    : XMLPersElemContentTContext(rImp, rQName)
    , m_bPersistent(bPersistent)
    , m_bControlStyle(false)
{
}

XMLStyleOASISTContext::XMLStyleOASISTContext(
    XMLTransformerBase& rImp, const OUString& rQName,
    sal_uInt16 nPrefix, ::xmloff::token::XMLTokenEnum eToken,
    bool bPersistent)
    : XMLPersElemContentTContext(rImp, rQName, nPrefix, eToken)
    , m_bPersistent(bPersistent)
    , m_bControlStyle(false)
{
}

XMLStyleOASISTContext::~XMLStyleOASISTContext()
{
}

XMLTransformerContext *XMLStyleOASISTContext::CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const OUString& rQName,
            const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext *pContext = nullptr;

    if( XML_NAMESPACE_STYLE == nPrefix || XML_NAMESPACE_LO_EXT == nPrefix )
    {
        XMLPropType ePropType =
            XMLPropertiesTContext_Impl::GetPropType( rLocalName );
        if( XML_PROP_TYPE_END != ePropType )
        {
            // if no properties context exist start a new one.
            if( !m_xPropContext.is() )
                m_xPropContext = new XMLPropertiesTContext_Impl(
                    GetTransformer(), rQName, ePropType, m_aStyleFamily, m_bControlStyle );
            else
                m_xPropContext->SetQNameAndPropType( rQName, ePropType );
            pContext = m_xPropContext.get();
        }
    }
    if( !pContext )
    {
        // if a properties context exist close it
        if( m_xPropContext.is() && !m_bPersistent )
        {
            m_xPropContext->Export();
            m_xPropContext = nullptr;
        }

        pContext = m_bPersistent
                        ? XMLPersElemContentTContext::CreateChildContext(
                                nPrefix, rLocalName, rQName, rAttrList )
                        : XMLTransformerContext::CreateChildContext(
                                nPrefix, rLocalName, rQName, rAttrList );
    }

    return pContext;
}

void XMLStyleOASISTContext::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OASIS_STYLE_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    Reference< XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = nullptr;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    sal_Int16 nFamilyAttr = -1;
    m_bControlStyle = false;

    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
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
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_STYLE_FAMILY:
                if( IsXMLToken( rAttrValue, XML_GRAPHIC ) )
                {
                    m_aStyleFamily = GetXMLToken( XML_GRAPHICS ) ;
                    pMutableAttrList->SetValueByIndex( i, m_aStyleFamily );
                }
                else
                {
                    m_aStyleFamily = rAttrValue;

                    if( IsXMLToken( rAttrValue, XML_PARAGRAPH ) )
                        nFamilyAttr = i;
                }

                break;
            case XML_ATACTION_STYLE_DISPLAY_NAME:
            case XML_ATACTION_REMOVE:
                pMutableAttrList->RemoveAttributeByIndex( i );
                --i;
                --nAttrCount;
                break;
            case XML_ATACTION_DECODE_STYLE_NAME:
                m_bControlStyle = rAttrValue.startsWith( "ctrl" );
                SAL_FALLTHROUGH;
            case XML_ATACTION_DECODE_STYLE_NAME_REF:
                {
                    OUString aAttrValue( rAttrValue );
                    if( XMLTransformerBase::DecodeStyleName(aAttrValue) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_IN2INCH:
                {
                    OUString aAttrValue( rAttrValue );
                    if( XMLTransformerBase::ReplaceSingleInWithInch(
                                aAttrValue ) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_NEG_PERCENT:
                {
                    OUString aAttrValue( rAttrValue );
                    if( XMLTransformerBase::NegPercent(aAttrValue) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_URI_OASIS:
                {
                    OUString aAttrValue( rAttrValue );
                    if( GetTransformer().ConvertURIToOOo( aAttrValue,
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

    if( m_bControlStyle && nFamilyAttr != -1 )
        pMutableAttrList->SetValueByIndex( nFamilyAttr, GetXMLToken( XML_CONTROL ) );

    if( m_bPersistent )
        XMLPersElemContentTContext::StartElement( xAttrList );
    else
        GetTransformer().GetDocHandler()->startElement( GetExportQName(),
                                                        xAttrList );
}

void XMLStyleOASISTContext::EndElement()
{
    if( m_bPersistent )
    {
        XMLPersElemContentTContext::EndElement();
    }
    else
    {
        // if a properties context exist close it
        if( m_xPropContext.is() )
        {
            m_xPropContext->Export();
            m_xPropContext = nullptr;
        }
        GetTransformer().GetDocHandler()->endElement( GetExportQName() );
    }
}

void XMLStyleOASISTContext::Characters( const OUString& )
{
    // element content only:
}

void XMLStyleOASISTContext::ExportContent()
{
    if( m_xPropContext.is() )
        m_xPropContext->Export();
    XMLPersElemContentTContext::ExportContent();
}

bool XMLStyleOASISTContext::IsPersistent() const
{
    return m_bPersistent;
}

XMLTransformerActions *XMLStyleOASISTContext::CreateTransformerActions(
        sal_uInt16 nType )
{
    XMLTransformerActionInit *pInit = nullptr;

    switch( nType )
    {
    case PROP_OASIS_GRAPHIC_ATTR_ACTIONS:
        pInit = aGraphicPropertyOASISAttrActionTable;
        break;
    case PROP_OASIS_DRAWING_PAGE_ATTR_ACTIONS:
        pInit = aDrawingPagePropertyOASISAttrActionTable;
        break;
    case PROP_OASIS_PAGE_LAYOUT_ATTR_ACTIONS:
        pInit = aPageLayoutPropertyOASISAttrActionTable;
        break;
    case PROP_OASIS_HEADER_FOOTER_ATTR_ACTIONS:
        pInit = aHeaderFooterPropertyOASISAttrActionTable;
        break;
    case PROP_OASIS_TEXT_ATTR_ACTIONS:
        pInit = aTextPropertyOASISAttrActionTable;
        break;
    case PROP_OASIS_PARAGRAPH_ATTR_ACTIONS:
        pInit = aParagraphPropertyOASISAttrActionTable;
        break;
    case PROP_OASIS_SECTION_ATTR_ACTIONS:
        pInit = aSectionPropertyOASISAttrActionTable;
        break;
    case PROP_OASIS_TABLE_ATTR_ACTIONS:
        pInit = aTablePropertyOASISAttrActionTable;
        break;
    case PROP_OASIS_TABLE_COLUMN_ATTR_ACTIONS:
        pInit = aTableColumnPropertyOASISAttrActionTable;
        break;
    case PROP_OASIS_TABLE_ROW_ATTR_ACTIONS:
        pInit = aTableRowPropertyOASISAttrActionTable;
        break;
    case PROP_OASIS_TABLE_CELL_ATTR_ACTIONS:
        pInit = aTableCellPropertyOASISAttrActionTable;
        break;
    case PROP_OASIS_LIST_LEVEL_ATTR_ACTIONS:
        pInit = aListLevelPropertyOASISAttrActionTable;
        break;
    case PROP_OASIS_CHART_ATTR_ACTIONS:
        pInit = aChartPropertyOASISAttrActionTable;
        break;
    }

    XMLTransformerActions *pActions = nullptr;
    if( pInit )
        pActions = new XMLTransformerActions( pInit );

    return pActions;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
