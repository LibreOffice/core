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

#include "rtl/math.h"
#include "rtl/math.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include <sal/macros.h>

#include <stdlib.h>

#include <sal/cppunit.h>

namespace {

struct FloatTraits
{
    typedef float Number;

    static inline char const * getPrefix() { return "float"; }
};

struct DoubleTraits
{
    typedef double Number;

    static inline char const * getPrefix() { return "double"; }
};

struct StringTraits
{
    typedef rtl::OString String;

    static inline char const * getPrefix() { return "OString"; }

    static inline rtl::OString createFromAscii(char const * pString)
    { return rtl::OString(pString); }

    static inline void appendBuffer(rtl::OStringBuffer & rBuffer,
                                    rtl::OString const & rString)
    { rBuffer.append(rString); }

    static inline rtl::OString doubleToString(double fValue,
                                              rtl_math_StringFormat eFormat,
                                              sal_Int32 nDecPlaces,
                                              sal_Char cDecSeparator,
                                              bool bEraseTrailingDecZeros)
    {
        return rtl::math::doubleToString(fValue, eFormat, nDecPlaces,
                                         cDecSeparator, bEraseTrailingDecZeros);
    }
};

struct UStringTraits
{
    typedef rtl::OUString String;

    static inline char const * getPrefix() { return "OUString"; }

    static inline rtl::OUString createFromAscii(char const * pString)
    { return rtl::OUString::createFromAscii(pString); }

    static inline void appendBuffer(rtl::OStringBuffer & rBuffer,
                                    rtl::OUString const & rString)
    { rBuffer.append(rtl::OUStringToOString(rString, RTL_TEXTENCODING_UTF8)); }

    static inline rtl::OUString doubleToString(double fValue,
                                               rtl_math_StringFormat eFormat,
                                               sal_Int32 nDecPlaces,
                                               sal_Unicode cDecSeparator,
                                               bool bEraseTrailingDecZeros)
        {
        return rtl::math::doubleToUString(fValue, eFormat, nDecPlaces,
                                          cDecSeparator,
                                          bEraseTrailingDecZeros);
    }
};

struct TestNumberToString
{
    double fValue;
    rtl_math_StringFormat eFormat;
    sal_Int32 nDecPlaces;
    char cDecSeparator;
    bool bEraseTrailingDecZeros;
    char const * pResult;
};

template< typename StringT, typename NumberT >
void testNumberToString(TestNumberToString const & rTest)
{
    typename NumberT::Number fValue = static_cast< typename NumberT::Number >(rTest.fValue);
    if (fValue != rTest.fValue)
        return;

    // LLA: t_print("size: %d ", sizeof(fValue));
    typename StringT::String aResult1;

    aResult1 = StringT::doubleToString(fValue, rTest.eFormat, rTest.nDecPlaces,
                                       rTest.cDecSeparator,
                                       rTest.bEraseTrailingDecZeros);

    typename StringT::String aResult2(StringT::createFromAscii(rTest.pResult));

    rtl::OStringBuffer aBuffer;
    aBuffer.append(StringT::getPrefix());
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM("/"));
    aBuffer.append(NumberT::getPrefix());
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM(" doubleToString("));
    aBuffer.append(fValue);
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM(", "));
    aBuffer.append(static_cast< sal_Int32 >(rTest.eFormat));
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM(", "));
    aBuffer.append(rTest.nDecPlaces);
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM(", "));
    aBuffer.append(rTest.cDecSeparator);
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM(", "));
    aBuffer.append(static_cast< sal_Int32 >(rTest.bEraseTrailingDecZeros));
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM("): "));
    StringT::appendBuffer(aBuffer, aResult1);
    if (aResult1 != aResult2)
    {
        aBuffer.append(RTL_CONSTASCII_STRINGPARAM(" != "));
        StringT::appendBuffer(aBuffer, aResult2);
    }
    CPPUNIT_ASSERT_MESSAGE(aBuffer.getStr(), aResult1 == aResult2);
}

template< typename StringT, typename NumberT >
void testNumberToString(TestNumberToString const * pTests, size_t nCount)
{
    for (size_t i = 0; i < nCount; ++i)
        testNumberToString< StringT, NumberT >(pTests[i]);
}

struct TestStringToNumberToString
{
    char const * pValue;
    rtl_math_StringFormat eFormat;
    sal_Int32 nDecPlaces;
    char cDecSeparator;
    bool bEraseTrailingDecZeros;
    char const * pResult;
};

template< typename StringT >
void testStringToNumberToString(TestStringToNumberToString const & rTest)
{
    double d = rtl::math::stringToDouble(StringT::createFromAscii(rTest.pValue),
                                         rTest.cDecSeparator, 0, 0, 0);
    typename StringT::String aResult1(
        StringT::doubleToString(d, rTest.eFormat, rTest.nDecPlaces,
                                rTest.cDecSeparator,
                                rTest.bEraseTrailingDecZeros));
    typename StringT::String aResult2(StringT::createFromAscii(rTest.pResult));
    rtl::OStringBuffer aBuffer;
    aBuffer.append(StringT::getPrefix());
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM(
                       " doubleToString(stringToDouble("));
    aBuffer.append(rTest.pValue);
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM(", "));
    aBuffer.append(rTest.cDecSeparator);
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM("), "));
    aBuffer.append(static_cast< sal_Int32 >(rTest.eFormat));
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM(", "));
    aBuffer.append(rTest.nDecPlaces);
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM(", "));
    aBuffer.append(rTest.cDecSeparator);
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM(", "));
    aBuffer.append(static_cast< sal_Int32 >(rTest.bEraseTrailingDecZeros));
    aBuffer.append(RTL_CONSTASCII_STRINGPARAM("): "));
    StringT::appendBuffer(aBuffer, aResult1);
    if (aResult1 != aResult2)
    {
        aBuffer.append(RTL_CONSTASCII_STRINGPARAM(" != "));
        StringT::appendBuffer(aBuffer, aResult2);
    }
    CPPUNIT_ASSERT_MESSAGE(aBuffer.getStr(), aResult1 == aResult2);
}

template< typename StringT >
void testStringToNumberToString(TestStringToNumberToString const * pTests,
                                size_t nCount)
{
    for (size_t i = 0; i < nCount; ++i)
        testStringToNumberToString< StringT >(pTests[i]);
}

}

class Math : public CppUnit::TestFixture
{
public:
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void rtl_math_001()
    {
        static TestNumberToString const aTest[]
            = { // 1, 1+2^-1, ..., 1+2^-52
                // Too few decimal digits are printed, so that various different
                // double values lead to the same output:
                { 1, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1" },
                { 1.5, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.5" },
                { 1.25, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.25" },
                { 1.125, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.125" },
                { 1.0625, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.0625" },
                { 1.03125, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.03125" },
                { 1.015625, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.015625" },
                { 1.0078125, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.0078125" },
                { 1.00390625, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.00390625" },
                { 1.001953125, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.001953125" },
                { 1.0009765625, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.0009765625" },
                { 1.00048828125, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.00048828125" },
                { 1.000244140625, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.000244140625" },
                { 1.0001220703125, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.0001220703125" },
                { 1.00006103515625, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.00006103515625" },
                { 1.000030517578125, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.00003051757813" },
                { 1.0000152587890625, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.00001525878906" },
                { 1.00000762939453125, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.00000762939453" },
                { 1.000003814697265625, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.00000381469727" },
                { 1.0000019073486328125, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.00000190734863" },
                { 1.00000095367431640625, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.00000095367432" },
                { 1.000000476837158203125, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.00000047683716" },
                { 1.0000002384185791015625, rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1.00000023841858" },
                { 1.00000011920928955078125, rtl_math_StringFormat_Automatic,
                  rtl_math_DecimalPlaces_Max, '.', true, "1.00000011920929" },
                { 1.000000059604644775390625, rtl_math_StringFormat_Automatic,
                  rtl_math_DecimalPlaces_Max, '.', true, "1.00000005960464" },
                { 1.0000000298023223876953125, rtl_math_StringFormat_Automatic,
                  rtl_math_DecimalPlaces_Max, '.', true, "1.00000002980232" },
                { 1.00000001490116119384765625, rtl_math_StringFormat_Automatic,
                  rtl_math_DecimalPlaces_Max, '.', true, "1.00000001490116" },
                { 1.000000007450580596923828125,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000745058" },
                { 1.0000000037252902984619140625,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000372529" },
                { 1.00000000186264514923095703125,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000186265" },
                { 1.000000000931322574615478515625,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000093132" },
                { 1.0000000004656612873077392578125,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000046566" },
                { 1.00000000023283064365386962890625,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000023283" },
                { 1.000000000116415321826934814453125,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000011642" },
                { 1.0000000000582076609134674072265625,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000005821" },
                { 1.00000000002910383045673370361328125,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.0000000000291" },
                { 1.000000000014551915228366851806640625,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000001455" },
                { 1.0000000000072759576141834259033203125,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000000728" },
                { 1.00000000000363797880709171295166015625,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000000364" },
                { 1.000000000001818989403545856475830078125,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000000182" },
                { 1.0000000000009094947017729282379150390625,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000000091" },
                { 1.00000000000045474735088646411895751953125,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000000045" },
                { 1.000000000000227373675443232059478759765625,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000000023" },
                { 1.0000000000001136868377216160297393798828125,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000000011" },
                { 1.00000000000005684341886080801486968994140625,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000000006" },
                { 1.000000000000028421709430404007434844970703125,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000000003" },
                { 1.0000000000000142108547152020037174224853515625,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000000001" },
                { 1.00000000000000710542735760100185871124267578125,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1.00000000000001" },
                { 1.000000000000003552713678800500929355621337890625,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1" },
                { 1.0000000000000017763568394002504646778106689453125,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1" },
                { 1.00000000000000088817841970012523233890533447265625,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1" },
                { 1.000000000000000444089209850062616169452667236328125,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1" },
                { 1.0000000000000002220446049250313080847263336181640625,
                  rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                  '.', true, "1" },

                // 1, 1+2^-1, ..., 1+2^-52
                // Too few decimal digits are printed, so that various different
                // double values lead to the same output:
                { 1, rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000000000000000000000000000000000000000000000" },
                { 1.5, rtl_math_StringFormat_F, 53, '.', false,
                  "1.50000000000000000000000000000000000000000000000000000" },
                { 1.25, rtl_math_StringFormat_F, 53, '.', false,
                  "1.25000000000000000000000000000000000000000000000000000" },
                { 1.125, rtl_math_StringFormat_F, 53, '.', false,
                  "1.12500000000000000000000000000000000000000000000000000" },
                { 1.0625, rtl_math_StringFormat_F, 53, '.', false,
                  "1.06250000000000000000000000000000000000000000000000000" },
                { 1.03125, rtl_math_StringFormat_F, 53, '.', false,
                  "1.03125000000000000000000000000000000000000000000000000" },
                { 1.015625, rtl_math_StringFormat_F, 53, '.', false,
                  "1.01562500000000000000000000000000000000000000000000000" },
                { 1.0078125, rtl_math_StringFormat_F, 53, '.', false,
                  "1.00781250000000000000000000000000000000000000000000000" },
                { 1.00390625, rtl_math_StringFormat_F, 53, '.', false,
                  "1.00390625000000000000000000000000000000000000000000000" },
                { 1.001953125, rtl_math_StringFormat_F, 53, '.', false,
                  "1.00195312500000000000000000000000000000000000000000000" },
                { 1.0009765625, rtl_math_StringFormat_F, 53, '.', false,
                  "1.00097656250000000000000000000000000000000000000000000" },
                { 1.00048828125, rtl_math_StringFormat_F, 53, '.', false,
                  "1.00048828125000000000000000000000000000000000000000000" },
                { 1.000244140625, rtl_math_StringFormat_F, 53, '.', false,
                  "1.00024414062500000000000000000000000000000000000000000" },
                { 1.0001220703125, rtl_math_StringFormat_F, 53, '.', false,
                  "1.00012207031250000000000000000000000000000000000000000" },
                { 1.00006103515625, rtl_math_StringFormat_F, 53, '.', false,
                  "1.00006103515625000000000000000000000000000000000000000" },
                { 1.000030517578125, rtl_math_StringFormat_F, 53, '.', false,
                  "1.00003051757813000000000000000000000000000000000000000" },
                { 1.0000152587890625, rtl_math_StringFormat_F, 53, '.', false,
                  "1.00001525878906000000000000000000000000000000000000000" },
                { 1.00000762939453125, rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000762939453000000000000000000000000000000000000000" },
                { 1.000003814697265625, rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000381469727000000000000000000000000000000000000000" },
                { 1.0000019073486328125, rtl_math_StringFormat_F, 53, '.',
                  false,
                  "1.00000190734863000000000000000000000000000000000000000" },
                { 1.00000095367431640625, rtl_math_StringFormat_F, 53, '.',
                  false,
                  "1.00000095367432000000000000000000000000000000000000000" },
                { 1.000000476837158203125, rtl_math_StringFormat_F, 53, '.',
                  false,
                  "1.00000047683716000000000000000000000000000000000000000" },
                { 1.0000002384185791015625, rtl_math_StringFormat_F, 53, '.',
                  false,
                  "1.00000023841858000000000000000000000000000000000000000" },
                { 1.00000011920928955078125, rtl_math_StringFormat_F, 53, '.',
                  false,
                  "1.00000011920929000000000000000000000000000000000000000" },
                { 1.000000059604644775390625, rtl_math_StringFormat_F, 53, '.',
                  false,
                  "1.00000005960464000000000000000000000000000000000000000" },
                { 1.0000000298023223876953125, rtl_math_StringFormat_F, 53, '.',
                  false,
                  "1.00000002980232000000000000000000000000000000000000000" },
                { 1.00000001490116119384765625, rtl_math_StringFormat_F, 53,
                  '.', false,
                  "1.00000001490116000000000000000000000000000000000000000" },
                { 1.000000007450580596923828125, rtl_math_StringFormat_F, 53,
                  '.', false,
                  "1.00000000745058000000000000000000000000000000000000000" },
                { 1.0000000037252902984619140625, rtl_math_StringFormat_F, 53,
                  '.', false,
                  "1.00000000372529000000000000000000000000000000000000000" },
                { 1.00000000186264514923095703125, rtl_math_StringFormat_F, 53,
                  '.', false,
                  "1.00000000186265000000000000000000000000000000000000000" },
                { 1.000000000931322574615478515625, rtl_math_StringFormat_F, 53,
                  '.', false,
                  "1.00000000093132000000000000000000000000000000000000000" },
                { 1.0000000004656612873077392578125, rtl_math_StringFormat_F,
                  53, '.', false,
                  "1.00000000046566000000000000000000000000000000000000000" },
                { 1.00000000023283064365386962890625, rtl_math_StringFormat_F,
                  53, '.', false,
                  "1.00000000023283000000000000000000000000000000000000000" },
                { 1.000000000116415321826934814453125, rtl_math_StringFormat_F,
                  53, '.', false,
                  "1.00000000011642000000000000000000000000000000000000000" },
                { 1.0000000000582076609134674072265625, rtl_math_StringFormat_F,
                  53, '.', false,
                  "1.00000000005821000000000000000000000000000000000000000" },
                { 1.00000000002910383045673370361328125,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000002910000000000000000000000000000000000000000" },
                { 1.000000000014551915228366851806640625,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000001455000000000000000000000000000000000000000" },
                { 1.0000000000072759576141834259033203125,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000728000000000000000000000000000000000000000" },
                { 1.00000000000363797880709171295166015625,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000364000000000000000000000000000000000000000" },
                { 1.000000000001818989403545856475830078125,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000182000000000000000000000000000000000000000" },
                { 1.0000000000009094947017729282379150390625,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000091000000000000000000000000000000000000000" },
                { 1.00000000000045474735088646411895751953125,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000045000000000000000000000000000000000000000" },
                { 1.000000000000227373675443232059478759765625,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000023000000000000000000000000000000000000000" },
                { 1.0000000000001136868377216160297393798828125,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000011000000000000000000000000000000000000000" },
                { 1.00000000000005684341886080801486968994140625,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000006000000000000000000000000000000000000000" },
                { 1.000000000000028421709430404007434844970703125,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000003000000000000000000000000000000000000000" },
                { 1.0000000000000142108547152020037174224853515625,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000001000000000000000000000000000000000000000" },
                { 1.00000000000000710542735760100185871124267578125,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000001000000000000000000000000000000000000000" },
                { 1.000000000000003552713678800500929355621337890625,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000000000000000000000000000000000000000000000" },
                { 1.0000000000000017763568394002504646778106689453125,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000000000000000000000000000000000000000000000" },
                { 1.00000000000000088817841970012523233890533447265625,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000000000000000000000000000000000000000000000" },
                { 1.000000000000000444089209850062616169452667236328125,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000000000000000000000000000000000000000000000" },
                { 1.0000000000000002220446049250313080847263336181640625,
                  rtl_math_StringFormat_F, 53, '.', false,
                  "1.00000000000000000000000000000000000000000000000000000" } };
        size_t const nCount = SAL_N_ELEMENTS(aTest);

//LLA: the float tests are wrong here, due to the fact that
//     we calculate with too less digits after the point

//        bReturn &= testNumberToString< StringTraits, FloatTraits >(
//            pTestResult, aTest, nCount);
        testNumberToString< StringTraits, DoubleTraits >(aTest, nCount);
//        bReturn &= testNumberToString< UStringTraits, FloatTraits >(
//            pTestResult, aTest, nCount);
        testNumberToString< UStringTraits, DoubleTraits >(aTest, nCount);
    }

    void rtl_math_002()
    {
        static TestStringToNumberToString const aTest[]
            = { { "1", rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, '.', true, "1" },
                { " 1", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1" },
                { "  1", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1" },
                { "\t1", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1" },
                { "\t 1", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1" },
                { " \t1", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "1" },

                { "-1", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "-1" },
                { " -1", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "-1" },
                { "  -1", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "-1" },
                { "\t-1", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "-1" },
                { "\t -1", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "-1" },
                { " \t-1", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true, "-1" },

                { "1.#INF", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "1.#INF" },
                { " 1.#INF", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "1.#INF" },
                { "  1.#INF", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "1.#INF" },
                { "\t1.#INF", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "1.#INF" },
                { "\t 1.#INF", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "1.#INF" },
                { " \t1.#INF", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "1.#INF" },

                { "-1.#INF", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "-1.#INF" },
                { " -1.#INF", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "-1.#INF" },
                { "  -1.#INF", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "-1.#INF" },
                { "\t-1.#INF", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "-1.#INF" },
                { "\t -1.#INF", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "-1.#INF" },
                { " \t-1.#INF", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "-1.#INF" },

                { "1.#NAN", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "1.#NAN" },
                { " 1.#NAN", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "1.#NAN" },
                { "  1.#NAN", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "1.#NAN" },
                { "\t1.#NAN", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "1.#NAN" },
                { "\t 1.#NAN", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "1.#NAN" },
                { " \t1.#NAN", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "1.#NAN" },

                { "-1.#NAN", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "-1.#NAN" },
                { " -1.#NAN", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "-1.#NAN" },
                { "  -1.#NAN", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "-1.#NAN" },
                { "\t-1.#NAN", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "-1.#NAN" },
                { "\t -1.#NAN", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "-1.#NAN" },
                { " \t-1.#NAN", rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', false, "-1.#NAN" },

                { "3.14E-2000", rtl_math_StringFormat_E, 4, '.', false,
                  "0.0000E+000" },
                { "3.14E-200", rtl_math_StringFormat_E, 4, '.', false,
                  "3.1400E-200" },
                { "3.14E-20", rtl_math_StringFormat_E, 4, '.', false,
                  "3.1400E-020" },
                { "3.14E-2", rtl_math_StringFormat_E, 4, '.', false,
                  "3.1400E-002" },
                { "3.14E2", rtl_math_StringFormat_E, 4, '.', false,
                  "3.1400E+002" },
                { "3.14E20", rtl_math_StringFormat_E, 4, '.', false,
                  "3.1400E+020" },
                { "3.14E200", rtl_math_StringFormat_E, 4, '.', false,
                  "3.1400E+200" },
                { "3.14E2000", rtl_math_StringFormat_E, 4, '.', false,
                  "1.#INF" },
            };
        size_t const nCount = SAL_N_ELEMENTS(aTest);
        testStringToNumberToString< StringTraits >(aTest, nCount);
        testStringToNumberToString< UStringTraits >(aTest, nCount);
    }

    CPPUNIT_TEST_SUITE(Math);
    CPPUNIT_TEST(rtl_math_001);
    CPPUNIT_TEST(rtl_math_002);
    CPPUNIT_TEST_SUITE_END();
};

// -----------------------------------------------------------------------------
//extern "C" void /* sal_Bool */ SAL_CALL test_rtl_math2( hTestResult hRtlTestResult )
//{
//    c_rtl_tres_state_start(hRtlTestResult, "rtl_math" );
//
//    test_rtl_math( hRtlTestResult );
//
//    c_rtl_tres_state_end(hRtlTestResult, "rtl_math" );
//}
// -----------------------------------------------------------------------------
//void RegisterAdditionalFunctions(FktRegFuncPtr _pFunc)
//{
//    if (_pFunc)
//    {
//        (_pFunc)(&test_rtl_math2, "");
//    }
//}

CPPUNIT_TEST_SUITE_REGISTRATION(::Math);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
