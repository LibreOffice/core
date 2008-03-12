/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: csmaphdl.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:50:47 $
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

#ifndef _XMLOFF_PROPERTYHANDLER_CASEMAPTYPES_HXX
#include <csmaphdl.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_CASEMAP_HPP_
#include <com/sun/star/style/CaseMap.hpp>
#endif


#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include <xmloff/xmlelement.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

static SvXMLEnumMapEntry pXML_Casemap_Enum[] =
{
    { XML_NONE,                 style::CaseMap::NONE },
    { XML_CASEMAP_LOWERCASE,    style::CaseMap::LOWERCASE },
    { XML_CASEMAP_UPPERCASE,    style::CaseMap::UPPERCASE },
    { XML_CASEMAP_CAPITALIZE,   style::CaseMap::TITLE },
    { XML_TOKEN_INVALID,        0 }
};

///////////////////////////////////////////////////////////////////////////////
//
// class XMLPosturePropHdl
//

XMLCaseMapPropHdl::~XMLCaseMapPropHdl()
{
    // nothing to do
}

sal_Bool XMLCaseMapPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 nVal;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum(
        nVal, rStrImpValue, pXML_Casemap_Enum );
    if( ( bRet ) )
        rValue <<= nVal;

    return bRet;
}

sal_Bool XMLCaseMapPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nValue = sal_uInt16();
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        bRet = SvXMLUnitConverter::convertEnum(
            aOut, nValue, pXML_Casemap_Enum );
        if( bRet )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLCaseMapVariantHdl
//

XMLCaseMapVariantHdl::~XMLCaseMapVariantHdl()
{
    // nothing to do
}

sal_Bool XMLCaseMapVariantHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    if( IsXMLToken( rStrImpValue, XML_CASEMAP_SMALL_CAPS ) )
    {
        rValue <<= (sal_Int16)style::CaseMap::SMALLCAPS;
        bRet = sal_True;
    }
    else if( IsXMLToken( rStrImpValue, XML_CASEMAP_NORMAL ) )
    {
        rValue <<= (sal_Int16)style::CaseMap::NONE;
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool XMLCaseMapVariantHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 nValue = sal_uInt16();
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        switch( nValue )
        {
        case style::CaseMap::NONE:
            aOut.append( GetXMLToken(XML_CASEMAP_NORMAL) );
            break;
        case style::CaseMap::SMALLCAPS:
            aOut.append( GetXMLToken(XML_CASEMAP_SMALL_CAPS) );
            break;
        }
    }

    rStrExpValue = aOut.makeStringAndClear();
    return rStrExpValue.getLength() != 0;
}
