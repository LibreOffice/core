/*************************************************************************
 *
 *  $RCSfile: xmlbahdl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dr $ $Date: 2000-10-20 16:35:57 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX
#include <xmlbahdl.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#include "xmlkywd.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::comphelper;

void lcl_xmloff_setAny( Any& rValue, sal_Int32 nValue, sal_Int8 nBytes )
{
    switch( nBytes )
    {
    case 1:
        if( nValue < SCHAR_MIN )
            nValue = SCHAR_MIN;
        else if( nValue > SCHAR_MAX )
            nValue = SCHAR_MAX;
        rValue <<= (sal_Int8)nValue;
        break;
    case 2:
        if( nValue < SHRT_MIN )
            nValue = SHRT_MIN;
        else if( nValue > SHRT_MAX )
            nValue = SHRT_MAX;
        rValue <<= (sal_Int16)nValue;
        break;
    case 4:
        rValue <<= nValue;
        break;
    }
}

sal_Bool lcl_xmloff_getAny( const Any& rValue, sal_Int32& nValue,
                            sal_Int8 nBytes )
{
    sal_Bool bRet = sal_False;

    switch( nBytes )
    {
    case 1:
        {
            sal_Int8 nValue8;
            bRet = rValue >>= nValue8;
            nValue = nValue8;
        }
        break;
    case 2:
        {
            sal_Int16 nValue16;
            bRet = rValue >>= nValue16;
            nValue = nValue16;
        }
        break;
    case 4:
        bRet = rValue >>= nValue;
        break;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLNumberPropHdl
//

XMLNumberPropHdl::~XMLNumberPropHdl()
{
    // nothing to do
}

sal_Bool XMLNumberPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    sal_Int32 nValue;
    bRet = rUnitConverter.convertNumber( nValue, rStrImpValue );
    lcl_xmloff_setAny( rValue, nValue, nBytes );

    return bRet;
}

sal_Bool XMLNumberPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nValue;
      OUStringBuffer aOut;

    if( lcl_xmloff_getAny( rValue, nValue, nBytes ) )
    {
         rUnitConverter.convertNumber( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
// class XMLNumberNonePropHdl
//

XMLNumberNonePropHdl::~XMLNumberNonePropHdl()
{
    // nothing to do
}

sal_Bool XMLNumberNonePropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    sal_Int32 nValue;
    if( rStrImpValue.compareToAscii( sXML_no_limit ) == 0 )
    {
        nValue = 0;
        bRet = sal_True;
    }
    else
    {
        bRet = rUnitConverter.convertNumber( nValue, rStrImpValue );
    }
    rValue <<= nValue;

    return bRet;
}

sal_Bool XMLNumberNonePropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nValue;

    if( rValue >>= nValue )
    {
          OUStringBuffer aOut;

        if( nValue == 0 )
        {
            aOut.appendAscii( sXML_no_limit );
        }
        else
        {
             rUnitConverter.convertNumber( aOut, nValue );
        }

        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLMeasurePropHdl
//

XMLMeasurePropHdl::~XMLMeasurePropHdl()
{
    // nothing to do
}

sal_Bool XMLMeasurePropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    sal_Int32 nValue;
    bRet = rUnitConverter.convertMeasure( nValue, rStrImpValue );
    lcl_xmloff_setAny( rValue, nValue, nBytes );

    return bRet;
}

sal_Bool XMLMeasurePropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nValue;
      OUStringBuffer aOut;

    if( lcl_xmloff_getAny( rValue, nValue, nBytes ) )
    {
         rUnitConverter.convertMeasure( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLBoolPropHdl
//

XMLBoolPropHdl::~XMLBoolPropHdl()
{
    // nothing to do
}

sal_Bool XMLBoolPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    sal_Bool bValue;
    bRet = rUnitConverter.convertBool( bValue, rStrImpValue );
    rValue <<= sal_Bool(bValue);

    return bRet;
}

sal_Bool XMLBoolPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
      OUStringBuffer aOut;
    sal_Bool bValue;

    if (rValue >>= bValue)
    {
        rUnitConverter.convertBool( aOut, bValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLNBoolPropHdl
//

XMLNBoolPropHdl::~XMLNBoolPropHdl()
{
    // nothing to do
}

sal_Bool XMLNBoolPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    sal_Bool bValue;
    bRet = rUnitConverter.convertBool( bValue, rStrImpValue );
    rValue <<= sal_Bool(!bValue);

    return bRet;
}

sal_Bool XMLNBoolPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
      OUStringBuffer aOut;
    sal_Bool bValue;

    if (rValue >>= bValue)
    {
        rUnitConverter.convertBool( aOut, !bValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLPercentPropHdl
//

XMLPercentPropHdl::~XMLPercentPropHdl()
{
    // nothing to do
}

sal_Bool XMLPercentPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    sal_Int32 nValue;
    bRet = rUnitConverter.convertPercent( nValue, rStrImpValue );
    lcl_xmloff_setAny( rValue, nValue, nBytes );

    return bRet;
}

sal_Bool XMLPercentPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nValue;
      OUStringBuffer aOut;

    if( lcl_xmloff_getAny( rValue, nValue, nBytes ) )
    {
         rUnitConverter.convertPercent( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLColorPropHdl
//

XMLColorPropHdl::~XMLColorPropHdl()
{
    // Nothing to do
}

sal_Bool XMLColorPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    Color aColor;

    bRet = rUnitConverter.convertColor( aColor, rStrImpValue );
    rValue <<= (sal_Int32)( aColor.GetColor() );

    return bRet;
}

sal_Bool XMLColorPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    Color aColor;
    sal_Int32 nColor;

    if( rValue >>= nColor )
    {
        aColor.SetColor( nColor );

        OUStringBuffer aOut;
        rUnitConverter.convertColor( aOut, aColor );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLStringPropHdl
//

XMLStringPropHdl::~XMLStringPropHdl()
{
    // Nothing to do
}

sal_Bool XMLStringPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    rValue <<= rStrImpValue;
    bRet = sal_True;

    return bRet;
}

sal_Bool XMLStringPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    if( rValue >>= rStrExpValue )
        bRet = sal_True;

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLDoublePropHdl
//

XMLDoublePropHdl::~XMLDoublePropHdl()
{
    // Nothing to do
}

sal_Bool XMLDoublePropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    double fDblValue;
    sal_Bool bRet = rUnitConverter.convertNumber( fDblValue, rStrImpValue );
    rValue <<= fDblValue;
    return bRet;
}

sal_Bool XMLDoublePropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    double fValue;

    if( rValue >>= fValue )
    {
        OUStringBuffer aOut;
        rUnitConverter.convertNumber( aOut, fValue );
        rStrExpValue = aOut.makeStringAndClear();
        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLColorTransparentPropHdl
//

XMLColorTransparentPropHdl::~XMLColorTransparentPropHdl()
{
    // Nothing to do
}

sal_Bool XMLColorTransparentPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    if( rStrImpValue.compareToAscii( sXML_transparent ) != 0 )
    {
        Color aColor;
        bRet = rUnitConverter.convertColor( aColor, rStrImpValue );
        rValue <<= (sal_Int32)( aColor.GetColor() );
    }

    return bRet;
}

sal_Bool XMLColorTransparentPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nColor;

    if( rStrExpValue.compareToAscii( sXML_transparent ) == 0 )
        bRet = sal_False;
    else if( rValue >>= nColor )
    {
        Color aColor( nColor );
        OUStringBuffer aOut;
        rUnitConverter.convertColor( aOut, aColor );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLIsTransparentPropHdl
//

XMLIsTransparentPropHdl::~XMLIsTransparentPropHdl()
{
    // Nothing to do
}

sal_Bool XMLIsTransparentPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bValue = (rStrImpValue.compareToAscii( sXML_transparent ) == 0);
    rValue.setValue( &bValue, ::getBooleanCppuType() );

    return sal_True;
}

sal_Bool XMLIsTransparentPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Bool bIsTrans = *(sal_Bool *)rValue.getValue();

    if( bIsTrans )
    {
        rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_transparent ) );
        bRet = sal_True;
    }

    return bRet;
}


///////////////////////////////////////////////////////////////////////////////
//
// class XMLCompareOnlyPropHdl
//

XMLCompareOnlyPropHdl::~XMLCompareOnlyPropHdl()
{
    // Nothing to do
}

sal_Bool XMLCompareOnlyPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    DBG_ASSERT( !this, "importXML called for compare-only-property" );
    return sal_False;
}

sal_Bool XMLCompareOnlyPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    DBG_ASSERT( !this, "exportXML called for compare-only-property" );
    return sal_False;
}


///////////////////////////////////////////////////////////////////////////////
//
// class XMLBoolValuesPropHdl
//

XMLBoolValuesPropHdl::XMLBoolValuesPropHdl(
        const sal_Char* sTrueValue,
        const sal_Char* sFalseValue ) :
    sTrueVal( OUString::createFromAscii( sTrueValue ) ),
    sFalseVal( OUString::createFromAscii( sFalseValue ) )
{
}

XMLBoolValuesPropHdl::~XMLBoolValuesPropHdl()
{
}

sal_Bool XMLBoolValuesPropHdl::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bTrue  = (rStrImpValue == sTrueVal);
    sal_Bool bFalse = !bTrue && (rStrImpValue == sFalseVal);

    if( bTrue || bFalse )
        setBOOL( rValue, bTrue );

    return (bTrue || bFalse);
}

sal_Bool XMLBoolValuesPropHdl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    rStrExpValue = getBOOL( rValue ) ? sTrueVal : sFalseVal;
    return sal_True;
}


