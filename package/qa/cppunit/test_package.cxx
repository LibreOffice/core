/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <unotest/filters-test.hxx>
#include <unotest/bootstrapfixturebase.hxx>
#include "com/sun/star/packages/zip/ZipFileAccess.hpp"

using namespace ::com::sun::star;

namespace
{
    class PackageTest
        : public test::FiltersTest
        , public test::BootstrapFixtureBase
    {
    public:
        PackageTest() {}

        virtual bool load(const OUString &,
            const OUString &rURL, const OUString &,
            SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

        void test();

        CPPUNIT_TEST_SUITE(PackageTest);
        CPPUNIT_TEST(test);
        CPPUNIT_TEST_SUITE_END();
    };

    bool PackageTest::load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int)
    {
        try
        {
            uno::Reference<css::packages::zip::XZipFileAccess2> xZip(
                css::packages::zip::ZipFileAccess::createWithURL(comphelper::getProcessComponentContext(), rURL));
            return xZip.is();
        }
        catch(...)
        {
            return false;
        }
    }

    void PackageTest::test()
    {
        testDir(OUString(),
            getURLFromSrc("/package/qa/cppunit/data/"),
            OUString());
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(PackageTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
