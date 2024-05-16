/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <unotools/configpaths.hxx>

namespace
{
class Test : public CppUnit::TestFixture
{
public:
    void testSplitLastFromConfigurationPath()
    {
        {
            OUString path, last;
            CPPUNIT_ASSERT(!utl::splitLastFromConfigurationPath(u"", path, last));
            CPPUNIT_ASSERT_EQUAL(u""_ustr, path);
            CPPUNIT_ASSERT_EQUAL(u""_ustr, last);
        }
        {
            // Already prior to 5edefc801fb48559c8064003f23d22d838710ee4 "use more string_view in
            // unotools", and in discordance with the documentation, this returned true (but
            // "@returns <FALSE/>, if the path was a one-level path or an invalid path"):
            OUString path, last;
            CPPUNIT_ASSERT(utl::splitLastFromConfigurationPath(u"/", path, last));
            CPPUNIT_ASSERT_EQUAL(u""_ustr, path);
            CPPUNIT_ASSERT_EQUAL(u""_ustr, last);
        }
        {
            // Already prior to 5edefc801fb48559c8064003f23d22d838710ee4 "use more string_view in
            // unotools", and in discordance with the documentation, this returned true (but
            // "@returns <FALSE/>, if the path was a one-level path or an invalid path"):
            OUString path, last;
            CPPUNIT_ASSERT(utl::splitLastFromConfigurationPath(u"/foo", path, last));
            CPPUNIT_ASSERT_EQUAL(u""_ustr, path);
            CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, last);
        }
        {
            // Already prior to 5edefc801fb48559c8064003f23d22d838710ee4 "use more string_view in
            // unotools", and in discordance with the documentation, this returned true (but
            // "@returns <FALSE/>, if the path was a one-level path or an invalid path"):
            OUString path, last;
            CPPUNIT_ASSERT(utl::splitLastFromConfigurationPath(u"/foo/", path, last));
            CPPUNIT_ASSERT_EQUAL(u""_ustr, path);
            CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, last);
        }
        {
            OUString path, last;
            CPPUNIT_ASSERT(utl::splitLastFromConfigurationPath(u"/foo/bar/baz", path, last));
            CPPUNIT_ASSERT_EQUAL(u"/foo/bar"_ustr, path);
            CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, last);
        }
        {
            // Trailing slash accepted for backwards compatibility (cf
            // . "for backwards compatibility, ignore a final slash" comment in
            // Data::resolvePathRepresentation, configmgr/source/data.cxx):
            OUString path, last;
            CPPUNIT_ASSERT(utl::splitLastFromConfigurationPath(u"/foo/bar/baz/", path, last));
            CPPUNIT_ASSERT_EQUAL(u"/foo/bar"_ustr, path);
            CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, last);
        }
        {
            OUString path, last;
            CPPUNIT_ASSERT(utl::splitLastFromConfigurationPath(
                u"/foo/bar/baz['abc']/baz['de&amp;f']", path, last));
            CPPUNIT_ASSERT_EQUAL(u"/foo/bar/baz['abc']"_ustr, path);
            CPPUNIT_ASSERT_EQUAL(u"de&f"_ustr, last);
        }
        {
            OUString path, last;
            CPPUNIT_ASSERT(!utl::splitLastFromConfigurationPath(u"foo", path, last));
            CPPUNIT_ASSERT_EQUAL(u""_ustr, path);
            CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, last);
        }
        {
            // In accordance with the documentation, this sets last to "foo/" ("If
            // <var>_sInPath</var> could not be parsed as a valid configuration path, this is set to
            // <var>_sInPath</var>"):
            OUString path, last;
            CPPUNIT_ASSERT(!utl::splitLastFromConfigurationPath(u"foo/", path, last));
            CPPUNIT_ASSERT_EQUAL(u""_ustr, path);
            CPPUNIT_ASSERT_EQUAL(u"foo/"_ustr, last);
        }
        {
            // Some broken input missing a leading slash happens to be considered OK:
            OUString path, last;
            CPPUNIT_ASSERT(utl::splitLastFromConfigurationPath(u"foo/bar/baz", path, last));
            CPPUNIT_ASSERT_EQUAL(u"foo/bar"_ustr, path);
            CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, last);
        }
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testSplitLastFromConfigurationPath);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
