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
#include <cdouthdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/uno/Any.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::xmloff::token;

SvXMLEnumMapEntry pXML_CrossedoutType_Enum[] =
{
    { XML_NONE,                 FontStrikeout::NONE },
    { XML_SINGLE,   FontStrikeout::SINGLE },
    { XML_DOUBLE,               FontStrikeout::DOUBLE },
    { XML_SINGLE,    FontStrikeout::BOLD },
    { XML_SINGLE,    FontStrikeout::SLASH },
    { XML_SINGLE,    FontStrikeout::X },
    { XML_TOKEN_INVALID,                0 }
};

SvXMLEnumMapEntry pXML_CrossedoutStyle_Enum[] =
{
    { XML_NONE,                         FontStrikeout::NONE },
    { XML_SOLID,                        FontStrikeout::SINGLE },
    { XML_SOLID,                        FontStrikeout::DOUBLE },
    { XML_SOLID,                        FontStrikeout::BOLD },
    { XML_SOLID,                        FontStrikeout::SLASH },
    { XML_SOLID,                        FontStrikeout::X },
    { XML_DOTTED,               FontStrikeout::SINGLE },
    { XML_DASH,             FontStrikeout::SINGLE },
    { XML_LONG_DASH,            FontStrikeout::SINGLE },
    { XML_DOT_DASH,         FontStrikeout::SINGLE },
    { XML_DOT_DOT_DASH,     FontStrikeout::SINGLE },
    { XML_WAVE,             FontStrikeout::SINGLE },
    { XML_TOKEN_INVALID,                0 }
};

SvXMLEnumMapEntry pXML_CrossedoutWidth_Enum[] =
{
    { XML_AUTO,                 FontStrikeout::NONE },
    { XML_AUTO,                 FontStrikeout::SINGLE },
    { XML_AUTO,                 FontStrikeout::DOUBLE },
    { XML_BOLD,     FontStrikeout::BOLD },
    { XML_AUTO,                 FontStrikeout::SLASH },
    { XML_AUTO,                 FontStrikeout::X },
    { XML_THIN,                 FontStrikeout::NONE },
    { XML_MEDIUM,               FontStrikeout::NONE },
    { XML_THICK,                FontStrikeout::NONE },
    { XML_TOKEN_INVALID,                0 }
};

///////////////////////////////////////////////////////////////////////////////
//
// class XMLCrossedOutTypePropHdl
//

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
        if( (rValue >>= eStrikeout) && FontStrikeout::NONE!=eStrikeout )
        {
            switch( eNewStrikeout )
            {
            case FontStrikeout::NONE:
            case FontStrikeout::SINGLE:
                // keep existing line style
                eNewStrikeout = eStrikeout;
                break;
            case FontStrikeout::DOUBLE:
                // A double line style has priority over a solid or a bold
                // line style,
                // but not about any other line style
                switch( eStrikeout )
                {
                case FontStrikeout::SINGLE:
                case FontStrikeout::BOLD:
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

    if( (rValue >>= nValue) && FontStrikeout::DOUBLE==nValue )
    {
        bRet = SvXMLUnitConverter::convertEnum(
            aOut, (sal_uInt16)nValue, pXML_CrossedoutType_Enum );
        if( bRet )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLCrossedOutStylePropHdl
//

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
        if( (rValue >>= eStrikeout) && FontStrikeout::NONE!=eStrikeout )
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

///////////////////////////////////////////////////////////////////////////////
//
// class XMLCrossedOutWidthPropHdl
//

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
        if( (rValue >>= eStrikeout) && FontStrikeout::NONE!=eStrikeout )
        {
            switch( eNewStrikeout )
            {
            case FontStrikeout::NONE:
                // keep existing line style
                eNewStrikeout = eStrikeout;
                break;
            case FontStrikeout::BOLD:
                switch( eStrikeout )
                {
                case FontStrikeout::SINGLE:
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

    if( (rValue >>= nValue) && (FontStrikeout::BOLD == nValue) )
    {
        bRet = SvXMLUnitConverter::convertEnum(
            aOut, (sal_uInt16)nValue, pXML_CrossedoutWidth_Enum );
        if( bRet )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLCrossedOutTextPropHdl
//

XMLCrossedOutTextPropHdl::~XMLCrossedOutTextPropHdl()
{
    // nothing to do
}

sal_Bool XMLCrossedOutTextPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    if( rStrImpValue.getLength() )
    {
        sal_Int16 eStrikeout = ('/' == rStrImpValue[0]
                                        ? FontStrikeout::SLASH
                                        : FontStrikeout::X);
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
        (FontStrikeout::SLASH == nValue || FontStrikeout::X == nValue) )
    {
        rStrExpValue = OUString::valueOf(
            static_cast< sal_Unicode>( FontStrikeout::SLASH == nValue ? '/'
                                                                      : 'X' ) );
        bRet = sal_True;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
