/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
        virtual void setUp();

        virtual bool load(const rtl::OUString &,
            const rtl::OUString &rURL, const rtl::OUString &,
            unsigned int, unsigned int, unsigned int);

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

        m_xFilter = uno::Reference< document::XFilter >(m_xSFactory->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.comp.hwpimport.HwpImportFilter"))),
            uno::UNO_QUERY_THROW);
    }

    bool HwpFilterTest::load(const rtl::OUString &,
        const rtl::OUString &rURL, const rtl::OUString &,
        unsigned int, unsigned int, unsigned int)
    {
        uno::Sequence< beans::PropertyValue > aDescriptor(1);
        aDescriptor[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"));
        aDescriptor[0].Value <<= rURL;
        return m_xFilter->filter(aDescriptor);
    }

    void HwpFilterTest::test()
    {
        testDir(rtl::OUString(),
            getURLFromSrc("/hwpfilter/qa/cppunit/data/"),
            rtl::OUString());
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(HwpFilterTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
