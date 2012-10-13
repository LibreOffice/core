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


#include <fonthdl.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <tools/fontenum.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

static const SvXMLEnumMapEntry* lcl_getFontFamilyGenericMapping()
{
    static SvXMLEnumMapEntry const aFontFamilyGenericMapping[] =
    {
        { XML_DECORATIVE,       FAMILY_DECORATIVE },

        { XML_MODERN,           FAMILY_MODERN   },
        { XML_ROMAN,            FAMILY_ROMAN    },
        { XML_SCRIPT,           FAMILY_SCRIPT   },
        { XML_SWISS,            FAMILY_SWISS    },
        { XML_SYSTEM,           FAMILY_SYSTEM   },
        { XML_TOKEN_INVALID,    0               }
    };
    return aFontFamilyGenericMapping;
}

static SvXMLEnumMapEntry const aFontPitchMapping[] =
{
    { XML_FIXED,            PITCH_FIXED     },
    { XML_VARIABLE,         PITCH_VARIABLE  },
    { XML_TOKEN_INVALID,    0               }
};
///////////////////////////////////////////////////////////////////////////////
//
// class XMLFontFamilyNamePropHdl
//

XMLFontFamilyNamePropHdl::~XMLFontFamilyNamePropHdl()
{
    // Nothing to do
}

sal_Bool XMLFontFamilyNamePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    OUStringBuffer sValue;
    sal_Int32 nPos = 0;

    do
    {
        sal_Int32 nFirst = nPos;
        nPos = ::sax::Converter::indexOfComma( rStrImpValue, nPos );
        sal_Int32 nLast = (-1 == nPos ? rStrImpValue.getLength() - 1 : nPos - 1);

        // skip trailing blanks
        while( nLast > nFirst && sal_Unicode(' ') == rStrImpValue[nLast] )
            nLast--;

        // skip leading blanks
        while(nFirst <= nLast && sal_Unicode(' ') == rStrImpValue[nFirst])
            nFirst++;

        // remove quotes
        sal_Unicode c = rStrImpValue[nFirst];
        if( nFirst < nLast && (sal_Unicode('\'') == c || sal_Unicode('\"') == c) && rStrImpValue[nLast] == c )
        {
            nFirst++;
            nLast--;
        }

        if( nFirst <= nLast )
        {
            if( sValue.getLength() != 0 )
                sValue.append(';');

            sValue.append(rStrImpValue.copy( nFirst, nLast-nFirst+1));
        }

        if( -1 != nPos )
            nPos++;
    }
    while( -1 != nPos );

    if (sValue.getLength())
    {
        rValue <<= sValue.makeStringAndClear();
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool XMLFontFamilyNamePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    OUString aStrFamilyName;

    if( rValue >>= aStrFamilyName )
    {
        OUStringBuffer sValue( aStrFamilyName.getLength() + 2L );
        sal_Int32 nPos = 0;
        do
        {
            sal_Int32 nFirst = nPos;
            nPos = aStrFamilyName.indexOf( sal_Unicode(';'), nPos );
            sal_Int32 nLast = (-1 == nPos ? aStrFamilyName.getLength() : nPos);

            // Set position to the character behind the ';', so we won't
            // forget this.
            if( -1L != nPos )
                nPos++;

            // If the property value was empty, we stop now.
            // If there is a ';' at the first position, the empty name
            // at the start will be removed.
            if( 0L == nLast )
                continue;

            // nFirst and nLast now denote the first and last character of
            // one font name.
            nLast--;

            // skip trailing blanks
            while(  nLast > nFirst && sal_Unicode(' ') == aStrFamilyName[nLast] )
                nLast--;

            // skip leading blanks
            while( nFirst <= nLast && sal_Unicode(' ') == aStrFamilyName[nFirst] )
                nFirst++;

            if( nFirst <= nLast )
            {
                if( sValue.getLength() != 0L )
                {
                    sValue.append( sal_Unicode( ',' ) );
                    sValue.append( sal_Unicode( ' ' ));
                }
                sal_Int32 nLen = nLast-nFirst+1;
                OUString sFamily( aStrFamilyName.copy( nFirst, nLen ) );
                sal_Bool bQuote = sal_False;
                for( sal_Int32 i=0; i < nLen; i++ )
                {
                    sal_Unicode c = sFamily[i];
                    if( sal_Unicode(' ') == c || sal_Unicode(',') == c )
                    {
                        bQuote = sal_True;
                        break;
                    }
                }
                if( bQuote )
                    sValue.append( sal_Unicode('\'') );
                sValue.append( sFamily );
                if( bQuote )
                    sValue.append( sal_Unicode('\'') );
            }
        }
        while( -1L != nPos );

        rStrExpValue = sValue.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLFontFamilyPropHdl
//

XMLFontFamilyPropHdl::~XMLFontFamilyPropHdl()
{
    // Nothing to do
}

sal_Bool XMLFontFamilyPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 eNewFamily;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( eNewFamily, rStrImpValue, lcl_getFontFamilyGenericMapping() );
    if( bRet )
        rValue <<= (sal_Int16)eNewFamily;

    return bRet;
}

sal_Bool XMLFontFamilyPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    OUStringBuffer aOut;

    sal_Int16 nFamily = sal_Int16();
    if( rValue >>= nFamily )
    {
        FontFamily eFamily = (FontFamily)nFamily;
        if( eFamily != FAMILY_DONTKNOW )
            bRet = SvXMLUnitConverter::convertEnum( aOut, eFamily, lcl_getFontFamilyGenericMapping() );
    }

    rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLFontEncodingPropHdl
//

XMLFontEncodingPropHdl::~XMLFontEncodingPropHdl()
{
    // Nothing to do
}

sal_Bool XMLFontEncodingPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_True;

    if( IsXMLToken( rStrImpValue, XML_X_SYMBOL ) )
        rValue <<= (sal_Int16) RTL_TEXTENCODING_SYMBOL;

    return bRet;
}

sal_Bool XMLFontEncodingPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    OUStringBuffer aOut;
    sal_Int16 nSet = sal_Int16();

    if( rValue >>= nSet )
    {
        if( (rtl_TextEncoding)nSet == RTL_TEXTENCODING_SYMBOL )
        {
            aOut.append( GetXMLToken(XML_X_SYMBOL) );
            rStrExpValue = aOut.makeStringAndClear();
            bRet = sal_True;
        }
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLFontPitchPropHdl
//

XMLFontPitchPropHdl::~XMLFontPitchPropHdl()
{
    // Nothing to do
}

sal_Bool XMLFontPitchPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 eNewPitch;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( eNewPitch, rStrImpValue, aFontPitchMapping );
    if( bRet )
        rValue <<= (sal_Int16)eNewPitch;

    return bRet;
}

sal_Bool XMLFontPitchPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nPitch = sal_Int16();
    OUStringBuffer aOut;

    FontPitch ePitch = PITCH_DONTKNOW;
    if( rValue >>= nPitch )
        ePitch =  (FontPitch)nPitch;

    if( PITCH_DONTKNOW != ePitch )
    {
        bRet = SvXMLUnitConverter::convertEnum( aOut, ePitch, aFontPitchMapping, XML_FIXED );
        rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
