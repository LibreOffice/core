/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <vcl/gradient.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>

class VclGradientTest : public test::BootstrapFixture
{
public:
    VclGradientTest()
        : BootstrapFixture(true, false)
    {
    }

    void testAddGradientActions_rect_linear();
    void testAddGradientActions_rect_axial();
    void testAddGradientActions_rect_complex();

    CPPUNIT_TEST_SUITE(VclGradientTest);
    CPPUNIT_TEST(testAddGradientActions_rect_linear);
    CPPUNIT_TEST(testAddGradientActions_rect_axial);
    CPPUNIT_TEST(testAddGradientActions_rect_complex);
    CPPUNIT_TEST_SUITE_END();
};

static size_t TestLinearStripes(GDIMetaFile& rMtf, size_t nTimes, size_t nIndex)
{
    nIndex++;
    MetaAction* pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action (start)", MetaActionType::FILLCOLOR,
                                 pAction->GetType());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action (start)", MetaActionType::POLYGON,
                                 pAction->GetType());

    for (size_t i = 0; i < nTimes - 1; i++)
    {
        nIndex++;
        pAction = rMtf.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                     pAction->GetType());

        nIndex++;
        pAction = rMtf.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYGON,
                                     pAction->GetType());
    }

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action (end)", MetaActionType::FILLCOLOR,
                                 pAction->GetType());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action (end)", MetaActionType::POLYGON,
                                 pAction->GetType());

    return nIndex;
}

void VclGradientTest::testAddGradientActions_rect_linear()
{
    GDIMetaFile aMtf;
    tools::Rectangle aRect(Point(10, 10), Size(40, 40));
    Gradient aGradient(css::awt::GradientStyle_LINEAR, COL_RED, COL_WHITE);
    aGradient.SetBorder(100);

    aGradient.AddGradientActions(aRect, aMtf);

    size_t nIndex = 0;

    MetaAction* pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a push action", MetaActionType::PUSH, pAction->GetType());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a rectangular intersect clip action",
                                 MetaActionType::ISECTRECTCLIPREGION, pAction->GetType());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a line color action", MetaActionType::LINECOLOR,
                                 pAction->GetType());

    TestLinearStripes(aMtf, 3, nIndex);
}

static size_t TestAxialStripes(GDIMetaFile& rMtf, size_t nTimes, size_t nIndex)
{
    nIndex++;
    MetaAction* pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                 pAction->GetType());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYGON,
                                 pAction->GetType());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYGON,
                                 pAction->GetType());

    for (size_t i = 0; i < nTimes - 1; i++)
    {
        nIndex++;
        pAction = rMtf.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                     pAction->GetType());

        nIndex++;
        pAction = rMtf.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYGON,
                                     pAction->GetType());

        nIndex++;
        pAction = rMtf.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYGON,
                                     pAction->GetType());
    }

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                 pAction->GetType());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYGON,
                                 pAction->GetType());

    return nIndex;
}

void VclGradientTest::testAddGradientActions_rect_axial()
{
    GDIMetaFile aMtf;
    tools::Rectangle aRect(Point(10, 10), Size(40, 40));
    Gradient aGradient(css::awt::GradientStyle_AXIAL, COL_RED, COL_WHITE);
    aGradient.SetBorder(100);

    aGradient.AddGradientActions(aRect, aMtf);

    size_t nIndex = 0;

    MetaAction* pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a push action", MetaActionType::PUSH, pAction->GetType());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a rectangular intersect clip action",
                                 MetaActionType::ISECTRECTCLIPREGION, pAction->GetType());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a line color action", MetaActionType::LINECOLOR,
                                 pAction->GetType());

    TestAxialStripes(aMtf, 3, nIndex);
}

static size_t TestComplexStripes(GDIMetaFile& rMtf, size_t nTimes, size_t nIndex)
{
    nIndex++;
    MetaAction* pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                 pAction->GetType());

    for (size_t i = 1; i < nTimes; i++)
    {
        nIndex++;
        pAction = rMtf.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polypolygon action", MetaActionType::POLYPOLYGON,
                                     pAction->GetType());

        nIndex++;
        pAction = rMtf.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                     pAction->GetType());
    }

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                 pAction->GetType());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polypolygon action", MetaActionType::POLYGON,
                                 pAction->GetType());

    return nIndex;
}

void VclGradientTest::testAddGradientActions_rect_complex()
{
    GDIMetaFile aMtf;
    tools::Rectangle aRect(Point(10, 10), Size(40, 40));
    Gradient aGradient(css::awt::GradientStyle_SQUARE, COL_RED, COL_WHITE);
    aGradient.SetBorder(10);

    aGradient.AddGradientActions(aRect, aMtf);

    size_t nIndex = 0;

    MetaAction* pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a push action", MetaActionType::PUSH, pAction->GetType());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a rectangular intersect clip action",
                                 MetaActionType::ISECTRECTCLIPREGION, pAction->GetType());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a line color action", MetaActionType::LINECOLOR,
                                 pAction->GetType());

    TestComplexStripes(aMtf, 40, nIndex);
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclGradientTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
