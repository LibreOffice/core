/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/bootstrapfixturebase.hxx>

#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/stylepool.hxx>
#include <svl/stritem.hxx>

namespace
{
/// Tests svl StylePool.
class StylePoolTest : public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(StylePoolTest, testIterationOrder)
{
    // Set up a style pool with multiple parents.
    SfxStringItem aDefault1(1);
    std::vector<SfxPoolItem*> aDefaults{ &aDefault1 };
    SfxItemInfo const aItems[] = { { 1, false } };

    SfxItemPool* pPool = new SfxItemPool("test", 1, 1, aItems);
    pPool->SetDefaults(&aDefaults);
    {
        // Set up parents in mixed order to make sure we do not sort by pointer address.
        SfxItemSet aParent1(*pPool, svl::Items<1, 1>{});
        SfxItemSet aChild1(*pPool, svl::Items<1, 1>{});
        aChild1.SetParent(&aParent1);
        SfxStringItem aItem1(1, "Item1");
        aChild1.Put(aItem1);

        SfxItemSet aParent3(*pPool, svl::Items<1, 1>{});
        SfxItemSet aChild3(*pPool, svl::Items<1, 1>{});
        aChild3.SetParent(&aParent3);
        SfxStringItem aItem3(1, "Item3");
        aChild3.Put(aItem3);

        SfxItemSet aParent2(*pPool, svl::Items<1, 1>{});
        SfxItemSet aChild2(*pPool, svl::Items<1, 1>{});
        aChild2.SetParent(&aParent2);
        SfxStringItem aItem2(1, "Item2");
        aChild2.Put(aItem2);

        // Insert item sets in alphabetical order.
        StylePool aStylePool;
        OUString aChild1Name("Child1");
        aStylePool.insertItemSet(aChild1, &aChild1Name);
        OUString aChild3Name("Child3");
        aStylePool.insertItemSet(aChild3, &aChild3Name);
        OUString aChild2Name("Child2");
        aStylePool.insertItemSet(aChild2, &aChild2Name);
        std::unique_ptr<IStylePoolIteratorAccess> pIter = aStylePool.createIterator();
        std::shared_ptr<SfxItemSet> pStyle1 = pIter->getNext();
        CPPUNIT_ASSERT(pStyle1);
        const SfxStringItem* pItem1 = static_cast<const SfxStringItem*>(pStyle1->GetItem(1));
        CPPUNIT_ASSERT_EQUAL(OUString("Item1"), pItem1->GetValue());
        std::shared_ptr<SfxItemSet> pStyle2 = pIter->getNext();
        CPPUNIT_ASSERT(pStyle2);
        const SfxStringItem* pItem2 = static_cast<const SfxStringItem*>(pStyle2->GetItem(1));
        // Without the accompanying fix in place, this test would have failed with 'Expected: Item2;
        // Actual: Item3'. The iteration order depended on the pointer address on the pointer
        // address of the parents.
        CPPUNIT_ASSERT_EQUAL(OUString("Item2"), pItem2->GetValue());
        std::shared_ptr<SfxItemSet> pStyle3 = pIter->getNext();
        CPPUNIT_ASSERT(pStyle3);
        const SfxStringItem* pItem3 = static_cast<const SfxStringItem*>(pStyle3->GetItem(1));
        CPPUNIT_ASSERT_EQUAL(OUString("Item3"), pItem3->GetValue());
        CPPUNIT_ASSERT(!pIter->getNext());
    }
    SfxItemPool::Free(pPool);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
