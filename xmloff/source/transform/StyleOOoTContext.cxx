/*************************************************************************
 *
 *  $RCSfile: StyleOOoTContext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:35:05 $
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
#ifndef _XMLOFF_RENAMEELEMTCONTEXT_HXX
#include "RenameElemTContext.hxx"
#endif
#ifndef _XMLOFF_PROCATTRTCONTEXT_HXX
#include "ProcAttrTContext.hxx"
#endif
#ifndef _XMLOFF_ACTIONMAPTYPESOOO_HXX
#include "ActionMapTypesOOo.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#include "TransformerActions.hxx"
#endif
#ifndef _XMLOFF_PROPERTYACTIONSOOO_HXX
#include "PropertyActionsOOo.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif

#ifndef _XMLOFF_STYLEOASISTCONTEXT_HXX
#include "StyleOOoTContext.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

using ::rtl::OUString;
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

static XMLPropType aPropTypes[XML_FAMILY_TYPE_END][MAX_PROP_TYPES] =
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

static XMLTokenEnum aPropTokens[XML_PROP_TYPE_END] =
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

static sal_uInt16 aAttrActionMaps[XML_PROP_TYPE_END] =
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

static sal_uInt16 aElemActionMaps[XML_PROP_TYPE_END] =
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


//------------------------------------------------------------------------------

class XMLTypedPropertiesOOoTContext_Impl : public XMLPersElemContentTContext
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList > m_xAttrList;

public:

    TYPEINFO();

    XMLTypedPropertiesOOoTContext_Impl( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName );

    virtual ~XMLTypedPropertiesOOoTContext_Impl();

    void AddAttribute( const ::rtl::OUString &sName ,
                       const ::rtl::OUString &sValue );
    void AddAttribute( sal_uInt16 nPrefix, XMLTokenEnum eToken,
                       const ::rtl::OUString &sValue );

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
    const ::rtl::OUString &sName ,
    const ::rtl::OUString &sValue )
{
    static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
            ->AddAttribute( sName, sValue );
}

void XMLTypedPropertiesOOoTContext_Impl::AddAttribute(
    sal_uInt16 nPrefix, XMLTokenEnum eToken,
    const ::rtl::OUString &sValue )
{
    OUString sName(
            GetTransformer().GetNamespaceMap().GetQNameByKey(
                        nPrefix, ::xmloff::token::GetXMLToken( eToken ) ) );
    static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
            ->AddAttribute( sName, sValue );
}

void XMLTypedPropertiesOOoTContext_Impl::StartElement(
        const Reference< XAttributeList >& rAttrList )
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

//------------------------------------------------------------------------------

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
                           const ::rtl::OUString& rQName,
                               XMLPropTypes& rTypes,
                               sal_Bool bPersistent );

    virtual ~XMLPropertiesOOoTContext_Impl();

    XMLTransformerContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const OUString& rQName,
            const Reference< XAttributeList >& rAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& rAttrList );

    virtual void EndElement();

    virtual void Characters( const ::rtl::OUString& rChars );

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

    OSL_ENSURE( XML_NAMESPACE_NONE == nPrefix ||
                (XML_NAMESPACE_UNKNOWN_FLAG & nPrefix) ||
                XML_PROP_TYPE_END==m_aPropTypes[1] ||
                (i<MAX_PROP_TYPES && XML_PROP_TYPE_END!=m_aPropTypes[i]),
                "didn't find property" );

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
    XMLPropTypes& rTypes,
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

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rAttrValue = xAttrList->getValueByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                 &aLocalName );
        TransformerAction_Impl aAction;
        XMLTypedPropertiesOOoTContext_Impl *pContext =
            GetPropContextAndAction( aAction, nPrefix, aLocalName, sal_False );
        switch( aAction.m_nActionType )
        {
        case XML_ATACTION_COPY:
            pContext->AddAttribute( rAttrName, rAttrValue );
            break;
        case XML_ATACTION_COPY_DUPLICATE:
            {
                pContext->AddAttribute( rAttrName, rAttrValue );
                XMLTypedPropertiesOOoTContext_Impl *pContext2 =
                    GetPropContext( (XMLPropType)aAction.m_nParam1 );
                if( pContext2 )
                    pContext2->AddAttribute( rAttrName, rAttrValue );
            }
            break;
        case XML_ATACTION_RENAME:
            {
                pContext->AddAttribute( aAction.GetQNamePrefixFromParam1(),
                                        aAction.GetQNameTokenFromParam1(),
                                        rAttrValue );
            }
            break;
        case XML_ATACTION_ENCODE_STYLE_NAME_REF:
            {
                OUString aAttrValue( rAttrValue );
                GetTransformer().EncodeStyleName(aAttrValue);
                pContext->AddAttribute( rAttrName, aAttrValue );
            }
            break;
        case XML_ATACTION_RENAME_ENCODE_STYLE_NAME_REF:
            {
                OUString aAttrValue( rAttrValue );
                GetTransformer().EncodeStyleName(aAttrValue);
                pContext->AddAttribute( aAction.GetQNamePrefixFromParam1(),
                                        aAction.GetQNameTokenFromParam1(),
                                        aAttrValue );
            }
            break;
        case XML_ATACTION_NEG_PERCENT:
            {
                OUString aAttrValue( rAttrValue );
                GetTransformer().NegPercent(aAttrValue);
                pContext->AddAttribute( rAttrName, aAttrValue );
            }
            break;
        case XML_ATACTION_RENAME_NEG_PERCENT:
            {
                OUString aAttrValue( rAttrValue );
                GetTransformer().NegPercent(aAttrValue);
                pContext->AddAttribute( aAction.GetQNamePrefixFromParam1(),
                                        aAction.GetQNameTokenFromParam1(),
                                        aAttrValue );
            }
            break;
        case XML_ATACTION_INCH2IN:
            {
                OUString aAttrValue( rAttrValue );
                XMLTransformerBase::ReplaceSingleInchWithIn( aAttrValue );
                pContext->AddAttribute( rAttrName, aAttrValue );
            }
            break;
        case XML_ATACTION_INCH2IN_DUPLICATE:
            {
                OUString aAttrValue( rAttrValue );
                XMLTransformerBase::ReplaceSingleInchWithIn( aAttrValue );
                pContext->AddAttribute( rAttrName, aAttrValue );
                XMLTypedPropertiesOOoTContext_Impl *pContext2 =
                    GetPropContext( (XMLPropType)aAction.m_nParam1 );
                if( pContext2 )
                    pContext2->AddAttribute( rAttrName, aAttrValue );
            }
            break;
        case XML_ATACTION_INCHS2INS:
            {
                OUString aAttrValue( rAttrValue );
                XMLTransformerBase::ReplaceInchWithIn( aAttrValue );
                pContext->AddAttribute( rAttrName, aAttrValue );
            }
            break;
        case XML_PTACTION_LINE_MODE:
            {
                OUString aAttrValue( GetXMLToken(
                                        IsXMLToken( rAttrValue, XML_TRUE )
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
                                        IsXMLToken( rAttrValue, XML_TRUE )
                                            ? XML_ALWAYS
                                            : XML_AUTO) );
                pContext->AddAttribute( rAttrName, aAttrValue );
            }
            break;
        case XML_PTACTION_UNDERLINE:
            {
                XMLTokenEnum eToken = GetTransformer().GetToken( rAttrValue );
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
                }
                pContext->AddAttribute(
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_STYLE,
                            GetXMLToken( XML_TEXT_UNDERLINE_STYLE ) ),
                        eToken != XML_TOKEN_END ? GetXMLToken( eToken )
                                                   : rAttrValue );
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
                XMLTokenEnum eToken = GetTransformer().GetToken( rAttrValue );
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
                case XML_X:
                    eToken = XML_SOLID;
                    c = 'X';
                    break;
                }
                pContext->AddAttribute(
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_STYLE,
                            GetXMLToken( XML_TEXT_LINE_THROUGH_STYLE ) ),
                        eToken != XML_TOKEN_END ? GetXMLToken( eToken )
                                                   : rAttrValue );
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
                            OUString::valueOf( c ) );
            }
            break;
        case XML_PTACTION_SPLINES:
            {
                sal_Int32 nSplineType = rAttrValue.toInt32();
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
                        OSL_ENSURE( false, "invalid spline type" );
                        pContext->AddAttribute(
                            aNewAttrName, GetXMLToken( XML_NONE ));
                        break;
                }
            }
            break;
        case XML_PTACTION_INTERVAL_MAJOR:
            pContext->AddAttribute( rAttrName, rAttrValue );
            SvXMLUnitConverter::convertDouble( fIntervalMajor, rAttrValue );
            break;
        case XML_PTACTION_INTERVAL_MINOR:
            SvXMLUnitConverter::convertDouble( fIntervalMinor, rAttrValue );
            pIntervalMinorDivisorContext = pContext;
            break;

        // #i25616#
        case XML_PTACTION_TRANSPARENCY :
            {
                OUString aAttrValue( rAttrValue );
                GetTransformer().NegPercent(aAttrValue);
                pContext->AddAttribute( XML_NAMESPACE_DRAW,
                                        XML_OPACITY,
                                        aAttrValue );
                pContext->AddAttribute( XML_NAMESPACE_DRAW,
                                        XML_IMAGE_OPACITY,
                                        aAttrValue );
            }
            break;

        default:
            OSL_ENSURE( !this, "unknown action" );
            break;
        }
    }

    if( pIntervalMinorDivisorContext )
    {
        if( fIntervalMinor != 0.0 )
        {
            sal_Int32 nIntervalMinorDivisor = static_cast< sal_Int32 >(
                ::rtl::math::round( fIntervalMajor / fIntervalMinor ));

            ::rtl::OUStringBuffer aBuf;
            SvXMLUnitConverter::convertNumber( aBuf, nIntervalMinorDivisor );
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

void XMLPropertiesOOoTContext_Impl::Characters( const OUString& rChars )
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


//------------------------------------------------------------------------------

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
                {
                    sal_Bool bControl = sal_False;
                    if( XML_FAMILY_TYPE_END == m_eFamily )
                    {
                        if( IsXMLToken( rAttrValue, XML_GRAPHICS ) )
                            m_eFamily = XML_FAMILY_TYPE_GRAPHIC;
                        else if( IsXMLToken( rAttrValue, XML_PRESENTATION ) )
                            m_eFamily = XML_FAMILY_TYPE_PRESENTATION;
                        else if( IsXMLToken( rAttrValue, XML_DRAWING_PAGE ) )
                            m_eFamily = XML_FAMILY_TYPE_DRAWING_PAGE;
                        else if( IsXMLToken( rAttrValue, XML_TEXT) )
                            m_eFamily = XML_FAMILY_TYPE_TEXT;
                        else if( IsXMLToken( rAttrValue, XML_PARAGRAPH) )
                            m_eFamily = XML_FAMILY_TYPE_PARAGRAPH;
                        else if( IsXMLToken( rAttrValue, XML_RUBY) )
                            m_eFamily = XML_FAMILY_TYPE_RUBY;
                        else if( IsXMLToken( rAttrValue, XML_SECTION) )
                            m_eFamily = XML_FAMILY_TYPE_SECTION;
                        else if( IsXMLToken( rAttrValue, XML_TABLE) )
                            m_eFamily = XML_FAMILY_TYPE_TABLE;
                        else if( IsXMLToken( rAttrValue, XML_TABLE_COLUMN) )
                            m_eFamily = XML_FAMILY_TYPE_TABLE_COLUMN;
                        else if( IsXMLToken( rAttrValue, XML_TABLE_ROW) )
                            m_eFamily = XML_FAMILY_TYPE_TABLE_ROW;
                        else if( IsXMLToken( rAttrValue, XML_TABLE_CELL) )
                            m_eFamily = XML_FAMILY_TYPE_TABLE_CELL;
                        else if( IsXMLToken( rAttrValue, XML_CHART) )
                            m_eFamily = XML_FAMILY_TYPE_CHART;
                        else if( IsXMLToken( rAttrValue, XML_CONTROL) )
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
                    OUString aAttrValue( rAttrValue );
                    if( XMLTransformerBase::ReplaceSingleInchWithIn(
                                aAttrValue ) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_ENCODE_STYLE_NAME:
                {
                    OUString aAttrValue( rAttrValue );
                    if( GetTransformer().EncodeStyleName(aAttrValue) )
                    {
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                        OUString aNewAttrQName(
                            GetTransformer().GetNamespaceMap().
                                GetQNameByKey(
                                    nPrefix, ::xmloff::token::GetXMLToken(
                                        XML_DISPLAY_NAME ) ) );
                        pMutableAttrList->AddAttribute( aNewAttrQName,
                                                        rAttrValue );
                    }
                }
                break;
            case XML_ATACTION_ENCODE_STYLE_NAME_REF:
                {
                    OUString aAttrValue( rAttrValue );
                    if( GetTransformer().EncodeStyleName(aAttrValue) )
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
            case XML_ATACTION_URI_OOO:
                {
                    OUString aAttrValue( rAttrValue );
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

void XMLStyleOOoTContext::Characters( const OUString& rChars )
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
