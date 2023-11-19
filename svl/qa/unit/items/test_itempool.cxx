/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/itempool.hxx>
#include <svl/voiditem.hxx>
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

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(PoolItemTest);

    CPPUNIT_TEST(testPool);

    // End of test suite definition
    CPPUNIT_TEST_SUITE_END();
};

void PoolItemTest::testPool()
{
    SfxItemInfo const aItems[] =
    {
        // _nSID, _bNeedsPoolRegistration, _bShareable
        { 4, true,  true },
        { 3, true,  false /* test NeedsPoolRegistration */ },
        { 2, false, false },
        { 1, true,  false /* test NeedsPoolRegistration */}
    };

    rtl::Reference<SfxItemPool> pPool = new SfxItemPool("testpool", 1, 4, aItems);

    // Poolable
    SfxVoidItem aItemOne( 1 );
    SfxVoidItem aNotherOne( 1 );

    {
        CPPUNIT_ASSERT(nullptr == pPool->ppRegisteredSfxPoolItems);
        const SfxPoolItem &rVal = pPool->DirectPutItemInPool(aItemOne);
        CPPUNIT_ASSERT(bool(rVal == aItemOne));
        CPPUNIT_ASSERT(nullptr != pPool->ppRegisteredSfxPoolItems);
        CPPUNIT_ASSERT(nullptr != pPool->ppRegisteredSfxPoolItems[0]);
        CPPUNIT_ASSERT(!pPool->ppRegisteredSfxPoolItems[0]->empty());
        const SfxPoolItem &rVal2 = pPool->DirectPutItemInPool(aNotherOne);
        CPPUNIT_ASSERT(bool(rVal2 == rVal));

        // ITEM: With leaving the paradigm that internally an already
        //   existing Item with true = operator==() (which is very
        //   expensive) the ptr's are no longer required to be equal,
        //   but the content-compare *is*
        CPPUNIT_ASSERT(SfxPoolItem::areSame(rVal, rVal2));

        // Clones on Put ...
        // ptr compare OK, we want to check just the ptrs here
        CPPUNIT_ASSERT(!areSfxPoolItemPtrsEqual(&rVal2, &aItemOne));
        CPPUNIT_ASSERT(!areSfxPoolItemPtrsEqual(&rVal2, &aNotherOne));
        CPPUNIT_ASSERT(!areSfxPoolItemPtrsEqual(&rVal, &aItemOne));
        CPPUNIT_ASSERT(!areSfxPoolItemPtrsEqual(&rVal, &aNotherOne));
    }

    // non-poolable
    SfxVoidItem aItemTwo( 2 );
    SfxVoidItem aNotherTwo( 2 );
    {
        CPPUNIT_ASSERT(nullptr == pPool->ppRegisteredSfxPoolItems[1]);
        const SfxPoolItem &rVal = pPool->DirectPutItemInPool(aItemTwo);
        CPPUNIT_ASSERT(bool(rVal == aItemTwo));
        CPPUNIT_ASSERT(nullptr != pPool->ppRegisteredSfxPoolItems[1]);
        CPPUNIT_ASSERT(!pPool->ppRegisteredSfxPoolItems[1]->empty());
        const SfxPoolItem &rVal2 = pPool->DirectPutItemInPool(aNotherTwo);
        CPPUNIT_ASSERT(bool(rVal2 == rVal));
        // ptr compare OK, we want to check just the ptrs here
        CPPUNIT_ASSERT(!areSfxPoolItemPtrsEqual(&rVal2, &rVal));
    }

    // Test removal.
    SfxVoidItem aRemoveFour(4);
    SfxVoidItem aNotherFour(4);

    const SfxPoolItem &rKeyFour = pPool->DirectPutItemInPool(aRemoveFour);
    pPool->DirectPutItemInPool(aNotherFour);
    CPPUNIT_ASSERT(pPool->ppRegisteredSfxPoolItems[3]->size() > 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pPool->ppRegisteredSfxPoolItems[3]->size());
    pPool->DirectRemoveItemFromPool(rKeyFour);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPool->ppRegisteredSfxPoolItems[3]->size());
    pPool->DirectPutItemInPool(aNotherFour);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pPool->ppRegisteredSfxPoolItems[3]->size());
}


CPPUNIT_TEST_SUITE_REGISTRATION(PoolItemTest);

CPPUNIT_PLUGIN_IMPLEMENT();
