/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <com/sun/star/i18n/XOrdinalSuffix.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <unotest/bootstrapfixturebase.hxx>

using namespace com::sun::star;

class TestOrdinalSuffix : public test::BootstrapFixtureBase
{
private:
    uno::Reference<i18n::XOrdinalSuffix> m_xOrdinal;

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    void testFrench();
    void testEnglish();

    CPPUNIT_TEST_SUITE(TestOrdinalSuffix);
    CPPUNIT_TEST(testFrench);
    CPPUNIT_TEST(testEnglish);
    CPPUNIT_TEST_SUITE_END();
};

void TestOrdinalSuffix::setUp()
{
    BootstrapFixtureBase::setUp();
    m_xOrdinal.set(m_xSFactory->createInstance(u"com.sun.star.i18n.OrdinalSuffix"_ustr), uno::UNO_QUERY_THROW);
}

void TestOrdinalSuffix::tearDown()
{
    m_xOrdinal.clear();
    BootstrapFixtureBase::tearDown();
}

void TestOrdinalSuffix::testFrench()
{
    lang::Locale aLocale(u"fr"_ustr, u"LU"_ustr, u""_ustr);

    //1er
    uno::Sequence<OUString> aSuffixes = m_xOrdinal->getOrdinalSuffix(1, aLocale);
    const OUString* pFind = std::find(std::cbegin(aSuffixes), std::cend(aSuffixes), u"er"_ustr);
    CPPUNIT_ASSERT(pFind != std::cend(aSuffixes));

    //2e, 3e, etc.
    aSuffixes = m_xOrdinal->getOrdinalSuffix(2, aLocale);
    pFind = std::find(std::cbegin(aSuffixes), std::cend(aSuffixes), u"e"_ustr);
    CPPUNIT_ASSERT(pFind != std::cend(aSuffixes));
}

void TestOrdinalSuffix::testEnglish()
{
    lang::Locale aLocale(u"en"_ustr, u"US"_ustr, u""_ustr);

    //1st
    uno::Sequence<OUString> aSuffixes = m_xOrdinal->getOrdinalSuffix(1, aLocale);
    const OUString* pFind = std::find(std::cbegin(aSuffixes), std::cend(aSuffixes), u"st"_ustr);
    CPPUNIT_ASSERT(pFind != std::cend(aSuffixes));

    //2nd
    aSuffixes = m_xOrdinal->getOrdinalSuffix(2, aLocale);
    pFind = std::find(std::cbegin(aSuffixes), std::cend(aSuffixes), u"nd"_ustr);
    CPPUNIT_ASSERT(pFind != std::cend(aSuffixes));

    //3rd
    aSuffixes = m_xOrdinal->getOrdinalSuffix(3, aLocale);
    pFind = std::find(std::cbegin(aSuffixes), std::cend(aSuffixes), u"rd"_ustr);
    CPPUNIT_ASSERT(pFind != std::cend(aSuffixes));
}


CPPUNIT_TEST_SUITE_REGISTRATION( TestOrdinalSuffix );

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
