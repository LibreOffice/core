/*************************************************************************
 *
 *  $RCSfile: undlihdl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:25:29 $
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

#ifndef _XMLOFF_PROPERTYHANDLER_UNDERLINETYPES_HXX
#include <undlihdl.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
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
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP
#include <com/sun/star/awt/FontUnderline.hpp>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlelement.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::xmloff::token;

SvXMLEnumMapEntry __READONLY_DATA pXML_UnderlineType_Enum[] =
{
    { XML_NONE,                         FontUnderline::NONE },
    { XML_SINGLE,               FontUnderline::SINGLE },
    { XML_DOUBLE,                       FontUnderline::DOUBLE },
    { XML_SINGLE,               FontUnderline::DOTTED },
    { XML_SINGLE,               FontUnderline::DASH },
    { XML_SINGLE,               FontUnderline::LONGDASH },
    { XML_SINGLE,               FontUnderline::DASHDOT },
    { XML_SINGLE,               FontUnderline::DASHDOTDOT },
    { XML_SINGLE,               FontUnderline::WAVE },
    { XML_SINGLE,               FontUnderline::BOLD },
    { XML_SINGLE,               FontUnderline::BOLDDOTTED },
    { XML_SINGLE,               FontUnderline::BOLDDASH },
    { XML_SINGLE,               FontUnderline::BOLDLONGDASH },
    { XML_SINGLE,               FontUnderline::BOLDDASHDOT },
    { XML_SINGLE,           FontUnderline::BOLDDASHDOTDOT },
    { XML_SINGLE,               FontUnderline::BOLDWAVE },
    { XML_DOUBLE,                       FontUnderline::DOUBLEWAVE },
    { XML_SINGLE,               FontUnderline::SMALLWAVE },
    { XML_TOKEN_INVALID,                0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_UnderlineStyle_Enum[] =
{
    { XML_NONE,                         FontUnderline::NONE },
    { XML_SOLID,                        FontUnderline::SINGLE },
    { XML_SOLID,                        FontUnderline::DOUBLE },
    { XML_DOTTED,               FontUnderline::DOTTED },
    { XML_DASH,             FontUnderline::DASH },
    { XML_LONG_DASH,            FontUnderline::LONGDASH },
    { XML_DOT_DASH,         FontUnderline::DASHDOT },
    { XML_DOT_DOT_DASH,     FontUnderline::DASHDOTDOT },
    { XML_WAVE,             FontUnderline::WAVE },
    { XML_SOLID,                        FontUnderline::BOLD },
    { XML_DOTTED,               FontUnderline::BOLDDOTTED },
    { XML_DASH,             FontUnderline::BOLDDASH },
    { XML_LONG_DASH,            FontUnderline::BOLDLONGDASH },
    { XML_DOT_DASH,         FontUnderline::BOLDDASHDOT },
    { XML_DOT_DOT_DASH,         FontUnderline::BOLDDASHDOTDOT },
    { XML_WAVE,             FontUnderline::BOLDWAVE },
    { XML_WAVE,                 FontUnderline::DOUBLEWAVE },
    { XML_SMALL_WAVE,           FontUnderline::SMALLWAVE },
    { XML_TOKEN_INVALID,                0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_UnderlineWidth_Enum[] =
{
    { XML_AUTO,                         FontUnderline::NONE },
    { XML_AUTO,                         FontUnderline::SINGLE },
    { XML_AUTO,                         FontUnderline::DOUBLE },
    { XML_AUTO,                         FontUnderline::DOTTED },
    { XML_AUTO,                         FontUnderline::DASH },
    { XML_AUTO,                         FontUnderline::LONGDASH },
    { XML_AUTO,                         FontUnderline::DASHDOT },
    { XML_AUTO,                         FontUnderline::DASHDOTDOT },
    { XML_AUTO,                         FontUnderline::WAVE },
    { XML_BOLD,             FontUnderline::BOLD },
    { XML_BOLD,             FontUnderline::BOLDDOTTED },
    { XML_BOLD,             FontUnderline::BOLDDASH },
    { XML_BOLD,             FontUnderline::BOLDLONGDASH },
    { XML_BOLD,             FontUnderline::BOLDDASHDOT },
    { XML_BOLD,                 FontUnderline::BOLDDASHDOTDOT },
    { XML_BOLD,             FontUnderline::BOLDWAVE },
    { XML_AUTO,                         FontUnderline::DOUBLEWAVE },
    { XML_THIN,                         FontUnderline::NONE },
    { XML_MEDIUM,                       FontUnderline::NONE },
    { XML_THICK,                        FontUnderline::BOLD},
    { XML_TOKEN_INVALID,                0 }
};



///////////////////////////////////////////////////////////////////////////////
//
// class XMLUnderlineTypePropHdl
//

XMLUnderlineTypePropHdl::~XMLUnderlineTypePropHdl()
{
    // nothing to do
}

sal_Bool XMLUnderlineTypePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 eNewUnderline;

    if( ( bRet = rUnitConverter.convertEnum( eNewUnderline, rStrImpValue,
                                             pXML_UnderlineType_Enum ) ) )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eUnderline;
        if( (rValue >>= eUnderline) && FontUnderline::NONE!=eUnderline )
        {
            switch( eNewUnderline )
            {
            case FontUnderline::NONE:
            case FontUnderline::SINGLE:
                // keep existing line style
                eNewUnderline = eUnderline;
                break;
            case FontUnderline::DOUBLE:
                // A double line style has priority over a bold line style,
                // but not over the line style itself.
                switch( eUnderline )
                {
                case FontUnderline::SINGLE:
                case FontUnderline::BOLD:
                    break;
                case FontUnderline::WAVE:
                case FontUnderline::BOLDWAVE:
                    eNewUnderline = FontUnderline::DOUBLEWAVE;
                    break;
                default:
                    // If a double line style is not supported for the existing
                    // value, keep the new one
                    eNewUnderline = eUnderline;
                    break;
                }
                break;
            default:
                OSL_ENSURE( bRet, "unexpected line type value" );
                break;
            }
            if( eNewUnderline != eUnderline )
                rValue <<= (sal_Int16)eNewUnderline;
        }
        else
        {
            rValue <<= (sal_Int16)eNewUnderline;
        }
    }

    return bRet;
}

sal_Bool XMLUnderlineTypePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue;
    OUStringBuffer aOut;

    if( (rValue >>= nValue) &&
        (FontUnderline::DOUBLE == nValue ||
         FontUnderline::DOUBLEWAVE == nValue) )
    {
        if( ( bRet = rUnitConverter.convertEnum( aOut, (sal_uInt16)nValue,
                                                 pXML_UnderlineType_Enum ) ) )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLUnderlineStylePropHdl
//

XMLUnderlineStylePropHdl::~XMLUnderlineStylePropHdl()
{
    // nothing to do
}

sal_Bool XMLUnderlineStylePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 eNewUnderline;

    if( ( bRet = rUnitConverter.convertEnum( eNewUnderline, rStrImpValue,
                                             pXML_UnderlineStyle_Enum ) ) )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eUnderline;
        if( (rValue >>= eUnderline) && FontUnderline::NONE!=eUnderline )
        {
            switch( eNewUnderline )
            {
            case FontUnderline::NONE:
            case FontUnderline::SINGLE:
                // keep double or bold line style
                eNewUnderline = eUnderline;
            case FontUnderline::DOTTED:
                // The line style has priority over a double type.
                if( FontUnderline::BOLD == eUnderline )
                    eNewUnderline = FontUnderline::BOLDDOTTED;
                break;
            case FontUnderline::DASH:
                if( FontUnderline::BOLD == eUnderline )
                    eNewUnderline = FontUnderline::BOLDDASH;
                break;
            case FontUnderline::LONGDASH:
                if( FontUnderline::BOLD == eUnderline )
                    eNewUnderline = FontUnderline::BOLDLONGDASH;
                break;
            case FontUnderline::DASHDOT:
                if( FontUnderline::BOLD == eUnderline )
                    eNewUnderline = FontUnderline::BOLDDASHDOT;
                break;
            case FontUnderline::DASHDOTDOT:
                if( FontUnderline::BOLD == eUnderline )
                    eNewUnderline = FontUnderline::BOLDDASHDOTDOT;
                break;
            case FontUnderline::WAVE:
                if( FontUnderline::BOLD == eUnderline )
                    eNewUnderline = FontUnderline::BOLDWAVE;
                break;
            case FontUnderline::SMALLWAVE:
                // SMALLWAVE is not used
            default:
                OSL_ENSURE( bRet, "unexpected line style value" );
                break;
            }
            if( eNewUnderline != eUnderline )
                rValue <<= (sal_Int16)eNewUnderline;
        }
        else
        {
            rValue <<= (sal_Int16)eNewUnderline;
        }
    }

    return bRet;
}

sal_Bool XMLUnderlineStylePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue;
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        if( ( bRet = rUnitConverter.convertEnum( aOut, (sal_uInt16)nValue,
                                                 pXML_UnderlineStyle_Enum ) ) )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLUnderlineWidthPropHdl
//

XMLUnderlineWidthPropHdl::~XMLUnderlineWidthPropHdl()
{
    // nothing to do
}

sal_Bool XMLUnderlineWidthPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 eNewUnderline;

    if( ( bRet = rUnitConverter.convertEnum( eNewUnderline, rStrImpValue,
                                             pXML_UnderlineWidth_Enum ) ) )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eUnderline;
        if( (rValue >>= eUnderline) && FontUnderline::NONE!=eUnderline )
        {
            switch( eNewUnderline )
            {
            case FontUnderline::NONE:
                // keep existing line style
                eNewUnderline = eUnderline;
                break;
            case FontUnderline::BOLD:
                // A double line style has priority over a bold line style,
                // but not over the line style itself.
                switch( eUnderline )
                {
                case FontUnderline::SINGLE:
                    break;
                case FontUnderline::DOTTED:
                    eNewUnderline = FontUnderline::BOLDDOTTED;
                    break;
                case FontUnderline::DASH:
                    eNewUnderline = FontUnderline::BOLDDASH;
                    break;
                case FontUnderline::LONGDASH:
                    eNewUnderline = FontUnderline::BOLDLONGDASH;
                    break;
                case FontUnderline::DASHDOT:
                    eNewUnderline = FontUnderline::BOLDDASHDOT;
                    break;
                case FontUnderline::DASHDOTDOT:
                    eNewUnderline = FontUnderline::BOLDDASHDOTDOT;
                    break;
                case FontUnderline::WAVE:
                    eNewUnderline = FontUnderline::BOLDWAVE;
                    break;
                default:
                    // a doube line style overwrites a bold one
                    eNewUnderline = eUnderline;
                    break;
                }
                break;
            default:
                OSL_ENSURE( bRet, "unexpected line width value" );
                break;
            }
            if( eNewUnderline != eUnderline )
                rValue <<= (sal_Int16)eNewUnderline;
        }
        else
        {
            rValue <<= (sal_Int16)eNewUnderline;
        }
    }

    return bRet;
}

sal_Bool XMLUnderlineWidthPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue;
    OUStringBuffer aOut;

    if( (rValue >>= nValue) && (FontUnderline::NONE != nValue) )
    {
        if( ( bRet = rUnitConverter.convertEnum( aOut, (sal_uInt16)nValue,
                                                 pXML_UnderlineWidth_Enum ) ) )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

