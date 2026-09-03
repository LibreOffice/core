/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/bootstrapfixturebase.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

using namespace ::com::sun::star;
using namespace ::cpo::uno;

namespace
{
class SpellCheckTest : public test::BootstrapFixtureBase
{
public:
    virtual void setUp() override;
};

void SpellCheckTest::setUp()
{
    test::BootstrapFixtureBase::setUp();

    // Register one spelling dictionary that names two locales, the shape the bundled Italian
    // and Dutch dictionaries have.
    uno::Reference<lang::XMultiServiceFactory> xProvider(
        css::configuration::theDefaultProvider::get(m_xContext));
    Any aNodePath(css::beans::NamedValue(
        u"nodepath"_ustr,
        Any(u"/org.openoffice.Office.Linguistic/ServiceManager/Dictionaries"_ustr)));
    uno::Reference<container::XNameContainer> xDictionaries(
        xProvider->createInstanceWithArguments(
            u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr, { aNodePath }),
        uno::UNO_QUERY_THROW);

    const OUString aLocation = m_directories.getURLFromSrc(u"/lingucomponent/qa/unit/data/spell");
    uno::Reference<beans::XPropertySet> xEntry(
        uno::Reference<lang::XSingleServiceFactory>(xDictionaries, uno::UNO_QUERY_THROW)
            ->createInstance(),
        uno::UNO_QUERY_THROW);
    xEntry->setPropertyValue(u"Locations"_ustr,
                             Any(Sequence<OUString>{ aLocation + ".aff", aLocation + ".dic" }));
    xEntry->setPropertyValue(u"Format"_ustr, Any(u"DICT_SPELL"_ustr));
    xEntry->setPropertyValue(u"Locales"_ustr,
                             Any(Sequence<OUString>{ u"it-IT"_ustr, u"it-CH"_ustr }));
    xDictionaries->insertByName(u"TestDictionary"_ustr, Any(xEntry));
    uno::Reference<util::XChangesBatch>(xDictionaries, uno::UNO_QUERY_THROW)->commitChanges();
}

// A word added at runtime is accepted in the locale it was added for and in no other locale of
// the same dictionary.
CPPUNIT_TEST_FIXTURE(SpellCheckTest, testAddedWordStaysInItsLocale)
{
    uno::Reference<linguistic2::XSpellChecker> xSpell(
        m_xSFactory->createInstance(u"org.openoffice.lingu.MySpellSpellChecker"_ustr),
        uno::UNO_QUERY_THROW);

    const lang::Locale aItalian(u"it"_ustr, u"IT"_ustr, OUString());
    const lang::Locale aSwissItalian(u"it"_ustr, u"CH"_ustr, OUString());

    CPPUNIT_ASSERT(xSpell->hasLocale(aItalian));
    CPPUNIT_ASSERT(xSpell->hasLocale(aSwissItalian));

    // Both locales spell against the word list of the one registered dictionary.
    CPPUNIT_ASSERT(xSpell->isValid(u"foo"_ustr, aItalian, {}));
    CPPUNIT_ASSERT(xSpell->isValid(u"foo"_ustr, aSwissItalian, {}));
    CPPUNIT_ASSERT(!xSpell->isValid(u"hello"_ustr, aItalian, {}));
    CPPUNIT_ASSERT(!xSpell->isValid(u"hello"_ustr, aSwissItalian, {}));

    // Add a word for it-IT. This is the query that a personal dictionary entry with a
    // "Grammar By" word sends while spell checking.
    xSpell->spell(u"<?xml?><query type='add'><word>hello</word></query>"_ustr, aItalian, {});

    CPPUNIT_ASSERT(xSpell->isValid(u"hello"_ustr, aItalian, {}));
    CPPUNIT_ASSERT(!xSpell->isValid(u"hello"_ustr, aSwissItalian, {}));

    // The words from the dictionary files still spell in both locales.
    CPPUNIT_ASSERT(xSpell->isValid(u"bar"_ustr, aItalian, {}));
    CPPUNIT_ASSERT(xSpell->isValid(u"bar"_ustr, aSwissItalian, {}));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
