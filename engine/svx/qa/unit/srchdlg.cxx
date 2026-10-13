/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <svl/eitem.hxx>
#include <svl/itempool.hxx>
#include <svx/srchdlg.hxx>
#include <svx/svxids.hrc>
#include <utility>
#include <vector>

namespace
{
class SearchAttrItemListTest : public CppUnit::TestFixture
{
};

class TestPool final : public SfxItemPool
{
public:
    TestPool()
        : SfxItemPool(u"SearchAttributeTest"_ustr)
    {
    }
    ~TestPool() override { SetSecondaryPool(nullptr); }
};

CPPUNIT_TEST_FIXTURE(SearchAttrItemListTest, testCopiedSearchPoolChainIsDetached)
{
    for (const int nPools : { 1, 2, 3 })
    {
        for (const bool bMove : { false, true })
        {
            rtl::Reference<TestPool> xSource(new TestPool);
            SfxItemPool* pLast = xSource.get();
            for (int i = 1; i < nPools; ++i)
            {
                pLast->SetSecondaryPool(new TestPool);
                pLast = pLast->GetSecondaryPool();
            }

            SfxBoolItem aItem(SID_ATTR_CHAR_WEIGHT, true);
            SearchAttrItemList aOriginal;
            aOriginal.Insert(
                SearchAttrInfo(SID_ATTR_CHAR_WEIGHT, SfxPoolItemHolder(*xSource, &aItem)));
            std::vector<rtl::Reference<SfxItemPool>> aClonedPools;
            {
                SearchAttrItemList aCopy(aOriginal);
                for (SfxItemPool* pPool = &aCopy[0].aItemPtr.getPool(); pPool;
                     pPool = pPool->GetSecondaryPool())
                    aClonedPools.emplace_back(pPool);
                if (bMove)
                {
                    SearchAttrItemList aMoved(std::move(aCopy));
                    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), aMoved.Count());
                    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), aCopy.Count());
                }
            }

            // External references let us inspect the cloned chain after list destruction.
            bool bDetached = true;
            for (const auto& xPool : aClonedPools)
                bDetached &= xPool->GetMasterPool() == xPool.get() && !xPool->GetSecondaryPool();
            // Keep cleanup safe even when the assertion below fails on the old implementation.
            for (const auto& xPool : aClonedPools)
                xPool->SetSecondaryPool(nullptr);
            CPPUNIT_ASSERT_MESSAGE("destroyed search lists must detach their cloned pool chain",
                                   bDetached);
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
