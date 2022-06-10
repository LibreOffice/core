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
#include <cstddef>
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
    for (std::size_t i = 1; i < maArray.size(); ++i)
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
        [&] { sw::Justify::SpaceDistribution(aActual.maArray, aText, 0, 10, 425, 0, false); });
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreJustifyTest, testSpaceDistributionNoHalfSpace)
{
    // Related to: tdf#149017
    static const OUStringLiteral aText = u"ne del pro";
    CharWidthArray aActual{ 720, 639, 360, 720, 639, 400, 360, 720, 480, 720 };
    CharWidthArray aExpected{ 720, 639, 785, 720, 639, 400, 785, 720, 480, 720 };

    aActual.InvokeWithKernArray(
        [&] { sw::Justify::SpaceDistribution(aActual.maArray, aText, 0, 10, 425, 0, true); });
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
        [&] { sw::Justify::SpaceDistribution(aActual.maArray, aText, 0, 12, 0, 200, false); });
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreJustifyTest, testSnapToGrid)
{
    tools::Long nDelta = 0;
    // "曰〈道高一尺化太平〉云云"
    static const OUStringLiteral aText
        = u"\u66f0\u3008\u9053\u9ad8\u4e00\u5c3a\u5316\u592a\u5e73\u3009\u4e91\u4e91";
    CharWidthArray aActual{ 880, 880, 880, 880, 880, 880, 880, 880, 880, 880, 880, 880 };
    CharWidthArray aExpected{
        1360, 1040, 1200, 1200, 1200, 1200, 1200, 1200, 1040, 1360, 1200, 1040
    };
    aActual.InvokeWithKernArray(
        [&] { nDelta = sw::Justify::SnapToGrid(aActual.maArray, aText, 0, 12, 400, false); });
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
    CPPUNIT_ASSERT_EQUAL(tools::Long(160), nDelta);
}

CPPUNIT_TEST_FIXTURE(SwCoreJustifyTest, testSnapToGridMixWidth)
{
    // Related to: tdf#149365
    tools::Long nDelta = 0;
    // "中中中ｹｺｻｼｽｾｿｶｹｺ" ( mixing fullwidth ideograph and half-width kana )
    static const OUStringLiteral aText
        = u"\u4e2d\u4e2d\u4e2d\uff79\uff7a\uff7b\uff7c\uff7d\uff7e\uff7f\uff76\uff79\uff7a";
    CharWidthArray aActual{ 640, 640, 640, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320 };
    CharWidthArray aExpected{ 800, 800, 760, 400, 400, 400, 400, 400, 400, 400, 400, 400, 360 };
    aActual.InvokeWithKernArray(
        [&] { nDelta = sw::Justify::SnapToGrid(aActual.maArray, aText, 0, 13, 400, false); });
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
    CPPUNIT_ASSERT_EQUAL(tools::Long(80), nDelta);
}

CPPUNIT_TEST_FIXTURE(SwCoreJustifyTest, testSnapToGridIVS)
{
    // Related to: tdf#149214
    tools::Long nDelta = 0;
    static const OUStringLiteral aText = u"\u9053\u9ad8\u4e00\U000E01E2\u5c3a\u5316";

    CharWidthArray aActual{ 800, 800, 800, 0, 0, 800, 800 };
    CharWidthArray aExpected{ 800, 800, 800, 0, 0, 800, 800 };
    aActual.InvokeWithKernArray(
        [&] { nDelta = sw::Justify::SnapToGrid(aActual.maArray, aText, 0, 7, 400, false); });
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
    CPPUNIT_ASSERT_EQUAL(tools::Long(0), nDelta);
}

CPPUNIT_TEST_FIXTURE(SwCoreJustifyTest, testSnapToGridEdge1)
{
    CharWidthArray aActual{ 640, 640, 640, 640, 640, 640, 320, 960 };
    CharWidthArray aExpected{ 840, 840, 840, 840, 840, 840, 440, 1240 };
    aActual.InvokeWithKernArray(
        [&] { sw::Justify::SnapToGridEdge(aActual.maArray, 8, 400, 40, 0); });
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreJustifyTest, testSnapToGridEdge2)
{
    CharWidthArray aActual{ 640, 640, 640, 640, 640, 640, 320, 640 };
    CharWidthArray aExpected{ 840, 840, 840, 840, 840, 840, 440, 840 };
    aActual.InvokeWithKernArray(
        [&] { sw::Justify::SnapToGridEdge(aActual.maArray, 8, 100, 40, 80); });
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreJustifyTest, testSnapToGridEdgeIVS)
{
    CharWidthArray aActual{ 640, 0, 0, 640, 640, 640, 640, 640 };
    CharWidthArray aExpected{ 840, 0, 0, 840, 840, 840, 840, 840 };
    aActual.InvokeWithKernArray(
        [&] { sw::Justify::SnapToGridEdge(aActual.maArray, 8, 400, 40, 0); });
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
