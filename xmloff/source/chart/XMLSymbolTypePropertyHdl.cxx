/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLSymbolTypePropertyHdl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:17:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "XMLSymbolTypePropertyHdl.hxx"

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTERRORINDICATORTYPE_HPP_
#include <com/sun/star/chart/ChartErrorIndicatorType.hpp>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

using namespace ::xmloff::token;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace
{
struct SvXMLSignedEnumMapEntry
{
    ::xmloff::token::XMLTokenEnum   eToken;
    sal_Int32                       nValue;
};

SvXMLSignedEnumMapEntry aXMLChartSymbolTypeEnumMap[] =
{
    { XML_NONE,                -3 },
    { XML_AUTOMATIC,           -2 },
    { XML_IMAGE,               -1 },
    { XML_TOKEN_INVALID,        0 }
};

SvXMLSignedEnumMapEntry aXMLChartSymbolNameMap[] =
{
    { XML_GRADIENTSTYLE_SQUARE, 0 },  // "square"
    { XML_DIAMOND,              1 },
    { XML_ARROW_DOWN,           2 },
    { XML_ARROW_UP,             3 },
    { XML_ARROW_RIGHT,          4 },
    { XML_ARROW_LEFT,           5 },
    { XML_BOW_TIE,              6 },
    { XML_HOURGLASS,            7 },
    { XML_TOKEN_INVALID,        0 }
};

sal_Bool lcl_convertEnum(
    OUStringBuffer & rBuffer,
    sal_Int32 nValue,
    const SvXMLSignedEnumMapEntry *pMap )
{
    enum XMLTokenEnum eTok = XML_TOKEN_INVALID;

    while( pMap->eToken != XML_TOKEN_INVALID )
    {
        if( pMap->nValue == nValue )
        {
            eTok = pMap->eToken;
            break;
        }
        pMap++;
    }

    if( eTok != XML_TOKEN_INVALID )
        rBuffer.append( GetXMLToken(eTok) );

    return (eTok != XML_TOKEN_INVALID);
}

sal_Bool lcl_convertEnum(
    sal_Int32 & rEnum,
    const OUString & rValue,
    const SvXMLSignedEnumMapEntry *pMap )
{
    while( pMap->eToken != XML_TOKEN_INVALID )
    {
        if( IsXMLToken( rValue, pMap->eToken ) )
        {
            rEnum = pMap->nValue;
            return sal_True;
        }
        pMap++;
    }
    return sal_False;
}

} // anonymous namespace

using namespace com::sun::star;

XMLSymbolTypePropertyHdl::XMLSymbolTypePropertyHdl( bool bIsNamedSymbol )
        : m_bIsNamedSymbol( bIsNamedSymbol )
{}

XMLSymbolTypePropertyHdl::~XMLSymbolTypePropertyHdl()
{}

sal_Bool XMLSymbolTypePropertyHdl::importXML( const OUString& rStrImpValue,
                                                  uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    sal_Bool bResult = sal_False;

    if( m_bIsNamedSymbol )
    {
        sal_Int32 nValue = -3; // NONE
        bResult = lcl_convertEnum( nValue, rStrImpValue, aXMLChartSymbolNameMap );
        rValue <<= nValue;
    }
    else
    {
        sal_Int32 nValue = -3; // NONE
        bResult = lcl_convertEnum( nValue, rStrImpValue, aXMLChartSymbolTypeEnumMap );
        rValue <<= nValue;
    }

    return bResult;
}

sal_Bool XMLSymbolTypePropertyHdl::exportXML( OUString& rStrExpValue,
                                              const uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    sal_Bool bResult = sal_False;

    sal_Int32 nType = -3; // NONE
    rValue >>= nType;

    if( m_bIsNamedSymbol )
    {
        OUStringBuffer aBuf;
        bResult = lcl_convertEnum( aBuf, nType, aXMLChartSymbolNameMap );
        rStrExpValue = aBuf.makeStringAndClear();
    }
    else
    {
        if( nType < 0 )
        {
            OUStringBuffer aBuf;
            bResult = lcl_convertEnum( aBuf, nType, aXMLChartSymbolTypeEnumMap );
            rStrExpValue = aBuf.makeStringAndClear();
        }
        else
        {
            bResult = true;
            rStrExpValue = GetXMLToken( XML_NAMED_SYMBOL );
        }
    }

    return bResult;
}
