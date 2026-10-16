/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <o3tl/string_view.hxx>
#include <test/a11y/AccessibilityTools.hxx>
#include <test/a11y/accessibletestbase.hxx>
#include <unotest/macros_test.hxx>
#include <vcl/scheduler.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>

#include <vector>

using namespace css;

namespace
{
// The heading level is exposed as the "level" entry of the extended attributes,
// serialized to a string such as "level:2;".
// Returns -1 when there is none.
sal_Int32 getHeadingLevel(const uno::Reference<accessibility::XAccessibleContext>& xContext)
{
    uno::Reference<accessibility::XAccessibleExtendedAttributes> xAttributes(xContext,
                                                                             uno::UNO_QUERY);
    if (!xAttributes.is())
        return -1;

    const OUString sAttrs = xAttributes->getExtendedAttributes();
    if (sAttrs.isEmpty())
        return -1;

    sal_Int32 nIndex = 0;
    do
    {
        const OUString sToken = sAttrs.getToken(0, ';', nIndex);
        const sal_Int32 nColon = sToken.indexOf(':');
        if (nColon > 0 && sToken.subView(0, nColon) == u"level")
            return o3tl::toInt32(sToken.subView(nColon + 1));
    } while (nIndex >= 0 && nIndex < sAttrs.getLength());

    return -1;
}
} // namespace

// Checks that the headings of the document are exposed properly, in document
// order, with their level, including the ones whose paragraph style is not
// called "Heading N".
CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestHeadings)
{
    loadFromSrc(u"/sw/qa/extras/accessibility/testdocuments/headings_context.fodt"_ustr);

    auto xDocument = getDocumentAccessibleContext();
    CPPUNIT_ASSERT(xDocument.is());

    struct Heading
    {
        OUString text;
        sal_Int32 level;
    };
    const Heading aExpected[] = {
        { u"Introduction"_ustr, 1 }, // Heading 1
        { u"Scope"_ustr, 2 }, // Heading 2
        { u"Method"_ustr, 1 }, // uses the "PB" style, not "Heading 1"
        { u"Samples"_ustr, 2 }, // Heading 2
        { u"Collection"_ustr, 3 }, // Heading 3
        { u"Results"_ustr, 1 }, // uses the "PB" style, not "Heading 1"
    };

    // Collect the document headings
    std::vector<uno::Reference<accessibility::XAccessibleContext>> aHeadings;
    const bool bHeadingsFound
        = AccessibilityTools::Await([this, &xDocument, &aHeadings, &aExpected] {
              for (const auto& xChildContext : getAllChildren(xDocument))
              {
                  CPPUNIT_ASSERT(xChildContext.is());
                  if (xChildContext->getAccessibleRole() == accessibility::AccessibleRole::HEADING)
                      aHeadings.push_back(xChildContext);
              }
              return aHeadings.size() >= std::size(aExpected);
          });

    CPPUNIT_ASSERT_MESSAGE("expected all headings to be accessible; "
                           "are all pages visible?",
                           bHeadingsFound);

    CPPUNIT_ASSERT_EQUAL(std::size(aExpected), aHeadings.size());

    for (size_t i = 0; i < aHeadings.size(); ++i)
    {
        const uno::Reference<accessibility::XAccessibleContext>& xHeading = aHeadings[i];
        uno::Reference<accessibility::XAccessibleText> xText(xHeading, uno::UNO_QUERY);

        CPPUNIT_ASSERT(xText.is());
        CPPUNIT_ASSERT_EQUAL(aExpected[i].text, xText->getText());
        CPPUNIT_ASSERT_EQUAL(aExpected[i].level, getHeadingLevel(xHeading));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
