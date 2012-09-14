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


#include <chrhghdl.hxx>

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/uno/Any.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmluconv.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;

// this is a copy of defines in svx/inc/escpitem.hxx
#define DFLT_ESC_PROP    58
#define DFLT_ESC_AUTO_SUPER 101
#define DFLT_ESC_AUTO_SUB  -101

///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
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
