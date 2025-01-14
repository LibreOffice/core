/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <svx/framelinkarray.hxx>
#include <svx/framelink.hxx>

using namespace com::sun::star;

class FrameLinkArrayTest : public UnoApiTest
{
public:
    FrameLinkArrayTest()
        : UnoApiTest(u"svx/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(FrameLinkArrayTest, testSingleCell)
{
    svx::frame::Array stArr;
    stArr.Initialize(10, 10);

    stArr.SetCellStyleLeft(/*col*/ 2, /*row*/ 4,
                           svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::ENGRAVED, 1.0 });
    stArr.SetCellStyleRight(/*col*/ 2, /*row*/ 4,
                            svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::DOTTED, 1.0 });
    stArr.SetCellStyleTop(/*col*/ 2, /*row*/ 4,
                          svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::DASHED, 1.0 });
    stArr.SetCellStyleBottom(/*col*/ 2, /*row*/ 4,
                             svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::DOUBLE, 1.0 });

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::ENGRAVED, stArr.GetCellStyleLeft(2, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, stArr.GetCellStyleRight(2, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASHED, stArr.GetCellStyleTop(2, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOUBLE, stArr.GetCellStyleBottom(2, 4).Type());
}

CPPUNIT_TEST_FIXTURE(FrameLinkArrayTest, testSingleCellRtl)
{
    svx::frame::Array stArr;
    stArr.Initialize(10, 10);

    stArr.SetCellStyleLeft(/*col*/ 2, /*row*/ 4,
                           svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::ENGRAVED, 1.0 });
    stArr.SetCellStyleRight(/*col*/ 2, /*row*/ 4,
                            svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::DOTTED, 1.0 });
    stArr.SetCellStyleTop(/*col*/ 2, /*row*/ 4,
                          svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::DASHED, 1.0 });
    stArr.SetCellStyleBottom(/*col*/ 2, /*row*/ 4,
                             svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::DOUBLE, 1.0 });

    stArr.MirrorSelfX();

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleLeft(2, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleRight(2, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleTop(2, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleBottom(2, 4).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, stArr.GetCellStyleLeft(7, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::ENGRAVED, stArr.GetCellStyleRight(7, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASHED, stArr.GetCellStyleTop(7, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOUBLE, stArr.GetCellStyleBottom(7, 4).Type());
}

CPPUNIT_TEST_FIXTURE(FrameLinkArrayTest, testMergedCell)
{
    svx::frame::Array stArr;
    stArr.Initialize(10, 10);

    stArr.SetCellStyleLeft(/*col*/ 2, /*row*/ 4,
                           svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::ENGRAVED, 1.0 });
    stArr.SetCellStyleRight(/*col*/ 2, /*row*/ 4,
                            svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::DOTTED, 1.0 });
    stArr.SetCellStyleTop(/*col*/ 2, /*row*/ 4,
                          svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::DASHED, 1.0 });
    stArr.SetCellStyleBottom(/*col*/ 2, /*row*/ 4,
                             svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::DOUBLE, 1.0 });
    stArr.SetMergedRange(/*first col*/ 2, /*first row*/ 4, /*last col*/ 4, /*last row*/ 6);

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::ENGRAVED, stArr.GetCellStyleLeft(2, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleRight(2, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASHED, stArr.GetCellStyleTop(2, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleBottom(2, 4).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleLeft(3, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleRight(3, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASHED, stArr.GetCellStyleTop(3, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleBottom(3, 4).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleLeft(4, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, stArr.GetCellStyleRight(4, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASHED, stArr.GetCellStyleTop(4, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleBottom(4, 4).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::ENGRAVED, stArr.GetCellStyleLeft(2, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleRight(2, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleTop(2, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleBottom(2, 5).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleLeft(3, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleRight(3, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleTop(3, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleBottom(3, 5).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleLeft(4, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, stArr.GetCellStyleRight(4, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleTop(4, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleBottom(4, 5).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::ENGRAVED, stArr.GetCellStyleLeft(2, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleRight(2, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleTop(2, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOUBLE, stArr.GetCellStyleBottom(2, 6).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleLeft(3, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleRight(3, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleTop(3, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOUBLE, stArr.GetCellStyleBottom(3, 6).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleLeft(4, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, stArr.GetCellStyleRight(4, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleTop(4, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOUBLE, stArr.GetCellStyleBottom(4, 6).Type());
}

CPPUNIT_TEST_FIXTURE(FrameLinkArrayTest, testMergedCellRtl)
{
    svx::frame::Array stArr;
    stArr.Initialize(10, 10);

    stArr.SetCellStyleLeft(/*col*/ 2, /*row*/ 4,
                           svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::ENGRAVED, 1.0 });
    stArr.SetCellStyleRight(/*col*/ 2, /*row*/ 4,
                            svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::DOTTED, 1.0 });
    stArr.SetCellStyleTop(/*col*/ 2, /*row*/ 4,
                          svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::DASHED, 1.0 });
    stArr.SetCellStyleBottom(/*col*/ 2, /*row*/ 4,
                             svx::frame::Style{ 1.0, 1.0, 1.0, SvxBorderLineStyle::DOUBLE, 1.0 });
    stArr.SetMergedRange(/*first col*/ 2, /*first row*/ 4, /*last col*/ 4, /*last row*/ 6);
    stArr.MirrorSelfX();

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, stArr.GetCellStyleLeft(5, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleRight(5, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASHED, stArr.GetCellStyleTop(5, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleBottom(5, 4).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleLeft(6, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleRight(6, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASHED, stArr.GetCellStyleTop(6, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleBottom(6, 4).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleLeft(7, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::ENGRAVED, stArr.GetCellStyleRight(7, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASHED, stArr.GetCellStyleTop(7, 4).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleBottom(7, 4).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, stArr.GetCellStyleLeft(5, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleRight(5, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleTop(5, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleBottom(5, 5).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleLeft(6, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleRight(6, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleTop(6, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleBottom(6, 5).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleLeft(7, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::ENGRAVED, stArr.GetCellStyleRight(7, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleTop(7, 5).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleBottom(7, 5).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, stArr.GetCellStyleLeft(5, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleRight(5, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleTop(5, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOUBLE, stArr.GetCellStyleBottom(5, 6).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleLeft(6, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleRight(6, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleTop(6, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOUBLE, stArr.GetCellStyleBottom(6, 6).Type());

    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleLeft(7, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::ENGRAVED, stArr.GetCellStyleRight(7, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, stArr.GetCellStyleTop(7, 6).Type());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOUBLE, stArr.GetCellStyleBottom(7, 6).Type());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
