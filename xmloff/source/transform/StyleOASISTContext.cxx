/*************************************************************************
 *
 *  $RCSfile: StyleOASISTContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:58:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_PROPTYPE_HXX
#include "PropType.hxx"
#endif

#ifndef _XMLOFF_DEEPTCONTEXT_HXX
#include "DeepTContext.hxx"
#endif
#ifndef _XMLOFF_PROCATTRTCONTEXT_HXX
#include "ProcAttrTContext.hxx"
#endif
#ifndef _XMLOFF_TRANSFOERMERBASE_HXX
#include "TransformerBase.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#include "TransformerActions.hxx"
#endif
#ifndef _XMLOFF_ACTIONMAPTYPESOASIS_HXX
#include "ActionMapTypesOASIS.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif
#ifndef _XMLOFF_PROPERTYACTIONSOASIS_HXX
#include "PropertyActionsOASIS.hxx"
#endif

#ifndef _XMLOFF_STYLEOASISTCONTEXT_HXX
#include "StyleOASISTContext.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

using ::rtl::OUString;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
//------------------------------------------------------------------------------

static sal_uInt16 aAttrActionMaps[XML_PROP_TYPE_END] =
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

//------------------------------------------------------------------------------

class XMLPropertiesTContext_Impl : public XMLPersElemContentTContext
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList > m_xAttrList;

    XMLPropType m_ePropType;

public:

    void SetQNameAndPropType( const ::rtl::OUString& rQName,
                                 XMLPropType ePropType  )
    {
        m_ePropType = ePropType;
        XMLTransformerContext::SetQName( rQName );
    };

    TYPEINFO();

    XMLPropertiesTContext_Impl( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                           XMLPropType eP );

    virtual ~XMLPropertiesTContext_Impl();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void Export();

    static XMLPropType GetPropType( const OUString& rLocalName );

    static OUString MergeUnderline( XMLTokenEnum eUnderline,
                                           sal_Bool bBold, sal_Bool bDouble );
    static OUString MergeLineThrough( XMLTokenEnum eLineThrough,
                                        sal_Bool bBold, sal_Bool bDouble,
                                           sal_Unicode c );
};

TYPEINIT1( XMLPropertiesTContext_Impl, XMLPersElemContentTContext );

XMLPropertiesTContext_Impl::XMLPropertiesTContext_Impl(
    XMLTransformerBase& rImp, const OUString& rQName, XMLPropType eP ) :
    XMLPersElemContentTContext( rImp, rQName, XML_NAMESPACE_STYLE,
                                XML_PROPERTIES),
    m_ePropType( eP )
{
}

XMLPropertiesTContext_Impl::~XMLPropertiesTContext_Impl()
{
}

void XMLPropertiesTContext_Impl::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerActions *pActions =  0;
    sal_uInt16 nActionMap = aAttrActionMaps[m_ePropType];
    if( nActionMap < MAX_OASIS_PROP_ACTIONS )
    {
        pActions = GetTransformer().GetUserDefinedActions( nActionMap );
        OSL_ENSURE( pActions, "go no actions" );
    }

    if( pActions )
    {
        XMLMutableAttributeList *pAttrList = 0;
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
        sal_Bool bBoldUnderline = sal_False, bDoubleUnderline = sal_False;
        XMLTokenEnum eLineThrough = XML_TOKEN_END;
        sal_Bool bBoldLineThrough = sal_False, bDoubleLineThrough = sal_False;
        sal_Unicode cLineThroughChar = 0;

        bool bIntervalMinorFound = false;
        double fIntervalMajor = 0.0;
        sal_Int32 nIntervalMinorDivisor = 0;

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
                        GetTransformer().DecodeStyleName(aAttrValue);
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
                        GetTransformer().DecodeStyleName(aAttrValue);
                        pAttrList->AddAttribute( aNewAttrQName, aAttrValue );
                    }
                    break;
                case XML_ATACTION_NEG_PERCENT:
                    {
                        OUString aAttrValue( rAttrValue );
                        GetTransformer().NegPercent(aAttrValue);
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
                        GetTransformer().NegPercent(aAttrValue);
                        pAttrList->AddAttribute( aNewAttrQName, aAttrValue );
                    }
                    break;
                case XML_OPTACTION_LINE_MODE:
                    {
                        sal_Bool bWordMode =
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
                        bBoldUnderline = sal_True;
                    break;
                case XML_OPTACTION_UNDERLINE_TYPE:
                    if( IsXMLToken( rAttrValue, XML_DOUBLE ) )
                        bDoubleUnderline = sal_True;
                    break;
                case XML_OPTACTION_UNDERLINE_STYLE:
                    eUnderline = GetTransformer().GetToken( rAttrValue );
                    break;
                case XML_OPTACTION_LINETHROUGH_WIDTH:
                    if( IsXMLToken( rAttrValue, XML_BOLD ) )
                        bBoldLineThrough = sal_True;
                    break;
                case XML_OPTACTION_LINETHROUGH_TYPE:
                    if( IsXMLToken( rAttrValue, XML_DOUBLE ) )
                        bDoubleLineThrough = sal_True;
                    break;
                case XML_OPTACTION_LINETHROUGH_STYLE:
                    eLineThrough = GetTransformer().GetToken( rAttrValue );
                    break;
                case XML_OPTACTION_LINETHROUGH_TEXT:
                    if( rAttrValue.getLength() )
                        cLineThroughChar = rAttrValue[0];
                    break;
                case XML_OPTACTION_INTERPOLATION:
                    {
                        // 0: none
                        sal_Int32 nSplineType = 0;
                        if( IsXMLToken( rAttrValue, XML_CUBIC_SPLINE ))
                            nSplineType = 1;
                        else if( IsXMLToken( rAttrValue, XML_B_SPLINE ))
                            nSplineType = 2;

                        pAttrList->AddAttribute(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_CHART,
                                GetXMLToken( XML_SPLINES )),
                            OUString::valueOf( nSplineType ));
                    }
                    break;
                case XML_OPTACTION_INTERVAL_MAJOR:
                    pAttrList->AddAttribute( rAttrName, rAttrValue );
                    SvXMLUnitConverter::convertDouble( fIntervalMajor, rAttrValue );
                    break;
                case XML_OPTACTION_INTERVAL_MINOR_DIVISOR:
                    SvXMLUnitConverter::convertNumber( nIntervalMinorDivisor, rAttrValue );
                    bIntervalMinorFound = true;
                    break;
                default:
                    OSL_ENSURE( !this, "unknown action" );
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

            ::rtl::OUStringBuffer aBuf;
            SvXMLUnitConverter::convertDouble( aBuf, fIntervalMinor );
            pAttrList->AddAttribute(
                GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_CHART,
                    GetXMLToken( XML_INTERVAL_MINOR )),
                aBuf.makeStringAndClear());
        }
    }
    else
    {
        if( !m_xAttrList.is() )
        {
            m_xAttrList = new XMLMutableAttributeList( rAttrList, sal_True );
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
    OUString aNewQName( GetTransformer().GetNamespaceMap().GetQNameByKey(
                XML_NAMESPACE_STYLE,
                ::xmloff::token::GetXMLToken( XML_PROPERTIES ) ) );
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
            XMLTokenEnum eUnderline, sal_Bool bBold, sal_Bool bDouble )
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
        }
    }
    else
    {
        switch( eUnderline )
        {
        case XML_SOLID:
            eUnderline = XML_SINGLE;
            break;
        }
    }

    return GetXMLToken( eUnderline );
}

OUString XMLPropertiesTContext_Impl::MergeLineThrough(
            XMLTokenEnum eLineThrough, sal_Bool bBold, sal_Bool bDouble,
               sal_Unicode c )
{
    if( c )
        eLineThrough = c=='/' ? XML_SLASH : XML_X;
    else if( bDouble )
        eLineThrough = XML_DOUBLE_LINE;
    else if( bBold )
        eLineThrough = XML_THICK_LINE;
    else if( XML_NONE != eLineThrough )
        eLineThrough = XML_SINGLE_LINE;

    return GetXMLToken( eLineThrough );
}

//------------------------------------------------------------------------------

TYPEINIT1( XMLStyleOASISTContext, XMLPersElemContentTContext );

XMLStyleOASISTContext::XMLStyleOASISTContext( XMLTransformerBase& rImp,
                                                const OUString& rQName,
                                                 sal_Bool bPersistent ) :
    XMLPersElemContentTContext ( rImp, rQName ),
    m_bPersistent( bPersistent )
{
}

XMLStyleOASISTContext::XMLStyleOASISTContext(
        XMLTransformerBase& rImp,
          const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
           sal_Bool bPersistent ) :
    XMLPersElemContentTContext( rImp, rQName, nPrefix, eToken ),
    m_bPersistent( bPersistent )
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
    XMLTransformerContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix )
    {
        XMLPropType ePropType =
            XMLPropertiesTContext_Impl::GetPropType( rLocalName );
        if( XML_PROP_TYPE_END != ePropType )
        {
            // if no properties context exist start a new one.
            if( !m_xPropContext.is() )
                m_xPropContext = new XMLPropertiesTContext_Impl(
                                    GetTransformer(), rQName, ePropType );
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
            m_xPropContext = 0;
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
    XMLMutableAttributeList *pMutableAttrList = 0;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    sal_Int16 nFamilyAttr = -1;
    sal_Bool bControlStyle = sal_False;
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
                    pMutableAttrList->SetValueByIndex( i,
                                            GetXMLToken( XML_GRAPHICS ) );
                }
                else if( IsXMLToken( rAttrValue, XML_PARAGRAPH ) )
                {
                    nFamilyAttr = i;
                }
                break;
            case XML_ATACTION_STYLE_DISPLAY_NAME:
                pMutableAttrList->RemoveAttributeByIndex( i );
                --i;
                --nAttrCount;
                break;
            case XML_ATACTION_DECODE_STYLE_NAME:
                bControlStyle = 0 == rAttrValue.compareToAscii( "ctrl", 4 );
            case XML_ATACTION_DECODE_STYLE_NAME_REF:
                {
                    OUString aAttrValue( rAttrValue );
                    if( GetTransformer().DecodeStyleName(aAttrValue) )
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
                    if( GetTransformer().NegPercent(aAttrValue) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_URI_OASIS:
                {
                    OUString aAttrValue( rAttrValue );
                    if( GetTransformer().ConvertURIToOOo( aAttrValue,
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

    if( bControlStyle && nFamilyAttr != -1 )
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
            m_xPropContext = 0;
        }
        GetTransformer().GetDocHandler()->endElement( GetExportQName() );
    }
}

void XMLStyleOASISTContext::Characters( const OUString& rChars )
{
    // element content only:
}

void XMLStyleOASISTContext::ExportContent()
{
    if( m_xPropContext.is() )
        m_xPropContext->Export();
    XMLPersElemContentTContext::ExportContent();
}

sal_Bool XMLStyleOASISTContext::IsPersistent() const
{
    return m_bPersistent;
}

XMLTransformerActions *XMLStyleOASISTContext::CreateTransformerActions(
        sal_uInt16 nType )
{
    XMLTransformerActionInit *pInit = 0;

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

    XMLTransformerActions *pActions = 0;
    if( pInit )
        pActions = new XMLTransformerActions( pInit );

    return pActions;
}
