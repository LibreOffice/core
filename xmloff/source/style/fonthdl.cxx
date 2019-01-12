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

#include <sal/config.h>

#include <string_view>

#include "fonthdl.hxx"

#include <sax/tools/converter.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlement.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <tools/fontenum.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

static const SvXMLEnumMapEntry<FontFamily>* lcl_getFontFamilyGenericMapping()
{
    static SvXMLEnumMapEntry<FontFamily> const aFontFamilyGenericMapping[] =
    {
        { XML_DECORATIVE,       FAMILY_DECORATIVE },

        { XML_MODERN,           FAMILY_MODERN   },
        { XML_ROMAN,            FAMILY_ROMAN    },
        { XML_SCRIPT,           FAMILY_SCRIPT   },
        { XML_SWISS,            FAMILY_SWISS    },
        { XML_SYSTEM,           FAMILY_SYSTEM   },
        { XML_TOKEN_INVALID,    FontFamily(0)   }
    };
    return aFontFamilyGenericMapping;
}

static SvXMLEnumMapEntry<FontPitch> const aFontPitchMapping[] =
{
    { XML_FIXED,            PITCH_FIXED     },
    { XML_VARIABLE,         PITCH_VARIABLE  },
    { XML_TOKEN_INVALID,    FontPitch(0)    }
};

// class XMLFontFamilyNamePropHdl

XMLFontFamilyNamePropHdl::~XMLFontFamilyNamePropHdl()
{
    // Nothing to do
}

bool XMLFontFamilyNamePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    OUStringBuffer sValue;
    sal_Int32 nPos = 0;

    do
    {
        sal_Int32 nFirst = nPos;
        nPos = ::sax::Converter::indexOfComma( rStrImpValue, nPos );
        sal_Int32 nLast = (-1 == nPos ? rStrImpValue.getLength() - 1 : nPos - 1);

        // skip trailing blanks
        while( nLast > nFirst && ' ' == rStrImpValue[nLast] )
            nLast--;

        // skip leading blanks
        while(nFirst <= nLast && ' ' == rStrImpValue[nFirst])
            nFirst++;

        // remove quotes
        sal_Unicode c = nFirst > nLast ? 0 : rStrImpValue[nFirst];
        if( nFirst < nLast && ('\'' == c || '\"' == c) && rStrImpValue[nLast] == c )
        {
            nFirst++;
            nLast--;
        }

        if( nFirst <= nLast )
        {
            if( !sValue.isEmpty() )
                sValue.append(';');

            sValue.append(std::u16string_view(rStrImpValue).substr(nFirst, nLast-nFirst+1));
        }

        if( -1 != nPos )
            nPos++;
    }
    while( -1 != nPos );

    if (!sValue.isEmpty())
    {
        rValue <<= sValue.makeStringAndClear();
        bRet = true;
    }

    return bRet;
}

bool XMLFontFamilyNamePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    OUString aStrFamilyName;

    if( rValue >>= aStrFamilyName )
    {
        OUStringBuffer sValue( aStrFamilyName.getLength() + 2 );
        sal_Int32 nPos = 0;
        do
        {
            sal_Int32 nFirst = nPos;
            nPos = aStrFamilyName.indexOf( ';', nPos );
            sal_Int32 nLast = (-1 == nPos ? aStrFamilyName.getLength() : nPos);

            // Set position to the character behind the ';', so we won't
            // forget this.
            if( -1 != nPos )
                nPos++;

            // If the property value was empty, we stop now.
            // If there is a ';' at the first position, the empty name
            // at the start will be removed.
            if( 0 == nLast )
                continue;

            // nFirst and nLast now denote the first and last character of
            // one font name.
            nLast--;

            // skip trailing blanks
            while(  nLast > nFirst && ' ' == aStrFamilyName[nLast] )
                nLast--;

            // skip leading blanks
            while( nFirst <= nLast && ' ' == aStrFamilyName[nFirst] )
                nFirst++;

            if( nFirst <= nLast )
            {
                if( !sValue.isEmpty() )
                {
                    sValue.append( ',' );
                    sValue.append( ' ' );
                }
                sal_Int32 nLen = nLast-nFirst+1;
                OUString sFamily( aStrFamilyName.copy( nFirst, nLen ) );
                bool bQuote = false;
                for( sal_Int32 i=0; i < nLen; i++ )
                {
                    sal_Unicode c = sFamily[i];
                    if( ' ' == c || ',' == c )
                    {
                        bQuote = true;
                        break;
                    }
                }
                if( bQuote )
                    sValue.append( '\'' );
                sValue.append( sFamily );
                if( bQuote )
                    sValue.append( '\'' );
            }
        }
        while( -1 != nPos );

        rStrExpValue = sValue.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

// class XMLFontFamilyPropHdl

XMLFontFamilyPropHdl::~XMLFontFamilyPropHdl()
{
    // Nothing to do
}

bool XMLFontFamilyPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    FontFamily eNewFamily;
    bool bRet = SvXMLUnitConverter::convertEnum( eNewFamily, rStrImpValue, lcl_getFontFamilyGenericMapping() );
    if( bRet )
        rValue <<= static_cast<sal_Int16>(eNewFamily);

    return bRet;
}

bool XMLFontFamilyPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    OUStringBuffer aOut;

    sal_Int16 nFamily = sal_Int16();
    if( rValue >>= nFamily )
    {
        FontFamily eFamily = static_cast<FontFamily>(nFamily);
        if( eFamily != FAMILY_DONTKNOW )
            bRet = SvXMLUnitConverter::convertEnum( aOut, eFamily, lcl_getFontFamilyGenericMapping() );
    }

    rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

// class XMLFontEncodingPropHdl

XMLFontEncodingPropHdl::~XMLFontEncodingPropHdl()
{
    // Nothing to do
}

bool XMLFontEncodingPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    if( IsXMLToken( rStrImpValue, XML_X_SYMBOL ) )
        rValue <<= sal_Int16(RTL_TEXTENCODING_SYMBOL);

    return true;
}

bool XMLFontEncodingPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    OUStringBuffer aOut;
    sal_Int16 nSet = sal_Int16();

    if( rValue >>= nSet )
    {
        if( static_cast<rtl_TextEncoding>(nSet) == RTL_TEXTENCODING_SYMBOL )
        {
            aOut.append( GetXMLToken(XML_X_SYMBOL) );
            rStrExpValue = aOut.makeStringAndClear();
            bRet = true;
        }
    }

    return bRet;
}

// class XMLFontPitchPropHdl

XMLFontPitchPropHdl::~XMLFontPitchPropHdl()
{
    // Nothing to do
}

bool XMLFontPitchPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    FontPitch eNewPitch;
    bool bRet = SvXMLUnitConverter::convertEnum( eNewPitch, rStrImpValue, aFontPitchMapping );
    if( bRet )
        rValue <<= static_cast<sal_Int16>(eNewPitch);

    return bRet;
}

bool XMLFontPitchPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_Int16 nPitch = sal_Int16();
    OUStringBuffer aOut;

    FontPitch ePitch = PITCH_DONTKNOW;
    if( rValue >>= nPitch )
        ePitch =  static_cast<FontPitch>(nPitch);

    if( PITCH_DONTKNOW != ePitch )
    {
        bRet = SvXMLUnitConverter::convertEnum( aOut, ePitch, aFontPitchMapping, XML_FIXED );
        rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
