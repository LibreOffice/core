/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/i18n/IndexEntrySupplier.hpp>
#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/bootstrap.hxx>

namespace
{
class IndexEntry : public CppUnit::TestFixture
{
public:
    void setUp()
    {
        supplier_ = css::i18n::IndexEntrySupplier::create(
            cppu::defaultBootstrap_InitialComponentContext());
    }

    void testJapanese()
    {
        css::lang::Locale loc(u"ja"_ustr, u"JP"_ustr, u""_ustr);
        auto const s = supplier_->getAlgorithmList(loc);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), s.getLength());
        CPPUNIT_ASSERT(
            comphelper::findValue(s, "phonetic (alphanumeric first) (grouped by consonant)") != -1);
        CPPUNIT_ASSERT(
            comphelper::findValue(s, "phonetic (alphanumeric first) (grouped by syllable)") != -1);
        CPPUNIT_ASSERT(
            comphelper::findValue(s, "phonetic (alphanumeric last) (grouped by consonant)") != -1);
        CPPUNIT_ASSERT(
            comphelper::findValue(s, "phonetic (alphanumeric last) (grouped by syllable)") != -1);
        CPPUNIT_ASSERT(supplier_->loadAlgorithm(
            loc, u"phonetic (alphanumeric first) (grouped by consonant)"_ustr, 0));
        CPPUNIT_ASSERT(supplier_->loadAlgorithm(
            loc, u"phonetic (alphanumeric first) (grouped by syllable)"_ustr, 0));
        CPPUNIT_ASSERT(supplier_->loadAlgorithm(
            loc, u"phonetic (alphanumeric last) (grouped by consonant)"_ustr, 0));
        CPPUNIT_ASSERT(supplier_->loadAlgorithm(
            loc, u"phonetic (alphanumeric last) (grouped by syllable)"_ustr, 0));
    }

    CPPUNIT_TEST_SUITE(IndexEntry);
    CPPUNIT_TEST(testJapanese);
    CPPUNIT_TEST_SUITE_END();

private:
    css::uno::Reference<css::i18n::XExtendedIndexEntrySupplier> supplier_;
};

CPPUNIT_TEST_SUITE_REGISTRATION(IndexEntry);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
