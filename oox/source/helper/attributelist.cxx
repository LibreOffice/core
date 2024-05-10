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

#include <oox/helper/attributelist.hxx>

#include <comphelper/string.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <sax/fastattribs.hxx>
#include <oox/token/tokenmap.hxx>
#include <o3tl/string_view.hxx>

namespace oox {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace {

const sal_Int32 XSTRING_ENCCHAR_LEN     = 7;

bool lclAddHexDigit( sal_Unicode& orcChar, sal_Unicode cDigit, int nBitShift )
{
    if( ('0' <= cDigit) && (cDigit <= '9') ) { orcChar |= ((cDigit - '0') << nBitShift); return true; }
    if( ('a' <= cDigit) && (cDigit <= 'f') ) { orcChar |= ((cDigit - 'a' + 10) << nBitShift); return true; }
    if( ('A' <= cDigit) && (cDigit <= 'F') ) { orcChar |= ((cDigit - 'A' + 10) << nBitShift); return true; }
    return false;
}

sal_Unicode lclGetXChar( const sal_Unicode*& rpcStr, const sal_Unicode* pcEnd )
{
    sal_Unicode cChar = 0;
    if( (pcEnd - rpcStr >= XSTRING_ENCCHAR_LEN) &&
        (rpcStr[ 0 ] == '_') &&
        (rpcStr[ 1 ] == 'x') &&
        (rpcStr[ 6 ] == '_') &&
        lclAddHexDigit( cChar, rpcStr[ 2 ], 12 ) &&
        lclAddHexDigit( cChar, rpcStr[ 3 ], 8 ) &&
        lclAddHexDigit( cChar, rpcStr[ 4 ], 4 ) &&
        lclAddHexDigit( cChar, rpcStr[ 5 ], 0 ) )
    {
        rpcStr += XSTRING_ENCCHAR_LEN;
        return cChar;
    }
    return *rpcStr++;
}

} // namespace

#define STRING_TO_TOKEN(color) if (sColorName == u"" #color) return XML_##color
sal_Int32 getHighlightColorTokenFromString(std::u16string_view sColorName)
{
    STRING_TO_TOKEN(black);
    STRING_TO_TOKEN(blue);
    STRING_TO_TOKEN(cyan);
    STRING_TO_TOKEN(darkBlue);
    STRING_TO_TOKEN(darkCyan);
    STRING_TO_TOKEN(darkGreen);
    STRING_TO_TOKEN(darkMagenta);
    STRING_TO_TOKEN(darkRed);
    STRING_TO_TOKEN(darkYellow);
    STRING_TO_TOKEN(darkGray);
    STRING_TO_TOKEN(green);
    STRING_TO_TOKEN(lightGray);
    STRING_TO_TOKEN(magenta);
    STRING_TO_TOKEN(red);
    STRING_TO_TOKEN(white);
    STRING_TO_TOKEN(yellow);
    STRING_TO_TOKEN(none);

    return XML_TOKEN_INVALID;
}

sal_Int32 AttributeConversion::decodeToken( std::u16string_view rValue )
{
    return TokenMap::getTokenFromUnicode( rValue );
}

OUString AttributeConversion::decodeXString( const OUString& rValue )
{
    // string shorter than one encoded character - no need to decode
    if( rValue.getLength() < XSTRING_ENCCHAR_LEN )
        return rValue;
    if (rValue.indexOf(u"_x") == -1)
        return rValue;

    OUStringBuffer aBuffer;
    const sal_Unicode* pcStr = rValue.getStr();
    const sal_Unicode* pcEnd = pcStr + rValue.getLength();
    while( pcStr < pcEnd )
        aBuffer.append( lclGetXChar( pcStr, pcEnd ) );
    return comphelper::string::sanitizeStringSurrogates(aBuffer.makeStringAndClear());
}

sal_Int32 AttributeConversion::decodeInteger( std::u16string_view rValue )
{
    return o3tl::toInt32(rValue);
}

sal_uInt32 AttributeConversion::decodeUnsigned( std::u16string_view rValue )
{
    return getLimitedValue< sal_uInt32, sal_Int64 >( o3tl::toInt64(rValue), 0, SAL_MAX_UINT32 );
}

sal_Int64 AttributeConversion::decodeHyper( std::u16string_view rValue )
{
    return o3tl::toInt64(rValue);
}

sal_Int32 AttributeConversion::decodeIntegerHex( std::u16string_view rValue )
{
    // It looks like all Office Open XML attributes containing hexadecimal
    // values are based on xsd:hexBinary and so use an unsigned representation:
    return static_cast< sal_Int32 >(o3tl::toUInt32(rValue, 16));
        //TODO: Change this function to return sal_uInt32 and get rid of the
        // cast, but that will have a ripple effect
}

AttributeList::AttributeList( const Reference< XFastAttributeList >& rxAttribs ) :
    mxAttribs( rxAttribs ),
    mpAttribList( nullptr )
{
    OSL_ENSURE( mxAttribs.is(), "AttributeList::AttributeList - missing attribute list interface" );
}

sax_fastparser::FastAttributeList *AttributeList::getAttribList() const
{
    if( mpAttribList == nullptr )
    {
        mpAttribList = &sax_fastparser::castToFastAttributeList( mxAttribs );
    }
    return mpAttribList;
}

bool AttributeList::hasAttribute( sal_Int32 nAttrToken ) const
{
    return mxAttribs->hasAttribute( nAttrToken );
}

oox::drawingml::Color AttributeList::getHighlightColor(sal_Int32 nAttrToken) const
{
    OUString sColorVal = mxAttribs->getValue(nAttrToken);
    oox::drawingml::Color aColor;
    aColor.setHighlight(getHighlightColorTokenFromString(sColorVal));
    return aColor;
}

// optional return values -----------------------------------------------------

std::optional< sal_Int32 > AttributeList::getToken( sal_Int32 nAttrToken ) const
{
    sal_Int32 nToken = mxAttribs->getOptionalValueToken( nAttrToken, XML_TOKEN_INVALID );
    return nToken == XML_TOKEN_INVALID ? std::optional< sal_Int32 >() : std::optional< sal_Int32 >( nToken );
}

std::optional< OUString > AttributeList::getString( sal_Int32 nAttrToken ) const
{
    // check if the attribute exists (empty string may be different to missing attribute)
    if( mxAttribs->hasAttribute( nAttrToken ) )
        return std::optional< OUString >( mxAttribs->getOptionalValue( nAttrToken ) );
    return std::optional< OUString >();
}

OUString AttributeList::getStringDefaulted( sal_Int32 nAttrToken ) const
{
    // check if the attribute exists (empty string may be different to missing attribute)
    if( mxAttribs->hasAttribute( nAttrToken ) )
        return mxAttribs->getOptionalValue( nAttrToken );
    return OUString();
}

std::optional< OUString > AttributeList::getXString( sal_Int32 nAttrToken ) const
{
    // check if the attribute exists (empty string may be different to missing attribute)
    if( mxAttribs->hasAttribute( nAttrToken ) )
        return std::optional< OUString >( AttributeConversion::decodeXString( mxAttribs->getOptionalValue( nAttrToken ) ) );
    return std::optional< OUString >();
}

std::optional< double > AttributeList::getDouble( sal_Int32 nAttrToken ) const
{
    double nValue;
    bool bValid = getAttribList()->getAsDouble( nAttrToken, nValue );
    return bValid ? std::optional< double >( nValue ) : std::optional< double >();
}

std::optional< sal_Int32 > AttributeList::getInteger( sal_Int32 nAttrToken ) const
{
    sal_Int32 nValue;
    bool bValid = getAttribList()->getAsInteger( nAttrToken, nValue );
    return bValid ? std::optional< sal_Int32 >( nValue ) : std::optional< sal_Int32 >();
}

std::optional< sal_uInt32 > AttributeList::getUnsigned( sal_Int32 nAttrToken ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nAttrToken );
    bool bValid = !aValue.isEmpty();
    return bValid ? std::optional< sal_uInt32 >( AttributeConversion::decodeUnsigned( aValue ) ) : std::optional< sal_uInt32 >();
}

std::optional< sal_Int64 > AttributeList::getHyper( sal_Int32 nAttrToken ) const
{
    std::string_view aValue = getView( nAttrToken );
    bool bValid = !aValue.empty();
    return bValid ? std::optional< sal_Int64 >( o3tl::toInt64( aValue ) ) : std::optional< sal_Int64 >();
}

std::optional< sal_Int32 > AttributeList::getIntegerHex( sal_Int32 nAttrToken ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nAttrToken );
    bool bValid = !aValue.isEmpty();
    return bValid ? std::optional< sal_Int32 >( AttributeConversion::decodeIntegerHex( aValue ) ) : std::optional< sal_Int32 >();
}

std::optional< bool > AttributeList::getBool( sal_Int32 nAttrToken ) const
{
    std::string_view pAttr;

    // catch the common cases as quickly as possible first
    bool bHasAttr = getAttribList()->getAsView( nAttrToken, pAttr );
    if( !bHasAttr )
        return std::optional< bool >();
    if( pAttr == "false" )
        return std::optional< bool >( false );
    if( pAttr == "true" )
        return std::optional< bool >( true );

    // now for all the crazy stuff

    // boolean attributes may be "t", "f", "true", "false", "on", "off", "1", or "0"
    switch( getToken( nAttrToken, XML_TOKEN_INVALID ) )
    {
        case XML_t:     return std::optional< bool >( true );  // used in VML
        case XML_true:  return std::optional< bool >( true );
        case XML_on:    return std::optional< bool >( true );
        case XML_f:     return std::optional< bool >( false ); // used in VML
        case XML_false: return std::optional< bool >( false );
        case XML_off:   return std::optional< bool >( false );
    }
    std::optional< sal_Int32 > onValue = getInteger( nAttrToken );
    return onValue.has_value() ? std::optional< bool >( onValue.value() != 0 ) : std::optional< bool >();
}

std::optional< util::DateTime > AttributeList::getDateTime( sal_Int32 nAttrToken ) const
{
    std::string_view aValue = getView( nAttrToken );
    util::DateTime aDateTime;
    bool bValid = (aValue.size() == 19 || (aValue.size() == 20 && aValue[19] == 'Z')) &&
        (aValue[ 4 ] == '-') && (aValue[ 7 ] == '-') && (aValue[ 10 ] == 'T') &&
        (aValue[ 13 ] == ':') && (aValue[ 16 ] == ':');
    if (!bValid)
    {
        SAL_WARN("oox", "bad date string: " << aValue);
        return std::optional< util::DateTime >();
    }
    aDateTime.Year    = static_cast< sal_uInt16 >( o3tl::toInt32(aValue.substr( 0, 4 )) );
    aDateTime.Month   = static_cast< sal_uInt16 >( o3tl::toInt32(aValue.substr( 5, 2 )) );
    aDateTime.Day     = static_cast< sal_uInt16 >( o3tl::toInt32(aValue.substr( 8, 2 )) );
    aDateTime.Hours   = static_cast< sal_uInt16 >( o3tl::toInt32(aValue.substr( 11, 2 )) );
    aDateTime.Minutes = static_cast< sal_uInt16 >( o3tl::toInt32(aValue.substr( 14, 2 )) );
    aDateTime.Seconds = static_cast< sal_uInt16 >( o3tl::toInt32(aValue.substr( 17, 2 )) );
    return std::optional< util::DateTime >( aDateTime );
}

// defaulted return values ----------------------------------------------------

sal_Int32 AttributeList::getToken( sal_Int32 nAttrToken, sal_Int32 nDefault ) const
{
    return mxAttribs->getOptionalValueToken( nAttrToken, nDefault );
}

OUString AttributeList::getString( sal_Int32 nAttrToken, const OUString& rDefault ) const
{
    // try to avoid slow exception throw/catch if we can
    if (rDefault.isEmpty())
        return mxAttribs->getOptionalValue( nAttrToken );

    try
    {
        return mxAttribs->getValue( nAttrToken );
    }
    catch( Exception& )
    {
    }
    return rDefault;
}

OUString AttributeList::getXString( sal_Int32 nAttrToken, const OUString& rDefault ) const
{
    return getXString( nAttrToken ).value_or( rDefault );
}

std::string_view AttributeList::getView( sal_Int32 nAttrToken ) const
{
    std::string_view p;
    getAttribList()->getAsView(nAttrToken, p);
    return p;
}

double AttributeList::getDouble( sal_Int32 nAttrToken, double fDefault ) const
{
    return getDouble( nAttrToken ).value_or( fDefault );
}

sal_Int32 AttributeList::getInteger( sal_Int32 nAttrToken, sal_Int32 nDefault ) const
{
    return getInteger( nAttrToken ).value_or( nDefault );
}

sal_uInt32 AttributeList::getUnsigned( sal_Int32 nAttrToken, sal_uInt32 nDefault ) const
{
    return getUnsigned( nAttrToken ).value_or( nDefault );
}

sal_Int64 AttributeList::getHyper( sal_Int32 nAttrToken, sal_Int64 nDefault ) const
{
    return getHyper( nAttrToken ).value_or( nDefault );
}

sal_Int32 AttributeList::getIntegerHex( sal_Int32 nAttrToken, sal_Int32 nDefault ) const
{
    return getIntegerHex( nAttrToken ).value_or( nDefault );
}

sal_uInt32 AttributeList::getUnsignedHex( sal_Int32 nAttrToken, sal_uInt32 nDefault ) const
{
    return getIntegerHex( nAttrToken ).value_or( nDefault );
}

bool AttributeList::getBool( sal_Int32 nAttrToken, bool bDefault ) const
{
    return getBool( nAttrToken ).value_or( bDefault );
}

util::DateTime AttributeList::getDateTime( sal_Int32 nAttrToken, const util::DateTime& rDefault ) const
{
    return getDateTime( nAttrToken ).value_or( rDefault );
}

std::vector<sal_Int32> AttributeList::getTokenList(sal_Int32 nAttrToken) const
{
    std::vector<sal_Int32> aValues;
    OUString sValue = getString(nAttrToken, u""_ustr);
    sal_Int32 nIndex = 0;
    do
    {
        aValues.push_back(AttributeConversion::decodeToken(o3tl::getToken(sValue, 0, ' ', nIndex)));
    } while (nIndex >= 0);

    return aValues;
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
