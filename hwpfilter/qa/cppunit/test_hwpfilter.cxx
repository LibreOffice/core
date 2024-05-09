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
#include <com/sun/star/document/XFilter.hpp>

#include <comphelper/propertyvalue.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>

using namespace ::com::sun::star;

namespace
{
    class HwpFilterTest
        : public test::FiltersTest
        , public test::BootstrapFixture
    {
    public:

        virtual bool load(const OUString &,
            const OUString &rURL, const OUString &,
            SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

        void test();

        CPPUNIT_TEST_SUITE(HwpFilterTest);
        CPPUNIT_TEST(test);
        CPPUNIT_TEST_SUITE_END();
    };

    bool HwpFilterTest::load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int)
    {
        uno::Reference<document::XFilter> xFilter(m_xSFactory->createInstance(u"com.sun.comp.hwpimport.HwpImportFilter"_ustr),
                                                  uno::UNO_QUERY_THROW);

        return xFilter->filter({ comphelper::makePropertyValue(u"URL"_ustr, rURL) });
    }

    void HwpFilterTest::test()
    {
        testDir(OUString(),
            m_directories.getURLFromSrc(u"/hwpfilter/qa/cppunit/data/"));
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(HwpFilterTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
