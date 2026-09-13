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
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

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
};

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
