/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <memory>
#include <docsh.hxx>

class SwCoreThemeTest : public SwModelTestBase
{
public:
    SwCoreThemeTest()
        : SwModelTestBase("/sw/qa/core/theme/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwCoreThemeTest, testThemeColorInHeading)
{
    createSwDoc("ThemeColorInHeading.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    CPPUNIT_ASSERT_EQUAL(sal_Int16(4), getProperty<sal_Int16>(getParagraph(1), "CharColorTheme"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
