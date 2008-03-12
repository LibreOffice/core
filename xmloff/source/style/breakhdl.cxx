/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: breakhdl.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:49:45 $
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

#ifndef _XMLOFF_PROPERTYHANDLER_BREAKTYPES_HXX
#include <breakhdl.hxx>
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

#ifndef _COM_SUN_STAR_STYLE_BREAKTYPE_HPP_
#include <com/sun/star/style/BreakType.hpp>
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

SvXMLEnumMapEntry pXML_BreakTypes[] =
{
    { XML_AUTO,         0 },
    { XML_COLUMN,       1 },
    { XML_PAGE,         2 },
    { XML_EVEN_PAGE,    2 },
    { XML_ODD_PAGE,     2 },
    { XML_TOKEN_INVALID, 0}
};

///////////////////////////////////////////////////////////////////////////////
//
// class XMLFmtBreakBeforePropHdl
//

XMLFmtBreakBeforePropHdl::~XMLFmtBreakBeforePropHdl()
{
    // Nothing to do
}

sal_Bool XMLFmtBreakBeforePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 nEnum;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( nEnum, rStrImpValue, pXML_BreakTypes );
    if( bRet )
    {
        style::BreakType eBreak;
        switch ( nEnum )
        {
        case 0:
            eBreak = style::BreakType_NONE;
            break;
        case 1:
            eBreak = style::BreakType_COLUMN_BEFORE;
            break;
        default:
            eBreak = style::BreakType_PAGE_BEFORE;
            break;
        }
        rValue <<= eBreak;
    }

    return bRet;
}

sal_Bool XMLFmtBreakBeforePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    style::BreakType eBreak;

    if( !( rValue >>= eBreak ) )
    {
        sal_Int32 nValue = 0;
        if( !( rValue >>= nValue ) )
            return sal_False;

        eBreak = (style::BreakType) nValue;
    }

    sal_uInt16 nEnum = 0;
    switch( eBreak )
    {
        case style::BreakType_COLUMN_BEFORE:
            nEnum = 1;
            break;
        case style::BreakType_PAGE_BEFORE:
            nEnum = 2;
            break;
        case style::BreakType_NONE:
            nEnum = 0;
            break;
        default:
            return sal_False;
    }

    OUStringBuffer aOut;
    /* sal_Bool bOk = */ SvXMLUnitConverter::convertEnum( aOut, nEnum, pXML_BreakTypes );
    rStrExpValue = aOut.makeStringAndClear();

    return sal_True;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLFmtBreakBeforePropHdl
//

XMLFmtBreakAfterPropHdl::~XMLFmtBreakAfterPropHdl()
{
    // Nothing to do
}

sal_Bool XMLFmtBreakAfterPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 nEnum;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( nEnum, rStrImpValue, pXML_BreakTypes );
    if( bRet )
    {
        style::BreakType eBreak;
        switch ( nEnum )
        {
        case 0:
            eBreak = style::BreakType_NONE;
            break;
        case 1:
            eBreak = style::BreakType_COLUMN_AFTER;
            break;
        default:
            eBreak = style::BreakType_PAGE_AFTER;
            break;
        }
        rValue <<= eBreak;
    }

    return bRet;
}

sal_Bool XMLFmtBreakAfterPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    style::BreakType eBreak;

    if( !( rValue >>= eBreak ) )
    {
        sal_Int32 nValue = 0;
        if( !( rValue >>= nValue ) )
            return sal_False;

        eBreak = (style::BreakType) nValue;
    }

    sal_uInt16 nEnum = 0;
    switch( eBreak )
    {
        case style::BreakType_COLUMN_AFTER:
            nEnum = 1;
            break;
        case style::BreakType_PAGE_AFTER:
            nEnum = 2;
            break;
        case style::BreakType_NONE:
            nEnum = 0;
            break;
        default:
            return sal_False;
    }

    OUStringBuffer aOut;
    /* sal_Bool bOk = */ SvXMLUnitConverter::convertEnum( aOut, nEnum, pXML_BreakTypes );
    rStrExpValue = aOut.makeStringAndClear();

    return sal_True;
}
