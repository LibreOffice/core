/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/itempool.hxx>
#include <poolio.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

class PoolItemTest : public CppUnit::TestFixture
{
  public:
             PoolItemTest() {}
    virtual ~PoolItemTest() {}

    void testPool();

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(PoolItemTest);

    CPPUNIT_TEST(testPool);

    // End of test suite definition
    CPPUNIT_TEST_SUITE_END();
};

void PoolItemTest::testPool()
{
    SfxItemInfo aItems[] =
        { { 0, true },
          { 1, false /* not poolable */ },
          { 2, false },
          { 3, false /* not poolable */}
        };

    SfxItemPool *pPool = new SfxItemPool("testpool", 0, 3, aItems);
    SfxItemPool_Impl *pImpl = SfxItemPool_Impl::GetImpl(pPool);
    CPPUNIT_ASSERT(pImpl != nullptr);
    CPPUNIT_ASSERT(pImpl->maPoolItems.size() == 4);

    // Poolable
    SfxVoidItem aItemZero( 0 );
    SfxVoidItem aNotherZero( 0 );

    {
        CPPUNIT_ASSERT(pImpl->maPoolItems[0] == nullptr);
        const SfxPoolItem &rVal = pPool->Put(aItemZero);
        CPPUNIT_ASSERT(rVal == aItemZero);
        CPPUNIT_ASSERT(pImpl->maPoolItems[0] != nullptr);
        const SfxPoolItem &rVal2 = pPool->Put(aNotherZero);
        CPPUNIT_ASSERT(rVal2 == rVal);
        CPPUNIT_ASSERT(&rVal2 == &rVal);

        // Clones on Put ...
        CPPUNIT_ASSERT(&rVal2 != &aItemZero);
        CPPUNIT_ASSERT(&rVal2 != &aNotherZero);
        CPPUNIT_ASSERT(&rVal != &aItemZero);
        CPPUNIT_ASSERT(&rVal != &aNotherZero);
    }

    // non-poolable
    SfxVoidItem aItemOne( 1 );
    SfxVoidItem aNotherOne( 1 );
    {
        CPPUNIT_ASSERT(pImpl->maPoolItems[1] == nullptr);
        const SfxPoolItem &rVal = pPool->Put(aItemOne);
        CPPUNIT_ASSERT(rVal == aItemOne);
        CPPUNIT_ASSERT(pImpl->maPoolItems[1] != nullptr);

        const SfxPoolItem &rVal2 = pPool->Put(aNotherOne);
        CPPUNIT_ASSERT(rVal2 == rVal);
        CPPUNIT_ASSERT(&rVal2 != &rVal);
    }

    // Test rehash
    for (size_t i = 0; i < pImpl->maPoolItems.size(); ++i)
    {
        SfxPoolItemArray_Impl *pSlice = pImpl->maPoolItems[i];
        if (pSlice)
            pSlice->ReHash();
    }

    // Test removal.
    SfxVoidItem aRemoveThree(3);
    SfxVoidItem aNotherThree(3);
    const SfxPoolItem &rKeyThree = pPool->Put(aRemoveThree);
    pPool->Put(aNotherThree);
    CPPUNIT_ASSERT(pImpl->maPoolItems[3]->size() > 0);
    CPPUNIT_ASSERT(pImpl->maPoolItems[3]->maFree.size() == 0);
    pPool->Remove(rKeyThree);
    CPPUNIT_ASSERT(pImpl->maPoolItems[3]->maFree.size() == 1);
    pPool->Put(aNotherThree);
    CPPUNIT_ASSERT(pImpl->maPoolItems[3]->maFree.size() == 0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(PoolItemTest);

CPPUNIT_PLUGIN_IMPLEMENT();
