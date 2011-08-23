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

#ifndef _XMLOFF_PROPERTYHANDLER_ADJUSTTYPES_HXX
#include <adjushdl.hxx>
#endif

#ifndef _SOLAR_H 
#include <tools/solar.h>
#endif


#ifndef _XMLOFF_XMLUCONV_HXX 
#include "xmluconv.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_ 
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_PARAGRAPHADJUST_HPP_
#include <com/sun/star/style/ParagraphAdjust.hpp>
#endif


namespace binfilter {

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::binfilter::xmloff::token;

SvXMLEnumMapEntry __READONLY_DATA pXML_Para_Adjust_Enum[] =
{
    { XML_START,		style::ParagraphAdjust_LEFT },
    { XML_END,			style::ParagraphAdjust_RIGHT },
    { XML_CENTER,		style::ParagraphAdjust_CENTER },
    { XML_JUSTIFY,		style::ParagraphAdjust_BLOCK },
    { XML_JUSTIFIED,	style::ParagraphAdjust_BLOCK },	// obsolete
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_Para_Align_Last_Enum[] =
{
    { XML_START,		style::ParagraphAdjust_LEFT },
    { XML_CENTER,		style::ParagraphAdjust_CENTER },
    { XML_JUSTIFY,		style::ParagraphAdjust_BLOCK },
    { XML_JUSTIFIED,	style::ParagraphAdjust_BLOCK },	// obsolete
    { XML_TOKEN_INVALID, 0 }
};

///////////////////////////////////////////////////////////////////////////////
//
// class XMLParaAdjustPropHdl
//

XMLParaAdjustPropHdl::~XMLParaAdjustPropHdl()
{
    // nothing to do
}

sal_Bool XMLParaAdjustPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    sal_Bool bRet = sal_False;
    sal_uInt16 eAdjust;

    if( ( bRet = rUnitConverter.convertEnum( eAdjust, rStrImpValue, pXML_Para_Adjust_Enum ) ) )
        rValue <<= (sal_Int16)eAdjust;

    return bRet; 
}

sal_Bool XMLParaAdjustPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    if(!rValue.hasValue())
        return sal_False;     //added by BerryJia for fixing Bug102407 2002-11-5
    OUStringBuffer aOut;
    sal_Int16 nVal;

    rValue >>= nVal;

    sal_Bool bRet = rUnitConverter.convertEnum( aOut, nVal, pXML_Para_Adjust_Enum, XML_START ); 

    rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLLastLineAdjustPropHdl
//

XMLLastLineAdjustPropHdl::~XMLLastLineAdjustPropHdl()
{
    // nothing to do
}

sal_Bool XMLLastLineAdjustPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    sal_Bool bRet = sal_False;
    sal_uInt16 eAdjust;

    if( ( bRet = rUnitConverter.convertEnum( eAdjust, rStrImpValue, pXML_Para_Align_Last_Enum ) ) )
        rValue <<= (sal_Int16)eAdjust;

    return bRet; 
}

sal_Bool XMLLastLineAdjustPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    OUStringBuffer aOut;
    sal_Int16 nVal;
    sal_Bool bRet = sal_False;

    rValue >>= nVal;

    if( nVal != style::ParagraphAdjust_LEFT )
        bRet = rUnitConverter.convertEnum( aOut, nVal, pXML_Para_Align_Last_Enum, XML_START ); 

    rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

}//end of namespace binfilter
