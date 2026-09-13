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
    CPPUNIT_ASSERT(!xProofreader->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())));
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
        u"doc"_ustr, u"Here foo is."_ustr, lang::Locale(u"en"_ustr, u"US"_ustr, OUString()), 0, 12,
        {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aResult.aErrors.getLength());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
