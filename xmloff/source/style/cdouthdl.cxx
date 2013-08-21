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

#include <cdouthdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/uno/Any.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::xmloff::token;

SvXMLEnumMapEntry pXML_CrossedoutType_Enum[] =
{
    { XML_NONE,                 awt::FontStrikeout::NONE },
    { XML_SINGLE,   awt::FontStrikeout::SINGLE },
    { XML_DOUBLE,               awt::FontStrikeout::DOUBLE },
    { XML_SINGLE,    awt::FontStrikeout::BOLD },
    { XML_SINGLE,    awt::FontStrikeout::SLASH },
    { XML_SINGLE,    awt::FontStrikeout::X },
    { XML_TOKEN_INVALID,                0 }
};

SvXMLEnumMapEntry pXML_CrossedoutStyle_Enum[] =
{
    { XML_NONE,                         awt::FontStrikeout::NONE },
    { XML_SOLID,                        awt::FontStrikeout::SINGLE },
    { XML_SOLID,                        awt::FontStrikeout::DOUBLE },
    { XML_SOLID,                        awt::FontStrikeout::BOLD },
    { XML_SOLID,                        awt::FontStrikeout::SLASH },
    { XML_SOLID,                        awt::FontStrikeout::X },
    { XML_DOTTED,               awt::FontStrikeout::SINGLE },
    { XML_DASH,             awt::FontStrikeout::SINGLE },
    { XML_LONG_DASH,            awt::FontStrikeout::SINGLE },
    { XML_DOT_DASH,         awt::FontStrikeout::SINGLE },
    { XML_DOT_DOT_DASH,     awt::FontStrikeout::SINGLE },
    { XML_WAVE,             awt::FontStrikeout::SINGLE },
    { XML_TOKEN_INVALID,                0 }
};

SvXMLEnumMapEntry pXML_CrossedoutWidth_Enum[] =
{
    { XML_AUTO,                 awt::FontStrikeout::NONE },
    { XML_AUTO,                 awt::FontStrikeout::SINGLE },
    { XML_AUTO,                 awt::FontStrikeout::DOUBLE },
    { XML_BOLD,     awt::FontStrikeout::BOLD },
    { XML_AUTO,                 awt::FontStrikeout::SLASH },
    { XML_AUTO,                 awt::FontStrikeout::X },
    { XML_THIN,                 awt::FontStrikeout::NONE },
    { XML_MEDIUM,               awt::FontStrikeout::NONE },
    { XML_THICK,                awt::FontStrikeout::NONE },
    { XML_TOKEN_INVALID,                0 }
};

// class XMLCrossedOutTypePropHdl

XMLCrossedOutTypePropHdl::~XMLCrossedOutTypePropHdl()
{
    // nothing to do
}

sal_Bool XMLCrossedOutTypePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 eNewStrikeout;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum(
        eNewStrikeout, rStrImpValue, pXML_CrossedoutType_Enum );
    if( bRet )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eStrikeout = sal_Int16();
        if( (rValue >>= eStrikeout) && awt::FontStrikeout::NONE!=eStrikeout )
        {
            switch( eNewStrikeout )
            {
            case awt::FontStrikeout::NONE:
            case awt::FontStrikeout::SINGLE:
                // keep existing line style
                eNewStrikeout = eStrikeout;
                break;
            case awt::FontStrikeout::DOUBLE:
                // A double line style has priority over a solid or a bold
                // line style,
                // but not about any other line style
                switch( eStrikeout )
                {
                case awt::FontStrikeout::SINGLE:
                case awt::FontStrikeout::BOLD:
                    break;
                default:
                    // If a double line style is not supported for the existing
                    // value, keep the new one
                    eNewStrikeout = eStrikeout;
                    break;
                }
                break;
            default:
                OSL_ENSURE( bRet, "unexpected line type value" );
                break;
            }
            if( eNewStrikeout != eStrikeout )
                rValue <<= (sal_Int16)eNewStrikeout;
        }
        else
        {
            rValue <<= (sal_Int16)eNewStrikeout;
        }
    }

    return bRet;
}

sal_Bool XMLCrossedOutTypePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue = sal_Int16();
    OUStringBuffer aOut;

    if( (rValue >>= nValue) && awt::FontStrikeout::DOUBLE==nValue )
    {
        bRet = SvXMLUnitConverter::convertEnum(
            aOut, (sal_uInt16)nValue, pXML_CrossedoutType_Enum );
        if( bRet )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

// class XMLCrossedOutStylePropHdl

XMLCrossedOutStylePropHdl::~XMLCrossedOutStylePropHdl()
{
    // nothing to do
}

sal_Bool XMLCrossedOutStylePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 eNewStrikeout;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum(
        eNewStrikeout, rStrImpValue, pXML_CrossedoutStyle_Enum );
    if( bRet )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eStrikeout = sal_Int16();
        if( (rValue >>= eStrikeout) && awt::FontStrikeout::NONE!=eStrikeout )
        {
            // one NONE a SINGLE are possible new values. For both, the
            // existing value is kept.
        }
        else
        {
            rValue <<= (sal_Int16)eNewStrikeout;
        }
    }

    return bRet;
}

sal_Bool XMLCrossedOutStylePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue = sal_Int16();
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        bRet = SvXMLUnitConverter::convertEnum(
            aOut, (sal_uInt16)nValue, pXML_CrossedoutStyle_Enum );
        if( bRet )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

// class XMLCrossedOutWidthPropHdl

XMLCrossedOutWidthPropHdl::~XMLCrossedOutWidthPropHdl()
{
    // nothing to do
}

sal_Bool XMLCrossedOutWidthPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 eNewStrikeout;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum(
        eNewStrikeout, rStrImpValue, pXML_CrossedoutWidth_Enum );
    if( bRet )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eStrikeout = sal_Int16();
        if( (rValue >>= eStrikeout) && awt::FontStrikeout::NONE!=eStrikeout )
        {
            switch( eNewStrikeout )
            {
            case awt::FontStrikeout::NONE:
                // keep existing line style
                eNewStrikeout = eStrikeout;
                break;
            case awt::FontStrikeout::BOLD:
                switch( eStrikeout )
                {
                case awt::FontStrikeout::SINGLE:
                    break;
                default:
                    // If a double line style is not supported for the existing
                    // value, keep the new one
                    eNewStrikeout = eStrikeout;
                    break;
                }
            default:
                OSL_ENSURE( bRet, "unexpected line type value" );
                break;
            }
            if( eNewStrikeout != eStrikeout )
                rValue <<= (sal_Int16)eNewStrikeout;
        }
        else
        {
            rValue <<= (sal_Int16)eNewStrikeout;
        }
    }

    return bRet;
}

sal_Bool XMLCrossedOutWidthPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue = sal_Int16();
    OUStringBuffer aOut;

    if( (rValue >>= nValue) && (awt::FontStrikeout::BOLD == nValue) )
    {
        bRet = SvXMLUnitConverter::convertEnum(
            aOut, (sal_uInt16)nValue, pXML_CrossedoutWidth_Enum );
        if( bRet )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

// class XMLCrossedOutTextPropHdl

XMLCrossedOutTextPropHdl::~XMLCrossedOutTextPropHdl()
{
    // nothing to do
}

sal_Bool XMLCrossedOutTextPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    if( !rStrImpValue.isEmpty() )
    {
        sal_Int16 eStrikeout = ('/' == rStrImpValue[0]
                                        ? awt::FontStrikeout::SLASH
                                        : awt::FontStrikeout::X);
        rValue <<= (sal_Int16)eStrikeout;
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool XMLCrossedOutTextPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue = sal_Int16();

    if( (rValue >>= nValue) &&
        (awt::FontStrikeout::SLASH == nValue || awt::FontStrikeout::X == nValue) )
    {
        rStrExpValue = OUString(
            static_cast< sal_Unicode>( awt::FontStrikeout::SLASH == nValue ? '/'
                                                                      : 'X' ) );
        bRet = sal_True;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
