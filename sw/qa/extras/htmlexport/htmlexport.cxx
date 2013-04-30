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

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
