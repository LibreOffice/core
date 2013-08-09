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

#include <weighhdl.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmltoken.hxx>
#include <tools/fontenum.hxx>
#include <tools/solar.h>

#include <limits.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/awt/FontWeight.hpp>

using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

struct FontWeightMapper
{
    float fWeight;
    sal_uInt16 nValue;
};

FontWeightMapper const aFontWeightMap[] =
{
    { ::com::sun::star::awt::FontWeight::DONTKNOW,              0 },
    { ::com::sun::star::awt::FontWeight::THIN,                  100 },
    { ::com::sun::star::awt::FontWeight::ULTRALIGHT,            150 },
    { ::com::sun::star::awt::FontWeight::LIGHT,                 250 },
    { ::com::sun::star::awt::FontWeight::SEMILIGHT,             350 },
    { ::com::sun::star::awt::FontWeight::NORMAL,                400 },
    { ::com::sun::star::awt::FontWeight::NORMAL,                450 },
    { ::com::sun::star::awt::FontWeight::SEMIBOLD,              600 },
    { ::com::sun::star::awt::FontWeight::BOLD,                  700 },
    { ::com::sun::star::awt::FontWeight::ULTRABOLD,             800 },
    { ::com::sun::star::awt::FontWeight::BLACK,                 900 },
    { ::com::sun::star::awt::FontWeight::DONTKNOW,             1000 }
};

// class XMLFmtBreakBeforePropHdl

XMLFontWeightPropHdl::~XMLFontWeightPropHdl()
{
    // Nothing to do
}

sal_Bool XMLFontWeightPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nWeight = 0;

    if( IsXMLToken( rStrImpValue, XML_WEIGHT_NORMAL ) )
    {
        nWeight = 400;
        bRet = sal_True;
    }
    else if( IsXMLToken( rStrImpValue, XML_WEIGHT_BOLD ) )
    {
        nWeight = 700;
        bRet = sal_True;
    }
    else
    {
        sal_Int32 nTemp;
        bRet = ::sax::Converter::convertNumber(nTemp, rStrImpValue, 100, 900);
        if( bRet )
            nWeight = sal::static_int_cast< sal_uInt16 >(nTemp);
    }

    if( bRet )
    {
        bRet = sal_False;
        static int nCount = sizeof(aFontWeightMap)/sizeof(FontWeightMapper);
        for (int i = 0; i < (nCount-1); ++i)
        {
            if( (nWeight >= aFontWeightMap[i].nValue) && (nWeight <= aFontWeightMap[i+1].nValue) )
            {
                sal_uInt16 nDiff1 = nWeight - aFontWeightMap[i].nValue;
                sal_uInt16 nDiff2 = aFontWeightMap[i+1].nValue - nWeight;

                if( nDiff1 < nDiff2 )
                    rValue <<= aFontWeightMap[i].fWeight;
                else
                    rValue <<= aFontWeightMap[i+1].fWeight;

                bRet = sal_True;
                break;
            }
        }
    }

    return bRet;
}

sal_Bool XMLFontWeightPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    float fValue = float();
    if( !( rValue >>= fValue ) )
    {
        sal_Int32 nValue = 0;
        if( rValue >>= nValue )
        {
            fValue = (float)nValue;
            bRet = sal_True;
        }
    }
    else
        bRet = sal_True;

    if( bRet )
    {
        sal_uInt16 nWeight = 0;
        static int nCount = sizeof(aFontWeightMap)/sizeof(FontWeightMapper);
        for( int i=0; i<nCount; i++ )
        {
            if( fValue <= aFontWeightMap[i].fWeight )
            {
                 nWeight = aFontWeightMap[i].nValue;
                 break;
            }
        }

        OUStringBuffer aOut;

        if( 400 == nWeight )
            aOut.append( GetXMLToken(XML_WEIGHT_NORMAL) );
        else if( 700 == nWeight )
            aOut.append( GetXMLToken(XML_WEIGHT_BOLD) );
        else
            ::sax::Converter::convertNumber( aOut, (sal_Int32)nWeight );

        rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
