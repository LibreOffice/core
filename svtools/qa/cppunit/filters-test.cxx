/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 *       Caolán McNamara <caolanm@redhat.com>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>

#include <osl/file.hxx>
#include <osl/process.h>

#include <svtools/filter.hxx>

using namespace ::com::sun::star;

/* Implementation of Filters test */

class SvtoolsFiltersTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    SvtoolsFiltersTest() : BootstrapFixture(true, false) {}

    virtual bool load(const rtl::OUString &,
        const rtl::OUString &rURL, const rtl::OUString &,
        unsigned int, unsigned int, unsigned int);

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    CPPUNIT_TEST_SUITE(SvtoolsFiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();
};

bool SvtoolsFiltersTest::load(const rtl::OUString &,
    const rtl::OUString &rURL, const rtl::OUString &,
    unsigned int, unsigned int, unsigned int)
{
    GraphicFilter aGraphicFilter(false);
    SvFileStream aFileStream(rURL, STREAM_READ);
    Graphic aGraphic;
    return aGraphicFilter.ImportGraphic(aGraphic, rURL, aFileStream) == 0;
}

void SvtoolsFiltersTest::testCVEs()
{
    testDir(rtl::OUString(),
        getURLFromSrc("/svtools/qa/cppunit/data/wmf/"),
        rtl::OUString());

    testDir(rtl::OUString(),
        getURLFromSrc("/svtools/qa/cppunit/data/emf/"),
        rtl::OUString());

    testDir(rtl::OUString(),
        getURLFromSrc("/svtools/qa/cppunit/data/sgv/"),
        rtl::OUString());

    testDir(rtl::OUString(),
        getURLFromSrc("/svtools/qa/cppunit/data/png/"),
        rtl::OUString());

    testDir(rtl::OUString(),
        getURLFromSrc("/svtools/qa/cppunit/data/jpg/"),
        rtl::OUString());

    testDir(rtl::OUString(),
        getURLFromSrc("/svtools/qa/cppunit/data/gif/"),
        rtl::OUString());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SvtoolsFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
