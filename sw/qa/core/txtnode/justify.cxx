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
std::ostream& operator<<(std::ostream& rStrm, const std::vector<sal_Int32>& aArray)
{
    sal_Int32 nLen = aArray.size();
    rStrm << "{ ";
    for (sal_Int32 i = 0; i < nLen; ++i)
    {
        rStrm << aArray[i];
        rStrm << (i < nLen - 1 ? ", " : " ");
    }
    rStrm << "}";
    return rStrm;
}

std::vector<sal_Int32>& ToKernArray(std::vector<sal_Int32>& rArray)
{
    for (sal_Int32 i = 1; i < sal_Int32(rArray.size()); ++i)
        rArray[i] += rArray[i - 1];
    return rArray;
}

std::vector<sal_Int32>& ToCharWidths(std::vector<sal_Int32>& rArray)
{
    for (sal_Int32 i = rArray.size() - 1; i > 0; --i)
        rArray[i] -= rArray[i - 1];
    return rArray;
}

/// Covers sw/source/core/txtnode/justify fixes.
class SwCoreJustifyTest : public SwModelTestBase
{
};
}

CPPUNIT_TEST_FIXTURE(SwCoreJustifyTest, testSpaceDistributionHalfSpace)
{
    // Related to: tdf#149017
    OUString aText = u"ne del pro";
    std::vector<sal_Int32> aActual{ 720, 639, 360, 720, 639, 400, 360, 720, 480, 720 };
    std::vector<sal_Int32> aExpected{ 720, 851, 573, 720, 639, 612, 573, 720, 480, 720 };

    Justify::SpaceDistribution(ToKernArray(aActual), aText, 0, 10, 425, 0, false);
    CPPUNIT_ASSERT_EQUAL(aExpected, ToCharWidths(aActual));
}

CPPUNIT_TEST_FIXTURE(SwCoreJustifyTest, testSpaceDistributionNoHalfSpace)
{
    // Related to: tdf#149017
    OUString aText = u"ne del pro";
    std::vector<sal_Int32> aActual{ 720, 639, 360, 720, 639, 400, 360, 720, 480, 720 };
    std::vector<sal_Int32> aExpected{ 720, 639, 785, 720, 639, 400, 785, 720, 480, 720 };

    Justify::SpaceDistribution(ToKernArray(aActual), aText, 0, 10, 425, 0, true);
    CPPUNIT_ASSERT_EQUAL(aExpected, ToCharWidths(aActual));
}

CPPUNIT_TEST_FIXTURE(SwCoreJustifyTest, testSpaceDistributionUnicodeIVS)
{
    // Related to: tdf#148594
    OUString aText = u"\u9B54\u9AD8\u4E00\U000E01E1\u4E08\u4F55\u9B54\u9AD8\u4E00\U000E01E1";
    std::vector<sal_Int32> aActual{ 1600, 1600, 1600, 0, 0, 1600, 1600, 1600, 1600, 1600, 0, 0 };
    std::vector<sal_Int32> aExpected{ 1800, 1800, 1800, 0, 0, 1800, 1800, 1800, 1800, 1800, 0, 0 };
    Justify::SpaceDistribution(ToKernArray(aActual), aText, 0, 12, 0, 200, false);
    CPPUNIT_ASSERT_EQUAL(aExpected, ToCharWidths(aActual));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
