/*************************************************************************
 *
 *  $RCSfile: convert.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:50:05 $
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

#include "convert.hxx"

#include "unohelper.hxx"
#include <memory>
#include <algorithm>
#include <functional>
#include <rtl/math.hxx>
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif
#ifndef _COM_SUN_STAR_XSD_WHITESPACETREATMENT_HPP_
#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>
#endif

using xforms::Convert;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using namespace std;



Convert::Convert()
    : maMap()
{
    init();
}

Convert::~Convert()
{
}


#define ADD_ENTRY(XCONVERT,TYPE) XCONVERT->maMap[ getCppuType( static_cast<TYPE*>( NULL ) ) ] = Convert_t( &lcl_toXSD_##TYPE, &lcl_toAny_##TYPE )

OUString lcl_toXSD_OUString( const Any& rAny )
{ OUString sStr; rAny >>= sStr; return sStr; }

Any lcl_toAny_OUString( const OUString& rStr )
{ Any aAny; aAny <<= rStr; return aAny; }

OUString lcl_toXSD_bool( const Any& rAny )
{ bool b; rAny >>= b; return b ? OUSTRING("true") : OUSTRING("false"); }

Any lcl_toAny_bool( const OUString& rStr )
{
    bool b = ( rStr == OUSTRING("true")  ||  rStr == OUSTRING("1") );
    return makeAny( b );
}

OUString lcl_toXSD_double( const Any& rAny )
{
    double f = 0.0;
    rAny >>= f;

    return rtl::math::isFinite( f )
        ? rtl::math::doubleToUString( f, rtl_math_StringFormat_Automatic,
                                      rtl_math_DecimalPlaces_Max, '.',
                                      sal_True )
        : OUString();
}

Any lcl_toAny_double( const OUString& rString )
{
    rtl_math_ConversionStatus eStatus;
    double f = rtl::math::stringToDouble(
        rString, sal_Unicode('.'), sal_Unicode(','), &eStatus, NULL );
    return ( eStatus == rtl_math_ConversionStatus_Ok ) ? makeAny( f ) : Any();
}


void Convert::init()
{
    ADD_ENTRY( this, OUString );
    ADD_ENTRY( this, bool );
    ADD_ENTRY( this, double );
}


Convert& Convert::get()
{
    // create our Singleton instance on demand
    static Convert* pConvert = NULL;
    if( pConvert == NULL )
        pConvert = new Convert();

    OSL_ENSURE( pConvert != NULL, "no converter?" );
    return *pConvert;
}

bool Convert::hasType( const Type_t& rType )
{
    return maMap.find( rType ) != maMap.end();
}

Convert::Types_t Convert::getTypes()
{
    Types_t aTypes( maMap.size() );
    transform( maMap.begin(), maMap.end(), aTypes.getArray(),
               select1st<Map_t::value_type>() );
    return aTypes;
}

rtl::OUString Convert::toXSD( const Any_t& rAny )
{
    Map_t::iterator aIter = maMap.find( rAny.getValueType() );
    return aIter != maMap.end() ? aIter->second.first( rAny ) : OUString();
}

Convert::Any_t Convert::toAny( const rtl::OUString& rValue,
                               const Type_t& rType )
{
    Map_t::iterator aIter = maMap.find( rType );
    return aIter != maMap.end() ? aIter->second.second( rValue ) : Any_t();
}

//------------------------------------------------------------------------
::rtl::OUString Convert::convertWhitespace( const ::rtl::OUString& _rString, sal_Int16 _nWhitespaceTreatment )
{
    ::rtl::OUString sConverted;
    switch( _nWhitespaceTreatment )
    {
    default:
        OSL_ENSURE( sal_False, "Convert::convertWhitespace: invalid whitespace treatment constant!" );
        // NO break
    case com::sun::star::xsd::WhiteSpaceTreatment::Preserve:
        sConverted = _rString;
        break;
    case com::sun::star::xsd::WhiteSpaceTreatment::Replace:
        sConverted = replaceWhitespace( _rString );
        break;
    case com::sun::star::xsd::WhiteSpaceTreatment::Collapse:
        sConverted = collapseWhitespace( _rString );
        break;
    }
    return sConverted;
}

//------------------------------------------------------------------------
::rtl::OUString Convert::replaceWhitespace( const ::rtl::OUString& _rString )
{
    OUStringBuffer aBuffer( _rString );
    sal_Int32 nLength = aBuffer.getLength();
    const sal_Unicode* pBuffer = aBuffer.getStr();
    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        sal_Unicode c = pBuffer[i];
        if( c == sal_Unicode(0x08) ||
            c == sal_Unicode(0x0A) ||
            c == sal_Unicode(0x0D) )
            aBuffer.setCharAt( i, sal_Unicode(0x20) );
    }
    return aBuffer.makeStringAndClear();
}

//------------------------------------------------------------------------
::rtl::OUString Convert::collapseWhitespace( const ::rtl::OUString& _rString )
{
    sal_Int32 nLength = _rString.getLength();
    OUStringBuffer aBuffer( nLength );
    const sal_Unicode* pStr = _rString.getStr();
    bool bStrip = true;
    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        sal_Unicode c = pStr[i];
        if( c == sal_Unicode(0x08) ||
            c == sal_Unicode(0x0A) ||
            c == sal_Unicode(0x0D) ||
            c == sal_Unicode(0x20) )
        {
            if( ! bStrip )
            {
                aBuffer.append( sal_Unicode(0x20) );
                bStrip = true;
            }
        }
        else
        {
            bStrip = false;
            aBuffer.append( c );
        }
    }
    if( aBuffer[ aBuffer.getLength() - 1 ] == sal_Unicode( 0x20 ) )
        aBuffer.setLength( aBuffer.getLength() - 1 );
    return aBuffer.makeStringAndClear();
}
