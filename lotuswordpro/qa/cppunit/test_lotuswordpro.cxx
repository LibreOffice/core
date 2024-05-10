/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <comphelper/propertyvalue.hxx>

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <com/sun/star/document/XFilter.hpp>

using namespace ::com::sun::star;

namespace
{
    class LotusWordProTest
        : public test::FiltersTest
        , public test::BootstrapFixture
    {
    public:
        LotusWordProTest() : BootstrapFixture(true, false) {}

        virtual void setUp() override;

        virtual bool load(const OUString &,
            const OUString &rURL, const OUString &,
            SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

        void test();

        CPPUNIT_TEST_SUITE(LotusWordProTest);
        CPPUNIT_TEST(test);
        CPPUNIT_TEST_SUITE_END();
    private:
        uno::Reference<document::XFilter> m_xFilter;
    };

    void LotusWordProTest::setUp()
    {
        test::BootstrapFixture::setUp();

        m_xFilter.set(m_xSFactory->createInstance(
            u"com.sun.star.comp.Writer.LotusWordProImportFilter"_ustr),
            uno::UNO_QUERY_THROW);
    }

    bool LotusWordProTest::load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int)
    {
        uno::Sequence aDescriptor{ comphelper::makePropertyValue(u"URL"_ustr, rURL) };
        return m_xFilter->filter(aDescriptor);
    }

    void LotusWordProTest::test()
    {
        testDir(OUString(),
            m_directories.getURLFromSrc(u"/lotuswordpro/qa/cppunit/data/"));
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(LotusWordProTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
