/*************************************************************************
 *
 *  $RCSfile: XMLTextColumnsContext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dvo $ $Date: 2000-11-02 15:51:18 $
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

#ifndef _COM_SUN_STAR_TEXT_XTEXTCOLUMNS_HPP_
#include <com/sun/star/text/XTextColumns.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCOLUMN_HPP_
#include <com/sun/star/text/TextColumn.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_VERTICALALIGNMENT_HPP_
#include <com/sun/star/style/VerticalAlignment.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLTEXTCOLUMNSCONTEXT_HXX
#include "XMLTextColumnsContext.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;

enum SvXMLTokenMapAttrs
{
    XML_TOK_COLUMN_WIDTH,
    XML_TOK_COLUMN_MARGIN_LEFT,
    XML_TOK_COLUMN_MARGIN_RIGHT,
    XML_TOK_COLUMN_END=XML_TOK_UNKNOWN
};

enum SvXMLSepTokenMapAttrs
{
    XML_TOK_COLUMN_SEP_WIDTH,
    XML_TOK_COLUMN_SEP_HEIGHT,
    XML_TOK_COLUMN_SEP_COLOR,
    XML_TOK_COLUMN_SEP_ALIGN,
    XML_TOK_COLUMN_SEP_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aColAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  sXML_rel_width,     XML_TOK_COLUMN_WIDTH },
    { XML_NAMESPACE_FO,     sXML_margin_left,   XML_TOK_COLUMN_MARGIN_LEFT },
    { XML_NAMESPACE_FO,     sXML_margin_right,  XML_TOK_COLUMN_MARGIN_RIGHT },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aColSepAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  sXML_width,         XML_TOK_COLUMN_SEP_WIDTH },
    { XML_NAMESPACE_STYLE,  sXML_color,         XML_TOK_COLUMN_SEP_COLOR },
    { XML_NAMESPACE_STYLE,  sXML_height,        XML_TOK_COLUMN_SEP_HEIGHT },
    { XML_NAMESPACE_STYLE,  sXML_vertical_align,XML_TOK_COLUMN_SEP_ALIGN },
    XML_TOKEN_MAP_END
};

SvXMLEnumMapEntry __READONLY_DATA pXML_Sep_Align_Enum[] =
{
    { sXML_top,         VerticalAlignment_TOP   },
    { sXML_middle,      VerticalAlignment_MIDDLE },
    { sXML_bottom,      VerticalAlignment_BOTTOM },
    { 0, 0 }
};

class XMLTextColumnContext_Impl: public SvXMLImportContext
{
    text::TextColumn aColumn;

public:
    TYPEINFO();

    XMLTextColumnContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
                               const OUString& rLName,
                               const uno::Reference<
                                       xml::sax::XAttributeList > & xAttrList,
                               const SvXMLTokenMap& rTokenMap );

    virtual ~XMLTextColumnContext_Impl();

    text::TextColumn& getTextColumn() { return aColumn; }
};

TYPEINIT1( XMLTextColumnContext_Impl, SvXMLImportContext );

XMLTextColumnContext_Impl::XMLTextColumnContext_Impl(
                               SvXMLImport& rImport, sal_uInt16 nPrfx,
                               const OUString& rLName,
                               const uno::Reference<
                                       xml::sax::XAttributeList > & xAttrList,
                               const SvXMLTokenMap& rTokenMap ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    aColumn.Width = 0;
    aColumn.LeftMargin = 0;
    aColumn.RightMargin = 0;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        sal_Int32 nVal;
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_COLUMN_WIDTH:
            {
                sal_Int32 nPos = rValue.indexOf( (sal_Unicode)'*' );
                if( nPos != -1 && nPos+1 == rValue.getLength() )
                {
                    OUString sTmp( rValue.copy( 0, nPos ) );
                    if( GetImport().GetMM100UnitConverter().
                                        convertNumber( nVal, sTmp, 0, USHRT_MAX ) )
                    aColumn.Width = nVal;
                }
            }
            break;
        case XML_TOK_COLUMN_MARGIN_LEFT:
            if( GetImport().GetMM100UnitConverter().
                                        convertMeasure( nVal, rValue ) )
                aColumn.LeftMargin = nVal;
            break;
        case XML_TOK_COLUMN_MARGIN_RIGHT:

            if( GetImport().GetMM100UnitConverter().
                                        convertMeasure( nVal, rValue ) )
                aColumn.RightMargin = nVal;
            break;
        }
    }
}

XMLTextColumnContext_Impl::~XMLTextColumnContext_Impl()
{
}

// --------------------------------------------------------------------------

class XMLTextColumnSepContext_Impl: public SvXMLImportContext
{
    sal_Int32 nWidth;
    sal_Int32 nColor;
    sal_Int8 nHeight;
    VerticalAlignment eVertAlign;


public:
    TYPEINFO();

    XMLTextColumnSepContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
                               const OUString& rLName,
                               const uno::Reference<
                                       xml::sax::XAttributeList > & xAttrList,
                               const SvXMLTokenMap& rTokenMap );

    virtual ~XMLTextColumnSepContext_Impl();

    sal_Int32 GetWidth() const { return nWidth; }
    sal_Int32 GetColor() const { return  nColor; }
    sal_Int8 GetHeight() const { return nHeight; }
    VerticalAlignment GetVertAlign() const { return eVertAlign; }
};


TYPEINIT1( XMLTextColumnSepContext_Impl, SvXMLImportContext );

XMLTextColumnSepContext_Impl::XMLTextColumnSepContext_Impl(
                               SvXMLImport& rImport, sal_uInt16 nPrfx,
                               const OUString& rLName,
                               const uno::Reference<
                                       xml::sax::XAttributeList > & xAttrList,
                               const SvXMLTokenMap& rTokenMap ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nWidth( 2 ),
    nColor( 0 ),
    nHeight( 100 ),
    eVertAlign( VerticalAlignment_TOP )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        sal_Int32 nVal;
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_COLUMN_SEP_WIDTH:
            if( GetImport().GetMM100UnitConverter().
                                convertMeasure( nVal, rValue ) )
                nWidth = nVal;
            break;
        case XML_TOK_COLUMN_SEP_HEIGHT:
            if( GetImport().GetMM100UnitConverter().
                                        convertPercent( nVal, rValue ) &&
                 nVal >=1 && nVal <= 100 )
                nHeight = (sal_Int8)nVal;
            break;
        case XML_TOK_COLUMN_SEP_COLOR:
            {
                Color aColor;
                if( GetImport().GetMM100UnitConverter().
                                            convertColor( aColor, rValue ) )
                    nColor = nVal;
            }
            break;
        case XML_TOK_COLUMN_SEP_ALIGN:
            {
                sal_uInt16 nAlign;
                if( GetImport().GetMM100UnitConverter().
                                        convertEnum( nAlign, rValue,
                                                       pXML_Sep_Align_Enum ) )
                    eVertAlign = (VerticalAlignment)nAlign;
            }
            break;
        }
    }
}

XMLTextColumnSepContext_Impl::~XMLTextColumnSepContext_Impl()
{
}

// --------------------------------------------------------------------------

typedef XMLTextColumnContext_Impl *XMLTextColumnContext_ImplPtr;
SV_DECL_PTRARR( XMLTextColumnsArray_Impl, XMLTextColumnContext_ImplPtr, 5, 5 )

TYPEINIT1( XMLTextColumnsContext, XMLElementPropertyContext );

XMLTextColumnsContext::XMLTextColumnsContext(
                                SvXMLImport& rImport, sal_uInt16 nPrfx,
                                const OUString& rLName,
                                const Reference< xml::sax::XAttributeList >&
                                    xAttrList,
                                const XMLPropertyState& rProp,
                                 ::std::vector< XMLPropertyState > &rProps ) :
    XMLElementPropertyContext( rImport, nPrfx, rLName, rProp, rProps ),
    pColumnAttrTokenMap( new SvXMLTokenMap(aColAttrTokenMap) ),
    pColumnSepAttrTokenMap( new SvXMLTokenMap(aColSepAttrTokenMap) ),
      pColumns( 0 ),
    pColumnSep( 0 ),
    nCount( 0 ),
    sSeparatorLineIsOn(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineIsOn")),
    sSeparatorLineWidth(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineWidth")),
    sSeparatorLineColor(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineColor")),
    sSeparatorLineRelativeHeight(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineRelativeHeight")),
    sSeparatorLineVerticalAlignment(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineVerticalAlignment"))
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    sal_Int32 nVal;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );
        if( XML_NAMESPACE_FO == nPrefix &&
            aLocalName.equalsAsciiL( sXML_column_count,
                                      sizeof(sXML_column_count)-1 ) &&
            GetImport().GetMM100UnitConverter().
                                convertNumber( nVal, rValue, 0, SHRT_MAX ) )
        {
            nCount = (sal_Int16)nVal;
        }
    }
}

XMLTextColumnsContext::~XMLTextColumnsContext()
{
    if( pColumns )
    {
        sal_uInt16 nCount = pColumns->Count();
        while( nCount )
        {
            nCount--;
            XMLTextColumnContext_Impl *pColumn = (*pColumns)[nCount];
            pColumns->Remove( nCount, 1 );
            pColumn->ReleaseRef();
        }
    }
    if( pColumnSep )
        pColumnSep->ReleaseRef();

    delete pColumns;
    delete pColumnAttrTokenMap;
}

SvXMLImportContext *XMLTextColumnsContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        rLocalName.equalsAsciiL( sXML_column, sizeof(sXML_column)-1 ) )
    {
        XMLTextColumnContext_Impl *pColumn =
            new XMLTextColumnContext_Impl( GetImport(), nPrefix, rLocalName,
                                           xAttrList, *pColumnAttrTokenMap );

        // add new tabstop to array of tabstops
        if( !pColumns )
            pColumns = new XMLTextColumnsArray_Impl;

        pColumns->Insert( pColumn, pColumns->Count() );
        pColumn->AddRef();

        pContext = pColumn;
    }
    else if( XML_NAMESPACE_STYLE == nPrefix &&
             rLocalName.equalsAsciiL( sXML_column_sep,
                                       sizeof(sXML_column_sep)-1 ) )
    {
        pColumnSep =
            new XMLTextColumnSepContext_Impl( GetImport(), nPrefix, rLocalName,
                                           xAttrList, *pColumnSepAttrTokenMap );
        pColumnSep->AddRef();

        pContext = pColumnSep;
    }
    else
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void XMLTextColumnsContext::EndElement( )
{
    Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),UNO_QUERY);
    if( !xFactory.is() )
        return;

    Reference<XInterface> xIfc = xFactory->createInstance(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextColumns")));
    if( !xIfc.is() )
        return;

    Reference< XTextColumns > xColumns( xIfc, UNO_QUERY );
    if ( 0 == nCount )
    {
        // zero columns = no columns -> 1 column
        xColumns->setColumnCount( 1 );
    }
    else if( pColumns && pColumns->Count() == (sal_uInt16)nCount )
    {
        sal_Int32 nRelWidth = 0;
        sal_uInt16 nColumnsWithWidth = 0;
        for( sal_Int16 i=0; i < nCount; i++ )
        {
            const TextColumn& rColumn =
                (*pColumns)[(sal_uInt16)i]->getTextColumn();
            if( rColumn.Width > 0 )
            {
                nRelWidth += rColumn.Width;
                nColumnsWithWidth++;
            }
        }
        if( nColumnsWithWidth < nCount )
        {
            sal_Int32 nColWidth = 0==nRelWidth
                                        ? USHRT_MAX / nCount
                                        : nRelWidth / nColumnsWithWidth;

            for( i=0; i < nCount; i++ )
            {
                TextColumn& rColumn =
                    (*pColumns)[(sal_uInt16)i]->getTextColumn();
                if( rColumn.Width == 0 )
                {
                    rColumn.Width = nColWidth;
                    nRelWidth += rColumn.Width;
                    if( 0 == --nColumnsWithWidth )
                        break;
                }
            }
        }

        Sequence< TextColumn > aColumns( (sal_Int32)nCount );
        TextColumn *pTextColumns = aColumns.getArray();
        for( i=0; i < nCount; i++ )
            *pTextColumns++ = (*pColumns)[(sal_uInt16)i]->getTextColumn();

        xColumns->setColumns( aColumns );
    }
    else
    {
        xColumns->setColumnCount( nCount );
    }

    Reference < XPropertySet > xPropSet( xColumns, UNO_QUERY );
    if( xPropSet.is() )
    {
        Any aAny;
        sal_Bool bOn = pColumnSep != 0;

        aAny.setValue( &bOn, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( sSeparatorLineIsOn, aAny );

        if( pColumnSep )
        {
            if( pColumnSep->GetWidth() )
            {
                aAny <<= pColumnSep->GetWidth();
                xPropSet->setPropertyValue( sSeparatorLineWidth, aAny );
            }
            if( pColumnSep->GetHeight() )
            {
                aAny <<= pColumnSep->GetHeight();
                xPropSet->setPropertyValue( sSeparatorLineRelativeHeight,
                                            aAny );
            }


            aAny <<= pColumnSep->GetColor();
            xPropSet->setPropertyValue( sSeparatorLineColor, aAny );


            aAny <<= pColumnSep->GetVertAlign();
            xPropSet->setPropertyValue( sSeparatorLineVerticalAlignment, aAny );
        }
    }

    aProp.maValue <<= xColumns;

    SetInsert( sal_True );
    XMLElementPropertyContext::EndElement();

}
