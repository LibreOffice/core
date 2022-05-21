/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <justify.hxx>
#include <vector>

namespace
{
/// Covers sw/source/core/txtnode/justify fixes.
class SwCoreJustifyTest : public SwModelTestBase
{
};

/// CharWidthArray: store char widths because they are more readable.
class CharWidthArray
{
public:
    std::vector<sal_Int32> maArray;
    template <typename... Args>
    CharWidthArray(Args&&... args)
        : maArray{ std::forward<Args>(args)... }
    {
    }
    template <typename Function> void InvokeWithKernArray(Function f);
    void ConvertToKernArray();
    void ConvertToCharWidths();
};

inline bool operator==(const CharWidthArray& lhs, const CharWidthArray& rhs)
{
    return lhs.maArray == rhs.maArray;
}

std::ostream& operator<<(std::ostream& rStrm, const CharWidthArray& rCharWidthArray)
{
    const std::vector<sal_Int32>& rArray(rCharWidthArray.maArray);
    sal_Int32 nLen = rArray.size();
    rStrm << "{ ";
    for (sal_Int32 i = 0; i < nLen; ++i)
    {
        rStrm << rArray[i];
        rStrm << (i < nLen - 1 ? ", " : " ");
    }
    rStrm << "}";
    return rStrm;
}

void CharWidthArray::ConvertToKernArray()
{
    for (sal_Int32 i = 1; i < sal_Int32(maArray.size()); ++i)
        maArray[i] += maArray[i - 1];
}

void CharWidthArray::ConvertToCharWidths()
{
    for (sal_Int32 i = maArray.size() - 1; i > 0; --i)
        maArray[i] -= maArray[i - 1];
}

/// Convert maArray to kern array values, then invoke the function, and convert it back.
template <typename Function> void CharWidthArray::InvokeWithKernArray(Function f)
{
    ConvertToKernArray();
    f();
    ConvertToCharWidths();
}
}

CPPUNIT_TEST_FIXTURE(SwCoreJustifyTest, testSpaceDistributionHalfSpace)
{
    // Related to: tdf#149017
    static const OUStringLiteral aText = u"ne del pro";
    CharWidthArray aActual{ 720, 639, 360, 720, 639, 400, 360, 720, 480, 720 };
    CharWidthArray aExpected{ 720, 851, 573, 720, 639, 612, 573, 720, 480, 720 };

    aActual.InvokeWithKernArray(
        [&] { Justify::SpaceDistribution(aActual.maArray, aText, 0, 10, 425, 0, false); });
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreJustifyTest, testSpaceDistributionNoHalfSpace)
{
    // Related to: tdf#149017
    static const OUStringLiteral aText = u"ne del pro";
    CharWidthArray aActual{ 720, 639, 360, 720, 639, 400, 360, 720, 480, 720 };
    CharWidthArray aExpected{ 720, 639, 785, 720, 639, 400, 785, 720, 480, 720 };

    aActual.InvokeWithKernArray(
        [&] { Justify::SpaceDistribution(aActual.maArray, aText, 0, 10, 425, 0, true); });
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreJustifyTest, testSpaceDistributionUnicodeIVS)
{
    // Related to: tdf#148594
    static const OUStringLiteral aText
        = u"\u9B54\u9AD8\u4E00\U000E01E1\u4E08\u4F55\u9B54\u9AD8\u4E00\U000E01E1";
    CharWidthArray aActual{ 1600, 1600, 1600, 0, 0, 1600, 1600, 1600, 1600, 1600, 0, 0 };
    CharWidthArray aExpected{ 1800, 1800, 1800, 0, 0, 1800, 1800, 1800, 1800, 1800, 0, 0 };
    aActual.InvokeWithKernArray(
        [&] { Justify::SpaceDistribution(aActual.maArray, aText, 0, 12, 0, 200, false); });
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
