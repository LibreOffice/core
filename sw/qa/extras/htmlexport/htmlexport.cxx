/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

class Test : public SwModelTestBase
{
public:
    void testFdo62336();
    void testCharBorder();

    CPPUNIT_TEST_SUITE(Test);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(run);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    void run();
};

void Test::run()
{
    MethodEntry<Test> aMethods[] = {
        {"fdo62336.docx", &Test::testFdo62336},
        {"charborder.odt", &Test::testCharBorder},
    };
    header();
    for (unsigned int i = 0; i < SAL_N_ELEMENTS(aMethods); ++i)
    {
        MethodEntry<Test>& rEntry = aMethods[i];
        load("/sw/qa/extras/htmlexport/data/", rEntry.pName,
             false /* not doing layout is required for this test */);
        utl::TempFile aFile;
        save("HTML", aFile);
        (this->*rEntry.pMethod)();
        finish();
    }
}

void Test::testFdo62336()
{
    // The problem was essentially a crash during table export as docx/rtf/html
}

void Test::testCharBorder()
{

    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    // Different Border
    {
        CPPUNIT_ASSERT_EQUAL_BORDER(
            table::BorderLine2(6711039,12,12,12,3,37),
            getProperty<table::BorderLine2>(xRun,"CharTopBorder"));
        CPPUNIT_ASSERT_EQUAL_BORDER(
            table::BorderLine2(16750848,0,99,0,2,99),
            getProperty<table::BorderLine2>(xRun,"CharLeftBorder"));
        CPPUNIT_ASSERT_EQUAL_BORDER(
            table::BorderLine2(16711680,0,169,0,1,169),
            getProperty<table::BorderLine2>(xRun,"CharBottomBorder"));
        CPPUNIT_ASSERT_EQUAL_BORDER(
            table::BorderLine2(255,0,169,0,0,169),
            getProperty<table::BorderLine2>(xRun,"CharRightBorder"));
    }

    // Different Padding
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(450), getProperty<sal_Int32>(xRun,"CharTopBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(550), getProperty<sal_Int32>(xRun,"CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xRun,"CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xRun,"CharRightBorderDistance"));
    }

    // No shadow
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
