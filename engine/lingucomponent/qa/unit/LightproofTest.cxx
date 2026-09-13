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
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/linguistic2/LinguServiceEvent.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>

using namespace ::com::sun::star;
using namespace ::cpo;
using namespace ::cpo::uno;

namespace
{
// The Russian rule package, which the dictionaries module compiles from
// lightproof_ru_RU.py. Nothing has to be registered first: the checker finds
// its rule files in the install tree, which the test runs against.
class LightproofTest : public test::BootstrapFixtureBase
{
protected:
    uno::Reference<linguistic2::XProofreader> getProofreader();
    // The errors reported for a paragraph checked in Russian.
    Sequence<linguistic2::SingleProofreadingError> check(const OUString& rText);

    static lang::Locale russian() { return lang::Locale(u"ru"_ustr, u"RU"_ustr, OUString()); }
    static lang::Locale english() { return lang::Locale(u"en"_ustr, u"US"_ustr, OUString()); }

    // The errors reported for a paragraph checked in English.
    Sequence<linguistic2::SingleProofreadingError> checkEnglish(const OUString& rText);
    void setEnglishOption(const OUString& rName, bool bValue);

    static lang::Locale hungarian() { return lang::Locale(u"hu"_ustr, u"HU"_ustr, OUString()); }
    static lang::Locale brazilian() { return lang::Locale(u"pt"_ustr, u"BR"_ustr, OUString()); }
    Sequence<linguistic2::SingleProofreadingError> checkBrazilian(const OUString& rText);
    Sequence<linguistic2::SingleProofreadingError> checkHungarian(const OUString& rText);
    void setHungarianOption(const OUString& rName, bool bValue);

    // Writes one Russian option flag and commits, the way the Options dialog
    // does through the per-user xcu.
    void setOption(const OUString& rName, bool bValue);
};

void LightproofTest::setOption(const OUString& rName, bool bValue)
{
    uno::Reference<lang::XMultiServiceFactory> xProvider(
        css::configuration::theDefaultProvider::get(m_xContext));
    Any aNodePath(css::beans::NamedValue(
        u"nodepath"_ustr,
        Any(u"/org.openoffice.Office.Linguistic/GrammarChecking/SentenceChecking/ru_RU"_ustr)));
    uno::Reference<beans::XPropertySet> xGroup(
        xProvider->createInstanceWithArguments(
            u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr, { aNodePath }),
        uno::UNO_QUERY_THROW);
    xGroup->setPropertyValue(rName, Any(bValue));
    uno::Reference<util::XChangesBatch>(xGroup, uno::UNO_QUERY_THROW)->commitChanges();
}

void LightproofTest::setEnglishOption(const OUString& rName, bool bValue)
{
    uno::Reference<lang::XMultiServiceFactory> xProvider(
        css::configuration::theDefaultProvider::get(m_xContext));
    Any aNodePath(css::beans::NamedValue(
        u"nodepath"_ustr,
        Any(u"/org.openoffice.Office.Linguistic/GrammarChecking/SentenceChecking/en"_ustr)));
    uno::Reference<beans::XPropertySet> xGroup(
        xProvider->createInstanceWithArguments(
            u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr, { aNodePath }),
        uno::UNO_QUERY_THROW);
    xGroup->setPropertyValue(rName, Any(bValue));
    uno::Reference<util::XChangesBatch>(xGroup, uno::UNO_QUERY_THROW)->commitChanges();
}

Sequence<linguistic2::SingleProofreadingError>
LightproofTest::checkEnglish(const OUString& rText)
{
    return getProofreader()
        ->doProofreading(u"doc"_ustr, rText, english(), 0, rText.getLength(), {})
        .aErrors;
}

void LightproofTest::setHungarianOption(const OUString& rName, bool bValue)
{
    uno::Reference<lang::XMultiServiceFactory> xProvider(
        css::configuration::theDefaultProvider::get(m_xContext));
    Any aNodePath(css::beans::NamedValue(
        u"nodepath"_ustr,
        Any(u"/org.openoffice.Office.Linguistic/GrammarChecking/SentenceChecking/hu_HU"_ustr)));
    uno::Reference<beans::XPropertySet> xGroup(
        xProvider->createInstanceWithArguments(
            u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr, { aNodePath }),
        uno::UNO_QUERY_THROW);
    xGroup->setPropertyValue(rName, Any(bValue));
    uno::Reference<util::XChangesBatch>(xGroup, uno::UNO_QUERY_THROW)->commitChanges();
}

Sequence<linguistic2::SingleProofreadingError>
LightproofTest::checkHungarian(const OUString& rText)
{
    return getProofreader()
        ->doProofreading(u"doc"_ustr, rText, hungarian(), 0, rText.getLength(), {})
        .aErrors;
}

Sequence<linguistic2::SingleProofreadingError>
LightproofTest::checkBrazilian(const OUString& rText)
{
    return getProofreader()
        ->doProofreading(u"doc"_ustr, rText, brazilian(), 0, rText.getLength(), {})
        .aErrors;
}

uno::Reference<linguistic2::XProofreader> LightproofTest::getProofreader()
{
    return uno::Reference<linguistic2::XProofreader>(
        m_xSFactory->createInstance(u"cpo.lingu.Lightproof"_ustr), uno::UNO_QUERY_THROW);
}

Sequence<linguistic2::SingleProofreadingError> LightproofTest::check(const OUString& rText)
{
    const linguistic2::ProofreadingResult aResult = getProofreader()->doProofreading(
        u"doc"_ustr, rText, russian(), 0, rText.getLength(), {});
    return aResult.aErrors;
}

CPPUNIT_TEST_FIXTURE(LightproofTest, testSupportedLocales)
{
    uno::Reference<linguistic2::XProofreader> xProofreader(getProofreader());
    CPPUNIT_ASSERT(xProofreader->hasLocale(russian()));
    CPPUNIT_ASSERT(xProofreader->hasLocale(english()));
    CPPUNIT_ASSERT(!xProofreader->hasLocale(lang::Locale(u"de"_ustr, u"DE"_ustr, OUString())));
    CPPUNIT_ASSERT(!xProofreader->isSpellChecker());

    uno::Reference<lang::XServiceDisplayName> xDisplayName(xProofreader, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Lightproof grammar checker (Russian)"_ustr,
                         xDisplayName->getServiceDisplayName(russian()));
}

// A rule with no condition reports the match, its suggestion and its message.
CPPUNIT_TEST_FIXTURE(LightproofTest, testUnconditionalRule)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors = check(u"Here foo is."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors[0].aSuggestions.getLength());
    CPPUNIT_ASSERT_EQUAL(u"bar"_ustr, aErrors[0].aSuggestions[0]);
    CPPUNIT_ASSERT_EQUAL(u"test"_ustr, aErrors[0].aShortComment);
}

// The ellipsis rule, whose message was corrected in the table in 2021 and in
// the rule source only now.
CPPUNIT_TEST_FIXTURE(LightproofTest, testRussianEllipsis)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = check(u"Вот так ... и всё."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(u"…"_ustr, aErrors[0].aSuggestions[0]);
    CPPUNIT_ASSERT_EQUAL(u"Символ многоточия."_ustr,
                         aErrors[0].aShortComment);
}

// A rule whose condition is an option that defaults on fires.
CPPUNIT_TEST_FIXTURE(LightproofTest, testRuleEnabledByDefault)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = check(u"Он какбудто "
                u"ушёл."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(u"как будто"_ustr,
                         aErrors[0].aSuggestions[0]);
}

// The suggestion of a rule using a named group comes back with the group
// expanded, and the pattern's own backreference has to have matched for the
// rule to fire at all.
CPPUNIT_TEST_FIXTURE(LightproofTest, testNamedGroupExpansion)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = check(u"Это было было "
                u"давно."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(u"было"_ustr, aErrors[0].aSuggestions[0]);
}

// Rules behind an option that defaults off stay silent, even though their
// pattern matches. Without the option list the two texts below would each
// report an error.
CPPUNIT_TEST_FIXTURE(LightproofTest, testRuleDisabledByDefault)
{
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        check(u"Число 12345 тут."_ustr).getLength());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        check(u"Он сказал "
              u"\"привет\" мне."_ustr)
            .getLength());
}

// Turning an option on in the configuration makes its rules fire, and the
// running checker picks the change up without being recreated.
CPPUNIT_TEST_FIXTURE(LightproofTest, testOptionTurnedOn)
{
    const OUString aText(u"Число 12345 тут."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), check(aText).getLength());

    setOption(u"numsep"_ustr, true);
    const Sequence<linguistic2::SingleProofreadingError> aErrors = check(aText);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aErrors[0].nErrorLength);

    setOption(u"numsep"_ustr, false);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), check(aText).getLength());
}

// Turning an option off silences its rules.
CPPUNIT_TEST_FIXTURE(LightproofTest, testOptionTurnedOff)
{
    const OUString aText(u"Он какбудто ушёл."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), check(aText).getLength());

    setOption(u"together"_ustr, false);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), check(aText).getLength());

    setOption(u"together"_ustr, true);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), check(aText).getLength());
}

// A configuration change asks the caller to check the document again.
CPPUNIT_TEST_FIXTURE(LightproofTest, testOptionChangeAsksForRecheck)
{
    class Listener : public cppu::WeakImplHelper<linguistic2::XLinguServiceEventListener>
    {
    public:
        sal_Int32 nProofreadAgain = 0;
        void processLinguServiceEvent(const linguistic2::LinguServiceEvent& rEvent) override
        {
            if (rEvent.nEvent & linguistic2::LinguServiceEventFlags::PROOFREAD_AGAIN)
                ++nProofreadAgain;
        }
        void disposing(const lang::EventObject&) override {}
    };

    uno::Reference<linguistic2::XLinguServiceEventBroadcaster> xBroadcaster(getProofreader(),
                                                                           uno::UNO_QUERY_THROW);
    rtl::Reference<Listener> pListener(new Listener);
    CPPUNIT_ASSERT(xBroadcaster->addLinguServiceEventListener(pListener));

    setOption(u"numsep"_ustr, true);
    CPPUNIT_ASSERT(pListener->nProofreadAgain > 0);
    setOption(u"numsep"_ustr, false);

    CPPUNIT_ASSERT(xBroadcaster->removeLinguServiceEventListener(pListener));
}

// Only the request that starts at the beginning of the paragraph reports
// anything: a paragraph is checked as one unit.
CPPUNIT_TEST_FIXTURE(LightproofTest, testParagraphIsCheckedOnce)
{
    const OUString aText(u"Here foo is. Here foo is."_ustr);
    uno::Reference<linguistic2::XProofreader> xProofreader(getProofreader());

    const linguistic2::ProofreadingResult aFirst
        = xProofreader->doProofreading(u"doc"_ustr, aText, russian(), 0, 12, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aFirst.aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(aText.getLength(), aFirst.nStartOfNextSentencePosition);
    CPPUNIT_ASSERT_EQUAL(aText.getLength(), aFirst.nBehindEndOfSentencePosition);

    const linguistic2::ProofreadingResult aSecond
        = xProofreader->doProofreading(u"doc"_ustr, aText, russian(), 13, aText.getLength(), {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aSecond.aErrors.getLength());
}

// An ignored rule stops reporting until the ignore list is reset.
CPPUNIT_TEST_FIXTURE(LightproofTest, testIgnoreRule)
{
    uno::Reference<linguistic2::XProofreader> xProofreader(getProofreader());
    const OUString aText(u"Here foo is."_ustr);

    const linguistic2::ProofreadingResult aBefore
        = xProofreader->doProofreading(u"doc"_ustr, aText, russian(), 0, aText.getLength(), {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aBefore.aErrors.getLength());

    xProofreader->ignoreRule(aBefore.aErrors[0].aRuleIdentifier, russian());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        xProofreader->doProofreading(u"doc"_ustr, aText, russian(), 0, aText.getLength(), {})
            .aErrors.getLength());

    xProofreader->resetIgnoreRules();
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(1),
        xProofreader->doProofreading(u"doc"_ustr, aText, russian(), 0, aText.getLength(), {})
            .aErrors.getLength());
}

// A locale with no rule package is not answered at all.
CPPUNIT_TEST_FIXTURE(LightproofTest, testUnknownLocale)
{
    const linguistic2::ProofreadingResult aResult = getProofreader()->doProofreading(
        u"doc"_ustr, u"Here foo is."_ustr, lang::Locale(u"de"_ustr, u"DE"_ustr, OUString()), 0, 12,
        {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aResult.aErrors.getLength());
}

// A message built by looking a punctuation mark up in the package's table and
// joining the pieces.
CPPUNIT_TEST_FIXTURE(LightproofTest, testEnglishMessageFromTable)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkEnglish(u"This is a test ,and more."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aErrors.getLength());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(14), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(u"Reversed space and punctuation?"_ustr, aErrors[0].aShortComment);
    CPPUNIT_ASSERT_EQUAL(u", "_ustr, aErrors[0].aSuggestions[0]);

    CPPUNIT_ASSERT_EQUAL(u"Extra space before the comma?"_ustr, aErrors[1].aShortComment);
    CPPUNIT_ASSERT_EQUAL(u","_ustr, aErrors[1].aSuggestions[0]);
}

// A condition that looks the matched word up in one of the package's word
// sets, and a message whose long form is a URL.
CPPUNIT_TEST_FIXTURE(LightproofTest, testEnglishWordSet)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkEnglish(u"I saw an eucalyptus tree."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(13), aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(u"a eucalyptus"_ustr, aErrors[0].aSuggestions[0]);
    CPPUNIT_ASSERT_EQUAL(u"Did you mean:"_ustr, aErrors[0].aShortComment);

    // A comment holding a link is offered as a property, and the long form
    // falls back to the short one.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors[0].aProperties.getLength());
    CPPUNIT_ASSERT_EQUAL(u"FullCommentURL"_ustr, aErrors[0].aProperties[0].Name);
    CPPUNIT_ASSERT_EQUAL(aErrors[0].aShortComment, aErrors[0].aFullComment);
}

CPPUNIT_TEST_FIXTURE(LightproofTest, testEnglishWordSetForAn)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkEnglish(u"It took an hour and a hour."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(u"an hour"_ustr, aErrors[0].aSuggestions[0]);
}

// A pattern whose character class holds an opening bracket, which ICU reads
// as a nested set where Python re reads a plain character. The rule is
// dropped outright if the class is not escaped on the way in.
CPPUNIT_TEST_FIXTURE(LightproofTest, testEnglishBracketInCharacterClass)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkEnglish(u"I used ( x ) here."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(u"Extra space before the closing parenthesis?"_ustr,
                         aErrors[0].aShortComment);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aErrors[1].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(u"Extra space after the opening parenthesis?"_ustr,
                         aErrors[1].aShortComment);
    CPPUNIT_ASSERT_EQUAL(u"("_ustr, aErrors[1].aSuggestions[0]);
}

// A condition that asks the spelling dictionary about the matched word.
CPPUNIT_TEST_FIXTURE(LightproofTest, testEnglishSpellCondition)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkEnglish(u"a apple a day"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(u"an apple"_ustr, aErrors[0].aSuggestions[0]);
}

// Unit conversion, which is behind an option that is off by default.
CPPUNIT_TEST_FIXTURE(LightproofTest, testEnglishMeasurement)
{
    const OUString aText(u"Give me 5 lb of it."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), checkEnglish(aText).getLength());

    setEnglishOption(u"metric"_ustr, true);
    const Sequence<linguistic2::SingleProofreadingError> aErrors = checkEnglish(aText);
    setEnglishOption(u"metric"_ustr, false);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(u"Convert to metric:"_ustr, aErrors[0].aShortComment);
    CPPUNIT_ASSERT(aErrors[0].aSuggestions.hasElements());
    CPPUNIT_ASSERT_EQUAL(u"2 kg"_ustr, aErrors[0].aSuggestions[0]);
}
}

// A replacement built with str.replace(), gated on a slice compared against
// a literal. The same phrase is also a foreign expression, so the package's
// phrase list answers for it as well.
CPPUNIT_TEST_FIXTURE(LightproofTest, testHungarianStringReplace)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkHungarian(u"Ez vis major eset."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aErrors.getLength());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(u"vis maior"_ustr, aErrors[0].aSuggestions[0]);
    CPPUNIT_ASSERT_EQUAL(u"Helyesen vis maior."_ustr, aErrors[0].aShortComment);

    CPPUNIT_ASSERT_EQUAL(u"vis maior"_ustr, aErrors[1].aSuggestions[0]);
    CPPUNIT_ASSERT_EQUAL(u"Idegen eredetű kifejezés?"_ustr, aErrors[1].aShortComment);
}

// A condition that looks at the word before the match, which needs a slice
// whose bound is the match position rather than a literal.
CPPUNIT_TEST_FIXTURE(LightproofTest, testHungarianPrecedingWord)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkHungarian(u"Ez nagyon figyelemreméltó dolog."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(15), aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(u"figyelemre méltó"_ustr, aErrors[0].aSuggestions[0]);

    // The same phrase after "igen" is correct, and the rule says so.
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0), checkHungarian(u"Ez igen figyelemreméltó dolog."_ustr).getLength());
}

// A condition that runs a pattern written out in the rule itself.
CPPUNIT_TEST_FIXTURE(LightproofTest, testHungarianInlinePattern)
{
    setHungarianOption(u"comma"_ustr, true);
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkHungarian(u"Tudtam amit hogy kell."_ustr);
    setHungarianOption(u"comma"_ustr, false);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(u"amit, hogy"_ustr, aErrors[0].aSuggestions[0]);
}

// A suggestion built by substituting with a pattern written out in the rule.
CPPUNIT_TEST_FIXTURE(LightproofTest, testHungarianInlineSubstitution)
{
    setHungarianOption(u"thin"_ustr, true);
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkHungarian(u"Ez r i t k í t o t t szöveg."_ustr);
    setHungarianOption(u"thin"_ustr, false);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(17), aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aErrors[0].aSuggestions.getLength());
    CPPUNIT_ASSERT_EQUAL(u"ritkított"_ustr, aErrors[0].aSuggestions[0]);
    // The second offers the same letters separated by narrow no-break spaces.
    CPPUNIT_ASSERT_EQUAL(u"r\u202Fi\u202Ft\u202Fk\u202Fí\u202Ft\u202Fo\u202Ft\u202Ft"_ustr,
                         aErrors[0].aSuggestions[1]);
}

// One of the two pleonasm rules taken from upstream, which gate on the option
// the whole pleonasm block uses.
CPPUNIT_TEST_FIXTURE(LightproofTest, testBrazilianPleonasm)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkBrazilian(u"Essa e a primeira prioridade dele."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(19), aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(u"prioridade"_ustr, aErrors[0].aSuggestions[0]);
    CPPUNIT_ASSERT_EQUAL(u"Pleonasmo. Você quis dizer:"_ustr, aErrors[0].aShortComment);
}

// A rule with no condition at all, from the largest package.
CPPUNIT_TEST_FIXTURE(LightproofTest, testBrazilianPlainRule)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkBrazilian(u"Ele tem em celebro humano."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(u"em cérebro"_ustr, aErrors[0].aSuggestions[0]);
}

// An agreement rule, which is the shape most of the package's rules take: a
// condition looking the matched words up in the package's word lists. The
// pattern starts with an alternation, so it also guards against the
// paragraph filter treating one branch's text as required.
CPPUNIT_TEST_FIXTURE(LightproofTest, testBrazilianAgreement)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkBrazilian(u"Havia dezenas de indicativo aqui."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21), aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(u"dezenas de indicativos"_ustr, aErrors[0].aSuggestions[0]);
}

// Two rules answering for the same phrase, each offering both readings.
CPPUNIT_TEST_FIXTURE(LightproofTest, testBrazilianTwoSuggestions)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkBrazilian(u"Comprei uma casas novas."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aErrors[0].aSuggestions.getLength());
    CPPUNIT_ASSERT_EQUAL(u"uma casa"_ustr, aErrors[0].aSuggestions[0]);
    CPPUNIT_ASSERT_EQUAL(u"umas casas"_ustr, aErrors[0].aSuggestions[1]);
    CPPUNIT_ASSERT_EQUAL(u"Comprei uma casa"_ustr, aErrors[1].aSuggestions[0]);
}

// A replacement offering two whole phrases, which the rules separate with a
// newline.
CPPUNIT_TEST_FIXTURE(LightproofTest, testBrazilianMultipleSuggestions)
{
    const Sequence<linguistic2::SingleProofreadingError> aErrors
        = checkBrazilian(u"As pessoas já foram comunicadas ontem."_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aErrors[0].aSuggestions.getLength());
    CPPUNIT_ASSERT_EQUAL(u"pessoas já foram informadas"_ustr, aErrors[0].aSuggestions[0]);
    CPPUNIT_ASSERT_EQUAL(u"pessoas já foram avisadas"_ustr, aErrors[0].aSuggestions[1]);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
