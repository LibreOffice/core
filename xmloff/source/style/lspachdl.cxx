/*************************************************************************
 *
 *  $RCSfile: lspachdl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:05 $
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


#ifndef _XMLOFF_PROPERTYHANDLER_LINESPACETYPES_HXX
#include <lspachdl.hxx>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
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
#include "xmlelement.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

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
        aLSp.Height = nTemp;
    }
    else if( rStrImpValue.compareToAscii( sXML_casemap_normal ) )
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
