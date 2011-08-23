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


#ifndef _XMLOFF_PROPERTYHANDLER_LINESPACETYPES_HXX
#include <lspachdl.hxx>
#endif


#ifndef _XMLOFF_XMLUCONV_HXX 
#include "xmluconv.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_ 
#include <rtl/ustrbuf.hxx>
#endif


#ifndef _COM_SUN_STAR_STYLE_LINESPACING_HPP_
#include <com/sun/star/style/LineSpacing.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_LINESPACINGMODE_HPP_
#include <com/sun/star/style/LineSpacingMode.hpp>
#endif

namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_CASEMAP_NORMAL;

// this is a copy of defines in svx/inc/escpitem.hxx
#define DFLT_ESC_PROP	 58
#define DFLT_ESC_AUTO_SUPER	101
#define DFLT_ESC_AUTO_SUB  -101

///////////////////////////////////////////////////////////////////////////////
//
// class XMLEscapementPropHdl
//

XMLLineHeightHdl::~XMLLineHeightHdl()
{
    // nothing to do
}

sal_Bool XMLLineHeightHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    style::LineSpacing aLSp;
    sal_Int32 nTemp = 0;

    if( -1 != rStrImpValue.indexOf( sal_Unicode( '%' ) ) )
    {
        aLSp.Mode = style::LineSpacingMode::PROP;
        if(!rUnitConverter.convertPercent( nTemp, rStrImpValue ))
            return sal_False;
        aLSp.Height = nTemp;
    }
    else if( IsXMLToken( rStrImpValue, XML_CASEMAP_NORMAL) )
    {
        aLSp.Mode = style::LineSpacingMode::PROP;
        aLSp.Height = 100;
    }
    else
    {
        aLSp.Mode = style::LineSpacingMode::FIX;
        if(!rUnitConverter.convertMeasure( nTemp, rStrImpValue, 0x0000, 0xffff ))
            return sal_False;
        aLSp.Height = nTemp;
    }

    rValue <<= aLSp;
    return sal_True;
}

sal_Bool XMLLineHeightHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    OUStringBuffer aOut;

    style::LineSpacing aLSp;
    if(!(rValue >>= aLSp))
        return sal_False;

    if( style::LineSpacingMode::PROP != aLSp.Mode && style::LineSpacingMode::FIX  != aLSp.Mode )
        return sal_False;

    if( style::LineSpacingMode::PROP == aLSp.Mode )
    {
        rUnitConverter.convertPercent( aOut, aLSp.Height );
    }
    else
    {
        rUnitConverter.convertMeasure( aOut, aLSp.Height );
    }

    rStrExpValue = aOut.makeStringAndClear();
    return rStrExpValue.getLength() != 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLLineHeightAtLeastHdl
//

XMLLineHeightAtLeastHdl::~XMLLineHeightAtLeastHdl()
{
    // nothing to do
}

sal_Bool XMLLineHeightAtLeastHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    style::LineSpacing aLSp;

    sal_Int32 nTemp;
    aLSp.Mode = style::LineSpacingMode::MINIMUM;
    if(!rUnitConverter.convertMeasure( nTemp, rStrImpValue, 0x0000, 0xffff ))
        return sal_False;
    aLSp.Height = nTemp;

    rValue <<= aLSp;
    return sal_True;
}

sal_Bool XMLLineHeightAtLeastHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    OUStringBuffer aOut;

    style::LineSpacing aLSp;
    if(!(rValue >>= aLSp))
        return sal_False;

    if( style::LineSpacingMode::MINIMUM != aLSp.Mode )
        return sal_False;

    rUnitConverter.convertMeasure( aOut, aLSp.Height );

    rStrExpValue = aOut.makeStringAndClear();
    return rStrExpValue.getLength() != 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLLineSpacingHdl
//

XMLLineSpacingHdl::~XMLLineSpacingHdl()
{
    // nothing to do
}

sal_Bool XMLLineSpacingHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    style::LineSpacing aLSp;
    sal_Int32 nTemp;

    aLSp.Mode = style::LineSpacingMode::LEADING;
    if(!rUnitConverter.convertMeasure( nTemp, rStrImpValue, 0x0000, 0xffff ))
        return sal_False;
    aLSp.Height = nTemp;

    rValue <<= aLSp;
    return sal_True;
}

sal_Bool XMLLineSpacingHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    OUStringBuffer aOut;

    style::LineSpacing aLSp;
    if(!(rValue >>= aLSp))
        return sal_False;

    if( style::LineSpacingMode::LEADING != aLSp.Mode )
        return sal_False;

    rUnitConverter.convertMeasure( aOut, aLSp.Height );

    rStrExpValue = aOut.makeStringAndClear();
    return rStrExpValue.getLength() != 0;
}
}//end of namespace binfilter
