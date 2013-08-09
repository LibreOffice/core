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

#include <chrhghdl.hxx>

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/uno/Any.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmluconv.hxx>

using namespace ::com::sun::star;

//
// class XMLEscapementPropHdl
//

XMLCharHeightHdl::~XMLCharHeightHdl()
{
    // nothing to do
}

sal_Bool XMLCharHeightHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    if( rStrImpValue.indexOf( sal_Unicode('%') ) == -1 )
    {
        double fSize;
        sal_Int16 const eSrcUnit = ::sax::Converter::GetUnitFromString(
                rStrImpValue, util::MeasureUnit::POINT );
        if (::sax::Converter::convertDouble(fSize, rStrImpValue,
                    eSrcUnit, util::MeasureUnit::POINT))
        {
            fSize = ::std::max<double>(fSize, 1.0); // fdo#49876: 0pt is invalid
            rValue <<= (float)fSize;
            return sal_True;
        }
    }

    return sal_False;
}

sal_Bool XMLCharHeightHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    OUStringBuffer aOut;

    float fSize = 0;
    if( rValue >>= fSize )
    {
        fSize = ::std::max<float>(fSize, 1.0f); // fdo#49876: 0pt is invalid
        ::sax::Converter::convertDouble(aOut, (double)fSize, true,
                util::MeasureUnit::POINT, util::MeasureUnit::POINT);
        aOut.append( sal_Unicode('p'));
        aOut.append( sal_Unicode('t'));
    }

    rStrExpValue = aOut.makeStringAndClear();
    return !rStrExpValue.isEmpty();
}

//
// class XMLEscapementHeightPropHdl
//

XMLCharHeightPropHdl::~XMLCharHeightPropHdl()
{
    // nothing to do
}

sal_Bool XMLCharHeightPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    if( rStrImpValue.indexOf( sal_Unicode('%') ) != -1 )
    {
        sal_Int32 nPrc = 100;
        if (::sax::Converter::convertPercent( nPrc, rStrImpValue ))
        {
            rValue <<= (sal_Int16)nPrc;
            return sal_True;
        }
    }

    return sal_False;
}

sal_Bool XMLCharHeightPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    OUStringBuffer aOut( rStrExpValue );

    sal_Int16 nValue = sal_Int16();
    if( rValue >>= nValue )
    {
        ::sax::Converter::convertPercent( aOut, nValue );
    }

    rStrExpValue = aOut.makeStringAndClear();
    return !rStrExpValue.isEmpty();
}

//
// class XMLEscapementPropHdl
//

XMLCharHeightDiffHdl::~XMLCharHeightDiffHdl()
{
    // nothing to do
}

sal_Bool XMLCharHeightDiffHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nRel = 0;

    if (::sax::Converter::convertMeasure( nRel, rStrImpValue,
                util::MeasureUnit::POINT ))
    {
        rValue <<= (float)nRel;
        return sal_True;
    }

    return sal_False;
}

sal_Bool XMLCharHeightDiffHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    OUStringBuffer aOut;

    float nRel = 0;
    if( (rValue >>= nRel) && (nRel != 0) )
    {
        ::sax::Converter::convertMeasure( aOut, static_cast<sal_Int32>(nRel),
                util::MeasureUnit::POINT, util::MeasureUnit::POINT );
        rStrExpValue = aOut.makeStringAndClear();
    }

    return !rStrExpValue.isEmpty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
