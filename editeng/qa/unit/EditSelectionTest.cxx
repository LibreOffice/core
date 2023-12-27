/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <EditSelection.hxx>
#include <editdoc.hxx>

namespace
{
class EditSelectionTest : public test::BootstrapFixture
{
protected:
    rtl::Reference<EditEngineItemPool> mpItemPool;

public:
    void setUp() override
    {
        test::BootstrapFixture::setUp();
        mpItemPool = new EditEngineItemPool();
    }

    void tearDown() override
    {
        mpItemPool.clear();
        test::BootstrapFixture::tearDown();
    }
};

CPPUNIT_TEST_FIXTURE(EditSelectionTest, testConstruction)
{
    // Check empty selections
    EditSelection aEmpty;
    CPPUNIT_ASSERT(aEmpty.Min().GetNode() == nullptr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aEmpty.Min().GetIndex());

    CPPUNIT_ASSERT(aEmpty.Max().GetNode() == nullptr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aEmpty.Max().GetIndex());

    // Create content nodes
    ContentNode aContentNode1(*mpItemPool);
    ContentNode* pContentNode1 = &aContentNode1;
    ContentNode aContentNode2(*mpItemPool);
    ContentNode* pContentNode2 = &aContentNode2;

    // Check selection with (node1 10, node1 20)
    {
        EditSelection aNew(EditPaM(&aContentNode1, 10), EditPaM(&aContentNode1, 20));

        CPPUNIT_ASSERT_EQUAL(true, aNew.Min().GetNode() == pContentNode1);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aNew.Min().GetIndex());

        CPPUNIT_ASSERT_EQUAL(true, aNew.Max().GetNode() == pContentNode1);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(20), aNew.Max().GetIndex());
    }

    // Check selection with (node1 10, node2 10)
    {
        EditSelection aNew(EditPaM(&aContentNode1, 10), EditPaM(&aContentNode2, 10));

        CPPUNIT_ASSERT_EQUAL(true, aNew.Min().GetNode() == pContentNode1);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aNew.Min().GetIndex());

        CPPUNIT_ASSERT_EQUAL(true, aNew.Max().GetNode() == pContentNode2);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aNew.Max().GetIndex());
    }
}

CPPUNIT_TEST_FIXTURE(EditSelectionTest, testEquals)
{
    // Check empty equality
    EditSelection aEditSelectionEmpty1;
    EditSelection aEditSelectionEmpty2;

    CPPUNIT_ASSERT_EQUAL(true, aEditSelectionEmpty1 == aEditSelectionEmpty2);
    CPPUNIT_ASSERT_EQUAL(false, aEditSelectionEmpty1 != aEditSelectionEmpty2);

    // Check equal, in-equal
    ContentNode aContentNode(*mpItemPool);

    EditSelection aEditSelection1(EditPaM(&aContentNode, 10), EditPaM(&aContentNode, 20));

    CPPUNIT_ASSERT_EQUAL(false, aEditSelectionEmpty1 == aEditSelection1);
    CPPUNIT_ASSERT_EQUAL(true, aEditSelectionEmpty1 != aEditSelection1);

    EditSelection aEditSelection2(EditPaM(&aContentNode, 15), EditPaM(&aContentNode, 20));
    CPPUNIT_ASSERT_EQUAL(false, aEditSelection1 == aEditSelection2);
    CPPUNIT_ASSERT_EQUAL(true, aEditSelection1 != aEditSelection2);

    EditSelection aEditSelection3(EditPaM(&aContentNode, 10), EditPaM(&aContentNode, 20));
    CPPUNIT_ASSERT_EQUAL(true, aEditSelection1 == aEditSelection3);
    CPPUNIT_ASSERT_EQUAL(false, aEditSelection1 != aEditSelection3);
}

CPPUNIT_TEST_FIXTURE(EditSelectionTest, testAssign)
{
    ContentNode aContentNode(*mpItemPool);
    ContentNode* pContentNode = &aContentNode;

    EditSelection aEditSelection1(EditPaM(&aContentNode, 20), EditPaM(&aContentNode, 20));
    EditSelection aEditSelection2;

    // Check selection1 == selection2 (empty)
    CPPUNIT_ASSERT_EQUAL(false, aEditSelection1 == aEditSelection2);
    CPPUNIT_ASSERT_EQUAL(true, aEditSelection1 != aEditSelection2);

    // Assign with selection2 with (15, 20)
    aEditSelection2 = EditSelection(EditPaM(pContentNode, 15), EditPaM(pContentNode, 20));

    // Check if it matches aEditSelection1 (20, 20) -> no
    CPPUNIT_ASSERT_EQUAL(false, aEditSelection1 == aEditSelection2);
    CPPUNIT_ASSERT_EQUAL(true, aEditSelection1 != aEditSelection2);

    // Check if selection2 vales are set correctly
    CPPUNIT_ASSERT_EQUAL(true, aEditSelection2.Min().GetNode() == pContentNode);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(15), aEditSelection2.Min().GetIndex());

    CPPUNIT_ASSERT_EQUAL(true, aEditSelection2.Max().GetNode() == pContentNode);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), aEditSelection2.Max().GetIndex());

    // Assign EditPaM value (20)
    aEditSelection2 = EditPaM(pContentNode, 20);

    // Check if it matches aEditSelection1 (20, 20) -> yes
    CPPUNIT_ASSERT_EQUAL(true, aEditSelection1 == aEditSelection2);
    CPPUNIT_ASSERT_EQUAL(false, aEditSelection1 != aEditSelection2);

    // Check if selection2 vales are set correctly - expect selection min and max == the EditPaM (20)
    CPPUNIT_ASSERT_EQUAL(true, aEditSelection2.Min().GetNode() == pContentNode);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), aEditSelection2.Min().GetIndex());

    CPPUNIT_ASSERT_EQUAL(true, aEditSelection2.Max().GetNode() == pContentNode);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), aEditSelection2.Max().GetIndex());
}

} // end anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
