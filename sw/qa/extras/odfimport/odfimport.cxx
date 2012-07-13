/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@suse.cz> (SUSE, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "../swmodeltestbase.hxx"

using rtl::OUString;

class Test : public SwModelTestBase
{
public:
    void testHello();
    void testEmptySvgFamilyName();

    CPPUNIT_TEST_SUITE(Test);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(testHello);
    CPPUNIT_TEST(testEmptySvgFamilyName);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    /// Load an ODF file and make the document available via mxComponent.
    void load(const OUString& rURL);
};

void Test::load(const OUString& rFilename)
{
    mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/odfimport/data/") + rFilename);
}

void Test::testHello()
{
    load("hello.odt");
    CPPUNIT_ASSERT_EQUAL(12, getLength());
}

void Test::testEmptySvgFamilyName()
{
    // .odt import did crash on the empty font list (which I think is valid according SVG spec)
    load( "empty-svg-family-name.odt" );
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
