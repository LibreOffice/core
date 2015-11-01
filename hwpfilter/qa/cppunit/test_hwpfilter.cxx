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
        virtual void setUp() override;

        virtual bool load(const OUString &,
            const OUString &rURL, const OUString &,
            SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

        void test();

        CPPUNIT_TEST_SUITE(HwpFilterTest);
        CPPUNIT_TEST(test);
        CPPUNIT_TEST_SUITE_END();
    private:
        uno::Reference<document::XFilter> m_xFilter;
    };

    void HwpFilterTest::setUp()
    {
        test::BootstrapFixture::setUp();

        m_xFilter.set(m_xSFactory->createInstance("com.sun.comp.hwpimport.HwpImportFilter"),
                      uno::UNO_QUERY_THROW);
    }

    bool HwpFilterTest::load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int)
    {
        uno::Sequence< beans::PropertyValue > aDescriptor(1);
        aDescriptor[0].Name = "URL";
        aDescriptor[0].Value <<= rURL;
        return m_xFilter->filter(aDescriptor);
    }

    void HwpFilterTest::test()
    {
        testDir(OUString(),
            getURLFromSrc("/hwpfilter/qa/cppunit/data/"),
            OUString());
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(HwpFilterTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
