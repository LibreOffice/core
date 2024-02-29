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

#include <rtl/math.h>

#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <rtl/math.hxx>

#include <algorithm>
#include <bit>
#include <cassert>
#include <cfenv>
#include <cmath>
#include <float.h>
#include <limits>
#include <memory>
#include <stdlib.h>

#include "strtmpl.hxx"

#include <dtoa.h>

constexpr int minExp = -323, maxExp = 308;
constexpr double n10s[] = {
    1e-323, 1e-322, 1e-321, 1e-320, 1e-319, 1e-318, 1e-317, 1e-316, 1e-315, 1e-314, 1e-313, 1e-312,
    1e-311, 1e-310, 1e-309, 1e-308, 1e-307, 1e-306, 1e-305, 1e-304, 1e-303, 1e-302, 1e-301, 1e-300,
    1e-299, 1e-298, 1e-297, 1e-296, 1e-295, 1e-294, 1e-293, 1e-292, 1e-291, 1e-290, 1e-289, 1e-288,
    1e-287, 1e-286, 1e-285, 1e-284, 1e-283, 1e-282, 1e-281, 1e-280, 1e-279, 1e-278, 1e-277, 1e-276,
    1e-275, 1e-274, 1e-273, 1e-272, 1e-271, 1e-270, 1e-269, 1e-268, 1e-267, 1e-266, 1e-265, 1e-264,
    1e-263, 1e-262, 1e-261, 1e-260, 1e-259, 1e-258, 1e-257, 1e-256, 1e-255, 1e-254, 1e-253, 1e-252,
    1e-251, 1e-250, 1e-249, 1e-248, 1e-247, 1e-246, 1e-245, 1e-244, 1e-243, 1e-242, 1e-241, 1e-240,
    1e-239, 1e-238, 1e-237, 1e-236, 1e-235, 1e-234, 1e-233, 1e-232, 1e-231, 1e-230, 1e-229, 1e-228,
    1e-227, 1e-226, 1e-225, 1e-224, 1e-223, 1e-222, 1e-221, 1e-220, 1e-219, 1e-218, 1e-217, 1e-216,
    1e-215, 1e-214, 1e-213, 1e-212, 1e-211, 1e-210, 1e-209, 1e-208, 1e-207, 1e-206, 1e-205, 1e-204,
    1e-203, 1e-202, 1e-201, 1e-200, 1e-199, 1e-198, 1e-197, 1e-196, 1e-195, 1e-194, 1e-193, 1e-192,
    1e-191, 1e-190, 1e-189, 1e-188, 1e-187, 1e-186, 1e-185, 1e-184, 1e-183, 1e-182, 1e-181, 1e-180,
    1e-179, 1e-178, 1e-177, 1e-176, 1e-175, 1e-174, 1e-173, 1e-172, 1e-171, 1e-170, 1e-169, 1e-168,
    1e-167, 1e-166, 1e-165, 1e-164, 1e-163, 1e-162, 1e-161, 1e-160, 1e-159, 1e-158, 1e-157, 1e-156,
    1e-155, 1e-154, 1e-153, 1e-152, 1e-151, 1e-150, 1e-149, 1e-148, 1e-147, 1e-146, 1e-145, 1e-144,
    1e-143, 1e-142, 1e-141, 1e-140, 1e-139, 1e-138, 1e-137, 1e-136, 1e-135, 1e-134, 1e-133, 1e-132,
    1e-131, 1e-130, 1e-129, 1e-128, 1e-127, 1e-126, 1e-125, 1e-124, 1e-123, 1e-122, 1e-121, 1e-120,
    1e-119, 1e-118, 1e-117, 1e-116, 1e-115, 1e-114, 1e-113, 1e-112, 1e-111, 1e-110, 1e-109, 1e-108,
    1e-107, 1e-106, 1e-105, 1e-104, 1e-103, 1e-102, 1e-101, 1e-100, 1e-99,  1e-98,  1e-97,  1e-96,
    1e-95,  1e-94,  1e-93,  1e-92,  1e-91,  1e-90,  1e-89,  1e-88,  1e-87,  1e-86,  1e-85,  1e-84,
    1e-83,  1e-82,  1e-81,  1e-80,  1e-79,  1e-78,  1e-77,  1e-76,  1e-75,  1e-74,  1e-73,  1e-72,
    1e-71,  1e-70,  1e-69,  1e-68,  1e-67,  1e-66,  1e-65,  1e-64,  1e-63,  1e-62,  1e-61,  1e-60,
    1e-59,  1e-58,  1e-57,  1e-56,  1e-55,  1e-54,  1e-53,  1e-52,  1e-51,  1e-50,  1e-49,  1e-48,
    1e-47,  1e-46,  1e-45,  1e-44,  1e-43,  1e-42,  1e-41,  1e-40,  1e-39,  1e-38,  1e-37,  1e-36,
    1e-35,  1e-34,  1e-33,  1e-32,  1e-31,  1e-30,  1e-29,  1e-28,  1e-27,  1e-26,  1e-25,  1e-24,
    1e-23,  1e-22,  1e-21,  1e-20,  1e-19,  1e-18,  1e-17,  1e-16,  1e-15,  1e-14,  1e-13,  1e-12,
    1e-11,  1e-10,  1e-9,   1e-8,   1e-7,   1e-6,   1e-5,   1e-4,   1e-3,   1e-2,   1e-1,   1e0,
    1e1,    1e2,    1e3,    1e4,    1e5,    1e6,    1e7,    1e8,    1e9,    1e10,   1e11,   1e12,
    1e13,   1e14,   1e15,   1e16,   1e17,   1e18,   1e19,   1e20,   1e21,   1e22,   1e23,   1e24,
    1e25,   1e26,   1e27,   1e28,   1e29,   1e30,   1e31,   1e32,   1e33,   1e34,   1e35,   1e36,
    1e37,   1e38,   1e39,   1e40,   1e41,   1e42,   1e43,   1e44,   1e45,   1e46,   1e47,   1e48,
    1e49,   1e50,   1e51,   1e52,   1e53,   1e54,   1e55,   1e56,   1e57,   1e58,   1e59,   1e60,
    1e61,   1e62,   1e63,   1e64,   1e65,   1e66,   1e67,   1e68,   1e69,   1e70,   1e71,   1e72,
    1e73,   1e74,   1e75,   1e76,   1e77,   1e78,   1e79,   1e80,   1e81,   1e82,   1e83,   1e84,
    1e85,   1e86,   1e87,   1e88,   1e89,   1e90,   1e91,   1e92,   1e93,   1e94,   1e95,   1e96,
    1e97,   1e98,   1e99,   1e100,  1e101,  1e102,  1e103,  1e104,  1e105,  1e106,  1e107,  1e108,
    1e109,  1e110,  1e111,  1e112,  1e113,  1e114,  1e115,  1e116,  1e117,  1e118,  1e119,  1e120,
    1e121,  1e122,  1e123,  1e124,  1e125,  1e126,  1e127,  1e128,  1e129,  1e130,  1e131,  1e132,
    1e133,  1e134,  1e135,  1e136,  1e137,  1e138,  1e139,  1e140,  1e141,  1e142,  1e143,  1e144,
    1e145,  1e146,  1e147,  1e148,  1e149,  1e150,  1e151,  1e152,  1e153,  1e154,  1e155,  1e156,
    1e157,  1e158,  1e159,  1e160,  1e161,  1e162,  1e163,  1e164,  1e165,  1e166,  1e167,  1e168,
    1e169,  1e170,  1e171,  1e172,  1e173,  1e174,  1e175,  1e176,  1e177,  1e178,  1e179,  1e180,
    1e181,  1e182,  1e183,  1e184,  1e185,  1e186,  1e187,  1e188,  1e189,  1e190,  1e191,  1e192,
    1e193,  1e194,  1e195,  1e196,  1e197,  1e198,  1e199,  1e200,  1e201,  1e202,  1e203,  1e204,
    1e205,  1e206,  1e207,  1e208,  1e209,  1e210,  1e211,  1e212,  1e213,  1e214,  1e215,  1e216,
    1e217,  1e218,  1e219,  1e220,  1e221,  1e222,  1e223,  1e224,  1e225,  1e226,  1e227,  1e228,
    1e229,  1e230,  1e231,  1e232,  1e233,  1e234,  1e235,  1e236,  1e237,  1e238,  1e239,  1e240,
    1e241,  1e242,  1e243,  1e244,  1e245,  1e246,  1e247,  1e248,  1e249,  1e250,  1e251,  1e252,
    1e253,  1e254,  1e255,  1e256,  1e257,  1e258,  1e259,  1e260,  1e261,  1e262,  1e263,  1e264,
    1e265,  1e266,  1e267,  1e268,  1e269,  1e270,  1e271,  1e272,  1e273,  1e274,  1e275,  1e276,
    1e277,  1e278,  1e279,  1e280,  1e281,  1e282,  1e283,  1e284,  1e285,  1e286,  1e287,  1e288,
    1e289,  1e290,  1e291,  1e292,  1e293,  1e294,  1e295,  1e296,  1e297,  1e298,  1e299,  1e300,
    1e301,  1e302,  1e303,  1e304,  1e305,  1e306,  1e307,  1e308,
};
static_assert(SAL_N_ELEMENTS(n10s) == maxExp - minExp + 1);

// return pow(10.0,nExp) optimized for exponents in the interval [-323,308] (i.e., incl. denormals)
static double getN10Exp(int nExp)
{
    if (nExp < minExp || nExp > maxExp)
        return pow(10.0, static_cast<double>(nExp)); // will return 0 or INF with IEEE 754
    return n10s[nExp - minExp];
}

namespace
{
/** If value (passed as absolute value) is an integer representable as double,
    which we handle explicitly at some places.
 */
bool isRepresentableInteger(double fAbsValue)
{
    static_assert(std::numeric_limits<double>::is_iec559
                  && std::numeric_limits<double>::digits == 53);
    assert(fAbsValue >= 0.0);
    if (fAbsValue >= 0x1p53)
        return false;
    sal_Int64 nInt = static_cast<sal_Int64>(fAbsValue);
    return nInt == fAbsValue;
}

/** Returns number of binary bits for fractional part of the number
    Expects a proper non-negative double value, not +-INF, not NAN
 */
int getBitsInFracPart(double fAbsValue)
{
    assert(std::isfinite(fAbsValue) && fAbsValue >= 0.0);
    if (fAbsValue == 0.0)
        return 0;
    auto& rValParts = reinterpret_cast<const sal_math_Double*>(&fAbsValue)->parts;
    int nExponent = rValParts.exponent - 1023;
    if (nExponent >= 52)
        return 0; // All bits in fraction are in integer part of the number
    int nLeastSignificant = rValParts.fraction
                                ? std::countr_zero(rValParts.fraction) + 1
                                : 53; // the implied leading 1 is the least significant
    int nFracSignificant = 53 - nLeastSignificant;
    int nBitsInFracPart = nFracSignificant - nExponent;

    return std::max(nBitsInFracPart, 0);
}
}

void SAL_CALL rtl_math_doubleToString(rtl_String** pResult, sal_Int32* pResultCapacity,
                                      sal_Int32 nResultOffset, double fValue,
                                      rtl_math_StringFormat eFormat, sal_Int32 nDecPlaces,
                                      char cDecSeparator, sal_Int32 const* pGroups,
                                      char cGroupSeparator, sal_Bool bEraseTrailingDecZeros)
    SAL_THROW_EXTERN_C()
{
    rtl::str::doubleToString(pResult, pResultCapacity, nResultOffset, fValue, eFormat, nDecPlaces,
                             cDecSeparator, pGroups, cGroupSeparator, bEraseTrailingDecZeros);
}

void SAL_CALL rtl_math_doubleToUString(rtl_uString** pResult, sal_Int32* pResultCapacity,
                                       sal_Int32 nResultOffset, double fValue,
                                       rtl_math_StringFormat eFormat, sal_Int32 nDecPlaces,
                                       sal_Unicode cDecSeparator, sal_Int32 const* pGroups,
                                       sal_Unicode cGroupSeparator, sal_Bool bEraseTrailingDecZeros)
    SAL_THROW_EXTERN_C()
{
    rtl::str::doubleToString(pResult, pResultCapacity, nResultOffset, fValue, eFormat, nDecPlaces,
                             cDecSeparator, pGroups, cGroupSeparator, bEraseTrailingDecZeros);
}

namespace
{
template <typename CharT>
double stringToDouble(CharT const* pBegin, CharT const* pEnd, CharT cDecSeparator,
                      CharT cGroupSeparator, rtl_math_ConversionStatus* pStatus,
                      CharT const** pParsedEnd)
{
    double fVal = 0.0;
    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;

    CharT const* p0 = pBegin;
    while (p0 != pEnd && (*p0 == ' ' || *p0 == '\t'))
    {
        ++p0;
    }

    bool bSign;
    bool explicitSign = false;
    if (p0 != pEnd && *p0 == '-')
    {
        bSign = true;
        explicitSign = true;
        ++p0;
    }
    else
    {
        bSign = false;
        if (p0 != pEnd && *p0 == '+')
        {
            explicitSign = true;
            ++p0;
        }
    }

    CharT const* p = p0;
    bool bDone = false;

    // #i112652# XMLSchema-2
    if ((pEnd - p) >= 3)
    {
        if (!explicitSign && ('N' == p[0]) && ('a' == p[1]) && ('N' == p[2]))
        {
            p += 3;
            fVal = std::numeric_limits<double>::quiet_NaN();
            bDone = true;
        }
        else if (('I' == p[0]) && ('N' == p[1]) && ('F' == p[2]))
        {
            p += 3;
            fVal = HUGE_VAL;
            eStatus = rtl_math_ConversionStatus_OutOfRange;
            bDone = true;
        }
    }

    if (!bDone) // do not recognize e.g. NaN1.23
    {
        std::unique_ptr<char[]> bufInHeap;
        std::unique_ptr<const CharT* []> bufInHeapMap;
        constexpr int bufOnStackSize = 256;
        char bufOnStack[bufOnStackSize];
        const CharT* bufOnStackMap[bufOnStackSize];
        char* buf = bufOnStack;
        const CharT** bufmap = bufOnStackMap;
        int bufpos = 0;
        const size_t bufsize = pEnd - p + (bSign ? 2 : 1);
        if (bufsize > bufOnStackSize)
        {
            bufInHeap = std::make_unique<char[]>(bufsize);
            bufInHeapMap = std::make_unique<const CharT* []>(bufsize);
            buf = bufInHeap.get();
            bufmap = bufInHeapMap.get();
        }

        if (bSign)
        {
            buf[0] = '-';
            bufmap[0] = p; // yes, this may be the same pointer as for the next mapping
            bufpos = 1;
        }
        // Put first zero to buffer for strings like "-0"
        if (p != pEnd && *p == '0')
        {
            buf[bufpos] = '0';
            bufmap[bufpos] = p;
            ++bufpos;
            ++p;
        }
        // Leading zeros and group separators between digits may be safely
        // ignored. p0 < p implies that there was a leading 0 already,
        // consecutive group separators may not happen as *(p+1) is checked for
        // digit.
        while (p != pEnd
               && (*p == '0'
                   || (*p == cGroupSeparator && p0 < p && p + 1 < pEnd
                       && rtl::isAsciiDigit(*(p + 1)))))
        {
            ++p;
        }

        // integer part of mantissa
        for (; p != pEnd; ++p)
        {
            CharT c = *p;
            if (rtl::isAsciiDigit(c))
            {
                buf[bufpos] = static_cast<char>(c);
                bufmap[bufpos] = p;
                ++bufpos;
            }
            else if (c != cGroupSeparator)
            {
                break;
            }
            else if (p == p0 || (p + 1 == pEnd) || !rtl::isAsciiDigit(*(p + 1)))
            {
                // A leading or trailing (not followed by a digit) group
                // separator character is not a group separator.
                break;
            }
        }

        // fraction part of mantissa
        if (p != pEnd && *p == cDecSeparator)
        {
            buf[bufpos] = '.';
            bufmap[bufpos] = p;
            ++bufpos;
            ++p;

            for (; p != pEnd; ++p)
            {
                CharT c = *p;
                if (!rtl::isAsciiDigit(c))
                {
                    break;
                }
                buf[bufpos] = static_cast<char>(c);
                bufmap[bufpos] = p;
                ++bufpos;
            }
        }

        // Exponent
        if (p != p0 && p != pEnd && (*p == 'E' || *p == 'e'))
        {
            buf[bufpos] = 'E';
            bufmap[bufpos] = p;
            ++bufpos;
            ++p;
            if (p != pEnd && *p == '-')
            {
                buf[bufpos] = '-';
                bufmap[bufpos] = p;
                ++bufpos;
                ++p;
            }
            else if (p != pEnd && *p == '+')
                ++p;

            for (; p != pEnd; ++p)
            {
                CharT c = *p;
                if (!rtl::isAsciiDigit(c))
                    break;

                buf[bufpos] = static_cast<char>(c);
                bufmap[bufpos] = p;
                ++bufpos;
            }
        }
        else if (p - p0 == 2 && p != pEnd && p[0] == '#' && p[-1] == cDecSeparator && p[-2] == '1')
        {
            if (pEnd - p >= 4 && p[1] == 'I' && p[2] == 'N' && p[3] == 'F')
            {
                // "1.#INF", "+1.#INF", "-1.#INF"
                p += 4;
                fVal = HUGE_VAL;
                eStatus = rtl_math_ConversionStatus_OutOfRange;
                // Eat any further digits:
                while (p != pEnd && rtl::isAsciiDigit(*p))
                    ++p;
                bDone = true;
            }
            else if (pEnd - p >= 4 && p[1] == 'N' && p[2] == 'A' && p[3] == 'N')
            {
                // "1.#NAN", "+1.#NAN", "-1.#NAN"
                p += 4;
                fVal = std::copysign(std::numeric_limits<double>::quiet_NaN(), bSign ? -1.0 : 1.0);
                bSign = false; // don't negate again

                // Eat any further digits:
                while (p != pEnd && rtl::isAsciiDigit(*p))
                {
                    ++p;
                }
                bDone = true;
            }
        }

        if (!bDone)
        {
            buf[bufpos] = '\0';
            bufmap[bufpos] = p;
            char* pCharParseEnd;
            errno = 0;
            fVal = strtod_nolocale(buf, &pCharParseEnd);
            if (errno == ERANGE)
            {
                // Check for the dreaded rounded to 15 digits max value
                // 1.79769313486232e+308 for 1.7976931348623157e+308 we wrote
                // everywhere, accept with or without plus sign in exponent.
                const char* b = buf;
                if (b[0] == '-')
                    ++b;
                if (((pCharParseEnd - b == 21) || (pCharParseEnd - b == 20))
                    && !strncmp(b, "1.79769313486232", 16) && (b[16] == 'e' || b[16] == 'E')
                    && (((pCharParseEnd - b == 21) && !strncmp(b + 17, "+308", 4))
                        || ((pCharParseEnd - b == 20) && !strncmp(b + 17, "308", 3))))
                {
                    fVal = (buf < b) ? -DBL_MAX : DBL_MAX;
                }
                else
                {
                    eStatus = rtl_math_ConversionStatus_OutOfRange;
                }
            }
            p = bufmap[pCharParseEnd - buf];
            bSign = false;
        }
    }

    // overflow also if more than DBL_MAX_10_EXP digits without decimal
    // separator, or 0. and more than DBL_MIN_10_EXP digits, ...
    bool bHuge = fVal == HUGE_VAL; // g++ 3.0.1 requires it this way...
    if (bHuge)
        eStatus = rtl_math_ConversionStatus_OutOfRange;

    if (bSign)
        fVal = -fVal;

    if (pStatus)
        *pStatus = eStatus;

    if (pParsedEnd)
        *pParsedEnd = p == p0 ? pBegin : p;

    return fVal;
}
}

double SAL_CALL rtl_math_stringToDouble(char const* pBegin, char const* pEnd, char cDecSeparator,
                                        char cGroupSeparator, rtl_math_ConversionStatus* pStatus,
                                        char const** pParsedEnd) SAL_THROW_EXTERN_C()
{
    return stringToDouble(reinterpret_cast<unsigned char const*>(pBegin),
                          reinterpret_cast<unsigned char const*>(pEnd),
                          static_cast<unsigned char>(cDecSeparator),
                          static_cast<unsigned char>(cGroupSeparator), pStatus,
                          reinterpret_cast<unsigned char const**>(pParsedEnd));
}

double SAL_CALL rtl_math_uStringToDouble(sal_Unicode const* pBegin, sal_Unicode const* pEnd,
                                         sal_Unicode cDecSeparator, sal_Unicode cGroupSeparator,
                                         rtl_math_ConversionStatus* pStatus,
                                         sal_Unicode const** pParsedEnd) SAL_THROW_EXTERN_C()
{
    return stringToDouble(pBegin, pEnd, cDecSeparator, cGroupSeparator, pStatus, pParsedEnd);
}

double SAL_CALL rtl_math_round(double fValue, int nDecPlaces, enum rtl_math_RoundingMode eMode)
    SAL_THROW_EXTERN_C()
{
    if (!std::isfinite(fValue))
        return fValue;

    if (fValue == 0.0)
        return fValue;

    const double fOrigValue = fValue;

    // sign adjustment
    bool bSign = std::signbit(fValue);
    if (bSign)
        fValue = -fValue;

    // Rounding to decimals between integer distance precision (gaps) does not
    // make sense, do not even try to multiply/divide and introduce inaccuracy.
    // For same reasons, do not attempt to round integers to decimals.
    if (nDecPlaces >= 0 && (fValue >= 0x1p52 || isRepresentableInteger(fValue)))
        return fOrigValue;

    double fFac = 0;
    if (nDecPlaces != 0)
    {
        if (nDecPlaces > 0)
        {
            // Determine how many decimals are representable in the precision.
            // Anything greater 2^52 and 0.0 was already ruled out above.
            // Theoretically 0.5, 0.25, 0.125, 0.0625, 0.03125, ...
            const sal_math_Double* pd = reinterpret_cast<const sal_math_Double*>(&fValue);
            const sal_Int32 nDec = 52 - (pd->parts.exponent - 1023);

            if (nDec <= 0)
            {
                assert(!"Shouldn't this had been caught already as large number?");
                return fOrigValue;
            }

            if (nDec < nDecPlaces)
                nDecPlaces = nDec;
        }

        // Avoid 1e-5 (1.0000000000000001e-05) and such inaccurate fractional
        // factors that later when dividing back spoil things. For negative
        // decimals divide first with the inverse, then multiply the rounded
        // value back.
        fFac = getN10Exp(abs(nDecPlaces));

        if (fFac == 0.0 || (nDecPlaces < 0 && !std::isfinite(fFac)))
            // Underflow, rounding to that many integer positions would be 0.
            return 0.0;

        if (!std::isfinite(fFac))
            // Overflow with very small values and high number of decimals.
            return fOrigValue;

        if (nDecPlaces < 0)
            fValue /= fFac;
        else
            fValue *= fFac;

        if (!std::isfinite(fValue))
            return fOrigValue;
    }

    // Round only if not already in distance precision gaps of integers, where
    // for [2^52,2^53) adding 0.5 would even yield the next representable
    // integer.
    if (fValue < 0x1p52)
    {
        switch (eMode)
        {
            case rtl_math_RoundingMode_Corrected:
                fValue = rtl::math::approxFloor(fValue + 0.5);
                break;
            case rtl_math_RoundingMode_Down:
                fValue = rtl::math::approxFloor(fValue);
                break;
            case rtl_math_RoundingMode_Up:
                fValue = rtl::math::approxCeil(fValue);
                break;
            case rtl_math_RoundingMode_Floor:
                fValue = bSign ? rtl::math::approxCeil(fValue) : rtl::math::approxFloor(fValue);
                break;
            case rtl_math_RoundingMode_Ceiling:
                fValue = bSign ? rtl::math::approxFloor(fValue) : rtl::math::approxCeil(fValue);
                break;
            case rtl_math_RoundingMode_HalfDown:
            {
                double f = floor(fValue);
                fValue = ((fValue - f) <= 0.5) ? f : ceil(fValue);
            }
            break;
            case rtl_math_RoundingMode_HalfUp:
            {
                double f = floor(fValue);
                fValue = ((fValue - f) < 0.5) ? f : ceil(fValue);
            }
            break;
            case rtl_math_RoundingMode_HalfEven:
                if (const int oldMode = std::fegetround(); std::fesetround(FE_TONEAREST) == 0)
                {
                    fValue = std::nearbyint(fValue);
                    std::fesetround(oldMode);
                }
                else
                {
                    double f = floor(fValue);
                    if ((fValue - f) != 0.5)
                    {
                        fValue = floor(fValue + 0.5);
                    }
                    else
                    {
                        double g = f / 2.0;
                        fValue = (g == floor(g)) ? f : (f + 1.0);
                    }
                }
                break;
            default:
                OSL_ASSERT(false);
                break;
        }
    }

    if (nDecPlaces != 0)
    {
        if (nDecPlaces < 0)
            fValue *= fFac;
        else
            fValue /= fFac;
    }

    if (!std::isfinite(fValue))
        return fOrigValue;

    return bSign ? -fValue : fValue;
}

double SAL_CALL rtl_math_pow10Exp(double fValue, int nExp) SAL_THROW_EXTERN_C()
{
    return fValue * getN10Exp(nExp);
}

double SAL_CALL rtl_math_approxValue(double fValue) SAL_THROW_EXTERN_C()
{
    const double fBigInt = 0x1p41; // 2^41 -> only 11 bits left for fractional part, fine as decimal
    if (fValue == 0.0 || fValue == HUGE_VAL || !std::isfinite(fValue) || fValue > fBigInt)
    {
        // We don't handle these conditions.  Bail out.
        return fValue;
    }

    double fOrigValue = fValue;

    bool bSign = std::signbit(fValue);
    if (bSign)
        fValue = -fValue;

    // If the value is either integer representable as double,
    // or only has small number of bits in fraction part, then we need not do any approximation
    if (isRepresentableInteger(fValue) || getBitsInFracPart(fValue) <= 11)
        return fOrigValue;

    int nExp = static_cast<int>(floor(log10(fValue)));
    nExp = 14 - nExp;
    double fExpValue = getN10Exp(abs(nExp));

    if (nExp < 0)
        fValue /= fExpValue;
    else
        fValue *= fExpValue;

    // If the original value was near DBL_MIN we got an overflow. Restore and
    // bail out.
    if (!std::isfinite(fValue))
        return fOrigValue;

    fValue = std::round(fValue);

    if (nExp < 0)
        fValue *= fExpValue;
    else
        fValue /= fExpValue;

    // If the original value was near DBL_MAX we got an overflow. Restore and
    // bail out.
    if (!std::isfinite(fValue))
        return fOrigValue;

    return bSign ? -fValue : fValue;
}

bool SAL_CALL rtl_math_approxEqual(double a, double b) SAL_THROW_EXTERN_C()
{
    static const double e48 = 0x1p-48;

    if (a == b)
        return true;

    if (a == 0.0 || b == 0.0 || std::signbit(a) != std::signbit(b))
        return false;

    const double d = fabs(a - b);
    if (!std::isfinite(d))
        return false; // Nan or Inf involved

    a = fabs(a);
    if (d >= (a * e48))
        return false;
    b = fabs(b);
    if (d >= (b * e48))
        return false;

    if (isRepresentableInteger(a) && isRepresentableInteger(b))
        return false; // special case for representable integers.

    return true;
}

double SAL_CALL rtl_math_expm1(double fValue) SAL_THROW_EXTERN_C() { return expm1(fValue); }

double SAL_CALL rtl_math_log1p(double fValue) SAL_THROW_EXTERN_C()
{
#ifdef __APPLE__
    if (fValue == -0.0)
        return fValue; // macOS 10.8 libc returns 0.0 for -0.0
#endif

    return log1p(fValue);
}

double SAL_CALL rtl_math_atanh(double fValue) SAL_THROW_EXTERN_C() { return ::atanh(fValue); }

/** Parent error function (erf) */
double SAL_CALL rtl_math_erf(double x) SAL_THROW_EXTERN_C() { return erf(x); }

/** Parent complementary error function (erfc) */
double SAL_CALL rtl_math_erfc(double x) SAL_THROW_EXTERN_C() { return erfc(x); }

/** improved accuracy of asinh for |x| large and for x near zero
    @see #i97605#
 */
double SAL_CALL rtl_math_asinh(double fX) SAL_THROW_EXTERN_C()
{
    if (fX == 0.0)
        return 0.0;

    double fSign = 1.0;
    if (fX < 0.0)
    {
        fX = -fX;
        fSign = -1.0;
    }

    if (fX < 0.125)
        return fSign * rtl_math_log1p(fX + fX * fX / (1.0 + sqrt(1.0 + fX * fX)));

    if (fX < 1.25e7)
        return fSign * log(fX + sqrt(1.0 + fX * fX));

    return fSign * log(2.0 * fX);
}

/** improved accuracy of acosh for x large and for x near 1
    @see #i97605#
 */
double SAL_CALL rtl_math_acosh(double fX) SAL_THROW_EXTERN_C()
{
    volatile double fZ = fX - 1.0;
    if (fX < 1.0)
        return std::numeric_limits<double>::quiet_NaN();
    if (fX == 1.0)
        return 0.0;

    if (fX < 1.1)
        return rtl_math_log1p(fZ + sqrt(fZ * fZ + 2.0 * fZ));

    if (fX < 1.25e7)
        return log(fX + sqrt(fX * fX - 1.0));

    return log(2.0 * fX);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
