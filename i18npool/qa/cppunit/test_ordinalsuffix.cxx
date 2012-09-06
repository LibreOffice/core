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
#include <comphelper/stlunosequence.hxx>
#include <unotest/bootstrapfixturebase.hxx>

using namespace com::sun::star;

class TestOrdinalSuffix : public test::BootstrapFixtureBase
{
private:
    uno::Reference<i18n::XOrdinalSuffix> m_xOrdinal;

public:
    virtual void setUp();
    virtual void tearDown();

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
    m_xOrdinal = uno::Reference< i18n::XOrdinalSuffix >(m_xSFactory->createInstance(
        "com.sun.star.i18n.OrdinalSuffix"), uno::UNO_QUERY_THROW);
}

void TestOrdinalSuffix::tearDown()
{
    m_xOrdinal.clear();
    BootstrapFixtureBase::tearDown();
}

void TestOrdinalSuffix::testFrench()
{
    lang::Locale aLocale("fr", "LU", "");
    uno::Sequence< OUString > aSuffixes;
    OUString *pStart, *pEnd, *pFind;

    //1er
    aSuffixes = m_xOrdinal->getOrdinalSuffix(1, aLocale);
    pStart = comphelper::stl_begin(aSuffixes);
    pEnd = comphelper::stl_end(aSuffixes);
    pFind = std::find(pStart, pEnd, OUString("er"));
    CPPUNIT_ASSERT(pFind != pEnd);

    //2e, 3e, etc.
    aSuffixes = m_xOrdinal->getOrdinalSuffix(2, aLocale);
    pStart = comphelper::stl_begin(aSuffixes);
    pEnd = comphelper::stl_end(aSuffixes);
    pFind = std::find(pStart, pEnd, OUString("e"));
    CPPUNIT_ASSERT(pFind != pEnd);
}

void TestOrdinalSuffix::testEnglish()
{
    lang::Locale aLocale("en", "US", "");
    uno::Sequence< OUString > aSuffixes;
    OUString *pStart, *pEnd, *pFind;

    //1st
    aSuffixes = m_xOrdinal->getOrdinalSuffix(1, aLocale);
    pStart = comphelper::stl_begin(aSuffixes);
    pEnd = comphelper::stl_end(aSuffixes);
    pFind = std::find(pStart, pEnd, OUString("st"));
    CPPUNIT_ASSERT(pFind != pEnd);

    //2nd
    aSuffixes = m_xOrdinal->getOrdinalSuffix(2, aLocale);
    pStart = comphelper::stl_begin(aSuffixes);
    pEnd = comphelper::stl_end(aSuffixes);
    pFind = std::find(pStart, pEnd, OUString("nd"));
    CPPUNIT_ASSERT(pFind != pEnd);

    //3rd
    aSuffixes = m_xOrdinal->getOrdinalSuffix(3, aLocale);
    pStart = comphelper::stl_begin(aSuffixes);
    pEnd = comphelper::stl_end(aSuffixes);
    pFind = std::find(pStart, pEnd, OUString("rd"));
    CPPUNIT_ASSERT(pFind != pEnd);
}


CPPUNIT_TEST_SUITE_REGISTRATION( TestOrdinalSuffix );

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
