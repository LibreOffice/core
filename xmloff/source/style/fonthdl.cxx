/*************************************************************************
 *
 *  $RCSfile: fonthdl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-12 11:20:43 $
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

#ifndef _XMLOFF_PROPERTYHANDLER_FONTTYPES_HXX
#include <fonthdl.hxx>
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

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlelement.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

static SvXMLEnumMapEntry __READONLY_DATA aFontFamilyGenericMapping[] =
{
    { sXML_decorative,  FAMILY_DECORATIVE },

    { sXML_modern,      FAMILY_MODERN   },
    { sXML_roman,       FAMILY_ROMAN    },
    { sXML_script,      FAMILY_SCRIPT   },
    { sXML_swiss,       FAMILY_SWISS    },
    { sXML_system,      FAMILY_SYSTEM   },
    { 0,                0               }
};

static SvXMLEnumMapEntry __READONLY_DATA aFontPitchMapping[] =
{
    { sXML_fixed,       PITCH_FIXED     },
    { sXML_variable,    PITCH_VARIABLE  },
    { 0,                0               }
};
///////////////////////////////////////////////////////////////////////////////
//
// class XMLFontFamilyNamePropHdl
//

XMLFontFamilyNamePropHdl::~XMLFontFamilyNamePropHdl()
{
    // Nothing to do
}

sal_Bool XMLFontFamilyNamePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
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

sal_Bool XMLFontFamilyNamePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
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

sal_Bool XMLFontFamilyPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 eNewFamily;

    if( ( bRet = rUnitConverter.convertEnum( eNewFamily, rStrImpValue, aFontFamilyGenericMapping ) ) )
        rValue <<= eNewFamily;

    return bRet;
}

sal_Bool XMLFontFamilyPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    OUStringBuffer aOut;

    sal_Int16 nFamily;
    if( rValue >>= nFamily )
    {
        FontFamily eFamily = (FontFamily)nFamily;
        if( eFamily != FAMILY_DONTKNOW )
            bRet = rUnitConverter.convertEnum( aOut, eFamily, aFontFamilyGenericMapping );
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

sal_Bool XMLFontEncodingPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_True;

    if( rStrImpValue.compareToAscii( sXML_x_symbol ) )
        rValue <<= (sal_Int16) RTL_TEXTENCODING_SYMBOL;

    return bRet;
}

sal_Bool XMLFontEncodingPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    OUStringBuffer aOut;
    sal_Int16 nSet;

    if( rValue >>= nSet )
    {
        if( (rtl_TextEncoding)nSet == RTL_TEXTENCODING_SYMBOL )
        {
            aOut.appendAscii( sXML_x_symbol );
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

sal_Bool XMLFontPitchPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    USHORT eNewPitch;

    if( ( bRet = rUnitConverter.convertEnum( eNewPitch, rStrImpValue, aFontPitchMapping ) ) )
        rValue <<= (sal_Int16)eNewPitch;

    return bRet;
}

sal_Bool XMLFontPitchPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nPitch;
    OUStringBuffer aOut;

    FontPitch ePitch;
    if( rValue >>= nPitch )
        ePitch =  (FontPitch)nPitch;

    if( PITCH_DONTKNOW != ePitch )
    {
        bRet = rUnitConverter.convertEnum( aOut, ePitch, aFontPitchMapping, sXML_fixed );
        rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}
