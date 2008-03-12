/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fonthdl.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:51:33 $
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

#ifndef _XMLOFF_PROPERTYHANDLER_FONTTYPES_HXX
#include <fonthdl.hxx>
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

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include <xmloff/xmlelement.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

static SvXMLEnumMapEntry __READONLY_DATA aFontFamilyGenericMapping[] =
{
    { XML_DECORATIVE,       FAMILY_DECORATIVE },

    { XML_MODERN,           FAMILY_MODERN   },
    { XML_ROMAN,            FAMILY_ROMAN    },
    { XML_SCRIPT,           FAMILY_SCRIPT   },
    { XML_SWISS,            FAMILY_SWISS    },
    { XML_SYSTEM,           FAMILY_SYSTEM   },
    { XML_TOKEN_INVALID,    0               }
};

static SvXMLEnumMapEntry __READONLY_DATA aFontPitchMapping[] =
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
    String sValue;
    sal_Int32 nPos = 0;

    do
    {
        sal_Int32 nFirst = nPos;
        nPos = SvXMLUnitConverter::indexOfComma( rStrImpValue, nPos );
        sal_Int32 nLast = (-1 == nPos ? rStrImpValue.getLength() : nPos);
        if( nLast > 0 )
            nLast--;

        // skip trailing blanks
        while( sal_Unicode(' ') == rStrImpValue[nLast] && nLast > nFirst )
            nLast--;

        // skip leading blanks
        while( sal_Unicode(' ') == rStrImpValue[nFirst] && nFirst <= nLast )
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
            if( sValue.Len() != 0 )
                sValue += sal_Unicode(';');

            OUString sTemp = rStrImpValue.copy( nFirst, nLast-nFirst+1 );
            sValue += sTemp.getStr();
        }

        if( -1 != nPos )
            nPos++;
    }
    while( -1 != nPos );

    if( sValue.Len() )
    {
        rValue <<= OUString(sValue.GetBuffer());
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
        sal_Int32 nPos = 0L;
        do
        {
            sal_Int32 nFirst = nPos;
            nPos = aStrFamilyName.indexOf( sal_Unicode(';'), nPos );
            sal_Int32 nLast = (-1L == nPos ? aStrFamilyName.getLength() : nPos);

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
            while( sal_Unicode(' ') == aStrFamilyName[nLast] && nLast > nFirst )
                nLast--;

            // skip leading blanks
            while( sal_Unicode(' ') == aStrFamilyName[nFirst] && nFirst <= nLast )
                nFirst++;

            if( nFirst <= nLast )
            {
                if( sValue.getLength() != 0L )
                {
                    sValue.append( sal_Unicode( ',' ) );
                    sValue.append( sal_Unicode( ' ' ));
                }
                sal_Int32 nLen = nLast-nFirst+1L;
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
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( eNewFamily, rStrImpValue, aFontFamilyGenericMapping );
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
            bRet = SvXMLUnitConverter::convertEnum( aOut, eFamily, aFontFamilyGenericMapping );
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
    USHORT eNewPitch;
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
