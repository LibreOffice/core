/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <xmlbahdl.hxx>

#include <tools/debug.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmluconv.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <xmloff/xmltoken.hxx>

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

static bool lcl_xmloff_getAny( const Any& rValue, sal_Int32& nValue,
                            sal_Int8 nBytes )
{
    bool bRet = false;

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

// class XMLNumberPropHdl

XMLNumberPropHdl::~XMLNumberPropHdl()
{
    // nothing to do
}

bool XMLNumberPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue = 0;
    bool bRet = ::sax::Converter::convertNumber( nValue, rStrImpValue );
    lcl_xmloff_setAny( rValue, nValue, nBytes );

    return bRet;
}

bool XMLNumberPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_Int32 nValue;
      OUStringBuffer aOut;

    if( lcl_xmloff_getAny( rValue, nValue, nBytes ) )
    {
        ::sax::Converter::convertNumber( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

// class XMLNumberNonePropHdl

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

bool XMLNumberNonePropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    sal_Int32 nValue = 0;
    if( rStrImpValue == sZeroStr )
    {
        bRet = true;
    }
    else
    {
        bRet = ::sax::Converter::convertNumber( nValue, rStrImpValue );
    }
    lcl_xmloff_setAny( rValue, nValue, nBytes );

    return bRet;
}

bool XMLNumberNonePropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
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

        bRet = true;
    }

    return bRet;
}

// class XMLMeasurePropHdl

XMLMeasurePropHdl::~XMLMeasurePropHdl()
{
    // nothing to do
}

bool XMLMeasurePropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    bool bRet = false;

    sal_Int32 nValue = 0;
    bRet = rUnitConverter.convertMeasureToCore( nValue, rStrImpValue );
    lcl_xmloff_setAny( rValue, nValue, nBytes );

    return bRet;
}

bool XMLMeasurePropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    bool bRet = false;
    sal_Int32 nValue;
      OUStringBuffer aOut;

    if( lcl_xmloff_getAny( rValue, nValue, nBytes ) )
    {
        rUnitConverter.convertMeasureToXML( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

// class XMLBoolFalsePropHdl

XMLBoolFalsePropHdl::~XMLBoolFalsePropHdl()
{
    // nothing to do
}

bool XMLBoolFalsePropHdl::importXML( const OUString&, Any&, const SvXMLUnitConverter& ) const
{
    return false;
}

bool XMLBoolFalsePropHdl::exportXML( OUString& rStrExpValue, const Any& /*rValue*/, const SvXMLUnitConverter& rCnv) const
{
    return XMLBoolPropHdl::exportXML( rStrExpValue, makeAny( sal_False ), rCnv );
}

// class XMLBoolPropHdl

XMLBoolPropHdl::~XMLBoolPropHdl()
{
    // nothing to do
}

bool XMLBoolPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bValue(false);
    bool const bRet = ::sax::Converter::convertBool( bValue, rStrImpValue );
    rValue <<= sal_Bool(bValue);

    return bRet;
}

bool XMLBoolPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
      OUStringBuffer aOut;
    sal_Bool bValue = sal_Bool();

    if (rValue >>= bValue)
    {
        ::sax::Converter::convertBool( aOut, bValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

// class XMLNBoolPropHdl

XMLNBoolPropHdl::~XMLNBoolPropHdl()
{
    // nothing to do
}

bool XMLNBoolPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bValue(false);
    bool const bRet = ::sax::Converter::convertBool( bValue, rStrImpValue );
    rValue <<= sal_Bool(!bValue);

    return bRet;
}

bool XMLNBoolPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
      OUStringBuffer aOut;
    sal_Bool bValue = sal_Bool();

    if (rValue >>= bValue)
    {
        ::sax::Converter::convertBool( aOut, !bValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

// class XMLPercentPropHdl

XMLPercentPropHdl::~XMLPercentPropHdl()
{
    // nothing to do
}

bool XMLPercentPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue = 0;
    bool const bRet = ::sax::Converter::convertPercent( nValue, rStrImpValue );
    lcl_xmloff_setAny( rValue, nValue, nBytes );

    return bRet;
}

bool XMLPercentPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_Int32 nValue;
      OUStringBuffer aOut;

    if( lcl_xmloff_getAny( rValue, nValue, nBytes ) )
    {
        ::sax::Converter::convertPercent( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

// class XMLDoublePercentPropHdl

bool XMLDoublePercentPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;

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

bool XMLDoublePercentPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    double fValue = 0;

    if( rValue >>= fValue )
    {
        fValue *= 100.0;
        if( fValue > 0 ) fValue += 0.5; else    fValue -= 0.5;

        sal_Int32 nValue = (sal_Int32)fValue;

        OUStringBuffer aOut;
        ::sax::Converter::convertPercent( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

// class XMLNegPercentPropHdl

XMLNegPercentPropHdl::~XMLNegPercentPropHdl()
{
    // nothing to do
}

bool XMLNegPercentPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue = 0;
    bool const bRet = ::sax::Converter::convertPercent( nValue, rStrImpValue );
    lcl_xmloff_setAny( rValue, 100-nValue, nBytes );

    return bRet;
}

bool XMLNegPercentPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_Int32 nValue;
      OUStringBuffer aOut;

    if( lcl_xmloff_getAny( rValue, nValue, nBytes ) )
    {
        ::sax::Converter::convertPercent( aOut, 100-nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

// class XMLMeasurePxPropHdl

XMLMeasurePxPropHdl::~XMLMeasurePxPropHdl()
{
    // nothing to do
}

bool XMLMeasurePxPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    sal_Int32 nValue = 0;
    bRet = ::sax::Converter::convertMeasurePx( nValue, rStrImpValue );
    lcl_xmloff_setAny( rValue, nValue, nBytes );

    return bRet;
}

bool XMLMeasurePxPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_Int32 nValue;
      OUStringBuffer aOut;

    if( lcl_xmloff_getAny( rValue, nValue, nBytes ) )
    {
        ::sax::Converter::convertMeasurePx( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

// class XMLColorPropHdl

XMLColorPropHdl::~XMLColorPropHdl()
{
    // Nothing to do
}

bool XMLColorPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;

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

bool XMLColorPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_Int32 nColor = 0;

    OUStringBuffer aOut;
    if( rValue >>= nColor )
    {
        ::sax::Converter::convertColor( aOut, nColor );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }
    else
    {
        Sequence< double > aHSL;
        if( (rValue >>= aHSL) && (aHSL.getLength() == 3) )
        {
            aOut.append( "hsl(" + OUString::number(aHSL[0]) + "," + OUString::number(aHSL[1] * 100.0) + "%," + OUString::number(aHSL[2] * 100.0) + "%)" );
            rStrExpValue = aOut.makeStringAndClear();

            bRet = true;
        }
    }

    return bRet;
}

// class XMLHexPropHdl

XMLHexPropHdl::~XMLHexPropHdl()
{
    // Nothing to do
}

bool XMLHexPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_uInt32 nRsid;

    bRet = SvXMLUnitConverter::convertHex( nRsid, rStrImpValue );
    rValue <<= nRsid;

    return bRet;
}

bool XMLHexPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_uInt32 nRsid = 0;

    OUStringBuffer aOut;
    if( rValue >>= nRsid )
    {
        SvXMLUnitConverter::convertHex( aOut, nRsid );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }
    else
    {
        bRet = false;
    }

    return bRet;
}

// class XMLStringPropHdl

XMLStringPropHdl::~XMLStringPropHdl()
{
    // Nothing to do
}

bool XMLStringPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    rValue <<= rStrImpValue;
    bRet = true;

    return bRet;
}

bool XMLStringPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    if( rValue >>= rStrExpValue )
        bRet = true;

    return bRet;
}

// class XMLStyleNamePropHdl

XMLStyleNamePropHdl::~XMLStyleNamePropHdl()
{
    // Nothing to do
}

bool XMLStyleNamePropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    bool bRet = false;

    if( rValue >>= rStrExpValue )
    {
        rStrExpValue = rUnitConverter.encodeStyleName( rStrExpValue );
        bRet = true;
    }

    return bRet;
}

// class XMLDoublePropHdl

XMLDoublePropHdl::~XMLDoublePropHdl()
{
    // Nothing to do
}

bool XMLDoublePropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    double fDblValue(0.0);
    bool const bRet = ::sax::Converter::convertDouble(fDblValue, rStrImpValue);
    rValue <<= fDblValue;
    return bRet;
}

bool XMLDoublePropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    double fValue = 0;

    if( rValue >>= fValue )
    {
        OUStringBuffer aOut;
        ::sax::Converter::convertDouble( aOut, fValue );
        rStrExpValue = aOut.makeStringAndClear();
        bRet = true;
    }

    return bRet;
}

// class XMLColorTransparentPropHdl

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

bool XMLColorTransparentPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    if( rStrImpValue != sTransparent )
    {
        sal_Int32 nColor(0);
        bRet = ::sax::Converter::convertColor( nColor, rStrImpValue );
        rValue <<= nColor;
    }

    return bRet;
}

bool XMLColorTransparentPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_Int32 nColor = 0;

    if( rStrExpValue == sTransparent )
        bRet = false;
    else if( rValue >>= nColor )
    {
        OUStringBuffer aOut;
        ::sax::Converter::convertColor( aOut, nColor );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

// class XMLIsTransparentPropHdl

XMLIsTransparentPropHdl::XMLIsTransparentPropHdl(
    enum XMLTokenEnum eTransparent, bool bTransPropVal ) :
    sTransparent( GetXMLToken(
        eTransparent != XML_TOKEN_INVALID ? eTransparent : XML_TRANSPARENT ) ),
    bTransPropValue( bTransPropVal )
{
}

XMLIsTransparentPropHdl::~XMLIsTransparentPropHdl()
{
    // Nothing to do
}

bool XMLIsTransparentPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bValue = ( (rStrImpValue == sTransparent) == bTransPropValue);
    rValue.setValue( &bValue, ::getBooleanCppuType() );

    return true;
}

bool XMLIsTransparentPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    // MIB: This looks a bit strange, because bTransPropValue == bValue should
    // do the same, but this only applies if 'true' is represented by the same
    // 8 bit value in bValue and bTransPropValue. Who will ensure this?
    sal_Bool bValue = *(sal_Bool *)rValue.getValue();
    bool bIsTrans = bTransPropValue ? bValue : !bValue;

    if( bIsTrans )
    {
        rStrExpValue = sTransparent;
        bRet = true;
    }

    return bRet;
}

// class XMLColorAutoPropHdl

XMLColorAutoPropHdl::XMLColorAutoPropHdl()
{
    // Nothing to do
}

XMLColorAutoPropHdl::~XMLColorAutoPropHdl()
{
    // Nothing to do
}

bool XMLColorAutoPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;

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

bool XMLColorAutoPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    sal_Int32 nColor = 0;
    if( (rValue >>= nColor) && -1 != nColor )
    {
        OUStringBuffer aOut;
        ::sax::Converter::convertColor( aOut, nColor );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

// class XMLIsAutoColorPropHdl

XMLIsAutoColorPropHdl::XMLIsAutoColorPropHdl()
{
}

XMLIsAutoColorPropHdl::~XMLIsAutoColorPropHdl()
{
    // Nothing to do
}

bool XMLIsAutoColorPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    // An auto color overrides any other color set!
    bool bValue;
    bool const bRet = ::sax::Converter::convertBool( bValue, rStrImpValue );
    if( bRet && bValue )
        rValue <<= (sal_Int32)-1;

    return true;
}

bool XMLIsAutoColorPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_Int32 nColor = 0;

    if( (rValue >>= nColor) && -1 == nColor )
    {
        OUStringBuffer aOut;
        ::sax::Converter::convertBool( aOut, true );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

// class XMLCompareOnlyPropHdl

XMLCompareOnlyPropHdl::~XMLCompareOnlyPropHdl()
{
    // Nothing to do
}

bool XMLCompareOnlyPropHdl::importXML( const OUString&, Any&, const SvXMLUnitConverter& ) const
{
    DBG_ASSERT( !this, "importXML called for compare-only-property" );
    return false;
}

bool XMLCompareOnlyPropHdl::exportXML( OUString&, const Any&, const SvXMLUnitConverter& ) const
{
    DBG_ASSERT( !this, "exportXML called for compare-only-property" );
    return false;
}

// class XMLNumberWithoutZeroPropHdl

XMLNumberWithoutZeroPropHdl::XMLNumberWithoutZeroPropHdl( sal_Int8 nB ) :
    nBytes( nB )
{
}

XMLNumberWithoutZeroPropHdl::~XMLNumberWithoutZeroPropHdl()
{
}

bool XMLNumberWithoutZeroPropHdl::importXML(
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

bool XMLNumberWithoutZeroPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{

    sal_Int32 nValue = 0;
    bool bRet = lcl_xmloff_getAny( rValue, nValue, nBytes );
    bRet &= nValue != 0;

    if( bRet )
    {
          OUStringBuffer aBuffer;
        ::sax::Converter::convertNumber( aBuffer, nValue );
        rStrExpValue = aBuffer.makeStringAndClear();
    }

    return bRet;
}

// class XMLNumberWithAutoInsteadZeroPropHdl

XMLNumberWithAutoInsteadZeroPropHdl::~XMLNumberWithAutoInsteadZeroPropHdl()
{
}

bool XMLNumberWithAutoInsteadZeroPropHdl::importXML(
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
        bRet = true;
    }
    return bRet;
}

bool XMLNumberWithAutoInsteadZeroPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
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

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
