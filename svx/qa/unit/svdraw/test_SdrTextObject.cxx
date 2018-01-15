/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/svdotext.hxx>
#include <rtl/ustring.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

class SdrTextObjTest : public CppUnit::TestFixture {
public:
    void AllFamiliesCanBeRestoredFromSavedString();

    CPPUNIT_TEST_SUITE(SdrTextObjTest);
    CPPUNIT_TEST(AllFamiliesCanBeRestoredFromSavedString);
    CPPUNIT_TEST_SUITE_END();
};

void SdrTextObjTest::AllFamiliesCanBeRestoredFromSavedString() {
    std::vector<SfxStyleFamily> allFamilies;
    allFamilies.push_back(SfxStyleFamily::Char);
    allFamilies.push_back(SfxStyleFamily::Para);
    allFamilies.push_back(SfxStyleFamily::Page);
    allFamilies.push_back(SfxStyleFamily::Pseudo);

    for (SfxStyleFamily family : allFamilies) {
      OUString styleName = "styleName";
      SdrTextObj::AppendFamilyToStyleName(styleName, family);
      SfxStyleFamily readFamily = SdrTextObj::ReadFamilyFromStyleName(styleName);
      CPPUNIT_ASSERT_EQUAL(static_cast<int>(family), static_cast<int>(readFamily));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdrTextObjTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
