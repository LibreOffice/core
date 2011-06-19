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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <undlihdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>


#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/awt/FontUnderline.hpp>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::xmloff::token;

SvXMLEnumMapEntry const pXML_UnderlineType_Enum[] =
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

SvXMLEnumMapEntry const pXML_UnderlineStyle_Enum[] =
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

SvXMLEnumMapEntry const pXML_UnderlineWidth_Enum[] =
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

sal_Bool XMLUnderlineTypePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 eNewUnderline;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum(
        eNewUnderline, rStrImpValue, pXML_UnderlineType_Enum );
    if( bRet )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eUnderline = sal_Int16();
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

sal_Bool XMLUnderlineTypePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue = sal_Int16();
    OUStringBuffer aOut;

    if( (rValue >>= nValue) &&
        (FontUnderline::DOUBLE == nValue ||
         FontUnderline::DOUBLEWAVE == nValue) )
    {
        bRet = SvXMLUnitConverter::convertEnum(
            aOut, (sal_uInt16)nValue, pXML_UnderlineType_Enum );
        if( bRet )
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

sal_Bool XMLUnderlineStylePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 eNewUnderline;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum(
        eNewUnderline, rStrImpValue, pXML_UnderlineStyle_Enum );
    if( bRet )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eUnderline = sal_Int16();
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

sal_Bool XMLUnderlineStylePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue = sal_Int16();
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        bRet = SvXMLUnitConverter::convertEnum(
            aOut, (sal_uInt16)nValue, pXML_UnderlineStyle_Enum );
        if( bRet )
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

sal_Bool XMLUnderlineWidthPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 eNewUnderline;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum(
        eNewUnderline, rStrImpValue, pXML_UnderlineWidth_Enum );
    if( bRet )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eUnderline = sal_Int16();
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

sal_Bool XMLUnderlineWidthPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue = sal_Int16();
    OUStringBuffer aOut;

    if( (rValue >>= nValue) && (FontUnderline::NONE != nValue) )
    {
        bRet = SvXMLUnitConverter::convertEnum(
            aOut, (sal_uInt16)nValue, pXML_UnderlineWidth_Enum );
        if( bRet )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
