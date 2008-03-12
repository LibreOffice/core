/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lspachdl.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:52:29 $
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


#ifndef _XMLOFF_PROPERTYHANDLER_LINESPACETYPES_HXX
#include <lspachdl.hxx>
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

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_LINESPACING_HPP_
#include <com/sun/star/style/LineSpacing.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_LINESPACINGMODE_HPP_
#include <com/sun/star/style/LineSpacingMode.hpp>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include <xmloff/xmlelement.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_CASEMAP_NORMAL;

// this is a copy of defines in svx/inc/escpitem.hxx
#define DFLT_ESC_PROP    58
#define DFLT_ESC_AUTO_SUPER 101
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
        aLSp.Height = sal::static_int_cast< sal_Int16 >(nTemp);
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
        aLSp.Height = sal::static_int_cast< sal_Int16 >(nTemp);
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
    aLSp.Height = sal::static_int_cast< sal_Int16 >(nTemp);

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
    aLSp.Height = sal::static_int_cast< sal_Int16 >(nTemp);

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
