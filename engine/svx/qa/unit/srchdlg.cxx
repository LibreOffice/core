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
#include <svl/itemset.hxx>
#include <svx/srchdlg.hxx>
#include <svx/svxids.hrc>
#include <optional>
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

// A copy may outlive the document whose pool supplied its items, so every
// holder it receives must be bound to the copy's own pool.
CPPUNIT_TEST_FIXTURE(SearchAttrItemListTest, testCopiedListBindsInsertedItem)
{
    rtl::Reference<TestPool> xSource(new TestPool);
    SfxBoolItem aItem(SID_ATTR_CHAR_WEIGHT, true);
    SearchAttrItemList aOriginal;
    aOriginal.Insert(SearchAttrInfo(SID_ATTR_CHAR_WEIGHT, SfxPoolItemHolder(*xSource, &aItem)));
    SearchAttrItemList aCopy(aOriginal);
    aCopy.Insert(SearchAttrInfo(SID_ATTR_CHAR_WEIGHT, SfxPoolItemHolder(*xSource, &aItem)));
    CPPUNIT_ASSERT_EQUAL(&aCopy[0].aItemPtr.getPool(), &aCopy[1].aItemPtr.getPool());
}

CPPUNIT_TEST_FIXTURE(SearchAttrItemListTest, testCopiedListBindsPutItem)
{
    rtl::Reference<TestPool> xSource(new TestPool);
    SfxBoolItem aItem(SID_ATTR_CHAR_WEIGHT, true);
    SearchAttrItemList aOriginal;
    aOriginal.Insert(SearchAttrInfo(SID_ATTR_CHAR_WEIGHT, SfxPoolItemHolder(*xSource, &aItem)));
    SearchAttrItemList aCopy(aOriginal);
    SfxItemSet aSet(*xSource, svl::Items<SID_ATTR_CHAR_WEIGHT, SID_ATTR_CHAR_WEIGHT>);
    aSet.Put(aItem);
    aCopy.Put(aSet);
    CPPUNIT_ASSERT_EQUAL(&aCopy[0].aItemPtr.getPool(), &aCopy[1].aItemPtr.getPool());
}

CPPUNIT_TEST_FIXTURE(SearchAttrItemListTest, testEmptyCopiedListBindsInsertedItem)
{
    rtl::Reference<TestPool> xSource(new TestPool);
    SfxBoolItem aItem(SID_ATTR_CHAR_WEIGHT, true);
    const SearchAttrItemList aEmpty;
    SearchAttrItemList aCopy(aEmpty);
    aCopy.Insert(SearchAttrInfo(SID_ATTR_CHAR_WEIGHT, SfxPoolItemHolder(*xSource, &aItem)));
    CPPUNIT_ASSERT(&aCopy[0].aItemPtr.getPool() != xSource.get());
}

CPPUNIT_TEST_FIXTURE(SearchAttrItemListTest, testCopiedListBindsReplacedItem)
{
    rtl::Reference<TestPool> xSource(new TestPool);
    SfxBoolItem aItem(SID_ATTR_CHAR_WEIGHT, true);
    SearchAttrItemList aOriginal;
    aOriginal.Insert(SearchAttrInfo(SID_ATTR_CHAR_WEIGHT, SfxPoolItemHolder(*xSource, &aItem)));
    SearchAttrItemList aCopy(aOriginal);
    SfxBoolItem aReplacement(SID_ATTR_CHAR_WEIGHT, false);
    aCopy.SetItem(0, SfxPoolItemHolder(*xSource, &aReplacement));
    CPPUNIT_ASSERT(&aCopy[0].aItemPtr.getPool() != xSource.get());
    CPPUNIT_ASSERT(!static_cast<const SfxBoolItem*>(aCopy[0].aItemPtr.getItem())->GetValue());
}

CPPUNIT_TEST_FIXTURE(SearchAttrItemListTest, testUncopiedListBorrowsPool)
{
    rtl::Reference<TestPool> xSource(new TestPool);
    SfxBoolItem aItem(SID_ATTR_CHAR_WEIGHT, true);
    SearchAttrItemList aList;
    aList.Insert(SearchAttrInfo(SID_ATTR_CHAR_WEIGHT, SfxPoolItemHolder(*xSource, &aItem)));
    CPPUNIT_ASSERT_EQUAL(static_cast<SfxItemPool*>(xSource.get()), &aList[0].aItemPtr.getPool());
}

// A remembered copy may outlive the document whose pool supplied its items.
// Destroying it afterwards must not touch that pool. This shows up as an
// invalid read under valgrind or ASan rather than as an assertion.
CPPUNIT_TEST_FIXTURE(SearchAttrItemListTest, testCopiedListOutlivesSourcePool)
{
    std::optional<SearchAttrItemList> oCopy;
    {
        rtl::Reference<TestPool> xSource(new TestPool);
        SfxBoolItem aItem(SID_ATTR_CHAR_WEIGHT, true);
        SearchAttrItemList aOriginal;
        aOriginal.Insert(SearchAttrInfo(SID_ATTR_CHAR_WEIGHT, SfxPoolItemHolder(*xSource, &aItem)));
        oCopy.emplace(aOriginal);
        oCopy->Insert(SearchAttrInfo(SID_ATTR_CHAR_WEIGHT, SfxPoolItemHolder(*xSource, &aItem)));
    }
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), oCopy->Count());
    oCopy.reset();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
