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

#include <undlihdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/awt/FontUnderline.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::xmloff::token;

SvXMLEnumMapEntry const pXML_UnderlineType_Enum[] =
{
    { XML_NONE,                         awt::FontUnderline::NONE },
    { XML_SINGLE,               awt::FontUnderline::SINGLE },
    { XML_DOUBLE,                       awt::FontUnderline::DOUBLE },
    { XML_SINGLE,               awt::FontUnderline::DOTTED },
    { XML_SINGLE,               awt::FontUnderline::DASH },
    { XML_SINGLE,               awt::FontUnderline::LONGDASH },
    { XML_SINGLE,               awt::FontUnderline::DASHDOT },
    { XML_SINGLE,               awt::FontUnderline::DASHDOTDOT },
    { XML_SINGLE,               awt::FontUnderline::WAVE },
    { XML_SINGLE,               awt::FontUnderline::BOLD },
    { XML_SINGLE,               awt::FontUnderline::BOLDDOTTED },
    { XML_SINGLE,               awt::FontUnderline::BOLDDASH },
    { XML_SINGLE,               awt::FontUnderline::BOLDLONGDASH },
    { XML_SINGLE,               awt::FontUnderline::BOLDDASHDOT },
    { XML_SINGLE,           awt::FontUnderline::BOLDDASHDOTDOT },
    { XML_SINGLE,               awt::FontUnderline::BOLDWAVE },
    { XML_DOUBLE,                       awt::FontUnderline::DOUBLEWAVE },
    { XML_SINGLE,               awt::FontUnderline::SMALLWAVE },
    { XML_TOKEN_INVALID,                0 }
};

SvXMLEnumMapEntry const pXML_UnderlineStyle_Enum[] =
{
    { XML_NONE,                         awt::FontUnderline::NONE },
    { XML_SOLID,                        awt::FontUnderline::SINGLE },
    { XML_SOLID,                        awt::FontUnderline::DOUBLE },
    { XML_DOTTED,               awt::FontUnderline::DOTTED },
    { XML_DASH,             awt::FontUnderline::DASH },
    { XML_LONG_DASH,            awt::FontUnderline::LONGDASH },
    { XML_DOT_DASH,         awt::FontUnderline::DASHDOT },
    { XML_DOT_DOT_DASH,     awt::FontUnderline::DASHDOTDOT },
    { XML_WAVE,             awt::FontUnderline::WAVE },
    { XML_SOLID,                        awt::FontUnderline::BOLD },
    { XML_DOTTED,               awt::FontUnderline::BOLDDOTTED },
    { XML_DASH,             awt::FontUnderline::BOLDDASH },
    { XML_LONG_DASH,            awt::FontUnderline::BOLDLONGDASH },
    { XML_DOT_DASH,         awt::FontUnderline::BOLDDASHDOT },
    { XML_DOT_DOT_DASH,         awt::FontUnderline::BOLDDASHDOTDOT },
    { XML_WAVE,             awt::FontUnderline::BOLDWAVE },
    { XML_WAVE,                 awt::FontUnderline::DOUBLEWAVE },
    { XML_SMALL_WAVE,           awt::FontUnderline::SMALLWAVE },
    { XML_TOKEN_INVALID,                0 }
};

SvXMLEnumMapEntry const pXML_UnderlineWidth_Enum[] =
{
    { XML_AUTO,                         awt::FontUnderline::NONE },
    { XML_AUTO,                         awt::FontUnderline::SINGLE },
    { XML_AUTO,                         awt::FontUnderline::DOUBLE },
    { XML_AUTO,                         awt::FontUnderline::DOTTED },
    { XML_AUTO,                         awt::FontUnderline::DASH },
    { XML_AUTO,                         awt::FontUnderline::LONGDASH },
    { XML_AUTO,                         awt::FontUnderline::DASHDOT },
    { XML_AUTO,                         awt::FontUnderline::DASHDOTDOT },
    { XML_AUTO,                         awt::FontUnderline::WAVE },
    { XML_BOLD,             awt::FontUnderline::BOLD },
    { XML_BOLD,             awt::FontUnderline::BOLDDOTTED },
    { XML_BOLD,             awt::FontUnderline::BOLDDASH },
    { XML_BOLD,             awt::FontUnderline::BOLDLONGDASH },
    { XML_BOLD,             awt::FontUnderline::BOLDDASHDOT },
    { XML_BOLD,                 awt::FontUnderline::BOLDDASHDOTDOT },
    { XML_BOLD,             awt::FontUnderline::BOLDWAVE },
    { XML_AUTO,                         awt::FontUnderline::DOUBLEWAVE },
    { XML_THIN,                         awt::FontUnderline::NONE },
    { XML_MEDIUM,                       awt::FontUnderline::NONE },
    { XML_THICK,                        awt::FontUnderline::BOLD},
    { XML_TOKEN_INVALID,                0 }
};

//
// class XMLUnderlineTypePropHdl
//

XMLUnderlineTypePropHdl::~XMLUnderlineTypePropHdl()
{
    // nothing to do
}

bool XMLUnderlineTypePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 eNewUnderline;
    bool bRet = SvXMLUnitConverter::convertEnum(
        eNewUnderline, rStrImpValue, pXML_UnderlineType_Enum );
    if( bRet )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eUnderline = sal_Int16();
        if( (rValue >>= eUnderline) && awt::FontUnderline::NONE!=eUnderline )
        {
            switch( eNewUnderline )
            {
            case awt::FontUnderline::NONE:
            case awt::FontUnderline::SINGLE:
                // keep existing line style
                eNewUnderline = eUnderline;
                break;
            case awt::FontUnderline::DOUBLE:
                // A double line style has priority over a bold line style,
                // but not over the line style itself.
                switch( eUnderline )
                {
                case awt::FontUnderline::SINGLE:
                case awt::FontUnderline::BOLD:
                    break;
                case awt::FontUnderline::WAVE:
                case awt::FontUnderline::BOLDWAVE:
                    eNewUnderline = awt::FontUnderline::DOUBLEWAVE;
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

bool XMLUnderlineTypePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_Int16 nValue = sal_Int16();
    OUStringBuffer aOut;

    if( (rValue >>= nValue) &&
        (awt::FontUnderline::DOUBLE == nValue ||
         awt::FontUnderline::DOUBLEWAVE == nValue) )
    {
        bRet = SvXMLUnitConverter::convertEnum(
            aOut, (sal_uInt16)nValue, pXML_UnderlineType_Enum );
        if( bRet )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

//
// class XMLUnderlineStylePropHdl
//

XMLUnderlineStylePropHdl::~XMLUnderlineStylePropHdl()
{
    // nothing to do
}

bool XMLUnderlineStylePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 eNewUnderline;
    bool bRet = SvXMLUnitConverter::convertEnum(
        eNewUnderline, rStrImpValue, pXML_UnderlineStyle_Enum );
    if( bRet )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eUnderline = sal_Int16();
        if( (rValue >>= eUnderline) && awt::FontUnderline::NONE!=eUnderline )
        {
            switch( eNewUnderline )
            {
            case awt::FontUnderline::NONE:
            case awt::FontUnderline::SINGLE:
                // keep double or bold line style
                eNewUnderline = eUnderline;
            case awt::FontUnderline::DOTTED:
                // The line style has priority over a double type.
                if( awt::FontUnderline::BOLD == eUnderline )
                    eNewUnderline = awt::FontUnderline::BOLDDOTTED;
                break;
            case awt::FontUnderline::DASH:
                if( awt::FontUnderline::BOLD == eUnderline )
                    eNewUnderline = awt::FontUnderline::BOLDDASH;
                break;
            case awt::FontUnderline::LONGDASH:
                if( awt::FontUnderline::BOLD == eUnderline )
                    eNewUnderline = awt::FontUnderline::BOLDLONGDASH;
                break;
            case awt::FontUnderline::DASHDOT:
                if( awt::FontUnderline::BOLD == eUnderline )
                    eNewUnderline = awt::FontUnderline::BOLDDASHDOT;
                break;
            case awt::FontUnderline::DASHDOTDOT:
                if( awt::FontUnderline::BOLD == eUnderline )
                    eNewUnderline = awt::FontUnderline::BOLDDASHDOTDOT;
                break;
            case awt::FontUnderline::WAVE:
                if( awt::FontUnderline::BOLD == eUnderline )
                    eNewUnderline = awt::FontUnderline::BOLDWAVE;
                break;
            case awt::FontUnderline::SMALLWAVE:
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

bool XMLUnderlineStylePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
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

//
// class XMLUnderlineWidthPropHdl
//

XMLUnderlineWidthPropHdl::~XMLUnderlineWidthPropHdl()
{
    // nothing to do
}

bool XMLUnderlineWidthPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 eNewUnderline;
    bool bRet = SvXMLUnitConverter::convertEnum(
        eNewUnderline, rStrImpValue, pXML_UnderlineWidth_Enum );
    if( bRet )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eUnderline = sal_Int16();
        if( (rValue >>= eUnderline) && awt::FontUnderline::NONE!=eUnderline )
        {
            switch( eNewUnderline )
            {
            case awt::FontUnderline::NONE:
                // keep existing line style
                eNewUnderline = eUnderline;
                break;
            case awt::FontUnderline::BOLD:
                // A double line style has priority over a bold line style,
                // but not over the line style itself.
                switch( eUnderline )
                {
                case awt::FontUnderline::SINGLE:
                    break;
                case awt::FontUnderline::DOTTED:
                    eNewUnderline = awt::FontUnderline::BOLDDOTTED;
                    break;
                case awt::FontUnderline::DASH:
                    eNewUnderline = awt::FontUnderline::BOLDDASH;
                    break;
                case awt::FontUnderline::LONGDASH:
                    eNewUnderline = awt::FontUnderline::BOLDLONGDASH;
                    break;
                case awt::FontUnderline::DASHDOT:
                    eNewUnderline = awt::FontUnderline::BOLDDASHDOT;
                    break;
                case awt::FontUnderline::DASHDOTDOT:
                    eNewUnderline = awt::FontUnderline::BOLDDASHDOTDOT;
                    break;
                case awt::FontUnderline::WAVE:
                    eNewUnderline = awt::FontUnderline::BOLDWAVE;
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

bool XMLUnderlineWidthPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_Int16 nValue = sal_Int16();
    OUStringBuffer aOut;

    if( (rValue >>= nValue) && (awt::FontUnderline::NONE != nValue) )
    {
        bRet = SvXMLUnitConverter::convertEnum(
            aOut, (sal_uInt16)nValue, pXML_UnderlineWidth_Enum );
        if( bRet )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
