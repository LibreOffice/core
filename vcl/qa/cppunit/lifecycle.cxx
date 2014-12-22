/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>

#include <vcl/wrkwin.hxx>
#include <vcl/button.hxx>

class LifecycleTest : public test::BootstrapFixture
{
    void testWidgets(vcl::Window *pParent);

public:
    LifecycleTest() : BootstrapFixture(true, false) {}

    void testIsolatedWidgets();
    void testParentedWidgets();

    CPPUNIT_TEST_SUITE(LifecycleTest);
    CPPUNIT_TEST(testIsolatedWidgets);
    CPPUNIT_TEST(testParentedWidgets);
    CPPUNIT_TEST_SUITE_END();
};

void LifecycleTest::testWidgets(vcl::Window *pParent)
{
    { PushButtonPtr   aPtr(new PushButton(pParent));   }
    { OKButtonPtr     aPtr(new OKButton(pParent));     }
    { CancelButtonPtr aPtr(new CancelButton(pParent)); }
    { HelpButtonPtr   aPtr(new HelpButton(pParent));   }

    // Some widgets really insist on adoption.
    if (pParent)
    {
        { CheckBoxPtr     aPtr(new CheckBox(pParent));  }
    }
//    { RadioButtonPtr  aPtr(new RadioButton(pParent));  }
}

void LifecycleTest::testIsolatedWidgets()
{
    testWidgets(NULL);
}

void LifecycleTest::testParentedWidgets()
{
    VclReference<WorkWindow> xWin(new WorkWindow((vcl::Window *)NULL,
                                                 WB_APP|WB_STDWORK));
    CPPUNIT_ASSERT(xWin.get() != NULL);
    testWidgets(xWin.get());
}

CPPUNIT_TEST_SUITE_REGISTRATION(LifecycleTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
