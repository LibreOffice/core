/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <xmlbahdl.hxx>

#include <tools/debug.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmluconv.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <xmloff/xmltoken.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

static void lcl_xmloff_setAny( Any& rValue, sal_Int32 nValue, sal_Int8 nBytes )
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

static sal_Bool lcl_xmloff_getAny( const Any& rValue, sal_Int32& nValue,
                            sal_Int8 nBytes )
{
    sal_Bool bRet = sal_False;

    switch( nBytes )
    {
    case 1:
        {
            sal_Int8 nValue8 = 0;
            bRet = rValue >>= nValue8;
            nValue = nValue8;
        }
        break;
    case 2:
        {
            sal_Int16 nValue16 = 0;
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

sal_Bool XMLNumberPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue = 0;
    bool bRet = ::sax::Converter::convertNumber( nValue, rStrImpValue );
    lcl_xmloff_setAny( rValue, nValue, nBytes );

    return bRet;
}

sal_Bool XMLNumberPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nValue;
      OUStringBuffer aOut;

    if( lcl_xmloff_getAny( rValue, nValue, nBytes ) )
    {
        ::sax::Converter::convertNumber( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
// class XMLNumberNonePropHdl
//

XMLNumberNonePropHdl::XMLNumberNonePropHdl( sal_Int8 nB ) :
    sZeroStr( GetXMLToken(XML_NO_LIMIT) ),
    nBytes( nB )
{
}

XMLNumberNonePropHdl::XMLNumberNonePropHdl( enum XMLTokenEnum eZeroString, sal_Int8 nB ) :
    sZeroStr( GetXMLToken( eZeroString ) ),
    nBytes( nB )
{
}

XMLNumberNonePropHdl::~XMLNumberNonePropHdl()
{
    // nothing to do
}

sal_Bool XMLNumberNonePropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    sal_Int32 nValue = 0;
    if( rStrImpValue == sZeroStr )
    {
        bRet = sal_True;
    }
    else
    {
        bRet = ::sax::Converter::convertNumber( nValue, rStrImpValue );
    }
    lcl_xmloff_setAny( rValue, nValue, nBytes );

    return bRet;
}

sal_Bool XMLNumberNonePropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nValue;

    if( lcl_xmloff_getAny( rValue, nValue, nBytes ) )
    {
          OUStringBuffer aOut;

        if( nValue == 0 )
        {
            aOut.append( sZeroStr );
        }
        else
        {
            ::sax::Converter::convertNumber( aOut, nValue );
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

    sal_Int32 nValue = 0;
    bRet = rUnitConverter.convertMeasureToCore( nValue, rStrImpValue );
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
        rUnitConverter.convertMeasureToXML( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLBoolFalsePropHdl
//

XMLBoolFalsePropHdl::~XMLBoolFalsePropHdl()
{
    // nothing to do
}

sal_Bool XMLBoolFalsePropHdl::importXML( const OUString&, Any&, const SvXMLUnitConverter& ) const
{
    return sal_False;
}

sal_Bool XMLBoolFalsePropHdl::exportXML( OUString& rStrExpValue, const Any& /*rValue*/, const SvXMLUnitConverter& rCnv) const
{
    return XMLBoolPropHdl::exportXML( rStrExpValue, makeAny( sal_False ), rCnv );
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLBoolPropHdl
//

XMLBoolPropHdl::~XMLBoolPropHdl()
{
    // nothing to do
}

sal_Bool XMLBoolPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bValue(false);
    bool const bRet = ::sax::Converter::convertBool( bValue, rStrImpValue );
    rValue <<= sal_Bool(bValue);

    return bRet;
}

sal_Bool XMLBoolPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
      OUStringBuffer aOut;
    sal_Bool bValue = sal_Bool();

    if (rValue >>= bValue)
    {
        ::sax::Converter::convertBool( aOut, bValue );
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

sal_Bool XMLNBoolPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bValue(false);
    bool const bRet = ::sax::Converter::convertBool( bValue, rStrImpValue );
    rValue <<= sal_Bool(!bValue);

    return bRet;
}

sal_Bool XMLNBoolPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
      OUStringBuffer aOut;
    sal_Bool bValue = sal_Bool();

    if (rValue >>= bValue)
    {
        ::sax::Converter::convertBool( aOut, !bValue );
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

sal_Bool XMLPercentPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue = 0;
    bool const bRet = ::sax::Converter::convertPercent( nValue, rStrImpValue );
    lcl_xmloff_setAny( rValue, nValue, nBytes );

    return bRet;
}

sal_Bool XMLPercentPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nValue;
      OUStringBuffer aOut;

    if( lcl_xmloff_getAny( rValue, nValue, nBytes ) )
    {
        ::sax::Converter::convertPercent( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLDoublePercentPropHdl
//

sal_Bool XMLDoublePercentPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    double fValue = 1.0;

    if( rStrImpValue.indexOf( (sal_Unicode)'%' ) == -1 )
    {
        fValue = rStrImpValue.toDouble();
    }
    else
    {
        sal_Int32 nValue = 0;
        bRet = ::sax::Converter::convertPercent( nValue, rStrImpValue );
        fValue = ((double)nValue) / 100.0;
    }
    rValue <<= fValue;

    return bRet;
}

sal_Bool XMLDoublePercentPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    double fValue = 0;

    if( rValue >>= fValue )
    {
        fValue *= 100.0;
        if( fValue > 0 ) fValue += 0.5; else    fValue -= 0.5;

        sal_Int32 nValue = (sal_Int32)fValue;

        OUStringBuffer aOut;
        ::sax::Converter::convertPercent( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}


///////////////////////////////////////////////////////////////////////////////
//
// class XMLNegPercentPropHdl
//

XMLNegPercentPropHdl::~XMLNegPercentPropHdl()
{
    // nothing to do
}

sal_Bool XMLNegPercentPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue = 0;
    bool const bRet = ::sax::Converter::convertPercent( nValue, rStrImpValue );
    lcl_xmloff_setAny( rValue, 100-nValue, nBytes );

    return bRet;
}

sal_Bool XMLNegPercentPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nValue;
      OUStringBuffer aOut;

    if( lcl_xmloff_getAny( rValue, nValue, nBytes ) )
    {
        ::sax::Converter::convertPercent( aOut, 100-nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}


///////////////////////////////////////////////////////////////////////////////
//
// class XMLMeasurePxPropHdl
//

XMLMeasurePxPropHdl::~XMLMeasurePxPropHdl()
{
    // nothing to do
}

sal_Bool XMLMeasurePxPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    sal_Int32 nValue = 0;
    bRet = ::sax::Converter::convertMeasurePx( nValue, rStrImpValue );
    lcl_xmloff_setAny( rValue, nValue, nBytes );

    return bRet;
}

sal_Bool XMLMeasurePxPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nValue;
      OUStringBuffer aOut;

    if( lcl_xmloff_getAny( rValue, nValue, nBytes ) )
    {
        ::sax::Converter::convertMeasurePx( aOut, nValue );
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

sal_Bool XMLColorPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    const OUString astrHSL( "hsl"  );
    if( rStrImpValue.matchIgnoreAsciiCase( astrHSL ) )
    {
        sal_Int32 nOpen = rStrImpValue.indexOf( '(' );
        sal_Int32 nClose = rStrImpValue.lastIndexOf( ')' );

        if( (nOpen != -1) && (nClose > nOpen) )
        {
            const OUString aTmp( rStrImpValue.copy( nOpen+1, nClose - nOpen-1) );

            sal_Int32 nIndex = 0;

            Sequence< double > aHSL(3);
            aHSL[0] = aTmp.getToken( 0, ',', nIndex ).toDouble();
            aHSL[1] = aTmp.getToken( 0, ',', nIndex ).toDouble() / 100.0;
            aHSL[2] = aTmp.getToken( 0, ',', nIndex ).toDouble() / 100.0;
            rValue <<= aHSL;
            bRet = true;
        }
    }
    else
    {
        sal_Int32 nColor(0);
        bRet = ::sax::Converter::convertColor( nColor, rStrImpValue );
        rValue <<= nColor;
    }

    return bRet;
}

sal_Bool XMLColorPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nColor = 0;

    OUStringBuffer aOut;
    if( rValue >>= nColor )
    {
        ::sax::Converter::convertColor( aOut, nColor );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }
    else
    {
        Sequence< double > aHSL;
        if( (rValue >>= aHSL) && (aHSL.getLength() == 3) )
        {
            aOut.append( OUString("hsl(") );
            aOut.append( aHSL[0] );
            aOut.append( OUString(",") );
            aOut.append( aHSL[1] * 100.0 );
            aOut.append( OUString("%,") );
            aOut.append( aHSL[2] * 100.0 );
            aOut.append( OUString("%)") );
            rStrExpValue = aOut.makeStringAndClear();

            bRet = sal_True;
        }
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLHexPropHdl
//

XMLHexPropHdl::~XMLHexPropHdl()
{
    // Nothing to do
}

sal_Bool XMLHexPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt32 nRsid;

    bRet = SvXMLUnitConverter::convertHex( nRsid, rStrImpValue );
    rValue <<= nRsid;

    return bRet;
}

sal_Bool XMLHexPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt32 nRsid = 0;

    OUStringBuffer aOut;
    if( rValue >>= nRsid )
    {
        SvXMLUnitConverter::convertHex( aOut, nRsid );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }
    else
    {
        bRet = sal_False;
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

sal_Bool XMLStringPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    rValue <<= rStrImpValue;
    bRet = sal_True;

    return bRet;
}

sal_Bool XMLStringPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    if( rValue >>= rStrExpValue )
        bRet = sal_True;

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLStyleNamePropHdl
//

XMLStyleNamePropHdl::~XMLStyleNamePropHdl()
{
    // Nothing to do
}

sal_Bool XMLStyleNamePropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    if( rValue >>= rStrExpValue )
    {
        rStrExpValue = rUnitConverter.encodeStyleName( rStrExpValue );
        bRet = sal_True;
    }

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

sal_Bool XMLDoublePropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    double fDblValue(0.0);
    bool const bRet = ::sax::Converter::convertDouble(fDblValue, rStrImpValue);
    rValue <<= fDblValue;
    return bRet;
}

sal_Bool XMLDoublePropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    double fValue = 0;

    if( rValue >>= fValue )
    {
        OUStringBuffer aOut;
        ::sax::Converter::convertDouble( aOut, fValue );
        rStrExpValue = aOut.makeStringAndClear();
        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLColorTransparentPropHdl
//

XMLColorTransparentPropHdl::XMLColorTransparentPropHdl(
    enum XMLTokenEnum eTransparent ) :
    sTransparent( GetXMLToken(
        eTransparent != XML_TOKEN_INVALID ? eTransparent : XML_TRANSPARENT ) )
{
    // Nothing to do
}

XMLColorTransparentPropHdl::~XMLColorTransparentPropHdl()
{
    // Nothing to do
}

sal_Bool XMLColorTransparentPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    if( rStrImpValue != sTransparent )
    {
        sal_Int32 nColor(0);
        bRet = ::sax::Converter::convertColor( nColor, rStrImpValue );
        rValue <<= nColor;
    }

    return bRet;
}

sal_Bool XMLColorTransparentPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nColor = 0;

    if( rStrExpValue == sTransparent )
        bRet = sal_False;
    else if( rValue >>= nColor )
    {
        OUStringBuffer aOut;
        ::sax::Converter::convertColor( aOut, nColor );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}


///////////////////////////////////////////////////////////////////////////////
//
// class XMLIsTransparentPropHdl
//

XMLIsTransparentPropHdl::XMLIsTransparentPropHdl(
    enum XMLTokenEnum eTransparent, sal_Bool bTransPropVal ) :
    sTransparent( GetXMLToken(
        eTransparent != XML_TOKEN_INVALID ? eTransparent : XML_TRANSPARENT ) ),
    bTransPropValue( bTransPropVal )
{
}

XMLIsTransparentPropHdl::~XMLIsTransparentPropHdl()
{
    // Nothing to do
}

sal_Bool XMLIsTransparentPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bValue = ( (rStrImpValue == sTransparent) == bTransPropValue);
    rValue.setValue( &bValue, ::getBooleanCppuType() );

    return sal_True;
}

sal_Bool XMLIsTransparentPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    // MIB: This looks a bit strange, because bTransPropValue == bValue should
    // do the same, but this only applies if 'true' is represented by the same
    // 8 bit value in bValue and bTransPropValue. Who will ensure this?
    sal_Bool bValue = *(sal_Bool *)rValue.getValue();
    sal_Bool bIsTrans = bTransPropValue ? bValue : !bValue;

    if( bIsTrans )
    {
        rStrExpValue = sTransparent;
        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLColorAutoPropHdl
//

XMLColorAutoPropHdl::XMLColorAutoPropHdl()
{
    // Nothing to do
}

XMLColorAutoPropHdl::~XMLColorAutoPropHdl()
{
    // Nothing to do
}

sal_Bool XMLColorAutoPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    // This is a multi property: the value might be set to AUTO_COLOR
    // already by the XMLIsAutoColorPropHdl!
    sal_Int32 nColor = 0;
    if( !(rValue >>= nColor) || -1 != nColor )
    {
        bRet = ::sax::Converter::convertColor( nColor, rStrImpValue );
        if( bRet )
            rValue <<= nColor;
    }

    return bRet;
}

sal_Bool XMLColorAutoPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    sal_Int32 nColor = 0;
    if( (rValue >>= nColor) && -1 != nColor )
    {
        OUStringBuffer aOut;
        ::sax::Converter::convertColor( aOut, nColor );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLIsAutoColorPropHdl
//

XMLIsAutoColorPropHdl::XMLIsAutoColorPropHdl()
{
}

XMLIsAutoColorPropHdl::~XMLIsAutoColorPropHdl()
{
    // Nothing to do
}

sal_Bool XMLIsAutoColorPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    // An auto color overrides any other color set!
    bool bValue;
    bool const bRet = ::sax::Converter::convertBool( bValue, rStrImpValue );
    if( bRet && bValue )
        rValue <<= (sal_Int32)-1;

    return sal_True;
}

sal_Bool XMLIsAutoColorPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nColor = 0;

    if( (rValue >>= nColor) && -1 == nColor )
    {
        OUStringBuffer aOut;
        ::sax::Converter::convertBool( aOut, true );
        rStrExpValue = aOut.makeStringAndClear();

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

sal_Bool XMLCompareOnlyPropHdl::importXML( const OUString&, Any&, const SvXMLUnitConverter& ) const
{
    DBG_ASSERT( !this, "importXML called for compare-only-property" );
    return sal_False;
}

sal_Bool XMLCompareOnlyPropHdl::exportXML( OUString&, const Any&, const SvXMLUnitConverter& ) const
{
    DBG_ASSERT( !this, "exportXML called for compare-only-property" );
    return sal_False;
}

///////////////////////////////////////////////////////////////////////////////
// class XMLNumberWithoutZeroPropHdl
//

XMLNumberWithoutZeroPropHdl::XMLNumberWithoutZeroPropHdl( sal_Int8 nB ) :
    nBytes( nB )
{
}

XMLNumberWithoutZeroPropHdl::~XMLNumberWithoutZeroPropHdl()
{
}

sal_Bool XMLNumberWithoutZeroPropHdl::importXML(
    const OUString& rStrImpValue,
    Any& rValue,
    const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue = 0;
    bool const bRet = ::sax::Converter::convertNumber( nValue, rStrImpValue );
    if( bRet )
        lcl_xmloff_setAny( rValue, nValue, nBytes );
    return bRet;
}

sal_Bool XMLNumberWithoutZeroPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{

    sal_Int32 nValue = 0;
    sal_Bool bRet = lcl_xmloff_getAny( rValue, nValue, nBytes );
    bRet &= nValue != 0;

    if( bRet )
    {
          OUStringBuffer aBuffer;
        ::sax::Converter::convertNumber( aBuffer, nValue );
        rStrExpValue = aBuffer.makeStringAndClear();
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
// class XMLNumberWithAutoInsteadZeroPropHdl
//

XMLNumberWithAutoInsteadZeroPropHdl::~XMLNumberWithAutoInsteadZeroPropHdl()
{
}

sal_Bool XMLNumberWithAutoInsteadZeroPropHdl::importXML(
    const OUString& rStrImpValue,
    Any& rValue,
    const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue = 0;
    bool bRet = ::sax::Converter::convertNumber( nValue, rStrImpValue );
    if( bRet )
        lcl_xmloff_setAny( rValue, nValue, 2 );
    else if( rStrImpValue == GetXMLToken( XML_AUTO ) )
    {
        rValue <<= (sal_Int16)nValue;
        bRet = sal_True;
    }
    return bRet;
}

sal_Bool XMLNumberWithAutoInsteadZeroPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{

    sal_Int32 nValue = 0;
    lcl_xmloff_getAny( rValue, nValue, 2 );

    if( 0 == nValue )
        rStrExpValue = GetXMLToken( XML_AUTO );
    else
    {
        OUStringBuffer aBuffer;
        ::sax::Converter::convertNumber( aBuffer, nValue );
        rStrExpValue = aBuffer.makeStringAndClear();
    }

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
