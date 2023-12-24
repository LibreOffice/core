/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <EditPaM.hxx>
#include <editdoc.hxx>

namespace
{
class EditPaMTest : public test::BootstrapFixture
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

CPPUNIT_TEST_FIXTURE(EditPaMTest, testConstruction)
{
    EditPaM aEmpty;
    CPPUNIT_ASSERT(aEmpty.GetNode() == nullptr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aEmpty.GetIndex());

    ContentNode aContentNode(*mpItemPool);
    EditPaM aNew(&aContentNode, 10);

    CPPUNIT_ASSERT(aNew.GetNode() != nullptr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aNew.GetIndex());
}

CPPUNIT_TEST_FIXTURE(EditPaMTest, testEquals)
{
    ContentNode aContentNode(*mpItemPool);

    EditPaM aEditPaMEmpty1;
    EditPaM aEditPaMEmpty2;

    CPPUNIT_ASSERT_EQUAL(true, aEditPaMEmpty1 == aEditPaMEmpty2);
    CPPUNIT_ASSERT_EQUAL(false, aEditPaMEmpty1 != aEditPaMEmpty2);

    EditPaM aEditPaM1(&aContentNode, 10);

    CPPUNIT_ASSERT_EQUAL(false, aEditPaMEmpty1 == aEditPaM1);
    CPPUNIT_ASSERT_EQUAL(true, aEditPaMEmpty1 != aEditPaM1);

    EditPaM aEditPaM2(&aContentNode, 15);
    CPPUNIT_ASSERT_EQUAL(false, aEditPaM2 == aEditPaM1);
    CPPUNIT_ASSERT_EQUAL(true, aEditPaM2 != aEditPaM1);

    EditPaM aEditPaM3(&aContentNode, 10);
    CPPUNIT_ASSERT_EQUAL(true, aEditPaM3 == aEditPaM1);
    CPPUNIT_ASSERT_EQUAL(false, aEditPaM3 != aEditPaM1);
}

} // end anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
