/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_PROPERTYHANDLER_CASEMAPTYPES_HXX
#include <csmaphdl.hxx>
#endif


#ifndef _XMLOFF_XMLUCONV_HXX 
#include "xmluconv.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_ 
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_CASEMAP_HPP_
#include <com/sun/star/style/CaseMap.hpp>
#endif



namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

static SvXMLEnumMapEntry pXML_Casemap_Enum[] =
{
    { XML_NONE,				    style::CaseMap::NONE },
    { XML_CASEMAP_LOWERCASE,	style::CaseMap::LOWERCASE },
    { XML_CASEMAP_UPPERCASE,	style::CaseMap::UPPERCASE },
    { XML_CASEMAP_CAPITALIZE,	style::CaseMap::TITLE },
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

sal_Bool XMLCaseMapPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    sal_Bool bRet = sal_False;
    sal_uInt16 nVal;

    if( ( bRet = rUnitConverter.convertEnum( nVal, rStrImpValue, pXML_Casemap_Enum ) ) )
        rValue <<= nVal;

    return bRet; 
}

sal_Bool XMLCaseMapPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    sal_Bool bRet = sal_False;
    sal_uInt16 nValue;
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        if( ( bRet = rUnitConverter.convertEnum( aOut, nValue, pXML_Casemap_Enum ) ) )
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

sal_Bool XMLCaseMapVariantHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
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

sal_Bool XMLCaseMapVariantHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    sal_uInt16 nValue;
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
    return rStrExpValue.getLength();
}
}//end of namespace binfilter
