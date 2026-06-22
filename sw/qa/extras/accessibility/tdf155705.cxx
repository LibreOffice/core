/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/accessibility/XAccessibleTextMarkup.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager2.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <i18nlangtag/lang.h>

#include <vcl/scheduler.hxx>

#include <test/a11y/accessibletestbase.hxx>
#include <test/a11y/AccessibilityTools.hxx>

using namespace css;

/* This test only works if spell checking is available for en_US */
CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, tdf155705)
{
    loadFromSrc(u"/sw/qa/extras/accessibility/testdocuments/tdf155705.fodt"_ustr);

    /* we need to give it a second to fully set up the XAccessibleTextMarkup, otherwise we get a
     * dysfunctional one that asserts out on m_pTextFrame */
    Scheduler::ProcessEventsToIdle();

    uno::Reference<linguistic2::XLinguServiceManager2> xLSM2
        = linguistic2::LinguServiceManager::create(m_xContext);
    uno::Reference<linguistic2::XSpellChecker1> xSpell(xLSM2->getSpellChecker(), uno::UNO_QUERY);
    if (!xSpell.is() || !xSpell->hasLanguage(static_cast<sal_uInt16>(LANGUAGE_ENGLISH_US)))
        return;

    auto xContext = getDocumentAccessibleContext();
    CPPUNIT_ASSERT(xContext.is());

    auto xChildContext = getAllChildren(xContext).back();
    uno::Reference<accessibility::XAccessibleTextMarkup> xMarkup(xChildContext,
                                                                 uno::UNO_QUERY_THROW);

    // make sure the idle spellcheck pass runs
    AccessibilityTools::Await(
        [&xMarkup]() { return 0 < xMarkup->getTextMarkupCount(text::TextMarkupType::SPELLCHECK); });

    CPPUNIT_ASSERT_GREATER(sal_Int32(0),
                           xMarkup->getTextMarkupCount(text::TextMarkupType::SPELLCHECK));
    auto aSegment = xMarkup->getTextMarkup(0, text::TextMarkupType::SPELLCHECK);
    CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(0), aSegment.SegmentStart);
    CPPUNIT_ASSERT_GREATER(aSegment.SegmentStart, aSegment.SegmentEnd);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
