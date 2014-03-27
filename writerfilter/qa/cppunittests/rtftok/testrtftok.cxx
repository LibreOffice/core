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
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>

#include <osl/file.hxx>
#include <osl/process.h>

using namespace ::com::sun::star;

/**
 * Unit test invoking the Writer RTF import filter.
 *
 * This does only minimal testing, checking if the filter crashes and returns
 * the expected bool value for given inputs. More fine-grained tests can be
 * found under sw/qa/extras/rtfimport/.
 */
class RtfTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:

    virtual void setUp() SAL_OVERRIDE;

    virtual bool load(const OUString&,
                      const OUString& rURL, const OUString&,
                      unsigned int, unsigned int, unsigned int) SAL_OVERRIDE;

    void test();

    CPPUNIT_TEST_SUITE(RtfTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference<document::XFilter> m_xFilter;
};

void RtfTest::setUp()
{
    test::BootstrapFixture::setUp();

    m_xFilter = uno::Reference< document::XFilter >(m_xSFactory->createInstance("com.sun.star.comp.Writer.RtfFilter"), uno::UNO_QUERY_THROW);
}

bool RtfTest::load(const OUString&,
                   const OUString& rURL, const OUString&,
                   unsigned int, unsigned int, unsigned int)
{
    uno::Sequence< beans::PropertyValue > aDescriptor(1);
    aDescriptor[0].Name = "URL";
    aDescriptor[0].Value <<= rURL;
    try
    {
        return m_xFilter->filter(aDescriptor);
    }
    catch (const lang::WrappedTargetRuntimeException& rWrapped)
    {
        io::WrongFormatException e;
        if (rWrapped.TargetException >>= e)
        {
            return false;
        }
        throw;
    }
}

void RtfTest::test()
{
    testDir(OUString(),
            getURLFromSrc("/writerfilter/qa/cppunittests/rtftok/data/"),
            OUString());
}

CPPUNIT_TEST_SUITE_REGISTRATION(RtfTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
