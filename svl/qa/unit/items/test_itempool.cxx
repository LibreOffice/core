/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/itemiter.hxx>
#include <poolio.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

class PoolItemTest : public CppUnit::TestFixture
{
  public:
             PoolItemTest() {}

    void testPool();
    void testItemSet();

    CPPUNIT_TEST_SUITE(PoolItemTest);
    CPPUNIT_TEST(testPool);
    CPPUNIT_TEST(testItemSet);
    CPPUNIT_TEST_SUITE_END();
};

void PoolItemTest::testPool()
{
    SfxItemInfo aItems[] =
        { { 1, true },
          { 2, false /* not poolable */ },
          { 3, false },
          { 4, false /* not poolable */}
        };

    SfxItemPool *pPool = new SfxItemPool("testpool", 1, 4, aItems);
    SfxItemPool_Impl *pImpl = SfxItemPool_Impl::GetImpl(pPool);
    CPPUNIT_ASSERT(pImpl != nullptr);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pImpl->maPoolItems.size());

    // Poolable
    SfxVoidItem aItemOne( 1 );
    SfxVoidItem aNotherOne( 1 );

    {
        CPPUNIT_ASSERT(!pImpl->maPoolItems[0]);
        const SfxPoolItem &rVal = pPool->Put(aItemOne);
        CPPUNIT_ASSERT(bool(rVal == aItemOne));
        CPPUNIT_ASSERT(pImpl->maPoolItems[0] != nullptr);
        const SfxPoolItem &rVal2 = pPool->Put(aNotherOne);
        CPPUNIT_ASSERT(bool(rVal2 == rVal));
        CPPUNIT_ASSERT_EQUAL(&rVal, &rVal2);

        // Clones on Put ...
        CPPUNIT_ASSERT(&rVal2 != &aItemOne);
        CPPUNIT_ASSERT(&rVal2 != &aNotherOne);
        CPPUNIT_ASSERT(&rVal != &aItemOne);
        CPPUNIT_ASSERT(&rVal != &aNotherOne);
    }

    // non-poolable
    SfxVoidItem aItemTwo( 2 );
    SfxVoidItem aNotherTwo( 2 );
    {
        CPPUNIT_ASSERT(!pImpl->maPoolItems[1]);
        const SfxPoolItem &rVal = pPool->Put(aItemTwo);
        CPPUNIT_ASSERT(bool(rVal == aItemTwo));
        CPPUNIT_ASSERT(pImpl->maPoolItems[1] != nullptr);

        const SfxPoolItem &rVal2 = pPool->Put(aNotherTwo);
        CPPUNIT_ASSERT(bool(rVal2 == rVal));
        CPPUNIT_ASSERT(&rVal2 != &rVal);
    }

    // Test rehash
    for (SfxPoolItemArray_Impl *pSlice : pImpl->maPoolItems)
    {
        if (pSlice)
            pSlice->ReHash();
    }

    // Test removal.
    SfxVoidItem aRemoveFour(4);
    SfxVoidItem aNotherFour(4);
    const SfxPoolItem &rKeyFour = pPool->Put(aRemoveFour);
    pPool->Put(aNotherFour);
    CPPUNIT_ASSERT(pImpl->maPoolItems[3]->size() > 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pImpl->maPoolItems[3]->maFree.size());
    pPool->Remove(rKeyFour);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pImpl->maPoolItems[3]->maFree.size());
    pPool->Put(aNotherFour);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pImpl->maPoolItems[3]->maFree.size());
}

void PoolItemTest::testItemSet()
{
    SfxItemInfo aItems[] =
        { { 1, false },
          { 2, false },
          { 3, false },
          { 4, false },
          { 5, false },
          { 6, false },
          { 7, false }
        };

    SfxItemPool *pPool = new SfxItemPool("testpool", 1, 7, aItems);
    std::vector<SfxPoolItem*> aDefaults {
        new SfxVoidItem(1),
        new SfxVoidItem(2),
        new SfxVoidItem(3),
        new SfxVoidItem(4),
        new SfxVoidItem(5),
        new SfxVoidItem(6),
        new SfxVoidItem(7)
    };
    pPool->SetDefaults(&aDefaults);

    SfxItemSet aItemSet(*pPool, 1, 3, 5, 7, 0);
    aItemSet.Put(SfxVoidItem(1));
    aItemSet.Put(SfxVoidItem(2));
    aItemSet.Put(SfxVoidItem(3));
    aItemSet.Put(SfxVoidItem(5));
    aItemSet.Put(SfxVoidItem(6));
    aItemSet.Put(SfxVoidItem(7));

    SfxItemIter aIter(aItemSet);

    CPPUNIT_ASSERT_EQUAL((sal_uInt16)1, aIter.GetFirstWhich());
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)7, aIter.GetLastWhich());
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)1, aIter.FirstItem()->Which());
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)2, aIter.NextItem()->Which());
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)3, aIter.NextItem()->Which());
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)5, aIter.NextItem()->Which());
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)6, aIter.NextItem()->Which());
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)7, aIter.NextItem()->Which());
    CPPUNIT_ASSERT_EQUAL(static_cast<const SfxPoolItem*>(nullptr), aIter.NextItem());
    CPPUNIT_ASSERT_EQUAL(true, aIter.IsAtEnd());
}

CPPUNIT_TEST_SUITE_REGISTRATION(PoolItemTest);

CPPUNIT_PLUGIN_IMPLEMENT();
