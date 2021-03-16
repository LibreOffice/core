/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>


class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/odfexport/data/", "writer8") {}

    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // Only test import of .odt document
        return OString(filename).endsWith(".odt");
    }

    bool mustValidate(const char* /*filename*/) const override
    {
        return true;
    }

};

DECLARE_ODFEXPORT_TEST(testTdf52065_centerTabs, "testTdf52065_centerTabs.odt")
{
    sal_Int32 nTabStop = parseDump("//body/txt[4]/Text[3]", "nWidth").toInt32();
    // Without the fix, the text was unseen, with a tabstop width of 64057. It should be 3057
    CPPUNIT_ASSERT(nTabStop < 4000);
    CPPUNIT_ASSERT(3000 < nTabStop);
    CPPUNIT_ASSERT_EQUAL(OUString(u"Pečiatka zamestnávateľa"), parseDump("//body/txt[4]/Text[4]", "Portion"));
}

// This test started in LO 7.2. Use the odfexport.cxx if you intend to backport to 7.1.

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
