/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <test/outputdevice.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/vector/b2enums.hxx>

#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/print.hxx>
#include <vcl/rendercontext/RasterOp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <bufferdevice.hxx>
#include <window.h>

class VclGradientTest : public test::BootstrapFixture
{
public:
    VclGradientTest()
        : BootstrapFixture(true, false)
    {
    }

    void testDrawGradient_rect_linear();
    void testDrawGradient_rect_axial();

    CPPUNIT_TEST_SUITE(VclGradientTest);
    CPPUNIT_TEST(testDrawGradient_rect_linear);
    CPPUNIT_TEST(testDrawGradient_rect_axial);
    CPPUNIT_TEST_SUITE_END();
};

static void TestLinearStripes(MetaLinearGradientAction const& rAction)
{
    size_t nIndex = 0;

    MetaAction* pAction = rAction.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a comment action", MetaActionType::COMMENT,
                                 pAction->GetType());

    nIndex++;
    pAction = rAction.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                 pAction->GetType());

    nIndex++;
    pAction = rAction.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYGON,
                                 pAction->GetType());

    for (size_t i = 0; i < rAction.GetSteps() - 1; i++)
    {
        nIndex++;
        pAction = rAction.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                     pAction->GetType());

        nIndex++;
        pAction = rAction.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYGON,
                                     pAction->GetType());
    }

    nIndex++;
    pAction = rAction.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                 pAction->GetType());

    nIndex++;
    pAction = rAction.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYGON,
                                 pAction->GetType());

    nIndex++;
    pAction = rAction.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a comment action", MetaActionType::COMMENT,
                                 pAction->GetType());

    nIndex++;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("More actions to be processed", rAction.size(), nIndex);
}

void VclGradientTest::testDrawGradient_rect_linear()
{
    Gradient aGradient(GradientStyle::Linear, COL_RED, COL_WHITE);
    aGradient.SetBorder(100);

    TestLinearStripes(
        MetaLinearGradientAction(tools::Rectangle(Point(10, 10), Size(40, 40)), aGradient, 3));
}

static void TestAxialStripes(MetaLinearGradientAction const& rAction)
{
    size_t nIndex = 0;

    MetaAction* pAction = rAction.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a comment action (start)", MetaActionType::COMMENT,
                                 pAction->GetType());

    nIndex++;

    pAction = rAction.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action (start)", MetaActionType::FILLCOLOR,
                                 pAction->GetType());

    nIndex++;

    pAction = rAction.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action (start)", MetaActionType::POLYGON,
                                 pAction->GetType());

    nIndex++;

    pAction = rAction.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action (start)", MetaActionType::POLYGON,
                                 pAction->GetType());

    for (size_t i = 0; i < rAction.GetSteps(); i++)
    {
        nIndex++;
        pAction = rAction.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                     pAction->GetType());

        nIndex++;
        pAction = rAction.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYGON,
                                     pAction->GetType());
        nIndex++;
        pAction = rAction.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not an axial polygon action", MetaActionType::POLYGON,
                                     pAction->GetType());
    }

    nIndex++;

    pAction = rAction.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action (end)", MetaActionType::FILLCOLOR,
                                 pAction->GetType());
    nIndex++;

    pAction = rAction.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not an axial polygon action (end)", MetaActionType::POLYGON,
                                 pAction->GetType());

    nIndex++;

    pAction = rAction.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a comment action (end)", MetaActionType::COMMENT,
                                 pAction->GetType());

    nIndex++;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("More actions to be processed", rAction.size(), nIndex);
}

void VclGradientTest::testDrawGradient_rect_axial()
{
    Gradient aGradient(GradientStyle::Axial, COL_RED, COL_WHITE);
    aGradient.SetBorder(100);

    TestAxialStripes(
        MetaLinearGradientAction(tools::Rectangle(Point(10, 10), Size(40, 40)), aGradient, 3));
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclGradientTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
